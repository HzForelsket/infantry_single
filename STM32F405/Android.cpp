#include"Android.h"
void Android::Init(UART* huart, uint32_t baud, USART_TypeDef* uart_base)
{
	Status::init(true, false);
	huart->Init(uart_base, baud).DMARxInit(rxData);
	int addr = function(0, this);
	huart->huart.pthis = reinterpret_cast<void*>(addr);
	addr = function(0, &Android::OnIRQHandler);
	huart->huart.callback = reinterpret_cast<void(*)(void*,uint16_t)>(addr);
	m_uart = huart;
}
void Android::send_error_message()
{
	uint32_t length = 0;
	switch (infopack.what)
	{
	case 0:
		break;
	case 1:
	{
		struct {
			int32_t mode;
			float power;

			//TODO: CRC

		}data;

		data.mode = 1;
		data.power = judgement.data.ext_power_heat_data_t.chassis_power;
		memcpy(txData, &data, sizeof(data));
		length = sizeof(data);
	}break;
	case 2:
	{
		struct {
			int32_t mode;
			uint32_t status;
			// 2 bytes can1 + 2 bytes can2
			uint32_t motor_status;
			//TODO: CRC

		}data;
		data.mode = 2;
		data.status = 0;
		//if (this->can_test(&can1))
		//	data.status |= 1 << 0;
		//if (this->can_test(&can2))
		//	data.status |= 1 << 1;

		data.status = getStatusCode();
		data.motor_status = getMotorStatus(can1_motor, can2_motor);
		//bool canErrorM[2]={ this->can_test(&can1),this->can_test(&can2) };
		memcpy(txData, &data, sizeof(data));
		length = sizeof(data);
	}
	default:
		break;
	}
	m_uart->UARTTransmit(txData, length);
}
void Android::OnIRQHandler(uint16_t rxSize)
{
	if ((crc_test(rxData)) && (rxSize == MAX_DateLength) && mode == GYRO)
	{
		this->angle.pitch = getword(rxData[_PAYLOAD_EulerAngles_ + 1], rxData[_PAYLOAD_EulerAngles_ + 2]) / 100.f;
		this->angle.roll = getword(rxData[_PAYLOAD_EulerAngles_ + 3], rxData[_PAYLOAD_EulerAngles_ + 4]) / 100.f;
		this->angle.yaw = getword(rxData[_PAYLOAD_EulerAngles_ + 5], rxData[_PAYLOAD_EulerAngles_ + 6]) / 10.f;
		//this->angularVelocity.pitch = getword(rxData[_PAYLOAD_AngularVelocity_ + 1], rxData[_PAYLOAD_AngularVelocity_ + 2]) / 10.f;
		//this->angularVelocity.roll = getword(rxData[_PAYLOAD_AngularVelocity_ + 3], rxData[_PAYLOAD_AngularVelocity_ + 4]) / 10.f;
		//this->angularVelocity.yaw = getword(rxData[_PAYLOAD_AngularVelocity_ + 5], rxData[_PAYLOAD_AngularVelocity_ + 6]) / 10.f;
		//this->acceleration.pitch = getword(rxData[_PAYLOAD_Acceleration_ + 1], rxData[_PAYLOAD_Acceleration_ + 2]) / 1000.f;
		//this->acceleration.roll = getword(rxData[_PAYLOAD_Acceleration_ + 3], rxData[_PAYLOAD_Acceleration_ + 4]) / 1000.f;
		//this->acceleration.yaw = getword(rxData[_PAYLOAD_Acceleration_ + 5], rxData[_PAYLOAD_Acceleration_ + 6]) / 1000.f;
	}
	else
	{
		memcpy(&infopack, rxData, sizeof(InfoPack));
		//send_error_message();
		angle = infopack.angle;
		angle.yaw = (-angle.yaw + 180.f);
		angle.pitch = (-angle.pitch + 90.f);
		angle.roll = (-angle.roll + 180.f);
		aim = infopack.aim;
		if (aim.x)
			aim.x = infopack.aim.x - 0.103906f;//ÐÞÕýÆ«ÒÆÁ¿
		if (aim.y)
		aim.y = -1.0 * infopack.aim.y - 0.2377083f;
		
	}

}

