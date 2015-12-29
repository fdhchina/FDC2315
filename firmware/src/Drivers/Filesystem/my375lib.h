#ifndef MY375LIB_H
#define MY375LIB_H

#define MY_FILE_DATA_BUF_LEN	2048	/*对应为FILE_DATA_BUF_LEN的大小*/
#define MY_DISK_BASE_BUF_LEN	2048	/*对应为DISK_BASE_BUF_LEN的大小*/

extern bool usb_insert_flag;
extern bool usb_disk_Isready;
extern BOOL ch375_Init(void); // 芯片正常返回TRUE
extern u8 ch375_GetUSBStatus(void);
extern BOOL ch375_DiskIsReady(void);
extern FRESULT ch375_f_open(FIL * fp, const TCHAR * path, BYTE mode);
extern FRESULT ch375_f_close(FIL * fp);
extern FRESULT ch375_f_read(FIL * fp, void * buff, UINT btr, UINT * br);
extern FRESULT ch375_f_write(FIL * fp, const void * buff, UINT btw, UINT * bw);
extern FRESULT ch375_f_lseek(FIL * fp, DWORD ofs);
extern BOOL ch375_Reset( void );	
#endif
