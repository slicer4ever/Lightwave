#include "LWCore/LWTimer.h"
#include <chrono>


uint64_t LWTimer::GetCurrent(void){
	return std::chrono::high_resolution_clock::now().time_since_epoch().count();	
}

uint64_t LWTimer::GetResolution(void){
	return std::chrono::high_resolution_clock::period().den;
}

LWTimer &LWTimer::SetFrequency(uint64_t Frequency){
	m_Frequency = Frequency;
	m_Flag = (m_Flag&Running) | (m_DeltaTicks >= m_Frequency ? Completed : 0);
	return *this;
}

float LWTimer::ToSecond(uint64_t Time) {
	return (float)Time / GetResolution();
}

uint64_t LWTimer::ToMilliSecond(uint64_t Time) {	
	std::chrono::high_resolution_clock::duration c(Time);
	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(c);
	return ms.count();
}

uint64_t LWTimer::ToHighResolution(uint64_t Time) {
	std::chrono::milliseconds ms(Time);
	std::chrono::high_resolution_clock::duration c = std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(ms);
	return c.count();
}

uint64_t LWTimer::ToHighResolution(float Time) {
	return (uint64_t)(Time*GetResolution());
}

LWTimer &LWTimer::Play(uint64_t Current){
	if ((m_Flag&Running)) return *this;
	m_PrevTick = Current;
	m_Flag |= Running;
	return *this;
}

LWTimer &LWTimer::Pause(void){
	m_Flag &= ~Running;
	return *this;
}

LWTimer &LWTimer::ForceReset(uint64_t Current){
	m_PrevTick = Current;
	m_DeltaTicks = 0;
	m_Flag = (m_Flag&Running) | (m_DeltaTicks >= m_Frequency ? Completed : 0);
	return *this;
}

LWTimer &LWTimer::Reset(void){
	m_DeltaTicks = m_DeltaTicks >= m_Frequency ? (m_DeltaTicks - m_Frequency) : 0;
	m_Flag = (m_Flag&Running) | (m_DeltaTicks >= m_Frequency ? Completed : 0);
	return *this;
}

LWTimer &LWTimer::Update(uint64_t Current){
	if (!(m_Flag&Running)) return *this;
	m_DeltaTicks += (m_PrevTick>Current)?0:(Current - m_PrevTick);
	m_Flag = (m_Flag&Running) | (m_DeltaTicks >= m_Frequency ? Completed : 0);
	m_PrevTick = Current;
	return *this;
}

bool LWTimer::isCompleted(void) const {
	return (m_Flag & Completed) != 0;
}

uint64_t LWTimer::GetFrequency(void){
	return m_Frequency;
}

uint64_t LWTimer::GetTimeLeft(void){
	return m_DeltaTicks >= m_Frequency ? 0 : m_Frequency - m_DeltaTicks;
}

uint8_t LWTimer::GetFlag(void){
	return m_Flag;
}

LWTimer::LWTimer(uint64_t Frequency, uint8_t Flag) : m_PrevTick(GetCurrent()), m_Frequency(Frequency), m_DeltaTicks(0), m_Flag(Flag){
	m_Flag = (Flag&Running) | (m_DeltaTicks >= m_Frequency ? Completed : 0);
}