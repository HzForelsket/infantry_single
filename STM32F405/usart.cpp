#include "usart.h"
#include "RC.h"
#include "judgement.h"
UART& UART::Init(USART_TypeDef* Instance, const uint32_t BaudRate)
{
	huart.Instance = Instance;
	huart.Init.BaudRate = BaudRate;
	huart.Init.WordLength = UART_WORDLENGTH_8B;
	huart.Init.StopBits = UART_STOPBITS_1;
	huart.Init.Parity = UART_PARITY_NONE;
	huart.Init.Mode = UART_MODE_TX_RX;
	huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&huart);
	return *this;
}
UART& UART::DMATxInit(void)
{
	__HAL_DMA_DISABLE(huart.hdmatx);
	huart.hdmatx->Instance->PAR = reinterpret_cast<uint32_t>(&huart.Instance->DR);
	huart.hdmatx->Instance->M0AR = 0;
	DMAClearAllFlags(huart.hdmatx);
	//开启DMA发送中断
	__HAL_DMA_ENABLE_IT(huart.hdmatx, DMA_IT_TC);
	SET_BIT(huart.Instance->CR3, USART_CR3_DMAT);
	return *this;
}
UART& UART::DMARxInit(const uint8_t *buffer, const uint32_t size)//size = UART_MAX_LEN
{
	//使能串口DMA接收
	__HAL_DMA_DISABLE(huart.hdmarx);
	huart.hdmarx->Instance->PAR = reinterpret_cast<uint32_t>(&huart.Instance->DR);//PAR is DMA stream x peripheral address register,DR is UART Date register address
	huart.hdmarx->Instance->NDTR = size;
	if (buffer == nullptr)buffer = m_uartrx;
	huart.hdmarx->Instance->M0AR = reinterpret_cast<uint32_t>(buffer);
	DMAClearAllFlags(huart.hdmarx);
	__HAL_DMA_ENABLE(huart.hdmarx);
	SET_BIT(huart.Instance->CR3, USART_CR3_DMAR);
	//开启串口空闲中断
	__HAL_UART_CLEAR_PEFLAG(&huart);
	__HAL_UART_ENABLE_IT(&huart, UART_IT_IDLE);
	return *this;
}
//DMA Init
void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	if (uartHandle->Instance == USART1)
	{
		static DMA_HandleTypeDef hdma_rx;
		static DMA_HandleTypeDef hdma_tx;
		/* USART6 clock enable */
		__HAL_RCC_USART1_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();

		/**USART6 GPIO Configuration
		PG14     ------> USART6_TX
		PG9     ------> USART6_RX
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* USART6 DMA Init */
		__HAL_RCC_DMA2_CLK_ENABLE();
		/* USART6_RX Init */
		hdma_rx.Instance = DMA2_Stream2;
		hdma_rx.Init.Channel = DMA_CHANNEL_4;
		hdma_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_rx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_rx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_rx.Init.Mode = DMA_NORMAL;
		hdma_rx.Init.Priority = DMA_PRIORITY_LOW;
		hdma_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		HAL_DMA_Init(&hdma_rx);

		__HAL_LINKDMA(uartHandle, hdmarx, hdma_rx);

		/* USART6_TX Init */
		hdma_tx.Instance = DMA2_Stream7;
		hdma_tx.Init.Channel = DMA_CHANNEL_4;
		hdma_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
		hdma_tx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_tx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_tx.Init.Mode = DMA_NORMAL;
		hdma_tx.Init.Priority = DMA_PRIORITY_LOW;
		hdma_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		HAL_DMA_Init(&hdma_tx);

		__HAL_LINKDMA(uartHandle, hdmatx, hdma_tx);

		/* USART6 interrupt Init */
		HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(USART1_IRQn);

		/* DMA interrupt init */
		/* DMA2_Stream1_IRQn interrupt configuration */
		HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
		/* DMA2_Stream6_IRQn interrupt configuration */
		HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);
	}
	if (uartHandle->Instance == USART2)
	{
		static DMA_HandleTypeDef hdma_rx;
		static DMA_HandleTypeDef hdma_tx;
		/* USART2 clock enable */
		__HAL_RCC_USART2_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();

		/**USART6 GPIO Configuration
		PG14     ------> USART2_TX
		PG9     ------> USART2_RX
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* USART2 DMA Init */
		__HAL_RCC_DMA1_CLK_ENABLE();
		/* USART2_RX Init */
		hdma_rx.Instance = DMA1_Stream5;
		hdma_rx.Init.Channel = DMA_CHANNEL_4;
		hdma_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_rx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_rx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_rx.Init.Mode = DMA_NORMAL;
		hdma_rx.Init.Priority = DMA_PRIORITY_LOW;
		hdma_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		HAL_DMA_Init(&hdma_rx);

		__HAL_LINKDMA(uartHandle, hdmarx, hdma_rx);

		/* USART2_TX Init */
		hdma_tx.Instance = DMA1_Stream6;
		hdma_tx.Init.Channel = DMA_CHANNEL_4;
		hdma_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
		hdma_tx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_tx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_tx.Init.Mode = DMA_NORMAL;
		hdma_tx.Init.Priority = DMA_PRIORITY_LOW;
		hdma_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		HAL_DMA_Init(&hdma_tx);

		__HAL_LINKDMA(uartHandle, hdmatx, hdma_tx);

		/* USART2 interrupt Init */
		HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(USART2_IRQn);

		/* DMA interrupt init */
		/* DMA2_Stream1_IRQn interrupt configuration */
		HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
		/* DMA2_Stream6_IRQn interrupt configuration */
		HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
	}
	if (uartHandle->Instance == USART3)
	{
		static DMA_HandleTypeDef hdma_rx;
		static DMA_HandleTypeDef hdma_tx;
		/* USART2 clock enable */
		__HAL_RCC_USART3_CLK_ENABLE();
		__HAL_RCC_GPIOC_CLK_ENABLE();

		/**USART6 GPIO Configuration
		PG14     ------> USART2_TX
		PG9     ------> USART2_RX
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_10;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		/* USART2 DMA Init */
		__HAL_RCC_DMA1_CLK_ENABLE();
		/* USART2_RX Init */
		hdma_rx.Instance = DMA1_Stream1;
		hdma_rx.Init.Channel = DMA_CHANNEL_4;
		hdma_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_rx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_rx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_rx.Init.Mode = DMA_NORMAL;
		hdma_rx.Init.Priority = DMA_PRIORITY_LOW;
		hdma_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		HAL_DMA_Init(&hdma_rx);

		__HAL_LINKDMA(uartHandle, hdmarx, hdma_rx);

		/* USART2_TX Init */
		hdma_tx.Instance = DMA1_Stream3;
		hdma_tx.Init.Channel = DMA_CHANNEL_4;
		hdma_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
		hdma_tx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_tx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_tx.Init.Mode = DMA_NORMAL;
		hdma_tx.Init.Priority = DMA_PRIORITY_LOW;
		hdma_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		HAL_DMA_Init(&hdma_tx);

		__HAL_LINKDMA(uartHandle, hdmatx, hdma_tx);

		/* USART2 interrupt Init */
		HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(USART3_IRQn);

		/* DMA interrupt init */
		/* DMA2_Stream1_IRQn interrupt configuration */
		HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
		/* DMA2_Stream6_IRQn interrupt configuration */
		HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);
	}
	if (uartHandle->Instance == UART4)
	{
		static DMA_HandleTypeDef hdma_rx;
		static DMA_HandleTypeDef hdma_tx;
		/* USART1 clock enable */
		__HAL_RCC_UART4_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();

		/**USART1 GPIO Configuration
		PA0     ------> UART4_TX
		PA1     ------> UART4_RX
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

		/* USART1 DMA Init */
		__HAL_RCC_DMA1_CLK_ENABLE();
		/* USART1_RX Init */
		hdma_rx.Instance = DMA1_Stream2;
		hdma_rx.Init.Channel = DMA_CHANNEL_4;
		hdma_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_rx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_rx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_rx.Init.Mode = DMA_NORMAL;
		hdma_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
		hdma_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		HAL_DMA_Init(&hdma_rx);

		__HAL_LINKDMA(uartHandle, hdmarx, hdma_rx);
		/* USART1_TX Init */
		hdma_tx.Instance = DMA1_Stream4;
		hdma_tx.Init.Channel = DMA_CHANNEL_4;
		hdma_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
		hdma_tx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_tx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_tx.Init.Mode = DMA_NORMAL;
		hdma_tx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
		hdma_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		HAL_DMA_Init(&hdma_tx);
		__HAL_LINKDMA(uartHandle, hdmatx, hdma_tx);

		/* USART1 interrupt Init */
		HAL_NVIC_SetPriority(UART4_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(UART4_IRQn);

		/* DMA interrupt init */
		/* DMA2_Stream2_IRQn interrupt configuration */
		HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);
		/* DMA2_Stream7_IRQn interrupt configuration */
		HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);
	}
	if (uartHandle->Instance == UART5)
	{
		static DMA_HandleTypeDef hdma_rx;
		static DMA_HandleTypeDef hdma_tx;

		__HAL_RCC_UART5_CLK_ENABLE();
		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOD_CLK_ENABLE();

		GPIO_InitStruct.Pin = GPIO_PIN_12;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		GPIO_InitTypeDef GPIO_InitStruct2;
		GPIO_InitStruct2.Pin = GPIO_PIN_2;
		GPIO_InitStruct2.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct2.Pull = GPIO_PULLUP;
		GPIO_InitStruct2.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct2.Alternate = GPIO_AF8_UART5;
		HAL_GPIO_Init(GPIOD, &GPIO_InitStruct2);
		/* USART2 DMA Init */
		__HAL_RCC_DMA1_CLK_ENABLE();
		/* USART2_RX Init */
		hdma_rx.Instance = DMA1_Stream0;
		hdma_rx.Init.Channel = DMA_CHANNEL_4;
		hdma_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_rx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_rx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_rx.Init.Mode = DMA_NORMAL;
		hdma_rx.Init.Priority = DMA_PRIORITY_LOW;
		hdma_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		HAL_DMA_Init(&hdma_rx);
		__HAL_LINKDMA(uartHandle, hdmarx, hdma_rx);
		/* USART2_TX Init */
		hdma_tx.Instance = DMA1_Stream7;
		hdma_tx.Init.Channel = DMA_CHANNEL_4;
		hdma_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
		hdma_tx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_tx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_tx.Init.Mode = DMA_NORMAL;
		hdma_tx.Init.Priority = DMA_PRIORITY_LOW;
		hdma_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		HAL_DMA_Init(&hdma_tx);
		__HAL_LINKDMA(uartHandle, hdmatx, hdma_tx);
		/* USART2 interrupt Init */
		HAL_NVIC_SetPriority(UART5_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(UART5_IRQn);
		/* DMA interrupt init */
		/* DMA2_Stream1_IRQn interrupt configuration */
		HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
		/* DMA2_Stream6_IRQn interrupt configuration */
		HAL_NVIC_SetPriority(DMA1_Stream7_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(DMA1_Stream7_IRQn);
	}
	if (uartHandle->Instance == USART6)
	{
		static DMA_HandleTypeDef hdma_rx;
		static DMA_HandleTypeDef hdma_tx;
		/* USART1 clock enable */
		__HAL_RCC_USART6_CLK_ENABLE();
		__HAL_RCC_GPIOC_CLK_ENABLE();

		/**USART1 GPIO Configuration
		PC6     ------> USART6_TX
		PC7     ------> USART6_RX
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		/* USART6 DMA Init */
		__HAL_RCC_DMA2_CLK_ENABLE();
		/* USART6_RX Init */
		hdma_rx.Instance = DMA2_Stream1;
		hdma_rx.Init.Channel = DMA_CHANNEL_5;
		hdma_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
		hdma_rx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_rx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_rx.Init.Mode = DMA_NORMAL;
		hdma_rx.Init.Priority = DMA_PRIORITY_LOW;
		hdma_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		HAL_DMA_Init(&hdma_rx);

		__HAL_LINKDMA(uartHandle, hdmarx, hdma_rx);
		/* USART6_TX Init */
		hdma_tx.Instance = DMA2_Stream6;
		hdma_tx.Init.Channel = DMA_CHANNEL_5;
		hdma_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
		hdma_tx.Init.PeriphInc = DMA_PINC_DISABLE;
		hdma_tx.Init.MemInc = DMA_MINC_ENABLE;
		hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
		hdma_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
		hdma_tx.Init.Mode = DMA_NORMAL;
		hdma_tx.Init.Priority = DMA_PRIORITY_LOW;
		hdma_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
		HAL_DMA_Init(&hdma_tx);
		__HAL_LINKDMA(uartHandle, hdmatx, hdma_tx);

		/* USART1 interrupt Init */
		HAL_NVIC_SetPriority(USART6_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(USART6_IRQn);

		/* DMA interrupt init */
		/* DMA2_Stream2_IRQn interrupt configuration */
		HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
		/* DMA2_Stream7_IRQn interrupt configuration */
		HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);
	}
}

void UART::RxIdleItCallback(const uint16_t rxSize)
{
	m_receiveTime[pre] = m_receiveTime[now];
	m_receiveTime[now] = HAL_GetTick();
	if (huart.callback)
	{
		huart.callback(huart.pthis,rxSize);
	}
}
void UART::OnUARTITHandler(const uint16_t Size = UART_MAX_LEN)
{
	if (__HAL_UART_GET_FLAG(&huart, UART_FLAG_IDLE) && __HAL_UART_GET_IT_SOURCE(&huart, UART_IT_IDLE))
	{
		__HAL_UART_CLEAR_IDLEFLAG(&huart);
		__HAL_DMA_DISABLE(huart.hdmarx);

		DMAClearAllFlags(huart.hdmarx);
		RxIdleItCallback(Size - __HAL_DMA_GET_COUNTER(huart.hdmarx));

		__HAL_DMA_SET_COUNTER(huart.hdmarx, Size);
		__HAL_DMA_ENABLE(huart.hdmarx);
	}
	HAL_UART_IRQHandler(&huart);
}
void UART::OnDMAITHandler(void) const
{
	UART::DMAClearAllFlags(huart.hdmatx);
	__HAL_DMA_DISABLE(huart.hdmatx);
}


extern "C" void USART1_IRQHandler(void)
{
	uart1.OnUARTITHandler();
}
extern "C" void USART2_IRQHandler(void)
{
	uart2.OnUARTITHandler();
}
extern "C" void USART3_IRQHandler(void)
{
	uart3.OnUARTITHandler();
}
extern "C" void UART4_IRQHandler(void)
{
	uart4.OnUARTITHandler();
}
extern "C" void UART5_IRQHandler(void)
{
	uart5.OnUARTITHandler();
}
extern "C" void USART6_IRQHandler(void)
{
	uart6.OnUARTITHandler();
}

void UART::DMATransmit(uint8_t*const pData, const uint32_t Size) const
{
	__HAL_DMA_DISABLE(huart.hdmatx);
	huart.hdmatx->Instance->NDTR = Size;
	huart.hdmatx->Instance->M0AR = reinterpret_cast<uint32_t>(pData);
	DMAClearAllFlags(huart.hdmatx);
	__HAL_DMA_ENABLE(huart.hdmatx);
}
void UART::UARTTransmit(uint8_t* pData, uint32_t Size)
{
	HAL_UART_Transmit(&huart, static_cast<uint8_t*>(pData), Size, 0xffff);
}

