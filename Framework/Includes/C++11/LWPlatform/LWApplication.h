#ifndef LWAPPLICATION_H
#define LWAPPLICATION_H
#include "LWTypes.h"
#include <functional>

/*!< \brief this is the new entry point if including LWPlatform library. */
int LWMain(int argc, char **argv);

/*!< \brief attempts to execute the specefied platform, may not be implementing on some platform. */
bool LWExecute(const char *BinaryPath, const char *Parameters);

/*!< \brief in order to support emscripter for web javascript integration, some changes need to be made to infinite running loops, this function runs the specified function in a loop at the specefied interval. The looped function should return false when it wants to quit out of the loop.*/
bool LWRunLoop(std::function<bool(void*)> LoopFunc, uint64_t Frequency, void* UserData);

/*!< \brief attempts to spawn a process to send a email to the targeted email.  may not be implemented on some platforms. 
	 for a more coheisive email system, use Protocol_SMTP or Protocol_POP3(if they have been implemented yet.)
*/
bool LWEmail(const char *SrcEmail, const char *TargetEmail, const char *Subject, const char *Body, const char *SMTPServer, const char *SMTPUsername, const char *SMTPPassword);

/*!< \brief opens a browser window on the platform(if possible), and attempts to set it to the targeted url. */
bool LWBrowser(const char *URL);

/*!< \brief returns the systems current dpi scale. */
float LWSystemScale(void);

#endif