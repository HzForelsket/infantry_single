#ifndef __IWDG_H
#define __IWDG_H
#include<stm32f4xx_hal_iwdg.h>
class MyIWDG
{
public:
	void IWDG_Init(uint8_t prer, uint16_t rlr);
	void IWDG_Feed(void);
};
#endif
