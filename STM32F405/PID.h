#pragma once
#include "stm32f4xx_hal.h"
#include <algorithm>

enum { INTEGRATE = 0, LLAST = 0, LAST = 1, NOW = 2 };
#define FILTER 2
class PID
{
public:
	PID();
	PID(float Kp_High, float Ti_High, float Td_High, float alpha=0.f);
	void Adjust(float Kp, float Ti, float Td, float alpha);
	float Filter(float delta);
	float Delta(float error);
	float Position(float error);

	float m_Kp, m_Ti, m_Td;
	float m_error[3] = { 0 };
private:
	float m_alpha = 0.f, m_lderivative = 0.f;
	float m_filter[FILTER] = { 0 };
	uint16_t m_filterindex = 0;
};
