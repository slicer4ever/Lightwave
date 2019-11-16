#include "LWVideo/LWVideoDrivers/LWVideoDriver_OpenGLES2.h"
#include "LWPlatform/LWWindow.h"
#include <iostream>

LWVideoDriver_OpenGLES2 *LWVideoDriver_OpenGLES2::MakeVideoDriver(LWWindow *Window) {
	LWVideoDriver *Driver = nullptr;
	LWOpenGLES2Context Context;
	EAGLContext *GLContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
	if (!GLContext) {
		std::cout << "Failed to create gl context." << std::endl;
		return nullptr;
	}
	CGRect Bounds = LWAppContext.m_Window.bounds;
	Context.m_View = [[GLKView alloc] initWithFrame:Bounds context:GLContext];
	if (!Context.m_View) {
		std::cout << "Failed to create view context." << std::endl;
		return nullptr;
	}
	Context.m_View.drawableDepthFormat = GLKViewDrawableDepthFormat24;
	Context.m_View.drawableStencilFormat = GLKViewDrawableStencilFormat8;
	Context.m_View.enableSetNeedsDisplay = false;
    Context.m_View.autoresizingMask = UIViewAutoresizingFlexibleWidth|UIViewAutoresizingFlexibleHeight;
    Context.m_ViewChanged = false;
	[LWAppContext.m_Window addSubview : Context.m_View];
	[Context.m_View bindDrawable];
	int32_t DefaultFrameBuffer = 0;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &DefaultFrameBuffer);
	return Window->GetAllocator()->Allocate<LWVideoDriver_OpenGLES2>(Window, Context, DefaultFrameBuffer, 0);
}

bool LWVideoDriver_OpenGLES2::DestroyVideoContext(LWVideoDriver_OpenGLES2 *Driver) {
	LWAllocator::Destroy(Driver);
	return true;
}

bool LWVideoDriver_OpenGLES2::Update(void) {
    if(m_Window->GetFlag()&LWWindow::SizeChanged) m_Context.m_ViewChanged = true;
    bool Active = [EAGLContext setCurrentContext : m_Context.m_View.context];
    if(m_Window->GetFlag()&LWWindow::FocusChanged){
        if(m_Window->GetFlag()&LWWindow::Focused){
            [m_Context.m_View display];
            [m_Context.m_View bindDrawable];
            glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_DefaultFrameBuffer);
            m_ActiveFrameBuffer = nullptr;
        }else [m_Context.m_View deleteDrawable];
    }
    /*
    if(Active && m_Context.m_ViewChanged){
        std::cout << "Bounds has changed!" << std::endl;
        CGRect Bounds;
        Bounds.origin.x = 0;
        Bounds.origin.y = 0;
        Bounds.size.width = m_Window->GetSize().x;
        Bounds.size.height = m_Window->GetSize().y;
        [m_Context.m_View frame] = Bounds;
        m_Context.m_ViewChanged = false;
    }*/
    return Active;
}

LWVideoDriver &LWVideoDriver_OpenGLES2::Present(LWTexture *Texture, uint32_t SwapInterval) {
	[m_Context.m_View display];
	return *this;
}
