#include <LWAudio/LWAudioStream.h>
#include <LWPlatform/LWFileStream.h>
#include <LWCore/LWAllocator.h>
#include <LWCore/LWByteBuffer.h>
#include <LWCore/LWTimer.h>
#include <cstring>
#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"
#include <iostream>

struct VorbisContext {
	OggVorbis_File m_File;
	char *m_Buffer;
	uint32_t m_BufferLen;
	uint32_t m_Position;
};

uint32_t LWAudioStream::GetFormatFromExtension(const LWUTF8Iterator &FilePath) {
	uint32_t ExtIdx = LWFileStream::IsExtensions(FilePath, "ogg", "OGG", "wav", "WAV");
	if (ExtIdx < 2) return FormatVorbis;
	else if (ExtIdx < 4) return FormatWav;
	return -1;
}

LWAudioStream *LWAudioStream::Create(const LWUTF8Iterator &Filepath, uint32_t Flag, LWAllocator &Allocator) {
	LWFileStream Stream;
	uint32_t FormatType = GetFormatFromExtension(Filepath);
	if (FormatType == -1) return nullptr;
	if (!LWFileStream::OpenStream(Stream, Filepath, LWFileStream::ReadMode | LWFileStream::BinaryMode, Allocator)) return nullptr;
	return Create(Stream, Flag, FormatType, Allocator);
}

LWAudioStream *LWAudioStream::Create(LWFileStream &Stream, const LWUTF8Iterator &FilePath, uint32_t Flag, LWAllocator &Allocator) {
	uint32_t FormatType = GetFormatFromExtension(FilePath);
	if (FormatType == -1) return nullptr;
	return Create(Stream, Flag, FormatType, Allocator);
}

LWAudioStream *LWAudioStream::Create(const char *Buffer, uint32_t BufferLen, const LWUTF8Iterator &FilePath, uint32_t Flag, LWAllocator &Allocator) {
	uint32_t FormatType = GetFormatFromExtension(FilePath);
	if (FormatType == -1) return nullptr;
	return Create(Buffer, BufferLen, Flag, FormatType, Allocator);
}

LWAudioStream *LWAudioStream::Create(char *Buffer, uint32_t BufferLen, const LWUTF8Iterator &FilePath, uint32_t Flag, LWAllocator &Allocator) {
	uint32_t FormatType = GetFormatFromExtension(FilePath);
	if (FormatType == -1) return nullptr;
	return Create(Buffer, BufferLen, Flag, FormatType, Allocator);
}

LWAudioStream *LWAudioStream::Create(LWFileStream &Stream, uint32_t Flag, uint32_t FormatType, LWAllocator &Allocator) {
	uint32_t Len = Stream.Length();
	char *Buffer = Allocator.Allocate<char>(Len);
	Stream.Read(Buffer, Len);
	return Create(Buffer, Len, Flag, FormatType, Allocator);
}

LWAudioStream *LWAudioStream::Create(const char *Buffer, uint32_t BufferLen, uint32_t Flag, uint32_t FormatType, LWAllocator &Allocator) {
	char *Buf = Allocator.Allocate<char>(BufferLen);
	std::copy(Buffer, Buffer + BufferLen, Buf);
	return Create(Buf, BufferLen, Flag, FormatType, Allocator);
}

