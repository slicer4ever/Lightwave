#include "LWPlatform/LWApplication.h"
#include "LWPlatform/LWPlatform.h"
#include <LWCore/LWText.h>
#include <LWCore/LWTimer.h>
#include <ShellScalingApi.h>



void *LWSignal_UserData[LWSignal_Unknown];
LWSignalHandlerFunc LWSignal_Funcs[LWSignal_Unknown];


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
	std::fill(LWSignal_UserData, LWSignal_UserData + LWSignal_Unknown, nullptr);
	std::fill(LWSignal_Funcs, LWSignal_Funcs + LWSignal_Unknown, nullptr);
	HWND ConsoleWnd = GetConsoleWindow();
	if (ConsoleWnd) SetWindowPos(ConsoleWnd, HWND_TOP, 1, 1, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)LWSignal_Handler, true);
	return LWMain(argc, argv);
}

//The WinMain entry point for when the application is a windows subsystem, immediately call the applications entry point. */
int __stdcall WinMain(HINSTANCE, HINSTANCE, LPSTR, int CmdCnt){
	const uint32_t MaxResults = 32;
	const uint32_t BufferLen = 1024;
	char Buffer[BufferLen];
	char *Results[MaxResults];
	char *Command = GetCommandLine();
	uint32_t CmndCnt = 0;
	uint32_t o = 0;
	for (char *C = LWText::NextWord(Command, true); o<BufferLen && C && CmndCnt<MaxResults; C = LWText::NextWord(C, true)) {
		char *P = C;
		if (*C == '\"') {
			C = LWText::CopyToTokens(P + 1, Buffer + o, BufferLen - o, "\"");
			Results[CmndCnt++] = Buffer + o;
			o += (uint32_t)(uintptr_t)(C - (P + 1)) + 1;
			C = LWText::FirstToken(C, ' ');
			continue;
		}
		if (*C == '\'') {
			C = LWText::CopyToTokens(P + 1, Buffer + o, BufferLen - o, "\'");
			Results[CmndCnt++] = Buffer + o;
			o += (uint32_t)(uintptr_t)(C - (P + 1)) + 1;
			C = LWText::FirstToken(C, ' ');
			continue;

		}
		C = LWText::CopyToTokens(C, Buffer + o, BufferLen - o, " ");
		Results[CmndCnt++] = Buffer + o;
		o += (uint32_t)(uintptr_t)(C - P) + 1;
	}
	//for (uint32_t i = 0; i < CmndCnt; i++) std::cout << i << ": " << Results[i] << std::endl;

	std::fill(LWSignal_UserData, LWSignal_UserData + LWSignal_Unknown, nullptr);
	std::fill(LWSignal_Funcs, LWSignal_Funcs + LWSignal_Unknown, nullptr);
	SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)LWSignal_Handler, true);

	return LWMain(CmndCnt, Results);
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

bool LWExecute(const char *BinaryPath, const char *Parameters) {
	HINSTANCE I = ShellExecute(NULL, "OPEN", BinaryPath, Parameters, nullptr, SW_SHOWDEFAULT);
	return I !=  nullptr;
}

bool LWEmail(const char *SrcEmail, const char *TargetEmail, const char *Subject, const char *Body, const char *SMTPServer, const char *SMTPUsername, const char *SMTPPassword){
	char Buffer[1024 * 16];
	snprintf(Buffer, sizeof(Buffer),
		"$S = New-Object Net.Mail.MailMessage('%s', '%s', '%s', '%s');"
		"$S.IsBodyHTML = $true;"
		"$SC = New-Object Net.Mail.SmtpClient('%s', 587);"
		"$SC.EnableSSL=$true;"
		"$SC.Credentials = New-Object Net.NetworkCredential('%s', '%s');"
		"$SC.Send($S);",
		SrcEmail, TargetEmail, Subject, Body, SMTPServer, SMTPUsername, SMTPPassword);

	HINSTANCE I = ShellExecute(NULL, "OPEN", "powershell", Buffer, nullptr, SW_HIDE);
	return I != nullptr;
}

bool LWBrowser(const char *URL) {
	HINSTANCE I = ShellExecute(NULL, "OPEN", URL, nullptr, nullptr, SW_SHOW);
	return I!=nullptr;
}

float LWSystemScale(void) {
	uint32_t DPI = GetDpiForSystem();
	return (float)DPI / 96.0f;
}