#include "LWPlatform/LWApplication.h"
#include "LWPlatform/LWPlatform.h"
#include <LWCore/LWText.h>
#include <LWCore/LWTimer.h>
#include <ShellScalingApi.h>

//Application entry points and other data is managed here.

int main(int argc, char **argv){
	HWND ConsoleWnd = GetConsoleWindow();
	if (ConsoleWnd) SetWindowPos(ConsoleWnd, HWND_TOP, 1, 1, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
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
	SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);

	return LWMain(CmndCnt, Results);
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