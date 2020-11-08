#include "LWVideo/LWVideoDrivers/LWVideoDriver_OpenGL2_1.h"
#include "LWPlatform/LWPlatform.h"
#include "LWPlatform/LWWindow.h"

LWVideoDriver_OpenGL2_1 *LWVideoDriver_OpenGL2_1::MakeVideoDriver(LWWindow *Window, uint32_t Type) {
	LWWindowContext WinCon = Window->GetContext();
	LWVideoDriver *Driver = nullptr;
	LWOpenGL2_1Context GLContext = { nullptr };
	int AttributeList[] = { GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 24, GLX_STENCIL_SIZE, 8,  0 };
	XVisualInfo *Visual = nullptr;
	if ((Visual = glXChooseVisual(WinCon.m_Display, 0, AttributeList)) == nullptr) return nullptr;
	if ((GLContext.m_GLContext = glXCreateContext(WinCon.m_Display, Visual, nullptr, true)) == 0) return nullptr;
	glXMakeCurrent(WinCon.m_Display, WinCon.m_Window, GLContext.m_GLContext);
	XFree(Visual);
	if (glewInit() != GLEW_OK) return nullptr;
	if (GLEW_VERSION_2_1) return Window->GetAllocator()->Create<LWVideoDriver_OpenGL2_1>(Window, GLContext, 1);

	glXMakeCurrent(WinCon.m_Display, 0, nullptr);
	glXDestroyContext(WinCon.m_Display, GLContext.m_GLContext);
	return nullptr;
}

/*! \brief deallocates and destroys the specified video context, and detaches it from the associated window. */
bool LWVideoDriver_OpenGL2_1::DestroyVideoContext(LWVideoDriver_OpenGL2_1 *Driver) {

	LWWindowContext WinCon = Driver->GetWindow()->GetContext();
	LWOpenGL2_1Context GLContext = Driver->GetContext();
	glXMakeCurrent(WinCon.m_Display, 0, nullptr);
	glXDestroyContext(WinCon.m_Display, GLContext.m_GLContext);
	LWAllocator::Destroy(Driver);

	return true;
}

bool LWVideoDriver_OpenGL2_1::Update(void) {
	return true;
}

LWVideoDriver &LWVideoDriver_OpenGL2_1::Present(LWTexture *Texture, uint32_t SwapInterval) {
	LWWindowContext WinCon = m_Window->GetContext();
#ifdef glXSwapIntervalSGI
	glXSwapIntervalSGI(SwapInterval);
#endif
#ifdef glXSwapIntervalMESA
	glXSwapIntervalMESA(SwapInterval);
#endif
	glXSwapBuffers(WinCon.m_Display, WinCon.m_Window);
	return *this;
}
