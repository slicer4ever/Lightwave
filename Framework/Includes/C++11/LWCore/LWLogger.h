#ifndef LWELOGGER_H
#define LWELOGGER_H
#include "LWCore/LWTypes.h"
#include "LWCore/LWUnicodeIterator.h"
#include "LWCore/LWAllocators/LWAllocator_Default.h"
#include <chrono>
#include <time.h>

#define LWLOG_EVENT 0 /*!< \brief event level log. */
#define LWLOG_WARN 1 /*!< \brief warning level log. */
#define LWLOG_CRITICAL 2 /*!< \brief critical level log. */

//Indicate which logging mechanism inside main.cpp with one of the following defines:
//Default logging to console only:
#define LWLOG_DEFAULT \
bool LWLog(uint32_t LogLevel, const LWUTF8Iterator &Text) { \
	if(LWCurrentLogLevel<LogLevel) return false; \
	auto ErrorMessage = LWLog_FormatString<4096>(LogLevel, Text);\
	fmt::print("{}", ErrorMessage);\
	return false;\
}

//Logging to single file(overwriting the file on every start), Must include (LWPlatform/LWFileStream) before this macro:
#define LWLOG_FILE(FilePath, bOutputConsole)\
bool LWLog(uint32_t LogLevel, const LWUTF8Iterator &Text) {\
	if(LWCurrentLogLevel<LogLevel) return false;\
	static LWAllocator_Default DefAlloc;\
	static bool bInitialized = false;\
	static bool bDidOpen=false;\
	static LWFileStream LogFile;\
	if(!bInitialized) {\
		bDidOpen = LWFileStream::OpenStream(LogFile, FilePath, LWFileStream::WriteMode, DefAlloc);\
		if(!bDidOpen) {\
			fmt::print("Error: Could not create log file '{}'\n", FilePath);\
		}\
		bInitialized=true;\
	}\
	auto ErrorMessage = LWLog_FormatString<4096>(LogLevel, Text);\
	if(bOutputConsole) fmt::print("{}", ErrorMessage);\
	if(bDidOpen) {\
		LogFile.Write(*ErrorMessage, ErrorMessage.m_DataLen-1);\
		LogFile.Flush();\
	}\
	return false;\
}

//Logging into multiple files, checks if n-FileCount exist, moving the files "down" 1 and deleting the last if it exceeds FileCount.
#define LWLOG_MULTIFILE(FilePath, bOutputConsole, FileCount) \
bool LWLog(uint32_t LogLevel, const LWUTF8Iterator &Text) { \
	if (LWCurrentLogLevel < LogLevel) return false; \
	static LWAllocator_Default DefAlloc; \
	static bool bInitialized = false; \
	static bool bDidOpen = false; \
	static LWFileStream LogFile; \
	if (!bInitialized) {\
		char8_t SourceFileName[256];\
		char8_t TargetFileName[256];\
		LWUTF8I::Fmt_n(SourceFileName, sizeof(SourceFileName), "{}_{}.txt", FilePath, FileCount-1);\
		if(LWFileStream::Exists(SourceFileName)) LWFileStream::DelFile(SourceFileName);\
		for(uint32_t i=FileCount-1;i>1;--i) { \
			LWUTF8I::Fmt_n(TargetFileName, sizeof(TargetFileName), "{}_{}.txt", FilePath, i); \
			LWUTF8I::Fmt_n(SourceFileName, sizeof(SourceFileName), "{}_{}.txt", FilePath, i-1); \
			if(LWFileStream::Exists(SourceFileName)) LWFileStream::MovFile(SourceFileName, TargetFileName);\
		} \
		LWUTF8I::Fmt_n(TargetFileName, sizeof(TargetFileName), "{}_{}.txt", FilePath, 1);\
		LWUTF8I::Fmt_n(SourceFileName, sizeof(SourceFileName), "{}.txt", FilePath);\
		if(LWFileStream::Exists(SourceFileName)) LWFileStream::MovFile(SourceFileName, TargetFileName);\
		bDidOpen = LWFileStream::OpenStream(LogFile, SourceFileName, LWFileStream::WriteMode, DefAlloc); \
		if (!bDidOpen) {\
			fmt::print("Error: Could not create log file '{}'\n", FilePath); \
		}\
		bInitialized = true; \
	}\
	auto ErrorMessage = LWLog_FormatString<4096>(LogLevel, Text); \
	if (bOutputConsole) fmt::print("{}", ErrorMessage); \
	if (bDidOpen) {\
		LogFile.Write(*ErrorMessage, ErrorMessage.m_DataLen - 1); \
		LogFile.Flush(); \
	}\
	return false; \
}

