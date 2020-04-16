#include "LWVideo/LWVideoDrivers/LWVideoDriver_OpenGL2_1.h"
#include "LWPlatform/LWWindow.h"

LWVideoDriver_OpenGL2_1 *LWVideoDriver_OpenGL2_1::MakeVideoDriver(LWWindow *Window, uint32_t Type) {
	
	LWWindowContext WinCon = Window->GetContext();
	int32_t PixelFormat;
	PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR), 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0 };

	LWOpenGL2_1Context Context = { nullptr, nullptr };
	LWVideoDriver_OpenGL2_1 *Driver = nullptr;
	if ((Context.m_DC = GetDC(WinCon.m_WND)) == nullptr)                   LWWindow::MakeDialog(LWText("Error: 'GetDC'"), LWText("ERROR"), LWWindow::DialogOK);
	else if ((PixelFormat = ChoosePixelFormat(Context.m_DC, &pfd)) == 0)   LWWindow::MakeDialog(LWText("Error: 'ChoosePixelFormat'"), LWText("ERROR"), LWWindow::DialogOK);
	else if (!SetPixelFormat(Context.m_DC, PixelFormat, &pfd))             LWWindow::MakeDialog(LWText("Error: 'SetPixelFormat'"), LWText("ERROR"), LWWindow::DialogOK);
	else if ((Context.m_GLRC = wglCreateContext(Context.m_DC)) == nullptr) LWWindow::MakeDialog(LWText("Error: 'wglCreateContext'"), LWText("ERROR"), LWWindow::DialogOK);
	else if (!wglMakeCurrent(Context.m_DC, Context.m_GLRC))                LWWindow::MakeDialog(LWText("Error: 'wglMakeCurrent'"), LWText("ERROR"), LWWindow::DialogOK);
	else if (glewInit() != GLEW_OK)                                        LWWindow::MakeDialog(LWText("Error: 'glewInit'"), LWText("ERROR"), LWWindow::DialogOK);
	else if (GLEW_VERSION_2_1)  Driver = Window->GetAllocator()->Allocate<LWVideoDriver_OpenGL2_1>(Window, Context, 1);
	if (!Driver) {
		if (Context.m_GLRC) wglDeleteContext(Context.m_GLRC);
		if (Context.m_DC) ReleaseDC(WinCon.m_WND, Context.m_DC);
	}
	return Driver;
}


bool LWVideoDriver_OpenGL2_1::DestroyVideoContext(LWVideoDriver_OpenGL2_1 *Driver) {
	LWWindowContext WinCon = Driver->GetWindow()->GetContext();
	LWOpenGL2_1Context Con = Driver->GetContext();
	if (Con.m_GLRC) wglDeleteContext(Con.m_GLRC);
	if (Con.m_DC) ReleaseDC(WinCon.m_WND, Con.m_DC);
	LWAllocator::Destroy(Driver);
	return false;
}

bool LWVideoDriver_OpenGL2_1::Update(void) {
	return true;
}

LWVideoDriver &LWVideoDriver_OpenGL2_1::Present(uint32_t SwapInterval) {
	wglSwapIntervalEXT(SwapInterval);
	SwapBuffers(m_Context.m_DC);
	return *this;
}