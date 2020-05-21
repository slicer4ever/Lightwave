#include "LWPlatform/LWPlatform.h"
#include "LWPlatform/LWWindow.h"
#include "LWPlatform/LWFileStream.h"
#include "LWPlatform/LWDirectory.h"
#include "LWPlatform/LWInputDevice.h"
#include "LWPlatform/LWVideoMode.h"
#include "LWVideo/LWVideoBuffer.h"
#include "LWVideo/LWVideoDriver.h"
#include "LWVideo/LWImage.h"
#include "LWVideo/LWMesh.h"
#include "LWVideo/LWFrameBuffer.h"
#include "LWVideo/LWFont.h"
#include "LWVideo/LWPipeline.h"
#include "LWCore/LWAllocators/LWAllocator_Default.h"
#include "LWCore/LWTimer.h"
#include "LWCore/LWMatrix.h"
#include "LWCore/LWMath.h"
#include "LWPlatform/LWApplication.h"
#include "LWAudio/LWAudioDriver.h"
#include "LWAudio/LWAudioStream.h"
#include "LWAudio/LWSound.h"
#include <iostream>
#include <functional>
#include <cstdarg>
//This sample demonstrate a very simple fully-complete example of the LWFramework in action.
//The end goal of this sample should be a textured rectangle bumping around the screen(and grabable), with accompanying audio noises, this test should demonstrate a number of features utilized in the framework.  although it is not comprise to every possible function, it should cover many.

struct UniformBlock{
	LWVector4f Color;
	LWMatrix4f Matrix;
};

struct App {
public:
	

	void SetFinished(const char *Error) {
		std::cout << Error << std::endl;
		return;
	}
	
	void SetFinishedf(const char *ErrorFmt, ...) {
		char Buffer[1024];
		va_list lst;
		va_start(lst, ErrorFmt);
		vsnprintf(Buffer, sizeof(Buffer), ErrorFmt, lst);
		va_end(lst);
		return SetFinished(Buffer);
	}

