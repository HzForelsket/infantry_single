#pragma once
#include <stm32f4xx_hal.h>
#include <stm32f405xx.h>
#include "usbd_core.h"
#include "usbd_cdc.h"
#include "memory"
#include<stdlib.h>
#include "string.h"
/*---------- -----------*/
#define USBD_MAX_NUM_INTERFACES      1U
/*---------- -----------*/
#define USBD_MAX_NUM_CONFIGURATION   1U
/*---------- -----------*/
#define USBD_MAX_STR_DESC_SIZ        512U
/*---------- -----------*/
#define USBD_SUPPORT_USER_STRING     0U
/*---------- -----------*/
#define USBD_DEBUG_LEVEL             0U
/*---------- -----------*/
#define USBD_LPM_ENABLED             0U
/*---------- -----------*/
#define USBD_SELF_POWERED            1U

/****************************************/
/* #define for FS and HS identification */
#define DEVICE_FS 		0
#define DEVICE_HS 		1

#define         DEVICE_ID1          (UID_BASE)
#define         DEVICE_ID2          (UID_BASE + 0x4)
#define         DEVICE_ID3          (UID_BASE + 0x8)

#define  USB_SIZ_STRING_SERIAL       0x1A
/* Memory management macros */
/** Alias for memory allocation. */
#define USBD_malloc      malloc
/** Alias for memory release. */
#define USBD_free        free
/** Alias for memory set. */
#define USBD_memset         memset
/** Alias for memory copy. */
#define USBD_memcpy         memcpy
/** Alias for delay. */
#define USBD_Delay          HAL_Delay

extern USBD_DescriptorsTypeDef HS_Desc;
extern USBD_CDC_ItfTypeDef USBD_Interface_fops_HS;
uint8_t CDC_Transmit_HS(uint8_t* Buf, uint16_t Len);

class USB
{
public:
	USBD_HandleTypeDef hUsbDeviceHS;
	uint8_t *rxBuffer = nullptr;
	void(*OnRecive)(void) = nullptr;

	USB* Init()
	{
		/* Init Device Library, add supported class and start the library. */
		if (USBD_Init(&hUsbDeviceHS, &HS_Desc, DEVICE_HS) != USBD_OK)
		{
			Error();
		}
		if (USBD_RegisterClass(&hUsbDeviceHS, &USBD_CDC) != USBD_OK)
		{
			Error();
		}
		if (USBD_CDC_RegisterInterface(&hUsbDeviceHS, &USBD_Interface_fops_HS) != USBD_OK)
		{
			Error();
		}
		if (USBD_Start(&hUsbDeviceHS) != USBD_OK)
		{
			Error();
		}
		return this;
	}
	void Error()
	{
		while (1);
	}
	void Transmit(uint8_t* pData, uint16_t Size)
	{
		CDC_Transmit_HS(pData, Size);
	}
	void RxInit(uint8_t *buffer, void(*OnRecive)(void))
	{
		this->rxBuffer = buffer;
		this->OnRecive = OnRecive;
	}
};

extern USB usb;

