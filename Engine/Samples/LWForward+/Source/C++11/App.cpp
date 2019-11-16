#include "App.h"
#include <LWCore/LWTimer.h>
#include <LWPlatform/LWVideoMode.h>
#include <LWPlatform/LWFileStream.h>
#include <LWPlatform/LWApplication.h>
#include <LWVideo/LWFont.h>
#include <LWEXML.h>
#include <algorithm>

const float App::OrbitRadius = 15.0f;

App &App::Update(uint64_t lCurrentTime) {
	lFrame *F = m_Renderer->BeginFrame();
	float Delta = (float)(lCurrentTime - m_LastUpdateTime) / LWTimer::GetResolution();
	m_Scene->Update(Delta);
	m_ADriver->Update(lCurrentTime, m_Window);
	m_ADriver->SetListener(m_Camera.GetPosition(), m_Camera.GetDirection(), m_Camera.GetUp());
	if (!F) return *this;
	if (!m_LastUpdateTime) m_LastUpdateTime = lCurrentTime;
	uint64_t UpdateFreq = PushTrackedTime(lCurrentTime - m_LastUpdateTime);
	m_Camera.SetAspect(m_Window->GetAspect()).BuildFrustrum();
	LWVector3f SceneSize = m_Scene->GetAAMax()-m_Scene->GetAAMin();
	float SceneRadi = std::max<float>(std::max<float>(SceneSize.x, SceneSize.y), SceneSize.z);

	m_Flag = (m_Flag&~OrbitCamera) | (SceneRadi < OrbitRadius) ? OrbitCamera : 0;

	LWMatrix4f SceneTransform = LWMatrix4f(m_SceneScale, m_SceneScale, m_SceneScale, 1.0f) * LWMatrix4f::RotationY(m_SceneTheta);
	uint32_t Vertices = m_Scene->DrawScene(lCurrentTime, SceneTransform, m_Renderer, *F, m_Camera, m_Driver);
	float Deg = fmodf(m_SceneTheta*LW_RADTODEG, 360.0f);
	if (Deg < 0.0f) Deg += 360.0f;
	m_DefaultFont->DrawTextmf("Vertices: %d FrameTime: %dms Scale: %.2f Rotation: %.2f", LWVector2f(), 2.0f, LWVector4f(1.0f, 1.0f, 1.0f, 1.0f), &F->m_FontWriter, &LWFontSimpleWriter::WriteGlyph, Vertices, UpdateFreq, m_SceneScale, Deg);
	m_Renderer->EndFrame();
	m_LastUpdateTime = lCurrentTime;
	return *this;
}