	void ProcessInput(void) {
		m_Window->Update(LWTimer::GetCurrent());
		m_AudioDriver->Update(LWTimer::GetCurrent(), m_Window);
		if (m_Window->GetFlag()&LWWindow::Terminate) m_Finished = true;
		if (m_Window->GetFlag()&LWWindow::OrientationChanged) {
			std::cout << "Orientation has changed: " << ((m_Window->GetOrientation() == LWWindow::Rotation_0) ? "Portrait" : ((m_Window->GetOrientation() == LWWindow::Rotation_90) ? "Landscape" : ((m_Window->GetOrientation() == LWWindow::Rotation_180) ? "Upside down portrait" : "Upside down landscape"))) << std::endl;
		}
		if (m_Window->GetFlag()&LWWindow::FocusChanged) std::cout << "Focus has changed: " << ((m_Window->GetFlag()&LWWindow::Focused) ? "Current" : "Not Current") << std::endl;
		if (m_Window->GetFlag()&LWWindow::PosChanged) std::cout << "Position has changed: " << m_Window->GetPosition().x << " " << m_Window->GetPosition().y << std::endl;
		LWMouse *Mouse = m_Window->GetMouseDevice();
		LWKeyboard *KB = m_Window->GetKeyboardDevice();
		LWTouch *TD = m_Window->GetTouchDevice();
		if (Mouse) {
			if (Mouse->ButtonDown(LWMouseKey::Left)) std::cout << "Left mouse down at: " << Mouse->GetPosition().x << " " << Mouse->GetPosition().y << " scroll: " << Mouse->GetScroll() << std::endl;

			if (Mouse->ButtonPressed(LWMouseKey::Left)) {
				std::cout << "Left mouse pressed!" << std::endl;
				/*
				if((Wnd->GetFlag()&LWWindow::KeyboardPresent)==0) Wnd->OpenKeyboard("abc");
				else{
				uint32_t CursorPos = 0;
				uint32_t EditLength = 0;
				Wnd->GetKeyboardEditRange(CursorPos, EditLength);
				Wnd->GetKeyboardText(TextBuffer, sizeof(TextBuffer));
				std::cout << "Cursor: " << CursorPos << " - " << EditLength << std::endl << "'" << TextBuffer << "'" << std::endl;
				Wnd->CloseKeyboard();
				}*/
			}
			if (Mouse->ButtonReleased(LWMouseKey::Left)) std::cout << "Left mouse released!" << std::endl;
			if (Mouse->ButtonDown(LWMouseKey::Right)) std::cout << "Right mouse down!" << std::endl;
			if (Mouse->ButtonPressed(LWMouseKey::Right)) std::cout << "Right mouse pressed!" << std::endl;
			if (Mouse->ButtonReleased(LWMouseKey::Right)) std::cout << "Right mouse released!" << std::endl;
			if (Mouse->ButtonDown(LWMouseKey::Middle)) std::cout << "Middle mouse down!" << std::endl;
			if (Mouse->ButtonPressed(LWMouseKey::Middle)) std::cout << "Middle mouse pressed!" << std::endl;
			if (Mouse->ButtonReleased(LWMouseKey::Middle)) std::cout << "Middle mouse released!" << std::endl;
			if (Mouse->ButtonDown(LWMouseKey::X1)) std::cout << "X1 mouse down!" << std::endl;
			if (Mouse->ButtonPressed(LWMouseKey::X1)) std::cout << "X1 mouse pressed!" << std::endl;
			if (Mouse->ButtonReleased(LWMouseKey::X1)) std::cout << "X1 mouse released!" << std::endl;
			if (Mouse->ButtonDown(LWMouseKey::X2)) std::cout << "X2 mouse down!" << std::endl;
			if (Mouse->ButtonPressed(LWMouseKey::X2)) std::cout << "X2 mouse pressed!" << std::endl;
			if (Mouse->ButtonReleased(LWMouseKey::X2)) std::cout << "X2 mouse released!" << std::endl;
		}
		if (KB) {
			for (uint32_t i = 0; i<KB->GetKeyChangeCount(); i++) {
				uint32_t Key = KB->GetKeyChanged(i);
				std::cout << "Key: " << Key << " is: " << (KB->ButtonDown(Key) ? "True" : "False") << std::endl;
			}
			for (uint32_t i = 0; i<KB->GetCharPressed(); i++) {
				uint32_t Char = KB->GetChar(i);
				std::cout << "Char: " << (char)Char << std::endl;
			}
			if (KB->ButtonDown(LWKey::A)) m_Window->SetPosition(m_Window->GetPosition() + LWVector2i(-1, 0));
			if (KB->ButtonDown(LWKey::D)) m_Window->SetPosition(m_Window->GetPosition() + LWVector2i(1, 0));
			if (KB->ButtonDown(LWKey::S)) m_Window->SetPosition(m_Window->GetPosition() + LWVector2i(0, 1));
			if (KB->ButtonDown(LWKey::W)) m_Window->SetPosition(m_Window->GetPosition() + LWVector2i(0, -1));
			if (KB->ButtonPressed(LWKey::Space)) m_UseMSDF = !m_UseMSDF;
			if (KB->ButtonPressed(LWKey::R)) m_Window->SetMousePosition(m_Window->GetSize() / 2);

			if (KB->ButtonPressed(LWKey::C)) m_Window->SetMouseVisible((m_Window->GetFlag()&LWWindow::MouseVisible) != 0 ? false : true);
			if (KB->ButtonPressed(LWKey::F11)) {
				bool isFullscreen = (m_Window->GetFlag()&LWWindow::Borderless) != 0;
				m_Window->SetBorderless(!isFullscreen, true);
				if (isFullscreen) {
					m_Window->SetSize(m_PrevSize).SetPosition(m_PrevPos);
				} else {
					m_PrevPos = m_Window->GetPosition();
					m_PrevSize = m_Window->GetSize();
					m_Window->SetPosition(LWVector2i(0)).SetSize(LWVideoMode::GetActiveMode().GetSize());
				}
				m_SizeChanged = true;
			}
			if (KB->ButtonPressed(LWKey::Esc)) m_Finished = true;
		}
		if (TD) {
			for (uint32_t i = 0; i < TD->GetPointCount(); i++) {
				const LWTouchPoint &Pnt = TD->GetPoint(i);
				std::cout << "Point: " << Pnt.m_Position.x << " " << Pnt.m_Position.y << " State: " << Pnt.m_State << " Init: " << Pnt.m_InitPosition.x << " " << Pnt.m_InitPosition.y << " DownTime: " << Pnt.m_DownTime << " Size: " << Pnt.m_Size << std::endl;
				if (Pnt.m_State == LWTouchPoint::DOWN) m_UseMSDF = !m_UseMSDF;
			}
			const LWGesture &Gest = TD->GetGesture();
			if (Gest.m_Type != LWGesture::None) {
				std::cout << "Gesture: " << Gest.m_Type << " Source: " << Gest.m_Source.x << " " << Gest.m_Source.y << " Dir: " << Gest.m_Direction.x << " " << Gest.m_Direction.y << " Scale: " << Gest.m_Scale << std::endl;
			}
		}
	}

