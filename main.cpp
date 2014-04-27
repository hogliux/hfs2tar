#include <stdint.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <boost/fusion/container/map.hpp>
#include <boost/fusion/include/map.hpp>
#include <boost/fusion/container/map/map_fwd.hpp>
#include <boost/fusion/include/map_fwd.hpp>
#include <boost/fusion/sequence/intrinsic/at_key.hpp>
#include <boost/fusion/include/at_key.hpp>
#include <boost/fusion/include/io.hpp>

#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/convert.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "apm.hpp"
#include "hfsplus.hpp"
#include "locale.hpp"
#include "bigendian_binary_iarchive.hpp"
#include "imemstream.hpp"
#include "fastunicodecompare.hpp"

class HFSUniStr {
private:
  std::basic_string<uint16_t> m_str;
public:
  HFSUniStr() {}
  HFSUniStr(const HFSUniStr & other) : m_str(other.m_str) {}
  HFSUniStr& operator=(const HFSUniStr& other) {
    m_str = other.m_str;
    return *this;
  }
  template <typename Archive>
  void serialize(Archive & a, unsigned int version) {
    uint16_t length;
    a >> length;
    m_str.clear();
    for ( unsigned int i=0; i<length; ++i ) {
      uint16_t rune;
      a >> rune;
      m_str.push_back(rune);
    }
  }
  
  const std::basic_string<uint16_t> & str() const {
    return m_str;
  }
};

class BTreeKey {
private:
  uint32_t m_bTreeAttributes;
  uint32_t m_maxKeyLength;
  uint16_t m_key_length;
  int8_t m_kind;
  std::basic_string<uint8_t> m_data;
private:
  template <typename Archive>
  static void skip( Archive & ar, unsigned int bytes ) {
    for ( unsigned int i=0; i<bytes; ++i ) {
      uint8_t byte;
      ar & byte;
    }
  }
public:
  BTreeKey( uint32_t bTreeAttributes, uint32_t maxKeyLength, int8_t kind )
    : m_bTreeAttributes(bTreeAttributes), m_maxKeyLength(maxKeyLength), m_kind(kind) {
  }

  const std::basic_string<uint8_t> & data() const {
    return m_data;
  }
  
  unsigned int length() const {
    return m_key_length;
  }

  template <typename Archive>
  void serialize( Archive & ar, unsigned int version ) {
    m_data.clear();
    int bytes=0;
    if ( ( m_bTreeAttributes & kBTBigKeysMask ) != 0 ) {
      ar & m_key_length;
      bytes += 2;
    } else {
      uint8_t byte;
      ar & byte;
      m_key_length = byte;
      bytes++;
    }
    for ( unsigned int i=0; i<m_key_length; ++i ) {
      uint8_t byte;
      ar & byte;
      m_data.push_back(byte);
      bytes++;
    }
    /* how much to skip until data */
    if ( ( m_kind == kBTIndexNode ) && ( ( m_bTreeAttributes & kBTVariableIndexKeysMask ) == 0 ) ) {
      skip(ar, m_maxKeyLength-m_key_length);
      bytes += (m_maxKeyLength-m_key_length);
    }
    if ( ( bytes & 1 ) != 0 ) {
      uint8_t pad;
      ar & pad;
    }
  }
};

class BTreeCatalogKey {
private:
  uint32_t m_parent_id;
  HFSUniStr m_node_name;
public:
  BTreeCatalogKey() : m_parent_id(0), m_node_name() {}
  template <typename Archive>
  void serialize( Archive & ar, unsigned int version ) {
    ar & m_parent_id;
    ar & m_node_name;
  }
  
  uint32_t parent() const {
    return m_parent_id;
  }

  const std::basic_string<uint16_t> & NodeName() const {
    return m_node_name.str();
  }
  
};

void advance_to_next_block( std::istream & str, unsigned int blockSize ) {
  int pos = str.tellg();
  if ( pos < 0 ) {
    throw std::runtime_error( "Position shouldn't be negative" );
  }
  if ( ( ((unsigned int)pos) % blockSize ) != 0 ) {
    str.seekg( ( ( pos / blockSize ) + 1 ) * blockSize, str.beg);
  }
}