LWAudioStream *LWAudioStream::Create(char *Buffer, uint32_t BufferLen, uint32_t Flag, uint32_t FormatType, LWAllocator &Allocator) {
	auto ProcessWav = [](char *Buffer, uint32_t BufferLen, uint32_t Flag, LWAllocator &Allocator)->LWAudioStream* {
		const uint32_t HeaderMagicA = 0x46464952;//"RIFF"
		const uint32_t HeaderMagicB = 0x45564157; //"Wave"
		const uint32_t FmtChunkMagic = 0x20746d66; //"fmt "
		const uint32_t DataChunkHeader = 0x61746164; //"data"
		const uint16_t PCMTag = 0x1;
		const uint16_t IEEEFloatTag = 0x3;

		LWByteBuffer ByteBuffer((const int8_t*)Buffer, BufferLen);

		auto ReadToChunk = [](LWByteBuffer &Buffer, uint32_t ChunkID)->uint32_t {
			while (!Buffer.EndOfBuffer()) {
				uint32_t CID = Buffer.Read<uint32_t>();
				uint32_t ChunkSize = Buffer.Read<uint32_t>();
				if (CID == ChunkID) return ChunkSize;
				Buffer.OffsetPosition(ChunkSize);
			}
			return 0;
		};
		uint32_t HSize = ReadToChunk(ByteBuffer, HeaderMagicA);
		if (!HSize) return nullptr;
		uint32_t HWaveID = ByteBuffer.Read<uint32_t>();
		if (HWaveID != HeaderMagicB) return nullptr;
		uint32_t FmtSize = ReadToChunk(ByteBuffer, FmtChunkMagic);
		if (!FmtSize) return nullptr;
		uint16_t FormatTag = ByteBuffer.Read<uint16_t>();
		uint16_t Channels = ByteBuffer.Read<uint16_t>();
		uint32_t SamplesPerSec = ByteBuffer.Read<uint32_t>();
		uint32_t AvgBytesPerSec = ByteBuffer.Read<uint32_t>();
		uint16_t BlockAlign = ByteBuffer.Read<uint16_t>();
		uint16_t BitsPerSample = ByteBuffer.Read<uint16_t>();
		if (FormatTag != PCMTag && FormatTag != IEEEFloatTag) return nullptr;
		uint32_t DataSize = ReadToChunk(ByteBuffer, DataChunkHeader);
		if (!DataSize) return nullptr;

		uint32_t SampleType = FormatTag == IEEEFloatTag ? IEEEFloatPCM : LinearPCM;
		uint32_t SampleSize = BitsPerSample / 8;
		uint32_t TotalSamples = DataSize / (SampleSize*Channels);
		ByteBuffer.Read<char>(Buffer, DataSize);
		return Allocator.Create<LWAudioStream>(nullptr, Buffer, TotalSamples, SampleSize, SamplesPerSec, SampleType, (uint32_t)Channels, FormatRaw);
	};

	auto ProcessVorbis = [](char *Buffer, uint32_t BufferLen, uint32_t Flag, LWAllocator &Allocator)->LWAudioStream*{
		VorbisContext *Context = Allocator.Create<VorbisContext>();
		Context->m_Buffer = Buffer;
		Context->m_BufferLen = BufferLen;
		Context->m_Position = 0;

		auto ReadFunc = [](void *Ptr, size_t Size, size_t nmemb, void *DataSource)->size_t {
			VorbisContext *Context = (VorbisContext*)DataSource;
			uint32_t Len = std::min<uint32_t>(Context->m_Position + (uint32_t)(Size*nmemb), Context->m_BufferLen)-Context->m_Position;
			memcpy(Ptr, Context->m_Buffer + Context->m_Position, (size_t)Len);
			Context->m_Position += Len;
			return (size_t)Len;
		};
		auto SeekFunc = [](void *DataSource, ogg_int64_t offset, int whence)->int {
			VorbisContext *Context = (VorbisContext*)DataSource;
			if (whence == SEEK_CUR) Context->m_Position += (int32_t)offset;
			else if (whence == SEEK_SET) Context->m_Position = (int32_t)offset;
			else if (whence == SEEK_END) Context->m_Position = Context->m_BufferLen + (int32_t)offset;
			return 0;
		};
		auto TellFunc = [](void *DataSource)->long {
			VorbisContext *Context = (VorbisContext*)DataSource;
			return Context->m_Position;
		};
        
		ov_callbacks Callbacks = { ReadFunc, SeekFunc, nullptr, TellFunc };
		if (ov_open_callbacks(Context, &Context->m_File, nullptr, 0, Callbacks) < 0) {
			LWAllocator::Destroy(Context);
			return nullptr;
		}
		vorbis_info *vi = ov_info(&Context->m_File, -1);
		uint32_t TotalSamples = (uint32_t)ov_pcm_total(&Context->m_File, -1);
		LWAudioStream *Stream = nullptr;
		if (Flag&Decompressed) {
			uint32_t TotalBufferSize = TotalSamples * 2*2;//channels*(sizeof(uint16_t)/8))
			char *RawBuffer = Allocator.Allocate<char>(TotalBufferSize);
			uint32_t o = 0;
			int32_t cs = 0;
			while (o != TotalBufferSize) {
				int32_t r = (int32_t)ov_read(&Context->m_File, RawBuffer + o, TotalBufferSize - o, 0, 2, 1, &cs);
				if (r <= 0) {
					fmt::print("Error reading ogg samples.\n");
					ov_clear(&Context->m_File);
					LWAllocator::Destroy(RawBuffer);
					LWAllocator::Destroy(Context);
					LWAllocator::Destroy(Buffer);
					return nullptr;
				}
				o += r;
			}

			ov_clear(&Context->m_File);
			LWAllocator::Destroy(Context);
			LWAllocator::Destroy(Buffer);
			Stream = Allocator.Create<LWAudioStream>(nullptr, RawBuffer, TotalSamples, (uint32_t)sizeof(uint16_t), 44100, LinearPCM, 2, FormatRaw);
		} else {
			Stream = Allocator.Create<LWAudioStream>(Context, Buffer, TotalSamples, (uint32_t)sizeof(uint16_t), 44100, LinearPCM, 2, FormatVorbis);
		}
		return Stream;
        //return Allocator.Create<LWAudioStream>(Context, Buffer, TotalSamples, sizeof(uint16_t), 44100, LinearPCM, 2, FormatVorbis);
    };

	LWAudioStream *Stream = nullptr;
	if (FormatType == FormatWav) Stream = ProcessWav(Buffer, BufferLen, Flag, Allocator);
	else if (FormatType == FormatVorbis) Stream = ProcessVorbis(Buffer, BufferLen, Flag, Allocator);
	if (!Stream) LWAllocator::Destroy(Buffer);
	return Stream;
}