	void Draw(void) {
		if (m_Window->SizeUpdated()) m_SizeChanged = true;
		
		if (!m_Driver->Update()) return;
		
		if (m_SizeChanged) {
			std::cout << "Size has changed: " << m_Window->GetSize().x << " " << m_Window->GetSize().y << std::endl;
			m_Driver->ViewPort();
			LWMatrix4f Ortho = LWMatrix4f::Ortho(0.0f, m_Window->GetSizef().x, 0.0f, m_Window->GetSizef().y, 0.0f, 1.0f);
			memcpy(m_FontUniformBuffer->GetLocalBuffer(), &Ortho, m_FontUniformBuffer->GetRawLength());
			m_FontUniformBuffer->SetEditLength(m_FontUniformBuffer->GetRawLength());
			m_SizeChanged = false;
		}
		//std::cout << "FrameA " << std::this_thread::get_id() << std::endl;
		//NDKFlushOutput();*/
		m_Driver->ClearColor(0xFF0000FF);
		m_UniformBuffer->SetEditLength(sizeof(UniformBlock));
		m_Driver->DrawMesh(m_DefaultPipeline, LWVideoDriver::Triangle, m_RectMesh);
		/*Driver->DrawBuffer(S, LWVideoDriver::Triangle, InputData, 3, sizeof(LWVertexPosition), 0);

		Block->Color = LWVector4f(0.0f, 1.0f, 0.0f, 1.0f);
		UniformBuffer->SetEditLength(sizeof(UniformBlock)).MarkUpdated();
		Driver->DrawBuffer(S, LWVideoDriver::Triangle, InputData, 3, sizeof(LWVertexPosition), 3);
		*/
		//UniformBuffer->SetEditRange(sizeof(LWVector4f), offsetof(UniformBlock, Color)).MarkUpdated();
		//Driver->DrawMesh(S, LWVideoDriver::Triangle, RectMesh);
		//Driver->DrawBuffer(S, LWVideoDriver::Triangle, InputData, nullptr, 6, sizeof(LWVertexPosition));
		//if (m_UseSDF) m_Driver->DrawMesh(m_SDFFontShader, LWVideoDriver::Triangle, m_SDFFontMesh);
		//else 
		if (m_UseMSDF) m_Driver->DrawMesh(m_FontMSDFPipeline, LWVideoDriver::Triangle, m_MSDFFontMesh);
		else m_Driver->DrawMesh(m_FontColorPipeline, LWVideoDriver::Triangle, m_ColorFontMesh);
		m_Driver->Present(1);
		
		m_Theta += 0.01f;
		//Since we use an ortho matrix, we scale down our rotating image so it's not z clipped while rotating.
		m_UniBlock->Matrix = LWMatrix4f(0.666f, 0.666f, 0.0f, 1.0f)*LWMatrix4f::RotationY(m_Theta)*LWMatrix4f::Translation(0.0f, 0.0f, 0.5f);
		
	}

