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
	int8_t *nBuffer = Allocator.AllocateArray<int8_t>(NewSize);
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

int32_t LWByteStream::ReadUTF8(uint8_t *Out, uint32_t OutLen) {
	typedef int32_t(*FuncA_T)(uint16_t*, const int8_t *);
	typedef int32_t(*FuncB_T)(uint8_t*, uint32_t, const int8_t*);

	FuncA_T FuncA[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
	FuncB_T FuncB[] = { LWByteBuffer::Read, LWByteBuffer::ReadNetwork };
	if (!CanReadBytes(sizeof(uint16_t))) return 0;
	uint16_t Len = 0;
	m_Position += FuncA[m_SelectedFunc](&Len, m_DataBuffer + m_Position);
	Len = std::min<int16_t>(Len, OutLen);
	if (!CanReadBytes(sizeof(uint8_t)*Len)) return 0;
	m_Position += FuncB[m_SelectedFunc](Out, Len, m_DataBuffer + m_Position);
	if (Len == OutLen) *(Out + (Len - 1)) = '\0';
	else *(Out + Len) = '\0';
	return sizeof(int16_t) + Len;
}

int32_t LWByteStream::ReadUTF8(char *Out, uint32_t OutLen) {
	return ReadUTF8((uint8_t*)Out, OutLen);
}

int32_t LWByteStream::ReadText(uint8_t *Out, uint32_t OutLen) {
	uint32_t c = 0;
	const uint8_t *L = Out + OutLen;
	for (; Out != L; Out++) {
		if (!CanReadBytes(1)) return c;
		*Out = *(m_DataBuffer + m_Position++);
		c++;
		if (*Out == '\0') break;
	}
	if (Out == L) *(Out - 1) = '\0';
	return c;
}

int32_t LWByteStream::ReadText(char *Out, uint32_t OutLen) {
	return ReadText((uint8_t*)Out, OutLen);
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
	m_DataBuffer = m_Allocator->AllocateArray<int8_t>(CachedBufferLength);
}

LWByteStream::LWByteStream(LWByteStream &&O) : m_Allocator(O.m_Allocator), m_ReadCallback(O.m_ReadCallback), m_DataBuffer(O.m_DataBuffer), m_UserData(O.m_UserData), m_TargetCachedLength(O.m_TargetCachedLength), m_CachedBufferLength(O.m_CachedBufferLength), m_Position(O.m_Position), m_SelectedFunc(O.m_SelectedFunc), m_Flag(O.m_Flag) {
	O.m_DataBuffer = nullptr;
}

LWByteStream::~LWByteStream() {
	LWAllocator::Destroy(m_DataBuffer);
}
