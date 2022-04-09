#include"motor.h"
Motor::Motor(const type_t type, const motor_mode mode, const motor_use use, const uint32_t id, PID _speed, PID _position)
	: ID(id), type(type), mode(mode), m_motor_use(use), m_status(UNCONNECTED)
{
	getmax(type);
	memcpy(&pid[speed], &_speed, sizeof(PID));
	memcpy(&pid[position], &_position, sizeof(PID));
}
Motor::Motor(const type_t type, const motor_mode mode, const motor_use use, const uint32_t id, PID _speed)
	: ID(id), type(type), mode(mode), m_motor_use(use), m_status(UNCONNECTED)
{
	getmax(type);
	memcpy(&pid[speed], &_speed, sizeof(PID));
}
void Motor::StatusIdentifier(int32_t torque_current)
{
	if (torque_current == old_torque_current)
		disconnectCount++;
	else
		disconnectCount = 0;

	if (disconnectCount >= disconnectMax)
	{
		disconnectCount = disconnectMax;
		if (old_torque_current == 0)
			m_status = UNCONNECTED;
		else
			m_status = DISCONNECTED;
	}
	else
		m_status = FINE;

	old_torque_current = torque_current;
}
uint8_t Motor::getStatus()const
{
	return (uint8_t)m_status;
}
void Motor::Ontimer(uint8_t idata[][8], uint8_t* odata)//idate: receive;odate: trainsmit;RC
{
	uint32_t trainsmit_or_receive_ID = this->ID - ID1;

	//----------------------------------------------------------------
	/*if (this->type == M6020)
	{
		trainsmit_or_receive_ID += 4;
	}*/
	//----------------------------------------------------------------
	this->torque_current = getword(idata[trainsmit_or_receive_ID][4], idata[trainsmit_or_receive_ID][5]);
	this->StatusIdentifier(this->torque_current);
	this->angle[now] = getword(idata[trainsmit_or_receive_ID][0], idata[trainsmit_or_receive_ID][1]);

	//Get currrent speed
	if (type == EC60)
	{
		curspeed = static_cast<float>(getdeltaa(angle[now] - angle[pre])) / T / 8192.f * 60.f;
	}
	else {
		curspeed = getword(idata[trainsmit_or_receive_ID][2], idata[trainsmit_or_receive_ID][3]);
	}
	//----------------------------------------------------------------
	/*if (this->type == M6020)
	{
		trainsmit_or_receive_ID -= 4;
	}*/
	//----------------------------------------------------------------
	//20220121--hz
	if (mode == ACE)
	{

		if (spinning)
		{
			//1秒8发 36/1减速比 一圈八格
			current += pid[speed].Delta(setspeed - curspeed);
			current = setrange(current, maxcurrent);
		}
		else {
			if (need_curcircle > 0)
			{
				current += pid[speed].Delta(setspeed - curspeed);
				current = setrange(current, maxcurrent);
				uint8_t deviation = 125;
				if (angle[now] >= stopAngle - deviation && angle[now] <= stopAngle + deviation)
					need_curcircle--;
				if (need_curcircle <= 0)
				{
					need_curcircle = 0;
					stopAngle = angle[now];
					pd = 0;
					setspeed = 0;
					current = 0;
				}
			}
			else if (need_curcircle <= 0)
			{
				setspeed = 0;
				current += pid[speed].Delta(setspeed - curspeed);
				current = setrange(current, maxcurrent);
			}
		}
	}
	else if (mode == POS)
	{
		setspeed = pid[position].Position(kalman.Filter(getdeltaa(setangle - angle[now])));
		setspeed = setrange(setspeed, maxspeed);
		current = pid[speed].Position(setspeed - curspeed);
		current = currentKalman.Filter(current);
		current = setrange(current, maxcurrent);
	}
	else if (mode == SPD)
	{
		current += pid[speed].Delta(setspeed - curspeed);
		current = setrange(current, maxcurrent);
	}
	angle[pre] = angle[now];
	odata[trainsmit_or_receive_ID * 2] = (current & 0xff00) >> 8;//高八位
	odata[trainsmit_or_receive_ID * 2 + 1] = current & 0x00ff;
}
void Motor::getmax(const type_t type)
{
	adjspeed = 3000;
	switch (type)
	{
	case M3508:
		maxcurrent = 16384;
		maxspeed = 3800;
		break;
	case M3510:
		maxcurrent = 13000;
		maxspeed = 9000;
		break;
	case M2310:
		maxcurrent = 13000;
		maxspeed = 9000;
		adjspeed = 1000;
		break;
	case EC60:
		maxcurrent = 5000;
		maxspeed = 300;
		break;
	case M6623:
		maxcurrent = 5000;
		maxspeed = 300;
		break;
	case M6020:
		maxcurrent = 30000;
		maxspeed = 200;
		adjspeed = 80;
		break;
	case M2006:
		maxcurrent = 10000;
		adjspeed = 1000;
		maxspeed = 3000;
		break;
	default:;
	}
}

int16_t Motor::getdeltaa(int16_t diff)
{
	if (diff <= -4096)
		diff += 8192;
	else if (diff > 4096)
		diff -= 8192;
	return diff;
}

int16_t Motor::getword(const uint8_t high, const uint8_t low)
{
	const int16_t word = high;
	return (word << 8) + low;
}

int32_t Motor::setrange(const int32_t original, const int32_t range)
{
	return std::max(std::min(range, original), -range);
}