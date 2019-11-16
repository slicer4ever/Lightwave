#include "LWEVideoDecoderWEBM.h"
#include <LWPlatform/LWFileByteStream.h>


uint32_t ParseEBMLID(LWByteStream &Stream, uint64_t &BytesRead) {
	uint8_t Buffer[4];
	Buffer[0] = Stream.Read<uint8_t>();
	uint32_t Bytes = 1;
	uint32_t Bit = 0x80;
	uint32_t ExcludeBits = 0x80;
	while ((Buffer[0] & Bit) == 0) {
		Bit >>= 1;
		Bytes++;
	}
	Stream.Read<uint8_t>(Buffer + 1, Bytes - 1);
	Buffer[0] &= ~Bit;
	uint32_t Val = LWByteBuffer::MakeHost(*((uint32_t*)Buffer));
	Val >>= (4 - Bytes) * 8;
	BytesRead += Bytes;
	return Val;
}

uint64_t ParseEBMLSize(LWByteStream &Stream, uint64_t &BytesRead) {
	uint8_t Buffer[8];
	Buffer[0] = Stream.Read<uint8_t>();
	uint32_t Bytes = 1;
	uint32_t Bit = 0x80;
	uint32_t ExcludeBits = 0x80;
	while ((Buffer[0] & Bit) == 0) {
		Bit >>= 1;
		Bytes++;
	}
	Stream.Read<uint8_t>(Buffer + 1, Bytes - 1);
	Buffer[0] &= ~Bit;
	uint64_t Val = LWByteBuffer::MakeHost(*((uint64_t*)Buffer));
	Val >>= (8 - Bytes) * 8;
	BytesRead += Bytes;
	return Val;
}

int64_t ParseEBMLInt(LWByteStream &Stream, uint64_t DataSize) {
	int8_t Buffer[8];
	Stream.Read<int8_t>(Buffer, (uint32_t)DataSize);
	uint64_t Val = LWByteBuffer::MakeHost(*((uint64_t*)Buffer));
	uint64_t NBit = Val & 0x8000000000000000;
	Val &= ~NBit;
	Val >>= (8 - DataSize) * 8;
	Val |= NBit;
	return (int64_t)Val;
}

uint64_t ParseEBMLUInt(LWByteStream &Stream, uint64_t DataSize) {
	int8_t Buffer[8];
	Stream.Read<int8_t>(Buffer, (uint32_t)DataSize);
	uint64_t Val = LWByteBuffer::MakeHost(*((uint64_t*)Buffer));
	Val >>= (8 - DataSize) * 8;
	return Val;
}

double ParseEBMLDouble(LWByteStream &Stream, uint64_t DataSize) {
	int8_t Buffer[8];
	Stream.Read<int8_t>(Buffer, (uint32_t)DataSize);
	if (DataSize == 4) {
		float Val = LWByteBuffer::MakeHostf(*((uint32_t*)Buffer));
		return (double)Val;
	} else {
		double Val = LWByteBuffer::MakeHostf(*((uint64_t*)Buffer));
		return Val;
	}
}

uint32_t ParseEBMLString(LWByteStream &Stream, char *Buffer, uint32_t BufferLen, uint64_t DataSize) {
	uint32_t Len = std::min<uint32_t>(BufferLen, (uint32_t)DataSize);
	uint32_t n = Stream.Read<char>(Buffer, Len);
	Buffer[Len == BufferLen ? Len - 1 : Len] = '\0';
	Stream.OffsetStream((uint32_t)DataSize - n);
	return n;
}

uint64_t ParseEBMLDate(LWByteStream &Stream, uint64_t DataSize) {
	if (DataSize != 8) {
		std::cout << "Error reading date." << std::endl;
		return 0;
	}
	return Stream.Read<uint64_t>();
}

bool ParseEBMLLoop(LWByteStream &Stream, uint64_t &Length, std::function<bool(LWByteStream &, uint32_t, uint64_t)> ParseIDFunc) {
	while (Length) {
		uint64_t BytesRead = 0;
		uint32_t ID = ParseEBMLID(Stream, BytesRead);
		uint64_t DataSize = ParseEBMLSize(Stream, BytesRead);
		if (!ParseIDFunc(Stream, ID, DataSize)) {
			std::cout << "Unknown ID encountered: " << std::hex << ID << std::dec << std::endl;
			Stream.OffsetStream((uint32_t)DataSize);
		}
		Length -= (BytesRead + DataSize);
	}
	return true;
};

LWEWebmHeader::LWEWebmHeader(LWByteStream &Stream, uint64_t HeaderSize){
	auto ParseID = [this](LWByteStream &Stream, uint32_t ID, uint64_t DataSize)->bool {
		if (ID == LWE_EBLM_VERSION_ID) m_Version = (uint32_t)ParseEBMLInt(Stream, DataSize);
		else if (ID == LWE_EBLM_READ_VERSION_ID) m_ReadVersion = (uint32_t)ParseEBMLInt(Stream, DataSize);
		else if (ID == LWE_EBLM_MAXIDLENGTH_ID) m_MaxIDLength = (uint32_t)ParseEBMLInt(Stream, DataSize);
		else if (ID == LWE_EBLM_MAXSIZELENGTH_ID) m_MaxSizeLength = (uint32_t)ParseEBMLInt(Stream, DataSize);
		else if (ID == LWE_EBLM_DOCTYPE_ID) ParseEBMLString(Stream, m_DocType, MaxTextLength, DataSize);
		else if (ID == LWE_EBLM_DOCTYPE_VERSION_ID) m_DocTypeVersion = (uint32_t)ParseEBMLInt(Stream, DataSize);
		else if (ID == LWE_EBLM_DOCTYPE_READVERSION_ID) m_DocTypeReadVersion = (uint32_t)ParseEBMLInt(Stream, DataSize);
		else return false;
		return true;
	};
	*m_DocType = '\0';
	ParseEBMLLoop(Stream, HeaderSize, ParseID);
};

bool LWEVideoDecoderWEBM::CreateDecoder(LWEVideoPlayer &Player, LWFileStream &Stream, LWVideoDriver *Driver, uint32_t PlayerFlags, float PlaybackSpeed, uint32_t LoopCnt, void *Userdata, std::function<void(LWEVideoPlayer &, void *)> FinishedCallback, LWAllocator &Allocator) {
	LWFileByteStream ByteStream(&Stream, 2048, LWByteStream::AutoSize|LWByteStream::Network, Allocator);
	uint64_t BytesRead = 0;
	uint32_t HeaderID = ParseEBMLID(ByteStream, BytesRead);
	if (HeaderID != LWE_EBLM_ID) return false;
	uint64_t HeaderSize = ParseEBMLSize(ByteStream, BytesRead);
	//std::cout << "HeaderID: "  << std::hex << HeaderID << " | " << HeaderSize << std::endl;
	LWEWebmHeader Header(ByteStream, HeaderSize);
	if (!LWText::Compare(Header.m_DocType, "webm") || Header.m_DocTypeReadVersion>LWE_WEBM_READVERSION) return false;

	return false;
}

uint32_t LWEVideoDecoderWEBM::AdvanceFrame(uint8_t *PixelBuffer, const LWVector2i &FrameSize) {
	return Error_None;
}

bool LWEVideoDecoderWEBM::GoToFrame(uint32_t FrameIdx) {
	return true;
}

LWEVideoDecoderWEBM::LWEVideoDecoderWEBM() {
}

LWEVideoDecoderWEBM::~LWEVideoDecoderWEBM() {
}
