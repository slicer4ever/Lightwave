#include "LWEVideoPlayer.h"
#include <LWCore/LWTimer.h>
#include <LWPlatform/LWFileStream.h>
#include <LWVideo/LWVideoDriver.h>
#include <LWVideo/LWTexture.h>
#include <LWVideo/LWImage.h>
#include <LWCore/LWByteBuffer.h>
#include <iostream>
#include "LWEVideoDecoderIVF.h"
#include "LWEVideoDecoderWEBM.h"

bool LWEVideoPlayer::YUVToRGBA(const uint8_t **YUVArrays, const LWVector3i &YUVStrides, const LWVector2i &ImageSize, uint8_t *Buffer, uint32_t BufferSize) {
	LWVector2i hSize = ImageSize / 2;
	uint32_t Stride = 4 * ImageSize.x;
	
	if (Stride*ImageSize.y > BufferSize) {
		fmt::print("Error buffer too small for size: {} Expected: {}\n", BufferSize, Stride * ImageSize.y);
		return false;
	}

	for(int32_t y=0;y<ImageSize.y;y++){
		uint8_t *Dst = Buffer + (Stride * y);
		const uint8_t *SrcY = YUVArrays[0] + (YUVStrides.x*y);
		const uint8_t *SrcYB = SrcY + YUVStrides.x;
		const uint8_t *SrcU = YUVArrays[1] + (YUVStrides.y * (y/2));
		const uint8_t *SrcV = YUVArrays[2] + (YUVStrides.z * (y/2));

		for(int32_t x=0;x<ImageSize.x;x++){			
			uint8_t Y, U, V;
			int16_t R, G, B;
			int16_t iR, iG, iB;
			U = SrcU[x / 2];
			V = SrcV[x / 2];
			iR = (351 * (V - 128)) / 256;
			iG = -(179 * (V - 128)) / 256 - (86 * (U - 128)) / 256;
			iB = (444 * (U - 128)) / 256;

			Y = SrcY[x];
			R = Y + iR; G = Y + iG; B = Y + iB;
			R = (R < 0 ? 0 : (R > 255 ? 255 : R)); 
			G = (G < 0 ? 0 : (G > 255 ? 255 : G)); 
			B = (B < 0 ? 0 : (B > 255 ? 255 : B));
			*(Dst++) = (uint8_t)R; *(Dst++) = (uint8_t)G; *(Dst++) = (uint8_t)B; *(Dst++) = 255;
		}
	}
	return true;
}

bool LWEVideoPlayer::OpenVideo(LWEVideoPlayer &Player, LWVideoDriver *Driver, const LWUTF8Iterator &Path, bool StartPlaying, uint32_t LoopCnt, void *UserData, std::function<void(LWEVideoPlayer &, void*)> FinishedCallback, float PlaybackSpeed, LWAllocator &Allocator, LWFileStream *Existing) {
	LWFileStream Stream;
	if (!LWFileStream::OpenStream(Stream, Path, LWFileStream::BinaryMode | LWFileStream::ReadMode, Allocator, Existing)) {
		fmt::print("Failed to open video file: '{}'\n", Path);
		return false;
	}

	auto OpenIVF = [](LWEVideoPlayer &Player, LWFileStream &Stream, LWVideoDriver *Driver, uint32_t Flags, uint32_t LoopCnt, void *UserData, std::function<void(LWEVideoPlayer&, void*)> FinishedCallback, float PlaybackSpeed, LWAllocator &Allocator)->bool {
		Stream.Seek(0, LWFileStream::SeekStart);
		return LWEVideoDecoderIVF::CreateDecoder(Player, Stream, Driver, Flags, PlaybackSpeed, LoopCnt, UserData, FinishedCallback, Allocator);
	};
	/*
	auto OpenWEBM = [](LWEVideoPlayer &Player, LWFileStream &Stream, LWVideoDriver *Driver, uint32_t Flags, uint32_t LoopCnt, void *UserData, std::function<void(LWEVideoPlayer&, void*)> FinishedCallback, float PlaybackSpeed, LWAllocator &Allocator)->bool {
		Stream.Seek(0, LWFileStream::SeekStart);
		return LWEVideoDecoderWEBM::CreateDecoder(Player, Stream, Driver, Flags, PlaybackSpeed, LoopCnt, UserData, FinishedCallback, Allocator);
	};*/
	uint32_t Flag = StartPlaying ? PlayRequested : 0;
	//if (OpenWEBM(Player, Stream, Driver, Flag, LoopCnt, UserData, FinishedCallback, PlaybackSpeed, Allocator)) return true;
	if (OpenIVF(Player, Stream, Driver, Flag, LoopCnt, UserData, FinishedCallback, PlaybackSpeed, Allocator)) return true;
	fmt::print("Error could not find decoder for: '{}'\n", Path);
	return false;
}

