/*
 *__/\\\_______/\\\__/\\\\____________/\\\\__/\\\________/\\\______________/\\\\\\\\\____________/\\\\\\\\\_____/\\\\\\\\\\\___
 * _\///\\\___/\\\/__\/\\\\\\________/\\\\\\_\/\\\_______\/\\\____________/\\\///////\\\_______/\\\////////____/\\\/////////\\\_
 *  ___\///\\\\\\/____\/\\\//\\\____/\\\//\\\_\/\\\_______\/\\\___________\/\\\_____\/\\\_____/\\\/____________\//\\\______\///__
 *   _____\//\\\\______\/\\\\///\\\/\\\/_\/\\\_\/\\\_______\/\\\___________\/\\\\\\\\\\\/_____/\\\_______________\////\\\_________
 *    ______\/\\\\______\/\\\__\///\\\/___\/\\\_\/\\\_______\/\\\___________\/\\\//////\\\____\/\\\__________________\////\\\______
 *     ______/\\\\\\_____\/\\\____\///_____\/\\\_\/\\\_______\/\\\___________\/\\\____\//\\\___\//\\\____________________\////\\\___
 *      ____/\\\////\\\___\/\\\_____________\/\\\_\//\\\______/\\\____________\/\\\_____\//\\\___\///\\\___________/\\\______\//\\\__
 *       __/\\\/___\///\\\_\/\\\_____________\/\\\__\///\\\\\\\\\/_____________\/\\\______\//\\\____\////\\\\\\\\\_\///\\\\\\\\\\\/___
 *        _\///_______\///__\///_____________\///_____\/////////_______________\///________\///________\/////////____\///////////_____
*/

//double_well_done
#include <stm32f4xx_hal.h>

#include "sysclk.h"
#include "can.h"
#include "tim.h"
#include "PID.h"
#include "gpio.h"
#include "usart.h"
#include "delay.h"
#include "motor.h"
#include "RC.h"
#include "control.h"
#include "IMU.h"
#include "label.h"
#include "LED.h"
#include "judgement.h"
#include "USB.h"
#include "iwdg.h"
#include "kalman.h"
#include "Android.h"
#include "SSD1306.h"
#include "Status.h"
//
// |------|			|------|
// |	  |			|	   |
// |一号轮|			|二号轮|	
// |	  |			|	   |
// |------|			|------|
// 
// 俯视
// 
// |------|			|------|
// |	  |			|	   |
// |四号轮|			|三号轮|
// |	  |			| 	   |
// |------|			|------|
// 
// 
// 
// 
// 
// 
// 
// 
// 
//定义电机对象，定义时底盘电机，摩擦轮一般选用SPD模式;云台电机选用POS模式;拨弹轮选用ACE模式
//此处的ID5对应6020的ID1，该处只定义0x0201-0x0208，故设置6020ID时尽量设为1-4--------------(2)
// 
// (2)已修改
// 现6020ID已与实际对应
// 但由于C620，C610电调发送数据时ID5，6，7，8与M6020ID1，2，3，4对应一致故其不可于同一CAN线
// 总之，C620，C610ID尽量比M6020小
// 2022-01-20
// 
//已有的PID数值都是实测效果比较好的
//举例如何添加一个电机，若需要在can1上再加一个ID5的3508发射机构电机，需更改以下几处：
//1、LEDBlink.cpp:
//               定义对象数加一：Motor can1_motor[5];
//	             添加一行：Motor(M3508,SPD, ID5, PID(10.f, 0.0655f, 3.49e-4f)),
//               ctrl.Init最后一行改为{ &can2_motor[0], &can2_motor[1], &can2_motor[2],&can2_motor[3], &can2_motor[6], &can2_motor[7], &can1_motor[5] });
//2、motor.h:
//               extern Motor can1_motor[4]改为extern Motor can1_motor[5]
//3、control.h:
//               std::array<Motor*, 6> shooter改为std::array<Motor*, 7> shooter
//               for (size_t i = 0; i != 6; ++i)改为for (size_t i = 0; i != 7; ++i)
Motor can1_motor[can1MotorNumber] = {
	Motor(M3508,SPD,chassisMotor, ID1, PID(10.f, 0.0655f, 3.49e-4f)),
	Motor(M3508,SPD,chassisMotor,  ID2, PID(10.f, 0.0655f, 3.49e-4f)),
	Motor(M3508,SPD,chassisMotor,  ID3, PID(10.f, 0.0655f, 3.49e-4f)),
	Motor(M3508,SPD,chassisMotor,  ID4, PID(10.f, 0.0655f, 3.49e-4f)),
	//Motor(M6020,POS,yawMotor,  ID5, PID(120.f, 0.f, 3.f,0.f),
	//								PID(0.2f, 0.f, 4.f,0.f)),
	Motor(M6020,POS,yawMotor,  ID5, PID(150.f, 0.f, 200.f,0.f),
									PID(0.5f, 0.01f, 10.f,0.f)),
};

