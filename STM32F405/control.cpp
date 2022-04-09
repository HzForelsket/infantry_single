#include"control.h"
#include"label.h"
void Control::Init(std::vector<Motor*> chassis, std::vector<Motor*> pantile, Android* android, std::vector<Motor*> shooter)
{
	this->chassis.m_chassisMode = Control::Chassis::FOLLOW;
	for (size_t i = 0; i < chassis.size(); ++i)
	{
		this->chassis.chassis[i] = chassis[i];
		switch (chassis[i]->m_motor_use)
		{
		case chassisMotor:
			chassis[i]->setspeed = 0;
			chassis[i]->current = 0;
			break;
		default:
			break;
		}
	}
	for (size_t i = 0; i < pantile.size(); ++i)
	{
		this->pantile.pantile[i] = pantile[i];
		switch (pantile[i]->m_motor_use)
		{
		case pitchMotor:
			(pantile[i])->setangle = ((this->pantile).pitchmax + (this->pantile).pitchmin) / 2;
			break;
		case yawMotor:
			(pantile[i])->setangle = (this->pantile).setyaw;
			break;
		default:
			break;
		}
	}
	for (size_t i = 0; i < shooter.size(); ++i)
	{
		this->shooter.shooter[i] = shooter[i];
		switch (shooter[i]->m_motor_use)
		{
		case hitMotor:
			shooter[i]->setangle = this->shooter.setHitMotor;
			shooter[i]->spinning = false;
			shooter[i]->need_curcircle = 0;
			shooter[i]->stopAngle = this->shooter.setHitMotor;
			break;
		case rubMotor:
			shooter[i]->setangle == this->shooter.setLeftMotorSpeed;
			break;
		default:
			break;
		}

	}
	this->pantile.android = android;
}
//call me at 1khz

void Control::follow_speed(int32_t* speedx, int32_t* speedy, int32_t* speedz)//使云台方向为行进的正方向
{
	double s_x = (*speedx), s_y = (*speedy);
	double theat = PI * getDelta(mechanicalToDegree(pantile.pantile[0]->angle[now]) - mechanicalToDegree(pantile.midyaw))/180.f;
	double st = sin(theat);
	double ct = cos(theat);
	(*speedx) = s_x * ct - s_y * st;
	(*speedy) = s_x * st + s_y * ct;
}

void Control::manual_chassis(int32_t speedx, int32_t speedy, int32_t speedz)
{
	follow_speed(&speedx, &speedy, &speedz);
	int32_t setX, setY, setZ;
	setX = speedx;
	setY = speedy; 
	setZ = speedz;
	chassis.PowerUpdate(&setX, &setY, &setZ);
	this->chassis.speedx = setX;
	this->chassis.speedy = setY;
	this->chassis.speedz = setZ;
}

