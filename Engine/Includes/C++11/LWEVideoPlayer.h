#ifndef LWEVIDEOPLAYER_H
#define LWEVIDEOPLAYER_H
#include <LWCore/LWTypes.h>
#include <LWPlatform/LWTypes.h>
#include <LWVideo/LWTypes.h>
#include <LWCore/LWVector.h>
#include <LWPlatform/LWFileStream.h>
#include "LWEVideoDecoder.h"
#include <functional>

class LWEVideoPlayer {
public:
	enum {
		MaxCachedFrames = 3,
		Playing=0x1,
		PlayRequested=0x2,
		StopRequested=0x4,
	};
	//Finished callback is called at the end of each loop.
	static bool OpenVideo(LWEVideoPlayer &Player, LWVideoDriver *Driver, const LWText &Path, bool StartPlaying, uint32_t LoopCnt, void *Userdata, std::function<void(LWEVideoPlayer &, void*)> FinishedCallback, float PlaybackSpeed, LWAllocator &Allocator, LWFileStream *Existing = nullptr);

	static bool YUVToRGBA(const uint8_t **YUVArrays, const LWVector3i &YUVStrides, const LWVector2i &ImageSize, uint8_t *Buffer, uint32_t BufferSize);

	//Builds frames for the updateTexture to be ready.
	LWEVideoPlayer &Update(uint64_t lCurrentTime);

	//Updates the internal texture.
	LWEVideoPlayer &UpdateTexture();

	LWEVideoPlayer &SetFinishedCallback(std::function<void(LWEVideoPlayer &, void*)> Callback, void *UserData);

	//Returns the current textured frame.
	LWTexture *Frame(void);

	LWEVideoPlayer &SeekTo(const uint64_t Time);

	LWEVideoPlayer &SetPlaybackSpeed(float Speed);

	LWEVideoPlayer &Play(void);

	LWEVideoPlayer &Stop(void);

	LWVector2i GetFrameSize(void) const;

	LWVector2f GetFrameSizef(void) const;

	uint32_t GetFrameCount(void) const;

	uint64_t GetTotalPlayTime(void) const;

	uint64_t GetPosition(void) const;

	uint32_t GetLoopCount(void) const;

	uint32_t GetLoopedCount(void) const;

	float GetPlaybackSpeed(void) const;

	bool IsFinished(void) const;

	bool IsPlaying(void) const;

	bool IsStopped(void) const;

	LWEVideoPlayer &operator = (LWEVideoPlayer &&O);

	LWEVideoPlayer(LWVideoDriver *Driver, LWEVideoDecoder *Decoder, const LWVector2i &FrameSize, uint64_t FrameRate, uint32_t FrameCount, uint32_t LoopCnt, uint32_t Flag, void *UserData, std::function<void(LWEVideoPlayer&, void*)> FinishedCallback, float PlaybackSpeed, LWAllocator &Allocator);

	LWEVideoPlayer(LWEVideoPlayer &&O);

	LWEVideoPlayer() = default;

	~LWEVideoPlayer();
private:
	std::function<void(LWEVideoPlayer &, void *)> m_FinishedCallback;
	LWEVideoDecoder *m_Decoder = nullptr;
	LWTexture *m_Texture = nullptr;
	void *m_UserData = nullptr;
	LWVideoDriver *m_VideoDriver;
	uint8_t *m_PixelBuffer[MaxCachedFrames];
	float m_PlaybackSpeed;
	LWVector2i m_FrameSize;
	uint64_t m_Framerate;
	uint64_t m_LastTime;
	uint64_t m_Position = 0;
	uint32_t m_LoopCount;
	uint32_t m_LoopedCount = 0;
	uint32_t m_FrameCount;
	uint32_t m_CachedFrame = 0;
	uint32_t m_ReadFrame = 0;
	uint32_t m_WriteFrame = 0;
	uint32_t m_Flag = 0;
};

#endif