char *LWAudioStream::DecodeSamples(char *Buffer, uint32_t SamplePos, uint32_t SampleLen, bool ForceCopy) {
	
	auto ProcessRawFormat = [this](char *Buffer, uint32_t SamplePos, uint32_t SampleLen, uint32_t FrameSize, bool ForceCopy)->char* {
		if (!ForceCopy) return m_Buffer + SamplePos*FrameSize;
		memcpy(Buffer, m_Buffer + SamplePos*FrameSize, SampleLen*FrameSize);
		return Buffer;
	};

	auto ProcessVorbisFormat = [this](char *Buffer, uint32_t SamplePos, uint32_t SampleLen, uint32_t FrameSize, bool ForceCopy)->char* {
		VorbisContext *Context = (VorbisContext*)m_Context;
		ov_pcm_seek(&Context->m_File, (ogg_int64_t)(SamplePos));
		uint32_t o = 0;
		int32_t cs = 0;
		while (o != (SampleLen*FrameSize)) {
			int32_t Ret = (int32_t)ov_read(&Context->m_File, Buffer + o, (SampleLen*FrameSize) - o, 0, 2, 1, &cs);
			if (Ret <= 0) {
				fmt::print("Failed: {} | {}\n", Ret, o);
				return Buffer;
			}
			o += (uint32_t)Ret;
		}
		return Buffer;
	};

	uint32_t FormatType = m_Flag&FormatFlag;
	uint32_t FrameSize = m_SampleSize*m_Channels;
	if (FormatType == FormatRaw) return ProcessRawFormat(Buffer, SamplePos, SampleLen, FrameSize, ForceCopy);
	else if (FormatType == FormatVorbis) return ProcessVorbisFormat(Buffer, SamplePos, SampleLen, FrameSize, ForceCopy);
	return nullptr;
}