/*!< \brief Time metric logging between frames, logs a series of values(MaxValues) over time and can calculate the lowest frame, highest, and average over that time sampling. */
struct LWLoggerTimeMetrics {
	static const uint32_t MaxValues = 120;
	uint64_t m_Values[MaxValues];
	uint64_t m_LastTime = -1;
	uint32_t m_ValueCount = 0;
	
	friend std::ostream &operator << (std::ostream &o, const LWLoggerTimeMetrics &DA);

	void PushValue(uint64_t Value);

	float MakeDeltaTime(uint64_t lCurrentTime);

	//Call in place of MakeDeltaTime if the deltaTime is not needed, if lCurrentTime is 0 then LWTimer::GetCurrent() will be called.
	void RecordTimeStart(uint64_t lCurrentTime = 0);

	//Call after RecordTimeStart, or MakeDeltaTime, returns the time recorded as well.
	uint64_t RecordTime(void);

	uint64_t MakeFPS(void) const;

	void GetValues(uint64_t &Lowest, uint64_t &Highest, uint64_t &Average) const;
};

extern uint32_t LWCurrentLogLevel;

/*!< \brief constructs a formatted string with time stamp and error code. */
template<uint32_t Len>
inline LWUTF8I::C_View<Len> LWLog_FormatString(uint32_t LogLevel, const LWUTF8Iterator &LogText) {
	const char8_t *Levels[] = { "EVENT:", "WARN:", "CRIT:" };
	LogLevel = std::min<uint32_t>(LogLevel, 2);
	std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
	time_t Time = std::chrono::system_clock::to_time_t(tp);
	tm t;
#ifdef _MSC_VER //because people need to be weird :/
	localtime_s(&t, &Time);	
#else
	localtime_s(&Time, &t);
#endif
	return LWUTF8I::Fmt<Len>("[{}/{}/{} {}:{:02}:{:02}]{} {}\n", t.tm_mon + 1, t.tm_mday, (t.tm_year + 1900), t.tm_hour, t.tm_min, t.tm_sec, Levels[LogLevel], LogText);
}

//Log events, always returns false.
bool LWLog(uint32_t LogLevel, const LWUTF8Iterator &Text);

//Returns false. prints if LogLevel is set to allow event level logging.
inline bool LWLogEvent(const LWUTF8Iterator &Text) {
	return LWLog(LWLOG_EVENT, Text);
}

//returns false, prints Text only if bVerbose is set to true(and logging for events is enabled), otherwise nothing is outputted.
inline bool LWLogEventv(bool bVerbose, const LWUTF8Iterator &Text) {
	if(!bVerbose) return false;
	return LWLog(LWLOG_EVENT, Text);
}

//Returns true and logs nothing if bDidSucceed is true, otherwise returns false and prints text if log level is set to include event's.
inline bool LWLogEventIf(bool bDidSucceed, const LWUTF8Iterator &Text) {
	if(bDidSucceed) return bDidSucceed;
	return LWLog(LWLOG_EVENT, Text);
}

//Returns true and logs nothing if bDidSucceed is true, otherwise returns false and prints only if bVerbose is set to true.
inline bool LWLogEventIfv(bool bDidSucceed, bool bVerbose, const LWUTF8Iterator &Text) {
	if(bDidSucceed) return bDidSucceed;
	if(!bVerbose) return false;
	return LWLog(LWLOG_EVENT, Text);
}

//Returns false.
inline bool LWLogWarn(const LWUTF8Iterator &Text) {
	return LWLog(LWLOG_WARN, Text);
}

//returns false, prints Text only if bVerbose is set to true(and log level is set to allowing warnings).
inline bool LWLogWarnv(bool bVerbose, const LWUTF8Iterator &Text) {
	if(!bVerbose) return false;
	return LWLog(LWLOG_WARN, Text);
}

