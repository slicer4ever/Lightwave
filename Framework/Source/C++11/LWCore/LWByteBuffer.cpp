#include "LWCore/LWByteBuffer.h"
#include "LWCore/LWAllocator.h"
#include "LWCore/LWText.h"
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


int32_t LWByteBuffer::WritePointer(void *Value, int8_t *Buffer){
	if(Buffer) *(void**)Buffer = Value;
	return sizeof(void*);
}

int32_t LWByteBuffer::WriteUTF8(const uint8_t *Text, int8_t *Buffer){
	uint32_t Len = (uint32_t)std::strlen((const char*)Text);
	uint32_t Pad = Len & 1; //Pad for arm processors to be div by 2 or 4.
	uint32_t o = LWByteBuffer::Write<uint16_t>((uint16_t)Len+Pad, Buffer);
	o += LWByteBuffer::Write<uint8_t>(Len, Text, Buffer?Buffer + o:Buffer);
	if (Pad) o += LWByteBuffer::Write<uint8_t>('\0', Buffer?Buffer+o:Buffer);
	return o;
}

int32_t LWByteBuffer::WriteUTF8(const char *Text, int8_t *Buffer){
	return WriteUTF8((const uint8_t*)Text, Buffer);
}

int32_t LWByteBuffer::WriteText(const uint8_t *Text, int8_t *Buffer) {
	uint32_t c = 0;
	if (Buffer) {
		for (; Text[c]; c++) Buffer[c] = Text[c];
		Buffer[c] = Text[c];
	} else {
		for (; Text[c]; c++) {}
	}
	c++;
	return c;
}

int32_t LWByteBuffer::WriteText(const char *Text, int8_t *Buffer) {
	return WriteText((const uint8_t*)Text, Buffer);
}

int32_t LWByteBuffer::WriteNetworkUTF8(const uint8_t *Text, int8_t *Buffer){
	uint32_t Len = (uint32_t)std::strlen((const char*)Text);
	uint32_t Pad = Len & 1; //Pad for arm processors to be div by 2 or 4.
	uint32_t o = LWByteBuffer::WriteNetwork<uint16_t>((uint16_t)(Len+Pad), Buffer);
	o += LWByteBuffer::WriteNetwork<uint8_t>(Len, Text, Buffer?Buffer + o:Buffer);
	if (Pad) o += LWByteBuffer::WriteNetwork<uint8_t>('\0', Buffer ? Buffer + o : Buffer);
	return o;
}

int32_t LWByteBuffer::WriteNetworkUTF8(const char *Text, int8_t *Buffer){
	return WriteNetworkUTF8((const uint8_t*)Text, Buffer);
}

int32_t LWByteBuffer::ReadUTF8(uint8_t *Out, uint32_t OutLen, const int8_t *Buffer, const uint32_t BufferLen){
	uint32_t o = 0;
	uint16_t Len = 0;
	if (BufferLen < sizeof(uint16_t)) return 0;
	o += LWByteBuffer::Read<uint16_t>(&Len, Buffer);
	Len = std::min<uint32_t>(Len, (BufferLen - sizeof(uint16_t)));
	if (Out && OutLen > 0){
		uint32_t OLen = Len >= OutLen ? OutLen - 1 : Len;
		o += LWByteBuffer::Read<uint8_t>(Out, OLen, Buffer+o);
		*(Out + OLen) = '\0';
	}
	return sizeof(uint16_t)+Len;
}

int32_t LWByteBuffer::ReadUTF8(char *Out, uint32_t OutLen, const int8_t *Buffer, const uint32_t BufferLen){
	return ReadUTF8((uint8_t*)Out, OutLen, Buffer, BufferLen);
}

int32_t LWByteBuffer::ReadText(uint8_t *Out, uint32_t OutLen, const int8_t *Buffer, const uint32_t BufferLen) {
	uint32_t c = 0;
	const int8_t *L = Buffer + BufferLen;
	if (Out) {
		Out[0] = '\0'; //In case nothing to write to.
		for (; Buffer[c] && (Buffer+c)!=L; c++) {
			if (c < OutLen) Out[c] = Buffer[c];
		}
		if (c < OutLen) {
			Out[c] = Buffer[c];
			if (c == OutLen - 1) Out[c] = '\0';
		}
	} else {
		for (; Buffer[c] && (Buffer+c)!=L; c++) {}
	}
	c++;
	return c;
}

int32_t LWByteBuffer::ReadText(char *Out, uint32_t OutLen, const int8_t *Buffer, const uint32_t BufferLen) {
	return ReadText((uint8_t*)Out, OutLen, Buffer, BufferLen);
}

int32_t LWByteBuffer::ReadNetworkUTF8(uint8_t *Out, uint32_t OutLen, const int8_t *Buffer, const uint32_t BufferLen){
	uint32_t o = 0;
	uint16_t Len = 0;
	if (BufferLen < sizeof(uint16_t)) return 0;
	o += LWByteBuffer::ReadNetwork<uint16_t>(&Len, Buffer);
	Len = std::min<uint32_t>(Len, BufferLen - sizeof(uint16_t));
	if (Out && OutLen > 0){
		uint32_t OLen = Len >= OutLen ? OutLen - 1 : Len;
		o += LWByteBuffer::Read<uint8_t>(Out, OLen, Buffer + o);
		*(Out + OLen) = '\0';
	}
	return sizeof(uint16_t)+Len;
}

