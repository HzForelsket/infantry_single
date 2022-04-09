#pragma once
#include <stm32f4xx_hal.h>

class I2C
{
public:
	I2C_HandleTypeDef hi2c2;
	uint16_t address;

	void Init(I2C_TypeDef* i2c = I2C2)
	{
		hi2c2.Instance = i2c;
		hi2c2.Init.ClockSpeed = 100000;
		hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
		hi2c2.Init.OwnAddress1 = 0;
		hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
		hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
		hi2c2.Init.OwnAddress2 = 0;
		hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
		hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
		HAL_I2C_Init(&hi2c2);
	}

	void SetAddress(uint16_t addr)
	{
		address = addr;
	}

	uint8_t Write(uint8_t command)
	{
		return HAL_I2C_Mem_Write(&hi2c2, address, 0x00, 1, &command, 1, 10);
	}
	uint8_t WriteBuffer(uint8_t* buffer)
	{
		return HAL_I2C_Mem_Write(&hi2c2, address, 0x40, 1, buffer, 128/*SSD1306_WIDTH*/, 100);
	}
};