#include"PID.h"
PID::PID()
{
	m_Kp = 0.f;
	m_Ti = 0.f;
	m_Td = 0.f;
}

PID::PID(float Kp_High, float Ti_High, float Td_High, float alpha)
{
	 m_Kp = Kp_High;
	 m_Ti = Ti_High;
	 m_Td = Td_High;
	 m_alpha = alpha;
}
void PID::Adjust(float Kp, float Ti, float Td, float alpha)
{
	m_Kp = Kp;
	m_Ti = Ti;
	m_Td = Td;
	m_alpha = alpha;
}
float PID::Filter(float delta)
{
	float sum = 0;
	m_filter[m_filterindex++] = delta;
	if (m_filterindex == FILTER)m_filterindex = 0;
	for (int16_t t = 0; t != FILTER; t++)
		sum += m_filter[t];
	return sum / static_cast<float>(FILTER);
}
float PID::Delta(float error)//增量pid
{
	m_error[LLAST] = m_error[LAST] * 0.92f;
	m_error[LAST] = m_error[NOW] * 0.92f;
	m_error[NOW] = error * 1.08f;

	return m_Kp * (m_error[NOW] - m_error[LAST]) + m_Ti * m_error[NOW] + m_Td * (m_error[NOW] - 2 * m_error[LAST] + m_error[LLAST]);
}
float PID::Position(float error)//位置pid
{
	m_error[NOW] = error;
	m_error[INTEGRATE] += m_error[NOW];
	m_error[INTEGRATE] = std::max(std::min(m_error[INTEGRATE], 1000.f), -1000.f);
	//不完全微分
	m_lderivative = m_Td * (1.f - m_alpha) * (m_error[NOW] - m_error[LAST]) + m_alpha * m_lderivative;
	const float result = m_error[NOW] * m_Kp + m_error[INTEGRATE] * m_Ti + m_lderivative;
	m_error[LAST] = m_error[NOW];
	return result;
}