App &App::ProcessInput(uint64_t lCurrentTime) {
	LWKeyboard *KB = m_Window->GetKeyboardDevice();
	LWMouse *Mouse = m_Window->GetMouseDevice();
	LWVector2f MP = Mouse->GetPositionf();
	LWVector2f TargetMousePos = m_Window->GetSizef() / 2;
	const float MouseSensitivity = 0.005f;
	//const float MoveSpeed = 8.0f;
	//const float VerticalSpeed = 3.0f;
	const float Multiplier = 1.0f;
	const float MoveSpeed = 4.0f*Multiplier;
	const float VerticalSpeed = 1.0f*Multiplier;
	m_Window->Update(lCurrentTime);
	if (m_Window->GetFlag()&LWWindow::Terminate) m_Flag |= Terminate;
	if (!m_Window->isFocused()) return *this;
	if (KB->ButtonPressed(LWKey::Esc)) m_Flag |= Terminate;

	LWVector2f MouseMove = (MP - TargetMousePos);
	if (m_InputIgnoreTicks) {
		if (MouseMove == LWVector2f(0.0f)) m_InputIgnoreTicks = 0;
		MouseMove = LWVector2f();
	} else if (MouseMove.Length() < 1.0f) MouseMove = LWVector2f();
	bool Orbit = (m_Flag&OrbitCamera) != 0;
	if (Orbit) {
		m_Camera.ProcessDirectionInputThird(LWVector3f(), m_OrbitRadius, MouseMove, MouseSensitivity, MouseSensitivity, LWVector4f(-LW_PI, LW_PI, -LW_PI_2 * 0.9f, LW_PI_2*0.9f), true);
	} else {
		m_Camera.ProcessDirectionInputFirst(MouseMove, MouseSensitivity, MouseSensitivity, LWVector4f(-LW_PI, LW_PI, -LW_PI_2 * 0.9f, LW_PI_2*0.9f), true);
		LWVector3f FlatDir = m_Camera.GetFlatDirection();
		LWVector3f RightDir = m_Camera.GetUp().Cross(FlatDir);
		LWVector3f CamPos = m_Camera.GetPosition();
		LWVector3f MoveDir = LWVector3f();
		if (KB->ButtonDown(LWKey::A)) MoveDir += RightDir;
		if (KB->ButtonDown(LWKey::D)) MoveDir -= RightDir;
		if (KB->ButtonDown(LWKey::W)) MoveDir += FlatDir;
		if (KB->ButtonDown(LWKey::S)) MoveDir -= FlatDir;
		if (KB->ButtonDown(LWKey::Space)) CamPos.y += VerticalSpeed;
		if (KB->ButtonDown(LWKey::LShift)) CamPos.y -= VerticalSpeed;
		CamPos += MoveDir.Normalize()*MoveSpeed;
		m_Camera.SetPosition(CamPos);
	}
	const float ScaleRate = 0.25f;
	const float ScaleMin = 0.25f;
	const float ScaleMax = 50.0f;
	const float RotationSpeed = LW_DEGTORAD * 2.0f;

	if (Mouse->GetScroll()) {
		
		if (Mouse->GetScroll() < 0) m_SceneScale = std::min<float>(m_SceneScale + ScaleRate, ScaleMax);
		else m_SceneScale = std::max<float>(m_SceneScale - ScaleRate, ScaleMin);
		/*
		if (Mouse->GetScroll() < 0) m_OrbitRadius = std::min<float>(m_OrbitRadius + ScaleRate, ScaleMax);
		else m_OrbitRadius = std::max<float>(m_OrbitRadius - ScaleRate, ScaleMin);*/
	}
	if (KB->ButtonDown(LWKey::Q)) m_SceneTheta -= RotationSpeed;
	if (KB->ButtonDown(LWKey::E)) m_SceneTheta += RotationSpeed;

	m_Window->SetMouseVisible(false);
	m_Window->SetMousePosition(TargetMousePos.CastTo<int32_t>());
	return *this;
}

App &App::Render(uint64_t lCurrentTime) {
	m_Renderer->Render(m_Window);
	return *this;
}

uint64_t App::PushTrackedTime(uint64_t UpdateTime) {
	uint32_t Idx = (m_TrackedFrameCount++%TimeTrackerCount);
	m_TrackedFrameTime[Idx] = UpdateTime;
	uint64_t Sum = 0;
	for (uint32_t i = 0; i < TimeTrackerCount; i++) Sum += m_TrackedFrameTime[i];
	return LWTimer::ToMilliSecond(Sum / TimeTrackerCount);
}

bool App::LoadAssets(void) {
	char Buffer[1024 * 32];
	LWFileStream AssetStream;
	if (!LWFileStream::OpenStream(AssetStream, "App:AssetManager.xml", LWFileStream::ReadMode | LWFileStream::BinaryMode, m_Allocator)) {
		std::cout << "Failed to open: 'App:AssetManager.xml'" << std::endl;
		return false;
	}
	AssetStream.ReadText(Buffer, sizeof(Buffer));
	m_AssetManager = m_Allocator.Allocate<LWEAssetManager>(m_Driver, nullptr, m_Allocator);
	LWEXML X;
	if (!LWEXML::ParseBuffer(X, m_Allocator, Buffer, true)) {
		std::cout << "Error parsing xml: 'App:AssetManager.xml'" << std::endl;
		return false;
	}
	X.PushParser("AssetManager", LWEAssetManager::XMLParser, m_AssetManager);
	X.Process();
	m_DefaultFont = m_AssetManager->GetAsset("DefaultFont")->AsFont();
	return true;
}

