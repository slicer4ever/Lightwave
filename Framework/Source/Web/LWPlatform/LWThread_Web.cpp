#include "LWPlatform/LWThread.h"
#include <iostream>

LWThread &LWThread::Join(void) {
	return *this;
}

LWThread &LWThread::Detatch(void) {
	return *this;
}

void *LWThread::GetUserData(void) {
	return m_UserData;
}

LWThread::LWThread(std::function<void(LWThread *T)> Func, void *UserData) : m_UserData(UserData) {
	Func(this);
	return;
}

LWThread::~LWThread() {
}