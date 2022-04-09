#include "iwdg.h" 
void MyIWDG::IWDG_Init(uint8_t prer, uint16_t rlr)
{

        IWDG->KR = 0X5555;		 										  
        IWDG->PR = prer;  
        IWDG->RLR = rlr;    
        IWDG->KR = 0XAAAA;										   
        IWDG->KR = 0XCCCC;	
}
void MyIWDG::IWDG_Feed(void)
{
    IWDG->KR = 0XAAAA;										   
}
