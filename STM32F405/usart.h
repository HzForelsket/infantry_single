#pragma once
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_uart.h"


#define UART_MAX_LEN 100

class UART
{
public:
	UART& Init(USART_TypeDef* Instance, uint32_t BaudRate);
	UART& DMATxInit(void);
	UART& DMARxInit(const uint8_t *buffer = nullptr,const uint32_t size = UART_MAX_LEN);
	
	void DMATransmit(uint8_t* pData, uint32_t Size) const;
	void OnUARTITHandler(uint16_t Size);
	void OnDMAITHandler(void) const;
	void RxIdleItCallback(uint16_t rxSize);
	void UARTTransmit(uint8_t* pData, uint32_t Size);

	static void DMAClearAllFlags(DMA_HandleTypeDef *dmax)
	{
		const auto ele = reinterpret_cast<uint32_t>(dmax->Instance);

		(ele == reinterpret_cast<uint32_t>(DMA1_Stream0)) ? (DMA1->LIFCR = 0x0000003D) :\
		(ele == reinterpret_cast<uint32_t>(DMA1_Stream1)) ? (DMA1->LIFCR = 0x00000F40) :\
		(ele == reinterpret_cast<uint32_t>DMA1_Stream2) ? (DMA1->LIFCR = 0x003D0000) :\
		(ele == reinterpret_cast<uint32_t>DMA1_Stream3) ? (DMA1->LIFCR = 0x0F400000) :\
		(ele == reinterpret_cast<uint32_t>DMA1_Stream4) ? (DMA1->HIFCR = 0x0000003D) :\
		(ele == reinterpret_cast<uint32_t>DMA1_Stream5) ? (DMA1->HIFCR = 0x00000F40) :\
		(ele == reinterpret_cast<uint32_t>DMA1_Stream6) ? (DMA1->HIFCR = 0x003D0000) :\
		(ele == reinterpret_cast<uint32_t>DMA1_Stream7) ? (DMA1->HIFCR = 0x0F400000) :\
		(ele == reinterpret_cast<uint32_t>DMA2_Stream0) ? (DMA2->LIFCR = 0x0000003D) :\
		(ele == reinterpret_cast<uint32_t>DMA2_Stream1) ? (DMA2->LIFCR = 0x00000F40) :\
		(ele == reinterpret_cast<uint32_t>DMA2_Stream2) ? (DMA2->LIFCR = 0x003D0000) :\
		(ele == reinterpret_cast<uint32_t>DMA2_Stream3) ? (DMA2->LIFCR = 0x0F400000) :\
		(ele == reinterpret_cast<uint32_t>DMA2_Stream4) ? (DMA2->HIFCR = 0x0000003D) :\
		(ele == reinterpret_cast<uint32_t>DMA2_Stream5) ? (DMA2->HIFCR = 0x00000F40) :\
		(ele == reinterpret_cast<uint32_t>DMA2_Stream6) ? (DMA2->HIFCR = 0x003D0000) :\
		(DMA2->HIFCR = 0x0F400000);
	}

	UART_HandleTypeDef huart;
	uint8_t m_uartrx[UART_MAX_LEN];

	uint32_t m_receiveTime[2]{};
	enum {now,pre};
	bool IsOK()
	{
		if (m_receiveTime[now] == 0 || (HAL_GetTick() - m_receiveTime[now]) > 1000)
			return false;
		return true;
	}
};

extern UART uart1,uart2,uart3, uart4, uart5, uart6;