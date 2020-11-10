#include "App.h"
#include "LWPlatform/LWPlatform.h"
#include "LWPlatform/LWWindow.h"
#include "LWPlatform/LWVideoMode.h"
#include "LWVideo/LWVideoDriver.h"
#include <LWEUIManager.h>
#include <LWEAsset.h>
#include <LWELocalization.h>
#include <LWEUI/LWEUILabel.h>
#include "Renderer.h"

void App::UpdateJob(LWEJob &J, LWEJobThread &Th, LWEJobQueue &Q, uint64_t lCurrentTime) {
	GFrame *F = m_Renderer->BeginFrame();
	if (!F) return;
	m_UIManager->Draw(F->m_UIFrame, lCurrentTime);
	m_Renderer->EndFrame();
	return;
}

void App::InputJob(LWEJob &J, LWEJobThread &Th, LWEJobQueue &Q, uint64_t lCurrentTime) {
	LWKeyboard *KB = m_Window->GetKeyboardDevice();
	m_Window->Update(lCurrentTime);
	m_JobQueue.SetFinished(m_Window->isFinished() || KB->ButtonDown(LWKey::Esc));
	m_UIManager->Update(lCurrentTime);
	return;
}

void App::RenderJob(LWEJob &J, LWEJobThread &Th, LWEJobQueue &Q, uint64_t lCurrentTime) {
	m_Renderer->Render(m_Window);
	return;
}

void App::Run(void) {
	m_JobQueue.SetSleep(true).Start();
	m_JobQueue.RunThread(&m_JobQueue.GetMainThread(), &m_JobQueue);
	m_JobQueue.WaitForAllJoined();
	m_JobQueue.OutputJobTimings();
	m_JobQueue.OutputThreadTimings();
	return;
}

bool App::LoadAssets(const LWUTF8Iterator &Path) {
	LWELocalization *oLocalize = m_Localization;
	LWEAssetManager *oAsset = m_AssetManager;
	LWEUIManager *oUI = m_UIManager;
	LWVideoMode CurrMode = LWVideoMode::GetActiveMode();

	LWEXML X;
	if (!LWEXML::LoadFile(X, m_Allocator, Path, true)) {
		fmt::print("Error: could not load '{}'\n", Path);
		return false;
	}
	m_Localization = m_Allocator.Create<LWELocalization>(m_Allocator);
	m_AssetManager = m_Allocator.Create<LWEAssetManager>(m_Driver, m_Localization, m_Allocator);
	m_UIManager = m_Allocator.Create<LWEUIManager>(m_Window, CurrMode.GetDPI().x, m_Allocator, m_Localization, m_AssetManager);
	X.PushParser("Localization", LWELocalization::XMLParser, m_Localization);
	X.PushParser("AssetManager", LWEAssetManager::XMLParser, m_AssetManager);
	X.PushParser("UIManager", LWEUIManager::XMLParser, m_UIManager);
	X.Process();

	m_Renderer->LoadAssets(m_AssetManager, m_Allocator);
	new (&m_UITextInputTest) UITextInputTest("UITextInputTest", m_UIManager, &m_Allocator);
	
	LWAllocator::Destroy(oLocalize);
	LWAllocator::Destroy(oUI);
	LWAllocator::Destroy(oAsset);
	return true;
}

App::App(LWAllocator &Allocator) : m_Allocator(Allocator) {
	const char8_t Title[] = "LWEUIIntegrationTest";
	const char8_t *DriverNames[] = LWVIDEODRIVER_NAMES;
	const char8_t *PlatformNames[] = LWPLATFORM_NAMES;
	const char8_t *ArchNames[] = LWARCH_NAMES;

	LWVideoMode CurrMode = LWVideoMode::GetActiveMode();
	LWVector2i TargetSize = LWVector2i(1280, 720);
	m_Window = m_Allocator.Create<LWWindow>(Title, Title, Allocator, LWWindow::KeyboardDevice | LWWindow::MouseDevice | LWWindow::WindowedMode, CurrMode.GetSize() / 2 - TargetSize / 2, TargetSize);
	if ((m_Window->GetFlag() & LWWindow::Error) != 0) {
		fmt::print("Error creating window.\n");
		m_JobQueue.SetFinished(true);
		return;
	}
	uint32_t TargetDriver = LWVideoDriver::Unspecefied;
	m_Driver = LWVideoDriver::MakeVideoDriver(m_Window, TargetDriver);
	if (!m_Driver) {
		fmt::print("Error creating driver.\n");
		m_JobQueue.SetFinished(true);
		return;
	}
	m_Window->SetTitle(LWUTF8I::Fmt<64>("{} | {} | {} | {}", Title, DriverNames[m_Driver->GetDriverID()], PlatformNames[LWPLATFORM_ID], ArchNames[LWARCH_ID]));

	m_Renderer = m_Allocator.Create<Renderer>(m_Driver, m_Allocator);

	if (!LoadAssets("App:UIData.xml")) {
		m_JobQueue.SetFinished(true);
	}

	m_JobQueue.PushJob(LWEJob::MakeMethod(&App::UpdateJob, this, nullptr, 0, 0, 0, 0, 0, 0, ~0x1));
	m_JobQueue.PushJob(LWEJob::MakeMethod(&App::InputJob, this, nullptr, 0, 0, 0, 0, 0, 0, 0x1));
	m_JobQueue.PushJob(LWEJob::MakeMethod(&App::RenderJob, this, nullptr, 0, 0, 0, 0, 0, 0, 0x1));
}

App::~App() {
	LWAllocator::Destroy(m_Localization);
	LWAllocator::Destroy(m_UIManager);
	LWAllocator::Destroy(m_AssetManager);
	LWAllocator::Destroy(m_Renderer);
	LWVideoDriver::DestroyVideoDriver(m_Driver);
	LWAllocator::Destroy(m_Window);
}
