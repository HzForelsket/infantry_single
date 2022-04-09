#pragma once
#include "usart.h."
#include "control.h"
#include"core_cm4.h"
#include"label.h"
#include"iwdg.h"
#define mid_position  (3)
#define up_position  (1)
#define down_position  (2)

#define W     ((uint8_t)0x01<<0)
#define S     ((uint8_t)0x01<<1)
#define A     ((uint8_t)0x01<<2)
#define D     ((uint8_t)0x01<<3)
#define SHIFT ((uint8_t)0x01<<4)
#define CTRL  ((uint8_t)0x01<<5)
#define Q     ((uint8_t)0x01<<6)
#define E     ((uint8_t)0x01<<7)

#define R     ((uint8_t)0x01<<0)
#define F     ((uint8_t)0x01<<1)
#define G     ((uint8_t)0x01<<2)
#define Z     ((uint8_t)0x01<<3)
#define X     ((uint8_t)0x01<<4)
#define C     ((uint8_t)0x01<<5)

class RC
{
public:
	bool follow_mode = true;
	bool top_mode = false;
	bool lock_mode = false;
	bool auto_aim = false;
	bool fix = false;
	bool first_enter_lock_mode = false;
	float front_adjustment = 20;
	struct
	{
		bool fritIntoLock = true;
		int16_t ch[4];
		uint8_t s[2];
		Kalman ch_kalman[4] = { { 1.f, 1.f }, { 1.f, 1.f } ,{ 1.f, 1.f }, { 1.f, 1.f } };
	}rc;
	struct
	{
		int16_t x, y, z;
		uint8_t press_l, press_r, key_h, key_l;
		bool topmodeJudge = false;
		int8_t direct;
		int32_t setX, setY, setZ;
		bool prePressCtrl = 0;
		bool prePressShift = 0;
		bool prePress_L = 0;
		const float spdratio = 10.f;
		Kalman kalman[2] = { { 1.f, 20.f }, { 1.f, 20.f } }, KalmanRe[2] = { {1.f,30.f}, {1.f,30.f} };
	}pc;
	void OnRC();
	//pc.press_l：鼠标左键 pc.press_r：鼠标右键 pc.x:鼠标左右平移 pc.y:鼠标前后平移 pc.key_l，pc.key_h：按键
	void OnPC();
	void Update();
	void Init(UART* huart, uint32_t baud, USART_TypeDef* uart_base);
	float32_t pcGetMove(int32_t change);
private:
	void OnIRQHandler(size_t rxSize);
//	uint8_t* GetDMARx(void) { return m_frame; }
	UART* m_uart;
	uint8_t m_frame[UART_MAX_LEN]{};
};

extern RC rc;
extern MyIWDG m_iwdg;
extern Android android;