LWEVideoPlayer &LWEVideoPlayer::Update(uint64_t lCurrentTime) {
	uint32_t CachedCnt = m_CachedFrame - m_WriteFrame;
	uint64_t DeltaTime = (uint64_t)((float)(lCurrentTime - m_LastTime) * m_PlaybackSpeed);
	if ((m_Flag&PlayRequested)) m_Flag = (m_Flag&~PlayRequested) | Playing;
	else if ((m_Flag&StopRequested)) m_Flag = (m_Flag&~(StopRequested | Playing));
	if (!(m_Flag&Playing)) DeltaTime = 0;
	uint64_t pPosition = (m_Position / m_Framerate)*m_Framerate;
	uint64_t TotalPlaytime = m_Framerate * m_FrameCount;
	m_LastTime = lCurrentTime;
	m_Position += DeltaTime;

	if (m_Position >= TotalPlaytime) {
		if (m_FinishedCallback) m_FinishedCallback(*this, m_UserData);

		if (m_LoopedCount < m_LoopCount) {
			SeekTo(m_Position - TotalPlaytime);
			m_LoopedCount++;
		} else m_Position = TotalPlaytime;
	} else if (m_Position - pPosition >= m_Framerate || !m_WriteFrame) m_WriteFrame++;

	if (CachedCnt < MaxCachedFrames-1) {
		uint32_t Err = m_Decoder->AdvanceFrame(m_PixelBuffer[m_CachedFrame%MaxCachedFrames], m_FrameSize);
		if(Err==LWEVideoDecoder::Error_None) m_CachedFrame++;
	}
	return *this;
}

LWEVideoPlayer &LWEVideoPlayer::UpdateTexture(void) {
	if (m_ReadFrame == m_WriteFrame || m_ReadFrame>=m_CachedFrame) return *this;
	m_VideoDriver->UpdateTexture2D(m_Texture, 0, m_PixelBuffer[m_ReadFrame%MaxCachedFrames]);
	m_ReadFrame++;
	return *this;
}

LWEVideoPlayer &LWEVideoPlayer::SetFinishedCallback(LWEVideoFinishedCallback Callback, void *UserData) {
	m_FinishedCallback = Callback;
	m_UserData = UserData;
	return *this;
}

LWEVideoPlayer &LWEVideoPlayer::SeekTo(const uint64_t Time) {
	m_Position = std::min<uint64_t>(Time, m_Framerate*m_FrameCount-1);
	m_Decoder->GoToFrame((uint32_t)(m_Position / m_Framerate));
	m_WriteFrame = m_ReadFrame = m_CachedFrame = 0;
	return *this;
}

LWEVideoPlayer &LWEVideoPlayer::SetPlaybackSpeed(float Speed) {
	m_PlaybackSpeed = Speed;
	return *this;
}

LWEVideoPlayer &LWEVideoPlayer::Play(void) {
	m_Flag |= PlayRequested;
	return *this;
}

LWEVideoPlayer &LWEVideoPlayer::Stop(void) {
	m_Flag |= StopRequested;
	return *this;
}

LWTexture *LWEVideoPlayer::Frame(void) {
	return m_Texture;
}


LWVector2i LWEVideoPlayer::GetFrameSize(void) const{
	return m_FrameSize;
}

LWVector2f LWEVideoPlayer::GetFrameSizef(void) const{
	return m_FrameSize.CastTo<float>();
}

uint64_t LWEVideoPlayer::GetPosition(void) const {
	return m_Position;
}

uint32_t LWEVideoPlayer::GetLoopCount(void) const {
	return m_LoopCount;
}

uint32_t LWEVideoPlayer::GetLoopedCount(void) const {
	return m_LoopedCount;
}

float LWEVideoPlayer::GetPlaybackSpeed(void) const {
	return m_PlaybackSpeed;
}

bool LWEVideoPlayer::IsFinished(void) const {
	return m_Position >= GetTotalPlayTime() && m_LoopedCount == m_LoopCount;
}

bool LWEVideoPlayer::IsPlaying(void) const {
	return (m_Flag&Playing) != 0;
}

