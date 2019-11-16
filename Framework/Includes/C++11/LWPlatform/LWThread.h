#ifndef LWTHREAD_H
#define LWTHREAD_H
#include "LWPlatform/LWPlatform.h"
#include <functional>

class LWThread {
public:
	LWThread &Join(void);

	LWThread &Detatch(void);

	void *GetUserData(void);

	LWThread(std::function<void(LWThread *T)> Func, void *UserData);

	~LWThread();
private:
	LWThreadType m_Thread;
	void *m_UserData;
};



#endif