uint64_t LWAudioStream::GetTimePerSample(void) const {
	return LWTimer::GetResolution() / m_SampleRate;
}

uint64_t LWAudioStream::GetTotalTime(void) const {
	return GetTimePerSample()*m_SampleLength;
}

uint32_t LWAudioStream::GetSampleSliceSize(uint32_t BufferSize) const {
	uint32_t FormatType = m_Flag & FormatFlag;
	if (FormatType == FormatRaw) return m_SampleLength;
	uint32_t FrameSize = m_SampleSize * m_Channels;
	return BufferSize / FrameSize;
}

uint32_t LWAudioStream::GetSampleLength(void) const {
	return m_SampleLength;
}

uint32_t LWAudioStream::GetSampleSize(void) const {
	return m_SampleSize;
}

float LWAudioStream::GetLength(void) const {
	return (float)m_SampleLength / (float)m_SampleRate;
}

uint32_t LWAudioStream::GetSampleRate(void) const {
	return m_SampleRate;
}

uint32_t LWAudioStream::GetSampleType(void) const {
	return m_SampleType;
}

uint32_t LWAudioStream::GetFormatType(void) const {
	return m_Flag&FormatFlag;
}

uint32_t LWAudioStream::GetChannels(void) const {
	return m_Channels;
}

uint32_t LWAudioStream::GetFlag(void) const {
	return m_Flag;
}

LWAudioStream &LWAudioStream::operator=(LWAudioStream &&Stream) {
	m_Context = Stream.m_Context;
	m_Buffer = Stream.m_Buffer;
	m_SampleLength = Stream.m_SampleLength;
	m_SampleRate = Stream.m_SampleRate;
	m_SampleSize = Stream.m_SampleSize;
	m_SampleType = Stream.m_SampleType;
	m_Channels = Stream.m_Channels;
	m_Flag = Stream.m_Flag;
	Stream.m_Context = nullptr;
	Stream.m_Buffer = nullptr;
	return *this;
}

LWAudioStream::LWAudioStream(LWAudioStream &&Stream) {
	m_Context = Stream.m_Context;
	m_Buffer = Stream.m_Buffer;
	m_SampleLength = Stream.m_SampleLength;
	m_SampleRate = Stream.m_SampleRate;
	m_SampleSize = Stream.m_SampleSize;
	m_SampleType = Stream.m_SampleType;
	m_Channels = Stream.m_Channels;
	m_Flag = Stream.m_Flag;
	Stream.m_Context = nullptr;
	Stream.m_Buffer = nullptr;
}

LWAudioStream::LWAudioStream(void *Context, char *Buffer, uint32_t SampleLength, uint32_t SampleSize, uint32_t SampleRate, uint32_t SampleType, uint32_t Channels, uint32_t Flag) : m_Context(Context), m_Buffer(Buffer), m_SampleLength(SampleLength), m_SampleRate(SampleRate), m_SampleType(SampleType), m_SampleSize(SampleSize), m_Channels(Channels), m_Flag(Flag) {}

LWAudioStream::LWAudioStream() : m_Context(nullptr), m_Buffer(nullptr), m_SampleLength(0), m_SampleRate(0), m_SampleType(0), m_Channels(0), m_Flag(0) {}

LWAudioStream::~LWAudioStream() {
	auto CloseRaw = [this]() {
	};

	auto CloseVorbis = [this]() {
		VorbisContext *Context = (VorbisContext*)m_Context;
		ov_clear(&Context->m_File);
		LWAllocator::Destroy(Context);
	};
	uint32_t Type = (m_Flag&FormatFlag);
	if (Type == FormatRaw) CloseRaw();
	else if (Type == FormatVorbis) CloseVorbis();
	LWAllocator::Destroy(m_Buffer);
}