typedef union {
  HFSPlusCatalogFile file;
  HFSPlusCatalogFolder folder;
} HFSCatalogFileFolder;

typedef struct {
  bool isFile;
  HFSCatalogFileFolder info;
} tCatalogEntry;

class BTree {
private:
  std::istream & m_str;
  unsigned int m_btree_offset;
  BTHeaderRec m_header;
public:
  BTree( std::istream & str, unsigned int btree_offset )
    : m_str(str), m_btree_offset(btree_offset) {
      m_str.seekg( m_btree_offset, m_str.beg );
      bigendian_binary_iarchive ar(m_str);
      /* we cannot read the first node with read_btree_node because we don't know the nodeSize yet */
      BTNodeDescriptor node;
      ar & node;
      if ( node.kind != kBTHeaderNode ) {
	throw std::runtime_error("Expected catalog file to start with a b-tree header node");
      }
      if ( node.numRecords < 3 ) {
	throw std::runtime_error("B-tree header node needs to have at least 3 records");
      }
      ar & m_header;
  }
public:
  bool find_node( uint32_t parent_CNID, const std::basic_string<uint16_t> & node_name, tCatalogEntry & result ) {
    return find_node( parent_CNID, node_name, result, m_header.rootNode );
  }

  bool find_children( uint32_t CNID, std::vector<std::basic_string<uint16_t> > & children ) {
    return find_children( CNID, children, m_header.rootNode );
  }
private:
  bool find_children( uint32_t CNID, std::vector<std::basic_string<uint16_t> > & children, uint32_t start_node ) {
    unsigned int node_offset = m_btree_offset+(start_node*m_header.nodeSize);
    m_str.seekg( node_offset, m_str.beg );
    bigendian_binary_iarchive ar(m_str);
    
    BTNodeDescriptor node;
    ar & node;
    if ( ( node.kind != kBTLeafNode ) && ( node.kind != kBTIndexNode ) ) {
      throw std::runtime_error( "find_node only works in index or leaf nodes" );
    }
    
    /* read-in record offsets */
    unsigned int record_offset_table_offset = m_header.nodeSize - ( 2 * (node.numRecords + 1 ) );
    m_str.seekg( node_offset+record_offset_table_offset, m_str.beg );
    std::vector<uint16_t> record_table(node.numRecords+1);
    for ( int i=0; i<(node.numRecords+1); ++i ) {
      ar & record_table[node.numRecords-i];
    }
    
    uint32_t last_CNID = 0;
    uint32_t last_child = 0;
    bool result = false;
    /* process records */
    for ( std::vector<uint16_t>::const_iterator it = record_table.begin();
	  it != (record_table.end()-1); ++it ) {
      m_str.seekg( node_offset+(*it), m_str.beg );
      BTreeCatalogKey catalogKey;
      {
	BTreeKey bKey(m_header.attributes, m_header.maxKeyLength, node.kind);
	ar & bKey;
	{
	  std::basic_string<uint8_t> data(bKey.data());
	  imemstream stream((char*)data.c_str(), data.size());
	  bigendian_binary_iarchive key_ar(stream);	  
	  key_ar & catalogKey;
	}
      }
      if ( node.kind == kBTIndexNode ) {
	uint32_t new_child;
	ar & new_child;
	if ( last_CNID == CNID ) {
	  if ( last_child != 0 ) {
	    result = true;
	    find_children( CNID, children, last_child );
	  }
	} else {
	  if ( catalogKey.parent() > CNID ) {
	    if ( last_child != 0 ) {
	      result |= find_children(CNID, children, last_child);
	      return result;
	    } else {
	      return false;
	    }
	  } else {
	    if ( ( catalogKey.parent() == CNID ) && ( hfscmp( catalogKey.NodeName(), std::basic_string<uint16_t>() ) > 0 ) ) {
	      if ( last_child != 0 ) {
		result = true;
		find_children(CNID, children, last_child);
	      }
	    }
	  }
	}
	last_CNID = catalogKey.parent();
	last_child = new_child;
      } else {
	if ( CNID == catalogKey.parent() ) {
	  result = true;
	  children.push_back( catalogKey.NodeName() );
	}
      }
    }
    if ( ( last_CNID <= CNID ) && ( node.kind == kBTIndexNode ) ) {
      result |= find_children( CNID, children, last_child );
    }
    return result;
  }

