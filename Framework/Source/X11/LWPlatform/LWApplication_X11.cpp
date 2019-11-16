#include "LWPlatform/LWApplication.h"
#include "LWPlatform/LWPlatform.h"
#include "LWCore/LWTimer.h"
#include <unistd.h>
#include <spawn.h>
#include <cstring>
#include <stdlib.h>
#include <cstdio>
#include <iostream>
char **env;

int main(int argc, char **argv){
	return LWMain(argc, argv);
}

bool LWExecute(const char *BinaryPath, const char *Parameters) {
	char Buffer[256];
	snprintf(Buffer, sizeof(Buffer), "chmod +x %s", BinaryPath);
	//chmod the file for execution first.
	system(Buffer);
	snprintf(Buffer, sizeof(Buffer), "%s %s &", BinaryPath, Parameters ? Parameters : "");
	system(Buffer);
	return true;
}

bool LWRunLoop(std::function<bool(void*)> LoopFunc, uint64_t Frequency, void* UserData) {
	uint64_t Prev = LWTimer::GetCurrent();
	while (true) {
		uint64_t Curr = LWTimer::GetCurrent();
		if (Curr - Prev >= Frequency) {
			if (!LoopFunc(UserData)) break;
			Prev += Frequency;
		}
	}
	return true;
}

bool LWEmail(const char *SrcEmail, const char *TargetEmail, const char *Subject, const char *Body, const char *SMTPServer, const char *SMTPUsername, const char *SMTPPassword){
	return false;
}

bool LWBrowser(const char *URL) {
	return false;
}

float LWSystemScale(void) {
	return 1.0f;
}