#include "LWVideo/LWVideoDrivers/LWVideoDriver_OpenGL3_3.h"
#include "LWPlatform/LWPlatform.h"
#include "LWPlatform/LWWindow.h"

LWVideoDriver_OpenGL3_3 *LWVideoDriver_OpenGL3_3::MakeVideoDriver(LWWindow *Window, uint32_t Type) {
	LWWindowContext WinCon = Window->GetContext();
	LWVideoDriver *Driver = nullptr;
	LWOpenGL3_3Context GLContext = { nullptr };
	int AttributeList[] = { GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 24, GLX_STENCIL_SIZE, 8,  0 };
	XVisualInfo *Visual = nullptr;
	if ((Visual = glXChooseVisual(WinCon.m_Display, 0, AttributeList)) == nullptr) return nullptr;
	if ((GLContext.m_GLContext = glXCreateContext(WinCon.m_Display, Visual, nullptr, true)) == 0) return nullptr;
	glXMakeCurrent(WinCon.m_Display, WinCon.m_Window, GLContext.m_GLContext);
	XFree(Visual);
	if (glewInit() != GLEW_OK) return nullptr;
	if (GLEW_VERSION_3_3) {
		int32_t UniformBlockSize = 0;
		sglGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBlockSize);
		return Window->GetAllocator()->Allocate<LWVideoDriver_OpenGL3_3>(Window, GLContext, (uint32_t)UniformBlockSize);
	}
	glXMakeCurrent(WinCon.m_Display, 0, nullptr);
	glXDestroyContext(WinCon.m_Display, GLContext.m_GLContext);
	return nullptr;
}

bool LWVideoDriver_OpenGL3_3::DestroyVideoContext(LWVideoDriver_OpenGL3_3 *Driver) {

	LWWindowContext &WinCon = Driver->GetWindow()->GetContext();
	LWOpenGL3_3Context &GLContext = Driver->GetContext();
	glXMakeCurrent(WinCon.m_Display, 0, nullptr);
	glXDestroyContext(WinCon.m_Display, GLContext.m_GLContext);
	LWAllocator::Destroy(Driver);

	return true;
}

bool LWVideoDriver_OpenGL3_3::Update(void) {
	return true;
}

LWVideoDriver &LWVideoDriver_OpenGL3_3::Present(uint32_t SwapInterval){
	LWWindowContext &WinCon = m_Window->GetContext();
#ifdef glXSwapIntervalSGI
	glXSwapIntervalSGI(SwapInterval);
#endif
#ifdef glXSwapIntervalMESA
	glXSwapIntervalMESA(SwapInterval);
#endif
	glXSwapBuffers(WinCon.m_Display, WinCon.m_Window);
	return *this;
}

