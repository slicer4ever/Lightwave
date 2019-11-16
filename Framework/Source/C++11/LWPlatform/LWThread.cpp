#include "LWPlatform/LWThread.h"
#include <thread>
#include <iostream>

LWThread &LWThread::Join(void) {
	m_Thread.join();
	return *this;
}

LWThread &LWThread::Detatch(void) {
	m_Thread.detach();
	return *this;
}

void *LWThread::GetUserData(void) {
	return m_UserData;
}

LWThread::LWThread(std::function<void(LWThread *T)> Func, void *UserData) : m_UserData(UserData) {
	m_Thread = std::move(std::thread(Func, this));
	return;
}

LWThread::~LWThread() {
}