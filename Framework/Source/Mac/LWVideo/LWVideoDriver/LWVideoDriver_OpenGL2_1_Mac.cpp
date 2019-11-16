#include "LWVideo/LWVideoDrivers/LWVideoDriver_OpenGL2_1.h"
#include "LWPlatform/LWPlatform.h"
#include "LWPlatform/LWWindow.h"
#import <Cocoa/Cocoa.h>

LWVideoDriver_OpenGL2_1 *LWVideoDriver_OpenGL2_1::MakeVideoDriver(LWWindow *Window) {
	LWWindowContext WinCon = Window->GetContext();
	LWOpenGL2_1Context Context;

	CGLPixelFormatAttribute CGAttribs[] = { kCGLPFADoubleBuffer, kCGLPFAColorSize, (CGLPixelFormatAttribute)24, kCGLPFAAlphaSize, (CGLPixelFormatAttribute)0, kCGLPFADepthSize, (CGLPixelFormatAttribute)24, kCGLPFAStencilSize, (CGLPixelFormatAttribute)8, (CGLPixelFormatAttribute)0 };
	CGLPixelFormatObj PixelFormat = nullptr;
	int32_t Count = 0;
	bool Valid = true;
	if (CGLChoosePixelFormat(CGAttribs, &PixelFormat, &Count) != kCGLNoError) return nullptr;
	NSOpenGLPixelFormat *PFormat = [[NSOpenGLPixelFormat alloc] initWithCGLPixelFormatObj:PixelFormat];
	NSRect Frame = [[WinCon.Window contentView] frame];
	NSOpenGLView *View = [[NSOpenGLView alloc] initWithFrame:Frame pixelFormat : PFormat];
	[[View openGLContext] makeCurrentContext];
	//[View prepareOpenGL];
	[WinCon.Window setContentView : View]; 

	return Window->GetAllocator()->Allocate<LWVideoDriver_OpenGL2_1>(Window, Context, 1);
}

/*! \brief deallocates and destroys the specified video context, and detaches it from the associated window. */
bool LWVideoDriver_OpenGL2_1::DestroyVideoContext(LWVideoDriver_OpenGL2_1 *Driver) {
	LWAllocator::Destroy(Driver);
	return true;
}

bool LWVideoDriver_OpenGL2_1::Update(void) {
	return true;
}

LWVideoDriver &LWVideoDriver_OpenGL2_1::Present(LWTexture *Texture, uint32_t SwapInterval) {
	LWWindowContext WinCon = m_Window->GetContext();
	NSOpenGLView *View = [WinCon.Window contentView];
	[[View openGLContext] setValues:(int32_t*)&SwapInterval forParameter : NSOpenGLCPSwapInterval];
	[[View openGLContext] flushBuffer];
	return *this;
}