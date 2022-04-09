#include"RC.h"
#include"label.h"
void RC::OnIRQHandler(size_t rxSize)
{
	if (rxSize != 18)return;
	if ((m_frame[0] | m_frame[1] | m_frame[2] | m_frame[3] | m_frame[4] | m_frame[5]) == 0)return;

	rc.ch[0] = ((m_frame[0] | m_frame[1] << 8) & 0x07FF) - 1024;
	rc.ch[1] = ((m_frame[1] >> 3 | m_frame[2] << 5) & 0x07FF) - 1024;
	rc.ch[2] = ((m_frame[2] >> 6 | m_frame[3] << 2 | m_frame[4] << 10) & 0x07FF) - 1024;
	rc.ch[3] = ((m_frame[4] >> 1 | m_frame[5] << 7) & 0x07FF) - 1024;

	for (int i = 0; i < 4; i++)
	rc.ch[i] = (rc.ch_kalman[i].Filter(rc.ch[i]));


	if (rc.ch[0] <= 8 && rc.ch[0] >= -8)rc.ch[0] = 0;
	if (rc.ch[1] <= 8 && rc.ch[1] >= -8)rc.ch[1] = 0;
	if (rc.ch[2] <= 8 && rc.ch[2] >= -8)rc.ch[2] = 0;
	if (rc.ch[3] <= 8 && rc.ch[3] >= -8)rc.ch[3] = 0;

	rc.s[0] = ((m_frame[5] >> 4) & 0x0C) >> 2;//s2
	rc.s[1] = ((m_frame[5] >> 4) & 0x03);//s1

	pc.x = ((int16_t)m_frame[6]) | (((int16_t)m_frame[7]) << 8);
	pc.y = ((int16_t)m_frame[8]) | (((int16_t)m_frame[9]) << 8);
	static int32_t mid_num_x=0, mid_num_y=0;
	mid_num_x = pc.x;
	mid_num_y = pc.y;
	mid_num_x= pc.KalmanRe[0].Filter(pc.kalman[0].Filter(mid_num_x * 1.f));
	mid_num_y = pc.KalmanRe[1].Filter(pc.kalman[1].Filter(mid_num_y * 1.f));
	if (pc.x)
	pc.x = mid_num_x;
	if(pc.y)
	pc.y = mid_num_y;

	pc.z = ((int16_t)m_frame[10]) | (((int16_t)m_frame[11] << 8));

	pc.press_l = m_frame[12];
	pc.press_r = m_frame[13];


	pc.key_h = m_frame[15];
	pc.key_l = m_frame[14];

	if (android.mode == IMU::ANDROID)
	{
		memcpy(&imu_pantile.angle, &android.angle, sizeof(Angle));
		memcpy(&imu_pantile.aim, &android.aim, sizeof(Aim));
	}
	else
	{
		imu_pantile.aim.x = 0;
		imu_pantile.aim.y = 0;
	}
}

