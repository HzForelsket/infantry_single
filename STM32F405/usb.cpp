#include "USB.h"

#define APP_RX_DATA_SIZE  2048
#define APP_TX_DATA_SIZE  2048

uint8_t UserRxBufferHS[APP_RX_DATA_SIZE];
uint8_t UserTxBufferHS[APP_TX_DATA_SIZE];

static int8_t CDC_Init_HS(void);
static int8_t CDC_DeInit_HS(void);
static int8_t CDC_Control_HS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_HS(uint8_t* pbuf, uint32_t *Len);

USBD_CDC_ItfTypeDef USBD_Interface_fops_HS =
{
  CDC_Init_HS,
  CDC_DeInit_HS,
  CDC_Control_HS,
  CDC_Receive_HS
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes the CDC media low layer over the USB HS IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Init_HS(void)
{
	/* USER CODE BEGIN 8 */
	/* Set Application Buffers */
	USBD_CDC_SetTxBuffer(&usb.hUsbDeviceHS, UserTxBufferHS, 0);
	USBD_CDC_SetRxBuffer(&usb.hUsbDeviceHS, UserRxBufferHS);
	return (USBD_OK);
	/* USER CODE END 8 */
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @param  None
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_DeInit_HS(void)
{
	return (USBD_OK);
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Control_HS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
	uint8_t tempbuf[7] = { 0,0,0,0,0,0,0 };
	switch (cmd)
	{
	case CDC_SEND_ENCAPSULATED_COMMAND:

		break;

	case CDC_GET_ENCAPSULATED_RESPONSE:

		break;

	case CDC_SET_COMM_FEATURE:

		break;

	case CDC_GET_COMM_FEATURE:

		break;

	case CDC_CLEAR_COMM_FEATURE:

		break;

		/*******************************************************************************/
		/* Line Coding Structure                                                       */
		/*-----------------------------------------------------------------------------*/
		/* Offset | Field       | Size | Value  | Description                          */
		/* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
		/* 4      | bCharFormat |   1  | Number | Stop bits                            */
		/*                                        0 - 1 Stop bit                       */
		/*                                        1 - 1.5 Stop bits                    */
		/*                                        2 - 2 Stop bits                      */
		/* 5      | bParityType |  1   | Number | Parity                               */
		/*                                        0 - None                             */
		/*                                        1 - Odd                              */
		/*                                        2 - Even                             */
		/*                                        3 - Mark                             */
		/*                                        4 - Space                            */
		/* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
		/*******************************************************************************/
	case CDC_SET_LINE_CODING:
		//tempbuf[0] = pbuf[0];
		//tempbuf[1] = pbuf[1];
		//tempbuf[2] = pbuf[2];
		//tempbuf[3] = pbuf[3];
		//tempbuf[4] = pbuf[4];
		//tempbuf[5] = pbuf[5];
		//tempbuf[6] = pbuf[6];
		break;

	case CDC_GET_LINE_CODING:
		//pbuf[0] = tempbuf[0];
		//pbuf[1] = tempbuf[1];
		//pbuf[2] = tempbuf[2];
		//pbuf[3] = tempbuf[3];
		//pbuf[4] = tempbuf[4];
		//pbuf[5] = tempbuf[5];
		//pbuf[6] = tempbuf[6];
		break;

	case CDC_SET_CONTROL_LINE_STATE:

		break;

	case CDC_SEND_BREAK:

		break;

	default:
		break;
	}

	return (USBD_OK);
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will block any OUT packet reception on USB endpoint
  *         untill exiting this function. If you exit this function before transfer
  *         is complete on CDC interface (ie. using DMA controller) it will result
  *         in receiving more data while previous ones are still not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Receive_HS(uint8_t* Buf, uint32_t *Len)
{
	/* USER CODE BEGIN 11 */
	memcpy(usb.rxBuffer, Buf, *Len);
	if(usb.OnRecive)
		usb.OnRecive();

	USBD_CDC_SetRxBuffer(&usb.hUsbDeviceHS, &Buf[0]);
	USBD_CDC_ReceivePacket(&usb.hUsbDeviceHS);
	return (USBD_OK);
	/* USER CODE END 11 */
}

/**
  * @brief  Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t CDC_Transmit_HS(uint8_t* Buf, uint16_t Len)
{
	uint8_t result = USBD_OK;
	USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)usb.hUsbDeviceHS.pClassData;
	if (hcdc->TxState != 0) {
		return USBD_BUSY;
	}
	USBD_CDC_SetTxBuffer(&usb.hUsbDeviceHS, Buf, Len);
	result = USBD_CDC_TransmitPacket(&usb.hUsbDeviceHS);
	return result;
}

PCD_HandleTypeDef hpcd_USB_OTG_HS;
USBD_StatusTypeDef USBD_Get_USB_Status(HAL_StatusTypeDef hal_status);


/*******************************************************************************
					   LL Driver Callbacks (PCD -> USB Device Library)
*******************************************************************************/
/* MSP Init */
void HAL_PCD_MspInit(PCD_HandleTypeDef* pcdHandle)
{
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	if (pcdHandle->Instance == USB_OTG_HS)
	{
		__HAL_RCC_GPIOH_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();
		/**USB_OTG_HS GPIO Configuration
		PB14     ------> USB_OTG_HS_DM
		PB15     ------> USB_OTG_HS_DP
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_14 | GPIO_PIN_15;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF12_OTG_HS_FS;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		/* Peripheral clock enable */
		__HAL_RCC_USB_OTG_HS_CLK_ENABLE();

		/* Peripheral interrupt init */
		HAL_NVIC_SetPriority(OTG_HS_EP1_OUT_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(OTG_HS_EP1_OUT_IRQn);
		HAL_NVIC_SetPriority(OTG_HS_EP1_IN_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(OTG_HS_EP1_IN_IRQn);
		HAL_NVIC_SetPriority(OTG_HS_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(OTG_HS_IRQn);
		/* USER CODE BEGIN USB_OTG_HS_MspInit 1 */

		/* USER CODE END USB_OTG_HS_MspInit 1 */
	}
}

void HAL_PCD_MspDeInit(PCD_HandleTypeDef* pcdHandle)
{
	if (pcdHandle->Instance == USB_OTG_HS)
	{
		/* USER CODE BEGIN USB_OTG_HS_MspDeInit 0 */

		/* USER CODE END USB_OTG_HS_MspDeInit 0 */
		  /* Peripheral clock disable */
		__HAL_RCC_USB_OTG_HS_CLK_DISABLE();

		/**USB_OTG_HS GPIO Configuration
		PB14     ------> USB_OTG_HS_DM
		PB15     ------> USB_OTG_HS_DP
		*/
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_14 | GPIO_PIN_15);

		/* Peripheral interrupt Deinit*/
		HAL_NVIC_DisableIRQ(OTG_HS_EP1_OUT_IRQn);

		HAL_NVIC_DisableIRQ(OTG_HS_EP1_IN_IRQn);

		HAL_NVIC_DisableIRQ(OTG_HS_IRQn);

		/* USER CODE BEGIN USB_OTG_HS_MspDeInit 1 */

		/* USER CODE END USB_OTG_HS_MspDeInit 1 */
	}
}

/**
  * @brief  Setup stage callback
  * @param  hpcd: PCD handle
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd)
#else
void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
	USBD_LL_SetupStage((USBD_HandleTypeDef*)hpcd->pData, (uint8_t *)hpcd->Setup);
}

/**
  * @brief  Data Out stage callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint number
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#else
void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
	USBD_LL_DataOutStage((USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->OUT_ep[epnum].xfer_buff);
}

/**
  * @brief  Data In stage callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint number
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#else
void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
	USBD_LL_DataInStage((USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->IN_ep[epnum].xfer_buff);
}

/**
  * @brief  SOF callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_SOFCallback(PCD_HandleTypeDef *hpcd)
#else
void HAL_PCD_SOFCallback(PCD_HandleTypeDef *hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
	USBD_LL_SOF((USBD_HandleTypeDef*)hpcd->pData);
}

/**
  * @brief  Reset callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_ResetCallback(PCD_HandleTypeDef *hpcd)
#else
void HAL_PCD_ResetCallback(PCD_HandleTypeDef *hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
	USBD_SpeedTypeDef speed = USBD_SPEED_FULL;

	if (hpcd->Init.speed == PCD_SPEED_HIGH)
	{
		speed = USBD_SPEED_HIGH;
	}
	else if (hpcd->Init.speed == PCD_SPEED_FULL)
	{
		speed = USBD_SPEED_FULL;
	}
	else
	{
		//Error_Handler();
	}
	/* Set Speed. */
	USBD_LL_SetSpeed((USBD_HandleTypeDef*)hpcd->pData, speed);

	/* Reset Device. */
	USBD_LL_Reset((USBD_HandleTypeDef*)hpcd->pData);
}

/**
  * @brief  Suspend callback.
  * When Low power mode is enabled the debug cannot be used (IAR, Keil doesn't support it)
  * @param  hpcd: PCD handle
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_SuspendCallback(PCD_HandleTypeDef *hpcd)
#else
void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
	/* Inform USB library that core enters in suspend Mode. */
	USBD_LL_Suspend((USBD_HandleTypeDef*)hpcd->pData);
	__HAL_PCD_GATE_PHYCLOCK(hpcd);
	/* Enter in STOP mode. */
	/* USER CODE BEGIN 2 */
	if (hpcd->Init.low_power_enable)
	{
		/* Set SLEEPDEEP bit and SleepOnExit of Cortex System Control Register. */
		SCB->SCR |= (uint32_t)((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
	}
	/* USER CODE END 2 */
}

/**
  * @brief  Resume callback.
  * When Low power mode is enabled the debug cannot be used (IAR, Keil doesn't support it)
  * @param  hpcd: PCD handle
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_ResumeCallback(PCD_HandleTypeDef *hpcd)
#else
void HAL_PCD_ResumeCallback(PCD_HandleTypeDef *hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
	/* USER CODE BEGIN 3 */

	/* USER CODE END 3 */
	USBD_LL_Resume((USBD_HandleTypeDef*)hpcd->pData);
}

/**
  * @brief  ISOOUTIncomplete callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint number
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#else
void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
	USBD_LL_IsoOUTIncomplete((USBD_HandleTypeDef*)hpcd->pData, epnum);
}

/**
  * @brief  ISOINIncomplete callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint number
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#else
void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
	USBD_LL_IsoINIncomplete((USBD_HandleTypeDef*)hpcd->pData, epnum);
}

/**
  * @brief  Connect callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_ConnectCallback(PCD_HandleTypeDef *hpcd)
#else
void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
	USBD_LL_DevConnected((USBD_HandleTypeDef*)hpcd->pData);
}

/**
  * @brief  Disconnect callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd)
#else
void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
	USBD_LL_DevDisconnected((USBD_HandleTypeDef*)hpcd->pData);
}

/*******************************************************************************
					   LL Driver Interface (USB Device Library --> PCD)
*******************************************************************************/

/**
  * @brief  Initializes the low level portion of the device driver.
  * @param  pdev: Device handle
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_Init(USBD_HandleTypeDef *pdev)
{
	/* Init USB Ip. */
	if (pdev->id == DEVICE_HS) {
		/* Link the driver to the stack. */
		hpcd_USB_OTG_HS.pData = pdev;
		pdev->pData = &hpcd_USB_OTG_HS;

		hpcd_USB_OTG_HS.Instance = USB_OTG_HS;
		hpcd_USB_OTG_HS.Init.dev_endpoints = 6;
		hpcd_USB_OTG_HS.Init.speed = PCD_SPEED_FULL;
		hpcd_USB_OTG_HS.Init.dma_enable = ENABLE;
		hpcd_USB_OTG_HS.Init.phy_itface = USB_OTG_EMBEDDED_PHY;
		hpcd_USB_OTG_HS.Init.Sof_enable = DISABLE;
		hpcd_USB_OTG_HS.Init.low_power_enable = DISABLE;
		hpcd_USB_OTG_HS.Init.lpm_enable = DISABLE;
		hpcd_USB_OTG_HS.Init.vbus_sensing_enable = DISABLE;
		hpcd_USB_OTG_HS.Init.use_dedicated_ep1 = DISABLE;
		hpcd_USB_OTG_HS.Init.use_external_vbus = DISABLE;
		if (HAL_PCD_Init(&hpcd_USB_OTG_HS) != HAL_OK)
		{
			//Error_Handler();
		}

#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
		/* Register USB PCD CallBacks */
		HAL_PCD_RegisterCallback(&hpcd_USB_OTG_HS, HAL_PCD_SOF_CB_ID, PCD_SOFCallback);
		HAL_PCD_RegisterCallback(&hpcd_USB_OTG_HS, HAL_PCD_SETUPSTAGE_CB_ID, PCD_SetupStageCallback);
		HAL_PCD_RegisterCallback(&hpcd_USB_OTG_HS, HAL_PCD_RESET_CB_ID, PCD_ResetCallback);
		HAL_PCD_RegisterCallback(&hpcd_USB_OTG_HS, HAL_PCD_SUSPEND_CB_ID, PCD_SuspendCallback);
		HAL_PCD_RegisterCallback(&hpcd_USB_OTG_HS, HAL_PCD_RESUME_CB_ID, PCD_ResumeCallback);
		HAL_PCD_RegisterCallback(&hpcd_USB_OTG_HS, HAL_PCD_CONNECT_CB_ID, PCD_ConnectCallback);
		HAL_PCD_RegisterCallback(&hpcd_USB_OTG_HS, HAL_PCD_DISCONNECT_CB_ID, PCD_DisconnectCallback);

		HAL_PCD_RegisterDataOutStageCallback(&hpcd_USB_OTG_HS, PCD_DataOutStageCallback);
		HAL_PCD_RegisterDataInStageCallback(&hpcd_USB_OTG_HS, PCD_DataInStageCallback);
		HAL_PCD_RegisterIsoOutIncpltCallback(&hpcd_USB_OTG_HS, PCD_ISOOUTIncompleteCallback);
		HAL_PCD_RegisterIsoInIncpltCallback(&hpcd_USB_OTG_HS, PCD_ISOINIncompleteCallback);
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
		HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG_HS, 0x200);
		HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_HS, 0, 0x80);
		HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_HS, 1, 0x174);
	}
	return USBD_OK;
}

