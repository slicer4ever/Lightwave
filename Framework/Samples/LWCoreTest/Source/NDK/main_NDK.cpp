#include <jni.h>
#include <android/sensor.h>
#include <android/log.h>
#include <android/native_activity.h>
#include <iostream>
#include <sstream>
int main(int argc, char **argv);

void ANativeActivity_onCreate(ANativeActivity *Activity, void *savedState, size_t savedStateSize){
	std::streambuf *oCout = std::cout.rdbuf();
	std::ostringstream nCout;
	std::cout.rdbuf(nCout.rdbuf());

	std::cout << std::endl << std::endl << std::endl << std::endl;
	main(0, nullptr);
	std::cout << std::endl << std::endl << std::endl << std::endl;


	std::cout.rdbuf(oCout);
	for (int32_t Pos = 0; nCout.str().c_str()[Pos];) {
		Pos += __android_log_write(ANDROID_LOG_VERBOSE, "LWCoreTest", nCout.str().c_str() + Pos);
	}
	ANativeActivity_finish(Activity);
	return;
}