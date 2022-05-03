#include "LWCore/LWByteBuffer.h"
#include "LWCore/LWAllocator.h"
#include <stdint.h>
#include <cmath>
#include <cstring>

const uint32_t CheckEndianValue = 0x44332211;
const bool BigEndian = ((int8_t*)&CheckEndianValue)[0] != 0x11;
const bool NetworkOrdered = BigEndian;

uint16_t inline OrderSwap(uint16_t Value){
	return (Value & 0xFF) << 8 | (Value & 0xFF00) >> 8;
}

uint32_t inline OrderSwap(uint32_t Value){
	return (Value & 0xFF) << 24 | (Value & 0xFF00) << 8 | (Value & 0xFF0000) >> 8 | (Value & 0xFF000000) >> 24;
}

uint64_t inline OrderSwap(uint64_t Value){
	return (Value & 0xFF) << 56 | (Value & 0xFF00) << 40 | (Value & 0xFF0000) << 24 | (Value & 0xFF000000) << 8 | (Value & 0xFF00000000) >> 8 | (Value & 0xFF0000000000) >> 24 | (Value & 0xFF000000000000) >> 40 | (Value & 0xFF00000000000000) >> 56;
}

int8_t LWByteBuffer::MakeNetwork(int8_t Value){
	return Value;
}

uint8_t LWByteBuffer::MakeNetwork(uint8_t Value){
	return Value;
}

char LWByteBuffer::MakeNetwork(char Value) {
	return Value;
}

int16_t LWByteBuffer::MakeNetwork(int16_t Value){
	if (NetworkOrdered) return Value;
	return (int16_t)OrderSwap((uint16_t)Value);
}

uint16_t LWByteBuffer::MakeNetwork(uint16_t Value){
	if (NetworkOrdered) return Value;
	return OrderSwap(Value);
}

int32_t LWByteBuffer::MakeNetwork(int32_t Value){
	if (NetworkOrdered) return Value;
	return (int32_t)OrderSwap((uint32_t)Value);
}

uint32_t LWByteBuffer::MakeNetwork(uint32_t Value){
	if (NetworkOrdered) return Value;
	return OrderSwap(Value);
}

int64_t LWByteBuffer::MakeNetwork(int64_t Value){
	if (NetworkOrdered) return Value;
	return (int64_t)OrderSwap((uint64_t)Value);
}

uint64_t LWByteBuffer::MakeNetwork(uint64_t Value){
	if (NetworkOrdered) return Value;
	return OrderSwap(Value);
}

uint32_t LWByteBuffer::MakeNetwork(float Value){
	union{
		float f;
		uint32_t u;
	};
	f = Value;
	if (NetworkOrdered) return u;
	return OrderSwap(u);

}

uint64_t LWByteBuffer::MakeNetwork(double Value){
	union{
		double f;
		uint64_t u;
	};
	f = Value;
	if (NetworkOrdered) return u;
	return OrderSwap(u);
}

int8_t LWByteBuffer::MakeHost(int8_t Value){
	return Value;
}

uint8_t LWByteBuffer::MakeHost(uint8_t Value){
	return Value;
}

char LWByteBuffer::MakeHost(char Value) {
	return Value;
}

int16_t LWByteBuffer::MakeHost(int16_t Value){
	if (NetworkOrdered) return Value;
	return (int16_t)OrderSwap((uint16_t)Value);
}

uint16_t LWByteBuffer::MakeHost(uint16_t Value){
	if (NetworkOrdered) return Value;
	return OrderSwap(Value);
}

int32_t LWByteBuffer::MakeHost(int32_t Value){
	if (NetworkOrdered) return Value;
	return (int32_t)OrderSwap((uint32_t)Value);
}

uint32_t LWByteBuffer::MakeHost(uint32_t Value){
	if (NetworkOrdered) return Value;
	return OrderSwap(Value);
}

int64_t LWByteBuffer::MakeHost(int64_t Value){
	if (NetworkOrdered) return Value;
	return (int64_t)OrderSwap((uint64_t)Value);
}
 
uint64_t LWByteBuffer::MakeHost(uint64_t Value){
	if (NetworkOrdered) return Value;
	return OrderSwap(Value);
}

float LWByteBuffer::MakeHostf(uint32_t Value){
	union{
		float f;
		uint32_t u;
	};
	u = Value;
	if (NetworkOrdered) return f;
	u = OrderSwap(u);
	return f;
}

