#pragma once
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_can.h"
#include <cstdlib>
#include <cstring>

class CAN
{
public:	
	//CAN();
	//CAN(Motor* motor, uint16_t number);
	void Init(CAN_TypeDef* instance);
	void InitFilter();
	HAL_StatusTypeDef Transmit(const uint32_t ID, const uint8_t*const pData, const uint8_t len = 8);
	bool IsOK()
	{
		return ready;
	}
private:
	CanTxMsgTypeDef	TxMessage;
	CanRxMsgTypeDef RxMessage;
public:
	bool ready = false;
	CAN_HandleTypeDef hcan;
	uint8_t data[12][8];//接收信息
};

extern CAN can1, can2;
/*
* setspeed数据在ontimer函数换算电流后赋值于odate，于triansmit函数发送
*/