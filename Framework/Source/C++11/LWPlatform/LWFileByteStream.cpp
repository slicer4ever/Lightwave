#include <LWPlatform/LWFileByteStream.h>


uint32_t LWFileByteStream::ReadCallback(int8_t *Buffer, uint32_t Len, void *UserData) {
	LWFileStream *Stream = (LWFileStream*)UserData;
	return Stream->Read((char*)Buffer, Len);
}

LWFileByteStream &LWFileByteStream::operator = (LWFileByteStream &&O) {
	LWByteStream::operator=(std::move(O));
	m_Stream = O.m_Stream;
	return *this;
}

LWFileByteStream::LWFileByteStream(LWFileStream *Stream, uint32_t CachedBufferLength, uint32_t Flag, LWAllocator &Allocator) : LWByteStream(CachedBufferLength, ReadCallback, Flag, Stream, Allocator), m_Stream(Stream) {
}

LWFileByteStream::LWFileByteStream(LWFileByteStream &&O) : LWByteStream(std::move(O)), m_Stream(O.m_Stream) {}