  bool find_node( uint32_t parent_CNID, const std::basic_string<uint16_t> & node_name, tCatalogEntry & result, 
		  uint32_t start_node ) {
    unsigned int node_offset = m_btree_offset+(start_node*m_header.nodeSize);
    m_str.seekg( node_offset, m_str.beg );
    bigendian_binary_iarchive ar(m_str);
    
    BTNodeDescriptor node;
    ar & node;
    if ( ( node.kind != kBTLeafNode ) && ( node.kind != kBTIndexNode ) ) {
      throw std::runtime_error( "find_node only works in index or leaf nodes" );
    }

    /* read-in record offsets */
    unsigned int record_offset_table_offset = m_header.nodeSize - ( 2 * (node.numRecords + 1 ) );
    m_str.seekg( node_offset+record_offset_table_offset, m_str.beg );
    std::vector<uint16_t> record_table(node.numRecords+1);
    for ( int i=0; i<(node.numRecords+1); ++i ) {
      ar & record_table[node.numRecords-i];
    }
    
    uint32_t last_child = 0;
    uint32_t last_parent_CNID = 0;
    /* process records */
    for ( std::vector<uint16_t>::const_iterator it = record_table.begin();
	  it != (record_table.end()-1); ++it ) {
      m_str.seekg( node_offset+(*it), m_str.beg );
      BTreeCatalogKey catalogKey;
      {
	BTreeKey bKey(m_header.attributes, m_header.maxKeyLength, node.kind);
	ar & bKey;
	{
	  std::basic_string<uint8_t> data(bKey.data());
	  imemstream stream((char*)data.c_str(), data.size());
	  bigendian_binary_iarchive key_ar(stream);	  
	  key_ar & catalogKey;
	}
      }
      if ( node.kind == kBTIndexNode ) {
	uint32_t new_child;
	ar & new_child;
	if ( (  parent_CNID < catalogKey.parent() ) || 
	     ( ( parent_CNID == catalogKey.parent() ) && ( hfscmp(node_name, catalogKey.NodeName() ) < 0 ) ) ) {
	  if ( last_child == 0 ) {
	    return false;
	  }
	  return find_node( parent_CNID, node_name, result, last_child );
	}
	last_child = new_child;
	last_parent_CNID = catalogKey.parent();
      } else {
	int16_t record_type;
	ar & record_type;
	if ( ( parent_CNID == catalogKey.parent() ) && ( hfscmp( node_name, catalogKey.NodeName() ) == 0 ) && 
	     ( ( record_type == kHFSPlusFolderRecord ) || ( record_type == kHFSPlusFileRecord ) ) ) {
	  result.isFile = ( record_type == kHFSPlusFileRecord );
	  if ( result.isFile ) {
	    ar & result.info.file;
	  } else {
	    ar & result.info.folder;
	  }
	  return true;
	}
      }
    }
    if ( ( node.kind == kBTIndexNode ) && ( last_parent_CNID < parent_CNID ) ) {
      return find_node( parent_CNID, node_name, result, last_child );
    } else {
      return false;
    }
  }
};

struct PosixTarHeader {
  char name[100];
  char mode[8];
  char uid[8];
  char gid[8];
  char size[12];
  char mtime[12];
  char checksum[8];
  char typeflag[1];
  char linkname[100];
  char magic[6];
  char version[2];
  char uname[32];
  char gname[32];
  char devmajor[8];
  char devminor[8];
  char prefix[155];
  char pad[12];
};

void print_octal( char * result, int octal, int size ) {
  int i;
  for ( i=0; i<size; ++i ) {
    result[i] = '0' + ( ( octal >> ( 3 * ( size - i - 1 ) ) ) & 7 );
  }
  result[i] = 0;
}

