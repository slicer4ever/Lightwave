#ifndef LWEVIDEODECODER_WEBM
#define LWEVIDEODECODER_WEBM
#include "LWEVideoDecoder.h"
#include <LWCore/LWTypes.h>
#include <LWVideo/LWTypes.h>
#include <functional>


//WEBMDecoder is still work-in progress that has been shelved for the time being.

#define LWE_EBLM_ID 0xA45DFA3
#define LWE_EBLM_VERSION_ID 0x286
#define LWE_EBLM_READ_VERSION_ID 0x2F7
#define LWE_EBLM_MAXIDLENGTH_ID 0x2F2
#define LWE_EBLM_MAXSIZELENGTH_ID 0x2F3
#define LWE_EBLM_DOCTYPE_ID 0x282
#define LWE_EBLM_DOCTYPE_VERSION_ID 0x287
#define LWE_EBLM_DOCTYPE_READVERSION_ID 0x285

#define LWE_EBLM_CRC32_ID 0x3F
#define LWE_EBLM_VOID_ID 0x6C

#define LWE_WEBM_SEGMENT_ID 0x8538067

#define LWE_WEBM_READVERSION 0x2

struct LWEWebmHeader {
	static const uint32_t MaxTextLength = 64;
	uint32_t m_Version = 1;
	uint32_t m_ReadVersion = 1;
	uint32_t m_MaxIDLength = 4;
	uint32_t m_MaxSizeLength = 8;
	char m_DocType[MaxTextLength];
	uint32_t m_DocTypeVersion = 1;
	uint32_t m_DocTypeReadVersion = 1;

	LWEWebmHeader(LWByteStream &Stream, uint64_t HeaderSize);
};

struct LWEWebmSeek {
	uint32_t m_ID;
	uint32_t m_Position;

	LWEWebmSeek(LWByteStream &Stream, uint64_t HeaderSize);
};

struct LWEWebmSegment {
	static const uint32_t MaxTextLength = 64;
	LWEWebmSeek *m_Seeks;
	uint64_t m_TimestampScale;
	float m_Duration;

	LWEWebmSegment(LWByteStream &Stream, uint64_t HeaderSize, LWAllocator &Allocator);
};


class LWEVideoDecoderWEBM : public LWEVideoDecoder {
public:
	static bool CreateDecoder(LWEVideoPlayer &Player, LWFileStream &Stream, LWVideoDriver *Driver, uint32_t PlayerFlags, float PlaybackSpeed, uint32_t LoopCnt, void *Userdata, std::function<void(LWEVideoPlayer &, void*)> FinishedCallback, LWAllocator &Allocator);

	uint32_t AdvanceFrame(uint8_t *PixelBuffer, const LWVector2i &FrameSize);

	bool GoToFrame(uint32_t FrameIdx);

	LWEVideoDecoderWEBM();

	~LWEVideoDecoderWEBM();
private:
};

#endif