bool LWEVideoPlayer::IsStopped(void) const {
	return (m_Flag&Playing) == 0;
}

LWEVideoPlayer &LWEVideoPlayer::operator=(LWEVideoPlayer &&O) {
	m_FinishedCallback = O.m_FinishedCallback;
	m_Decoder = O.m_Decoder;
	m_Texture = O.m_Texture;
	m_VideoDriver = O.m_VideoDriver;
	m_FrameSize = O.m_FrameSize;
	m_Framerate = O.m_Framerate;
	m_FrameCount = O.m_FrameCount;
	m_LastTime = O.m_LastTime;
	m_PlaybackSpeed = O.m_PlaybackSpeed;
	m_Position = O.m_Position;
	m_LoopCount = O.m_LoopCount;
	m_LoopedCount = O.m_LoopedCount;
	m_CachedFrame = O.m_CachedFrame;
	m_ReadFrame = O.m_ReadFrame;
	m_WriteFrame = O.m_WriteFrame;
	m_Flag = O.m_Flag;
	m_UserData = O.m_UserData;
	O.m_Decoder = nullptr;
	O.m_Texture = nullptr;
	for (uint32_t i = 0; i < MaxCachedFrames; i++) {
		m_PixelBuffer[i] = O.m_PixelBuffer[i];
		O.m_PixelBuffer[i] = nullptr;
	}
	return *this;
}

uint32_t LWEVideoPlayer::GetFrameCount(void) const {
	return m_FrameCount;
}

uint64_t LWEVideoPlayer::GetTotalPlayTime(void) const {
	return m_Framerate * m_FrameCount;
}

LWEVideoPlayer::LWEVideoPlayer(LWVideoDriver *VideoDriver, LWEVideoDecoder *Decoder, const LWVector2i &FrameSize, uint64_t FrameRate, uint32_t FrameCount, uint32_t LoopCnt, uint32_t Flag, void *UserData, LWEVideoFinishedCallback FinishedCallback, float PlaybackSpeed, LWAllocator &Allocator) : m_FinishedCallback(FinishedCallback), m_VideoDriver(VideoDriver), m_Decoder(Decoder), m_LoopCount(LoopCnt), m_FrameSize(FrameSize), m_Framerate(FrameRate), m_LastTime(LWTimer::GetCurrent()), m_UserData(UserData), m_FrameCount(FrameCount), m_PlaybackSpeed(PlaybackSpeed), m_Flag(Flag) {
	m_Texture = m_VideoDriver->CreateTexture2D(LWTexture::MinLinear | LWTexture::MagLinear, LWImage::R8U, LWVector2i(FrameSize.x, FrameSize.y + FrameSize.y / 2), nullptr, 0, Allocator);
	for (uint32_t i = 0; i < MaxCachedFrames; i++) {
		m_PixelBuffer[i] = Allocator.Allocate<uint8_t>(m_FrameSize.x*(m_FrameSize.y + FrameSize.y / 2));
	}
}

LWEVideoPlayer::LWEVideoPlayer(LWEVideoPlayer &&O) : m_FinishedCallback(O.m_FinishedCallback), m_Texture(O.m_Texture), m_VideoDriver(O.m_VideoDriver), m_Decoder(O.m_Decoder), m_FrameSize(O.m_FrameSize), m_Framerate(O.m_Framerate), m_Position(O.m_Position), m_LastTime(O.m_LastTime), m_LoopedCount(O.m_LoopedCount), m_LoopCount(O.m_LoopCount), m_FrameCount(O.m_FrameCount), m_CachedFrame(O.m_CachedFrame), m_ReadFrame(O.m_ReadFrame), m_WriteFrame(O.m_WriteFrame), m_UserData(O.m_UserData), m_PlaybackSpeed(O.m_PlaybackSpeed), m_Flag(O.m_Flag) {
	O.m_Texture = nullptr;
	O.m_Decoder = nullptr;
	for (uint32_t i = 0; i < MaxCachedFrames; i++) {
		m_PixelBuffer[i] = O.m_PixelBuffer[i];
		O.m_PixelBuffer[i] = nullptr;
	}
}

LWEVideoPlayer::~LWEVideoPlayer() {
	if(m_Texture) m_VideoDriver->DestroyTexture(m_Texture);
	for(uint32_t i=0;i<MaxCachedFrames;i++)	LWAllocator::Destroy(m_PixelBuffer[i]);
	LWAllocator::Destroy(m_Decoder);
}