/**
  * @brief  De-Initializes the low level portion of the device driver.
  * @param  pdev: Device handle
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_DeInit(USBD_HandleTypeDef *pdev)
{
	HAL_StatusTypeDef hal_status = HAL_OK;
	USBD_StatusTypeDef usb_status = USBD_OK;

	hal_status = HAL_PCD_DeInit((PCD_HandleTypeDef*)pdev->pData);

	usb_status = USBD_Get_USB_Status(hal_status);

	return usb_status;
}

/**
  * @brief  Starts the low level portion of the device driver.
  * @param  pdev: Device handle
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_Start(USBD_HandleTypeDef *pdev)
{
	HAL_StatusTypeDef hal_status = HAL_OK;
	USBD_StatusTypeDef usb_status = USBD_OK;

	hal_status = HAL_PCD_Start((PCD_HandleTypeDef*)pdev->pData);

	usb_status = USBD_Get_USB_Status(hal_status);

	return usb_status;
}

/**
  * @brief  Stops the low level portion of the device driver.
  * @param  pdev: Device handle
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_Stop(USBD_HandleTypeDef *pdev)
{
	HAL_StatusTypeDef hal_status = HAL_OK;
	USBD_StatusTypeDef usb_status = USBD_OK;

	hal_status = HAL_PCD_Stop((PCD_HandleTypeDef*)pdev->pData);

	usb_status = USBD_Get_USB_Status(hal_status);

	return usb_status;
}

/**
  * @brief  Opens an endpoint of the low level driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @param  ep_type: Endpoint type
  * @param  ep_mps: Endpoint max packet size
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_OpenEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t ep_type, uint16_t ep_mps)
{
	HAL_StatusTypeDef hal_status = HAL_OK;
	USBD_StatusTypeDef usb_status = USBD_OK;

	hal_status = HAL_PCD_EP_Open((PCD_HandleTypeDef*)pdev->pData, ep_addr, ep_mps, ep_type);

	usb_status = USBD_Get_USB_Status(hal_status);

	return usb_status;
}

/**
  * @brief  Closes an endpoint of the low level driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_CloseEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
	HAL_StatusTypeDef hal_status = HAL_OK;
	USBD_StatusTypeDef usb_status = USBD_OK;

	hal_status = HAL_PCD_EP_Close((PCD_HandleTypeDef*)pdev->pData, ep_addr);

	usb_status = USBD_Get_USB_Status(hal_status);

	return usb_status;
}

/**
  * @brief  Flushes an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_FlushEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
	HAL_StatusTypeDef hal_status = HAL_OK;
	USBD_StatusTypeDef usb_status = USBD_OK;

	hal_status = HAL_PCD_EP_Flush((PCD_HandleTypeDef*)pdev->pData, ep_addr);

	usb_status = USBD_Get_USB_Status(hal_status);

	return usb_status;
}

/**
  * @brief  Sets a Stall condition on an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_StallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
	HAL_StatusTypeDef hal_status = HAL_OK;
	USBD_StatusTypeDef usb_status = USBD_OK;

	hal_status = HAL_PCD_EP_SetStall((PCD_HandleTypeDef*)pdev->pData, ep_addr);

	usb_status = USBD_Get_USB_Status(hal_status);

	return usb_status;
}

/**
  * @brief  Clears a Stall condition on an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_ClearStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
	HAL_StatusTypeDef hal_status = HAL_OK;
	USBD_StatusTypeDef usb_status = USBD_OK;

	hal_status = HAL_PCD_EP_ClrStall((PCD_HandleTypeDef*)pdev->pData, ep_addr);

	usb_status = USBD_Get_USB_Status(hal_status);

	return usb_status;
}

/**
  * @brief  Returns Stall condition.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval Stall (1: Yes, 0: No)
  */