	App(LWAllocator &Allocator) : m_Allocator(Allocator) {
		char ErrorBuffer[1024];
		m_Stream = LWAudioStream::Create("App:SampleSound.ogg", 0, Allocator);

		if(!m_Stream){
			SetFinished("Error opening sound: 'App:SampleSound.ogg'");
			return;
		}
		auto FinishedCallback = [](LWSound *S, LWAudioDriver *AD) {
			std::cout << "Finished: " << S->GetFinishedCount() << " " << S->GetPlayCount() << std::endl;
			if(S->isFinished()) {
				std::cout << "Releasing sound!" << std::endl;
				S->Release();
			}
		};

		auto CreateCallback = [](LWSound *S, LWAudioDriver *AD) {
			std::cout << "Sound created: " << LWSound::CalculateTime(S->GetAudioStream()->GetSampleLength(), S->GetAudioStream()->GetSampleRate()) << "s" << std::endl;
		};

		auto ReleaseCallback = [](LWSound *S, LWAudioDriver *AD) {
			std::cout << "Sound released!" << std::endl;
		};
		m_AudioDriver = Allocator.Allocate<LWAudioDriver>(nullptr, m_Allocator, FinishedCallback, CreateCallback, ReleaseCallback);

		if (m_AudioDriver->GetFlag()&LWAudioDriver::Error) {
			SetFinished("Error creating audio driver.");
			return;
		}

		LWSound *Snd = m_AudioDriver->CreateSound2D(m_Stream, nullptr, 0, true, 3 );
		if (!Snd) {
			std::cout << "Failed to create sound..." << std::endl;
		} else {
			std::cout << "Sound created!" << std::endl;
		}
		//AudioStream.Finished();*/
		//while ((Wnd->Update(LWTimer::GetCurrent()).GetFlag()&LWWindow::Terminate) == 0 && !KB->ButtonDown(LWKey::Esc)){
		//if (Wnd->GetFlag()&LWWindow::SizeChanged) SizeChanged = true;
		/*if (!ADriver->Update(LWTimer::GetCurrent(), Wnd)) {
		std::this_thread::yield();
		continue;
		}*/
		//}
		auto OutputMode = [](const LWVideoMode &Mode) { std::cout << "Mode: " << Mode.GetSize().x << " " << Mode.GetSize().y << " Rate: " << Mode.GetFrequency() << " Interlaced: " << ((Mode.GetFlag()&LWVideoMode::Interlaced) ? "Yes" : "No") << " | Colored Mode: " << ((Mode.GetColorMode() == LWVideoMode::Colored4Bit) ? "4 Bit Colored" : ((Mode.GetColorMode() == LWVideoMode::Colored8Bit) ? "8 Bit Colored" : ((Mode.GetColorMode() == LWVideoMode::Colored16Bit) ? "16 Bit Colored" : ((Mode.GetColorMode() == LWVideoMode::Colored32Bit) ? "32 Bit Colored" : "Unknown colored mode")))) << " Rotation: " << ((Mode.GetRotation() == LWVideoMode::Rotation_0) ? "0" : ((Mode.GetRotation() == LWVideoMode::Rotation_90) ? "90" : ((Mode.GetRotation() == LWVideoMode::Rotation_180) ? "180" : "270"))) << " Flag: " << std::hex << Mode.GetFlag() << std::dec << std::endl; };
		LWVideoMode Active = LWVideoMode::GetActiveMode();

		std::cout << "Active ";
		OutputMode(Active);
		uint32_t AvailableModes = LWVideoMode::GetAllDisplayModes(nullptr, 0);
		LWVideoMode *Modes = Allocator.AllocateArray<LWVideoMode>(AvailableModes);
		LWVideoMode::GetAllDisplayModes(Modes, AvailableModes);
		//for (uint32_t i = 0; i < AvailableModes; i++) OutputMode(Modes[i]);

		LWVector2i PrevSize = LWVector2i();
		LWVector2i PrevPos = LWVector2i();
		LWVector2i Size = LWVector2i(800, 600);
		/*LWVideoMode Target = LWVideoMode(LWVector2i(800, 600), 60, LWVideoMode::Colored32Bit);*/
		//We comment this out because we don't want to change our display right now.
		/*if(!LWVideoMode::SetDisplayTo(Target)){
		LWWindow::MakeDialog(LWText("Error: Changing display size"), LWText("ERROR"),  LWWindow::DialogOK);
		return 0;
		}*/
		*ErrorBuffer = '\0';
		LWAllocator::Destroy(Modes);

		m_Window = m_Allocator.Allocate<LWWindow>("LWPlatform Testing suite.", "LWPlatformTest", m_Allocator, LWWindow::WindowedMode | LWWindow::KeyboardDevice | LWWindow::MouseDevice | LWWindow::TouchDevice, Active.GetSize() / 2 - Size / 2, Size);
		if (m_Window->GetFlag()&LWWindow::Error) {
			SetFinished("Error creating window!");
			return;
		}
		//uint32_t TargetDriver = LWVideoDriver::Vulkan | LWVideoDriver::DebugLayer;
		uint32_t TargetDriver = LWVideoDriver::Unspecefied ;
		std::cout << "Window created: " << m_Window->GetSize().x << " " << m_Window->GetSize().y << std::endl;
		m_Driver = LWVideoDriver::MakeVideoDriver(m_Window, TargetDriver);
		if (!m_Driver) {
			SetFinished("Error: Creating video driver.");
			return;
		}
		std::cout << "Driver created: " << m_Driver->GetDriverType() << std::endl;

		
		m_DefaultVertexShader = m_Driver->LoadShader(LWShader::Vertex, "App:DefaultShader.vlws", Allocator, 0, nullptr, nullptr, ErrorBuffer, nullptr, sizeof(ErrorBuffer));
		if (!m_DefaultVertexShader) {
			SetFinishedf("Error loading vertex shader:\n%s\n", ErrorBuffer);
			return;
		}
		m_DefaultVertexShader->SetInputMap(2, "Position", LWShaderInput::Vec4, 1, "TexCoord", LWShaderInput::Vec4, 1);
		m_DefaultPixelShader = m_Driver->LoadShader(LWShader::Pixel, "App:DefaultShader.plws", Allocator, 0, nullptr, nullptr, ErrorBuffer, nullptr, sizeof(ErrorBuffer));
		if (!m_DefaultPixelShader) {
			SetFinishedf("Error loading pixel shader:\n%s\n", ErrorBuffer);
			return;
		}
		m_DefaultPipeline = m_Driver->CreatePipeline(m_DefaultVertexShader, nullptr, m_DefaultPixelShader, 0, 0, LWPipeline::CULL_NONE, LWPipeline::SOLID, Allocator);
		if (!m_DefaultPipeline) {
			SetFinished("Error creating default pipeline.");
			return;
		}
		
		UniformBlock B = { LWVector4f(1.0f, 0.0f, 1.0f, 1.0f), LWMatrix4f(1.0f) };
		m_UniformBuffer = m_Driver->CreateVideoBuffer(LWVideoBuffer::Uniform, LWVideoBuffer::WriteDiscardable | LWVideoBuffer::LocalCopy, sizeof(UniformBlock), 1, Allocator, (const uint8_t*)&B);
		if (!m_UniformBuffer) {
			SetFinished("Error: Creating Uniform buffer");
			return;
		}
		m_UniBlock = (UniformBlock*)m_UniformBuffer->GetLocalBuffer();
		
		LWImage Image;
		if (!LWImage::LoadImage(Image, LWText("App:Sample.png"), Allocator)) {
			SetFinished("Error: loading image.");
			return;
		}
		std::cout << "Size: " << Image.GetSize2D().x << " " << Image.GetSize2D().y << " PackType: " << Image.GetPackType() << std::endl;
		
		m_Tex = m_Driver->CreateTexture(0, Image, Allocator);
		if (!m_Tex) {
			SetFinished("Error: Creating texture!!");
			return;
		}
		
		m_DefaultPipeline->SetUniformBlock(0, m_UniformBuffer);
		m_DefaultPipeline->SetResource(0, m_Tex);
		m_RTex = m_Driver->CreateTexture2D(LWTexture::RenderTarget, LWImage::RGBA8, LWVector2i(128, 128), nullptr, 0, Allocator);
		if (!m_RTex) {
			SetFinished("Error: Creating render texture!");
			return;
		}
		//float Pnts[] = { -0.25f, -0.25f, 0.0f, 1.0f, 0.25f, -0.25f, 0.0f, 1.0f,  -0.25f, 0.25f, 0.0f, 1.0f, -0.25f, 0.25f, 0.0f, 1.0f, 0.25f, -0.25f, 0.0f, 1.0f, 1.0f, 0.0f, 0.25f, 0.25f, 0.0f, 1.0f, 1.0f, 1.0f };

		//float Pnts[] = { -0.25f, -0.25f, 0.0f, 1.0f, -0.25f, 0.25f, 0.0f, 1.0f, 0.25f, -0.25f, 0.0f, 1.0f, 0.25f, -0.25f, 0.0f, 1.0f, -0.25f, 0.25f, 0.0f, 1.0f, 0.25f, 0.25f, 0.0f, 1.0f };
		//LWVertexPosition P[] = { { LWVector4f(-0.25f, -0.25f, 0.0f, 1.0f) }, { LWVector4f(-0.25f, 0.25f, 0.0f, 1.0f) }, { LWVector4f(0.25f, -0.25f, 0.0f, 1.0f) }, { LWVector4f(-0.25f, 0.25f, 0.0f, 1.0f) }, { LWVector4f(0.25f, 0.25f, 0.0f, 1.0f) }, { LWVector4f(0.25f, -0.25f, 0.0f, 1.0f) } };
		//LWVertexPosition P[] = { { LWVector4f(-1.0f, -1.0f, 0.0f, 1.0f) }, { LWVector4f(1.0f, -1.0f, 0.0f, 1.0f) }, { LWVector4f(0.0f, 1.0f, 0.0f, 1.0f) } };

		m_VertexBuffer = m_Driver->CreateVideoBuffer(LWVideoBuffer::Vertex, LWVideoBuffer::LocalCopy | LWVideoBuffer::WriteDiscardable, sizeof(LWVertexTexture), 6, Allocator, nullptr);
		if (!m_VertexBuffer) {
			SetFinished("Error: Creating input buffer");
			return;
		}
		
		m_RectMesh = LWVertexTexture::MakeMesh(Allocator, m_VertexBuffer, 6);
		LWVertexTexture::WriteRectangle(m_RectMesh, LWVector2f(0.75f), LWVector2f(-0.75f), LWVector2f(0.0f, 0.0f), LWVector2f(1.0f, 1.0f));
		m_Driver->UpdateMesh(&m_RectMesh->Finished());
		const char *DriverNames[] = LWVIDEODRIVER_NAMES;
		m_Window->SetTitlef("LWPlatform Testing suite. | %s", DriverNames[m_Driver->GetDriverID()]);

		m_UniBlock->Color = LWVector4f(1.0f);
		m_UniformBuffer->SetEditLength(sizeof(UniformBlock));

		
		m_FrameBuffer = m_Driver->CreateFrameBuffer(LWVector2i(128, 128), Allocator);
		if (!m_FrameBuffer) {
			SetFinished("Error: Creating framebuffer!");
			return;
		}
		
		m_FrameBuffer->SetAttachment(LWFrameBuffer::Color0, m_RTex);
		m_Driver->SetFrameBuffer(m_FrameBuffer);
		m_Driver->ViewPort(m_FrameBuffer);
		
		m_Driver->ClearColor(0xFFFF);
		m_Driver->DrawMesh(m_DefaultPipeline, LWVideoDriver::Triangle, m_RectMesh);
		m_Driver->SetFrameBuffer(nullptr);
		m_Driver->ViewPort();
		m_DefaultPipeline->SetResource(0, m_RTex);
		float Theta = 0.0f;
		
		LWFileStream FontColorStream;
		if (!LWFileStream::OpenStream(FontColorStream, "App:Junicode-Regular.ttf", LWFileStream::ReadMode | LWFileStream::BinaryMode, Allocator, nullptr)) {
			SetFinished("Error opening Junicode-Regular.ttf");
			return;
		}
		m_ColorFont = LWFont::LoadFontTTF(&FontColorStream, m_Driver, 32, 32, 96, Allocator);
		if (!m_ColorFont) {
			SetFinished("Error loading ttf font.");
			return;
		}

		LWFileStream MSDFStream;
		if (!LWFileStream::OpenStream(MSDFStream, "App:Arial.arfont", LWFileStream::ReadMode | LWFileStream::BinaryMode, Allocator, nullptr)) {
			SetFinished("Error opening Junicode-Regular.arfont");
			return;
		}
		m_MSDFFont = LWFont::LoadFontAR(&MSDFStream, m_Driver, Allocator);

		if (!m_MSDFFont) {
			SetFinished("Error loading Arial.arfont");
			return;
		}

		m_FontVertexShader = m_Driver->ParseShader(LWShader::Vertex, LWFont::GetVertexShaderSource(), Allocator, 0, nullptr, nullptr, ErrorBuffer, nullptr, sizeof(ErrorBuffer));
		if (!m_FontVertexShader) {
			SetFinishedf("Error creating Font vertex shader:\n%s\n", ErrorBuffer);
			return;
		}
		m_FontVertexShader->SetInputMap(3, "Position", LWShaderInput::Vec4, 1, "Color", LWShaderInput::Vec4, 1, "TexCoord", LWShaderInput::Vec4, 1);

		m_FontColorShader = m_Driver->ParseShader(LWShader::Pixel, LWFont::GetPixelColorShaderSource(), Allocator, 0, nullptr, nullptr, ErrorBuffer, nullptr, sizeof(ErrorBuffer));
		if (!m_FontColorShader) {
			SetFinishedf("Error creating Color pixel shader:\n%s\n", ErrorBuffer);
			return;
		}

		m_FontMSDFShader = m_Driver->ParseShader(LWShader::Pixel, LWFont::GetPixelMSDFShaderSource(), Allocator, 0, nullptr, nullptr, ErrorBuffer, nullptr, sizeof(ErrorBuffer));
		if (!m_FontMSDFShader) {
			SetFinishedf("Error creating msdf pixel shader:\n%s\n", ErrorBuffer);
			return;
		}


		m_FontColorPipeline = m_Driver->CreatePipeline(m_FontVertexShader, nullptr, m_FontColorShader, LWPipeline::BLENDING, 0, LWPipeline::CULL_CW, LWPipeline::SOLID, LWPipeline::BLEND_SRC_ALPHA, LWPipeline::BLEND_ONE_MINUS_SRC_ALPHA, Allocator);
		m_FontMSDFPipeline = m_Driver->CreatePipeline(m_FontVertexShader, nullptr, m_FontMSDFShader, LWPipeline::BLENDING, 0, LWPipeline::CULL_CW, LWPipeline::SOLID, LWPipeline::BLEND_SRC_ALPHA, LWPipeline::BLEND_ONE_MINUS_SRC_ALPHA, Allocator);
		
		m_FontUniformBuffer = m_Driver->CreateVideoBuffer<LWMatrix4f>(LWVideoBuffer::Uniform, LWVideoBuffer::LocalCopy | LWVideoBuffer::WriteDiscardable, 1, Allocator, nullptr);
		m_ColorFontBuffer = m_Driver->CreateVideoBuffer<LWVertexUI>(LWVideoBuffer::Vertex, LWVideoBuffer::WriteDiscardable | LWVideoBuffer::LocalCopy, 6 * 256, Allocator,  nullptr);
		m_MSDFFontBuffer = m_Driver->CreateVideoBuffer<LWVertexUI>(LWVideoBuffer::Vertex, LWVideoBuffer::WriteDiscardable | LWVideoBuffer::LocalCopy, 6 * 256, Allocator, nullptr);

		m_FontColorPipeline->SetResource(0, m_ColorFont->GetTexture(0));
		m_FontMSDFPipeline->SetResource(0, m_MSDFFont->GetTexture(0));

		m_FontColorPipeline->SetUniformBlock(0, m_FontUniformBuffer);
		m_FontMSDFPipeline->SetUniformBlock(0, m_FontUniformBuffer);

		m_ColorFontMesh = LWVertexUI::MakeMesh(Allocator, m_ColorFontBuffer, 0);
		m_MSDFFontMesh = LWVertexUI::MakeMesh(Allocator, m_MSDFFontBuffer, 0);

		LWFontSimpleWriter ColorWriter(m_ColorFontMesh);
		LWFontSimpleWriter MSDFWriter(m_MSDFFontMesh);
		m_ColorFont->DrawTextm("LWSimple Example Color", LWVector2f(0.0f, m_ColorFont->GetLineSize()), 2.0f, LWVector4f(0.0f, 1.0f, 1.0f, 1.0f), &ColorWriter, &LWFontSimpleWriter::WriteGlyph);
		m_MSDFFont->DrawTextm("LWSimple Example MSDF", LWVector2f(0.0f, m_MSDFFont->GetLineSize()), 2.0f, LWVector4f(0.0f, 1.0f, 1.0f, 1.0f), &MSDFWriter, &LWFontSimpleWriter::WriteGlyph);
		m_ColorFontMesh->Finished();
		m_MSDFFontMesh->Finished();
	};