bool App::isTerminate(void) {
	return (m_Flag&Terminate) != 0;
}

App::App(const char *Path, LWAllocator &Allocator) : m_Allocator(Allocator) {
	const char Title[] = "LWWindow Forward+ Sample.";
	const char *DriverNames[] = LWVIDEODRIVER_NAMES;
	const char *PlatformNames[] = LWPLATFORM_NAMES;
	const char *ArchNames[] = LWARCH_NAMES;
	char ScenePath[256];

	LWVector2i WndSize = (LWVector2f(1280.0f, 720.0f)*LWSystemScale()).CastTo<int32_t>();
	LWVideoMode Current = LWVideoMode::GetActiveMode();
	LWVector2i ScreenSize = Current.GetSize();
	m_Window = m_Allocator.Allocate<LWWindow>(Title, "LWForward+", m_Allocator, LWWindow::WindowedMode | LWWindow::MouseDevice | LWWindow::KeyboardDevice, ScreenSize / 2 - WndSize / 2, WndSize);
	uint32_t TargetDriver = LWVideoDriver::OpenGL4_5;

	m_Driver = LWVideoDriver::MakeVideoDriver(m_Window, TargetDriver);
	if (!m_Driver) {
		m_Flag |= Terminate;
		return;
	}

	m_ADriver = m_Allocator.Allocate<LWAudioDriver>(this, m_Allocator, [](LWSound *S, LWAudioDriver *Driver) { if (S->isFinished()) S->Release(); }, nullptr, nullptr);
	if (!m_ADriver) {
		m_Flag |= Terminate;
		return;
	}
	if (!LoadAssets()) {
		m_Flag |= Terminate;
		return;
	}

	m_Renderer = m_Allocator.Allocate<Renderer>(m_Driver, this, m_AssetManager, m_Allocator);
	m_Window->SetTitlef("%s | %s | %s | %s", Title, DriverNames[m_Driver->GetDriverID()], PlatformNames[LWPLATFORM_ID], ArchNames[LWARCH_ID]);
	if (Path) strncpy(ScenePath, Path, sizeof(ScenePath));
	else {
		if (!m_Window->MakeLoadFileDialog("*.gltf\0GLTF File\0*.glb\0GLB File\0*.*\0Any File\0", ScenePath, sizeof(ScenePath))) {
			m_Flag |= Terminate;
			return;
		}
	}
		
	m_Scene = Scene::LoadGLTF(ScenePath, m_Driver, Allocator);
	if (!m_Scene) {
		m_Flag |= Terminate;
		return;
	}

	m_Scene->PushLight(Light(LWVector4f(1.0f), 0.05f));
	if (m_Scene->GetLightCount() == 1) { //Add "sun" if no lighting is provided by the model.
		m_Scene->PushLight(Light(LWVector3f(1.0f, -0.1f, 0.0f).Normalize(), LWVector4f(1.0f), 1.0f, LWVector4i(-1)));
	}

	m_Camera = Camera(LWVector3f(0.0f, 0.0f, 0.0f), LWVector3f(-1.0f, 0.0f, 0.0f), LWVector3f(0.0f, 1.0f, 0.0f), 1, 1.0f, LW_PI_4, 0.1f, 5000.0f, false);
	m_Camera.ToggleCameraControl();
	std::fill(m_TrackedFrameTime, m_TrackedFrameTime + TimeTrackerCount, 0);
}

App::~App() {
	LWAllocator::Destroy(m_ADriver);
	LWAllocator::Destroy(m_Renderer);
	LWAllocator::Destroy(m_AssetManager);
	LWAllocator::Destroy(m_Scene);
	if(m_Driver) LWVideoDriver::DestroyVideoDriver(m_Driver);
	if(m_Window) LWAllocator::Destroy(m_Window);
}