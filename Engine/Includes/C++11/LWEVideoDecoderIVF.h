#ifndef LWEVIDEODECODERIVF_H
#define LWEVIDEODECODERIVF_H
#include "LWEVideoDecoder.h"
#include <LWPlatform/LWFileStream.h>
#include <functional>

#define LWE_IVF_FILE_HDR_SZ  32
#define LWE_IVF_FRAME_HDR_SZ 12
#define LWE_IVF_HEADER_MAGIC "DKIF"
#define LWE_VP8_FOURCC 0x30385056
#define LWE_VP9_FOURCC 0x30395056

struct vpx_codec_ctx;

struct LWEVideoIVFHeader {
	char m_Magic[4];
	int16_t m_Version;
	int16_t m_HeaderSize;
	int32_t m_fourcc;
	int16_t m_Width;
	int16_t m_Height;
	int32_t m_TimeNumer;
	int32_t m_TimeDenom;
	int32_t m_FrameCount;

	LWEVideoIVFHeader(const int8_t *Buffer);
};

struct LWEVideoIVFFrameHeader {
	uint32_t m_FrameSize;
	uint64_t m_Timestamp;

	LWEVideoIVFFrameHeader(const int8_t *Buffer);
};

class LWEVideoDecoderIVF : public LWEVideoDecoder {
public:
	static bool CreateDecoder(LWEVideoPlayer &Player, LWFileStream &Stream, LWVideoDriver *Driver, uint32_t PlayerFlags, float PlaybackSpeed, uint32_t LoopCnt, void *Userdata, std::function<void(LWEVideoPlayer &, void*)> FinishedCallback, LWAllocator &Allocator);

	uint32_t AdvanceFrame(uint8_t *PixelBuffer, const LWVector2i &FrameSize);

	bool GoToFrame(uint32_t FrameIdx);

	LWEVideoDecoderIVF(LWFileStream &Stream, vpx_codec_ctx &Codec, uint32_t FrameCount, LWAllocator &Allocator);

	~LWEVideoDecoderIVF();
private:
	LWAllocator &m_Allocator;
	LWFileStream m_Stream;
	vpx_codec_ctx *m_Codec;
	uint32_t *m_FramePositions = nullptr;
	uint8_t *m_FrameBuffer = nullptr;
	uint32_t m_FrameBufferSize = 0;
	uint32_t m_FrameCount;
	uint32_t m_Frame = 0;
};

#endif