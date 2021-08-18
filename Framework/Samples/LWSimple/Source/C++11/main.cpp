#include "LWCore/LWTypes.h"
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
	

	void SetFinished(const LWUTF8Iterator &Error) {
		fmt::print("Error: {}\n", Error);
		return;
	}	

	void ProcessInput(void) {
		m_Window->Update(LWTimer::GetCurrent());
		m_AudioDriver->Update(LWTimer::GetCurrent(), m_Window);
		if (m_Window->GetFlag()&LWWindow::Terminate) m_Finished = true;
		if (m_Window->GetFlag()&LWWindow::OrientationChanged) {
			fmt::print("Orientation has changed: {}\n", ((m_Window->GetOrientation() == LWWindow::Rotation_0) ? "Portrait" : ((m_Window->GetOrientation() == LWWindow::Rotation_90) ? "Landscape" : ((m_Window->GetOrientation() == LWWindow::Rotation_180) ? "Upside down portrait" : "Upside down landscape"))));
		}
		if (m_Window->GetFlag() & LWWindow::FocusChanged) fmt::print("Focus has changed: {}\n", ((m_Window->GetFlag() & LWWindow::Focused) ? "Current" : "Not Current"));
		if (m_Window->GetFlag() & LWWindow::PosChanged) fmt::print("Position has changed: {}\n", m_Window->GetPosition());
		LWMouse *Mouse = m_Window->GetMouseDevice();
		LWKeyboard *KB = m_Window->GetKeyboardDevice();
		LWTouch *TD = m_Window->GetTouchDevice();
		if (Mouse) {
			if (Mouse->ButtonDown(LWMouseKey::Left)) fmt::print("Left mouse down at: {} Scroll: {}\n", Mouse->GetPosition(), Mouse->GetScroll());
			if (Mouse->ButtonPressed(LWMouseKey::Left)) fmt::print("Left mouse pressed.\n");
			if (Mouse->ButtonReleased(LWMouseKey::Left)) fmt::print("Left mouse released!\n");
			if (Mouse->ButtonDown(LWMouseKey::Right)) fmt::print("Right mouse down!\n");
			if (Mouse->ButtonPressed(LWMouseKey::Right)) fmt::print("Right mouse pressed!\n");
			if (Mouse->ButtonReleased(LWMouseKey::Right)) fmt::print("Right mouse released!\n");
			if (Mouse->ButtonDown(LWMouseKey::Middle)) fmt::print("Middle mouse down!\n");
			if (Mouse->ButtonPressed(LWMouseKey::Middle)) fmt::print("Middle mouse pressed!\n");
			if (Mouse->ButtonReleased(LWMouseKey::Middle)) fmt::print("Middle mouse released!\n");
			if (Mouse->ButtonDown(LWMouseKey::X1)) fmt::print("X1 mouse down!\n");
			if (Mouse->ButtonPressed(LWMouseKey::X1)) fmt::print("X1 mouse pressed!\n");
			if (Mouse->ButtonReleased(LWMouseKey::X1)) fmt::print("X1 mouse released!\n");
			if (Mouse->ButtonDown(LWMouseKey::X2)) fmt::print("X2 mouse down!\n");
			if (Mouse->ButtonPressed(LWMouseKey::X2)) fmt::print("X2 mouse pressed!\n");
			if (Mouse->ButtonReleased(LWMouseKey::X2)) fmt::print("X2 mouse released!\n");
		}
		if (KB) {
			for (uint32_t i = 0; i<KB->GetKeyChangeCount(); i++) {
				uint32_t Key = KB->GetKeyChanged(i);
				fmt::print("Key: {} is: {}\n", Key, KB->ButtonDown(Key));
			}
			for (uint32_t i = 0; i<KB->GetCharPressed(); i++) {
				uint32_t Char = KB->GetChar(i);
				fmt::print("Char: {}\n", (char)Char);
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
				fmt::print("Point: {} State: {} Init: {} DownTime: {} Size: {}\n", Pnt.m_Position, Pnt.m_State, Pnt.m_InitPosition, Pnt.m_DownTime, Pnt.m_Size);
				if (Pnt.m_State == LWTouchPoint::DOWN) m_UseMSDF = !m_UseMSDF;
			}
			const LWGesture &Gest = TD->GetGesture();
			if (Gest.m_Type != LWGesture::None) {
				fmt::print("Gesture: {} Source: {} Dir: {} Scale: {}\n", Gest.m_Type, Gest.m_Source, Gest.m_Direction, Gest.m_Scale);
			}
		}
	}

	void Draw(void) {
		if (m_Window->SizeUpdated()) m_SizeChanged = true;
		
		if (!m_Driver->Update()) return;
		
		if (m_SizeChanged) {
			fmt::print("Size has changed: {}\n", m_Window->GetSize());
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
		const char *DriverNames[] = LWVIDEODRIVER_NAMES;
		const char *ArchNames[] = LWARCH_NAMES;
		const char *PlatformNames[] = LWPLATFORM_NAMES;

		m_Stream = LWAudioStream::Create(u8"App:SampleSound.ogg", 0, Allocator);

		if(!m_Stream){
			SetFinished("Error opening sound: 'App:SampleSound.ogg'");
			return;
		}
		auto FinishedCallback = [](LWSound *S, LWAudioDriver *AD) {
			fmt::print("Finished: {} {}\n", S->GetFinishedCount(), S->GetPlayCount());
			if(S->isFinished()) {
				fmt::print("Releasing sound.\n");
				S->Release();
			}
		};

		auto CreateCallback = [](LWSound *S, LWAudioDriver *AD) {
			LWAudioStream *Stream = S->GetAudioStream();
			fmt::print("Sound Created: {}s\n", LWSound::CalculateTime(Stream->GetSampleLength(), Stream->GetSampleRate()));
		};

		auto ReleaseCallback = [](LWSound *S, LWAudioDriver *AD) {
			fmt::print("Sound released.\n");
		};
		m_AudioDriver = Allocator.Create<LWAudioDriver>(nullptr, m_Allocator, FinishedCallback, CreateCallback, ReleaseCallback);

		if (m_AudioDriver->GetFlag()&LWAudioDriver::Error) {
			SetFinished("Error creating audio driver.");
			return;
		}

		LWSound *Snd = m_AudioDriver->CreateSound2D(m_Stream, nullptr, 0, true, 3 );
		if (!Snd) {
			fmt::print("Failed to create sound...\n");
		} else {
			fmt::print("Sound created!\n");
		}
		//AudioStream.Finished();*/
		//while ((Wnd->Update(LWTimer::GetCurrent()).GetFlag()&LWWindow::Terminate) == 0 && !KB->ButtonDown(LWKey::Esc)){
		//if (Wnd->GetFlag()&LWWindow::SizeChanged) SizeChanged = true;
		/*if (!ADriver->Update(LWTimer::GetCurrent(), Wnd)) {
		std::this_thread::yield();
		continue;
		}*/
		//}
		auto OutputMode = [](const LWVideoMode &Mode) {
			uint32_t ColoredMode = Mode.GetColorMode();
			uint32_t Rotation = Mode.GetRotation();
			fmt::print("Mode: {} Rate: {} Interlaced: {} | Colored Mode: {} | Rotation: {} Flag: {}\n", 
				Mode.GetSize(), Mode.GetFrequency(), Mode.isInterlaced(), ((ColoredMode == LWVideoMode::Colored4Bit) ? u8"4 Bit Colored" : ((ColoredMode == LWVideoMode::Colored8Bit) ? u8"8 Bit Colored" : ((ColoredMode == LWVideoMode::Colored16Bit) ? u8"16 Bit Colored" : ((ColoredMode == LWVideoMode::Colored32Bit) ? u8"32 Bit Colored" : u8"Unknown colored mode")))), ((Rotation == LWVideoMode::Rotation_0) ? u8"0" : ((Rotation == LWVideoMode::Rotation_90) ? u8"90" : ((Rotation == LWVideoMode::Rotation_180) ? u8"180" : u8"270"))), Mode.GetFlag());
		};
		LWVideoMode Active = LWVideoMode::GetActiveMode();

		fmt::print("Active ");
		OutputMode(Active);
		uint32_t AvailableModes = LWVideoMode::GetAllDisplayModes(nullptr, 0);
		LWVideoMode *Modes = Allocator.Allocate<LWVideoMode>(AvailableModes);
		uint32_t nModes = LWVideoMode::GetAllDisplayModes(Modes, AvailableModes);
		if (nModes != AvailableModes) {
			SetFinished(LWUTF8Iterator::C_View<256>("Error incorrect mode list encountered {}, Expected: {}\n", nModes, AvailableModes));
			return;
		}
		fmt::print("Modes: {}\n", AvailableModes);
		for (uint32_t i = 0; i < AvailableModes; i++) {
			fmt::print("i: ");
			OutputMode(Modes[i]);
		}
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

		m_Window = m_Allocator.Create<LWWindow>("LWPlatform Testing suite.", "LWPlatformTest", m_Allocator, LWWindow::WindowedMode | LWWindow::KeyboardDevice | LWWindow::MouseDevice | LWWindow::TouchDevice, Active.GetSize() / 2 - Size / 2, Size);
		if (m_Window->GetFlag()&LWWindow::Error) {
			SetFinished("Error creating window!");
			return;
		}
		//uint32_t TargetDriver = LWVideoDriver::Vulkan | LWVideoDriver::DebugLayer;
		uint32_t TargetDriver = LWVideoDriver::OpenGL2_1;
		fmt::print("Window created: {} Arch: {} Platform: {}\n", m_Window->GetSize(), ArchNames[LWARCH_ID], PlatformNames[LWPLATFORM_ID]);
		m_Driver = LWVideoDriver::MakeVideoDriver(m_Window, TargetDriver);
		if (!m_Driver) {
			SetFinished("Creating video driver.");
			return;
		}
		fmt::print("Driver created: {}\n", DriverNames[m_Driver->GetDriverID()]);
		uint32_t CompiledLen = 0;
		m_DefaultVertexShader = m_Driver->LoadShader(LWShader::Vertex, "App:DefaultShader.vlws", Allocator, 0, nullptr, nullptr, ErrorBuffer, CompiledLen, sizeof(ErrorBuffer));
		if (!m_DefaultVertexShader) {
			SetFinished(LWUTF8Iterator::C_View<256>("Error loading vertex shader:\n{}\n", ErrorBuffer));
			return;
		}
		m_DefaultVertexShader->SetInputMapList("Position", LWShaderInput::Vec4, 1, "TexCoord", LWShaderInput::Vec4, 1);
		m_DefaultPixelShader = m_Driver->LoadShader(LWShader::Pixel, "App:DefaultShader.plws", Allocator, 0, nullptr, nullptr, ErrorBuffer, CompiledLen, sizeof(ErrorBuffer));
		if (!m_DefaultPixelShader) {
			SetFinished(LWUTF8Iterator::C_View<256>("Error loading pixel shader:\n{}\n", ErrorBuffer));
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
		if (!LWImage::LoadImage(Image, "App:Sample.png", Allocator)) {
			SetFinished("Error: loading image.");
			return;
		}
		fmt::print("Size: {} PackType: {}\n", Image.GetSize2D(), Image.GetPackType());
		
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
		m_Window->SetTitle(LWUTF8Iterator::C_View<256>("LWPlatform Simple example. | {} | {} | {}", DriverNames[m_Driver->GetDriverID()], PlatformNames[LWPLATFORM_ID], ArchNames[LWARCH_ID]));

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
			SetFinished("opening Junicode-Regular.ttf");
			return;
		}
		m_ColorFont = LWFont::LoadFontTTF(&FontColorStream, m_Driver, 32, 32, 96, Allocator);
		if (!m_ColorFont) {
			SetFinished("loading ttf font.");
			return;
		}

		LWFileStream MSDFStream;
		if (!LWFileStream::OpenStream(MSDFStream, "App:Arial.arfont", LWFileStream::ReadMode | LWFileStream::BinaryMode, Allocator, nullptr)) {
			SetFinished("Error opening Arial.arfont");
			return;
		}
		m_MSDFFont = LWFont::LoadFontAR(&MSDFStream, m_Driver, Allocator);

		if (!m_MSDFFont) {
			SetFinished("Error loading Arial.arfont");
			return;
		}

		m_FontVertexShader = m_Driver->ParseShader(LWShader::Vertex, LWShaderSources[LWShaderFontVertex], Allocator, 0, nullptr, nullptr, ErrorBuffer, CompiledLen, sizeof(ErrorBuffer));
		if (!m_FontVertexShader) {
			SetFinished(LWUTF8Iterator::C_View<256>("Error creating Font vertex shader:\n{}\n", ErrorBuffer));
			return;
		}
		m_FontVertexShader->SetInputMapList("Position", LWShaderInput::Vec4, 1, "Color", LWShaderInput::Vec4, 1, "TexCoord", LWShaderInput::Vec4, 1);

		m_FontColorShader = m_Driver->ParseShader(LWShader::Pixel, LWShaderSources[LWShaderFontColor], Allocator, 0, nullptr, nullptr, ErrorBuffer, CompiledLen, sizeof(ErrorBuffer));
		if (!m_FontColorShader) {
			SetFinished(LWUTF8Iterator::C_View<256>("Error creating Color pixel shader:\n{}\n", ErrorBuffer));
			return;
		}

		m_FontMSDFShader = m_Driver->ParseShader(LWShader::Pixel, LWShaderSources[LWShaderFontMSDF], Allocator, 0, nullptr, nullptr, ErrorBuffer, CompiledLen, sizeof(ErrorBuffer));
		if (!m_FontMSDFShader) {
			SetFinished(LWUTF8Iterator::C_View<256>("Error creating msdf pixel shader:\n{}\n", ErrorBuffer));
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

int LWMain(int, LWUTF8Iterator *){
	LWAllocator_Default *Allocator = new LWAllocator_Default();
	App *A = Allocator->Create<App>(*Allocator);
	
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
