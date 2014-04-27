#pragma once
#include <stdint.h>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include "fusion_map_serialization.hpp"

struct HFSUniStr255 {
  uint16_t  length;
  uint16_t unicode[255];
};

typedef char tHFSUniStrData[255];
BOOST_FUSION_ADAPT_STRUCT(
			  HFSUniStr255,
			  (uint16_t,length)
			  (tHFSUniStrData,unicode))
BOOST_FUSION_ADD_SERIALIZER( HFSUniStr255 )


struct HFSPlusExtentDescriptor {
  uint32_t                  startBlock;
  uint32_t                  blockCount;
};

BOOST_FUSION_ADAPT_STRUCT(
			  HFSPlusExtentDescriptor,
			  (uint32_t,startBlock)
			  (uint32_t,blockCount))
BOOST_FUSION_ADD_SERIALIZER( HFSPlusExtentDescriptor )


typedef HFSPlusExtentDescriptor HFSPlusExtentRecord[8];

struct HFSPlusForkData {
  uint64_t                  logicalSize;
  uint32_t                  clumpSize;
  uint32_t                  totalBlocks;
  HFSPlusExtentRecord     extents;
};

BOOST_FUSION_ADAPT_STRUCT(
			  HFSPlusForkData,
			  (uint64_t,logicalSize)
			  (uint32_t,clumpSize)
			  (uint32_t,totalBlocks)
			  (HFSPlusExtentRecord,extents))
BOOST_FUSION_ADD_SERIALIZER( HFSPlusForkData )

struct HFSPlusVolumeHeader {
  char              signature[2];
  uint16_t              version;
  uint32_t              attributes;
  uint32_t              lastMountedVersion;
  uint32_t              journalInfoBlock;
 
  uint32_t              createDate;
  uint32_t              modifyDate;
  uint32_t              backupDate;
  uint32_t              checkedDate;
 
  uint32_t              fileCount;
  uint32_t              folderCount;
 
  uint32_t              blockSize;
  uint32_t              totalBlocks;
  uint32_t              freeBlocks;
 
  uint32_t              nextAllocation;
  uint32_t              rsrcClumpSize;
  uint32_t              dataClumpSize;
  uint32_t              nextCatalogID;
 
  uint32_t              writeCount;
  uint64_t              encodingsBitmap;
 
  uint32_t              finderInfo[8];
 
  HFSPlusForkData     allocationFile;
  HFSPlusForkData     extentsFile;
  HFSPlusForkData     catalogFile;
  HFSPlusForkData     attributesFile;
  HFSPlusForkData     startupFile;
};

typedef char tHFSPlusSignature[2];
typedef uint32_t tHFSPlusFinderInfo[8];

BOOST_FUSION_ADAPT_STRUCT(
			  HFSPlusVolumeHeader,
			  (tHFSPlusSignature,signature)
			  (uint16_t,version)
			  (uint32_t,attributes)
			  (uint32_t,lastMountedVersion)
			  (uint32_t,journalInfoBlock)
			  (uint32_t,createDate)
			  (uint32_t,modifyDate)
			  (uint32_t,backupDate)
			  (uint32_t,checkedDate)
			  (uint32_t,fileCount)
			  (uint32_t,folderCount)
			  (uint32_t,blockSize)
			  (uint32_t,totalBlocks)
			  (uint32_t,freeBlocks)
			  (uint32_t,nextAllocation)
			  (uint32_t,rsrcClumpSize)
			  (uint32_t,dataClumpSize)
			  (uint32_t,nextCatalogID)
			  (uint32_t,writeCount)
			  (uint64_t,encodingsBitmap)
			  (tHFSPlusFinderInfo,finderInfo)
			  (HFSPlusForkData,allocationFile)
			  (HFSPlusForkData,extentsFile)
			  (HFSPlusForkData,catalogFile)
			  (HFSPlusForkData,attributesFile)
			  (HFSPlusForkData,startupFile))
BOOST_FUSION_ADD_SERIALIZER( HFSPlusVolumeHeader )
			  
enum {
  kBTInvalid        = -2,
  kBTLeafNode       = -1,
  kBTIndexNode      =  0,
  kBTHeaderNode     =  1,
  kBTMapNode        =  2
};

#define BTNodeDescriptorSize 14

struct BTNodeDescriptor {
  uint32_t    fLink;
  uint32_t    bLink;
  int8_t     kind;
  uint8_t    height;
  uint16_t    numRecords;
  uint16_t    reserved;
};

BOOST_FUSION_ADAPT_STRUCT(
			  BTNodeDescriptor,
			  (uint32_t,fLink)
			  (uint32_t,bLink)
			  (int8_t, kind)
			  (uint8_t,height)
			  (uint16_t,numRecords)
			  (uint16_t,reserved))