double LWByteBuffer::MakeHostf(uint64_t Value){
	union{
		double f;
		uint64_t u;
	};
	u = Value;
	if (NetworkOrdered) return f;
	u = OrderSwap(u);
	return f;
}

int8_t LWByteBuffer::MakeBig(int8_t Value){
	return Value;
}

uint8_t LWByteBuffer::MakeBig(uint8_t Value){
	return Value;
}

char LWByteBuffer::MakeBig(char Value) {
	return Value;
}

int16_t LWByteBuffer::MakeBig(int16_t Value){
	if (BigEndian) return Value;
	return (int16_t)OrderSwap((uint16_t)Value);
}

uint16_t LWByteBuffer::MakeBig(uint16_t Value){
	if (BigEndian) return Value;
	return OrderSwap(Value);
}

int32_t LWByteBuffer::MakeBig(int32_t Value){
	if (BigEndian) return Value;
	return (int32_t)OrderSwap((uint32_t)Value);
}

uint32_t LWByteBuffer::MakeBig(uint32_t Value){
	if (BigEndian) return Value;
	return OrderSwap(Value);
}

int64_t LWByteBuffer::MakeBig(int64_t Value){
	if (BigEndian) return Value;
	return (int64_t)OrderSwap((uint64_t)Value);
}

uint64_t LWByteBuffer::MakeBig(uint64_t Value){
	if (BigEndian) return Value;
	return OrderSwap(Value);
}

float LWByteBuffer::MakeBig(float Value){
	if (BigEndian) return Value;
	union{
		float f;
		uint32_t u;
	};
	f = Value;
	u = OrderSwap(u);
	return f;
}

double LWByteBuffer::MakeBig(double Value){
	if (BigEndian) return Value;
	union{
		double f;
		uint64_t u;
	};
	f = Value;
	u = OrderSwap(u);
	return f;
}

int8_t LWByteBuffer::MakeLittle(int8_t Value){
	return Value;
}

uint8_t LWByteBuffer::MakeLittle(uint8_t Value){
	return Value;
}

char LWByteBuffer::MakeLittle(char Value) {
	return Value;
}

int16_t LWByteBuffer::MakeLittle(int16_t Value){
	if (!BigEndian) return Value;
	return (int16_t)OrderSwap((uint16_t)Value);
}

uint16_t LWByteBuffer::MakeLittle(uint16_t Value){
	if (!BigEndian) return Value;
	return OrderSwap(Value);
}

int32_t LWByteBuffer::MakeLittle(int32_t Value){
	if (!BigEndian) return Value;
	return (int32_t)OrderSwap((uint32_t)Value);
}

uint32_t LWByteBuffer::MakeLittle(uint32_t Value){
	if (!BigEndian) return Value;
	return OrderSwap(Value);
}

int64_t LWByteBuffer::MakeLittle(int64_t Value){
	if (!BigEndian) return Value;
	return (int64_t)OrderSwap((uint64_t)Value);
}

uint64_t LWByteBuffer::MakeLittle(uint64_t Value){
	if (!BigEndian) return Value;
	return OrderSwap(Value);
}

float LWByteBuffer::MakeLittle(float Value){
	if (!BigEndian) return Value;
	union{
		uint32_t u;
		float f;
	};
	f = Value;
	u = OrderSwap(u);
	return f;
}

double LWByteBuffer::MakeLittle(double Value){
	if (!BigEndian) return Value;
	union{
		uint64_t u;
		double f;
	};
	f = Value;
	u = OrderSwap(u);
	return f;
}

int32_t LWByteBuffer::VariantLength(uint16_t Value) {
	if (Value >= 0x4000) return 3;
	if (Value >= 0x80) return 2;
	return 1;
}

int32_t LWByteBuffer::VariantLength(int16_t Value) {
	return VariantLength((uint64_t)Value);
}

int32_t LWByteBuffer::VariantLength(uint32_t Value) {
	if (Value >= 0x10000000) return 5;
	if (Value >= 0x200000) return 4;
	if (Value >= 0x4000) return 3;
	if (Value >= 0x80) return 2;
	return 1;
}

int32_t LWByteBuffer::VariantLength(int32_t Value) {
	return VariantLength((uint64_t)Value);
}