void Control::manual_pantile(float32_t ch_yaw, float32_t ch_pitch)
{
	ch_pitch *= (1.f);
	ch_yaw *= (1.f);//方向相反修改这里正负
	float adjangle = this->pantile.sensitivity * 2;
	if (this->chassis.m_chassisMode == Control::Chassis::FOLLOW)
	{
		imu_pantile.initialYaw = imu_pantile.GetAngleYaw();
		imu_pantile.initialPitch = imu_pantile.GetAnglePitch();
		//imu_pantile.initialYaw = getDelta(imu_pantile.initialYaw - (adjangle * ch_yaw / 22.75f));
		//imu_pantile.initialPitch = getDelta(MIN(MAX(imu_pantile.initialPitch - (adjangle * ch_pitch / 22.75), -27.f), 43.f));
		this->pantile.setpitch -= (float)(adjangle * ch_pitch);
		this->pantile.setyaw -= (float)(adjangle * ch_yaw);
		//keepPantile(imu_pantile.initialYaw, &(pantile.setyaw), imu_pantile);

	}
	else if (this->chassis.m_chassisMode == Control::Chassis::ROTATION)
	{
		imu_pantile.initialYaw = getDelta(imu_pantile.initialYaw - (adjangle * ch_yaw / 22.75f));
		imu_pantile.initialPitch = getDelta(MIN(MAX( imu_pantile.initialPitch - (adjangle * ch_pitch / 22.75),-27.f),45.f));
		keepPantile(imu_pantile.initialYaw, &(pantile.setyaw), imu_pantile);
		keepPantile(imu_pantile.initialPitch, &(pantile.setpitch), imu_pantile);
	}
	else if (this->chassis.m_chassisMode == Control::Chassis::LOCK)
	{
		imu_pantile.initialYaw = getDelta(imu_pantile.initialYaw - (adjangle * ch_yaw / 22.75f));
		imu_pantile.initialPitch = getDelta(MIN(MAX(imu_pantile.initialPitch - (adjangle * ch_pitch / 22.75), -27.f), 45.f));
		keepPantile(imu_pantile.initialYaw, &(pantile.setyaw), imu_pantile);
		this->pantile.setpitch -= (float)(adjangle * ch_pitch);
	}
}
int32_t Control::keepChassisFollow()
{
	if (this->chassis.m_chassisMode == Control::Chassis::LOCK)
	{
		float theat = pantile.midyaw - pantile.pantile[0]->angle[now];
		if (theat <= -4096.f)
		{
			theat += 8192.f;
		}
		else if (theat >= 4096.f)
		{
			theat -= 8192;
		}
		if (abs(theat) >= LOCK_MODE_ERROR/2.f)
			return -chassis.chassis_LOCK_PID.Filter(chassis.chassis_LOCK_PID.Position(theat));
			//return -chassis.chassis_LOCK_PID.Position(theat);
	}
}
void Control::manual_shoot(bool shoot, bool auto_shoot, bool openRub)
{
	if (judgeShootHeat())
	{
	this->shooter.shoot = shoot;
	this->shooter.auto_shoot = auto_shoot;
	}
	this->shooter.openRub = openRub;
}

void Control::Chassis::Update()
{
	chassis[0]->setspeed = Ramp(+speedx + speedy - speedz, chassis[0]->setspeed);
	chassis[1]->setspeed = Ramp(-1 * (-speedx + speedy + speedz), chassis[1]->setspeed);
	chassis[2]->setspeed = Ramp(-1 * (+speedx + speedy + speedz), chassis[2]->setspeed);
	chassis[3]->setspeed = Ramp(-speedx + speedy - speedz, chassis[3]->setspeed);
	/*
	* the value of speedx,speedy and speedz are determined by function manual_chassis which is defined in control.h
	* the function of OnRC which is defined in RC.h updates the value
	*
	*/
}
void Control::Pantile::Update()
{
	if (setyaw > 8192.0)setyaw -= 8192.0;
	if (setyaw < 0.0)setyaw += 8192.0;

	setpitch = std::max(std::min(setpitch, pitchmax), pitchmin);
	int16_t anglepitch = setpitch, angleyaw = setyaw;
	if (angleyaw >= 8192)angleyaw -= 8192.0;
	if (angleyaw <= 0)angleyaw += 8192.0;

	if (anglepitch <= pitchmin)anglepitch = pitchmin;
	if (anglepitch >= pitchmax)anglepitch = pitchmax;

	//输出

	pantile[0]->setangle = angleyaw;
	pantile[1]->setangle = anglepitch;
}
void Control::Shooter::Update()
{
	now_bullet_speed = judgement.data.ext_shoot_data_t.bullet_speed;

	switch (judgement.data.ext_game_robot_status_t.shooter_id1_17mm_speed_limit)
	{
	case 15:
		speed = 4250;
		break;
	case 18:
		speed = 4800;
		break;
	case 30:
		speed = 8000;
		break;
	default:break;
	}
	if (openRub)
	{
		shooter[0]->setspeed = -speed;
		shooter[1]->setspeed = speed;
	}
	else
	{
		shooter[0]->setspeed = 0;
		shooter[1]->setspeed = 0;
	}

	if (auto_shoot && (!shoot))
	{
		shooter[2]->setspeed = 2160;
		shooter[2]->spinning = true;
		shooter[2]->pd = false;
	}
	else if ((!auto_shoot) && shoot)
	{
		shooter[2]->need_curcircle+=4;
		shooter[2]->setspeed = 500;
		shooter[2]->spinning = false;
		shooter[2]->pd = true;
	}
	else if ((!shoot) && (!auto_shoot))
	{
		//shooter[2]->setspeed = 0;
		shooter[2]->spinning = false;
		shooter[2]->pd = false;
	}
}
void Control::Update()
{
	chassis.Update();
	pantile.Update();
	shooter.Update();
}
void Control::keepPantile(float angleKeep, float *which, IMU frameOfReference)
{
	float delta = 0, adjust = this->pantile.sensitivity;
		if (which == (&(pantile.setyaw)))
		{
			delta = degreeToMechanical(getDelta(angleKeep - frameOfReference.GetAngleYaw()));
			if (delta <= -4096.f)
				delta += 8192.f;
			else if (delta >= 4096.f)
				delta -= 8291.f;
			if (abs(delta) >= 10.f)
			(*(which)) += pantile.pantile_PID[Pantile::TOP_USE_YAW].Delta(delta);
		}
		else if (which == (&(pantile.setpitch)))
		{
			delta = degreeToMechanical(getDelta(angleKeep - frameOfReference.GetAnglePitch()));		
			if (delta <= -4096.f)
				delta += 8192.f;
			else if (delta >= 4096.f)
				delta -= 8291.f;
		
			if (abs(delta) >= 10.f)
			{
				(*(which)) += pantile.pantile_PID[Pantile::TOP_USE_PITCH].Delta(delta);
			}
		}
}

