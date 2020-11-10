#include <LWCore/LWAllocator.h>
#include <LWCore/LWByteStream.h>


bool LWByteStream::EndOfStream(void) {
	return !CanReadBytes(1);
}

bool LWByteStream::CanReadBytes(uint32_t Bytes) {
	if (m_Position + Bytes <= m_CachedBufferLength) return true;
	if (Bytes > m_TargetCachedLength){
		if (!(m_Flag&AutoSize)) return false;
		ResizeCacheBuffer(Bytes, *m_Allocator);
	}else std::copy(m_DataBuffer + m_Position, m_DataBuffer + m_CachedBufferLength, m_DataBuffer);
	uint32_t Remain = m_CachedBufferLength - m_Position;
	uint32_t ReadLen = m_ReadCallback(m_DataBuffer + Remain, m_TargetCachedLength - Remain, m_UserData);
	m_CachedBufferLength = Remain + ReadLen;
	m_Position = 0;
	return m_Position+Bytes<=m_CachedBufferLength;
}

bool LWByteStream::ResizeCacheBuffer(uint32_t NewBufferCacheLen, LWAllocator &Allocator, bool MakeSmaller) {
	if (!MakeSmaller && NewBufferCacheLen < m_TargetCachedLength) return true;
	uint32_t Remain = m_CachedBufferLength - m_Position;
	uint32_t NewSize = std::max<uint32_t>(Remain, NewBufferCacheLen);
	int8_t *oBuffer = m_DataBuffer;
	int8_t *nBuffer = Allocator.Allocate<int8_t>(NewSize);
	std::copy(oBuffer + m_Position, oBuffer + m_CachedBufferLength, nBuffer);
	m_Position = 0;
	m_DataBuffer = nBuffer;
	m_CachedBufferLength = Remain;
	m_TargetCachedLength = NewSize;
	LWAllocator::Destroy(oBuffer);
	return true;
}

LWByteStream &LWByteStream::operator=(LWByteStream &&O) {
	m_Allocator = O.m_Allocator;
	m_ReadCallback = O.m_ReadCallback;
	m_DataBuffer = O.m_DataBuffer;
	m_UserData = O.m_UserData;
	m_TargetCachedLength = O.m_TargetCachedLength;
	m_CachedBufferLength = O.m_CachedBufferLength;
	m_Position = O.m_Position;
	m_SelectedFunc = O.m_SelectedFunc;
	m_Flag = O.m_Flag;
	O.m_DataBuffer = nullptr;
	return *this;
}

int32_t LWByteStream::ReadText(uint8_t *Out, uint32_t OutLen) {
	uint32_t o = 0;
	const uint8_t *L = Out + std::min<uint32_t>(OutLen-1, OutLen);
	for (; Out != L; Out++) {
		if (!CanReadBytes(1)) return o;
		*Out = *(m_DataBuffer + m_Position++);
		o++;
		if (*Out == '\0') break;
	}
	if (o & 1) {
		if (!CanReadBytes(1)) return o; //read out padding.
		m_Position += 1;
		o++;
	}
	return o;
}

int32_t LWByteStream::ReadText(char *Out, uint32_t OutLen) {
	return ReadText((uint8_t*)Out, OutLen);
}

int32_t LWByteStream::ReadToToken(uint8_t *Out, uint32_t Outlen, uint8_t Token) {
	uint32_t c = 0;
	const uint8_t *L = Out + std::min<uint32_t>(Outlen, Outlen - 1);
	for (; Out != L; ++Out) {
		if (!CanReadBytes(1)) return c;
		*Out = *(m_DataBuffer + m_Position++);
		c++;
		if (*Out == '\0' || *Out == Token) break;
	}
	if (Outlen) *Out = '\0';
	return c;
}

int32_t LWByteStream::ReadToToken(char *Out, uint32_t OutLen, char Token) {
	return ReadToToken((uint8_t*)Out, OutLen, (uint8_t)Token);
}

bool LWByteStream::OffsetStream(uint32_t Offset) {
	if (!CanReadBytes(Offset)) return false;
	m_Position += Offset;
	return true;
}

bool LWByteStream::IsNetworkStream(void) const {
	return (m_Flag&Network) != 0;
}

uint32_t LWByteStream::GetCachedBufferLength(void) const {
	return m_TargetCachedLength;
}

uint32_t LWByteStream::GetRemainingCache(void) const {
	return m_CachedBufferLength - m_Position;
}

LWByteStream::LWByteStream(uint32_t CachedBufferLength, std::function<int32_t(int8_t*, uint32_t, void*)> DataReadCallback, uint32_t Flag, void *UserData, LWAllocator &Allocator) : m_ReadCallback(DataReadCallback), m_UserData(UserData), m_TargetCachedLength(CachedBufferLength), m_SelectedFunc((Flag&Network) == 0 ? 0 : 1), m_Flag(Flag), m_Allocator(&Allocator) {
	m_DataBuffer = m_Allocator->Allocate<int8_t>(CachedBufferLength);
}

LWByteStream::LWByteStream(LWByteStream &&O) : m_Allocator(O.m_Allocator), m_ReadCallback(O.m_ReadCallback), m_DataBuffer(O.m_DataBuffer), m_UserData(O.m_UserData), m_TargetCachedLength(O.m_TargetCachedLength), m_CachedBufferLength(O.m_CachedBufferLength), m_Position(O.m_Position), m_SelectedFunc(O.m_SelectedFunc), m_Flag(O.m_Flag) {
	O.m_DataBuffer = nullptr;
}

LWByteStream::~LWByteStream() {
	LWAllocator::Destroy(m_DataBuffer);
}