int32_t LWByteBuffer::VariantLength(uint64_t Value) {
	if (Value >= 0x8000000000000000) return 10;
	if (Value >= 0x100000000000000) return 9;
	if (Value >= 0x2000000000000) return 8;
	if (Value >= 0x40000000000) return 7;
	if (Value >= 0x800000000) return 6;
	if (Value >= 0x10000000) return 5;
	if (Value >= 0x200000) return 4;
	if (Value >= 0x4000) return 3;
	if (Value >= 0x80) return 2;
	return 1;
}

int32_t LWByteBuffer::VariantLength(int64_t Value) {
	return VariantLength((uint64_t)Value);
}

int32_t LWByteBuffer::SVariantLength(int16_t Value) {
	return VariantLength((uint16_t)((Value << 1) ^ (Value >> 15)));
}

int32_t LWByteBuffer::SVariantLength(int32_t Value) {
	return VariantLength((uint32_t)((Value << 1) ^ (Value >> 31)));
}

int32_t LWByteBuffer::SVariantLength(int64_t Value) {
	return VariantLength((uint64_t)((Value << 1) ^ (Value >> 63)));
}

int32_t LWByteBuffer::WriteVariant(uint16_t Value, int8_t *Buffer, int32_t MinBytes) {
	int32_t o = 0;
	for (; Value >= 0x80 || (o+1)<MinBytes; Value >>= 7, ++o) {
		if (Buffer) Buffer[o] = (uint8_t)Value | 0x80;
	}
	if (Buffer) Buffer[o] = (uint8_t)Value;
	return ++o;
}

int32_t LWByteBuffer::WriteVariant(int16_t Value, int8_t *Buffer, int32_t MinBytes) {
	return WriteVariant((uint64_t)Value, Buffer, MinBytes);
}

int32_t LWByteBuffer::WriteVariant(uint32_t Value, int8_t *Buffer, int32_t MinBytes) {
	int32_t o = 0;
	for (; Value >= 0x80 || (o+1)<MinBytes; Value >>= 7, ++o) {
		if (Buffer) Buffer[o] = (uint8_t)Value | 0x80;
	}
	if (Buffer) Buffer[o] = (uint8_t)Value;
	return ++o;
}

int32_t LWByteBuffer::WriteVariant(int32_t Value, int8_t *Buffer, int32_t MinBytes) {
	return WriteVariant((uint64_t)Value, Buffer, MinBytes);
}

int32_t LWByteBuffer::WriteVariant(uint64_t Value, int8_t *Buffer, int32_t MinBytes) {
	int32_t o = 0;
	for (; Value >= 0x80 || (o+1)<MinBytes; Value >>= 7, ++o) {
		if (Buffer) Buffer[o] = (uint8_t)Value | 0x80;
	}
	if (Buffer) Buffer[o] = (uint8_t)Value;
	return ++o;
}

int32_t LWByteBuffer::WriteSVariant(int16_t Value, int8_t *Buffer, int32_t MinBytes) {
	return WriteVariant((uint16_t)((Value << 1) ^ (Value >> 15)), Buffer, MinBytes);
}

int32_t LWByteBuffer::WriteSVariant(int32_t Value, int8_t *Buffer, int32_t MinBytes) {
	return WriteVariant((uint32_t)((Value << 1) ^ (Value >> 31)), Buffer, MinBytes);
}

int32_t LWByteBuffer::WriteSVariant(int64_t Value, int8_t *Buffer, int32_t MinBytes) {
	return WriteVariant((uint64_t)((Value << 1) ^ (Value >> 63)), Buffer, MinBytes);
}

int32_t LWByteBuffer::WritePointer(void *Value, int8_t *Buffer){
	if(Buffer) *(void**)Buffer = Value;
	return sizeof(void*);
}

int32_t LWByteBuffer::WriteStorage(int32_t Len, int8_t *&Buffer) {
	Buffer = nullptr;
	if (m_Position + Len > m_BufferSize) return Len;
	Buffer = m_WriteBuffer + m_Position;
	m_Position += Len;
	m_BytesWritten += Len;
	return Len;
}

int32_t LWByteBuffer::WriteText(const uint8_t *Text, int8_t *Buffer) {
	const uint8_t *P = Text;
	uint32_t o = 0;
	for (; *P; ++P, o++)
		if (Buffer) *Buffer++ = *P;
	if (Buffer) *Buffer++ = 0;
	o++;
	if (o & 1) {
		if (Buffer) *Buffer++ = 0;
		o++;
	}
	return o;
}

