#include "LWCore/LWLogger.h"
#include "LWCore/LWTimer.h"
#include "LWCore/LWUnicode.h"
#include <iostream>
#include <chrono>
#include <algorithm>
#include <ctime>
#include <cstdarg>
#include <algorithm>

uint32_t LWCurrentLogLevel = LWLOG_CRITICAL;
const uint32_t LWLoggerTimeMetrics::MaxValues;

void LWLoggerTimeMetrics::PushValue(uint64_t Value) {
	m_Values[m_ValueCount%MaxValues] = Value;
	m_ValueCount++;
	return;
}

float LWLoggerTimeMetrics::MakeDeltaTime(uint64_t lCurrentTime) {
	m_LastTime = std::min<uint64_t>(m_LastTime, lCurrentTime);
	uint64_t Delta = lCurrentTime - m_LastTime;
	m_LastTime = lCurrentTime;
	return LWTimer::ToSecond(Delta);
}

void LWLoggerTimeMetrics::RecordTimeStart(uint64_t lCurrentTime) {
	m_LastTime = lCurrentTime ? lCurrentTime : LWTimer::GetCurrent();
	return;
}

uint64_t LWLoggerTimeMetrics::RecordTime(void) {
	uint64_t Elapsed = LWTimer::GetCurrent() - m_LastTime;
	PushValue(Elapsed);
	return Elapsed;
}

std::ostream &operator<<(std::ostream &o, const LWLoggerTimeMetrics &DA) {
	uint64_t Lowest, Highest, Average;
	DA.GetValues(Lowest, Highest, Average);
	o << "L: " << Lowest << "ms H: " << Highest << "ms A: " << Average << "ms";
	return o;
}

uint64_t LWLoggerTimeMetrics::MakeFPS(void) const {
	if (!m_ValueCount) return 1000;
	uint64_t ValueCnt = std::min<uint64_t>(m_ValueCount, MaxValues);
	uint64_t Average = 0;
	for (uint32_t i = 0; i < ValueCnt; i++) Average += m_Values[i];
	Average /= ValueCnt;
	if (!Average) return 1000;
	return LWTimer::GetResolution() / Average;
}

void LWLoggerTimeMetrics::GetValues(uint64_t &Lowest, uint64_t &Highest, uint64_t &Average) const {
	if (!m_ValueCount) {
		Lowest = Highest = Average = 0;
		return;
	}
	uint32_t ValueCnt = std::min<uint32_t>(m_ValueCount, MaxValues);
	Average = m_Values[0];
	Lowest = Highest =  LWTimer::ToMilliSecond(m_Values[0]);
	for (uint32_t i = 1; i < ValueCnt; i++) {
		uint64_t Val = LWTimer::ToMilliSecond(m_Values[i]);
		Average += m_Values[i];
		Lowest = std::min<uint64_t>(Lowest, Val);
		Highest = std::max<uint64_t>(Highest, Val);
	}
	Average = LWTimer::ToMilliSecond(Average / ValueCnt);
	return;
}

