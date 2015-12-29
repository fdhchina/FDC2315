/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2012        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control module to the FatFs module with a defined API.        */
/*-----------------------------------------------------------------------*/
#include "hal.h"
#include "diskio.h"		/* FatFs lower layer API */
// #include "CH375\usbdisk.h"	/* Example: USB drive control */
#include "MSD\msd.h"		/* Example: MMC/SDC contorl */

DSTATUS usb_disk_initialize (void);
DSTATUS usb_disk_status (void);
DRESULT usb_disk_read (BYTE*, DWORD, BYTE);
#if	_READONLY == 0
DRESULT usb_disk_write (const BYTE*, DWORD, BYTE);
#endif
DRESULT usb_disk_ioctl (BYTE, void*);

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE drv				/* Physical drive nmuber (0..) */
)
{
	switch (drv) {
	case MMC :
		return msd_disk_initialize();
#if (_VOLUMES>1)
	case USB :
		return usb_disk_initialize();
#endif
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0..) */
)
{
	switch (drv) {
	case MMC :
		return msd_disk_status();
#if (_VOLUMES>1)
	case USB :
		return usb_disk_status();
#endif
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..128) */
)
{
	switch (drv) {
	case MMC :
		return msd_disk_read(buff, sector, count);
#if (_VOLUMES>1)
	case USB :
		return usb_disk_read(buff, sector, count);
#endif
	}
	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..128) */
)
{
	switch (drv) {
	case MMC :
		return msd_disk_write(buff, sector, count);
#if (_VOLUMES>1)
	case USB :
		return usb_disk_write(buff, sector, count);
#endif	
	}
	return RES_PARERR;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	switch (drv) {
	case MMC :
		return msd_disk_ioctl(ctrl, buff);
#if (_VOLUMES>1)
	case USB :
		return usb_disk_ioctl(ctrl, buff);
#endif	
	}
	return RES_PARERR;
}
#endif
