﻿#include "can.h"
#include "PID.h"

//CAN::CAN(Motor* motor,uint16_t number)
//{
//	m_motor = motor;
//	m_number = number;
//}

//CAN::CAN()
//{
//
//}
void CAN::Init(CAN_TypeDef* instance)
{
	hcan.Instance       = instance;
	hcan.Init.Prescaler = 6;
	hcan.Init.Mode      = CAN_MODE_NORMAL;
	hcan.Init.SJW       = CAN_SJW_1TQ;
	hcan.Init.BS1       = CAN_BS1_2TQ;
	hcan.Init.BS2       = CAN_BS2_4TQ;
	hcan.Init.TTCM      = DISABLE;
	hcan.Init.ABOM      = ENABLE;
	hcan.Init.AWUM      = ENABLE;
	hcan.Init.NART      = DISABLE;
	hcan.Init.RFLM      = DISABLE;
	hcan.Init.TXFP      = DISABLE;
	HAL_CAN_Init(&hcan);
	HAL_CAN_Transmit_IT(&hcan);
	HAL_CAN_Receive_IT(&hcan, CAN_FIFO0);
	InitFilter();
}
void CAN::InitFilter()
{
	//can1 &can2 use same filter config
	CAN_FilterConfTypeDef		CAN_FilterConfigStructure;

	CAN_FilterConfigStructure.FilterNumber         = 0;
	CAN_FilterConfigStructure.FilterMode           = CAN_FILTERMODE_IDMASK;
	CAN_FilterConfigStructure.FilterScale          = CAN_FILTERSCALE_32BIT;
	CAN_FilterConfigStructure.FilterIdHigh         = 0x0000;
	CAN_FilterConfigStructure.FilterIdLow          = 0x0000;
	CAN_FilterConfigStructure.FilterMaskIdHigh     = 0x0000;
	CAN_FilterConfigStructure.FilterMaskIdLow      = 0x0000;
	CAN_FilterConfigStructure.FilterFIFOAssignment = CAN_FilterFIFO0;
	//can1(0-13)和can2(14-27)分别得到一半的filter
	CAN_FilterConfigStructure.BankNumber = 0;
	CAN_FilterConfigStructure.FilterActivation = ENABLE;

	HAL_CAN_ConfigFilter(&hcan, &CAN_FilterConfigStructure);
	//filter config for can2 
	//can1(0-13)和can2(14-27)分别得到一半的filter
	CAN_FilterConfigStructure.FilterNumber = 14;
	HAL_CAN_ConfigFilter(&hcan, &CAN_FilterConfigStructure);

	hcan.pTxMsg = &TxMessage;
	hcan.pRxMsg = &RxMessage;
}

void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	if (hcan->Instance == CAN1)
	{
		/* Peripheral clock enable */
		__HAL_RCC_CAN1_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**CAN1 GPIO Configuration
		PD0     ------> CAN1_RX
		PD1     ------> CAN1_TX*/
		GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		/* Peripheral interrupt init */
		HAL_NVIC_SetPriority(CAN1_TX_IRQn, 1, 0);
		HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
		HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
	}
	else if (hcan->Instance == CAN2)
	{
		/* Peripheral clock enable */
		__HAL_RCC_CAN2_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();
		/**CAN1 GPIO Configuration
		PB12     ------> CAN2_RX
		PB13     ------> CAN2_TX*/
		GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF9_CAN2;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
		/* Peripheral interrupt init */
		HAL_NVIC_SetPriority(CAN2_TX_IRQn, 1, 0);
		HAL_NVIC_EnableIRQ(CAN2_TX_IRQn);
		HAL_NVIC_SetPriority(CAN2_RX0_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(CAN2_RX0_IRQn);
	}
}

extern "C" void CAN1_TX_IRQHandler()
{
	HAL_CAN_IRQHandler(&can1.hcan);
}
extern "C" void CAN1_RX0_IRQHandler()
{
	HAL_CAN_IRQHandler(&can1.hcan);
}
extern "C" void CAN2_TX_IRQHandler()
{
	HAL_CAN_IRQHandler(&can2.hcan);
}
extern "C" void CAN2_RX0_IRQHandler()
{
	HAL_CAN_IRQHandler(&can2.hcan);
}

HAL_StatusTypeDef CAN::Transmit(const uint32_t ID, const uint8_t*const pData, const uint8_t len)
{
	hcan.pTxMsg->StdId = ID;
	hcan.pTxMsg->IDE = CAN_ID_STD;
	hcan.pTxMsg->RTR = CAN_RTR_DATA;
	hcan.pTxMsg->DLC = len;
	memcpy(hcan.pTxMsg->Data, pData, len);
	HAL_StatusTypeDef status = HAL_CAN_Transmit(&hcan, 10);

	ready = status!= HAL_ERROR;// == HAL_OK;
	return status;
}

void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* hcan)
{
	if (hcan == &can1.hcan)memcpy(can1.data[hcan->pRxMsg->StdId - 0x201], hcan->pRxMsg->Data, sizeof(uint8_t) * 8);
	else memcpy(can2.data[hcan->pRxMsg->StdId - 0x201], hcan->pRxMsg->Data, sizeof(uint8_t) * 8);

	/*#### add enable can it again to solve can receive only one ID problem!!!####**/
	__HAL_CAN_ENABLE_IT(hcan, CAN_IT_FMP0);
}
