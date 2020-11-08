#include <LWCore/LWTimer.h>
#include "LWVideo/LWVideoDrivers/LWVideoDriver_OpenGLES2.h"
#include "LWPlatform/LWWindow.h"
#include <iostream>


LWVideoDriver_OpenGLES2 *LWVideoDriver_OpenGLES2::MakeVideoDriver(LWWindow *Window, uint32_t Type) {
	LWVideoDriver *Driver = nullptr;
	LWWindowContext WinCon = Window->GetContext();
	LWOpenGLES2Context ESContext;
	//Wait until we have a native window.
	while (WinCon.m_ActiveWindow == nullptr) {
		if (Window->Update(LWTimer::GetCurrent()).GetFlag()&LWWindow::Terminate) {
			LWWindow::MakeDialog("Error window terminating before display found.", "ERROR", LWWindow::DialogOK);
			return nullptr;
		}
		WinCon = Window->GetContext();
	}

	const GLint ES2Attribs[] = { EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_DEPTH_SIZE, 24, EGL_ALPHA_SIZE, 8, EGL_RED_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_NONE };
	const GLint ES2MinAttribs[] = { EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_RED_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_NONE };
	const GLint ContextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
	ESContext.m_Display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	int ConfigCount = 0;
	if (!ESContext.m_Display) {
		LWWindow::MakeDialog("Error Display not found.", "ERROR", LWWindow::DialogOK);
		return nullptr;
	}
	if (!eglInitialize(ESContext.m_Display, nullptr, nullptr)) {
		LWWindow::MakeDialog("Error Initializing openGL", "ERROR", LWWindow::DialogOK);
		return nullptr;
	}
	if (!eglChooseConfig(ESContext.m_Display, ES2Attribs, &ESContext.m_DisplayConfig, 1, &ConfigCount)) {
		LWWindow::MakeDialog("Error configuring display.", "ERROR", LWWindow::DialogOK);
		return nullptr;
	}

	if (ConfigCount == 0) {
		if (!eglChooseConfig(ESContext.m_Display, ES2MinAttribs, &ESContext.m_DisplayConfig, 1, &ConfigCount)) {
			LWWindow::MakeDialog("Error configuring min display.", "ERROR", LWWindow::DialogOK);
			return nullptr;
		}
		if (ConfigCount == 0) {
			LWWindow::MakeDialog("Error no configuration found.", "ERROR", LWWindow::DialogOK);
			return nullptr;
		}
	}
	GLint Format = 0;
	if (!eglGetConfigAttrib(ESContext.m_Display, ESContext.m_DisplayConfig, EGL_NATIVE_VISUAL_ID, &Format)) {
		LWWindow::MakeDialog("Error in selecting display configuration.", "ERROR", LWWindow::DialogOK);
		return nullptr; //failed to get our config id.
	}

	ANativeWindow_setBuffersGeometry(WinCon.m_ActiveWindow, 0, 0, Format);
	ESContext.m_Context = eglCreateContext(ESContext.m_Display, ESContext.m_DisplayConfig, nullptr, ContextAttribs);
	if (!ESContext.m_Context) {
		LWWindow::MakeDialog("Error in creating context.", "ERROR", LWWindow::DialogOK);
		return nullptr;
	}
	ESContext.m_Surface = eglCreateWindowSurface(ESContext.m_Display, ESContext.m_DisplayConfig, WinCon.m_ActiveWindow, nullptr);
	if (!ESContext.m_Surface) {
		LWWindow::MakeDialog("Error in creating window surface.", "ERROR", LWWindow::DialogOK);
		eglDestroyContext(ESContext.m_Display, ESContext.m_Context);
		return nullptr;
	}
	ESContext.m_ActiveWindow = WinCon.m_ActiveWindow;
	eglMakeCurrent(ESContext.m_Display, ESContext.m_Surface, ESContext.m_Surface, ESContext.m_Context);
	int32_t DefaultFrameBuffer = 0;
	return Window->GetAllocator()->Create<LWVideoDriver_OpenGLES2>(Window, ESContext, DefaultFrameBuffer, 1);
}

bool LWVideoDriver_OpenGLES2::DestroyVideoContext(LWVideoDriver_OpenGLES2 *Driver) {
	LWOpenGLES2Context Con = Driver->GetContext();
	eglMakeCurrent(Con.m_Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	if (Con.m_Surface) eglDestroySurface(Con.m_Display, Con.m_Surface);
	eglDestroyContext(Con.m_Display, Con.m_Context);
	eglTerminate(Con.m_Display);
	LWAllocator::Destroy(Driver);
	return true;
}

bool LWVideoDriver_OpenGLES2::Update(void) {
	LWWindowContext WinCon = m_Window->GetContext();
	if (m_Context.m_ActiveWindow != WinCon.m_ActiveWindow) {
		if (WinCon.m_ActiveWindow) {
			GLint Format = 0;
			if (!eglGetConfigAttrib(m_Context.m_Display, m_Context.m_DisplayConfig, EGL_NATIVE_VISUAL_ID, &Format)) return false; //failed to get our config id.
			ANativeWindow_setBuffersGeometry(WinCon.m_ActiveWindow, 0, 0, Format);
			m_Context.m_Surface = eglCreateWindowSurface(m_Context.m_Display, m_Context.m_DisplayConfig, WinCon.m_ActiveWindow, nullptr);
			if (!eglMakeCurrent(m_Context.m_Display, m_Context.m_Surface, m_Context.m_Surface, m_Context.m_Context)) return false;
		} else {
			eglDestroySurface(m_Context.m_Display, m_Context.m_Surface);
			m_Context.m_Surface = nullptr;
			eglMakeCurrent(m_Context.m_Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		}
		m_Context.m_ActiveWindow = WinCon.m_ActiveWindow;
	}
	if ((m_Window->GetSize().x <= 1 && m_Window->GetSize().y <= 1)) return false;
	return (m_Context.m_ActiveWindow != nullptr);

}

LWVideoDriver &LWVideoDriver_OpenGLES2::Present(LWTexture *Texture, uint32_t SwapInterval) {
	eglSwapInterval(m_Context.m_Display, SwapInterval);
	eglSwapBuffers(m_Context.m_Display, m_Context.m_Surface);
	return *this;
}