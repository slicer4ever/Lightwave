#include "LWVideo/LWVideoDrivers/LWVideoDriver_OpenGL4_5.h"
#include "LWPlatform/LWWindow.h"
#include "LWCore/LWLogger.h"
#include <iostream>

LWVideoDriver_OpenGL4_5 *LWVideoDriver_OpenGL4_5::MakeVideoDriver(LWWindow *Window, uint32_t Type) {
	auto DebugOutput = [](GLenum Source, GLenum Type, GLuint ID, GLenum Severity, GLsizei Length, const char *Message, const void *UserDAta) {
		const GLenum SourceEnums[] = { GL_DEBUG_SOURCE_API, GL_DEBUG_SOURCE_WINDOW_SYSTEM, GL_DEBUG_SOURCE_SHADER_COMPILER, GL_DEBUG_SOURCE_THIRD_PARTY, GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_SOURCE_OTHER };
		const GLenum TypeEnums[] = { GL_DEBUG_TYPE_ERROR, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, GL_DEBUG_TYPE_PORTABILITY, GL_DEBUG_TYPE_PERFORMANCE, GL_DEBUG_TYPE_MARKER, GL_DEBUG_TYPE_PUSH_GROUP, GL_DEBUG_TYPE_POP_GROUP, GL_DEBUG_TYPE_OTHER };
		const GLenum SeverityEnums[] = { GL_DEBUG_SEVERITY_HIGH, GL_DEBUG_SEVERITY_MEDIUM, GL_DEBUG_SEVERITY_LOW, GL_DEBUG_SEVERITY_NOTIFICATION };
		const char *SourceNames[] = { "API", "Window System", "Shader Compiler", "Third Party", "Application", "Other" };
		const char *TypeNames[] = { "Error", "Deprecated behavior", "Undefined behavior", "Portability", "Performance", "Marker", "Push Group", "Pop Group", "Other" };
		const char *SeverityNames[] = { "High", "Medium", "Low", "Notification" };
		const uint32_t SourceCnt = sizeof(SourceEnums) / sizeof(GLenum);
		const uint32_t TypeCnt = sizeof(TypeEnums) / sizeof(GLenum);
		const uint32_t SeverityCnt = sizeof(SeverityEnums) / sizeof(GLenum);
		uint32_t SourceID = 0;
		uint32_t TypeID = 0;
		uint32_t SeverityID = 0;
		for (; SourceID < SourceCnt && SourceEnums[SourceID] != Source; SourceID++) {}
		for (; TypeID < TypeCnt && TypeEnums[TypeID] != Type; TypeID++) {}
		for (; SeverityID < SeverityCnt && SeverityEnums[SeverityID] != Severity; SeverityID++) {}
		LWLogEvent<256>("({},{},{},{}): '{}'", SourceNames[SourceID], TypeNames[TypeID], SeverityNames[SeverityID], ID, Message);
		LWVerify(TypeID != 0);
		return;
	};

	LWWindowContext WinCon = Window->GetContext();
	int32_t PixelFormat;
	PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0 };
	int32_t AttribList[] = { WGL_CONTEXT_MAJOR_VERSION_ARB, 3, WGL_CONTEXT_MINOR_VERSION_ARB, 2, 0, 0 };
	LWOpenGL4_5Context Context = { nullptr, nullptr };
	LWVideoDriver_OpenGL4_5 *Driver = nullptr;
	HGLRC GLContext = nullptr;
	if ((Context.m_DC = GetDC(WinCon.m_WND)) == nullptr)                   LWWindow::MakeDialog("Error: 'GetDC'", "ERROR", LWWindow::DialogOK);
	else if ((PixelFormat = ChoosePixelFormat(Context.m_DC, &pfd)) == 0)   LWWindow::MakeDialog("Error: 'ChoosePixelFormat'", "ERROR", LWWindow::DialogOK);
	else if (!SetPixelFormat(Context.m_DC, PixelFormat, &pfd))             LWWindow::MakeDialog("Error: 'SetPixelFormat'", "ERROR", LWWindow::DialogOK);
	else if ((GLContext = wglCreateContext(Context.m_DC)) == nullptr)      LWWindow::MakeDialog("Error: 'wglCreateContext'", "ERROR", LWWindow::DialogOK);
	else if (!wglMakeCurrent(Context.m_DC, GLContext))                     LWWindow::MakeDialog("Error: 'wglMakeCurrent'", "ERROR", LWWindow::DialogOK);
	else if (glewInit() != GLEW_OK)                                        LWWindow::MakeDialog("Error: 'glewInit'", "ERROR", LWWindow::DialogOK);
	else if (GLEW_VERSION_4_4) {
		if ((Context.m_GLRC = wglCreateContextAttribsARB(Context.m_DC, 0, AttribList)) == nullptr) LWWindow::MakeDialog("Error: 'wglCreateContextAttribsARB'", "ERROR", LWWindow::DialogOK);
		else {
			wglDeleteContext(GLContext);
			wglMakeCurrent(Context.m_DC, Context.m_GLRC);
			int32_t UniformBlockSize = 0;
			glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBlockSize);
			if ((Type&DebugLayer) != 0) {
				glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
				glEnable(GL_DEBUG_OUTPUT);
				glDebugMessageCallback(DebugOutput, nullptr);
				glDebugMessageInsert(GL_DEBUG_SOURCE_THIRD_PARTY, GL_DEBUG_TYPE_OTHER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, (GLsizei)strlen("DebugLayer Initiated."), "DebugLayer Initiated.");
			}
			Driver = Window->GetAllocator()->Create<LWVideoDriver_OpenGL4_5>(Window, Context, (uint32_t)UniformBlockSize);
		}
	}
	if (!Driver) {
		if (GLContext) wglDeleteContext(GLContext);
		if (Context.m_GLRC) wglDeleteContext(Context.m_GLRC);
		if (Context.m_DC) ReleaseDC(WinCon.m_WND, Context.m_DC);
	}


	return Driver;
}


bool LWVideoDriver_OpenGL4_5::DestroyVideoContext(LWVideoDriver_OpenGL4_5 *Driver) {
	LWWindowContext WinCon = Driver->GetWindow()->GetContext();
	LWOpenGL4_5Context Con = Driver->GetContext();
	if (Con.m_GLRC) wglDeleteContext(Con.m_GLRC);
	if (Con.m_DC) ReleaseDC(WinCon.m_WND, Con.m_DC);
	LWAllocator::Destroy(Driver);
	return true;
}

bool LWVideoDriver_OpenGL4_5::Update(void) {
	return true;
}

LWVideoDriver &LWVideoDriver_OpenGL4_5::Present(uint32_t SwapInterval) {
	wglSwapIntervalEXT(SwapInterval);
	SwapBuffers(m_Context.m_DC);
	return *this;
}