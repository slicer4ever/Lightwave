#include "LWEJobQueue.h"
#include <LWCore/LWTimer.h>
#include <LWCore/LWLogger.h>
#include <algorithm>
#include <iostream>

LWEJob::LWEJob(std::function<void(LWEJob &, LWEJobThread &, LWEJobQueue &, uint64_t)> Func, void *UserData, uint64_t UpdateFrequency, uint32_t LockIDs, uint32_t UnlockIDs, uint32_t LockedOutIDs, uint32_t LockedInIDs, uint32_t LoopCount, uint32_t ThreadLimit) {
	m_Func = Func;
	m_UserData = UserData;
	m_LockIDs = LockIDs;
	m_UnlockIDs = UnlockIDs;
	m_LockedOutIDs = LockedOutIDs;
	m_LockedInIDs = LockedInIDs;
	m_ThreadLimit = ThreadLimit;
	m_LoopCount = LoopCount;
	m_UpdateFrequency = UpdateFrequency;
	m_NextUpdate = 0;
	m_RunCount = 0;
	m_ElapsedTime = 0;
	m_JobIdx = 0;
}

LWEJob::LWEJob() {}

void LWEJobQueue::RunThread(LWEJobThread *Thread, LWEJobQueue *Queue) {
	uint32_t Flag = Queue->GetFlag();
	uint32_t JobIdx = 0;
	uint32_t JobSleepCnt = 0;
	while(!Queue->isFinished()){
		Flag = Queue->GetFlag();
		if (Flag&LWEJobQueue::Paused) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			JobSleepCnt = 0;
			continue;
		}
		uint64_t CurrentTime = LWTimer::GetCurrent();
		LWEJob *J = Queue->NextJob(*Thread, JobIdx, CurrentTime);
		if (!J) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			JobSleepCnt = 0;
			continue;
		}
		CurrentTime = LWTimer::GetCurrent();
		J->m_Func(*J, *Thread, *Queue, CurrentTime);
		uint64_t Elasped = LWTimer::GetCurrent() - CurrentTime;
		Thread->m_TimeInJobs += Elasped;
		Thread->m_JobsRan++;
		Queue->FinishedJob(*J, Elasped);
		JobSleepCnt++;
		if (Flag&LWEJobQueue::AlwaysSleep && JobSleepCnt >= Queue->GetJobCount()) {
			JobSleepCnt = 0;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
}

void LWEJobQueue::FlagJob(LWEJob &Job, LWEJobThread &Thread, LWEJobQueue &Queue, uint64_t lCurrentTime) {
	return;
}

bool LWEJobQueue::PushJob(const LWEJob &Job) {
	//Check if there's an open job.
	uint32_t JC = m_JobCount.load();
	uint32_t i = 0;
	bool Inserted = false;
	for (; i < JC; i++) { 
		uint32_t Exp = (uint32_t)JobNull;
		if (m_JobState[i].compare_exchange_weak(Exp, (uint32_t)JobRunning)) break;
	}
	if (i < JC) {
		m_Jobs[i] = Job;
		m_Jobs[i].m_JobIdx = i;
		m_JobState[i].store((uint32_t)JobOpen);
		return true;
	}
	JC = m_ReserveJobCount.load();
	do {
		if (JC >= MaxJobs) return false;
		Inserted = m_ReserveJobCount.compare_exchange_weak(JC, JC + 1);
	} while (!Inserted);
	m_Jobs[JC] = Job;
	m_Jobs[JC].m_JobIdx = JC;
	m_JobState[JC].store((uint32_t)JobOpen);
	uint32_t J = m_JobCount.load();
	while (J < JC+1) {
		if (m_JobCount.compare_exchange_weak(J, J + 1)) J++;
	}
	return true;
}

LWEJobQueue &LWEJobQueue::RemoveJob(uint32_t Idx) {
	uint32_t Exp = (uint32_t)JobOpen;
	if (m_JobState[Idx].compare_exchange_weak(Exp, (uint32_t)JobRunning)) {
		m_JobState[Idx].store((uint32_t)JobNull);
		return *this;
	}
	m_Jobs[Idx].m_LoopCount = 1;
	return *this;
}

LWEJob *LWEJobQueue::NextJob(LWEJobThread &Thread, uint32_t &JobPos, uint64_t lCurrentTime) {
	uint32_t JobCnt = m_JobCount.load();
	for (uint32_t i = 0; i < JobCnt; i++) {
		uint32_t n = (i + JobPos) % JobCnt;
		uint32_t Exp = (uint32_t)JobOpen;
		if (!m_JobState[n].compare_exchange_weak(Exp, (uint32_t)JobRunning)) continue;
		bool Valid = true;
		Valid = Valid ? m_Jobs[n].m_NextUpdate < lCurrentTime : Valid;
		Valid = Valid ? (m_Jobs[n].m_ThreadLimit&Thread.m_ThreadID) != 0 : Valid;
		if (!Valid) {
			m_JobState[n].store((uint32_t)JobOpen);
			continue;
		}
		uint32_t LockFlag = m_LockedFlag.load();
		Valid = Valid ? (m_Jobs[n].m_LockedOutIDs&LockFlag) == 0 : Valid;
		Valid = Valid ? (m_Jobs[n].m_LockedInIDs&LockFlag) == m_Jobs[n].m_LockedInIDs : Valid;
		if (!Valid) {
			m_JobState[n].store((uint32_t)JobOpen);
			continue;
		}
		if (m_Jobs[n].m_LockIDs) {
			if (!m_LockedFlag.compare_exchange_weak(LockFlag, LockFlag | m_Jobs[n].m_LockIDs)) {
				m_JobState[n].store((uint32_t)JobOpen);
				continue;
			}
		}
		if (!m_Jobs[n].m_NextUpdate) m_Jobs[n].m_NextUpdate = lCurrentTime;
		m_Jobs[n].m_NextUpdate += m_Jobs[n].m_UpdateFrequency;
		JobPos = (n + 1) % JobCnt;
		return m_Jobs + n;
	}
	JobPos = 0;
	return nullptr;
}

