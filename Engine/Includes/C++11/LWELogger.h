#ifndef LWELOGGER_H
#define LWELOGGER_H
#include <LWCore/LWTypes.h>
#include <LWCore/LWUnicodeIterator.h>

#define LWELOG_EVENT 0
#define LWELOG_WARN 1
#define LWELOG_CRITICAL 2

struct LWELoggerTimeMetrics {
	static const uint32_t MaxValues = 120;
	uint64_t m_Values[MaxValues];
	uint64_t m_LastTime = -1;
	uint32_t m_ValueCount = 0;
	
	friend std::ostream &operator << (std::ostream &o, const LWELoggerTimeMetrics &DA);

	void PushValue(uint64_t Value);

	float MakeDeltaTime(uint64_t lCurrentTime);

	//Call in place of MakeDeltaTime if the deltaTime is not needed, if lCurrentTime is 0 then LWTimer::GetCurrent() will be called.
	void RecordTimeStart(uint64_t lCurrentTime = 0);

	//Call after RecordTimeStart, or MakeDeltaTime, returns the time recorded as well.
	uint64_t RecordTime(void);

	uint64_t MakeFPS(void) const;

	void GetValues(uint64_t &Lowest, uint64_t &Highest, uint64_t &Average) const;
};

void LWESetLogLevel(uint32_t Level);

void LWECreateLogFile(const LWUTF8Iterator &Path, LWAllocator &Allocator);

//Returns false.
bool LWELogEvent(const LWUTF8Iterator &Text);

//Returns false.
bool LWELogWarn(const LWUTF8Iterator &Text);

//Returns false.
bool LWELogCritical(const LWUTF8Iterator &Text);

template<uint32_t Len, typename ...Args>
bool LWELogEvent(const char8_t *Fmt, Args ...Pack) {
	return LWELogEvent(LWUTF8I::Fmt<Len>(Fmt, Pack...));
}

template<uint32_t Len, typename ...Args>
bool LWELogWarn(const char8_t *Fmt, Args ...Pack) {
	return LWELogWarn(LWUTF8I::Fmt<Len>(Fmt, Pack...));
}

template<uint32_t Len, typename ...Args>
bool LWELogCritical(const char8_t *Fmt, Args ...Pack) {
	return LWELogCritical(LWUTF8I::Fmt<Len>(Fmt, Pack...));
}

#endif