int32_t LWByteBuffer::WriteText(const char *Text, int8_t *Buffer) {
	return WriteText((const uint8_t*)Text, Buffer);
}

int32_t LWByteBuffer::ReadText(uint8_t *Out, uint32_t OutLen, const int8_t *Buffer, const uint32_t BufferLen) {
	uint32_t o = 0;
	uint8_t *oP = Out;
	uint8_t *oL = oP + std::min<uint32_t>(OutLen - 1, OutLen);
	const int8_t *bP = Buffer;
	const int8_t *bL = Buffer + BufferLen;
	for (; bP != bL && *bP; ++bP) {
		if (oP < oL) *oP++ = *bP;
		o++;
	}
	if (OutLen) *oP = '\0';
	o++;
	if (o & 1) o++; //Remove padding if it was added.
	return o;
}

int32_t LWByteBuffer::ReadText(char *Out, uint32_t OutLen, const int8_t *Buffer, const uint32_t BufferLen) {
	return ReadText((uint8_t*)Out, OutLen, Buffer, BufferLen);
}

int32_t LWByteBuffer::WriteText(const uint8_t *Text) {
	int32_t Length = LWByteBuffer::WriteText(Text, nullptr);
	if (m_Position + Length > m_BufferSize) return Length;
	m_Position += LWByteBuffer::WriteText(Text, m_WriteBuffer ? m_WriteBuffer + m_Position : nullptr);
	m_BytesWritten += Length;
	return Length;
}
	
int32_t LWByteBuffer::WriteText(const char *Text) {
	return WriteText((const uint8_t*)Text);
}

int32_t LWByteBuffer::ReadVariant(uint16_t *Out, const int8_t *Buffer, const int8_t *BufferLast) {
	uint64_t Val;
	int32_t r = ReadVariant(Val, Buffer, BufferLast);
	if (Out) *Out = (uint16_t)Val;
	return r;
}

int32_t LWByteBuffer::ReadVariant(int16_t *Out, const int8_t *Buffer, const int8_t *BufferLast) {
	uint64_t Val;
	int32_t r = ReadVariant(Val, Buffer, BufferLast);
	if (Out) *Out = (int16_t)Val;
	return r;

}

int32_t LWByteBuffer::ReadVariant(uint32_t *Out, const int8_t *Buffer, const int8_t *BufferLast) {
	uint64_t Val;
	int32_t r = ReadVariant(Val, Buffer, BufferLast);
	if (Out) *Out = (uint32_t)Val;
	return r;
}

int32_t LWByteBuffer::ReadVariant(int32_t *Out, const int8_t *Buffer, const int8_t *BufferLast) {
	uint64_t Val;
	int32_t r = ReadVariant(Val, Buffer, BufferLast);
	if (Out) *Out = (int32_t)Val;
	return r;
}

int32_t LWByteBuffer::ReadVariant(uint64_t *Out, const int8_t *Buffer, const int8_t *BufferLast) {
	uint64_t Val;
	int32_t r = ReadVariant(Val, Buffer, BufferLast);
	if (Out) *Out = Val;
	return r;
}

int32_t LWByteBuffer::ReadVariant(int64_t *Out, const int8_t *Buffer, const int8_t *BufferLast) {
	uint64_t Val;
	int32_t r = ReadVariant(Val, Buffer, BufferLast);
	if (Out) *Out = (int64_t)Val;
	return r;
}

int32_t LWByteBuffer::ReadVariant(uint64_t &Out, const int8_t *Buffer, const int8_t *BufferLast) {
	uint64_t n = 0;
	int32_t o = 0;
	uint8_t *B = (uint8_t*)Buffer;
	uint8_t *BL = (uint8_t*)BufferLast;

	Out = 0;
	for (; B+o<BL && (B[o] & 0x80) != 0; n += 7) Out = Out | ((((uint64_t)B[o++]) & 0x7F) << n);
	if(B+o<BL) Out |= (((uint64_t)B[o++])&0x7f) << n;
	return o;
}

int32_t LWByteBuffer::ReadSVariant(int16_t *Out, const int8_t *Buffer, const int8_t *BufferLast) {
	int16_t Val = 0;
	int32_t r = ReadVariant(&Val, Buffer, BufferLast);
	if (Out) *Out = ((Val >> 1) ^ -(Val & 1));
	return r;
}

