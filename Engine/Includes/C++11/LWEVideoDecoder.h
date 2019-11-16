#ifndef LWEVIDEO_DECODER_H
#define LWEVIDEO_DECODER_H
#include <LWCore/LWTypes.h>
#include <LWVideo/LWTypes.h>

class LWEVideoPlayer;

class LWEVideoDecoder {
public:
	enum {
		Error_None = 0,
		Error_OutofFrames,
		Error_Decoding
	};
	virtual uint32_t AdvanceFrame(uint8_t *PixelBuffer, const LWVector2i &FrameSize) = 0;

	virtual bool GoToFrame(uint32_t FrameIdx) = 0;
};


#endif