BOOST_FUSION_ADD_SERIALIZER( BTNodeDescriptor )

enum {
  kBTBadCloseMask           = 0x00000001,
  kBTBigKeysMask            = 0x00000002,
  kBTVariableIndexKeysMask  = 0x00000004
};

enum {
  kHFSRootParentID            = 1,
  kHFSRootFolderID            = 2,
  kHFSExtentsFileID           = 3,
  kHFSCatalogFileID           = 4,
  kHFSBadBlockFileID          = 5,
  kHFSAllocationFileID        = 6,
  kHFSStartupFileID           = 7,
  kHFSAttributesFileID        = 8,
  kHFSRepairCatalogFileID     = 14,
  kHFSBogusExtentFileID       = 15,
  kHFSFirstUserCatalogNodeID  = 16
};

struct BTHeaderRec {
  uint16_t    treeDepth;
  uint32_t    rootNode;
  uint32_t    leafRecords;
  uint32_t    firstLeafNode;
  uint32_t    lastLeafNode;
  uint16_t    nodeSize;
  uint16_t    maxKeyLength;
  uint32_t    totalNodes;
  uint32_t    freeNodes;
  uint16_t    reserved1;
  uint32_t    clumpSize;      // misaligned
  uint8_t     btreeType;
  uint8_t     keyCompareType;
  uint32_t    attributes;     // long aligned again
  uint32_t    reserved3[16];
};

typedef uint32_t tBTHeaderRecReserved3[16];

BOOST_FUSION_ADAPT_STRUCT(
			  BTHeaderRec,
			  (uint16_t,treeDepth)
			  (uint32_t,rootNode)
			  (uint32_t,leafRecords)
			  (uint32_t,firstLeafNode)
			  (uint32_t,lastLeafNode)
			  (uint16_t,nodeSize)
			  (uint16_t,maxKeyLength)
			  (uint32_t,totalNodes)
			  (uint32_t,freeNodes)
			  (uint16_t,reserved1)
			  (uint32_t,clumpSize)
			  (uint8_t,btreeType)
			  (uint8_t,keyCompareType)
			  (uint32_t,attributes)
			  (tBTHeaderRecReserved3,reserved3))
BOOST_FUSION_ADD_SERIALIZER( BTHeaderRec )

struct HFSPlusBSDInfo {
  uint32_t  ownerID;
  uint32_t  groupID;
  uint8_t   adminFlags;
  uint8_t   ownerFlags;
  uint16_t  fileMode;
  uint32_t iNodeNum; /* or link count, or raw device */
};

BOOST_FUSION_ADAPT_STRUCT(HFSPlusBSDInfo,
			  (uint32_t,ownerID)
			  (uint32_t,groupID)
			  (uint8_t,adminFlags)
			  (uint8_t,ownerFlags)
			  (uint16_t,fileMode)
			  (uint32_t,iNodeNum))
BOOST_FUSION_ADD_SERIALIZER(HFSPlusBSDInfo)

struct HFSPoint {
  int16_t              v;
  int16_t              h;
};
BOOST_FUSION_ADAPT_STRUCT(HFSPoint,
			  (int16_t,v)
			  (int16_t,h))
BOOST_FUSION_ADD_SERIALIZER(HFSPoint)

struct HFSRect {
  int16_t              top;
  int16_t              left;
  int16_t              bottom;
  int16_t              right;
};
BOOST_FUSION_ADAPT_STRUCT(HFSRect,
			  (int16_t,top)
			  (int16_t,left)
			  (int16_t,bottom)
			  (int16_t,right))
BOOST_FUSION_ADD_SERIALIZER(HFSRect)

typedef char OSType[4];

struct FileInfo {
  OSType    fileType;           /* The type of the file */
  OSType    fileCreator;        /* The file's creator */
  uint16_t    finderFlags;
  HFSPoint     location;           /* File's location in the folder. */
  uint16_t    reservedField;
};
BOOST_FUSION_ADAPT_STRUCT(FileInfo,
			  (OSType,fileType)
			  (OSType,fileCreator)
			  (uint16_t,finderFlags)
			  (HFSPoint,location)
			  (uint16_t,reservedField))
BOOST_FUSION_ADD_SERIALIZER(FileInfo)

typedef int16_t tExtendedFileInfoReserved1[4];

struct ExtendedFileInfo {
  tExtendedFileInfoReserved1    reserved1;
  uint16_t    extendedFinderFlags;
  int16_t    reserved2;
  int32_t    putAwayFolderID;
};
BOOST_FUSION_ADAPT_STRUCT(ExtendedFileInfo,
			  (tExtendedFileInfoReserved1,reserved1)
			  (uint16_t,extendedFinderFlags)
			  (int16_t,reserved2)
			  (int32_t,putAwayFolderID))
