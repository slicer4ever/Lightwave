#include "LWPlatform/LWApplication.h"
#include "LWPlatform/LWPlatform.h"
#include <LWCore/LWUnicode.h>
#include <LWCore/LWTimer.h>
#include <ShellScalingApi.h>

void *LWSignal_UserData[LWSignal_Unknown];
LWSignalHandlerFunc LWSignal_Funcs[LWSignal_Unknown];
/*!< \brief enable dedicated gpu's if applicable. */
extern "C" {
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

bool LWSignal_Handler(int32_t Signal) {
	uint32_t LWSignalIDs[5] = { LWSignal_CtrlC, LWSignal_Break, LWSignal_Close, LWSignal_Logoff, LWSignal_Shutdown };
	if (Signal >= LWSignal_Unknown) return false;
	uint32_t ID = LWSignalIDs[Signal];
	if (ID >= LWSignal_Unknown) return false;
	if (!LWSignal_Funcs[ID]) return false;
	return LWSignal_Funcs[ID](Signal, LWSignal_UserData[ID]);
};

//Application entry points and other data is managed here.
int main(int argc, char **argv){
	const uint32_t MaxIterList = 32;
	LWUTF8Iterator IterList[MaxIterList];
	uint32_t Cnt = std::min<uint32_t>(argc, MaxIterList);
	for (uint32_t i = 0; i < Cnt; i++) IterList[i] = LWUTF8Iterator((const char8_t*)argv[i]);

	std::fill(LWSignal_UserData, LWSignal_UserData + LWSignal_Unknown, nullptr);
	std::fill(LWSignal_Funcs, LWSignal_Funcs + LWSignal_Unknown, nullptr);
	HWND ConsoleWnd = GetConsoleWindow();
	if (ConsoleWnd) SetWindowPos(ConsoleWnd, HWND_TOP, 1, 1, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)LWSignal_Handler, true);
	return LWMain(Cnt, IterList);
}

//The WinMain entry point for when the application is a windows subsystem, immediately call the applications entry point. */
int __stdcall WinMain(HINSTANCE, HINSTANCE, LPSTR, int CmdCnt){
	const uint32_t MaxResults = 32;
	LWUTF8Iterator ResultList[MaxResults];
	uint32_t CmndCnt = LWDecodeCommandLineArguments(LWUTF8Iterator((const char8_t*)GetCommandLine()), ResultList, MaxResults);

	std::fill(LWSignal_UserData, LWSignal_UserData + LWSignal_Unknown, nullptr);
	std::fill(LWSignal_Funcs, LWSignal_Funcs + LWSignal_Unknown, nullptr);
	SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)LWSignal_Handler, true);

	return LWMain(0, nullptr);
}

void LWRegisterSignal(LWSignalHandlerFunc Handler, uint32_t Signal, void *UserData){
	LWSignal_UserData[Signal] = UserData;
	LWSignal_Funcs[Signal] = Handler;
	return;
}


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

bool LWExecute(const LWUTF8Iterator &BinaryPath, const LWUTF8Iterator &Parameters) {
	HINSTANCE I = ShellExecute(NULL, "OPEN", *BinaryPath.c_str<256>(), *Parameters.c_str<256>(), nullptr, SW_SHOWDEFAULT);
	return I !=  nullptr;
}

bool LWEmail(const LWUTF8Iterator &SrcEmail, const LWUTF8Iterator &TargetEmail, const LWUTF8Iterator &Subject, const LWUTF8Iterator &Body, const LWUTF8Iterator &SMTPServer, const LWUTF8Iterator &SMTPUsername, const LWUTF8Iterator &SMTPPassword){
	auto Mail = LWUTF8I::Fmt<1024*16>(
		"$S = New-Object Net.Mail.MailMessage('{}', '{}', '{}', '{}');"
		"$S.IsBodyHTML = $true;"
		"$SC = New-Object Net.Mail.SmtpClient('{}', 587);"
		"$SC.EnableSSL=$true;"
		"$SC.Credentials = New-Object Net.NetworkCredential('{}', '{}');"
		"$SC.Send($S);", SrcEmail, TargetEmail, Subject, SMTPServer, SMTPUsername, SMTPPassword);
	HINSTANCE I = ShellExecute(NULL, "OPEN", "powershell", *Mail, nullptr, SW_HIDE);
	return I != nullptr;
}

bool LWBrowser(const LWUTF8Iterator &URL) {
	HINSTANCE I = ShellExecute(NULL, "OPEN", *URL.c_str<256>(), nullptr, nullptr, SW_SHOW);
	return I!=nullptr;
}

float LWSystemScale(void) {
	uint32_t DPI = GetDpiForSystem();
	return (float)DPI / 96.0f;
}