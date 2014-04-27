#pragma once
#include <stdint.h>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include "fusion_map_serialization.hpp"

struct APMDDMap {
  uint32_t  ddBlock;              /* (driver's block start, sbBlkSize-blocks) */
  uint16_t  ddSize;               /* (driver's block count, 512-blocks)       */
  uint16_t  ddType;               /* (driver's system type)                   */
};

BOOST_FUSION_ADAPT_STRUCT(
			  APMDDMap,
			  (uint32_t,ddBlock)
			  (uint16_t,ddSize)
			  (uint16_t,ddType))
BOOST_FUSION_ADD_SERIALIZER( APMDDMap )

struct APMBlock0 {
  char  sbSig[2];                     /* (unique value for block zero, 'ER') */
  uint16_t  sbBlkSize;                 /* (block size for this device)        */
  uint32_t  sbBlkCount;                /* (block count for this device)       */
  uint16_t  sbDevType;                 /* (device type)                       */
  uint16_t  sbDevId;                   /* (device id)                         */
  uint32_t  sbDrvrData;                /* (driver data)                       */
  uint16_t  sbDrvrCount;               /* (driver descriptor count)           */
  struct APMDDMap   sbDrvrMap[8];              /* (driver descriptor table)           */
  uint8_t   sbReserved[430];           /* (reserved for future use)           */
};

typedef char APMBlock0SignatureType[2];
typedef struct APMDDMap sbDrvrMapArrayType[8];
typedef uint8_t APMBlock0ReservedType[430];

BOOST_FUSION_ADAPT_STRUCT(
			  APMBlock0,
			  (APMBlock0SignatureType,sbSig)
			  (uint16_t,sbBlkSize)
			  (uint32_t,sbBlkCount)
			  (uint16_t,sbDevType)
			  (uint16_t,sbDevId)
			  (uint32_t,sbDrvrData)
			  (uint16_t,sbDrvrCount)
			  (sbDrvrMapArrayType,sbDrvrMap)
			  (APMBlock0ReservedType,sbReserved))
BOOST_FUSION_ADD_SERIALIZER( APMBlock0 )

struct APMPartitionEntryStruct {
  char signature[2];
  uint16_t reserved;
  uint32_t numberOfPartitions;
  uint32_t startingSectorOfPartition;
  uint32_t sizeOfPartiion;
  char nameOfPartition[32];
  char typeOfPartition[32];
  uint32_t statringSectorOfDataArea;
  uint32_t sectorsOfDataArea;
  uint32_t statusOfPartition;
  uint32_t startingSectorOfBootCode;
  uint32_t sizeOfBootCode;
  uint32_t addressOfBootloaderCode;
  uint32_t reserved1;
  uint32_t bootCodeEntryPoint;
  uint32_t reserved2;
  uint32_t bootCodeCheckSum;
  char processorType[16];
  uint8_t reserved3[376];
};

typedef char tSignature[2];
typedef char tPartitionMapStringType[32];
typedef char tProcessorType[16];
typedef uint8_t APMEntryReservedType[376];

BOOST_FUSION_ADAPT_STRUCT(
			  APMPartitionEntryStruct,
			  (tSignature,signature)
			  (uint16_t,reserved)
			  (uint32_t,numberOfPartitions)
			  (uint32_t,startingSectorOfPartition)
			  (uint32_t,sizeOfPartiion)
			  (tPartitionMapStringType,nameOfPartition)
			  (tPartitionMapStringType,typeOfPartition)
			  (uint32_t,statringSectorOfDataArea)
			  (uint32_t,sectorsOfDataArea)
			  (uint32_t,statusOfPartition)
			  (uint32_t,startingSectorOfBootCode)
			  (uint32_t,sizeOfBootCode)
			  (uint32_t,addressOfBootloaderCode)
			  (uint32_t,reserved1)
			  (uint32_t,bootCodeEntryPoint)
			  (uint32_t,reserved2)
			  (uint32_t,bootCodeCheckSum)
			  (tProcessorType,processorType)
			  (APMEntryReservedType,reserved3))
BOOST_FUSION_ADD_SERIALIZER( APMPartitionEntryStruct )
