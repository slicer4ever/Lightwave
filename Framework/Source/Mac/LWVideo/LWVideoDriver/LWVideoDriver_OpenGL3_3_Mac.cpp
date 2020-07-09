#include "LWVideo/LWVideoDrivers/LWVideoDriver_OpenGL3_3.h"
#include "LWPlatform/LWPlatform.h"
#include "LWPlatform/LWWindow.h"
#import <Cocoa/Cocoa.h>
#include <iostream>

LWVideoDriver_OpenGL3_3 *LWVideoDriver_OpenGL3_3::MakeVideoDriver(LWWindow *Window, uint32_t Type) {
	LWWindowContext WinCon = Window->GetContext();
	LWOpenGL3_2Context Context;

	CGLPixelFormatAttribute CGAttribs[] = { kCGLPFADoubleBuffer, kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_3_2_Core, kCGLPFAColorSize, (CGLPixelFormatAttribute)24, kCGLPFAAlphaSize, (CGLPixelFormatAttribute)0, kCGLPFADepthSize, (CGLPixelFormatAttribute)24, kCGLPFAStencilSize, (CGLPixelFormatAttribute)8, (CGLPixelFormatAttribute)0 };
	CGLPixelFormatObj PixelFormat = nullptr;
	int32_t Count = 0;

	//First create a context to check it can support the OpenGL context we want.
	if (CGLChoosePixelFormat(CGAttribs, &PixelFormat, &Count) != kCGLNoError) return nullptr;
	NSOpenGLPixelFormat *PFormat = [[NSOpenGLPixelFormat alloc] initWithCGLPixelFormatObj:PixelFormat];
	NSRect Frame = [[WinCon.Window contentView] frame];
	NSOpenGLView *View = [[NSOpenGLView alloc] initWithFrame:Frame pixelFormat : PFormat];
	[[View openGLContext] makeCurrentContext];
	[WinCon.Window setContentView : View];
	int32_t UniformBlockSize = 0;
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UniformBlockSize);

	return Window->GetAllocator()->Allocate<LWVideoDriver_OpenGL3_3>(Window, Context, (uint32_t)UniformBlockSize);
} 

/*! \brief deallocates and destroys the specified video context, and detaches it from the associated window. */
bool LWVideoDriver_OpenGL3_3::DestroyVideoContext(LWVideoDriver_OpenGL3_3 *Driver) {
	LWOpenGL3_2Context &Context = Driver->GetContext();
	LWAllocator::Destroy(Driver);
	return true;
}

bool LWVideoDriver_OpenGL3_3::Update(void) {

	return true;
}

LWVideoDriver &LWVideoDriver_OpenGL3_3::Present(uint32_t SwapInterval){
	LWWindowContext &WinCon = m_Window->GetContext();
	NSOpenGLView *View = [WinCon.Window contentView];
	[[View openGLContext] setValues:(int32_t*)&SwapInterval forParameter : NSOpenGLCPSwapInterval];
    [[View openGLContext] flushBuffer];
	return *this;
}