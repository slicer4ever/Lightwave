#include <LWCore/LWTypes.h>
#include <LWCore/LWUnicode.h>
#include <LWCore/LWAllocators/LWAllocator_Default.h>
#include <LWCore/LWByteBuffer.h>
#include <LWCore/LWSVector.h>
#include <LWPlatform/LWFileStream.h>
const char FPNames[][16] = { "Fast", "Precise", "Strict", "Exceptions", "Unknown" };
const uint32_t FPFast = 0;
const uint32_t FPPrecise = 1;
const uint32_t FPStrict = 2;
const uint32_t FPExceptions = 3;
const uint32_t FPUnknown = 4;

#ifdef _M_FP_FAST
static uint32_t FPMode = FPFast;
#elif _M_FP_PRECISE
static uint32_t FPMode = FPPrecise;
#elif _M_FP_STRICT
static uint32_t FPMode = FPStrict;
#elif _M_FP_EXCEPT
static uint32_t FPMode = FPExceptions;
#else
static uint32_t FPMode = FPUnknown;
#endif

template<class Type>
bool ValidateResults(const LWUTF8Iterator &TestName, LWByteBuffer &Buffer, const Type &Value, bool WriteMode) {
	int8_t CmpBuffer[sizeof(Type)];
	assert(Buffer.GetPosition() + sizeof(Type) <= Buffer.GetBufferSize());
	if (WriteMode) {
		Buffer.Write<Type>(Value);
		return true;
	}
	int32_t c = LWByteBuffer::Write<Type>(Value, CmpBuffer);
	bool Result = memcmp(Buffer.GetReadBuffer() + Buffer.GetPosition(), CmpBuffer, c) == 0;
	if (!Result) fmt::print("Failed at test: '{}'\n", TestName);
	Buffer.OffsetPosition(c);
	return Result;
}

template<>
bool ValidateResults(const LWUTF8Iterator &TestName, LWByteBuffer &Buffer, const LWSVector4f &Value, bool WriteMode) {
	int8_t CmpBuffer[sizeof(LWSVector4f)];
	assert(Buffer.GetPosition() + sizeof(LWSVector4f) <= Buffer.GetBufferSize());
	if (WriteMode) {
		Buffer.Write<float>(Value);
		return true;
	}
	int32_t c = LWByteBuffer::Write<float>(Value, CmpBuffer);
	bool Result = memcmp(Buffer.GetReadBuffer() + Buffer.GetPosition(), CmpBuffer, c) == 0;
	if (!Result) fmt::print("Failed at test: '{}'\n", TestName);
	Buffer.OffsetPosition(c);
	return Result;
}

bool WriteFile(const LWUTF8Iterator &FileName, LWByteBuffer &Buffer, LWAllocator &Allocator) {
	LWFileStream Stream;
	if (!LWFileStream::OpenStream(Stream, LWUTF8I::Fmt<128>("{}.bin", FileName, FPNames[FPMode]), LWFileStream::WriteMode | LWFileStream::BinaryMode, Allocator)) {
		fmt::print("Error opening file '{}' for writing.\n", FileName);
		return false;
	}
	Stream.Write((char*)&FPMode, sizeof(int32_t));
	Stream.Write((const char*)Buffer.GetReadBuffer(), Buffer.GetBytesWritten());
	return true;
}

bool ReadFile(const LWUTF8Iterator &FileName, int8_t *Buffer, uint32_t BufferSize, LWAllocator &Allocator) {
	LWFileStream Stream;
	if (!LWFileStream::OpenStream(Stream, LWUTF8I::Fmt<128>("{}.bin", FileName, FPNames[FPMode]), LWFileStream::ReadMode | LWFileStream::BinaryMode, Allocator)) {
		fmt::print("Error opening file '{}' for reading, switching to write mode for test.\n", FileName);
		return false;
	}
	LWVerify(Stream.Length() <= BufferSize);
	LWVerify(Stream.Read((char*)Buffer, Stream.Length())==Stream.Length());
	uint32_t FileMode = *(int32_t*)Buffer;
	fmt::print("File '{}' was generated with '{}' mode.\n", FileName, FPNames[FPMode]);
	return true;
}