uint8_t USBD_LL_IsStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
	PCD_HandleTypeDef *hpcd = (PCD_HandleTypeDef*)(PCD_HandleTypeDef*)pdev->pData;

	if ((ep_addr & 0x80) == 0x80)
	{
		return hpcd->IN_ep[ep_addr & 0x7F].is_stall;
	}
	else
	{
		return hpcd->OUT_ep[ep_addr & 0x7F].is_stall;
	}
}

/**
  * @brief  Assigns a USB address to the device.
  * @param  pdev: Device handle
  * @param  dev_addr: Device address
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_SetUSBAddress(USBD_HandleTypeDef *pdev, uint8_t dev_addr)
{
	HAL_StatusTypeDef hal_status = HAL_OK;
	USBD_StatusTypeDef usb_status = USBD_OK;

	hal_status = HAL_PCD_SetAddress((PCD_HandleTypeDef*)pdev->pData, dev_addr);

	usb_status = USBD_Get_USB_Status(hal_status);

	return usb_status;
}

/**
  * @brief  Transmits data over an endpoint.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @param  pbuf: Pointer to data to be sent
  * @param  size: Data size
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_Transmit(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint16_t size)
{
	HAL_StatusTypeDef hal_status = HAL_OK;
	USBD_StatusTypeDef usb_status = USBD_OK;

	hal_status = HAL_PCD_EP_Transmit((PCD_HandleTypeDef*)pdev->pData, ep_addr, pbuf, size);

	usb_status = USBD_Get_USB_Status(hal_status);

	return usb_status;
}

/**
  * @brief  Prepares an endpoint for reception.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @param  pbuf: Pointer to data to be received
  * @param  size: Data size
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_PrepareReceive(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint16_t size)
{
	HAL_StatusTypeDef hal_status = HAL_OK;
	USBD_StatusTypeDef usb_status = USBD_OK;

	hal_status = HAL_PCD_EP_Receive((PCD_HandleTypeDef*)pdev->pData, ep_addr, pbuf, size);

	usb_status = USBD_Get_USB_Status(hal_status);

	return usb_status;
}

/**
  * @brief  Returns the last transfered packet size.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval Recived Data Size
  */
