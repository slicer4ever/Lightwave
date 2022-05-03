#ifndef APP_H
#define APP_H
#include <LWCore/LWAllocator.h>
#include <LWPlatform/LWWindow.h>
#include <LWAudio/LWAudioDriver.h>
#include <LWEAsset.h>
#include "Renderer.h"
#include "Scene.h"
#include "Camera.h"

class App {
public:
	enum {
		Terminate = 0x1,
		OrbitCamera = 0x2,
		TimeTrackerCount = 30
	};
	static const float OrbitRadius;

	App &Update(uint64_t lCurrentTime);

	App &ProcessInput(uint64_t lCurrentTime);

	App &Render(uint64_t lCurrentTime);

	//Inserts new time into the tracker, then averages the time out.
	uint64_t PushTrackedTime(uint64_t UpdateTime);

	bool LoadAssets(void);

	bool isTerminate(void);

	App(const LWUTF8Iterator &Path, LWAllocator &Allocator);

	~App();
private:
	LWAllocator &m_Allocator;
	LWEAssetManager *m_AssetManager = nullptr;
	LWWindow *m_Window = nullptr;
	LWAudioDriver *m_ADriver = nullptr;
	LWVideoDriver *m_Driver = nullptr;
	Renderer *m_Renderer = nullptr;
	LWFont *m_DefaultFont = nullptr;
	Scene *m_Scene = nullptr;
	uint32_t m_Flag = 0;
	Camera m_Camera;
	float m_SceneScale = 1.0f;
	float m_SceneTheta = 0.0f;
	uint64_t m_TrackedFrameTime[TimeTrackerCount];
	uint32_t m_TrackedFrameCount = 0;
	uint32_t m_InputIgnoreTicks = 1;
	uint64_t m_LastUpdateTime = 0;
	float m_OrbitRadius = 15.0f;
};

#endif