bool LinearMathTest(const LWUTF8Iterator &Name, int8_t *Buffer, uint32_t BufferSize, LWAllocator &Allocator) {
	fmt::print("Starting Test: {}\n", Name);
	char FileName[] = "LinearMathTest";
	const uint32_t FloatTestCount = 128;
	bool WriteMode = !ReadFile(FileName, Buffer, BufferSize, Allocator);
	float fBuffer[FloatTestCount];
	LWByteBuffer Buf = LWByteBuffer(Buffer + sizeof(int32_t), BufferSize, LWByteBuffer::BufferNotOwned);
	
	//Initialize fbuffer.
	for (uint32_t i = 0; i < FloatTestCount; i++) {
		fBuffer[i] = ((float)i) * 0.1f;
		if (!ValidateResults(LWUTF8I::Fmt<128>("Generating data {}", i), Buf, fBuffer[i], WriteMode)) return false;
	}
	//Sum fbuffer.
	float Sum = 0.0f;
	for (uint32_t i = 0; i < FloatTestCount; i++) {
		Sum += fBuffer[i];
		if (!ValidateResults(LWUTF8I::Fmt<128>("Summing data {}", i), Buf, Sum, WriteMode)) return false;
	}
	//Sub fbuffer.
	for (uint32_t i = 0; i < FloatTestCount; i++) {
		Sum -= fBuffer[i];
		if (!ValidateResults(LWUTF8I::Fmt<128>("Subbing data {}", i), Buf, Sum, WriteMode)) return false;
	}

	//Mul fbuffer.
	float Mul = 1.0f;
	for (uint32_t i = 0; i < FloatTestCount; i++) {
		Mul *= (fBuffer[i] + 1.0f);
		if (!ValidateResults(LWUTF8I::Fmt<128>("Multiplying data {}", i), Buf, Mul, WriteMode)) return false;
	}
	//Div fbuffer.
	float Div = 100.0f;
	for (uint32_t i = 1; i < FloatTestCount; i++) {//Skip 0.
		Div /= fBuffer[i];
		if (!ValidateResults(LWUTF8I::Fmt<128>("Dividing data {}", i), Buf, Div, WriteMode)) return false;
	}
	//Mixed operations:
	float Value = 0.0f;
	for (uint32_t i = 0; i < FloatTestCount; i += 4) {
		Value = fBuffer[i + 3] / (fBuffer[i] + fBuffer[i + 1] * fBuffer[i + 2]);
		if (!ValidateResults(LWUTF8I::Fmt<128>("Mixed operations {}", i), Buf, Value, WriteMode)) return false;
	}
	if (WriteMode) WriteFile(FileName, Buf, Allocator);
	fmt::print("Finished Test: {}\n", Name);
	return true;
}

bool TrigMathTest(const LWUTF8Iterator &Name, int8_t *Buffer, uint32_t BufferSize, LWAllocator &Allocator) {
	fmt::print("Starting Test: {}\n", Name);
	char FileName[] = "TrigMathTest";
	const uint32_t FloatTestCount = 4096;
	bool WriteMode = !ReadFile(FileName, Buffer+sizeof(int32_t), BufferSize, Allocator);
	LWByteBuffer Buf = LWByteBuffer(Buffer+sizeof(int32_t), BufferSize, LWByteBuffer::BufferNotOwned);
	float fBuffer[FloatTestCount];
	for (uint32_t i = 0; i < FloatTestCount; i++) {
		fBuffer[i] = ((float)i) * LW_DEGTORAD * 0.5f - LW_2PI*2.0f;
		if (!ValidateResults(LWUTF8I::Fmt<128>("Generating Data {}", i), Buf, fBuffer[i], WriteMode)) return false;
	}
	//sin tests:
	for (uint32_t i = 0; i < FloatTestCount; i++) {
		float s = sinf(fBuffer[i]);
		if (!ValidateResults(LWUTF8I::Fmt<128>("sin test {}", i), Buf, s, WriteMode)) return false;
	}
	//cos tests:
	for (uint32_t i = 0; i < FloatTestCount; i++) {
		float c = cosf(fBuffer[i]);
		if (!ValidateResults(LWUTF8I::Fmt<128>("cos test {}", i), Buf, c, WriteMode)) return false;
	}

	//acos tests:
	for (uint32_t i = 0; i < FloatTestCount; i++) {
		float f = std::min<float>(std::max<float>(fmodf(fBuffer[i], 1.0f), -1.0f), 1.0f);
		float ac = acosf(f);
		if (!ValidateResults(LWUTF8I::Fmt<128>("acos test {}", i), Buf, ac, WriteMode)) return false;
	}
	//asin tests:
	for (uint32_t i = 0; i < FloatTestCount; i++) {
		float f = std::min<float>(std::max<float>(fmodf(fBuffer[i], 1.0f), -1.0f), 1.0f);
		float as = asinf(f);
		if (!ValidateResults(LWUTF8I::Fmt<128>("asin test {}", i), Buf, as, WriteMode)) return false;
	}
	if (WriteMode) WriteFile(FileName, Buf, Allocator);
	fmt::print("Finished Test: {}\n", Name);
	return true;
}