//returns bDidSucceed, and only prints if bDidSucceed is false, or if log level is set to allowing warnings).
inline bool LWLogWarnIf(bool bDidSucceed, const LWUTF8Iterator &Text) {
	if(bDidSucceed) return true;
	return LWLog(LWLOG_WARN, Text);
}

//returns bDidSucceed, and only prints if bDidSucceed is false and bVerbose is true(as well if log level is set to allow warnings).
inline bool LWLogWarnIfv(bool bDidSucceed, bool bVerbose, const LWUTF8Iterator &Text) {
	if(bDidSucceed) return true;
	if(!bVerbose) return false;
	return LWLog(LWLOG_WARN, Text);
}

//Returns false.
inline bool LWLogCritical(const LWUTF8Iterator &Text) {
	return LWLog(LWLOG_CRITICAL, Text);
}

//Returns false, only prints if bVerbose is set to true(and if log level is set to allow critical events).
inline bool LWLogCriticalv(bool bVerbose, const LWUTF8Iterator &Text) {
	if(!bVerbose) return false;
	return LWLog(LWLOG_CRITICAL, Text);
}

inline bool LWLogCriticalIf(bool bDidSucceeed, const LWUTF8Iterator &Text) {
	if(bDidSucceeed) return true;
	return LWLog(LWLOG_CRITICAL, Text);
}

inline bool LWLogCriticalIfv(bool bDidSucceeed, bool bVerbose, const LWUTF8Iterator &Text) {
	if (bDidSucceeed) return true;
	if(!bVerbose) return false;
	return LWLog(LWLOG_CRITICAL, Text);
}

template<uint32_t Len, typename ...Args>
inline bool LWLogEvent(const char8_t *Fmt, Args ...Pack) {
	return LWLog(LWLOG_EVENT, LWUTF8I::Fmt<Len>(Fmt, Pack...));
}

/*!< \brief compares In to Expected, if true returns true and print's nothing, if false attempts to print out in the following format: 'FuncName' Error: In */
template<uint32_t Len, class Type>
inline bool LWLogEventFunc(Type In, Type Expected, const LWUTF8Iterator &FuncName) {
	if(In==Expected) return true;
	return LWLog(LWLOG_EVENT, LWUTF8I::Fmt<Len>("'{}' Error: '{}'", FuncName, In));
}


/*!< \brief compares In to Expected, if true returns true and print's nothing, if false attempts to print out in the following format: 'FuncName' Error: In */
template<uint32_t Len, class Type>
inline bool LWLogEventFuncv(Type In, Type Expected, bool bVerbose, const LWUTF8Iterator &FuncName) {
	if (In == Expected) return true;
	if(!bVerbose) return false;
	return LWLog(LWLOG_EVENT, LWUTF8I::Fmt<Len>("'{}' Error: '{}'", FuncName, In));
}

template<uint32_t Len, typename ...Args>
inline bool LWLogEventv(bool bVerbose, const char8_t *Fmt, Args ...Pack) {
	if(!bVerbose) return false;
	return LWLog(LWLOG_EVENT, LWUTF8I::Fmt<Len>(Fmt, Pack...));
}

template<uint32_t Len, typename ...Args>
inline bool LWLogEventIf(bool bDidSucceed, const char8_t *Fmt, Args ...Pack) {
	if(bDidSucceed) return true;
	return LWLog(LWLOG_EVENT, LWUTF8I::Fmt<Len>(Fmt, Pack...));
}

template<uint32_t Len, typename ...Args>
inline bool LWLogEventIfv(bool bDidSucceed, bool bVerbose, const char8_t *Fmt, Args ...Pack) {
	if(bDidSucceed) return true;
	if(!bVerbose) return false;
	return LWLog(LWLOG_EVENT, LWUTF8I::Fmt<Len>(Fmt, Pack...));
}

/*!< \brief compares In to Expected, if true returns true and print's nothing, if false attempts to print out in the following format: 'FuncName' Error: In */
template<uint32_t Len, class Type>
inline bool LWLogWarnFunc(Type In, Type Expected, const LWUTF8Iterator &FuncName) {
	if (In == Expected) return true;
	return LWLog(LWLOG_WARN, LWUTF8I::Fmt<Len>("'{}' Error: '{}'", FuncName, In));
}


