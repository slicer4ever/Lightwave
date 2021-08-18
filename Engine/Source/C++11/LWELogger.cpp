#include "LWELogger.h"
#include <LWCore/LWTimer.h>
#include <LWCore/LWUnicode.h>
#include <LWPlatform/LWFileStream.h>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <ctime>
#include <cstdarg>
#include <algorithm>

void LWELoggerTimeMetrics::PushValue(uint64_t Value) {
	m_Values[m_ValueCount%MaxValues] = Value;
	m_ValueCount++;
	return;
}

float LWELoggerTimeMetrics::MakeDeltaTime(uint64_t lCurrentTime) {
	m_LastTime = std::min<uint64_t>(m_LastTime, lCurrentTime);
	uint64_t Delta = lCurrentTime - m_LastTime;
	m_LastTime = lCurrentTime;
	return LWTimer::ToSecond(Delta);
}

void LWELoggerTimeMetrics::RecordTimeStart(uint64_t lCurrentTime) {
	m_LastTime = lCurrentTime ? lCurrentTime : LWTimer::GetCurrent();
	return;
}

uint64_t LWELoggerTimeMetrics::RecordTime(void) {
	uint64_t Elapsed = LWTimer::GetCurrent() - m_LastTime;
	PushValue(Elapsed);
	return Elapsed;
}

std::ostream &operator<<(std::ostream &o, const LWELoggerTimeMetrics &DA) {
	uint64_t Lowest, Highest, Average;
	DA.GetValues(Lowest, Highest, Average);
	o << "L: " << Lowest << "ms H: " << Highest << "ms A: " << Average << "ms";
	return o;
}

uint64_t LWELoggerTimeMetrics::MakeFPS(void) const {
	if (!m_ValueCount) return 1000;
	uint64_t ValueCnt = std::min<uint64_t>(m_ValueCount, MaxValues);
	uint64_t Average = 0;
	for (uint32_t i = 0; i < ValueCnt; i++) Average += m_Values[i];
	Average /= ValueCnt;
	if (!Average) return 1000;
	return LWTimer::GetResolution() / Average;
}

void LWELoggerTimeMetrics::GetValues(uint64_t &Lowest, uint64_t &Highest, uint64_t &Average) const {
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

uint32_t LWECurrentLogLevel = LWELOG_CRITICAL;
LWFileStream LWELogFileStream;
bool LWELogFileOpen = false;
void LWESetLogLevel(uint32_t Level) {
	LWECurrentLogLevel = Level;
	return;
}

void LWECreateLogFile(const LWUTF8Iterator &Path, LWAllocator &Allocator) {
	if (LWELogFileOpen) return;
	LWELogFileOpen = LWFileStream::OpenStream(LWELogFileStream, Path, LWFileStream::WriteMode | LWFileStream::BinaryMode, Allocator);
	return;
}

bool LWELog(const LWUTF8Iterator &Text, uint32_t LogLevel) {
	if (LWECurrentLogLevel < LogLevel) return false;
	const char8_t *Log[] = { "EVENT:", "WARN:", "CRIT:" };
	LogLevel = std::min<uint32_t>(LogLevel, 2);
	std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
	time_t Time = std::chrono::system_clock::to_time_t(tp);
	tm t;
	localtime_s(&t, &Time);
	auto ErrorMessage = LWUTF8I::Fmt<2048>("[{}/{}/{} {}:{:02}:{:02}]{} {}\n", t.tm_mon + 1, t.tm_mday, (t.tm_year + 1900), t.tm_hour, t.tm_min, t.tm_sec, Log[LogLevel], Text);
	fmt::print("{}", ErrorMessage);
	if (LWELogFileOpen) {
		LWELogFileStream.Write(*ErrorMessage, ErrorMessage.m_DataLen - 1);
		LWELogFileStream.Flush();
	}
	return false;
}

bool LWELogEvent(const LWUTF8Iterator &Text) {
	return LWELog(Text, LWELOG_EVENT);
}

bool LWELogWarn(const LWUTF8Iterator &Text) {
	return LWELog(Text, LWELOG_WARN);
}

bool LWELogCritical(const LWUTF8Iterator &Text) {
	return LWELog(Text, LWELOG_CRITICAL);
}