int16_t Control::setrange(const int16_t original, const int16_t range)
{
	return fmaxf(fminf(range, original), -range);
}
bool Control::judgeShootHeat()
{
	int16_t limit = judgement.data.ext_game_robot_status_t.shooter_id1_17mm_cooling_limit;
	int16_t nowHeat = judgement.data.ext_power_heat_data_t.shooter_id1_17mm_cooling_heat;
	int16_t rate = judgement.data.ext_game_robot_status_t.shooter_id1_17mm_cooling_rate;
	if (limit - nowHeat >= ONE_BULLET_HEAT||this->shooter.heat_ulimit)
		return true;
	return false;
}
/*
 * 	static int16_t setrange(const int16_t original, const int16_t range);
 *	this function is not called anywhere
 *	2022/01/17
 */
void Control::Chassis::PowerUpdate(int32_t* speedx, int32_t* speedy, int32_t* speedz)
{
	chassis_power_limit = judgement.data.ext_game_robot_status_t.chassis_power_limit;
	chassis_buffer = judgement.data.ext_power_heat_data_t.chassis_power_buffer;
	now_chassis_power = now_chassis_power_kalman.Filter(judgement.data.ext_power_heat_data_t.chassis_power);
	static double del_used_power_limit = 0.0;
	double error;
	error = chassis_power_limit - now_chassis_power;
	if ((*speedx) || (*speedy) || (*speedz))
		del_used_power_limit = MIN(MAX(del_used_power_limit + speed_limit_PID.Delta(error), 0.0), 5.0);
	else
		del_used_power_limit = 0;
	(*speedx) = -(*speedx) * del_used_power_limit;
	(*speedy) = -(*speedy) * del_used_power_limit;
	(*speedz) = (*speedz) * del_used_power_limit;
}
float getDelta(float delta)
{
	if (delta <= -180.f)
	{
		delta += 360.f;
	}

	if (delta > 180.f)
	{
		delta -= 360.f;
	}
	return delta;
}
float Ramp(float setval, float curval, float RampSlope)
{
	RampSlope *= 20.f;
	if ((setval - curval) >= 0)
	{
		curval += RampSlope;
		curval = std::min(curval, setval);
	}
	else
	{
		curval -= RampSlope;
		curval = std::max(curval, setval);
	}
	return curval;
}