void print_directory( std::istream & str, uint32_t blockSize, uint32_t hfs_offset,
		      BTree & b_tree, uint32_t dir_CNID, const std::string & path ) {
  std::vector<std::basic_string<uint16_t> > children;
  b_tree.find_children(dir_CNID, children);
  char buffer[512];
  for ( std::vector<std::basic_string<uint16_t> >::const_iterator it = children.begin();
	it != children.end(); ++it ) {
    tCatalogEntry result;
    if ( b_tree.find_node( dir_CNID, *it, result ) == true ) {
      std::fill( buffer, &buffer[512], 0 );
      struct PosixTarHeader* tar_header = (struct PosixTarHeader*)buffer;
      {
	std::stringstream ss;
	ss << "./" << path << utf16_to_utf8(*it);
	if ( result.isFile == false ) {
	  ss << "/";
	}
	strcpy( tar_header->name, ss.str().c_str() );
      }
      memset( tar_header->checksum, ' ', 8 );
      print_octal( tar_header->mode, result.info.file.permissions.fileMode & (~HFS_IFMT), 7 );
      print_octal( tar_header->uid, result.info.file.permissions.ownerID, 7 );
      print_octal( tar_header->uid, result.info.file.permissions.groupID, 7 );
      if ( result.isFile == false ) {
	print_octal( tar_header->size, 0, 11 );
      } else {
	print_octal( tar_header->size, result.info.file.dataFork.logicalSize, 11 );
      }
      print_octal( tar_header->mtime, result.info.file.contentModDate, 11 );
      if ( ( result.info.file.permissions.fileMode & HFS_IFREG ) != 0 ) {
	tar_header->typeflag[0] = '0';
      } else if ( ( result.info.file.permissions.fileMode & HFS_IFLNK ) != 0 ) {
	tar_header->typeflag[0] = '2';
      } else if ( ( result.info.file.permissions.fileMode & HFS_IFDIR ) != 0 ) {
	tar_header->typeflag[0] = '5';
      } else if ( ( result.info.file.permissions.fileMode & HFS_IFCHR ) != 0 ) {
	tar_header->typeflag[0] = '3';
	print_octal( tar_header->devmajor, 
		     ( result.info.file.permissions.iNodeNum & 0xFFFF0000 ) >> 16, 7 );
	print_octal( tar_header->devminor, 
		     result.info.file.permissions.iNodeNum & 0x0000FFFF, 7 );
      } else if ( ( result.info.file.permissions.fileMode & HFS_IFBLK ) != 0 ) {
	tar_header->typeflag[0] = '4';
	print_octal( tar_header->devmajor, 
		     ( result.info.file.permissions.iNodeNum & 0xFFFF0000 ) >> 16, 7 );
	print_octal( tar_header->devminor, 
		     result.info.file.permissions.iNodeNum & 0x0000FFFF, 7 );
      } else if ( ( result.info.file.permissions.fileMode & HFS_IFIFO ) != 0 ) {
	tar_header->typeflag[0] = '6';
      }
      strcpy( tar_header->magic, "ustar  " );
      {
	unsigned int checksum = 0;
	char * buf = buffer;
	for ( unsigned int i=0; i<512; ++i ) {
	  checksum += buf[i] & 0xFF;
	}
	print_octal( tar_header->checksum, checksum, 6 );
      }
      std::cout.write(buffer, 512);
      if ( result.isFile == true ) {
	uint32_t file_length = result.info.file.dataFork.logicalSize;
	char block[512];
	unsigned int extent = 0;
        unsigned int file_pos = 0;
	unsigned int pos_in_extent = 0;
	str.seekg( (blockSize*result.info.file.dataFork.extents[extent].startBlock)+hfs_offset, str.beg );
	while ( file_pos < file_length ) {
	  unsigned int pos = 0;
	  unsigned int block_len = file_length - file_pos;
	  block_len = block_len < 512 ? block_len : 512;
	  memset( block, 0, 512 );
	  while ( pos < block_len ) {
	    unsigned int left = (result.info.file.dataFork.extents[extent].blockCount*blockSize)-pos_in_extent;
	    unsigned int len = (block_len-pos) < left ? (block_len-pos) : left;
	    str.read( &block[pos], len );
	    pos += len;
	    pos_in_extent += len;
	    if ( pos_in_extent >= (result.info.file.dataFork.extents[extent].blockCount*blockSize) ) {
	      extent++;
	      if ( extent >= 8 ) {
		throw std::runtime_error( "hfs2tar does not support HFS volumes with extent overflows" );
	      }
	      pos_in_extent=0;
	      str.seekg( (blockSize*result.info.file.dataFork.extents[extent].startBlock)+hfs_offset, str.beg );
	    }
	  }
	  /* always write full blocks */
	  std::cout.write(block, 512);
	  file_pos += block_len;
	}
      } else {
	/* this is a folder */
	print_directory( str, blockSize, hfs_offset, b_tree, result.info.folder.folderID, path + utf16_to_utf8(*it) + std::string("/") );
      }
    }
  }
}

