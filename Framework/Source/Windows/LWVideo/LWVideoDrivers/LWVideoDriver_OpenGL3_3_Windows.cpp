#include "LWVideo/LWVideoDrivers/LWVideoDriver_OpenGL3_3.h"
#include "LWPlatform/LWWindow.h"

LWVideoDriver_OpenGL3_3 *LWVideoDriver_OpenGL3_3::MakeVideoDriver(LWWindow *Window, uint32_t Type) {
	LWWindowContext WinCon = Window->GetContext();
	int32_t PixelFormat;
	PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0 };
	int32_t AttribList[] = { WGL_CONTEXT_MAJOR_VERSION_ARB, 3, WGL_CONTEXT_MINOR_VERSION_ARB, 2, 0, 0 };
	LWOpenGL3_3Context Context = { nullptr, nullptr };
	LWVideoDriver_OpenGL3_3 *Driver = nullptr;
	HGLRC GLContext = nullptr;
	if ((Context.m_DC = GetDC(WinCon.m_WND)) == nullptr)                   LWWindow::MakeDialog("Error: 'GetDC'", "ERROR", LWWindow::DialogOK);
	else if ((PixelFormat = ChoosePixelFormat(Context.m_DC, &pfd)) == 0)   LWWindow::MakeDialog("Error: 'ChoosePixelFormat'", "ERROR", LWWindow::DialogOK);
	else if (!SetPixelFormat(Context.m_DC, PixelFormat, &pfd))             LWWindow::MakeDialog("Error: 'SetPixelFormat'", "ERROR", LWWindow::DialogOK);
	else if ((GLContext = wglCreateContext(Context.m_DC)) == nullptr)      LWWindow::MakeDialog("Error: 'wglCreateContext'", "ERROR", LWWindow::DialogOK);
	else if (!wglMakeCurrent(Context.m_DC, GLContext))                     LWWindow::MakeDialog("Error: 'wglMakeCurrent'", "ERROR", LWWindow::DialogOK);
	else if (glewInit() != GLEW_OK)                                        LWWindow::MakeDialog("Error: 'glewInit'", "ERROR", LWWindow::DialogOK);
	else if (GLEW_VERSION_3_3) {
		if ((Context.m_GLRC = wglCreateContextAttribsARB(Context.m_DC, 0, AttribList)) == nullptr) LWWindow::MakeDialog("Error: 'wglCreateContextAttribsARB'", "ERROR", LWWindow::DialogOK);
		else {
			wglDeleteContext(GLContext);
			wglMakeCurrent(Context.m_DC, Context.m_GLRC);
			int32_t UniformBlockSize = 0;
			glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBlockSize);
			Driver = Window->GetAllocator()->Create<LWVideoDriver_OpenGL3_3>(Window, Context, (uint32_t)UniformBlockSize);
		}
	}
	if (!Driver) {
		if (GLContext) wglDeleteContext(GLContext);
		if (Context.m_GLRC) wglDeleteContext(Context.m_GLRC);
		if (Context.m_DC) ReleaseDC(WinCon.m_WND, Context.m_DC);
	}
	

	return Driver;
}


bool LWVideoDriver_OpenGL3_3::DestroyVideoContext(LWVideoDriver_OpenGL3_3 *Driver) {
	LWWindowContext &WinCon = Driver->GetWindow()->GetContext();
	LWOpenGL3_3Context &Con = Driver->GetContext();
	if (Con.m_GLRC) wglDeleteContext(Con.m_GLRC);
	if (Con.m_DC) ReleaseDC(WinCon.m_WND, Con.m_DC);
	LWAllocator::Destroy(Driver);
	return true;
}

bool LWVideoDriver_OpenGL3_3::Update(void) {
	return true;
}

LWVideoDriver &LWVideoDriver_OpenGL3_3::Present(uint32_t SwapInterval){
	wglSwapIntervalEXT(SwapInterval);
	SwapBuffers(m_Context.m_DC);
	return *this;
}