/*!< \brief compares In to Expected, if true returns true and print's nothing, if false attempts to print out in the following format: 'FuncName' Error: In */
template<uint32_t Len, class Type>
inline bool LWLogWarnFuncv(Type In, Type Expected, bool bVerbose, const LWUTF8Iterator &FuncName) {
	if (In == Expected) return true;
	if (!bVerbose) return false;
	return LWLog(LWLOG_WARN, LWUTF8I::Fmt<Len>("'{}' Error: '{}'", FuncName, In));
}

template<uint32_t Len, typename ...Args>
inline bool LWLogWarn(const char8_t *Fmt, Args ...Pack) {
	return LWLog(LWLOG_WARN, LWUTF8I::Fmt<Len>(Fmt, Pack...));
}

template<uint32_t Len, typename ...Args>
inline bool LWLogWarnv(bool bVerbose, const char8_t *Fmt, Args ...Pack) {
	if(!bVerbose) return false;
	return LWLog(LWLOG_WARN, LWUTF8I::Fmt<Len>(Fmt, Pack...));
}

template<uint32_t Len, typename ...Args>
inline bool LWLogWarnIf(bool bDidSucceed, const char8_t *Fmt, Args ...Pack) {
	if(bDidSucceed) return true;
	return LWLog(LWLOG_WARN, LWUTF8I::Fmt<Len>(Fmt, Pack...));
}

template<uint32_t Len, typename ...Args>
inline bool LWLogWarnIfv(bool bDidSucceed, bool bVerbose, const char8_t *Fmt, Args ...Pack) {
	if(bDidSucceed) return true;
	if(!bVerbose) return false;
	return LWLog(LWLOG_WARN, LWUTF8I::Fmt<Len>(Fmt, Pack...));
}

/*!< \brief compares In to Expected, if true returns true and print's nothing, if false attempts to print out in the following format: 'FuncName' Error: In */
template<uint32_t Len, class Type>
inline bool LWLogCriticalFunc(Type In, Type Expected, const LWUTF8Iterator &FuncName) {
	if (In == Expected) return true;
	return LWLog(LWLOG_CRITICAL, LWUTF8I::Fmt<Len>("'{}' Error: '{}'", FuncName, In));
}

/*!< \brief compares In to Expected, if true returns true and print's nothing, if false attempts to print out in the following format: 'FuncName' Error: In */
template<uint32_t Len, class Type>
inline bool LWLogCriticalFuncv(Type In, Type Expected, bool bVerbose, const LWUTF8Iterator &FuncName) {
	if (In == Expected) return true;
	if (!bVerbose) return false;
	return LWLog(LWLOG_CRITICAL, LWUTF8I::Fmt<Len>("'{}' Error: '{}'", FuncName, In));
}

template<uint32_t Len, typename ...Args>
inline bool LWLogCritical(const char8_t *Fmt, Args ...Pack) {
	return LWLog(LWLOG_CRITICAL, LWUTF8I::Fmt<Len>(Fmt, Pack...));
}

template<uint32_t Len, typename ...Args>
inline bool LWLogCriticalv(bool bVerbose, const char8_t *Fmt, Args ...Pack) {
	if(!bVerbose) return false;
	return LWLog(LWLOG_CRITICAL, LWUTF8I::Fmt<Len>(Fmt, Pack...));
}

template<uint32_t Len, typename ...Args>
inline bool LWLogCriticalIf(bool bDidSucceed, const char8_t *Fmt, Args ...Pack) {
	if(bDidSucceed) return true;
	return LWLog(LWLOG_CRITICAL, LWUTF8I::Fmt<Len>(Fmt, Pack...));
}

template<uint32_t Len, typename ...Args>
inline bool LWLogCriticalIfv(bool bDidSucceed, bool bVerbose, const char8_t *Fmt, Args ...Pack) {
	if(bDidSucceed) return true;
	if(!bVerbose) return false;
	return LWLog(LWLOG_CRITICAL, LWUTF8I::Fmt<Len>(Fmt, Pack...));
}

#endif