Motor can2_motor[can2MotorNumber] = {
	Motor(M3508,SPD,rubMotor, ID1, PID(10.f, 0.0655f, 3.49e-4f)),
	Motor(M3508,SPD,rubMotor, ID2, PID(10.f, 0.0655f, 3.49e-4f)),
	Motor(M2006,ACE,hitMotor, ID3, PID(4.0f, 0.1f, 5.f)),
	//Motor(M6020,POS,pitchMotor, ID4, PID(0.f,0.f,0.f,0.f),
	//								PID(0.f,0.f,0.f,0.f)),
	Motor(M6020,POS,pitchMotor, ID6, PID(80.f, 0.1f, 20.f,0.f),
									PID(0.8f, 0.01f, 16.f,0.f)),
};



CAN		  can1, can2;
RC		  rc;
TIM		  tasks, fraction, photogate;
IMU       imu_pantile;
NUC       nuc;
LED       led1, led2;
USB       usb;
UART	  uart1, uart2, uart3, uart4, uart5, uart6;
Delay     delay;
Control   ctrl;
Judgement judgement;
Android   android;
I2C       i2c;
SSD1306   oled;
MyIWDG    m_iwdg;
int main(void)
{
	HAL_Init();
	SystemClockConfig();
	delay.Init(168);
	m_iwdg.IWDG_Init(IWDG_PRESCALER_64, 500);
	led1.Init({ GPIOB,GPIO_PIN_14 });
	led2.Init({ GPIOB,GPIO_PIN_15 });

	judgement.Init(&uart3, 115200, USART3);
	rc.Init(&uart2, 100000, USART2);
	android.Init(&uart5, 115200, UART5);
	//imu_pantile.Init(&uart5, 115200, UART5);
	/*
	 *uart3.Init(USART3, 115200).DMARxInit(judgement.GetDMARx());
	 *uart5.Init(UART5, 115200).DMARxInit(imu_pantile.GetDMARx());
	 *uart2.Init(USART2, 100000).DMARxInit(rc.GetDMARx());
	 *imu_pantile.Init(&uart5, 115200, UART5, IMU::OFF);
	 *
	 * urat DMA 读取数据项数由 __HAL_DMA_GET_COUNTER 操作取得 DMA stream x number of data register 与初始size之差为进行读取项数
	 * 传输终止应该是由电调控制
	 * 2022/01/17
	 */
	can1.Init(CAN1);
	can2.Init(CAN2);

	ctrl.Init(
		{ &can1_motor[0], &can1_motor[1], &can1_motor[2], &can1_motor[3] },
		{ &can1_motor[4] , &can2_motor[3] },//yaw,pitch
		{ &android },
		{ &can2_motor[0], &can2_motor[1], &can2_motor[2] });
	tasks.Init(BASE, TIM3, 1000).BaseInit();
	//imu_pantile.Init();
	//nuc.Init(&uart5);

	for (;;)
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
		delay.ms(500);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
		delay.ms(500);

		//char buf[30];
		//sprintf(buf, "Voltage= %.2f V", judgement.data.ext_power_heat_data_t.chassis_volt / 1000.f);
		//oled.SetCursor(0, 0);
		//oled.WriteString(buf, Font_7x10, oled.White);
		//sprintf(buf, "Current= %.2f A", judgement.data.ext_power_heat_data_t.chassis_current / 1000.f);
		//oled.SetCursor(0, 10);
		//oled.WriteString(buf, Font_7x10, oled.White);
		//sprintf(buf, "Power  = %.2f W", judgement.data.ext_power_heat_data_t.chassis_power);
		//oled.SetCursor(0, 20);
		//oled.WriteString(buf, Font_7x10, oled.White);
		//oled.UpdateScreen();
	}
}