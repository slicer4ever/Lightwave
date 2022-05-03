#include "LWPlatform/LWApplication.h"
#include "LWPlatform/LWPlatform.h"
#include "LWCore/LWTimer.h"
#include "LWCore/LWConcurrent/LWFIFO.h"
#include "LWCore/LWText.h"
#include "LWPlatform/LWInputDevice.h"
#include <atomic>
#include <thread>
#include <iostream>
LWIOSAppContext LWAppContext;

void PushEvent(uint32_t EventCode){
	LWAppContext.m_EventLoop.Push({ EventCode });
};

void PushSyncEvent(uint32_t EventCode, uint32_t Timeout){
	uint32_t Current = LWAppContext.m_EventState.load();
	LWAppContext.m_EventLoop.Push({ EventCode });
	uint64_t Start = LWTimer::GetCurrent();
	while (LWAppContext.m_EventState.load() == Current && (LWTimer::GetCurrent() - Start) <= Timeout) std::this_thread::yield();
};

bool LWRunLoop(std::function<bool(void*)> LoopFunc, uint64_t Frequency, void* UserData) {
	uint64_t Prev = LWTimer::GetCurrent();
	while (true) {
		uint64_t Curr = LWTimer::GetCurrent();
		if (Curr - Prev >= Frequency) {
			if (!LoopFunc(UserData)) break;
			Prev += Frequency;
		} else std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	return true;
}

@implementation LWIOSWindow

-(void)PushTouchEvent:(uint32_t)EventCode withEvent:(UIEvent*)event{
    LWIOSEvent TouchEvent;
    TouchEvent.m_EventCode = EventCode;
    TouchEvent.m_TouchCount = 0;
    NSSet *TouchSet = [event allTouches];
    
    for(UITouch *Point in TouchSet){
        CGPoint Pos = [Point locationInView:nil];
        
        TouchEvent.m_TouchPoints[TouchEvent.m_TouchCount] = LWVector2i((uint32_t)Pos.x, (uint32_t)Pos.y);
        TouchEvent.m_TouchState[TouchEvent.m_TouchCount] = (uint32_t)[Point phase];
        TouchEvent.m_TouchSize[TouchEvent.m_TouchCount] = (float)[Point majorRadius];
        TouchEvent.m_TouchCount++;
        if(TouchEvent.m_TouchCount>=LWMAXTOUCHPOINTS) break;
    }
    LWAppContext.m_EventLoop.Push(TouchEvent);
    return;
}

-(void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event{
    [self PushTouchEvent:(uint32_t)LWIOSEventCode::TouchDown withEvent:event];
    return [super touchesBegan:touches withEvent:event];
}

-(void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event{
    [self PushTouchEvent:(uint32_t)LWIOSEventCode::TouchMoved withEvent:event];
    return [super touchesMoved:touches withEvent:event];
}

-(void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event{
    [self PushTouchEvent:(uint32_t)LWIOSEventCode::TouchEnded withEvent:event];
    return [super touchesEnded:touches withEvent:event];
}

-(void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event{
    [self PushTouchEvent:(uint32_t)LWIOSEventCode::TouchEnded withEvent:event];
    return [super touchesCancelled:touches withEvent:event];
}
@end

@implementation MainAppDelegate

- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions{
    //Spawn our thread for the main application.
    auto MainApplication = [&self](){
        //LWAppContext.m_EventState++;
        LWMain(0, nullptr);
        LWAppContext.m_EventState++;
        return;
    };
    //Create the default window used by the application.
    LWAppContext.m_Applicaiton = [UIApplication sharedApplication];
    LWAppContext.m_EventState.store(0);
    LWAppContext.m_SyncState = false;
    LWAppContext.m_KeyboardTextField = nullptr;
	LWAppContext.m_MotionManager = [[CMMotionManager alloc] init];
    LWAppContext.m_Applicaiton.idleTimerDisabled = TRUE;
    CGRect Screen = [UIScreen mainScreen].bounds;
	CGFloat Scale = [[UIScreen mainScreen] scale];
    LWAppContext.m_Window = [[LWIOSWindow alloc] init];//Allocate the window context that can be used by the app.
    //std::cout << "Main screen: " << Screen.size.width << " " << Screen.size.height << " Scale: " << Scale << std::endl;
	CADisplayLink *DisplayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(update:)];
    [DisplayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSRunLoopCommonModes];
    NSNotificationCenter *DefCenter = [NSNotificationCenter defaultCenter];
    [DefCenter addObserver:self selector:@selector(keyboardOnScreen:) name:UIKeyboardDidShowNotification object:nil];
    
    std::thread Thread(MainApplication);
    Thread.detach();
    while(LWAppContext.m_EventState.load()==0) std::this_thread::yield();
    
    [LWAppContext.m_Window makeKeyAndVisible];
    return YES;
}

-(void)update:(CADisplayLink*)DisplayLink{
    //Update and process the application messages.
    if(!LWAppContext.m_KeyboardTextField){
        LWUITextField *Input = [[LWUITextField alloc] init];
        Input.hidden = true;
        NSString *S = [NSString stringWithCString:" " encoding:NSASCIIStringEncoding];
        [Input setText:S];
        UITextPosition *End = LWAppContext.m_KeyboardTextField.endOfDocument;
        Input.autocorrectionType = UITextAutocorrectionTypeNo;
        Input.autocapitalizationType = UITextAutocapitalizationTypeNone;
        [Input setSelectedTextRange:[Input textRangeFromPosition:End toPosition:End]];
        Input.delegate = Input;
        LWAppContext.m_KeyboardTextField = Input;
        [LWAppContext.m_Window.rootViewController.view addSubview:LWAppContext.m_KeyboardTextField];
    }
    LWIOSEvent Event;
    while(LWAppContext.m_AppLoop.Pop(Event)){
        if(Event.m_EventCode==(uint32_t)LWIOSEventCode::KeyboardOpen){
            [LWAppContext.m_KeyboardTextField becomeFirstResponder];
            //[LWAppContext.m_KeyboardTextField positionFromPosition:LWAppContext.m_KeyboardTextField.endOfDocument inDirection:UITextLayoutDirectionLeft offset:0];
            UITextRange *SelRange = LWAppContext.m_KeyboardTextField.selectedTextRange;
            NSInteger CursorPos = [LWAppContext.m_KeyboardTextField offsetFromPosition:LWAppContext.m_KeyboardTextField.beginningOfDocument toPosition:SelRange.start];
            //std::cout << "Text: '" << LWAppContext.m_KeyboardTextField.text << "' " << CursorPos<< std::endl;
            LWAppContext.m_EventLoop.Push(Event);
        }else if(Event.m_EventCode==(uint32_t)LWIOSEventCode::KeyboardClose){
            [LWAppContext.m_KeyboardTextField resignFirstResponder];
            LWAppContext.m_EventLoop.Push(Event);
        }
    }
    return;
}

-(void)applicationDidBecomeActive:(UIApplication *)application{
    PushEvent((uint32_t)LWIOSEventCode::FocusGained);
    return;
}

-(void)applicationWillResignActive:(UIApplication *)application{
    const uint32_t TimeOut = (uint32_t)(2*LWTimer::GetResolution());
    PushSyncEvent((uint32_t)LWIOSEventCode::FocusLost, TimeOut);
    return;
}

-(void)applicationWillTerminate:(UIApplication *)application{
    const uint32_t TimeOut = (uint32_t)(2*LWTimer::GetResolution());
    PushSyncEvent((uint32_t)LWIOSEventCode::Destroy, TimeOut);
    return;
}

-(void)application:(UIApplication *)application didChangeStatusBarOrientation:(UIInterfaceOrientation)oldStatusBarOrientation{
    PushEvent((uint32_t)LWIOSEventCode::OrientationChanged);
    return;
}

-(void)keyboardOnScreen:(NSNotification*)notification{
    NSDictionary *Info = notification.userInfo;
    NSValue *Value = Info[UIKeyboardFrameEndUserInfoKey];
    
    LWAppContext.m_KeyboardLayout = [Value CGRectValue];
    return;
}
    
@end
    
@implementation LWUITextField
-(BOOL)textField:(UITextField *)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string{
    //std::cout << "Changed: '" << [string UTF8String] << "' Len: " <<  [string length] << std::endl;
    if(string.length>0){
        LWUTF8Iterator C = LWUTF8Iterator((const char8_t*)[string UTF8String]);
        for(;!C.AtEnd();++C) {
            LWIOSEvent e;
            e.m_EventCode = LWIOSEventCode::CharEvent;
            e.m_TouchPoints[0].x =*C;
            LWAppContext.m_EventLoop.Push(e);
        }
    }else{
        LWIOSEvent e;
        e.m_EventCode = LWIOSEventCode::KeyEvent;
        e.m_TouchPoints[0].x = (uint32_t)LWKey::Back;
        LWAppContext.m_EventLoop.Push(e);
    }
    return NO;
}
    
@end
    
int main(int argc, char **argv){
    @autoreleasepool{
        return UIApplicationMain(argc, argv, nullptr, @"MainAppDelegate");
    }
}

bool LWExecute(const LWUTF8Iterator &BinaryPath, const LWUTF8Iterator &Parameters) {
	return false;
}

bool LWEmail(const LWUTF8Iterator &SrcEmail, const LWUTF8Iterator &TargetEmail, const LWUTF8Iterator &Subject, const LWUTF8Iterator &Body, const LWUTF8Iterator &SMTPServer, const LWUTF8Iterator &SMTPUsername, const LWUTF8Iterator &SMTPPassword){
	return false;
}

bool LWBrowser(const LWUTF8Iterator &URL) {
	return false;
}

float LWSystemScale(void) {
	return 1.0f;
}