int32_t LWByteBuffer::ReadSVariant(int32_t *Out, const int8_t *Buffer, const int8_t *BufferLast) {
	int32_t Val = 0;
	int32_t r = ReadVariant(&Val, Buffer, BufferLast);
	if (Out) *Out = ((Val >> 1) ^ -(Val & 1));
	return r;
}

int32_t LWByteBuffer::ReadSVariant(int64_t *Out, const int8_t *Buffer, const int8_t *BufferLast) {
	int64_t Val = 0;
	int32_t r = ReadVariant(&Val, Buffer, BufferLast);
	if (Out) *Out = ((Val >> 1) ^ -(Val & 1));
	return r;
}

int32_t LWByteBuffer::ReadPointer(void **Value, int8_t *Buffer) {
	if (Value) *Value = *(void**)Buffer;
	return sizeof(void*);
}

int32_t LWByteBuffer::ReadText(uint8_t *Out, uint32_t OutLen) {
	int32_t Length = ReadText(Out, OutLen, m_ReadBuffer + m_Position, (uint32_t)(m_BufferSize-m_Position));
	m_Position += Length;
	return Length;
}

int32_t LWByteBuffer::ReadText(char *Out, uint32_t OutLen) {
	return ReadText((uint8_t*)Out, OutLen);
}

int32_t LWByteBuffer::ReadText(uint8_t *Out, uint32_t OutLen, int32_t Position) {
	if (Position >= m_BufferSize) return 0;
	return LWByteBuffer::ReadText(Out, OutLen, m_ReadBuffer + Position, (uint32_t)(m_BufferSize-Position));
}

int32_t LWByteBuffer::ReadText(char *Out, uint32_t OutLen, int32_t Position) {
	return ReadText((uint8_t*)Out, OutLen, Position);
}

int32_t LWByteBuffer::WritePointer(void *Value) {
	int32_t Len = sizeof(void*);
	if (m_Position + Len > m_BufferSize) return Len;
	m_Position += WritePointer(Value, m_WriteBuffer ? m_WriteBuffer + m_Position : nullptr);
	m_BytesWritten += Len;
	return Len;
}

LWByteBuffer &LWByteBuffer::SetBytesWritten(uint32_t BytesWritten){
	m_BytesWritten = BytesWritten;
	return *this;
}

LWByteBuffer &LWByteBuffer::SetPosition(int32_t Position){
	m_Position = Position;
	return *this;
}

int32_t LWByteBuffer::AlignPosition(uint32_t Alignment, bool Write) {
	int32_t r = Alignment - (m_Position & (Alignment - 1));
	if (m_Position + r > m_BufferSize) return r;
	m_Position += r;
	if (Write) m_BytesWritten += r;
	return r;
}

LWByteBuffer &LWByteBuffer::Seek(int32_t Offset, bool Write){
	m_Position += Offset;
	if(Write) m_BytesWritten = std::max<uint32_t>(m_BytesWritten, m_Position);
	return *this;
}

int32_t LWByteBuffer::GetBufferSize(void) const{
	return m_BufferSize;
}

int32_t LWByteBuffer::GetBytesWritten(void) const{
	return m_BytesWritten;
}

int32_t LWByteBuffer::GetPosition(void) const{
	return m_Position;
}

bool LWByteBuffer::IsEndOfReadData(void) const{
	return m_Position >= m_BufferSize;
}

bool LWByteBuffer::IsEndOfWriteData(void) const {
	return m_WriteBuffer==nullptr || m_BytesWritten>=m_BufferSize;
}

const int8_t *LWByteBuffer::GetReadBuffer(void) const{
	return m_ReadBuffer;
}

LWByteBuffer::LWByteBuffer(int8_t *Buffer, uint32_t BufferSize, uint8_t Flag) : m_WriteBuffer(Flag&ReadOnly ? nullptr : Buffer), m_ReadBuffer(Buffer), m_BufferSize(BufferSize), m_SelectedFunc((Flag&Network) ? 1 : 0), m_Flag(Flag){}

LWByteBuffer::LWByteBuffer(const int8_t *ReadBuffer, uint32_t BufferSize, uint8_t Flag) : m_ReadBuffer(ReadBuffer), m_BufferSize(BufferSize), m_SelectedFunc((Flag&Network) ? 1 : 0), m_Flag(Flag | ReadOnly){}

LWByteBuffer::~LWByteBuffer(){
	if (m_Flag&BufferOwned) LWAllocator::Destroy(m_WriteBuffer);
}