void RC::OnRC()
{
	if (rc.s[0] == mid_position)
	{	

		m_iwdg.IWDG_Feed();
		android.mode = IMU::GYRO;

		switch (rc.s[1])
		{
		case up_position:
			ctrl.manual_chassis(rc.ch[0] * MAXSPEED / 660, rc.ch[1] * MAXSPEED / 660, 0);
			ctrl.manual_pantile(rc.ch[2] / 660.f, -rc.ch[3] / 660.f);
			ctrl.manual_shoot(false, true, true);
			break;
		case mid_position:
			ctrl.manual_shoot(false, false, false);
			if (rc.ch[0] || rc.ch[1])
			{
				if (!lock_mode)
				{
					if (rc.fritIntoLock)
					{
						imu_pantile.initialYaw = imu_pantile.GetAngleYaw();
						imu_pantile.initialPitch = imu_pantile.GetAnglePitch();
					}
					rc.fritIntoLock = false;
					lock_mode = true;
					top_mode = false;
					follow_mode = false;
					ctrl.chassis.m_chassisMode = Control::Chassis::LOCK;
				}
				if(abs(ctrl.pantile.midyaw- ctrl.pantile.pantile[0]->angle[now])<= LOCK_MODE_ERROR)
				{
					ctrl.chassis.m_chassisMode = Control::Chassis::FOLLOW;
					ctrl.manual_chassis(rc.ch[0] * MAXSPEED / 660, rc.ch[1] * MAXSPEED / 660, -1.f*rc.ch[2] * MAXSPEED / 660.f);
					ctrl.manual_pantile(0, -rc.ch[3] / 660.f);
				}
				else
				{
					ctrl.chassis.m_chassisMode = Control::Chassis::LOCK;
					ctrl.manual_chassis(rc.ch[0] * MAXSPEED / 660, rc.ch[1] * MAXSPEED / 660, ctrl.keepChassisFollow());
					ctrl.manual_pantile(rc.ch[2] / 660.f, -rc.ch[3] / 660.f);
				}
			}
			else if (!rc.ch[0] && !rc.ch[1])
			{
				if (!follow_mode)
				{
					ctrl.chassis.m_chassisMode = Control::Chassis::FOLLOW;
					top_mode = false;
					follow_mode = true;
					lock_mode = false;
				}
				if (!rc.fritIntoLock)
					rc.fritIntoLock = true;
				ctrl.manual_chassis(rc.ch[0] * MAXSPEED / 660, rc.ch[1] * MAXSPEED / 660, 0);
				ctrl.manual_pantile(rc.ch[2] / 660.f, -rc.ch[3] / 660.f);
			}
			break;
		case down_position:
			if (!top_mode)
			{
				imu_pantile.initialYaw = imu_pantile.GetAngleYaw();
				imu_pantile.initialPitch = imu_pantile.GetAnglePitch();
				top_mode = true;//进入小陀螺模式
				follow_mode = false;
				lock_mode = false;
				ctrl.chassis.m_chassisMode = Control::Chassis::ROTATION;
			}
			ctrl.manual_shoot(false, false, false);

			ctrl.manual_chassis(rc.ch[0] * MAXSPEED / 660, rc.ch[1] * MAXSPEED / 660, 1500.f);
			ctrl.manual_pantile(rc.ch[2] / 660.f, -rc.ch[3] / 660.0);
			break;
		default:
			break;
		}
	}
	else if (rc.s[0] == down_position)
	{
		ctrl.manual_chassis(0, 0, 0);
		ctrl.pantile.setpitch = (ctrl.pantile.pitchmax + ctrl.pantile.pitchmin) / 2;
		ctrl.pantile.setyaw = ctrl.pantile.midyaw;
		ctrl.manual_shoot(false, false, false);

	}
}
//pc.press_l：鼠标左键 pc.press_r：鼠标右键 pc.x:鼠标左右平移 pc.y:鼠标前后平移,朝己为正 pc.key_l，pc.key_h：按键
void RC::OnPC()
{

	if (rc.s[0] == up_position)
	{	
		m_iwdg.IWDG_Feed();
		double ch_yaw=pc.x, ch_pitch=pc.y;
		const float adjspeed = MAXSPEED;
		if (rc.s[1] == up_position)
		{
			android.mode = IMU::ANDROID;
			ctrl.manual_shoot(pc.press_l&&(!pc.prePress_L), pc.press_r, true);
			pc.prePress_L = pc.press_l;
		}
		else if (rc.s[1] == mid_position)
		{
			ctrl.manual_shoot(pc.press_l && (!pc.prePress_L), pc.press_r, (pc.press_l || pc.press_r));
			pc.prePress_L = pc.press_l;

		}
		else if (rc.s[1] == down_position)
		{
			android.mode = IMU::GYRO;
			ctrl.manual_shoot(pc.press_l && (!pc.prePress_L), pc.press_r, true);
			pc.prePress_L = pc.press_l;
		}
		if (pc.key_h & X)
		{
			ctrl.shooter.heat_ulimit = true;
		}
		else if(!(pc.key_h&X))
		{
			ctrl.shooter.heat_ulimit = false;
		}
		if (pc.key_l & Q)
		{
			if (!top_mode)
			{
				imu_pantile.initialYaw = imu_pantile.GetAngleYaw();
				imu_pantile.initialPitch = imu_pantile.GetAnglePitch();
				pc.prePressShift = false;
				top_mode = true;//进入小陀螺模式
				follow_mode = false;
				lock_mode = false;
				ctrl.chassis.m_chassisMode = Control::Chassis::ROTATION;
			}
		}
		if (pc.key_l & E)
		{
			if (!lock_mode)
			{
				imu_pantile.initialYaw = imu_pantile.GetAngleYaw();
				imu_pantile.initialPitch = imu_pantile.GetAnglePitch();
				ctrl.chassis.m_chassisMode = Control::Chassis::LOCK;
				first_enter_lock_mode = true;
				top_mode = false;//退出小陀螺模式
				follow_mode = false;
				lock_mode = true;
			}

		}
		if (pc.key_h & Z)
		{
			pc.x = 0;
			pc.y = 0;
		}
		if (pc.key_l & CTRL)
		{
			auto_aim = true;
		}
		else if(!(pc.key_l&CTRL))
		{
			auto_aim = false;
		}

		if (top_mode || follow_mode || lock_mode)
		{
			pc.direct = ((!!(pc.key_l & W)) + (!!(pc.key_l & S)) * (-1));
			pc.setY = pc.direct * adjspeed;
			pc.direct = ((!!(pc.key_l & D)) + (!!(pc.key_l & A)) * (-1));
			pc.setX = pc.direct * adjspeed;

			if (top_mode)
			{
				if (pc.key_l & SHIFT)
				{

					ctrl.chassis.m_chassisMode = Control::Chassis::ROTATION;
					pc.setZ = adjspeed;//底盘旋转
					if (!pc.prePressShift)
					{
						imu_pantile.initialYaw = imu_pantile.GetAngleYaw()+front_adjustment;
						if (imu_pantile.initialYaw>180.f)
						{
							imu_pantile.initialYaw -= 180.f;
						}
						else if(imu_pantile.initialYaw<-180.f)
						{
							imu_pantile.initialYaw += 180.f;
						}
						imu_pantile.initialPitch = imu_pantile.GetAnglePitch();
					}
					pc.prePressShift = true;
				}
				else
				{
					
					//ctrl.chassis.m_chassisMode = Control::Chassis::FOLLOW;
					if (pc.prePressShift)
					{
						imu_pantile.initialYaw = imu_pantile.GetAngleYaw();
						imu_pantile.initialPitch = imu_pantile.GetAnglePitch();
					}
					pc.setZ = 0;
					pc.prePressShift = false;
				}
			}
			else if (lock_mode)
			{
				if (pc.setX || pc.setY||first_enter_lock_mode)
				{

					if (abs(ctrl.pantile.midyaw - ctrl.pantile.pantile[0]->angle[now]) <= LOCK_MODE_ERROR)
					{
						ctrl.chassis.m_chassisMode = Control::Chassis::FOLLOW;
						pc.setZ = -1.f*pc.x * adjspeed;
						first_enter_lock_mode = false;
						ch_yaw = 0;
					}
					else
					{
						ctrl.chassis.m_chassisMode = Control::Chassis::LOCK;
						pc.setZ = -1.f*pc.x * 25+ctrl.keepChassisFollow();;//25 只是一个系数
					}
				}
				else
				{
					ctrl.chassis.m_chassisMode = Control::Chassis::FOLLOW;
					pc.setZ = 0;
				}
			}

			ctrl.manual_chassis(pc.setX, pc.setY, pc.setZ);
		}
		if (auto_aim)
		{
			ctrl.manual_pantile(pcGetMove(degreeToMechanical(imu_pantile.aim.x)), pcGetMove(degreeToMechanical(imu_pantile.aim.y)));
		}
		else
		{
			ctrl.manual_pantile(pcGetMove(ch_yaw), pcGetMove(ch_pitch));
		}
	}
	else if (rc.s[0] == down_position)
	{
		ctrl.manual_chassis(0, 0, 0);
		ctrl.pantile.setpitch = (ctrl.pantile.pitchmax + ctrl.pantile.pitchmin) / 2;
		ctrl.pantile.setyaw = ctrl.pantile.midyaw;
		ctrl.manual_shoot(false, false, false);
	}
}

float32_t RC::pcGetMove(int32_t change)
{
	float32_t re = 1.0;
	if (change == 0)
		return 0;
	if (change < 0)
	{
		re *= -1.0;
	}
	if (change <= 10 && change >= -10)
	{
		re *= 0.1;
	}
	else if ((change > 10 && change <= 50) || (change >= -50 && change < -10))
	{
		re *= 0.2;
	}
	else if ((change > 50 && change <= 150) || (change >= -150 && change < -50))
	{
		re *= 0.5;
	}
	else if (change < -150 || change>150)
	{
		re *= 1.0;
	}
	return re;
}


void RC::Update()
{
	OnRC();
	OnPC();
}

void RC::Init(UART* huart, uint32_t baud, USART_TypeDef* uart_base)
{
	huart->Init(uart_base, baud).DMARxInit(m_frame);
	int addr = function(0, this);
	huart->huart.pthis = reinterpret_cast<void*>(addr);
	addr = function(0, &RC::OnIRQHandler);
	huart->huart.callback = reinterpret_cast<void(*)(void*, uint16_t)>(addr);
	m_uart = huart;
}

extern IMU imu_pantile;
extern Android android;