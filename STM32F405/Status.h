#pragma once
#include "can.h"
#include "usart.h"
#include "motor.h"
class Status
{
	bool m_CAN, m_UART[3];
public:
		bool can_test(CAN* here_can);
		void init(bool canTestOpen,bool uartTestOpen);
		template<size_t N1, size_t N2>
		uint32_t getMotorStatus(Motor(&can1_motor)[N1], Motor(&can2_motor)[N2])
		{
			uint16_t high = 0, low = 0;
			for (int i = 0; i != N1; ++i)
			{
				high|= (can1_motor[i].getStatus() << (2 * i));
			}
			for (int i = 0; i != N2; ++i)
			{
				low |= (can2_motor[i].getStatus() << (2 * i));
			}
			return (high << 16) | low;
		}
		uint32_t getStatusCode()
		{
			uint32_t status = 0;

			if (can1.IsOK())
				status |= 1 << 0;
			if (can2.IsOK())
				status |= 1 << 1;
			if(uart2.IsOK())
				status |= 1 << 2;
			if (uart3.IsOK())
				status |= 1 << 3;
			if (uart5.IsOK())
				status |= 1 << 4;
			if(IsDebugOK())
				status |= 1 << 5;
		}
		bool IsDebugOK()
		{
			return CoreDebug->DHCSR & 1;
		}
		uint16_t sumTest;
		//bool uart_test();
};