uint32_t USBD_LL_GetRxDataSize(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
	return HAL_PCD_EP_GetRxCount((PCD_HandleTypeDef*)(PCD_HandleTypeDef*)pdev->pData, ep_addr);
}

/**
  * @brief  Delays routine for the USB Device Library.
  * @param  Delay: Delay in ms
  * @retval None
  */
void USBD_LL_Delay(uint32_t Delay)
{
	HAL_Delay(Delay);
}

/**
  * @brief  Retuns the USB status depending on the HAL status:
  * @param  hal_status: HAL status
  * @retval USB status
  */
USBD_StatusTypeDef USBD_Get_USB_Status(HAL_StatusTypeDef hal_status)
{
	USBD_StatusTypeDef usb_status = USBD_OK;

	switch (hal_status)
	{
	case HAL_OK:
		usb_status = USBD_OK;
		break;
	case HAL_ERROR:
		usb_status = USBD_FAIL;
		break;
	case HAL_BUSY:
		usb_status = USBD_BUSY;
		break;
	case HAL_TIMEOUT:
		usb_status = USBD_FAIL;
		break;
	default:
		usb_status = USBD_FAIL;
		break;
	}
	return usb_status;
}

/**
  * @brief This function handles USB On The Go HS End Point 1 Out global interrupt.
  */
