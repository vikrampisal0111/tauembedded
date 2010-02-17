//
//  $Id: disk.c 42 2008-10-04 18:40:36Z jcw $
//  $Revision: 42 $
//  $Author: jcw $
//  $Date: 2008-10-04 14:40:36 -0400 (Sat, 04 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/fatfs/disk.c $
//

#include <stdio.h> 
#include <stdlib.h>
#include "disk.h"
#include "mmc.h"
#include "spi1.h"
#include "debug.h"

#define S_MAX_SIZ 512
static volatile DSTATUS gDiskStatus = DSTATUS_NOINIT; 
static mediaStatus_t mediaStatus;

extern BYTE MMCWRData[MMC_DATA_SIZE];
extern BYTE MMCRDData[MMC_DATA_SIZE];

//
//
//
DSTATUS diskInitialize (BYTE drv __attribute__ ((unused)))
{
  //
  //  Media Init
  //
  SPI_Init();
  switch (mmc_init ())
  {
    case 0 :
      {
        mediaStatus.statusCode = DSC_COMMANDPASS;
        mediaStatus.mediaChanged = 1;
        gDiskStatus = 0;
        pmesg(MSG_DEBUG,"\nMMC INIT OK\n");
      }
      break;



    default:
      {
        mediaStatus.statusCode = DSC_NOTPRESENT;
        gDiskStatus = DSTATUS_NODISK;
      }
      break;
  }

  return gDiskStatus;
}

//
//
//
DSTATUS diskShutdown (void)
{
  gDiskStatus |= DSTATUS_NOINIT;

  return gDiskStatus;
}

//
//
//
DSTATUS diskStatus (BYTE drv __attribute__ ((unused)))
{
  pmesg(MSG_DEBUG_MORE,"&&&diskstatus\n");
  return gDiskStatus;
}

//
//
//
DRESULT diskRead (BYTE disk __attribute__ ((unused)), BYTE *buff, DWORD sector, BYTE count)
{
  DWORD res = 0;
  int i;

  if (gDiskStatus & DSTATUS_NOINIT) 
    return DRESULT_NOTRDY;
  if (!count) 
    return DRESULT_PARERR;
  pmesg(MSG_DEBUG_MORE,"diskRead ( %d , %d )\n", sector, count);
  for (i = 0; i < count; i++)
  {
		res = mmc_read_block(i + sector);
int j;
for (j = 0 ; j < 512; j++)
{
	pmesg(MSG_DEBUG_MORE,"%x ", MMCRDData[j]);
	if (((j+1) % 32) == 0) pmesg(MSG_DEBUG_MORE,"\n");
}

		if (res == 0)
			memcpy(buff + i*512, MMCRDData, 512);
		else
			break;
  }
  
pmesg(MSG_DEBUG_MORE,"&&&diskread result=%d\n",res);
  if (res == 0)
    return DRESULT_OK;
  else
    return DRESULT_ERROR; 
}

//
//
//
#if _FS_READONLY == 0
DRESULT diskWrite (BYTE disk __attribute__ ((unused)), const BYTE *buff, DWORD sector, BYTE count)
{
  DWORD res = 0;
  int i;

int j;
for (j = 0; j < 512; j++)
{
	pmesg(MSG_DEBUG_MORE,"%x ", MMCWRData[j]);
	if (((j+1) % 32) == 0) pmesg(MSG_DEBUG_MORE,"\n");
}


  if (gDiskStatus & DSTATUS_NOINIT) 
    return DRESULT_NOTRDY;
  if (gDiskStatus & DSTATUS_PROTECT) 
    return DRESULT_WRPRT;
  if (!count) 
    return DRESULT_PARERR;

//printf("diskWrite ( %d , %d )\n", sector, count);
  for (i = 0; i < count; i++)
  {
		if (res == 0)
			memcpy(MMCWRData, buff + i*512, 512);
		else
			break;

		res = mmc_write_block(i+sector);
  }

pmesg(MSG_DEBUG_MORE,"&&&diskwrite result=%d\n",res);
  if (res == 0)
    return DRESULT_OK;
  else
    return DRESULT_ERROR; 
}
#endif

//
//
//
DRESULT diskIoctl (BYTE drv, BYTE ctrl, void *buff)
{
  DRESULT res;
  BYTE n;
  BYTE csd [16];
  short csize;

  if (drv) 
    return DRESULT_PARERR;
  if (gDiskStatus & DSTATUS_NOINIT) 
    return DRESULT_NOTRDY;

  res = DRESULT_ERROR;

  pmesg(MSG_DEBUG_MORE,"ioctl - ctrl = %d\n", ctrl);

  switch (ctrl) 
  {
    case IOCTL_GET_SECTOR_COUNT :
      { 
         pmesg(MSG_DEBUG_MORE,"\n IOCTL QRY CAP %d \n", mmc_card_capacity() / 512);
		return mmc_card_capacity() / 512;
      }
      break;

    case IOCTL_GET_SECTOR_SIZE :
      {		
		res = DRESULT_OK;
		(*(int*)buff) = 512;
      }
      break;

    case IOCTL_CTRL_SYNC :
      {
	    res = DRESULT_OK;
      }
      break;

    case IOCTL_MMC_GET_CSD :
      {

      }
      break;

    case IOCTL_MMC_GET_CID :
      {
      }
      break;

# if 0
    case IOCTL_CTRL_POWER:
    case IOCTL_CTRL_LOCK:
    case IOCTL_CTRL_EJECT:
    case IOCTL_MMC_GET_OCR:     /* Receive OCR as an R3 resp (4 bytes) */
    case IOCTL_ATA_GET_REV:
    case IOCTL_ATA_GET_MODEL:
    case IOCTL_ATA_GET_SN:
#endif                        

    default:
      res = DRESULT_PARERR;
  }

  pmesg(MSG_DEBUG_MORE,"&&&diskioctl result=%d\n",res);

  return 0;
}

//
//
//
BYTE diskPresent (void)
{
  return 1;
}

//
//
//
const char *diskErrorText (DRESULT d)
{
  unsigned int i;

  typedef struct errorStrings_s
  {
    DRESULT dresult;
    const char *string;
  }
  errorStrings_t;

  static const errorStrings_t errorStrings [] =
  {
    { DRESULT_OK,       "OK"           },
    { DRESULT_ERROR,    "R/W ERROR"    },
    { DRESULT_WRPRT,    "WP ERROR"     },
    { DRESULT_NOTRDY,   "NOT READY"    },
    { DRESULT_PARERR,   "INVALID PARM" },
  };

  for (i = 0; i < sizeof (errorStrings)/sizeof(errorStrings_t); i++)
    if (errorStrings [i].dresult == d)
      return errorStrings [d].string;

  return "(no err text)";
}

void diskErrorTextPrint (DRESULT d)
{
//  printf ("rrc=%u %s\n", d, diskErrorText (d));
}
