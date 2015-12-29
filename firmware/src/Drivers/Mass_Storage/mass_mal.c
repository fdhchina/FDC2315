/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : mass_mal.c
* Author             : MCD Application Team
* Version            : V3.1.1
* Date               : 04/07/2010
* Description        : Medium Access Layer interface
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "hal.h"
#include "msd.h"
#include "mass_mal.h"


/* Private typedef -----------------------------------------------------------*/
#define SDCARD_USE_SPIMODE 1 //=1，SD卡的操作使用SPI模式；=0，使用SD模式

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t Mass_Memory_Size[2];
uint32_t Mass_Block_Size[2];
uint32_t Mass_Block_Count[2];
__IO uint32_t Status = 0;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : MAL_Init
* Description    : Initializes the Media on the STM32
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t MAL_Init(uint8_t lun)
{
  uint16_t status = MAL_OK;

  switch (lun)
  {
    case 0:
#ifdef SDCARD_USE_SPIMODE //SD卡使用SPI模式
      Status = SD_Init();
#else
	  SD_Init();
      Status = SD_GetCardInfo(&SDCardInfo);
      Status = SD_SelectDeselect((uint32_t) (SDCardInfo.RCA << 16));
      Status = SD_EnableWideBusOperation(SDIO_BusWide_4b);
      Status = SD_SetDeviceMode(SD_DMA_MODE);*/
#endif
      break;
    default:
      return MAL_FAIL;
  }
  return status;
}
/*******************************************************************************
* Function Name  : MAL_Write
* Description    : Write sectors
* Input          : - lun: 逻辑单元号.
*                  - Memory_Offset: 逻辑字节号.
*				   - *Writebuff: 数据缓冲区（由于NAND_Write()，宽度应该至少 NAND_PAGE_SIZE 字节）
*				   - Transfer_Length: 字节个数.
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t MAL_Write(uint8_t lun, uint32_t Memory_Offset, uint32_t *Writebuff, uint16_t Transfer_Length)
{
  switch (lun)
  {
    case 0:
#ifdef SDCARD_USE_SPIMODE //SD卡使用SPI模式
	  Status = msd_disk_write((uint8_t *)Writebuff, Memory_Offset, Transfer_Length>>9);
      if ( Status != MSD_RESPONSE_NO_ERROR )
#else
      Status = SD_WriteBlock(Memory_Offset, Writebuff, Transfer_Length);
      if ( Status != SD_OK )
#endif
      {
        return MAL_FAIL;
      }
      break;
    default:
      return MAL_FAIL;
  }
  return MAL_OK;
}

/*******************************************************************************
* Function Name  : MAL_Read
* Description    : Read sectors
* Input          : - lun: 逻辑单元号.
*                  - Memory_Offset: 逻辑字节号.
*				   - *Readbuff: 数据缓冲区（由于NAND_Read()，宽度应该至少 NAND_PAGE_SIZE 字节）
*				   - Transfer_Length: 字节个数.
* Output         : None
* Return         : Buffer pointer
*******************************************************************************/
uint16_t MAL_Read(uint8_t lun, uint32_t Memory_Offset, uint32_t *Readbuff, uint16_t Transfer_Length)
{

  switch (lun)
  {
    case 0:
#ifdef SDCARD_USE_SPIMODE //SD卡使用SPI模式
	  Status = msd_disk_read ((uint8_t *)Readbuff, Memory_Offset, Transfer_Length>>9);
      if ( Status != MSD_RESPONSE_NO_ERROR )
#else
      Status = SD_ReadBlock(Memory_Offset, Readbuff, Transfer_Length);
      if ( Status != SD_OK )
#endif
      {
        return MAL_FAIL;
      }
      break;
    default:
      return MAL_FAIL;
  }
  return MAL_OK;
}

/*******************************************************************************
* Function Name  : MAL_GetStatus
* Description    : Get status
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t MAL_GetStatus (uint8_t lun)
{
  //uint32_t DeviceSizeMul = 0;
#ifndef SDCARD_USE_SPIMODE //SD卡使用SPI模式
  uint32_t NumberOfBlocks;// = 0;
#endif

#ifdef SDCARD_USE_SPIMODE //SD卡使用SPI模式
  // uint32_t temp_block_mul = 0;
  // sMSD_CSD MSD_csd;
#endif

  if (lun == 0)
  {
#ifdef SDCARD_USE_SPIMODE //SD卡使用SPI模式
	// MSD_GetCSDRegister(&MSD_csd);
    //DeviceSizeMul = MSD_csd.DeviceSizeMul + 2;
    // temp_block_mul = (1 << MSD_csd.RdBlockLen)/ 512;
    Mass_Block_Count[0] = SD_GetCapacity() << 1;// ((MSD_csd.DeviceSize + 1) * (1 << (DeviceSizeMul))) * temp_block_mul;
    Mass_Block_Size[0] = 512;
#else
    if (SD_Init() == SD_OK)
    {
      SD_GetCardInfo(&SDCardInfo);
      SD_SelectDeselect((uint32_t) (SDCardInfo.RCA << 16));
      DeviceSizeMul = (SDCardInfo.SD_csd.DeviceSizeMul + 2);

      if(SDCardInfo.CardType == SDIO_HIGH_CAPACITY_SD_CARD)
      {
        Mass_Block_Count[0] = (SDCardInfo.SD_csd.DeviceSize + 1) * 1024;
      }
      else
      {
        NumberOfBlocks  = ((1 << (SDCardInfo.SD_csd.RdBlockLen)) / 512);
        Mass_Block_Count[0] = ((SDCardInfo.SD_csd.DeviceSize + 1) * (1 << DeviceSizeMul) << (NumberOfBlocks/2));
      }
      Mass_Block_Size[0]  = 512;

      Status = SD_SelectDeselect((uint32_t) (SDCardInfo.RCA << 16)); 
      Status = SD_EnableWideBusOperation(SDIO_BusWide_4b); 
      if ( Status != SD_OK )
      {
        return MAL_FAIL;
      }
       
      Status = SD_SetDeviceMode(SD_DMA_MODE);         
      if ( Status != SD_OK )
      {
        return MAL_FAIL;
      } 
#endif

      Mass_Memory_Size[0] = Mass_Block_Count[0] * Mass_Block_Size[0];
      // Led_On(LED2);//jie STM_EVAL_LEDOn(LED2);
      return MAL_OK;

#ifndef SDCARD_USE_SPIMODE //SD卡使用SPI模式
    }
#endif
  }
  // Led_On(LED2);//jie STM_EVAL_LEDOn(LED2);
  return MAL_FAIL;
}

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
