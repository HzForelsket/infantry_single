#pragma once

#include "usart.h"
#include "string"
#include "judgement.h"
#include "usb.h"

class NUC
{
public:
	typedef struct
	{
		float x, y;
	}datapack;

	float daimx, daimy;
	datapack data;
	uint8_t rxData[100] = {};

	UART* uart = nullptr;
	USB* usb = nullptr;

	void Init(UART* uart)
	{
		this->uart = uart;
	}
	uint8_t* GetDMARx(void)
	{
		return rxData;
	}

	void OnIRQHandler(size_t rxSize)
	{
		memcpy(&data, rxData+1, sizeof(datapack));
		daimx = data.x;
		daimy = data.y;
	}

};

extern NUC nuc;	

