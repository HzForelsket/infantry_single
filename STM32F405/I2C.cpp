#include "I2C.h"

void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	if (hi2c->Instance == I2C2)
	{
		/* USER CODE BEGIN I2C1_MspInit 0 */

		/* USER CODE END I2C1_MspInit 0 */

		__HAL_RCC_GPIOB_CLK_ENABLE();
		/**I2C1 GPIO Configuration
		PB6     ------> I2C1_SCL
		PB7     ------> I2C1_SDA
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		/* Peripheral clock enable */
		__HAL_RCC_I2C2_CLK_ENABLE();
		/* USER CODE BEGIN I2C1_MspInit 1 */

		/* USER CODE END I2C1_MspInit 1 */
	}
}