	~App() {
		if(m_Tex) m_Driver->DestroyTexture(m_Tex);
		if(m_RTex) m_Driver->DestroyTexture(m_RTex);
		LWAllocator::Destroy(m_MSDFFont);
		LWAllocator::Destroy(m_ColorFont);
		if (m_DefaultPipeline) m_Driver->DestroyPipeline(m_DefaultPipeline);
		if (m_FontColorPipeline) m_Driver->DestroyPipeline(m_FontColorPipeline);
		if (m_FontMSDFPipeline) m_Driver->DestroyPipeline(m_FontMSDFPipeline);

		if (m_DefaultVertexShader) m_Driver->DestroyShader(m_DefaultVertexShader);
		if (m_DefaultPixelShader) m_Driver->DestroyShader(m_DefaultPixelShader);
		if (m_FontVertexShader) m_Driver->DestroyShader(m_FontVertexShader);
		if (m_FontColorShader) m_Driver->DestroyShader(m_FontColorShader);
		if (m_FontMSDFShader) m_Driver->DestroyShader(m_FontMSDFShader);
		if(m_FrameBuffer) m_Driver->DestroyFrameBuffer(m_FrameBuffer);
		if(m_UniformBuffer) m_Driver->DestroyVideoBuffer(m_UniformBuffer);
		//if(m_SDFFontBuffer) m_Driver->DestroyVideoBuffer(m_SDFFontBuffer);
		if(m_FontUniformBuffer) m_Driver->DestroyVideoBuffer(m_FontUniformBuffer);
		if(m_RectMesh) m_RectMesh->Destroy(m_Driver);
		if (m_ColorFontMesh) m_ColorFontMesh->Destroy(m_Driver);
		if (m_MSDFFontMesh) m_MSDFFontMesh->Destroy(m_Driver);
		if (m_Driver) LWVideoDriver::DestroyVideoDriver(m_Driver);
		LWAllocator::Destroy(m_Window);

		LWAllocator::Destroy(m_AudioDriver);
		LWAllocator::Destroy(m_Stream);

	}

