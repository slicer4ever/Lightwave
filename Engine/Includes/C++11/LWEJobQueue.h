#ifndef LWEJOBQUEUE_H
#define LWEJOBQUEUE_H
#include <LWCore/LWTypes.h>
#include <thread>
#include <functional>
#include <atomic>
#include "LWETypes.h"

struct LWEJob {

	std::function<void(LWEJob &, LWEJobThread &, LWEJobQueue &, uint64_t)> m_Func;
	void *m_UserData;
	uint64_t m_UpdateFrequency;
	uint64_t m_NextUpdate;
	uint64_t m_ElapsedTime;
	uint32_t m_LockIDs; //Flag for what bits of the lock flag to set when the job runs.
	uint32_t m_UnlockIDs; //Flag for what bits of the lock flag to remove when the job finishs.
	uint32_t m_LockedOutIDs; //Flag for what bits of the lock flag must not be set in order for the job to be startable.
	uint32_t m_LockedInIDs; //Flag for what bits of the lock flag must be set in order for the job to be startable.
	uint32_t m_ThreadLimit;
	uint32_t m_RunCount;
	uint32_t m_LoopCount;
	uint32_t m_JobIdx;

	template<class Y, class T>
	static LWEJob MakeMethod(Y Method, T Obj, void *UserData, uint64_t UpdateFrequency, uint32_t LockIDs = 0, uint32_t UnlockIDs = 0, uint32_t LockedOutIDs = 0, uint32_t LockedInIDs = 0, uint32_t LoopCount = 0, uint32_t ThreadLimit = 0xFFFFFFFF) {
		return LWEJob(std::bind(Method, Obj, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), UserData, UpdateFrequency, LockIDs, UnlockIDs, LockedOutIDs, LockedInIDs, LoopCount, ThreadLimit);
	}

	LWEJob(std::function<void(LWEJob &, LWEJobThread &, LWEJobQueue &, uint64_t)> Func, void *UserData, uint64_t UpdateFrequency, uint32_t LockIDs = 0, uint32_t UnlockIDs = 0, uint32_t LockedOutIDs = 0, uint32_t LockedInIDs = 0, uint32_t LoopCount = 0, uint32_t ThreadLimit = 0xFFFFFFFF);

	LWEJob();
};

struct LWEJobThread {
	std::thread m_Thread;
	uint64_t m_TimeInJobs = 0;
	uint32_t m_JobsRan = 0;
	uint32_t m_ThreadID;
};

class LWEJobQueue {
public:
	enum {
		MaxThreads = 32,
		MaxJobs = 64,

		Finished = 0x1,
		Paused = 0x2,
		AlwaysSleep = 0x4,
		Joined = 0x8,

		JobNull = 0,
		JobOpen = 1,
		JobRunning = 2
	};

	static void RunThread(LWEJobThread *Thread, LWEJobQueue *Queue);

	static void FlagJob(LWEJob &Job, LWEJobThread &Thread, LWEJobQueue &Queue, uint64_t lCurrentTime);

	bool PushJob(const LWEJob &Job);

	LWEJobQueue &RemoveJob(uint32_t Idx);

	LWEJob *NextJob(LWEJobThread &Thread, uint32_t &JobPos, uint64_t lCurrentTime);

	LWEJobQueue &FinishedJob(LWEJob &Job, uint64_t lELapsedTime);

	LWEJobQueue &ForceFinished(void);

	LWEJobQueue &OutputJobTimings(void);

	LWEJobQueue &OutputThreadTimings(void);

	LWEJobQueue &SetFinished(bool isFinished);

	LWEJobQueue &WaitForAllJoined(void);

	LWEJobQueue &Pause(void);

	LWEJobQueue &Start(void);

	LWEJobQueue &SetSleep(bool Sleep);

	LWEJobThread &GetMainThread(void);

	uint32_t GetFlag(void) const;

	uint32_t GetLockedFlag(void) const;
	
	uint32_t GetThreadCount(void) const;

	uint32_t GetJobCount(void) const;

	bool isFinished(void) const;

	bool isJoined(void) const;

	LWEJobQueue(uint32_t ThreadCnt = 0);

	~LWEJobQueue();
private:
	LWEJobThread m_Threads[MaxThreads];
	LWEJob m_Jobs[MaxJobs];
	std::atomic<uint32_t> m_JobState[MaxJobs];
	std::atomic<uint32_t> m_LockedFlag;
	std::atomic<uint32_t> m_JobCount;
	std::atomic<uint32_t> m_ReserveJobCount;
	uint32_t m_ThreadCount;
	uint32_t m_Flag;
};

#endif