extern "C" void OTG_HS_EP1_OUT_IRQHandler(void)
{
	/* USER CODE BEGIN OTG_HS_EP1_OUT_IRQn 0 */

	/* USER CODE END OTG_HS_EP1_OUT_IRQn 0 */
	HAL_PCD_IRQHandler(&hpcd_USB_OTG_HS);
	/* USER CODE BEGIN OTG_HS_EP1_OUT_IRQn 1 */

	/* USER CODE END OTG_HS_EP1_OUT_IRQn 1 */
}

/**
  * @brief This function handles USB On The Go HS End Point 1 In global interrupt.
  */
extern "C" void OTG_HS_EP1_IN_IRQHandler(void)
{
	/* USER CODE BEGIN OTG_HS_EP1_IN_IRQn 0 */

	/* USER CODE END OTG_HS_EP1_IN_IRQn 0 */
	HAL_PCD_IRQHandler(&hpcd_USB_OTG_HS);
	/* USER CODE BEGIN OTG_HS_EP1_IN_IRQn 1 */

	/* USER CODE END OTG_HS_EP1_IN_IRQn 1 */
}

/**
  * @brief This function handles USB On The Go HS global interrupt.
  */
extern "C" void OTG_HS_IRQHandler(void)
{
	HAL_PCD_IRQHandler(&hpcd_USB_OTG_HS);
}