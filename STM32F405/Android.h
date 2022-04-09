#pragma once
#include <stm32f4xx_hal.h>
#include <arm_math.h>
#include"usart.h"
#include"IMU.h"
#include"Status.h"
#include"label.h"
#include"judgement.h"
class Android:public IMU,public Status
{
public:
	 void Init(UART* huart, uint32_t baud, USART_TypeDef* uart_base);
	 void send_error_message();
private:
	uint8_t rxData[99];
	uint8_t txData[99];
	UART* m_uart;
	typedef struct
	{
		int32_t what;
		Angle angle;
		Aim aim;
	}InfoPack;
	InfoPack infopack;
	 void OnIRQHandler(uint16_t rxSize);
};

extern Judgement judgement;