int32_t LWByteBuffer::ReadNetworkUTF8(char *Out, uint32_t OutLen, const int8_t *Buffer, const uint32_t BufferLen){
	return ReadNetworkUTF8((uint8_t*)Out, OutLen, Buffer, BufferLen);
}

int32_t LWByteBuffer::WriteUTF8(const uint8_t *Text){
	typedef int32_t(*Func_T)(const uint8_t *, int8_t*);
	Func_T Funcs[] = { LWByteBuffer::WriteUTF8, LWByteBuffer::WriteNetworkUTF8 };
	int32_t Length = Funcs[m_SelectedFunc](Text, nullptr);
	if (m_Position + Length > m_BufferSize) return Length;
	m_Position += Funcs[m_SelectedFunc](Text, m_WriteBuffer ? m_WriteBuffer + m_Position : nullptr);
	m_BytesWritten += Length;
	return Length;
}

int32_t LWByteBuffer::WriteUTF8(const char *Text){
	return WriteUTF8((const uint8_t*)Text);
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

int32_t LWByteBuffer::ReadUTF8(uint8_t *Out, uint32_t OutLen){
	typedef int32_t(*Func_T)(uint8_t *, uint32_t, const int8_t*, const uint32_t);
	Func_T Funcs[] = { LWByteBuffer::ReadUTF8, LWByteBuffer::ReadNetworkUTF8 };
	int32_t Length = Funcs[m_SelectedFunc](Out, OutLen, m_ReadBuffer + m_Position, (uint32_t)(m_BufferSize-m_Position));
	m_Position += Length;
	return Length;
}

int32_t LWByteBuffer::ReadUTF8(char *Out, uint32_t OutLen){
	return ReadUTF8((uint8_t*)Out, OutLen);
}

int32_t LWByteBuffer::ReadUTF8(uint8_t *Out, uint32_t OutLen, int32_t Position){
	typedef int32_t(*Func_T)(uint8_t *, uint32_t, const int8_t*, const uint32_t);
	Func_T Funcs[] = { LWByteBuffer::ReadUTF8, LWByteBuffer::ReadNetworkUTF8 };
	int32_t Length = Funcs[m_SelectedFunc](Out, OutLen, m_ReadBuffer + Position, (uint32_t)(m_BufferSize-Position));
	return Length;
}

int32_t LWByteBuffer::ReadUTF8(char *Out, uint32_t OutLen, int32_t Position){
	return ReadUTF8((uint8_t*)Out, OutLen, Position);
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
	return LWByteBuffer::ReadText(Out, OutLen, m_ReadBuffer + Position, (uint32_t)(m_BufferSize-Position));
}

int32_t LWByteBuffer::ReadText(char *Out, uint32_t OutLen, int32_t Position) {
	return ReadText((uint8_t*)Out, OutLen, Position);
}

LWByteBuffer &LWByteBuffer::SetBytesWritten(uint32_t BytesWritten){
	m_BytesWritten = BytesWritten;
	return *this;
}

LWByteBuffer &LWByteBuffer::SetPosition(int32_t Position){
	m_Position = Position;
	return *this;
}

LWByteBuffer &LWByteBuffer::AlignPosition(uint32_t Alignment, bool Write) {
	if (Alignment < 2) return *this;
	uint32_t r = (m_Position & (Alignment - 1));
	if (r) {
		r = Alignment - r;
		m_Position += r;
		if (Write) m_BytesWritten += r;
	}
	return *this;
}

LWByteBuffer &LWByteBuffer::OffsetPosition(int32_t Offset){
	m_Position += Offset;
	return *this;
}

int32_t LWByteBuffer::GetBufferSize(void){
	return m_BufferSize;
}

int32_t LWByteBuffer::GetBytesWritten(void){
	return m_BytesWritten;
}

int32_t LWByteBuffer::GetPosition(void){
	return m_Position;
}

bool LWByteBuffer::EndOfData(void){
	return m_Position >= m_BytesWritten;
}

bool LWByteBuffer::EndOfBuffer(void){
	return m_Position >= m_BufferSize;
}

const int8_t *LWByteBuffer::GetReadBuffer(void){
	return m_ReadBuffer;
}

LWByteBuffer::LWByteBuffer(int8_t *Buffer, uint32_t BufferSize, uint8_t Flag) : m_WriteBuffer(Flag&ReadOnly ? nullptr : Buffer), m_ReadBuffer(Buffer), m_BufferSize(BufferSize), m_SelectedFunc((Flag&Network) ? 1 : 0), m_Flag(Flag){}

LWByteBuffer::LWByteBuffer(const int8_t *ReadBuffer, uint32_t BufferSize, uint8_t Flag) : m_ReadBuffer(ReadBuffer), m_BufferSize(BufferSize), m_SelectedFunc((Flag&Network) ? 1 : 0), m_Flag(Flag | ReadOnly){}

LWByteBuffer::~LWByteBuffer(){
	if (!(m_Flag&BufferNotOwned)) LWAllocator::Destroy(m_WriteBuffer);
}
