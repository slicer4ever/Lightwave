#ifndef APP_H
#define APP_H
#include <LWCore/LWTypes.h>
#include <LWPlatform/LWTypes.h>
#include <LWVideo/LWTypes.h>
#include "LWEJobQueue.h"
#include "UITextInputTest.h"

class Renderer;

class App {
public:

	void UpdateJob(LWEJob &J, LWEJobThread &Th, LWEJobQueue &Q, uint64_t lCurrentTime);

	void InputJob(LWEJob &J, LWEJobThread &Th, LWEJobQueue &Q, uint64_t lCurrentTime);

	void RenderJob(LWEJob &J, LWEJobThread &Th, LWEJobQueue &Q, uint64_t lCurrentTime);

	void Run(void);

	bool LoadAssets(const LWUTF8Iterator &Path);

	App(LWAllocator &Allocator);

	~App();
private:
	LWAllocator &m_Allocator;
	LWEJobQueue m_JobQueue;
	LWEUILabel *m_MsgLbl = nullptr;
	LWWindow *m_Window = nullptr;
	Renderer *m_Renderer = nullptr;
	LWELocalization *m_Localization = nullptr;
	LWEAssetManager *m_AssetManager = nullptr;
	LWEUIManager *m_UIManager = nullptr;
	LWVideoDriver *m_Driver = nullptr;
	UITextInputTest m_UITextInputTest;

};

#endif