void output_hfsplus( std::istream & str, unsigned int offset ) {
  str.ignore(1024+offset);
  bigendian_binary_iarchive ar(str);
  HFSPlusVolumeHeader volume_header;
  ar & volume_header;
  if ( (volume_header.signature[0] != 'H') || (volume_header.signature[1] != '+') ) {
    throw std::runtime_error( "This is not a HFS+ volume" );
  }

  /* read catalog file into memory */
  std::stringstream catalog_file(std::ios_base::in | std::ios_base::out | std::ios_base::binary);
  {
    unsigned int i=0;
    for ( uint32_t blocks=0; (blocks<volume_header.catalogFile.totalBlocks) && (i<8); 
	  blocks+=volume_header.catalogFile.extents[i++].blockCount ) {
      str.seekg(offset+(volume_header.blockSize*volume_header.catalogFile.extents[i].startBlock),str.beg);
      char buffer[volume_header.blockSize];
      for ( unsigned int j=0; j<volume_header.catalogFile.extents[i].blockCount; ++j ) {
	str.read(buffer, volume_header.blockSize);
	catalog_file.write(buffer,volume_header.blockSize);
      }
    }
    if ( i >= 8 ) {
      throw std::runtime_error( "We do not support files that have extents in the extents overflow file yet" );
    }
  }
  
  BTree b_tree(catalog_file, 0);
  //  print_directory( str, volume_header.blockSize, offset, b_tree, kHFSRootFolderID, "" );
  print_directory( str, volume_header.blockSize, offset, b_tree, kHFSRootParentID, "" );
}

int main( int argc, char * argv[] ) {
  if ( argc < 2 ) {
    std::cerr << "Usage: hfs2tar raw-image-file.img" << std::endl;
    return 1;
  }
  std::ifstream test(argv[1], std::ios::binary);
  bigendian_binary_iarchive ar(test);
  
  APMBlock0 superblock;
  ar & superblock;
  if ((superblock.sbSig[0] != 'E') || (superblock.sbSig[1] != 'R')) {
    throw std::runtime_error( "This is not a valid disk image starting with an Apple Partition Map" );
  }
  advance_to_next_block( test, superblock.sbBlkSize );
  {
    std::vector<APMPartitionEntryStruct> partition_map(1);
    partition_map[0].numberOfPartitions = 1;
    for ( unsigned int i=0; i<partition_map[0].numberOfPartitions; ++i ) {
      ar & partition_map[i];
      if ((partition_map[i].signature[0] != 'P') || (partition_map[i].signature[1] != 'M')) {
	throw std::runtime_error( "Invalid Apple Partition Map" );
      }
      partition_map.push_back(APMPartitionEntryStruct());
      advance_to_next_block( test, superblock.sbBlkSize );
    }
    for ( std::vector<APMPartitionEntryStruct>::const_iterator it = partition_map.begin();
	  it != partition_map.end(); ++it ) {
      if ( std::string(it->typeOfPartition) == std::string("Apple_HFS") ) {
	test.seekg(0,test.beg);
	output_hfsplus( test, it->startingSectorOfPartition*superblock.sbBlkSize );
	return 0;
      }
    }
    throw std::runtime_error("Unable to find Apple HFS partition");
  }

  return 0;
}