BOOST_FUSION_ADD_SERIALIZER(ExtendedFileInfo)

struct FolderInfo {
  HFSRect      windowBounds;       /* The position and dimension of the */
                                /* folder's window */
  uint16_t    finderFlags;
  HFSPoint     location;           /* Folder's location in the parent */
                                /* folder. If set to {0, 0}, the Finder */
                                /* will place the item automatically */
  uint16_t    reservedField;
};

BOOST_FUSION_ADAPT_STRUCT(FolderInfo,
			  (HFSRect,windowBounds)
			  (uint16_t,finderFlags)
			  (HFSPoint,location)
			  (uint16_t,reservedField))
BOOST_FUSION_ADD_SERIALIZER(FolderInfo)

struct ExtendedFolderInfo {
  HFSPoint     scrollPosition;     /* Scroll position (for icon views) */
  int32_t    reserved1;
  uint16_t    extendedFinderFlags;
  int16_t    reserved2;
  int32_t    putAwayFolderID;
};
BOOST_FUSION_ADAPT_STRUCT(ExtendedFolderInfo,
			  (HFSPoint,scrollPosition)
			  (int32_t,reserved1)
			  (uint16_t,extendedFinderFlags)
			  (int16_t,reserved2)
			  (int32_t,putAwayFolderID))
BOOST_FUSION_ADD_SERIALIZER(ExtendedFolderInfo)

enum {
  kHFSPlusFolderRecord        = 0x0001,
  kHFSPlusFileRecord          = 0x0002,
  kHFSPlusFolderThreadRecord  = 0x0003,
  kHFSPlusFileThreadRecord    = 0x0004
};

struct HFSPlusCatalogFolder {
  uint16_t              flags;
  uint32_t              valence;
  uint32_t              folderID;
  uint32_t              createDate;
  uint32_t              contentModDate;
  uint32_t              attributeModDate;
  uint32_t              accessDate;
  uint32_t              backupDate;
  HFSPlusBSDInfo      permissions;
  FolderInfo          userInfo;
  ExtendedFolderInfo  finderInfo;
  uint32_t              textEncoding;
  uint32_t              reserved;
};

BOOST_FUSION_ADAPT_STRUCT(
			  HFSPlusCatalogFolder,
			  (uint16_t,flags)
			  (uint32_t,valence)
			  (uint32_t,folderID)
			  (uint32_t,createDate)
			  (uint32_t,contentModDate)
			  (uint32_t,attributeModDate)
			  (uint32_t,accessDate)
			  (uint32_t,backupDate)
			  (HFSPlusBSDInfo,permissions)
			  (FolderInfo,userInfo)
			  (ExtendedFolderInfo,finderInfo)
			  (uint32_t,textEncoding)
			  (uint32_t,reserved))
BOOST_FUSION_ADD_SERIALIZER( HFSPlusCatalogFolder )

struct HFSPlusCatalogFile {
  uint16_t              flags;
  uint32_t              reserved1;
  uint32_t    fileID;
  uint32_t              createDate;
  uint32_t              contentModDate;
  uint32_t              attributeModDate;
  uint32_t              accessDate;
  uint32_t              backupDate;
  HFSPlusBSDInfo      permissions;
  FileInfo            userInfo;
  ExtendedFileInfo    finderInfo;
  uint32_t              textEncoding;
  uint32_t              reserved2;
 
  HFSPlusForkData     dataFork;
  HFSPlusForkData     resourceFork;
};

BOOST_FUSION_ADAPT_STRUCT(
			  HFSPlusCatalogFile,
			  (uint16_t,flags)
			  (uint32_t,reserved1)
			  (uint32_t,fileID)
			  (uint32_t,createDate)
			  (uint32_t,contentModDate)
			  (uint32_t,attributeModDate)
			  (uint32_t,accessDate)
			  (uint32_t,backupDate)
			  (HFSPlusBSDInfo,permissions)
			  (FileInfo,userInfo)
			  (ExtendedFileInfo,finderInfo)
			  (uint32_t,textEncoding)
			  (uint32_t,reserved2)
			  (HFSPlusForkData,dataFork)
			  (HFSPlusForkData,resourceFork))
BOOST_FUSION_ADD_SERIALIZER( HFSPlusCatalogFile )

#define HFS_IFMT   0170000    /* type of file mask */
#define HFS_IFIFO  0010000    /* named pipe (fifo) */
#define HFS_IFCHR  0020000    /* character special */
#define HFS_IFDIR  0040000    /* directory */
#define HFS_IFBLK  0060000    /* block special */
#define HFS_IFREG  0100000    /* regular */
#define HFS_IFLNK  0120000    /* symbolic link */
#define HFS_IFSOCK 0140000    /* socket */
#define HFS_IFWHT  0160000    /* whiteout */