	LWAllocator &m_Allocator;
	LWAudioDriver *m_AudioDriver = nullptr;
	LWAudioStream *m_Stream = nullptr;
	LWWindow *m_Window = nullptr;
	LWVideoDriver *m_Driver = nullptr;
	LWShader *m_DefaultVertexShader = nullptr;
	LWShader *m_DefaultPixelShader = nullptr;
	LWShader *m_FontVertexShader = nullptr;
	LWShader *m_FontColorShader = nullptr;
	LWShader *m_FontMSDFShader = nullptr;

	LWPipeline *m_DefaultPipeline = nullptr;
	LWPipeline *m_FontColorPipeline = nullptr;
	LWPipeline *m_FontMSDFPipeline = nullptr;

	LWVideoBuffer *m_UniformBuffer = nullptr;
	LWVideoBuffer *m_ColorFontBuffer = nullptr;
	LWVideoBuffer *m_MSDFFontBuffer = nullptr;
	LWVideoBuffer *m_FontUniformBuffer = nullptr;
	UniformBlock *m_UniBlock = nullptr;
	LWVideoBuffer *m_VertexBuffer = nullptr;
	LWMesh<LWVertexTexture> *m_RectMesh = nullptr;
	LWMesh<LWVertexUI> *m_ColorFontMesh = nullptr;
	LWMesh<LWVertexUI> *m_MSDFFontMesh = nullptr;
	LWFrameBuffer *m_FrameBuffer = nullptr;
	LWTexture *m_Tex = nullptr;
	LWTexture *m_RTex = nullptr;
	LWFont *m_ColorFont = nullptr;
	LWFont *m_MSDFFont = nullptr;


	LWVector2i m_PrevSize;
	LWVector2i m_PrevPos;
	float m_Theta = 0.0f;
	bool m_SizeChanged = true;
	bool m_Finished = false;
	bool m_UseMSDF = true;

};
int LWMain(int, char **){
	LWAllocator_Default *Allocator = new LWAllocator_Default();
	App *A = Allocator->Allocate<App>(*Allocator);
	
	if (A->m_Finished) {
		LWAllocator::Destroy(A);
		delete Allocator;
		return 0;
	}
	auto Foo = [](void *UserData)->bool {
		App *A = (App*)UserData;
		A->ProcessInput();
		A->Draw();
		if (!A->m_Finished) return !A->m_Finished;

		LWAllocator_Default *DefAlloc = (LWAllocator_Default*)&A->m_Allocator;
		LWAllocator::Destroy(A);
		delete DefAlloc;
		return !A->m_Finished;
	};

	LWRunLoop(Foo, LWTimer::GetResolution() / 60, A);
	return 0; 
}
