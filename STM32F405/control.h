#ifndef __CONTROL__
#define __CONTROL__
#include <stm32f4xx_hal.h>
#include <vector>
#include "IMU.h"
#include "motor.h"
#include "tim.h"
#include <cmath>
#include "NUC.h"
//#include "judgement.h"
#include "Android.h"

float Ramp(float setval, float curval, float RampSlope = 1.f);
float getDelta(float delta);
class Control final
{
public:
	struct Chassis
	{
		enum ChassisMode
		{
			LOCK, FOLLOW, ROTATION
		};
		Motor* chassis[4];
		PID chassis_LOCK_PID{2.f,0.f,0.f};
		PID  power{30.f,200.f,2000.f};
		PID speed_limit_PID{0.0003f,0.f,0.f};
		Kalman now_chassis_power_kalman{ 1.f,30.f };
		double now_chassis_power = 0.f;
		int16_t chassis_buffer = 60, chassis_power_limit = 40;
		int16_t speedx = 0, speedy = 0, speedz = 0;
		bool openBuffer = false;
		ChassisMode m_chassisMode = FOLLOW;
		void Update();
		void PowerUpdate(int32_t* speedx, int32_t* speedy, int32_t* speedz);
	private:
	};
	struct Pantile
	{
		Motor* pantile[2];
		Android* android;
		PID pantile_PID[3] = { {0.04f,0.001f,0.f},{0.05f,0.f,0.f}, {0.04f,0.f,0.f} };
		const uint16_t midyaw = 5609;//yaw轴初始角度
		float pitchmax = 6400, pitchmin = 4750;//pitch轴限位
		float setpitch = (pitchmax + pitchmin) / 2, setyaw = midyaw;//pitch轴初始角度
		const float sensitivity = 2.5f;
		bool follow = true;
		bool pre_follow = false;
		bool aim = false;
		enum { YAW, PITCH };
		enum { TOP_USE_YAW,TOP_USE_PITCH, LOCK_USE };
		void Update();
	};
	struct Shooter
	{
		Motor* shooter[3];
		int16_t speed = 4275;//speed: 最速时电流大小
		int16_t setLeftMotorSpeed = 0, setRightMotorSpeed = 0;
		float now_bullet_speed = 0.f;
		float setHitMotor = 4096;
		bool openRub = false;
		bool shoot = false;
		bool auto_shoot = false;
		
		bool fraction = false;
		bool fullheat_shoot = false;
		bool heat_ulimit = false;

		void Update();
	private:
		uint32_t counter = 0;
	};

	Chassis chassis{};
	Pantile pantile{};
	Shooter shooter{};
	void follow_speed(int32_t* speedx, int32_t* speedy, int32_t* speedz);

	void manual_chassis(int32_t speedx, int32_t speedy, int32_t speedz = 0);

	void keepPantile(float angleKeep, float *which, IMU frameOfReference);
	//void keepPantile(float angleKeep, float *which, Pantile frameOfReference);
	int32_t keepChassisFollow();
	void manual_shoot(bool shoot, bool auto_shoot, bool openRub);
	void manual_pantile(float32_t ch_yaw, float32_t ch_pitch);//ch_yaw*sensitivity为改变量
	void Init(std::vector<Motor*> chassis, std::vector<Motor*> pantile, Android* android, std::vector<Motor*> shooter);
	//call me at 1khz
	void Update();
	static int16_t setrange(const int16_t original, const int16_t range);
	bool judgeShootHeat();
	/*
	 * 	static int16_t setrange(const int16_t original, const int16_t range);
	 *	this function is not called anywhere
	 *	2022/01/17
	 */
};

extern Control ctrl;
extern Judgement judgement;

#endif