LWEJobQueue &LWEJobQueue::FinishedJob(LWEJob &Job, uint64_t lElapsedTime) {
	Job.m_RunCount++;
	Job.m_ElapsedTime += lElapsedTime;
	uint32_t Flag = m_LockedFlag.load();
	if (Job.m_UnlockIDs) {
		while (!m_LockedFlag.compare_exchange_weak(Flag, Flag^Job.m_UnlockIDs)) {}
	}
	m_JobState[Job.m_JobIdx].store((!Job.m_LoopCount || Job.m_RunCount < Job.m_LoopCount) ? (uint32_t)JobOpen : (uint32_t)JobNull);
	return *this;
}

LWEJobQueue &LWEJobQueue::ForceFinished(void) {
	m_Flag |= Finished;
	for (uint32_t i = 1; i < m_ThreadCount; i++) {
		m_Threads[i].m_Thread.join();
	}
	m_ThreadCount = 0;
	return *this;
}

LWEJobQueue &LWEJobQueue::OutputJobTimings(void){
	for (uint32_t i = 0; i < m_JobCount; i++) {
		if (m_JobState[i].load() == (uint32_t)JobNull) continue;
		uint64_t Average = m_Jobs[i].m_RunCount ? m_Jobs[i].m_ElapsedTime / m_Jobs[i].m_RunCount : 0;
		LWLogEvent<256>("Job {}: Avg: {}ms Total: {}ms Times ran: {}", i, LWTimer::ToMilliSecond(Average), LWTimer::ToMilliSecond(m_Jobs[i].m_ElapsedTime), m_Jobs[i].m_RunCount);
	}
	return *this;
}

LWEJobQueue &LWEJobQueue::OutputThreadTimings(void) {
	for (uint32_t i = 0; i < m_ThreadCount; i++) {
		uint64_t Average = m_Threads[i].m_JobsRan ? m_Threads[i].m_TimeInJobs / m_Threads[i].m_JobsRan : 0;
		LWLogEvent<256>("JThread {}: Avg: {}ms Total: {}ms Jobs ran: {}", i, LWTimer::ToMilliSecond(Average), LWTimer::ToMilliSecond(m_Threads[i].m_TimeInJobs), m_Threads[i].m_JobsRan);
	}
	return *this;
}

LWEJobQueue &LWEJobQueue::SetFinished(bool isFinished) {
	m_Flag |= isFinished ? Finished : 0;
	return *this;
}

LWEJobQueue &LWEJobQueue::WaitForAllJoined(void) {
	if (isJoined()) return *this;
	for (uint32_t i = 1; i < m_ThreadCount; i++) m_Threads[i].m_Thread.join();
	m_Flag |= Joined;
	return *this;
}

LWEJobQueue &LWEJobQueue::Pause(void) {
	m_Flag |= Paused;
	return *this;
}

LWEJobQueue &LWEJobQueue::Start(void) {
	m_Flag &= ~Paused;
	return *this;
}

LWEJobQueue &LWEJobQueue::SetSleep(bool Sleep) {
	m_Flag = (m_Flag&~AlwaysSleep) | (Sleep ? AlwaysSleep : 0);
	return *this;
}

LWEJobThread &LWEJobQueue::GetMainThread(void) {
	return m_Threads[0];
}

uint32_t LWEJobQueue::GetFlag(void) const {
	return m_Flag;
}

uint32_t LWEJobQueue::GetLockedFlag(void) const {
	return m_LockedFlag;
}

uint32_t LWEJobQueue::GetThreadCount(void) const {
	return m_ThreadCount;
}

uint32_t LWEJobQueue::GetJobCount(void) const {
	return m_JobCount;
}

bool LWEJobQueue::isFinished(void) const {
	return (m_Flag&Finished) != 0;
}

bool LWEJobQueue::isJoined(void) const {
	return (m_Flag&Joined) != 0;
}

LWEJobQueue::LWEJobQueue(uint32_t ThreadCnt) : m_ThreadCount(0), m_JobCount(0), m_LockedFlag(0), m_ReserveJobCount(0), m_Flag(Paused) {
	for (uint32_t i = 0; i < MaxJobs; i++) m_JobState[i].store(JobNull);
	if (!ThreadCnt) ThreadCnt = std::thread::hardware_concurrency()-1;
	ThreadCnt = std::min<uint32_t>(ThreadCnt, MaxThreads-1);
	m_ThreadCount = ThreadCnt + 1;
	m_Threads[0].m_ThreadID = 0x1;
	for (uint32_t i = 1; i < m_ThreadCount; i++) {
		m_Threads[i].m_ThreadID = (1 << i);
		m_Threads[i].m_Thread = std::thread(RunThread, m_Threads+i, this);
	}
}

LWEJobQueue::~LWEJobQueue() {
	m_Flag |= Finished;
	WaitForAllJoined();
}