bool LinearSSEMathTest(const LWUTF8Iterator &Name, int8_t *Buffer, uint32_t BufferSize, LWAllocator &Allocator) {
	fmt::print("Starting Test: {}\n", Name);
	char FileName[] = "SSELinearMathTest";
	const uint32_t TestCount = 128;
	bool WriteMode = !ReadFile(FileName, Buffer, BufferSize, Allocator);
	LWSVector4f vBuffer[TestCount];
	LWByteBuffer Buf = LWByteBuffer(Buffer + sizeof(int32_t), BufferSize, LWByteBuffer::BufferNotOwned);
	//Initialize fbuffer.
	for (uint32_t i = 0; i < TestCount; i++) {
		float n = ((float)i) * 0.1f;
		vBuffer[i] = LWSVector4f(n);
		if (!ValidateResults(LWUTF8I::Fmt<128>("Generating data {}", i), Buf, vBuffer[i], WriteMode)) return false;
	}
	//Sum fbuffer.
	LWSVector4f Sum = LWSVector4f(0.0f);
	for (uint32_t i = 0; i < TestCount; i++) {
		Sum += vBuffer[i];
		if (!ValidateResults(LWUTF8I::Fmt<128>("Summing data {}", i), Buf, Sum, WriteMode)) return false;
	}
	//Sub fbuffer.
	for (uint32_t i = 0; i < TestCount; i++) {
		Sum -= vBuffer[i];
		if (!ValidateResults(LWUTF8I::Fmt<128>("Subbing data {}", i), Buf, Sum, WriteMode)) return false;
	}
	//Mul fbuffer.
	LWSVector4f Mul = 1.0f;
	for (uint32_t i = 0; i < TestCount; i++) {
		Mul *= (vBuffer[i] + 1.0f);
		if (!ValidateResults(LWUTF8I::Fmt<128>("Multiplying data {}", i), Buf, Mul, WriteMode)) return false;
	}
	//Div fbuffer.
	LWSVector4f Div = 100.0f;
	for (uint32_t i = 1; i < TestCount; i++) {//Skip 0.
		Div /= vBuffer[i];
		if (!ValidateResults(LWUTF8I::Fmt<128>("Dividing data {}", i), Buf, Div, WriteMode)) return false;
	}
	//Mixed operations:
	LWSVector4f Value = 0.0f;
	for (uint32_t i = 0; i < TestCount; i += 4) {
		Value = vBuffer[i + 3] / (vBuffer[i] + vBuffer[i + 1] * vBuffer[i + 2]);
		if (!ValidateResults(LWUTF8I::Fmt<128>("Mixed operations {}", i), Buf, Value, WriteMode)) return false;
	}
	if (WriteMode) WriteFile(FileName, Buf, Allocator);
	fmt::print("Finished Test: {}\n", Name);
	return true;
}

int main(int argc, char **argv) {
	const uint32_t MaxBufferSize = 1024 * 1024;
	LWAllocator_Default DefAlloc;
	int8_t *Buffer = DefAlloc.Allocate<int8_t>(MaxBufferSize);
	fmt::print("Starting determnistic tests, fpmode: {}\n", FPNames[FPMode]);
	LinearMathTest("Linear Math Test.", Buffer, MaxBufferSize, DefAlloc);
	TrigMathTest("Trig Math Test.", Buffer, MaxBufferSize, DefAlloc);
	LinearSSEMathTest("Linear SSE Math Test.", Buffer, MaxBufferSize, DefAlloc);
	fmt::print("Finished deterministic tests.\n");
	LWAllocator::Destroy(Buffer);
	return 0;
}