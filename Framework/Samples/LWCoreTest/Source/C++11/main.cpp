#include <LWCore/LWTypes.h>
#include <LWCore/LWByteBuffer.h>
#include <LWCore/LWByteStream.h>
#include <LWCore/LWMath.h>
#include <LWCore/LWVector.h>
#include <LWCore/LWSVector.h>
#include <LWCore/LWMatrix.h>
#include <LWCore/LWSMatrix.h>
#include <LWCore/LWQuaternion.h>
#include <LWCore/LWSQuaternion.h>
#include <LWCore/LWTimer.h>
#include <LWCore/LWUnicode.h> 
#include <LWCore/LWAllocators/LWAllocator_Default.h>
#include <LWCore/LWAllocators/LWAllocator_DefaultDebug.h>
#include <LWCore/LWAllocators/LWAllocator_ConcurrentCircular.h>
#include <LWCore/LWAllocators/LWAllocator_LocalCircular.h>
#include <LWCore/LWAllocators/LWAllocator_LocalHeap.h>
#include <LWCore/LWConcurrent/LWFIFO.h>
#include <LWCore/LWCrypto.h> 
#include <thread> 
#include <iostream>
#include <iomanip>
#include <clocale>
#include <functional>
#include <chrono>
#include <string>
#include <cstdarg>

template<typename Func, class ResultType>
bool PerformTest(const char *FunctionName, Func F, ResultType ExpectedResult, bool ResultHex = false){
	ResultType R = F();
	if (ResultHex) fmt::print("Test: '{}' Expected: {:#x} Receivied: {:#x}\n", FunctionName, ExpectedResult, R);
	else fmt::print("Test: '{}' Expected: {} Received: {}\n", FunctionName, ExpectedResult, R);
	return R == ExpectedResult;
}

template<typename Func>
bool PerformTest(const char *FunctionName, Func F, float ExpectedResult, bool ResultHex = false){
	float R = F();
	fmt::print("Test: '{}' Expected: {} Received: {}\n", FunctionName, ExpectedResult, R);
	return std::abs(R - ExpectedResult) < std::numeric_limits<float>::epsilon();
}

template<typename Func>
bool PerformTest(const char *FunctionName, Func F, double ExpectedResult, bool ResultHex = false){
	double e = (double)std::numeric_limits<float>::epsilon(); //uses float epsilon instead of double as double epsilon precision is too high for some of the tests.
	double R = F();
	fmt::print("Test: '{}' Expected: {} Received: {}\n", FunctionName, ExpectedResult, R);
	return std::abs(R - ExpectedResult) < e;
}

template<typename Func>
bool PerformTest(const char *FunctionName, Func F, bool ExpectedResult, bool ResultHex = false) {
	bool R = F();
	fmt::print("Test: '{}' Expected: {} Receivied: {}\n", FunctionName, ExpectedResult, R);
	return R == ExpectedResult;
}

template<class Type>
bool TestEquality(const char *TestName, const Type &A, const Type &B){
	bool Result = A == B;
	fmt::print("Test: '{}' Result: {}\n", TestName, (Result ? "Correct" : "Incorrect"));
	return Result;
}

template<class Type>
bool TestEquality(const char *TestName, uint32_t Cnt, const Type *A, const Type *B) {
	bool Result = true;
	for (uint32_t i = 0; i < Cnt && Result; i++) Result = A[i] == B[i];
	fmt::print("Test: '{}' Result: {}\n", TestName, (Result ? "Correct" : "Incorrect"));
	return Result;
}

bool PerformAllocatorTest(const char *AllocatorName, LWAllocator &Allocator){
	fmt::print("Beginning allocation testing for: {}\n", AllocatorName);
	const int32_t AllocationCount = 2500;
	const int32_t LargeCount = 1000;
	const int32_t SmallCount = 2;
	auto Start = std::chrono::steady_clock::now();
	int32_t *m_Allocations32t[AllocationCount];
	LWVector4i *m_AllocationsVec4t[AllocationCount]; 
	fmt::print("Performing small POD allocation Tests.\n");
	for (int32_t i = 0; i < AllocationCount; i++){
		m_Allocations32t[i] = Allocator.Allocate<int32_t>(SmallCount);
		if (!m_Allocations32t[i]){
			fmt::print("Error allocation: {}\n", i);
			return false;
		}
		if (LWAllocator::GetAllocationSize(m_Allocations32t[i]) != sizeof(int32_t) * SmallCount) {
			fmt::print("Error allocating: {} Got: {} Expected: {}\n", i, LWAllocator::GetAllocationSize(m_Allocations32t[i]), sizeof(int32_t) * SmallCount);
			return false;
		}
		for (int32_t d = 0; d < 2; d++) m_Allocations32t[i][d] = d; //populate data.
	}
	for (int32_t i = 0; i < AllocationCount; i++){
		for (int32_t d = 0; d < SmallCount; d++) if (m_Allocations32t[i][d] != d) return false; //verify data.
		LWAllocator::Destroy(m_Allocations32t[i]);
	}
	fmt::print("Performing large POD allocation Tests.\n");
	for (int32_t i = 0; i < AllocationCount; i++){
		m_Allocations32t[i] = Allocator.Allocate<int32_t>(LargeCount);
		if (!m_Allocations32t[i]){
			fmt::print("Error allocation: {}\n", i);
			return false;
		}
		if (LWAllocator::GetAllocationSize(m_Allocations32t[i]) != sizeof(int32_t) * LargeCount) {
			fmt::print("Error allocating: {} Got: {} Expected: {}\n", i, LWAllocator::GetAllocationSize(m_Allocations32t[i]), sizeof(int32_t) * LargeCount);
			return false;
		}

		for (int32_t d = 0; d < LargeCount; d++) m_Allocations32t[i][d] = d; //populate data.
	}
	for (int32_t i = 0; i < AllocationCount; i++){
		for (int32_t d = 0; d < LargeCount; d++) if (m_Allocations32t[i][d] != d) return false; //verify data.
		LWAllocator::Destroy(m_Allocations32t[i]);
	}
	fmt::print("Performing mixed POD allocation/deallocation Tests.\n");
	for (int32_t i = 0; i < AllocationCount; i++){
		m_Allocations32t[i] = Allocator.Allocate<int32_t>(i);
		if (!m_Allocations32t[i]){
			fmt::print("Error allocating: {}\n", i);
			return false;
		}
		if (LWAllocator::GetAllocationSize(m_Allocations32t[i]) != sizeof(int32_t) * i) {
			fmt::print("Error allocating: {} Got: {} Expected: {}\n", i, LWAllocator::GetAllocationSize(m_Allocations32t[i]), sizeof(int32_t) * i);
			return false;
		}
		for (int32_t d = 0; d < i; d++) m_Allocations32t[i][d] = d;
	}
	for (int32_t i = 0; i < AllocationCount; i+=2){
		for (int32_t d = 0; d < i; d++) if (m_Allocations32t[i][d] != d) return false;
		LWAllocator::Destroy(m_Allocations32t[i]);
	}
	for (int32_t i = 1; i < AllocationCount; i += 2){
		for (int32_t d = 0; d < i; d++) if (m_Allocations32t[i][d] != d) return false;
		LWAllocator::Destroy(m_Allocations32t[i]);
	}
	fmt::print("Performing small non-POD allocation Tests.\n");
	for (int32_t i = 0; i < AllocationCount; i++){
		m_AllocationsVec4t[i] = Allocator.Allocate<LWVector4i>(SmallCount);
		if (!m_AllocationsVec4t[i]){
			fmt::print("Error allocating: {}\n", i);
			return false;
		}
		if (LWAllocator::GetAllocationSize(m_AllocationsVec4t[i]) != sizeof(LWVector4i) * SmallCount) {
			fmt::print("Error allocating: {} Got: {} Expected: {}\n", i, LWAllocator::GetAllocationSize(m_AllocationsVec4t[i]), sizeof(LWVector4i) * SmallCount);
			return false;
		}
		for (int32_t d = 0; d < SmallCount; d++) m_AllocationsVec4t[i][d] = LWVector4i(d);
	}
	for (int32_t i = 0; i < AllocationCount; i++){
		for (int32_t d = 0; d < SmallCount; d++) if (m_AllocationsVec4t[i][d].x != d || m_AllocationsVec4t[i][d].y != d || m_AllocationsVec4t[i][d].z != d || m_AllocationsVec4t[i][d].w != d) return false;
		LWAllocator::Destroy(m_AllocationsVec4t[i]);
	}
	fmt::print("Performing large non-POD allocation Tests.\n");
	for (int32_t i = 0; i < AllocationCount; i++){
		m_AllocationsVec4t[i] = Allocator.Allocate<LWVector4i>(LargeCount);
		if (!m_AllocationsVec4t[i]){
			fmt::print("Error allocating: {}\n", i);
			return false;
		}
		if (LWAllocator::GetAllocationSize(m_AllocationsVec4t[i]) != sizeof(LWVector4i) * LargeCount) {
			fmt::print("Error allocating: {} Got: {} Expected: {}\n", i, LWAllocator::GetAllocationSize(m_AllocationsVec4t[i]), sizeof(LWVector4i) * LargeCount);
			return false;
		}
		for (int32_t d = 0; d < LargeCount; d++) m_AllocationsVec4t[i][d] = LWVector4i(d);
	}
	for (int32_t i = 0; i < AllocationCount; i++){
		for (int32_t d = 0; d < LargeCount; d++) if (m_AllocationsVec4t[i][d].x != d || m_AllocationsVec4t[i][d].y != d || m_AllocationsVec4t[i][d].z != d || m_AllocationsVec4t[i][d].w != d) return false;
		LWAllocator::Destroy(m_AllocationsVec4t[i]);
	}
	fmt::print("Performing mixed non-POD allocation/deallocation Tests.\n");
	for (int32_t i = 0; i < AllocationCount; i++){
		m_AllocationsVec4t[i] = Allocator.Allocate<LWVector4i>(i);
		if (!m_AllocationsVec4t[i]){
			fmt::print("Error allocating: {}", i);
			return false;
		}
		if (LWAllocator::GetAllocationSize(m_AllocationsVec4t[i]) != sizeof(LWVector4i) * i) {
			fmt::print("Error allocating: {} Got: {} Expected: {}\n", i, LWAllocator::GetAllocationSize(m_AllocationsVec4t[i]), sizeof(LWVector4i) * i);
			return false;
		}
		for (int32_t d = 0; d < i; d++) m_AllocationsVec4t[i][d] = LWVector4i(d);
	}
	for (int32_t i = 0; i < AllocationCount; i+=2){
		for (int32_t d = 0; d < i; d++) if (m_AllocationsVec4t[i][d].x != d || m_AllocationsVec4t[i][d].y != d || m_AllocationsVec4t[i][d].z != d || m_AllocationsVec4t[i][d].w != d) return false;
		LWAllocator::Destroy(m_AllocationsVec4t[i]);
	}
	for (int32_t i = 1; i < AllocationCount; i += 2){
		for (int32_t d = 0; d < i; d++) if (m_AllocationsVec4t[i][d].x != d || m_AllocationsVec4t[i][d].y != d || m_AllocationsVec4t[i][d].z != d || m_AllocationsVec4t[i][d].w != d) return false;
		LWAllocator::Destroy(m_AllocationsVec4t[i]);
	}
	fmt::print("Checking allocated Bytes: {}\n", Allocator.GetAllocatedBytes());
	if (Allocator.GetAllocatedBytes() != 0) return false;
	auto Elapsed = std::chrono::steady_clock::now()-Start;
	fmt::print("Finished allocation tests, time take: {}\n", std::chrono::duration_cast<std::chrono::milliseconds>(Elapsed));
	return true;
}

bool PerformLWAllocatorTest(void){
	fmt::print("Performing LWAllocator test:\n");
	const uint32_t AllocSize = 1024 * 1024 * 64;
	LWAllocator_Default Default;
	LWAllocator_DefaultDebug DefaultDebug;
	LWAllocator_LocalCircular Circular(AllocSize);
	LWAllocator_LocalHeap Heap(AllocSize);
	LWAllocator_ConcurrentCircular ConCircular(AllocSize);
	if (!PerformAllocatorTest("LWAllocator_LocalCircular", Circular)) return false;
	if (!PerformAllocatorTest("LWAllocator_ConcurrentCircular", ConCircular)) return false;
	if (!PerformAllocatorTest("LWAllocator_LocalHeap", Heap)) return false;
	if (!PerformAllocatorTest("LWAllocator_Default", Default)) return false;
	if (!PerformAllocatorTest("LWAllocator_DefaultDebug", DefaultDebug)) return false;
	return true;
}

bool PerformLWSVectorTest(void) {
	fmt::print("Testing LWSVector4f:\n");
	auto SumValuesf = [](const LWSVector4f &t) {
		LWVector4f V = t.AsVec4();
		return V.x + V.y + V.z + V.w;
	};

	auto SumValues3f = [](const LWSVector4f &t) {
		LWVector4f V = t.AsVec4();
		return V.x + V.y + V.z;
	};

	auto SumValues2f = [](const LWSVector4f &t) {
		LWVector4f V = t.AsVec4();
		return V.x + V.y;
	};

	LWSVector4f Af(1.0f, 2.0f, 3.0f, 4.0f);
	LWSVector4f Bf(5.0f, 6.0f, 7.0f, 8.0f);
	LWSVector4f Testf(1.0f);
	if (!PerformTest("LWSVector4f<Construct>", std::bind(SumValuesf, Testf), 4.0f)) return false;
	Testf += LWSVector4f(2.0f, 3.0f, 4.0f, 5.0f);
	if (!PerformTest("LWSVector4f<Add>", std::bind(SumValuesf, Testf), 18.0f)) return false;
	Testf += 1.0f;
	if (!PerformTest("LWSVector4f<Add>", std::bind(SumValuesf, Testf), 22.0f)) return false;
	Testf = Testf + 2.0f;
	if (!PerformTest("LWSVector4f<Add>", std::bind(SumValuesf, Testf), 30.0f)) return false;
	Testf = 2.0f + Testf;
	if (!PerformTest("LWSVector4f<Add>", std::bind(SumValuesf, Testf), 38.0f)) return false;
	Testf -= 1.0f;
	if (!PerformTest("LWSVector4f<Sub>", std::bind(SumValuesf, Testf), 34.0f)) return false;
	Testf = Testf - 2.0f;
	if (!PerformTest("LWSVector4f<Sub>", std::bind(SumValuesf, Testf), 26.0f)) return false;
	Testf = 2.0f - Testf; //-3 -4 -5 -6
	if (!PerformTest("LWSVector4f<Sub>", std::bind(SumValuesf, Testf), -18.0f)) return false;
	Testf = LWSVector4f(1.0f)*2.0f;
	if (!PerformTest("LWSVector4f<Mul>", std::bind(SumValuesf, Testf), 8.0f)) return false;
	Testf *= 2.0f;
	if (!PerformTest("LWSVector4f<Mul>", std::bind(SumValuesf, Testf), 16.0f)) return false;
	Testf = 2.0f*LWSVector4f(1.0f);
	if (!PerformTest("LWSVector4f<Mul>", std::bind(SumValuesf, Testf), 8.0f)) return false;
	Testf = LWSVector4f(2.0f) / 2.0f;
	if (!PerformTest("LWSVector4f<Div>", std::bind(SumValuesf, Testf), 4.0f)) return false;
	Testf /= 2.0f;
	if (!PerformTest("LWSVector4f<Div>", std::bind(SumValuesf, Testf), 2.0f)) return false;
	Testf = 2.0f / LWSVector4f(4.0f);
	if (!PerformTest("LWSVector4f<Div>", std::bind(SumValuesf, Testf), 2.0f)) return false;
	if (!PerformTest("LWSVector4f<==>", []()->bool {return LWVector4f(1.0f) == LWVector4f(1.0f); }, true)) return false;
	if (!PerformTest("LWSVector4f<!=>", []()->bool {return LWVector4f(2.0f) != LWVector4f(1.0f); }, true)) return false;
	LWSVector4f Test4f = LWSVector4f(0.0f, 0.0f, 1.0f, 1.0f).Normalize();
	LWSVector4f Test3f = LWSVector4f(0.0f, 0.0f, 1.0f, 1.0f).Normalize3();
	LWSVector4f Test2f = LWSVector4f(0.0f, 0.0f, 1.0f, 1.0f).Normalize2();
	LWSVector4f SValuesf = LWSVector4f(1.0f, 2.0f, 3.0f, 4.0f);
	if (!PerformTest("LWSVector4<Sum>", std::bind(SumValuesf, SValuesf.Sum()), 40.0f)) return false;
	if (!PerformTest("LWSVector4<Sum4>", std::bind(&LWSVector4f::Sum4, SValuesf), 10.0f)) return false;
	if (!PerformTest("LWSVector4<Sum3>", std::bind(&LWSVector4f::Sum3, SValuesf), 6.0f)) return false;
	if (!PerformTest("LWSVector4<Sum2>", std::bind(&LWSVector4f::Sum2, SValuesf), 3.0f)) return false;
	if (!PerformTest("LWSVector4f<Normalize>", std::bind(SumValuesf, Test4f), 1.41421356f)) return false;
	if (!PerformTest("LWSVector4f<Normalize3>", std::bind(SumValues3f, Test3f), 1.0f)) return false;
	if (!PerformTest("LWSVector4f<Normalize2>", std::bind(SumValues2f, Test2f), 0.0f)) return false;
	if (!PerformTest("LWSVector4f<Length>", std::bind(&LWSVector4f::Length, &Test4f), 1.0f)) return false;
	if (!PerformTest("LWSVector4f<Length3>", std::bind(&LWSVector4f::Length3, &Test3f), 1.0f)) return false;
	if (!PerformTest("LWSVector4f<Length2>", std::bind(&LWSVector4f::Length2, &Test2f), 0.0f)) return false;
	if (!PerformTest("LWSVector4f<LengthSq>", std::bind(&LWSVector4f::LengthSquared, &Test4f), 1.0f)) return false;
	if (!PerformTest("LWSVector4f<LengthSq3>", std::bind(&LWSVector4f::LengthSquared3, &Test3f), 1.0f)) return false;
	if (!PerformTest("LWSVector4f<LengthSq2>", std::bind(&LWSVector4f::LengthSquared2, &Test2f), 0.0f)) return false;
	Test4f = LWSVector4f(0.0f, 0.0f, 0.0f, 1.0f);
	Test3f = LWSVector4f(0.0f, 0.0f, 1.0f, 1.0f);
	Test2f = LWSVector4f(0.0f, 0.0f, 1.0f, 1.0f);
	if (!PerformTest("LWSVector4f<Distance>", std::bind(&LWSVector4f::Distance, &Test4f, LWSVector4f(0.0f, 0.0f, 0.0f, 2.0f)), 1.0f)) return false;
	if (!PerformTest("LWSVector4f<Distance3>", std::bind(&LWSVector4f::Distance3, &Test3f, LWSVector4f(0.0f, 0.0f, 2.0f, 2.0f)), 1.0f)) return false;
	if (!PerformTest("LWSVector4f<Distance2>", std::bind(&LWSVector4f::Distance2, &Test2f, LWSVector4f(0.0f, 1.0f, 2.0f, 2.0f)), 1.0f)) return false;
	Test4f = LWSVector4f(0.0f, 0.0f, 0.0f, 2.0f);
	Test3f = LWSVector4f(0.0f, 0.0f, 2.0f, 2.0f);
	Test2f = LWSVector4f(0.0f, 0.0f, 2.0f, 2.0f);
	if (!PerformTest("LWSVector4f<DistanceSq>", std::bind(&LWSVector4f::DistanceSquared, &Test4f, LWSVector4f(0.0f, 0.0f, 0.0f, 4.0f)), 4.0f)) return false;
	if (!PerformTest("LWSVector4f<DistanceSq3>", std::bind(&LWSVector4f::DistanceSquared3, &Test3f, LWSVector4f(0.0f, 0.0f, 4.0f, 4.0f)), 4.0f)) return false;
	if (!PerformTest("LWSVector4f<DistanceSq2>", std::bind(&LWSVector4f::DistanceSquared2, &Test2f, LWSVector4f(0.0f, 0.0f, 4.0f, 4.0f)), 0.0f)) return false;
	Test4f = LWSVector4f(0.0f, 0.0f, 0.0f, 1.0f);
	Test3f = LWSVector4f(0.0f, 0.0f, 1.0f, 1.0f);
	Test2f = LWSVector4f(0.0f, 0.0f, 1.0f, 1.0f);
	if (!PerformTest("LWSVector4f<Dot>", std::bind(&LWSVector4f::Dot, &Test4f, LWSVector4f(0.0f, 0.0f, 1.0f, 0.0f)), 0.0f)) return false;
	if (!PerformTest("LWSVector4f<Dot3>", std::bind(&LWSVector4f::Dot3, &Test3f, LWSVector4f(0.0f, 0.0f, 1.0f, 0.0f)), 1.0f)) return false;
	if (!PerformTest("LWSVector4f<Dot2>", std::bind(&LWSVector4f::Dot2, &Test2f, LWSVector4f(0.0f, 1.0f, 0.0f, 0.0f)), 0.0f)) return false;

	if (!TestEquality("LWSVector4f<AAAB>", Af.AAAB(Bf), LWSVector4f(1.0f, 2.0f, 3.0f, 8.0f))) return false;
	if (!TestEquality("LWSVector4f<AABA>", Af.AABA(Bf), LWSVector4f(1.0f, 2.0f, 7.0f, 4.0f))) return false;
	if (!TestEquality("LWSVector4f<AABB>", Af.AABB(Bf), LWSVector4f(1.0f, 2.0f, 7.0f, 8.0f))) return false;
	if (!TestEquality("LWSVector4f<ABAA>", Af.ABAA(Bf), LWSVector4f(1.0f, 6.0f, 3.0f, 4.0f))) return false;
	if (!TestEquality("LWSVector4f<ABAB>", Af.ABAB(Bf), LWSVector4f(1.0f, 6.0f, 3.0f, 8.0f))) return false;
	if (!TestEquality("LWSVector4f<ABBA>", Af.ABBA(Bf), LWSVector4f(1.0f, 6.0f, 7.0f, 4.0f))) return false;
	if (!TestEquality("LWSVector4f<ABBB>", Af.ABBB(Bf), LWSVector4f(1.0f, 6.0f, 7.0f, 8.0f))) return false;
	if (!TestEquality("LWSVector4f<BAAA>", Af.BAAA(Bf), LWSVector4f(5.0f, 2.0f, 3.0f, 4.0f))) return false;
	if (!TestEquality("LWSVector4f<BAAB>", Af.BAAB(Bf), LWSVector4f(5.0f, 2.0f, 3.0f, 8.0f))) return false;
	if (!TestEquality("LWSVector4f<BABA>", Af.BABA(Bf), LWSVector4f(5.0f, 2.0f, 7.0f, 4.0f))) return false;
	if (!TestEquality("LWSVector4f<BABB>", Af.BABB(Bf), LWSVector4f(5.0f, 2.0f, 7.0f, 8.0f))) return false;
	if (!TestEquality("LWSVector4f<BBAA>", Af.BBAA(Bf), LWSVector4f(5.0f, 6.0f, 3.0f, 4.0f))) return false;
	if (!TestEquality("LWSVector4f<BBAB>", Af.BBAB(Bf), LWSVector4f(5.0f, 6.0f, 3.0f, 8.0f))) return false;
	if (!TestEquality("LWSVector4f<BBBA>", Af.BBBA(Bf), LWSVector4f(5.0f, 6.0f, 7.0f, 4.0f))) return false;

	//Test doubles
	fmt::print("Testing LWSVector4d:\n");

	auto SumValuesd = [](const LWSVector4d &t) {
		LWVector4d V = t.AsVec4();
		return V.x + V.y + V.z + V.w;
	};

	auto SumValues3d = [](const LWSVector4d &t) {
		LWVector4d V = t.AsVec4();
		return V.x + V.y + V.z;
	};

	auto SumValues2d = [](const LWSVector4d &t) {
		LWVector4d V = t.AsVec4();
		return V.x + V.y;
	};

	LWSVector4d Ad(1.0, 2.0, 3.0, 4.0);
	LWSVector4d Bd(5.0, 6.0, 7.0, 8.0);
	LWSVector4d Testd(1.0);
	if (!PerformTest("LWSVector4d<Construct>", std::bind(SumValuesd, Testd), 4.0)) return false;
	Testd += LWSVector4d(2.0, 3.0, 4.0, 5.0);
	if (!PerformTest("LWSVector4d<Add>", std::bind(SumValuesd, Testd), 18.0)) return false;
	Testd += 1.0;
	if (!PerformTest("LWSVector4d<Add>", std::bind(SumValuesd, Testd), 22.0)) return false;
	Testd = Testd + 2.0;
	if (!PerformTest("LWSVector4d<Add>", std::bind(SumValuesd, Testd), 30.0)) return false;
	Testd = 2.0 + Testd;
	if (!PerformTest("LWSVector4d<Add>", std::bind(SumValuesd, Testd), 38.0)) return false;
	Testd -= 1.0;
	if (!PerformTest("LWSVector4d<Sub>", std::bind(SumValuesd, Testd), 34.0)) return false;
	Testd = Testd - 2.0;
	if (!PerformTest("LWSVector4d<Sub>", std::bind(SumValuesd, Testd), 26.0)) return false;
	Testd = 2.0 - Testd; //-3 -4 -5 -6
	if (!PerformTest("LWSVector4d<Sub>", std::bind(SumValuesd, Testd), -18.0)) return false;
	Testd = LWSVector4d(1.0)*2.0;
	if (!PerformTest("LWSVector4d<Mul>", std::bind(SumValuesd, Testd), 8.0)) return false;
	Testd *= 2.0;
	if (!PerformTest("LWSVector4d<Mul>", std::bind(SumValuesd, Testd), 16.0)) return false;
	Testd = 2.0*LWSVector4d(1.0);
	if (!PerformTest("LWSVector4d<Mul>", std::bind(SumValuesd, Testd), 8.0)) return false;
	Testd = LWSVector4d(2.0) / 2.0;
	if (!PerformTest("LWSVector4d<Div>", std::bind(SumValuesd, Testd), 4.0)) return false;
	Testd /= 2.0;
	if (!PerformTest("LWSVector4d<Div>", std::bind(SumValuesd, Testd), 2.0)) return false;
	Testd = 2.0 / LWSVector4d(4.0);
	if (!PerformTest("LWSVector4d<Div>", std::bind(SumValuesd, Testd), 2.0)) return false;
	if (!PerformTest("LWSVector4d<==>", []()->bool {return LWVector4d(1.0) == LWVector4d(1.0); }, true)) return false;
	if (!PerformTest("LWSVector4d<!=>", []()->bool {return LWVector4d(2.0) != LWVector4d(1.0); }, true)) return false;
	LWSVector4d Test4d = LWSVector4d(0.0, 0.0, 1.0, 1.0).Normalize();
	LWSVector4d Test3d = LWSVector4d(0.0, 0.0, 1.0, 1.0).Normalize3();
	LWSVector4d Test2d = LWSVector4d(0.0, 0.0, 1.0, 1.0).Normalize2();
	LWSVector4d SValuesd = LWSVector4d(1.0, 2.0, 3.0, 4.0);
	if (!PerformTest("LWSVector4d<Sum>", std::bind(SumValuesd, SValuesd.Sum()), 40.0)) return false;
	if (!PerformTest("LWSVector4d<Sum4>", std::bind(&LWSVector4d::Sum4, SValuesd), 10.0)) return false;
	if (!PerformTest("LWSVector4d<Sum3>", std::bind(&LWSVector4d::Sum3, SValuesd), 6.0)) return false;
	if (!PerformTest("LWSVector4d<Sum2>", std::bind(&LWSVector4d::Sum2, SValuesd), 3.0)) return false;
	if (!PerformTest("LWSVector4d<Normalize>", std::bind(SumValuesd, Test4d), 1.4142135623730949)) return false;
	if (!PerformTest("LWSVector4d<Normalize3>", std::bind(SumValues3d, Test3d), 1.0)) return false;
	if (!PerformTest("LWSVector4d<Normalize2>", std::bind(SumValues2d, Test2d), 0.0)) return false;
	if (!PerformTest("LWSVector4d<Length>", std::bind(&LWSVector4d::Length, &Test4d), 1.0)) return false;
	if (!PerformTest("LWSVector4d<Length3>", std::bind(&LWSVector4d::Length3, &Test3d), 1.0)) return false;
	if (!PerformTest("LWSVector4d<Length2>", std::bind(&LWSVector4d::Length2, &Test2d), 0.0)) return false;
	if (!PerformTest("LWSVector4d<LengthSq>", std::bind(&LWSVector4d::LengthSquared, &Test4d), 1.0)) return false;
	if (!PerformTest("LWSVector4d<LengthSq3>", std::bind(&LWSVector4d::LengthSquared3, &Test3d), 1.0)) return false;
	if (!PerformTest("LWSVector4d<LengthSq2>", std::bind(&LWSVector4d::LengthSquared2, &Test2d), 0.0)) return false;
	Test4d = LWSVector4d(0.0, 0.0, 0.0, 1.0);
	Test3d = LWSVector4d(0.0, 0.0, 1.0, 1.0);
	Test2d = LWSVector4d(0.0, 0.0, 1.0, 1.0);
	if (!PerformTest("LWSVector4d<Distance>", std::bind(&LWSVector4d::Distance, &Test4d, LWSVector4d(0.0, 0.0, 0.0, 2.0)), 1.0)) return false;
	if (!PerformTest("LWSVector4d<Distance3>", std::bind(&LWSVector4d::Distance3, &Test3d, LWSVector4d(0.0, 0.0, 2.0, 2.0)), 1.0)) return false;
	if (!PerformTest("LWSVector4d<Distance2>", std::bind(&LWSVector4d::Distance2, &Test2d, LWSVector4d(0.0, 1.0, 2.0, 2.0)), 1.0)) return false;
	Test4d = LWSVector4d(0.0, 0.0, 0.0, 2.0);
	Test3d = LWSVector4d(0.0, 0.0, 2.0, 2.0);
	Test2d = LWSVector4d(0.0, 0.0, 2.0, 2.0);
	if (!PerformTest("LWSVector4d<DistanceSq>", std::bind(&LWSVector4d::DistanceSquared, &Test4d, LWSVector4d(0.0, 0.0, 0.0, 4.0)), 4.0)) return false;
	if (!PerformTest("LWSVector4d<DistanceSq3>", std::bind(&LWSVector4d::DistanceSquared3, &Test3d, LWSVector4d(0.0, 0.0, 4.0, 4.0)), 4.0)) return false;
	if (!PerformTest("LWSVector4d<DistanceSq2>", std::bind(&LWSVector4d::DistanceSquared2, &Test2d, LWSVector4d(0.0, 0.0, 4.0, 4.0)), 0.0)) return false;
	Test4d = LWSVector4d(0.0, 0.0, 0.0, 1.0);
	Test3d = LWSVector4d(0.0, 0.0, 1.0, 1.0);
	Test2d = LWSVector4d(0.0, 0.0, 1.0, 1.0);
	if (!PerformTest("LWSVector4d<Dot>", std::bind(&LWSVector4d::Dot, &Test4d, LWSVector4d(0.0, 0.0, 1.0, 0.0)), 0.0)) return false;
	if (!PerformTest("LWSVector4d<Dot3>", std::bind(&LWSVector4d::Dot3, &Test3d, LWSVector4d(0.0, 0.0, 1.0, 0.0)), 1.0)) return false;
	if (!PerformTest("LWSVector4d<Dot2>", std::bind(&LWSVector4d::Dot2, &Test2d, LWSVector4d(0.0, 1.0, 0.0, 0.0)), 0.0)) return false;
	if (!TestEquality("LWSVector4d<AAAB>", Ad.AAAB(Bd), LWSVector4d(1.0, 2.0, 3.0, 8.0))) return false;
	if (!TestEquality("LWSVector4d<AABA>", Ad.AABA(Bd), LWSVector4d(1.0, 2.0, 7.0, 4.0))) return false;
	if (!TestEquality("LWSVector4d<AABB>", Ad.AABB(Bd), LWSVector4d(1.0, 2.0, 7.0, 8.0))) return false;
	if (!TestEquality("LWSVector4d<ABAA>", Ad.ABAA(Bd), LWSVector4d(1.0, 6.0, 3.0, 4.0))) return false;
	if (!TestEquality("LWSVector4d<ABAB>", Ad.ABAB(Bd), LWSVector4d(1.0, 6.0, 3.0, 8.0))) return false;
	if (!TestEquality("LWSVector4d<ABBA>", Ad.ABBA(Bd), LWSVector4d(1.0, 6.0, 7.0, 4.0))) return false;
	if (!TestEquality("LWSVector4d<ABBB>", Ad.ABBB(Bd), LWSVector4d(1.0, 6.0, 7.0, 8.0))) return false;
	if (!TestEquality("LWSVector4d<BAAA>", Ad.BAAA(Bd), LWSVector4d(5.0, 2.0, 3.0, 4.0))) return false;
	if (!TestEquality("LWSVector4d<BAAB>", Ad.BAAB(Bd), LWSVector4d(5.0, 2.0, 3.0, 8.0))) return false;
	if (!TestEquality("LWSVector4d<BABA>", Ad.BABA(Bd), LWSVector4d(5.0, 2.0, 7.0, 4.0))) return false;
	if (!TestEquality("LWSVector4d<BABB>", Ad.BABB(Bd), LWSVector4d(5.0, 2.0, 7.0, 8.0))) return false;
	if (!TestEquality("LWSVector4d<BBAA>", Ad.BBAA(Bd), LWSVector4d(5.0, 6.0, 3.0, 4.0))) return false;
	if (!TestEquality("LWSVector4d<BBAB>", Ad.BBAB(Bd), LWSVector4d(5.0, 6.0, 3.0, 8.0))) return false;
	if (!TestEquality("LWSVector4d<BBBA>", Ad.BBBA(Bd), LWSVector4d(5.0, 6.0, 7.0, 4.0))) return false;


	//Test int32_t
	fmt::print("Testing LWSVector4i:\n");

	auto SumValuesi = [](const LWSVector4i &t) {
		LWVector4i V = t.AsVec4();
		return V.x + V.y + V.z + V.w;
	};
	
	LWSVector4i Ai(1, 2, 3, 4);
	LWSVector4i Bi(5, 6, 7, 8);

	LWSVector4i Testi(1);
	if (!PerformTest("LWSVector4i<Construct>", std::bind(SumValuesi, Testi), 4.0)) return false;
	Testi += LWSVector4i(2, 3, 4, 5);
	if (!PerformTest("LWSVector4i<Add>", std::bind(SumValuesi, Testi), 18)) return false;
	Testi += 1;
	if (!PerformTest("LWSVector4i<Add>", std::bind(SumValuesi, Testi), 22)) return false;
	Testi = Testi + 2;
	if (!PerformTest("LWSVector4i<Add>", std::bind(SumValuesi, Testi), 30)) return false;
	Testi = 2 + Testi;
	if (!PerformTest("LWSVector4i<Add>", std::bind(SumValuesi, Testi), 38)) return false;
	Testi -= 1;
	if (!PerformTest("LWSVector4i<Sub>", std::bind(SumValuesi, Testi), 34)) return false;
	Testi = Testi - 2;
	if (!PerformTest("LWSVector4i<Sub>", std::bind(SumValuesi, Testi), 26)) return false;
	Testi = 2 - Testi; //-3 -4 -5 -6
	if (!PerformTest("LWSVector4i<Sub>", std::bind(SumValuesi, Testi), -18)) return false;
	Testi = LWSVector4i(1)*2;
	if (!PerformTest("LWSVector4i<Mul>", std::bind(SumValuesi, Testi), 8)) return false;
	Testi *= 2;
	if (!PerformTest("LWSVector4i<Mul>", std::bind(SumValuesi, Testi), 16)) return false;
	Testi = 2*LWSVector4i(1);
	if (!PerformTest("LWSVector4i<Mul>", std::bind(SumValuesi, Testi), 8)) return false;
	Testi = LWSVector4i(2) / 2;
	if (!PerformTest("LWSVector4i<Div>", std::bind(SumValuesi, Testi), 4)) return false;
	Testi /= 2;
	if (!PerformTest("LWSVector4i<Div>", std::bind(SumValuesi, Testi), 0)) return false;
	Testi = 2 / LWSVector4i(4);
	if (!PerformTest("LWSVector4i<Div>", std::bind(SumValuesi, Testi), 0)) return false;
	if (!PerformTest("LWSVector4i<==>", []()->bool {return LWVector4i(1) == LWVector4i(1); }, true)) return false;
	if (!PerformTest("LWSVector4i<!=>", []()->bool {return LWVector4i(2) != LWVector4i(1); }, true)) return false;
	LWSVector4i SValuesi = LWSVector4i(1, 2, 3, 4);
	if (!PerformTest("LWSVector4i<Sum>", std::bind(SumValuesi, SValuesi.Sum()), 40)) return false;
	if (!PerformTest("LWSVector4i<Sum4>", std::bind(&LWSVector4i::Sum4, SValuesi), 10)) return false;
	if (!PerformTest("LWSVector4i<Sum3>", std::bind(&LWSVector4i::Sum3, SValuesi), 6)) return false;
	if (!PerformTest("LWSVector4i<Sum2>", std::bind(&LWSVector4i::Sum2, SValuesi), 3)) return false;
	if (!TestEquality("LWSVector4i<AAAB>", Ai.AAAB(Bi), LWSVector4i(1, 2, 3, 8))) return false;
	if (!TestEquality("LWSVector4i<AABA>", Ai.AABA(Bi), LWSVector4i(1, 2, 7, 4))) return false;
	if (!TestEquality("LWSVector4i<AABB>", Ai.AABB(Bi), LWSVector4i(1, 2, 7, 8))) return false;
	if (!TestEquality("LWSVector4i<ABAA>", Ai.ABAA(Bi), LWSVector4i(1, 6, 3, 4))) return false;
	if (!TestEquality("LWSVector4i<ABAB>", Ai.ABAB(Bi), LWSVector4i(1, 6, 3, 8))) return false;
	if (!TestEquality("LWSVector4i<ABBA>", Ai.ABBA(Bi), LWSVector4i(1, 6, 7, 4))) return false;
	if (!TestEquality("LWSVector4i<ABBB>", Ai.ABBB(Bi), LWSVector4i(1, 6, 7, 8))) return false;
	if (!TestEquality("LWSVector4i<BAAA>", Ai.BAAA(Bi), LWSVector4i(5, 2, 3, 4))) return false;
	if (!TestEquality("LWSVector4i<BAAB>", Ai.BAAB(Bi), LWSVector4i(5, 2, 3, 8))) return false;
	if (!TestEquality("LWSVector4i<BABA>", Ai.BABA(Bi), LWSVector4i(5, 2, 7, 4))) return false;
	if (!TestEquality("LWSVector4i<BABB>", Ai.BABB(Bi), LWSVector4i(5, 2, 7, 8))) return false;
	if (!TestEquality("LWSVector4i<BBAA>", Ai.BBAA(Bi), LWSVector4i(5, 6, 3, 4))) return false;
	if (!TestEquality("LWSVector4i<BBAB>", Ai.BBAB(Bi), LWSVector4i(5, 6, 3, 8))) return false;
	if (!TestEquality("LWSVector4i<BBBA>", Ai.BBBA(Bi), LWSVector4i(5, 6, 7, 4))) return false;

	return true;
};

bool PerformLWVectorTest(void){
	fmt::print("Testing LWVector4:\n");

	auto SumValues = [](const LWVector4f &t){ return t.x + t.y + t.z + t.w;  };
	auto SumValues3 = [](const LWVector3f &t){ return t.x + t.y + t.z; };
	auto SumValues2 = [](const LWVector2f &t){ return t.x + t.y; };

	LWVector4f Test(1.0f);
	if (!PerformTest("LWVector4<Construct>", std::bind(SumValues, Test), 4.0f)) return false;
	Test += LWVector4f(2.0f, 3.0f, 4.0f, 5.0f);
	if (!PerformTest("LWVector4<Add>", std::bind(SumValues, Test), 18.0f)) return false;
	Test += 1.0f;
	if (!PerformTest("LWVector4<Add>", std::bind(SumValues, Test), 22.0f)) return false;
	Test = Test + 2.0f;
	if (!PerformTest("LWVector4<Add>", std::bind(SumValues, Test), 30.0f)) return false;
	Test = 2.0f + Test;
	if (!PerformTest("LWVector4<Add>", std::bind(SumValues, Test), 38.0f)) return false;
	Test -= 1.0f;
	if (!PerformTest("LWVector4<Sub>", std::bind(SumValues, Test), 34.0f)) return false;
	Test = Test - 2.0f;
	if (!PerformTest("LWVector4<Sub>", std::bind(SumValues, Test), 26.0f)) return false;
	Test = 2.0f - Test; //-3 -4 -5 -6
	if (!PerformTest("LWVector4<Sub>", std::bind(SumValues, Test), -18.0f)) return false;
	Test = LWVector4f(1.0f)*2.0f;
	if (!PerformTest("LWVector4<Mul>", std::bind(SumValues, Test), 8.0f)) return false;
	Test *= 2.0f;
	if (!PerformTest("LWVector4<Mul>", std::bind(SumValues, Test), 16.0f)) return false;
	Test = 2.0f*LWVector4f(1.0f);
	if (!PerformTest("LWVector4<Mul>", std::bind(SumValues, Test), 8.0f)) return false;
	Test = LWVector4f(2.0f) / 2.0f;
	if (!PerformTest("LWVector4<Div>", std::bind(SumValues, Test), 4.0f)) return false;
	Test /= 2.0f;
	if (!PerformTest("LWVector4<Div>", std::bind(SumValues, Test), 2.0f)) return false;
	Test = 2.0f / LWVector4f(4.0f);
	if (!PerformTest("LWVector4<Div>", std::bind(SumValues, Test), 2.0f)) return false;
	if (LWVector4f(1.0f) != LWVector4f(1.0f)) return false;
	if (LWVector4f(2.0f) == LWVector4f(1.0f)) return false;
	Test = LWVector4f(1.0f, 1.0f, 0.0f, 0.0f).Normalize();
	if (!PerformTest("LWVector4<Normalize>", std::bind(SumValues, Test), 1.41421356f)) return false;
	if (!PerformTest("LWVector4<Length>", std::bind(&LWVector4f::Length, &Test), 1.0f)) return false;
	if (!PerformTest("LWVector4<LengthSq>", std::bind(&LWVector4f::LengthSquared, &Test), 1.0f)) return false;
	Test = LWVector4f(1.0f, 0.0f, 0.0f, 0.0f);
	if (!PerformTest("LWVector4<Distance>", std::bind(&LWVector4f::Distance, &Test, LWVector4f(2.0f, 0.0f, 0.0f, 0.0f)), 1.0f)) return false;
	Test = LWVector4f(2.0f, 0.0f, 0.0f, 0.0f);
	if (!PerformTest("LWVector4<DistanceSq>" , std::bind(&LWVector4f::DistanceSquared, &Test, LWVector4f(4.0f, 0.0f, 0.0f, 0.0f)), 4.0f)) return false;
	Test = LWVector4f(1.0f, 0.0f, 0.0f, 0.0f);
	if (!PerformTest("LWVector4<Dot>", std::bind(&LWVector4f::Dot, &Test, LWVector4f(0.0f, 1.0f, 0.0f, 0.0f)), 0.0f)) return false;
	LWVector4i Testi = LWVector4f(2.0f, 3.0f, 4.0f, 5.0f).CastTo<int32_t>();
	if (!PerformTest("LWVector4<cast>", std::bind([](const LWVector4i &P){return P.x + P.y + P.z + P.w; }, Testi), 14)) return false;

	fmt::print("Testing LWVector3:\n");
	LWVector3f Test3(1.0f);
	if (!PerformTest("LWVector3<Construct>", std::bind(SumValues3, Test3), 3.0f)) return false;
	Test3 += LWVector3f(2.0f, 3.0f, 4.0f);
	if (!PerformTest("LWVector3<Add>", std::bind(SumValues3, Test3), 12.0f)) return false;
	Test3 += 1.0f;
	if (!PerformTest("LWVector3<Add>", std::bind(SumValues3, Test3), 15.0f)) return false;
	Test3 = Test3 + 2.0f;
	if (!PerformTest("LWVector3<Add>", std::bind(SumValues3, Test3), 21.0f)) return false;
	Test3 = 2.0f + Test3;
	if (!PerformTest("LWVector3<Add>", std::bind(SumValues3, Test3), 27.0f)) return false;
	Test3 -= 1.0f;
	if (!PerformTest("LWVector3<Sub>", std::bind(SumValues3, Test3), 24.0f)) return false;
	Test3 = Test3 - 2.0f;
	if (!PerformTest("LWVector3<Sub>", std::bind(SumValues3, Test3), 18.0f)) return false;
	Test3 = 2.0f - Test3; //-3 -4 -5
	if (!PerformTest("LWVector3<Sub>", std::bind(SumValues3, Test3), -12.0f)) return false;
	Test3 = LWVector3f(1.0f)*2.0f;
	if (!PerformTest("LWVector3<Mul>", std::bind(SumValues3, Test3), 6.0f)) return false;
	Test3 *= 2.0f;
	if (!PerformTest("LWVector3<Mul>", std::bind(SumValues3, Test3), 12.0f)) return false;
	Test3 = 2.0f*LWVector3f(1.0f);
	if (!PerformTest("LWVector3<Mul>", std::bind(SumValues3, Test3), 6.0f)) return false;
	Test3 = LWVector3f(2.0f) / 2.0f;
	if (!PerformTest("LWVector3<Div>", std::bind(SumValues3, Test3), 3.0f)) return false;
	Test3 /= 2.0f;
	if (!PerformTest("LWVector3<Div>", std::bind(SumValues3, Test3), 1.5f)) return false;
	Test3 = 2.0f / LWVector3f(4.0f);
	if (!PerformTest("LWVector3<Div>", std::bind(SumValues3, Test3), 1.5f)) return false;
	if (LWVector3f(1.0f) != LWVector3f(1.0f)) return false;
	if (LWVector3f(2.0f) == LWVector3f(1.0f)) return false;
	Test3 = LWVector3f(1.0f, 1.0f, 0.0f).Normalize();
	if (!PerformTest("LWVector3<Normalize>", std::bind(SumValues3, Test3), 1.41421356f)) return false;
	if (!PerformTest("LWVector3<Length>", std::bind(&LWVector3f::Length, &Test3), 1.0f)) return false;
	if (!PerformTest("LWVector3<LengthSq>", std::bind(&LWVector3f::LengthSquared, &Test3), 1.0f)) return false;
	Test3 = LWVector3f(1.0f, 0.0f, 0.0f);
	if (!PerformTest("LWVector3<Distance>", std::bind(&LWVector3f::Distance, &Test3, LWVector3f(2.0f, 0.0f, 0.0f)), 1.0f)) return false;
	Test3 = LWVector3f(2.0f, 0.0f, 0.0f);
	if (!PerformTest("LWVector3<DistanceSq>", std::bind(&LWVector3f::DistanceSquared, &Test3, LWVector3f(4.0f, 0.0f, 0.0f)), 4.0f)) return false;
	Test3 = LWVector3f(1.0f, 0.0f, 0.0f);
	if (!PerformTest("LWVector3<Dot>", std::bind(&LWVector3f::Dot, &Test3, LWVector3f(0.0f, 1.0f, 0.0f)), 0.0f)) return false;
	if (!PerformTest("LWVector3<Cross == 0,0,1>", std::bind([](const LWVector3f &P){ return P.x == 0.0f && P.y == 0.0f && P.z == 1.0f; }, LWVector3f(1.0f, 0.0f, 0.0f).Cross(LWVector3f(0.0f, 1.0f, 0.0f))), true)) return false;
	LWVector3i Test3i = LWVector3f(2.0f, 3.0f, 4.0f).CastTo<int32_t>();
	if (!PerformTest("LWVector3<cast>", std::bind([](const LWVector3i &P){return P.x + P.y + P.z; }, Test3i), 9)) return false;

	fmt::print("Testing LWVector2:\n");
	LWVector2f Test2(1.0f);
	if (!PerformTest("LWVector2<Construct>", std::bind(SumValues2, Test2), 2.0f)) return false;
	Test2 += LWVector2f(2.0f, 3.0f);
	if (!PerformTest("LWVector2<Add>", std::bind(SumValues2, Test2), 7.0f)) return false;
	Test2 += 1.0f;
	if (!PerformTest("LWVector2<Add>", std::bind(SumValues2, Test2), 9.0f)) return false;
	Test2 = Test2 + 2.0f;
	if (!PerformTest("LWVector2<Add>", std::bind(SumValues2, Test2), 13.0f)) return false;
	Test2 = 2.0f + Test2;
	if (!PerformTest("LWVector2<Add>", std::bind(SumValues2, Test2), 17.0f)) return false;
	Test2 -= 1.0f;
	if (!PerformTest("LWVector2<Sub>", std::bind(SumValues2, Test2), 15.0f)) return false;
	Test2 = Test2 - 2.0f;
	if (!PerformTest("LWVector2<Sub>", std::bind(SumValues2, Test2), 11.0f)) return false;
	Test2 = 2.0f - Test2; //-3 -4
	if (!PerformTest("LWVector2<Sub>", std::bind(SumValues2, Test2), -7.0f)) return false;
	Test2 = LWVector2f(1.0f)*2.0f;
	if (!PerformTest("LWVector2<Mul>", std::bind(SumValues2, Test2), 4.0f)) return false;
	Test2 *= 2.0f;
	if (!PerformTest("LWVector2<Mul>", std::bind(SumValues2, Test2), 8.0f)) return false;
	Test2 = 2.0f*LWVector2f(1.0f);
	if (!PerformTest("LWVector2<Mul>", std::bind(SumValues2, Test2), 4.0f)) return false;
	Test2 = LWVector2f(2.0f) / 2.0f;
	if (!PerformTest("LWVector2<Div>", std::bind(SumValues2, Test2), 2.0f)) return false;
	Test2 /= 2.0f;
	if (!PerformTest("LWVector2<Div>", std::bind(SumValues2, Test2), 1.0f)) return false;
	Test2 = 2.0f / LWVector2f(4.0f);
	if (!PerformTest("LWVector2<Div>", std::bind(SumValues2, Test2), 1.0f)) return false;
	if (LWVector2f(1.0f) != LWVector2f(1.0f)) return false;
	if (LWVector2f(2.0f) == LWVector2f(1.0f)) return false;
	Test2 = LWVector2f(1.0f, 1.0f).Normalize();
	if (!PerformTest("LWVector2<Normalize>", std::bind(SumValues2, Test2), 1.41421356f)) return false;
	if (!PerformTest("LWVector2<Length>", std::bind(&LWVector2f::Length, &Test2), 1.0f)) return false;
	if (!PerformTest("LWVector2<LengthSq>", std::bind(&LWVector2f::LengthSquared, &Test2), 1.0f)) return false;
	Test2 = LWVector2f(1.0f, 0.0f);
	if (!PerformTest("LWVector2<Distance>", std::bind(&LWVector2f::Distance, &Test2, LWVector2f(2.0f, 0.0f)), 1.0f)) return false;
	Test2 = LWVector2f(2.0f, 0.0f);
	if (!PerformTest("LWVector2<DistanceSq>", std::bind(&LWVector2f::DistanceSquared, &Test2, LWVector2f(4.0f, 0.0f)), 4.0f)) return false;
	Test2 = LWVector2f(1.0f, 0.0f);
	if (!PerformTest("LWVector2<Dot>", std::bind(&LWVector2f::Dot, &Test2, LWVector2f(0.0f, 1.0f)), 0.0f)) return false;
	Test2 = LWVector2f(1.0f, 1.0f).Perpindicular();
	if (!PerformTest("LWVector2<Perp>", std::bind(SumValues2, Test2), 0.0f)) return false;
	if (!PerformTest("LWVector2<Theta>", std::bind(&LWVector2f::Theta, LWVector2f(1.0f, 0.0f)), 0.0f)) return false;
	Test2 = LWVector2f::MakeTheta(LW_PI_2);
	if (!PerformTest("LWVector2<Make>", std::bind(&LWVector2f::Theta, &Test2), LW_PI_2)) return false;

	LWVector2i Test2i = LWVector2f(2.0f, 3.0f).CastTo<int32_t>();
	if (!PerformTest("LWVector2<cast>", std::bind([](const LWVector2i &P){return P.x + P.y; }, Test2i), 5)) return false;

	return true;
}

bool PerformLWByteStreamTest(void) {
	LWAllocator_Default DefAlloc;
	int16_t Values16[4] = { 0x1122, 0x3344, 0x4455, 0x5566 };
	int32_t Values32[4] = { 0x11223344, 0x22334455, 0x33445566, 0x44556677 };
	int64_t Values64[4] = { 0x1122334455667788, 0x2233445566778899, 0x33445566778899AA, 0x445566778899AABB };
	float ValuesF[4] = { LW_PI, LW_2PI, LW_PI_2, LW_PI_4 };
	double ValuesD[4] = { LW_PI, LW_2PI, LW_PI_2, LW_PI_4 };
	LWVector2d ValuesVec2d[4] = { LWVector2d(), LWVector2d(1.0), LWVector2d(2.0), LWVector2d(3.0) };
	LWVector2f ValuesVec2f[4] = { LWVector2f(), LWVector2f(1.0f), LWVector2f(2.0f), LWVector2f(3.0f) };
	LWVector2i ValuesVec2i[4] = { LWVector2i(), LWVector2i(1), LWVector2i(2), LWVector2i(3) };
	LWVector3d ValuesVec3d[4] = { LWVector3d(), LWVector3d(1.0), LWVector3d(2.0), LWVector3d(3.0) };
	LWVector3f ValuesVec3f[4] = { LWVector3f(), LWVector3f(1.0f), LWVector3f(2.0f), LWVector3f(3.0f) };
	LWVector3i ValuesVec3i[4] = { LWVector3i(), LWVector3i(1), LWVector3i(2), LWVector3i(3) };
	LWVector4d ValuesVec4d[4] = { LWVector4d(), LWVector4d(1.0), LWVector4d(2.0), LWVector4d(3.0) };
	LWVector4f ValuesVec4f[4] = { LWVector4f(), LWVector4f(1.0f), LWVector4f(2.0f), LWVector4f(3.0f) };
	LWVector4i ValuesVec4i[4] = { LWVector4i(), LWVector4i(1), LWVector4i(2), LWVector4i(3) };
	LWMatrix4d ValuesMat4d[4] = { LWMatrix4d(), LWMatrix4d(2.0, 2.0, 2.0, 2.0), LWMatrix4d(3.0, 3.0, 3.0, 3.0), LWMatrix4d(4.0, 4.0, 4.0, 4.0) };
	LWMatrix4f ValuesMat4f[4] = { LWMatrix4f(), LWMatrix4f(2.0f, 2.0f, 2.0f, 2.0f), LWMatrix4f(3.0f, 3.0f, 3.0f, 3.0f), LWMatrix4f(4.0f, 4.0f, 4.0f, 4.0f) };
	LWMatrix4i ValuesMat4i[4] = { LWMatrix4i(), LWMatrix4i(2, 2, 2, 2), LWMatrix4i(3, 3, 3, 3), LWMatrix4i(4, 4, 4, 4) };
	LWMatrix3d ValuesMat3d[4] = { LWMatrix3d(), LWMatrix3d(2.0, 2.0, 2.0), LWMatrix3d(3.0, 3.0, 3.0), LWMatrix3d(4.0, 4.0, 4.0) };
	LWMatrix3f ValuesMat3f[4] = { LWMatrix3f(), LWMatrix3f(2.0f, 2.0f, 2.0f), LWMatrix3f(3.0f, 3.0f, 3.0f), LWMatrix3f(4.0f, 4.0f, 4.0f) };
	LWMatrix3i ValuesMat3i[4] = { LWMatrix3i(), LWMatrix3i(2, 2, 2), LWMatrix3i(3, 3, 3) };
	LWMatrix2d ValuesMat2d[4] = { LWMatrix2d(), LWMatrix2d(2.0, 2.0), LWMatrix2d(3.0, 3.0), LWMatrix2d(4.0, 4.0) };
	LWMatrix2f ValuesMat2f[4] = { LWMatrix2f(), LWMatrix2f(2.0f, 2.0f), LWMatrix2f(3.0f, 3.0f), LWMatrix2f(4.0f, 4.0f) };
	LWMatrix2i ValuesMat2i[4] = { LWMatrix2i(), LWMatrix2i(2, 2), LWMatrix2i(3, 3), LWMatrix2i(4, 4) };
	LWQuaterniond ValueQuatd[4] = { LWQuaterniond(), LWQuaterniond::FromAxis(LW_PI, 0.0, 0.0, LW_PI), LWQuaterniond::FromAxis(0.0, LW_PI, 0.0, LW_PI), LWQuaterniond::FromAxis(0.0, 0.0, LW_PI, LW_PI) };
	LWQuaternionf ValueQuatf[4] = { LWQuaternionf(), LWQuaternionf::FromAxis(LW_PI, 0.0f, 0.0f, LW_PI), LWQuaternionf::FromAxis(0.0f, LW_PI, 0.0f, LW_PI), LWQuaternionf::FromAxis(0.0f, 0.0f, LW_PI, LW_PI) };
	char8_t ValuesText[] = u8"This is a test utf-8 string.";
	int16_t Result16[4] = { 0,0,0,0 };
	int32_t Result32[4] = { 0,0,0,0 };
	int64_t Result64[4] = { 0, 0, 0, 0 };
	float ResultF[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	double ResultD[4] = { 0.0, 0.0, 0.0, 0.0 };
	LWVector2d ResultVec2d[4];
	LWVector2f ResultVec2f[4];
	LWVector2i ResultVec2i[4];
	LWVector3d ResultVec3d[4];
	LWVector3f ResultVec3f[4];
	LWVector3i ResultVec3i[4];
	LWVector4d ResultVec4d[4];
	LWVector4f ResultVec4f[4];
	LWVector4i ResultVec4i[4];
	LWMatrix4d ResultMat4d[4];
	LWMatrix4f ResultMat4f[4];
	LWMatrix4i ResultMat4i[4];
	LWMatrix3d ResultMat3d[4];
	LWMatrix3f ResultMat3f[4];
	LWMatrix3i ResultMat3i[4];
	LWMatrix2d ResultMat2d[4];
	LWMatrix2f ResultMat2f[4];
	LWMatrix2i ResultMat2i[4];
	LWQuaterniond ResultQuatd[4];
	LWQuaternionf ResultQuatf[4];
	char ResultText[32];

	int8_t Buffer[4096];
	int8_t NetBuffer[4096];
	LWByteBuffer ByteBuf(Buffer, sizeof(Buffer), LWByteBuffer::BufferNotOwned);
	LWByteBuffer NetByteBuf(NetBuffer, sizeof(NetBuffer), LWByteBuffer::BufferNotOwned | LWByteBuffer::Network);
	ByteBuf.Write<int16_t>(4, Values16); ByteBuf.Write<int32_t>(4, Values32); ByteBuf.Write<int64_t>(4, Values64); ByteBuf.Write<float>(4, ValuesF); ByteBuf.Write<double>(4, ValuesD);
	ByteBuf.Write<LWVector2d>(4, ValuesVec2d); ByteBuf.Write<LWVector2f>(4, ValuesVec2f); ByteBuf.Write<LWVector2i>(4, ValuesVec2i);
	ByteBuf.Write<LWVector3d>(4, ValuesVec3d); ByteBuf.Write<LWVector3f>(4, ValuesVec3f); ByteBuf.Write<LWVector3i>(4, ValuesVec3i);
	ByteBuf.Write<LWVector4d>(4, ValuesVec4d); ByteBuf.Write<LWVector4f>(4, ValuesVec4f); ByteBuf.Write<LWVector4i>(4, ValuesVec4i);
	ByteBuf.Write<LWMatrix2d>(4, ValuesMat2d); ByteBuf.Write<LWMatrix2f>(4, ValuesMat2f); ByteBuf.Write<LWMatrix2i>(4, ValuesMat2i);
	ByteBuf.Write<LWMatrix3d>(4, ValuesMat3d); ByteBuf.Write<LWMatrix3f>(4, ValuesMat3f); ByteBuf.Write<LWMatrix3i>(4, ValuesMat3i);
	ByteBuf.Write<LWMatrix4d>(4, ValuesMat4d); ByteBuf.Write<LWMatrix4f>(4, ValuesMat4f); ByteBuf.Write<LWMatrix4i>(4, ValuesMat4i);
	ByteBuf.Write<LWQuaterniond>(4, ValueQuatd); ByteBuf.Write<LWQuaternionf>(4, ValueQuatf);
	ByteBuf.WriteUTF(LWUTF8Iterator(ValuesText)); ByteBuf.WriteText((const char*)ValuesText);
	NetByteBuf.Write<int16_t>(4, Values16); NetByteBuf.Write<int32_t>(4, Values32); NetByteBuf.Write<int64_t>(4, Values64); NetByteBuf.Write<float>(4, ValuesF); NetByteBuf.Write<double>(4, ValuesD);
	NetByteBuf.Write<LWVector2d>(4, ValuesVec2d); NetByteBuf.Write<LWVector2f>(4, ValuesVec2f); NetByteBuf.Write<LWVector2i>(4, ValuesVec2i);
	NetByteBuf.Write<LWVector3d>(4, ValuesVec3d); NetByteBuf.Write<LWVector3f>(4, ValuesVec3f); NetByteBuf.Write<LWVector3i>(4, ValuesVec3i);
	NetByteBuf.Write<LWVector4d>(4, ValuesVec4d); NetByteBuf.Write<LWVector4f>(4, ValuesVec4f); NetByteBuf.Write<LWVector4i>(4, ValuesVec4i);
	NetByteBuf.Write<LWMatrix2d>(4, ValuesMat2d); NetByteBuf.Write<LWMatrix2f>(4, ValuesMat2f); NetByteBuf.Write<LWMatrix2i>(4, ValuesMat2i);
	NetByteBuf.Write<LWMatrix3d>(4, ValuesMat3d); NetByteBuf.Write<LWMatrix3f>(4, ValuesMat3f); NetByteBuf.Write<LWMatrix3i>(4, ValuesMat3i);
	NetByteBuf.Write<LWMatrix4d>(4, ValuesMat4d); NetByteBuf.Write<LWMatrix4f>(4, ValuesMat4f); NetByteBuf.Write<LWMatrix4i>(4, ValuesMat4i);
	NetByteBuf.Write<LWQuaterniond>(4, ValueQuatd); NetByteBuf.Write<LWQuaternionf>(4, ValueQuatf);
	NetByteBuf.WriteUTF(LWUTF8Iterator(ValuesText)); NetByteBuf.WriteText((const char*)ValuesText);

	uint32_t BytePos = 0;
	auto ReadBytes = [&BytePos, &ByteBuf, &Buffer](int8_t *Buf, uint32_t Len, void *) ->uint32_t{
		Len = std::min<uint32_t>(ByteBuf.GetBytesWritten() - BytePos, Len);
		std::copy(Buffer + BytePos, Buffer + BytePos + Len, Buf);
		BytePos += Len;
		return Len;
	};

	uint32_t NetBytePos = 0;
	auto ReadNetBytes = [&NetBytePos, &NetByteBuf, &NetBuffer](int8_t *Buf, uint32_t Len, void*) {
		Len = std::min<uint32_t>(NetByteBuf.GetBytesWritten() - NetBytePos, Len);
		std::copy(NetBuffer + NetBytePos, NetBuffer + NetBytePos + Len, Buf);
		NetBytePos += Len;
		return Len;
	};
	fmt::print("Performing Byte Stream class test:\n");
	for (uint32_t i = 0; i < 2; i++) {
		LWByteStream Stream;
		if (i == 0) Stream = LWByteStream(5, ReadBytes, LWByteStream::AutoSize, nullptr, DefAlloc);
		else {
			fmt::print("Performing network byte stream test:\n");
			Stream = LWByteStream(5, ReadNetBytes, LWByteStream::Network|LWByteStream::AutoSize, nullptr, DefAlloc);
		}
		Result16[0] = Stream.Read<int16_t>();
		if (Stream.Read<int16_t>(Result16 + 1, 3) != sizeof(int16_t) * 3) return false;
		if (!TestEquality("Read<int16_t>", 4, Values16, Result16)) return false;
		Result32[0] = Stream.Read<int32_t>();
		if (Stream.Read<int32_t>(Result32 + 1, 3) != sizeof(int32_t) * 3) return false;
		if (!TestEquality("Read<int32_t>", 4, Values32, Result32)) return false;
		Result64[0] = Stream.Read<int64_t>();
		if (Stream.Read<int64_t>(Result64 + 1, 3) != sizeof(int64_t) * 3) return false;
		if (!TestEquality("Read<int64_t>", 4, Values64, Result64)) return false;
		ResultF[0] = Stream.Read<float>();
		if (Stream.Read<float>(ResultF + 1, 3) != sizeof(float) * 3) return false;
		if (!TestEquality("Read<float>", 4, ValuesF, ResultF)) return false;
		ResultD[0] = Stream.Read<double>();
		if (Stream.Read<double>(ResultD + 1, 3) != sizeof(double) * 3) return false;
		if (!TestEquality("Read<double>", 4, ValuesD, ResultD)) return false;
		ResultVec2d[0] = Stream.ReadVec2<double>();
		if (Stream.ReadVec2<double>(ResultVec2d + 1, 3) != sizeof(LWVector2d) * 3) return false;
		if (!TestEquality("ReadVec2<double>", 4, ValuesVec2d, ResultVec2d)) return false;
		ResultVec2f[0] = Stream.ReadVec2<float>();
		if (Stream.ReadVec2<float>(ResultVec2f + 1, 3) != sizeof(LWVector2f) * 3) return false;
		if (!TestEquality("ReadVec2<float>", 4, ValuesVec2f, ResultVec2f)) return false;
		ResultVec2i[0] = Stream.ReadVec2<int32_t>();
		if (Stream.ReadVec2<int32_t>(ResultVec2i + 1, 3) != sizeof(LWVector2i) * 3) return false;
		if (!TestEquality("ReadVec2<int32_t>", 4, ValuesVec2i, ResultVec2i)) return false;
		ResultVec3d[0] = Stream.ReadVec3<double>();
		if (Stream.ReadVec3<double>(ResultVec3d + 1, 3) != sizeof(LWVector3d) * 3) return false;
		if (!TestEquality("ReadVec3<double>", 4, ValuesVec3d, ResultVec3d)) return false;
		ResultVec3f[0] = Stream.ReadVec3<float>();
		if (Stream.ReadVec3<float>(ResultVec3f + 1, 3) != sizeof(LWVector3f) * 3) return false;
		if (!TestEquality("ReadVec3<float>", 4, ValuesVec3f, ResultVec3f)) return false;
		ResultVec3i[0] = Stream.ReadVec3<int32_t>();
		if (Stream.ReadVec3<int32_t>(ResultVec3i + 1, 3) != sizeof(LWVector3i) * 3) return false;
		if (!TestEquality("ReadVec3<int32_t>", 4, ValuesVec3i, ResultVec3i)) return false;
		ResultVec4d[0] = Stream.ReadVec4<double>();
		if (Stream.ReadVec4<double>(ResultVec4d + 1, 3) != sizeof(LWVector4d) * 3) return false;
		if (!TestEquality("ReadVec4<double>", 4, ValuesVec4d, ResultVec4d)) return false;
		ResultVec4f[0] = Stream.ReadVec4<float>();
		if (Stream.ReadVec4<float>(ResultVec4f + 1, 3) != sizeof(LWVector4f) * 3) return false;
		if (!TestEquality("ReadVec4<float>", 4, ValuesVec4f, ResultVec4f)) return false;
		ResultVec4i[0] = Stream.ReadVec4<int32_t>();
		if (Stream.ReadVec4<int32_t>(ResultVec4i + 1, 3) != sizeof(LWVector4i) * 3) return false;
		if (!TestEquality("ReadVec4<int32_t>", 4, ValuesVec4i, ResultVec4i)) return false;
		ResultMat2d[0] = Stream.ReadMat2<double>();
		if (Stream.ReadMat2<double>(ResultMat2d + 1, 3) != sizeof(LWMatrix2d) * 3) return false;
		if (!TestEquality("ReadMat2<double>", 4, ValuesMat2d, ResultMat2d)) return false;
		ResultMat2f[0] = Stream.ReadMat2<float>();
		if (Stream.ReadMat2<float>(ResultMat2f + 1, 3) != sizeof(LWMatrix2f) * 3) return false;
		if (!TestEquality("ReadMat2<float>", 4, ValuesMat2f, ResultMat2f)) return false;
		ResultMat2i[0] = Stream.ReadMat2<int32_t>();
		if (Stream.ReadMat2<int32_t>(ResultMat2i + 1, 3) != sizeof(LWMatrix2i) * 3) return false;
		if (!TestEquality("ReadMat2<int32_t>", 4, ValuesMat2i, ResultMat2i)) return false;
		ResultMat3d[0] = Stream.ReadMat3<double>();
		if (Stream.ReadMat3<double>(ResultMat3d + 1, 3) != sizeof(LWMatrix3d) * 3) return false;
		if (!TestEquality("ReadMat3<double>", 4, ValuesMat3d, ResultMat3d)) return false;
		ResultMat3f[0] = Stream.ReadMat3<float>();
		if (Stream.ReadMat3<float>(ResultMat3f + 1, 3) != sizeof(LWMatrix3f) * 3) return false;
		if (!TestEquality("ReadMat3<float>", 4, ValuesMat3f, ResultMat3f)) return false;
		ResultMat3i[0] = Stream.ReadMat3<int32_t>();
		if (Stream.ReadMat3<int32_t>(ResultMat3i + 1, 3) != sizeof(LWMatrix3i) * 3) return false;
		if (!TestEquality("ReadMat3<int32_t>", 4, ValuesMat3i, ResultMat3i)) return false;
		ResultMat4d[0] = Stream.ReadMat4<double>();
		if (Stream.ReadMat4<double>(ResultMat4d + 1, 3) != sizeof(LWMatrix4d) * 3) return false;
		if (!TestEquality("ReadMat4<double>", 4, ValuesMat4d, ResultMat4d)) return false;
		ResultMat4f[0] = Stream.ReadMat4<float>();
		if (Stream.ReadMat4<float>(ResultMat4f + 1, 3) != sizeof(LWMatrix4f) * 3) return false;
		if (!TestEquality("ReadMat4<float>", 4, ValuesMat4f, ResultMat4f)) return false;
		ResultMat4i[0] = Stream.ReadMat4<int32_t>();
		if (Stream.ReadMat4<int32_t>(ResultMat4i + 1, 3) != sizeof(LWMatrix4i) * 3) return false;
		if (!TestEquality("ReadMat4<int32_t>", 4, ValuesMat4i, ResultMat4i)) return false;
		ResultQuatd[0] = Stream.ReadQuaternion<double>();
		if (Stream.ReadQuaternion<double>(ResultQuatd + 1, 3) != sizeof(LWQuaterniond) * 3) return false;
		if (!TestEquality("ReadQuaternion<double>", 4, ValueQuatd, ResultQuatd)) return false;
		ResultQuatf[0] = Stream.ReadQuaternion<float>();
		if (Stream.ReadQuaternion<float>(ResultQuatf + 1, 3) != sizeof(LWQuaternionf) * 3) return false;
		if (!TestEquality("ReadQuaternion<float>", 4, ValueQuatf, ResultQuatf)) return false;
		Stream.ReadUTF(ResultText, sizeof(ResultText));
		if (!TestEquality("ReadUTF8", LWUTF8Iterator(ResultText).Compare(ValuesText), true)) return false;
		Stream.ReadText(ResultText, sizeof(ResultText));
		if (!TestEquality("ReadText", (uint32_t)strlen(ValuesText), ResultText, ValuesText)) return false;

	}
	fmt::print("LWByteStream test was successful.\n");
	return true;
}

bool PerformLWByteBufferTest(void){ 
	int8_t Buffer[4096];
	int64_t Values[4] = { 0x1122334455667788, 0x2233445566778899, 0x33445566778899AA, 0x445566778899AABB };
	float ValuesF[4] = { LW_PI, LW_2PI, LW_PI_2, LW_PI_4 };
	double ValuesD[4] = { LW_PI, LW_2PI, LW_PI_2, LW_PI_4 };
	LWVector2d ValuesVec2d[4] = { LWVector2d(), LWVector2d(1.0), LWVector2d(2.0), LWVector2d(3.0) };
	LWVector2f ValuesVec2f[4] = { LWVector2f(), LWVector2f(1.0f), LWVector2f(2.0f), LWVector2f(3.0f) };
	LWVector2i ValuesVec2i[4] = { LWVector2i(), LWVector2i(1), LWVector2i(2), LWVector2i(3) };
	LWVector3d ValuesVec3d[4] = { LWVector3d(), LWVector3d(1.0), LWVector3d(2.0), LWVector3d(3.0) };
	LWVector3f ValuesVec3f[4] = { LWVector3f(), LWVector3f(1.0f), LWVector3f(2.0f), LWVector3f(3.0f) };
	LWVector3i ValuesVec3i[4] = { LWVector3i(), LWVector3i(1), LWVector3i(2), LWVector3i(3) };
	LWVector4d ValuesVec4d[4] = { LWVector4d(), LWVector4d(1.0), LWVector4d(2.0), LWVector4d(3.0) };
	LWVector4f ValuesVec4f[4] = { LWVector4f(), LWVector4f(1.0f), LWVector4f(2.0f), LWVector4f(3.0f) };
	LWVector4i ValuesVec4i[4] = { LWVector4i(), LWVector4i(1), LWVector4i(2), LWVector4i(3) };
	LWMatrix4d ValuesMat4d[4] = { LWMatrix4d(), LWMatrix4d(2.0, 2.0, 2.0, 2.0), LWMatrix4d(3.0, 3.0, 3.0, 3.0), LWMatrix4d(4.0, 4.0, 4.0, 4.0) };
	LWMatrix4f ValuesMat4f[4] = { LWMatrix4f(), LWMatrix4f(2.0f, 2.0f, 2.0f, 2.0f), LWMatrix4f(3.0f, 3.0f, 3.0f, 3.0f), LWMatrix4f(4.0f, 4.0f, 4.0f, 4.0f) };
	LWMatrix4i ValuesMat4i[4] = { LWMatrix4i(), LWMatrix4i(2, 2, 2, 2), LWMatrix4i(3, 3, 3, 3), LWMatrix4i(4, 4, 4, 4) };
	LWMatrix3d ValuesMat3d[4] = { LWMatrix3d(), LWMatrix3d(2.0, 2.0, 2.0), LWMatrix3d(3.0, 3.0, 3.0), LWMatrix3d(4.0, 4.0, 4.0) };
	LWMatrix3f ValuesMat3f[4] = { LWMatrix3f(), LWMatrix3f(2.0f, 2.0f, 2.0f), LWMatrix3f(3.0f, 3.0f, 3.0f), LWMatrix3f(4.0f, 4.0f, 4.0f) };
	LWMatrix3i ValuesMat3i[4] = { LWMatrix3i(), LWMatrix3i(2, 2, 2), LWMatrix3i(3, 3, 3) };
	LWMatrix2d ValuesMat2d[4] = { LWMatrix2d(), LWMatrix2d(2.0, 2.0), LWMatrix2d(3.0, 3.0), LWMatrix2d(4.0, 4.0) };
	LWMatrix2f ValuesMat2f[4] = { LWMatrix2f(), LWMatrix2f(2.0f, 2.0f), LWMatrix2f(3.0f, 3.0f), LWMatrix2f(4.0f, 4.0f) };
	LWMatrix2i ValuesMat2i[4] = { LWMatrix2i(), LWMatrix2i(2, 2), LWMatrix2i(3, 3), LWMatrix2i(4, 4) };
	char ValuesText[] = { "This is a test utf-8 string." };
	
	int64_t Result[4] = { 0, 0, 0, 0 };
	float ResultF[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	double ResultD[4] = { 0.0, 0.0, 0.0, 0.0 };
	LWVector2d ResultVec2d[4];
	LWVector2f ResultVec2f[4];
	LWVector2i ResultVec2i[4];
	LWVector3d ResultVec3d[4];
	LWVector3f ResultVec3f[4];
	LWVector3i ResultVec3i[4];
	LWVector4d ResultVec4d[4];
	LWVector4f ResultVec4f[4];
	LWVector4i ResultVec4i[4];
	LWMatrix4d ResultMat4d[4];
	LWMatrix4f ResultMat4f[4];
	LWMatrix4i ResultMat4i[4];
	LWMatrix3d ResultMat3d[4];
	LWMatrix3f ResultMat3f[4];
	LWMatrix3i ResultMat3i[4];
	LWMatrix2d ResultMat2d[4];
	LWMatrix2f ResultMat2f[4];
	LWMatrix2i ResultMat2i[4];
	char ResultText[32];

	const uint32_t CheckEndianValue = 0x44332211;
	const bool BigEndian = ((int8_t*)&CheckEndianValue)[0] != 0x11;
	const uint32_t Pi32 = BigEndian ? 1078530011 : 3675212096;
	const uint64_t Pi64 = BigEndian ? 4614256656748904448 : 416530106688;
	fmt::print("Performing Byte Buffer class test:\nIs BigEndian: {}\nTesting Static members:\n", BigEndian);

	if (!PerformTest("MakeNetwork<int8>",   std::bind<int8_t(int8_t)>     (LWByteBuffer::MakeNetwork, 0x11), 0x11, true)) return false;
	if (!PerformTest("MakeNetwork<uint8>",  std::bind<uint8_t(uint8_t)>   (LWByteBuffer::MakeNetwork, 0x11), 0x11, true)) return false;
	if (!PerformTest("MakeNetwork<int16>",  std::bind<int16_t(int16_t)>   (LWByteBuffer::MakeNetwork, 0x1122),             BigEndian ? 0x1122:0x2211, true)) return false;
	if (!PerformTest("MakeNetwork<uint16>", std::bind<uint16_t(uint16_t)> (LWByteBuffer::MakeNetwork, 0x1122),             BigEndian ? 0x1122:0x2211, true)) return false;
	if (!PerformTest("MakeNetwork<int32>",  std::bind<int32_t(int32_t)>   (LWByteBuffer::MakeNetwork, 0x11223344),         BigEndian ? 0x11223344 : 0x44332211, true)) return false;
	if (!PerformTest("MakeNetwork<uint32>", std::bind<uint32_t(uint32_t)> (LWByteBuffer::MakeNetwork, 0x11223344),         BigEndian ? 0x11223344 : 0x44332211, true)) return false;
	if (!PerformTest("MakeNetwork<int64>",  std::bind<int64_t(int64_t)>   (LWByteBuffer::MakeNetwork, 0x1122334455667788), BigEndian ? 0x1122334455667788 : 0x8877665544332211, true)) return false;
	if (!PerformTest("MakeNetwork<uint64>", std::bind<uint64_t(uint64_t)> (LWByteBuffer::MakeNetwork, 0x1122334455667788), BigEndian ? 0x1122334455667788 : 0x8877665544332211, true)) return false;
	if (!PerformTest("MakeNetwork<float>",  std::bind<uint32_t(float)>    (LWByteBuffer::MakeNetwork, LW_PI),         Pi32)) return false;
	if (!PerformTest("MakeNetwork<double>", std::bind<uint64_t(double)>   (LWByteBuffer::MakeNetwork, (double)LW_PI), Pi64)) return false;
	if (!PerformTest("MakeHost<int8>",   std::bind<int8_t(int8_t)>    (LWByteBuffer::MakeHost, 0x11), 0x11, true)) return false;
	if (!PerformTest("MakeHost<uint8>",  std::bind<uint8_t(uint8_t)>  (LWByteBuffer::MakeHost, 0x11), 0x11, true)) return false;
	if (!PerformTest("MakeHost<int16>",  std::bind<int16_t(int16_t)>  (LWByteBuffer::MakeHost, BigEndian ? 0x1122 : 0x2211),                         0x1122, true)) return false;
	if (!PerformTest("MakeHost<uint16>", std::bind<uint16_t(uint16_t)>(LWByteBuffer::MakeHost, BigEndian ? 0x1122 : 0x2211),                         0x1122, true)) return false;
	if (!PerformTest("MakeHost<int32>",  std::bind<int32_t(int32_t)>  (LWByteBuffer::MakeHost, BigEndian ? 0x11223344 : 0x44332211),                 0x11223344, true)) return false;
	if (!PerformTest("MakeHost<uint32>", std::bind<uint32_t(uint32_t)>(LWByteBuffer::MakeHost, BigEndian ? 0x11223344 : 0x44332211),                 0x11223344, true)) return false;
	if (!PerformTest("MakeHost<int64>",  std::bind<int64_t(int64_t)>  (LWByteBuffer::MakeHost, BigEndian ? 0x1122334455667788 : 0x8877665544332211), 0x1122334455667788, true)) return false;
	if (!PerformTest("MakeHost<uint64>", std::bind<uint64_t(uint64_t)>(LWByteBuffer::MakeHost, BigEndian ? 0x1122334455667788 : 0x8877665544332211), 0x1122334455667788, true)) return false;
	if (!PerformTest("MakeHost<float>",  std::bind<float(uint32_t)>   (LWByteBuffer::MakeHostf, Pi32),         LW_PI)) return false;
	if (!PerformTest("MakeHost<double>", std::bind<double(uint64_t)>  (LWByteBuffer::MakeHostf, Pi64), (double)LW_PI)) return false;
	
	if (!PerformTest("Write<int8>", std::bind<int32_t(int8_t, int8_t*)>(LWByteBuffer::Write, 0x11, Buffer), 1)) return false;
	if ((*((int8_t*)Buffer)) != 0x11) return false;
	if (!PerformTest("Read<int8>", std::bind<int32_t(int8_t*, const int8_t*)>(LWByteBuffer::Read, (int8_t*)Result, Buffer), 1)) return false;
	if (Result[0] != 0x11) return false;
	if (!PerformTest("Write<uint8>", std::bind<int32_t(int8_t, int8_t*)>(LWByteBuffer::Write, 0x11, Buffer), 1)) return false;
	if ((*((uint8_t*)Buffer)) != 0x11) return false;
	if (!PerformTest("Read<uint8>", std::bind<int32_t(uint8_t*, const int8_t*)>(LWByteBuffer::Read, (uint8_t*)Result, Buffer), 1)) return false;
	if (Result[0] != 0x11) return false;

	if (!PerformTest("Write<int16>", std::bind<int32_t(int16_t, int8_t*)>(LWByteBuffer::Write, 0x1122, Buffer), 2)) return false;
	if ((*((int16_t*)Buffer)) != 0x1122) return false;
	if (!PerformTest("Read<int16>", std::bind<int32_t(int16_t*, const int8_t*)>(LWByteBuffer::Read, (int16_t*)&Result, Buffer), 2)) return false;
	if (Result[0] != 0x1122) return false;
	if (!PerformTest("Write<uint16>", std::bind<int32_t(uint16_t, int8_t*)>(LWByteBuffer::Write, 0x1122, Buffer), 2)) return false;
	if ((*((uint16_t*)Buffer)) != 0x1122) return false;
	if (!PerformTest("Read<uint16>", std::bind<int32_t(uint16_t*, const int8_t*)>(LWByteBuffer::Read, (uint16_t*)Result, Buffer), 2)) return false;
	if (Result[0] != 0x1122) return false;

	if (!PerformTest("Write<int32>", std::bind<int32_t(int32_t, int8_t*)>(LWByteBuffer::Write, 0x11223344, Buffer), 4)) return false;
	if ((*((int32_t*)Buffer)) != 0x11223344) return false;
	if (!PerformTest("Read<int32>", std::bind<int32_t(int32_t*, const int8_t*)>(LWByteBuffer::Read, (int32_t*)Result, Buffer), 4)) return false;
	if (Result[0] != 0x11223344) return false;
	if (!PerformTest("Write<uint32>", std::bind<int32_t(uint32_t, int8_t*)>(LWByteBuffer::Write, 0x11223344, Buffer), 4)) return false;
	if ((*((uint32_t*)Buffer)) != 0x11223344) return false;
	if (!PerformTest("Read<uint32>", std::bind<int32_t(uint32_t*, const int8_t*)>(LWByteBuffer::Read, (uint32_t*)Result, Buffer), 4)) return false;
	if (Result[0] != 0x11223344) return false;

	if (!PerformTest("Write<int64>", std::bind<int32_t(int64_t, int8_t*)>(LWByteBuffer::Write, 0x1122334455667788, Buffer), 8)) return false;
	if ((*((int64_t*)Buffer)) != 0x1122334455667788) return false;
	if (!PerformTest("Read<int64>", std::bind<int32_t(int64_t *, const int8_t*)>(LWByteBuffer::Read, (int64_t*)Result, Buffer), 8)) return false;
	if (Result[0] != 0x1122334455667788) return false;
	if (!PerformTest("Write<uint64>", std::bind<int32_t(uint64_t, int8_t*)>(LWByteBuffer::Write, 0x1122334455667788, Buffer), 8)) return false;
	if ((*((uint64_t*)Buffer)) != 0x1122334455667788) return false;
	if (!PerformTest("Read<int64>", std::bind<int32_t(uint64_t *, const int8_t*)>(LWByteBuffer::Read, (uint64_t*)Result, Buffer), 8)) return false;
	if (Result[0] != 0x1122334455667788) return false;

	if (!PerformTest("Write<Float>", std::bind<int32_t(float, int8_t*)>(LWByteBuffer::Write, LW_PI, Buffer), 4)) return false;
	if ((*((float*)Buffer)) != LW_PI) return false;
	if (!PerformTest("Read<float>", std::bind<int32_t(float*, const int8_t*)>(LWByteBuffer::Read, ResultF, Buffer), 4)) return false;
	if (ResultF[0] != LW_PI) return false;
	if (!PerformTest("Write<Double>", std::bind<int32_t(double, int8_t*)>(LWByteBuffer::Write, LW_PI, Buffer), 8)) return false;
	if ((*((double*)Buffer)) != LW_PI) return false;
	if (!PerformTest("Read<Double>", std::bind<int32_t(double*, const int8_t*)>(LWByteBuffer::Read, ResultD, Buffer), 8)) return false;
	if (ResultD[0] != LW_PI) return false;

	if (!PerformTest("Write<Vector4f>", std::bind<int32_t(const LWVector4f &, int8_t*)>(LWByteBuffer::Write, ValuesVec4f[0], Buffer), 16)) return false;
	if ((*((LWVector4f*)Buffer)) != ValuesVec4f[0]) return false;

	if (!PerformTest("Write<Vector4i>", std::bind<int32_t(const LWVector4i &, int8_t*)>(LWByteBuffer::Write, ValuesVec4i[0], Buffer), 16)) return false;
	if ((*((LWVector4i*)Buffer)) != ValuesVec4i[0]) return false;

	if (!PerformTest("Write<int8, 4>", std::bind<int32_t(uint32_t,const int8_t*, int8_t*)>(LWByteBuffer::Write, 4, (int8_t*)Values, Buffer), 4)) return false;
	if (((uint32_t*)Buffer)[0] != 0x55667788) return false;
	if (!PerformTest("Read<int8, 4>", std::bind<int32_t(int8_t*, uint32_t, const int8_t*)>(LWByteBuffer::Read, (int8_t*)Result, 4, Buffer), 4)) return false;
	if (((uint32_t*)Result)[0] != 0x55667788) return false;
	if (!PerformTest("Write<uint8, 4>", std::bind<int32_t(uint32_t, const uint8_t*, int8_t*)>(LWByteBuffer::Write, 4, (uint8_t*)Values, Buffer), 4)) return false;
	if (((uint32_t*)Buffer)[0] != 0x55667788) return false;
	if (!PerformTest("Read<uint8, 4>", std::bind<int32_t(uint8_t*, uint32_t, const int8_t*)>(LWByteBuffer::Read, (uint8_t*)Result, 4, Buffer), 4)) return false;
	if (((uint32_t*)Result)[0] != 0x55667788) return false;
	
	if (!PerformTest("Write<int16, 4>", std::bind<int32_t(uint32_t, const int16_t*, int8_t*)>(LWByteBuffer::Write, 4, (int16_t*)Values, Buffer), 8)) return false;
	if (((uint64_t*)Buffer)[0] != 0x1122334455667788) return false;
	if (!PerformTest("Read<int16, 4>", std::bind<int32_t(int16_t*, uint32_t, const int8_t*)>(LWByteBuffer::Read, (int16_t*)Result, 4, Buffer), 8)) return false;
	if (((uint64_t*)Result)[0] != 0x1122334455667788) return false;
	if (!PerformTest("Write<uint16, 4>", std::bind<int32_t(uint32_t, const uint16_t*, int8_t*)>(LWByteBuffer::Write, 4, (uint16_t*)Values, Buffer), 8)) return false;
	if (((uint64_t*)Buffer)[0] != 0x1122334455667788) return false;
	if (!PerformTest("Read<uint16, 4>", std::bind<int32_t(uint16_t*, uint32_t, const int8_t*)>(LWByteBuffer::Read, (uint16_t*)Result, 4, Buffer), 8)) return false;
	if (((uint64_t*)Result)[0] != 0x1122334455667788) return false;

	if (!PerformTest("Write<int32, 4>", std::bind<int32_t(uint32_t, const int32_t*, int8_t*)>(LWByteBuffer::Write, 4, (int32_t*)Values, Buffer), 16)) return false;
	if (((uint64_t*)Buffer)[0] != 0x1122334455667788 || ((int64_t*)Buffer)[1] != 0x2233445566778899) return false;
	if (!PerformTest("Read<int32, 4>", std::bind<int32_t(int32_t*, uint32_t, const int8_t*)>(LWByteBuffer::Read, (int32_t*)Result, 4, Buffer), 16)) return false;
	if (((uint64_t*)Result)[0] != 0x1122334455667788 || ((uint64_t*)Buffer)[1] != 0x2233445566778899) return false;
	if (!PerformTest("Write<uint32, 4>", std::bind<int32_t(uint32_t, const uint32_t*, int8_t*)>(LWByteBuffer::Write, 4, (uint32_t*)Values, Buffer), 16)) return false;
	if (((uint64_t*)Buffer)[0] != 0x1122334455667788 || ((uint64_t*)Buffer)[1] != 0x2233445566778899) return false;
	if (!PerformTest("Read<uint32, 4>", std::bind<int32_t(uint32_t*, uint32_t, const int8_t*)>(LWByteBuffer::Read, (uint32_t*)Result, 4, Buffer), 16)) return false;
	if (((uint64_t*)Result)[0] != 0x1122334455667788 || ((uint64_t*)Buffer)[1] != 0x2233445566778899) return false;

	if (!PerformTest("Write<int64, 4>", std::bind<int32_t(uint32_t, const int64_t*, int8_t*)>(LWByteBuffer::Write, 4, Values, Buffer), 32)) return false;
	if (((uint64_t*)Buffer)[0] != 0x1122334455667788 || ((uint64_t*)Buffer)[1] != 0x2233445566778899 || ((uint64_t*)Buffer)[2] != 0x33445566778899AA || ((uint64_t*)Buffer)[3] != 0x445566778899AABB) return false;
	if (!PerformTest("Read<int64, 4>", std::bind<int32_t(int64_t*, uint32_t, const int8_t*)>(LWByteBuffer::Read, Result, 4, Buffer), 32)) return false;
	if (((uint64_t*)Result)[0] != 0x1122334455667788 || ((uint64_t*)Result)[1] != 0x2233445566778899 || ((uint64_t*)Result)[2] != 0x33445566778899AA || ((uint64_t*)Result)[3] != 0x445566778899AABB) return false;
	if (!PerformTest("Write<uint64, 4>", std::bind<int32_t(uint32_t, const uint64_t*, int8_t*)>(LWByteBuffer::Write, 4, (uint64_t*)Values, Buffer), 32)) return false;
	if (((uint64_t*)Buffer)[0] != 0x1122334455667788 || ((uint64_t*)Buffer)[1] != 0x2233445566778899 || ((uint64_t*)Buffer)[2] != 0x33445566778899AA || ((uint64_t*)Buffer)[3] != 0x445566778899AABB) return false;
	if (!PerformTest("Read<int64, 4>", std::bind<int32_t(uint64_t*, uint32_t, const int8_t*)>(LWByteBuffer::Read, (uint64_t*)Result, 4, Buffer), 32)) return false;
	if (((uint64_t*)Result)[0] != 0x1122334455667788 || ((uint64_t*)Result)[1] != 0x2233445566778899 || ((uint64_t*)Result)[2] != 0x33445566778899AA || ((uint64_t*)Result)[3] != 0x445566778899AABB) return false;
	 
	if (!PerformTest("Write<float, 4>", std::bind<int32_t(uint32_t, const float*, int8_t*)>(LWByteBuffer::Write, 4, ValuesF, Buffer), 16)) return false;
	if (((float*)Buffer)[0] != ValuesF[0] || ((float*)Buffer)[1] != ValuesF[1] || ((float*)Buffer)[2] != ValuesF[2] || ((float*)Buffer)[3] != ValuesF[3]) return false;
	if (!PerformTest("Read<float, 4>", std::bind<int32_t(float*, uint32_t, const int8_t*)>(LWByteBuffer::Read, ResultF, 4, Buffer), 16)) return false;
	if (ResultF[0] != ValuesF[0] || ResultF[1] != ValuesF[1] || ResultF[2] != ValuesF[2] || ResultF[3] != ValuesF[3]) return false;
	if (!PerformTest("Write<Double, 4>", std::bind<int32_t(uint32_t, const double*, int8_t*)>(LWByteBuffer::Write, 4, ValuesD, Buffer), 32)) return false;
	if (((double*)Buffer)[0] != ValuesD[0] || ((double*)Buffer)[1] != ValuesD[1] || ((double*)Buffer)[2] != ValuesD[2] || ((double*)Buffer)[3] != ValuesD[3]) return false;
	if (!PerformTest("Read<Double, 4>", std::bind<int32_t(double*, uint32_t, const int8_t*)>(LWByteBuffer::Read, ResultD, 4, Buffer), 32)) return false;
	if (ResultD[0] != ValuesD[0] || ResultD[1] != ValuesD[1] || ResultD[2] != ValuesD[2] || ResultD[3] != ValuesD[3]) return false;

	if (!PerformTest("WriteNetwork<int8>", std::bind<int32_t(int8_t, int8_t*)>(LWByteBuffer::WriteNetwork, 0x11, Buffer), 1)) return false;
	if ((*((int8_t*)Buffer)) != 0x11) return false;
	if (!PerformTest("ReadNetwork<int8>", std::bind < int32_t(int8_t*, const int8_t*)>(LWByteBuffer::ReadNetwork, (int8_t*)Result, Buffer), 1)) return false;
	if (((int8_t*)Result)[0] != 0x11) return false;
	if (!PerformTest("WriteNetwork<uint8", std::bind<int32_t(int8_t, int8_t*)>(LWByteBuffer::WriteNetwork, 0x11, Buffer), 1)) return false;
	if ((*((uint8_t*)Buffer)) != 0x11) return false;
	if (!PerformTest("ReadNetwork<uint8>", std::bind<int32_t(uint8_t*, const int8_t*)>(LWByteBuffer::ReadNetwork, (uint8_t*)Result, Buffer), 1)) return false;
	if (((uint8_t*)Result)[0] != 0x11) return false;
	
	if (!PerformTest("WriteNetwork<int16>", std::bind<int32_t(int16_t, int8_t*)>(LWByteBuffer::WriteNetwork, 0x1122, Buffer), 2)) return false;
	if ((*((int16_t*)Buffer)) != (BigEndian?0x1122:0x2211)) return false;
	if (!PerformTest("ReadNetwork<int16>", std::bind<int32_t(int16_t*, const int8_t *)>(LWByteBuffer::ReadNetwork, (int16_t*)Result, Buffer), 2)) return false;
	if (((int16_t*)Result)[0] != 0x1122) return false;
	if (!PerformTest("WriteNetwork<uint16>", std::bind<int32_t(uint16_t, int8_t*)>(LWByteBuffer::WriteNetwork, 0x1122, Buffer), 2)) return false;
	if ((*((uint16_t*)Buffer)) != (BigEndian?0x1122:0x2211)) return false;
	if (!PerformTest("ReadNetwork<uint16>", std::bind<int32_t(uint16_t*, const int8_t*)>(LWByteBuffer::ReadNetwork, (uint16_t*)Result, Buffer), 2)) return false;
	if (((uint16_t*)Result)[0] != 0x1122) return false;

	if (!PerformTest("WriteNetwork<int32>", std::bind<int32_t(int32_t, int8_t*)>(LWByteBuffer::WriteNetwork, 0x11223344, Buffer), 4)) return false;
	if ((*((int32_t*)Buffer)) != (BigEndian?0x11223344:0x44332211)) return false;
	if (!PerformTest("ReadNetwork<int32>", std::bind<int32_t(int32_t*, const int8_t*)>(LWByteBuffer::ReadNetwork, (int32_t*)Result, Buffer), 4)) return false;
	if (((int32_t*)Result)[0] != 0x11223344) return false;
	if (!PerformTest("WriteNetwork<uint32>", std::bind<int32_t(uint32_t, int8_t*)>(LWByteBuffer::WriteNetwork, 0x11223344, Buffer), 4)) return false;
	if ((*((uint32_t*)Buffer)) != (BigEndian?0x11223344U:0x44332211U)) return false;
	if (!PerformTest("ReadNetwork<uint32>", std::bind<int32_t(uint32_t*, const int8_t*)>(LWByteBuffer::ReadNetwork, (uint32_t*)Result, Buffer), 4)) return false;
	if (((uint32_t*)Result)[0] != 0x11223344) return false;

	if (!PerformTest("WriteNetwork<int64>", std::bind<int32_t(int64_t, int8_t*)>(LWByteBuffer::WriteNetwork, 0x1122334455667788, Buffer), 8)) return false;
	if ((*((int64_t*)Buffer)) != (int64_t)(BigEndian?0x1122334455667788:0x8877665544332211)) return false;
	if (!PerformTest("ReadNetwork<int64>", std::bind<int32_t(int64_t*, const int8_t*)>(LWByteBuffer::ReadNetwork, (int64_t*)Result, Buffer), 8)) return false;
	if (((int64_t*)Result)[0] != 0x1122334455667788) return false;
	if (!PerformTest("WriteNetwork<uint64>", std::bind<int32_t(uint64_t, int8_t*)>(LWByteBuffer::WriteNetwork, 0x1122334455667788, Buffer), 8)) return false;
	if ((*((uint64_t*)Buffer)) != (BigEndian?0x1122334455667788:0x8877665544332211)) return false;
	if (!PerformTest("ReadNetwork<uint64>", std::bind<int32_t(uint64_t*, const int8_t*)>(LWByteBuffer::ReadNetwork, (uint64_t*)Result, Buffer), 8)) return false;
	if (((uint64_t*)Result)[0] != 0x1122334455667788) return false;
	
	if (!PerformTest("WriteNetwork<Float>", std::bind<int32_t(float, int8_t*)>(LWByteBuffer::WriteNetwork, LW_PI, Buffer), 4)) return false;
	if ((*((uint32_t*)Buffer)) != Pi32) return false;
	if (!PerformTest("ReadNetwork<Float>", std::bind<int32_t(float*, const int8_t*)>(LWByteBuffer::ReadNetwork, ResultF, Buffer), 4)) return false;
	if (ResultF[0] != LW_PI) return false;
	if (!PerformTest("WriteNetwork<Double>", std::bind<int32_t(double, int8_t*)>(LWByteBuffer::WriteNetwork, LW_PI, Buffer), 8)) return false;
	if ((*((uint64_t*)Buffer)) != Pi64) return false;
	if (!PerformTest("Read<Double>", std::bind<int32_t(double*, const int8_t*)>(LWByteBuffer::ReadNetwork, ResultD, Buffer), 8)) return false;
	if (ResultD[0] != LW_PI) return false;
	
	if (!PerformTest("WriteNetwork<LWVector4f>", std::bind<int32_t(const LWVector4f &, int8_t*)>(LWByteBuffer::WriteNetwork, ValuesVec4f[0], Buffer), 16)) return false;
	if (!PerformTest("ReadNetwork<LWVector4f>", std::bind<int32_t(LWVector4f*, const int8_t*)>(LWByteBuffer::ReadNetwork, ResultVec4f, Buffer), 16)) return false;
	if (ResultVec4f[0] != ValuesVec4f[0]) return false;
	if (!PerformTest("WriteNetwork<LWVector4i>", std::bind<int32_t(const LWVector4i &, int8_t*)>(LWByteBuffer::WriteNetwork, ValuesVec4i[0], Buffer), 16)) return false;
	if (!PerformTest("ReadNetwork<LWVector4i>", std::bind<int32_t(LWVector4i*, const int8_t*)>(LWByteBuffer::ReadNetwork, ResultVec4i, Buffer), 16)) return false;
	if (ResultVec4i[0] != ValuesVec4i[0]) return false;

	if (!PerformTest("WriteNetwork<LWVector4f, 4>", std::bind<int32_t(uint32_t, const LWVector4f *, int8_t*)>(LWByteBuffer::WriteNetwork, 4, ValuesVec4f, Buffer), 64)) return false;
	if (!PerformTest("ReadNetwork<LWVector4f, 4>", std::bind<int32_t(LWVector4f*, uint32_t, const int8_t*)>(LWByteBuffer::ReadNetwork, ResultVec4f, 4, Buffer), 64)) return false;
	if (ResultVec4f[0] != ValuesVec4f[0] || ResultVec4f[1] != ValuesVec4f[1] || ResultVec4f[2] != ValuesVec4f[2] || ResultVec4f[3] != ValuesVec4f[3]) return false;
	if (!PerformTest("WriteNetwork<LWVector4i, 4>", std::bind<int32_t(uint32_t, const LWVector4i *, int8_t*)>(LWByteBuffer::WriteNetwork, 4, ValuesVec4i, Buffer), 64)) return false;
	if (!PerformTest("ReadNetwork<LWVector4i, 4>", std::bind<int32_t(LWVector4i*, uint32_t, const int8_t*)>(LWByteBuffer::ReadNetwork, ResultVec4i, 4, Buffer), 64)) return false;
	if (ResultVec4i[0] != ValuesVec4i[0] || ResultVec4i[1] != ValuesVec4i[1] || ResultVec4i[2] != ValuesVec4i[2] || ResultVec4i[3] != ValuesVec4i[3]) return false;


	LWByteBuffer NormalBuf(Buffer, sizeof(Buffer), LWByteBuffer::BufferNotOwned);
	fmt::print("Performing Read/Write on Normal buffer:\n");
	NormalBuf.Write(10);
	NormalBuf.WriteL<int32_t>(2, 11, 12);
	NormalBuf.Write(LW_PI);
	NormalBuf.Write(4, Values);
	NormalBuf.Write(ValuesVec2d[0]);
	NormalBuf.Write(ValuesVec2f[0]);
	NormalBuf.Write(ValuesVec2i[0]);
	NormalBuf.Write(ValuesVec3d[0]);
	NormalBuf.Write(ValuesVec3f[0]);
	NormalBuf.Write(ValuesVec3i[0]);
	NormalBuf.Write(ValuesVec4d[0]);
	NormalBuf.Write(ValuesVec4f[0]);
	NormalBuf.Write(ValuesVec4i[0]);
	NormalBuf.Write(ValuesMat4d[0]);
	NormalBuf.Write(ValuesMat4f[0]);
	NormalBuf.Write(ValuesMat4i[0]);
	NormalBuf.Write(ValuesMat3d[0]);
	NormalBuf.Write(ValuesMat3f[0]);
	NormalBuf.Write(ValuesMat3i[0]);
	NormalBuf.Write(ValuesMat2d[0]);
	NormalBuf.Write(ValuesMat2f[0]);
	NormalBuf.Write(ValuesMat2i[0]);
	NormalBuf.Write(4, ValuesVec2d);
	NormalBuf.Write(4, ValuesVec2f);
	NormalBuf.Write(4, ValuesVec2i);
	NormalBuf.Write(4, ValuesVec3d);
	NormalBuf.Write(4, ValuesVec3f);
	NormalBuf.Write(4, ValuesVec3i);
	NormalBuf.Write(4, ValuesVec4d);
	NormalBuf.Write(4, ValuesVec4f);
	NormalBuf.Write(4, ValuesVec4i);
	NormalBuf.Write(4, ValuesMat4d);
	NormalBuf.Write(4, ValuesMat4f);
	NormalBuf.Write(4, ValuesMat4i);
	NormalBuf.Write(4, ValuesMat3d);
	NormalBuf.Write(4, ValuesMat3f);
	NormalBuf.Write(4, ValuesMat3i);
	NormalBuf.Write(4, ValuesMat2d);
	NormalBuf.Write(4, ValuesMat2f);
	NormalBuf.Write(4, ValuesMat2i);
	NormalBuf.WriteUTF(LWUTF8Iterator(ValuesText));
	fmt::print("Normal Wrote to position: {} | {}\n", NormalBuf.GetPosition(), NormalBuf.GetBytesWritten());
	NormalBuf.SetPosition(0);
	fmt::print("A");
	if (NormalBuf.Read<int32_t>() != 10) return false;
	fmt::print("B");
	if (NormalBuf.Read<int32_t>() != 11) return false;
	fmt::print("C");
	if (NormalBuf.Read<int32_t>() != 12) return false;
	fmt::print("D");
	if (NormalBuf.Read<float>() != LW_PI) return false;
	fmt::print("E");
	if (NormalBuf.Read<uint64_t>() != (uint64_t)Values[0]) return false;
	fmt::print("F");
	if (NormalBuf.Read<uint64_t>() != (uint64_t)Values[1]) return false;
	fmt::print("G");
	if (NormalBuf.Read<uint64_t>() != (uint64_t)Values[2]) return false;
	fmt::print("H");
	if (NormalBuf.Read<uint64_t>() != (uint64_t)Values[3]) return false;
	fmt::print("I");
	if (NormalBuf.Read<uint64_t>(16) != (uint64_t)Values[0]) return false;
	fmt::print("J");
	if (NormalBuf.Read<LWVector2d>() != ValuesVec2d[0]) return false;
	fmt::print("K");
	if (NormalBuf.Read<LWVector2f>() != ValuesVec2f[0]) return false;
	fmt::print("L");
	if (NormalBuf.Read<LWVector2i>() != ValuesVec2i[0]) return false;
	fmt::print("M");
	if (NormalBuf.Read<LWVector3d>() != ValuesVec3d[0]) return false;
	fmt::print("N");
	if (NormalBuf.Read<LWVector3f>() != ValuesVec3f[0]) return false;
	fmt::print("O");
	if (NormalBuf.Read<LWVector3i>() != ValuesVec3i[0]) return false;
	fmt::print("P");
	if (NormalBuf.Read<LWVector4d>() != ValuesVec4d[0]) return false;
	fmt::print("Q");
	if (NormalBuf.Read<LWVector4f>() != ValuesVec4f[0]) return false;
	fmt::print("R");
	if (NormalBuf.Read<LWVector4i>() != ValuesVec4i[0]) return false;
	fmt::print("S");
	if (NormalBuf.Read<LWMatrix4d>() != ValuesMat4d[0]) return false;
	fmt::print("T");
	if (NormalBuf.Read<LWMatrix4f>() != ValuesMat4f[0]) return false;
	fmt::print("U");
	if (NormalBuf.Read<LWMatrix4i>() != ValuesMat4i[0]) return false;
	fmt::print("V");
	if (NormalBuf.Read<LWMatrix3d>() != ValuesMat3d[0]) return false;
	fmt::print("W");
	if (NormalBuf.Read<LWMatrix3f>() != ValuesMat3f[0]) return false;
	fmt::print("X");
	if (NormalBuf.Read<LWMatrix3i>() != ValuesMat3i[0]) return false;
	fmt::print("Y");
	if (NormalBuf.Read<LWMatrix2d>() != ValuesMat2d[0]) return false;
	fmt::print("Z");
	if (NormalBuf.Read<LWMatrix2f>() != ValuesMat2f[0]) return false;
	fmt::print("a");
	if (NormalBuf.Read<LWMatrix2i>() != ValuesMat2i[0]) return false;
	fmt::print("b");
	NormalBuf.Read(ResultVec2d, 4);
	NormalBuf.Read(ResultVec2f, 4);
	NormalBuf.Read(ResultVec2i, 4);
	if (ResultVec2d[0] != ValuesVec2d[0] || ResultVec2d[1] != ValuesVec2d[1] || ResultVec2d[2] != ValuesVec2d[2] || ResultVec2d[3] != ValuesVec2d[3]) return false;
	fmt::print("c");
	if (ResultVec2f[0] != ValuesVec2f[0] || ResultVec2f[1] != ValuesVec2f[1] || ResultVec2f[2] != ValuesVec2f[2] || ResultVec2f[3] != ValuesVec2f[3]) return false;
	fmt::print("d");
	if (ResultVec2i[0] != ValuesVec2i[0] || ResultVec2i[1] != ValuesVec2i[1] || ResultVec2i[2] != ValuesVec2i[2] || ResultVec2i[3] != ValuesVec2i[3]) return false;
	fmt::print("e");
	NormalBuf.Read(ResultVec3d, 4);
	NormalBuf.Read(ResultVec3f, 4);
	NormalBuf.Read(ResultVec3i, 4);
	if (ResultVec3d[0] != ValuesVec3d[0] || ResultVec3d[1] != ValuesVec3d[1] || ResultVec3d[2] != ValuesVec3d[2] || ResultVec3d[3] != ValuesVec3d[3]) return false;
	fmt::print("f");
	if (ResultVec3f[0] != ValuesVec3f[0] || ResultVec3f[1] != ValuesVec3f[1] || ResultVec3f[2] != ValuesVec3f[2] || ResultVec3f[3] != ValuesVec3f[3]) return false;
	fmt::print("g");
	if (ResultVec3i[0] != ValuesVec3i[0] || ResultVec3i[1] != ValuesVec3i[1] || ResultVec3i[2] != ValuesVec3i[2] || ResultVec3i[3] != ValuesVec3i[3]) return false;
	fmt::print("h");
	NormalBuf.Read(ResultVec4d, 4);
	NormalBuf.Read(ResultVec4f, 4);
	NormalBuf.Read(ResultVec4i, 4);
	if (ResultVec4d[0] != ValuesVec4d[0] || ResultVec4d[1] != ValuesVec4d[1] || ResultVec4d[2] != ValuesVec4d[2] || ResultVec4d[3] != ValuesVec4d[3]) return false;
	fmt::print("i");
	if (ResultVec4f[0] != ValuesVec4f[0] || ResultVec4f[1] != ValuesVec4f[1] || ResultVec4f[2] != ValuesVec4f[2] || ResultVec4f[3] != ValuesVec4f[3]) return false;
	fmt::print("j");
	if (ResultVec4i[0] != ValuesVec4i[0] || ResultVec4i[1] != ValuesVec4i[1] || ResultVec4i[2] != ValuesVec4i[2] || ResultVec4i[3] != ValuesVec4i[3]) return false;
	fmt::print("k");
	NormalBuf.Read(ResultMat4d, 4);
	NormalBuf.Read(ResultMat4f, 4);
	NormalBuf.Read(ResultMat4i, 4);
	if (ResultMat4d[0] != ValuesMat4d[0] || ResultMat4d[1] != ValuesMat4d[1] || ResultMat4d[2] != ValuesMat4d[2] || ResultMat4d[3] != ValuesMat4d[3]) return false;
	fmt::print("l");
	if (ResultMat4f[0] != ValuesMat4f[0] || ResultMat4f[1] != ValuesMat4f[1] || ResultMat4f[2] != ValuesMat4f[2] || ResultMat4f[3] != ValuesMat4f[3]) return false;
	fmt::print("m");
	if (ResultMat4i[0] != ValuesMat4i[0] || ResultMat4i[1] != ValuesMat4i[1] || ResultMat4i[2] != ValuesMat4i[2] || ResultMat4i[3] != ValuesMat4i[3]) return false;
	fmt::print("n");
	NormalBuf.Read(ResultMat3d, 4);
	NormalBuf.Read(ResultMat3f, 4);
	NormalBuf.Read(ResultMat3i, 4);
	if (ResultMat3d[0] != ValuesMat3d[0] || ResultMat3d[1] != ValuesMat3d[1] || ResultMat3d[2] != ValuesMat3d[2] || ResultMat3d[3] != ValuesMat3d[3]) return false;
	fmt::print("o");
	if (ResultMat3f[0] != ValuesMat3f[0] || ResultMat3f[1] != ValuesMat3f[1] || ResultMat3f[2] != ValuesMat3f[2] || ResultMat3f[3] != ValuesMat3f[3]) return false;
	fmt::print("p");
	if (ResultMat3i[0] != ValuesMat3i[0] || ResultMat3i[1] != ValuesMat3i[1] || ResultMat3i[2] != ValuesMat3i[2] || ResultMat3i[3] != ValuesMat3i[3]) return false;
	fmt::print("q");
	NormalBuf.Read(ResultMat2d, 4);
	NormalBuf.Read(ResultMat2f, 4); 
	NormalBuf.Read(ResultMat2i, 4);
	if (ResultMat2d[0] != ValuesMat2d[0] || ResultMat2d[1] != ValuesMat2d[1] || ResultMat2d[2] != ValuesMat2d[2] || ResultMat2d[3] != ValuesMat2d[3]) return false;
	fmt::print("r");
	if (ResultMat2f[0] != ValuesMat2f[0] || ResultMat2f[1] != ValuesMat2f[1] || ResultMat2f[2] != ValuesMat2f[2] || ResultMat2f[3] != ValuesMat2f[3]) return false;
	fmt::print("s");
	if (ResultMat2i[0] != ValuesMat2i[0] || ResultMat2i[1] != ValuesMat2i[1] || ResultMat2i[2] != ValuesMat2i[2] || ResultMat2i[3] != ValuesMat2i[3]) return false;
	fmt::print("!\n");
	if (NormalBuf.ReadUTF((char8_t*)ResultText, sizeof(ResultText)) != sizeof(ValuesText) + 1) return false;
	fmt::print("Testing: {}\n", ResultText);
	if (strcmp(ResultText, ValuesText)!=0) return false;
	fmt::print("Performing Read/Write on Network Buffer!\n");
	LWByteBuffer NetworkBuf(Buffer, sizeof(Buffer), LWByteBuffer::BufferNotOwned | LWByteBuffer::Network);
	NetworkBuf.Write(10);
	NetworkBuf.WriteL<int32_t>(2, 11, 12);
	NetworkBuf.Write(LW_PI);
	NetworkBuf.Write(4, Values);
	NetworkBuf.Write(ValuesVec2d[0]);
	NetworkBuf.Write(ValuesVec2f[0]);
	NetworkBuf.Write(ValuesVec2i[0]);
	NetworkBuf.Write(ValuesVec3d[0]);
	NetworkBuf.Write(ValuesVec3f[0]);
	NetworkBuf.Write(ValuesVec3i[0]);
	NetworkBuf.Write(ValuesVec4d[0]);
	NetworkBuf.Write(ValuesVec4f[0]);
	NetworkBuf.Write(ValuesVec4i[0]);
	NetworkBuf.Write(ValuesMat4d[0]);
	NetworkBuf.Write(ValuesMat4f[0]);
	NetworkBuf.Write(ValuesMat4i[0]);
	NetworkBuf.Write(ValuesMat3d[0]);
	NetworkBuf.Write(ValuesMat3f[0]);
	NetworkBuf.Write(ValuesMat3i[0]);
	NetworkBuf.Write(ValuesMat2d[0]);
	NetworkBuf.Write(ValuesMat2f[0]);
	NetworkBuf.Write(ValuesMat2i[0]);
	NetworkBuf.Write(4, ValuesVec2d);
	NetworkBuf.Write(4, ValuesVec2f);
	NetworkBuf.Write(4, ValuesVec2i);
	NetworkBuf.Write(4, ValuesVec3d);
	NetworkBuf.Write(4, ValuesVec3f);
	NetworkBuf.Write(4, ValuesVec3i);
	NetworkBuf.Write(4, ValuesVec4d);
	NetworkBuf.Write(4, ValuesVec4f);
	NetworkBuf.Write(4, ValuesVec4i);
	NetworkBuf.Write(4, ValuesMat4d);
	NetworkBuf.Write(4, ValuesMat4f);
	NetworkBuf.Write(4, ValuesMat4i);
	NetworkBuf.Write(4, ValuesMat3d);
	NetworkBuf.Write(4, ValuesMat3f);
	NetworkBuf.Write(4, ValuesMat3i);
	NetworkBuf.Write(4, ValuesMat2d);
	NetworkBuf.Write(4, ValuesMat2f);
	NetworkBuf.Write(4, ValuesMat2i);
	NetworkBuf.WriteUTF(LWUTF8Iterator(ValuesText));
	fmt::print("Network wrote to position: {} | {}\n", NetworkBuf.GetPosition(), NetworkBuf.GetBytesWritten());
	NetworkBuf.OffsetPosition(-NetworkBuf.GetPosition());
	fmt::print("A");
	if (NetworkBuf.Read<int32_t>() != 10) return false;
	fmt::print("B");
	if (NetworkBuf.Read<int32_t>() != 11) return false;
	fmt::print("C");
	if (NetworkBuf.Read<int32_t>() != 12) return false;
	fmt::print("D");
	if (NetworkBuf.Read<float>() != LW_PI) return false;
	fmt::print("E");
	if (NetworkBuf.Read<uint64_t>() != (uint64_t)Values[0]) return false;
	fmt::print("F");
	if (NetworkBuf.Read<uint64_t>() != (uint64_t)Values[1]) return false;
	fmt::print("G");
	if (NetworkBuf.Read<uint64_t>() != (uint64_t)Values[2]) return false;
	fmt::print("H");
	if (NetworkBuf.Read<uint64_t>() != (uint64_t)Values[3]) return false;
	fmt::print("I");
	if (NetworkBuf.Read<uint64_t>(16) != (uint64_t)Values[0]) return false;
	fmt::print("J");
	if (NetworkBuf.Read<LWVector2d>() != ValuesVec2d[0]) return false;
	fmt::print("K");
	if (NetworkBuf.Read<LWVector2f>() != ValuesVec2f[0]) return false;
	fmt::print("L");
	if (NetworkBuf.Read<LWVector2i>() != ValuesVec2i[0]) return false;
	fmt::print("M");
	if (NetworkBuf.Read<LWVector3d>() != ValuesVec3d[0]) return false;
	fmt::print("N");
	if (NetworkBuf.Read<LWVector3f>() != ValuesVec3f[0]) return false;
	fmt::print("O");
	if (NetworkBuf.Read<LWVector3i>() != ValuesVec3i[0]) return false;
	fmt::print("P");
	if (NetworkBuf.Read<LWVector4d>() != ValuesVec4d[0]) return false;
	fmt::print("Q");
	if (NetworkBuf.Read<LWVector4f>() != ValuesVec4f[0]) return false;
	fmt::print("R");
	if (NetworkBuf.Read<LWVector4i>() != ValuesVec4i[0]) return false;
	fmt::print("S");
	if (NetworkBuf.Read<LWMatrix4d>() != ValuesMat4d[0]) return false;
	fmt::print("T");
	if (NetworkBuf.Read<LWMatrix4f>() != ValuesMat4f[0]) return false;
	fmt::print("U");
	if (NetworkBuf.Read<LWMatrix4i>() != ValuesMat4i[0]) return false;
	fmt::print("V");
	if (NetworkBuf.Read<LWMatrix3d>() != ValuesMat3d[0]) return false;
	fmt::print("W");
	if (NetworkBuf.Read<LWMatrix3f>() != ValuesMat3f[0]) return false;
	fmt::print("X");
	if (NetworkBuf.Read<LWMatrix3i>() != ValuesMat3i[0]) return false;
	fmt::print("Y");
	if (NetworkBuf.Read<LWMatrix2d>() != ValuesMat2d[0]) return false;
	fmt::print("Z");
	if (NetworkBuf.Read<LWMatrix2f>() != ValuesMat2f[0]) return false;
	fmt::print("a");
	if (NetworkBuf.Read<LWMatrix2i>() != ValuesMat2i[0]) return false;
	fmt::print("b");
	NetworkBuf.Read(ResultVec2d, 4);
	NetworkBuf.Read(ResultVec2f, 4);
	NetworkBuf.Read(ResultVec2i, 4);
	if (ResultVec2d[0] != ValuesVec2d[0] || ResultVec2d[1] != ValuesVec2d[1] || ResultVec2d[2] != ValuesVec2d[2] || ResultVec2d[3] != ValuesVec2d[3]) return false;
	fmt::print("b");
	if (ResultVec2f[0] != ValuesVec2f[0] || ResultVec2f[1] != ValuesVec2f[1] || ResultVec2f[2] != ValuesVec2f[2] || ResultVec2f[3] != ValuesVec2f[3]) return false;
	fmt::print("c");
	if (ResultVec2i[0] != ValuesVec2i[0] || ResultVec2i[1] != ValuesVec2i[1] || ResultVec2i[2] != ValuesVec2i[2] || ResultVec2i[3] != ValuesVec2i[3]) return false;
	fmt::print("d");
	NetworkBuf.Read(ResultVec3d, 4);
	NetworkBuf.Read(ResultVec3f, 4);
	NetworkBuf.Read(ResultVec3i, 4);
	if (ResultVec3d[0] != ValuesVec3d[0] || ResultVec3d[1] != ValuesVec3d[1] || ResultVec3d[2] != ValuesVec3d[2] || ResultVec3d[3] != ValuesVec3d[3]) return false;
	fmt::print("e");
	if (ResultVec3f[0] != ValuesVec3f[0] || ResultVec3f[1] != ValuesVec3f[1] || ResultVec3f[2] != ValuesVec3f[2] || ResultVec3f[3] != ValuesVec3f[3]) return false;
	fmt::print("f");
	if (ResultVec3i[0] != ValuesVec3i[0] || ResultVec3i[1] != ValuesVec3i[1] || ResultVec3i[2] != ValuesVec3i[2] || ResultVec3i[3] != ValuesVec3i[3]) return false;
	fmt::print("g");
	NetworkBuf.Read(ResultVec4d, 4);
	NetworkBuf.Read(ResultVec4f, 4);
	NetworkBuf.Read(ResultVec4i, 4);
	if (ResultVec4d[0] != ValuesVec4d[0] || ResultVec4d[1] != ValuesVec4d[1] || ResultVec4d[2] != ValuesVec4d[2] || ResultVec4d[3] != ValuesVec4d[3]) return false;
	fmt::print("h");
	if (ResultVec4f[0] != ValuesVec4f[0] || ResultVec4f[1] != ValuesVec4f[1] || ResultVec4f[2] != ValuesVec4f[2] || ResultVec4f[3] != ValuesVec4f[3]) return false;
	fmt::print("i");
	if (ResultVec4i[0] != ValuesVec4i[0] || ResultVec4i[1] != ValuesVec4i[1] || ResultVec4i[2] != ValuesVec4i[2] || ResultVec4i[3] != ValuesVec4i[3]) return false;
	fmt::print("j");
	NetworkBuf.Read(ResultMat4d, 4);
	NetworkBuf.Read(ResultMat4f, 4);
	NetworkBuf.Read(ResultMat4i, 4);
	if (ResultMat4d[0] != ValuesMat4d[0] || ResultMat4d[1] != ValuesMat4d[1] || ResultMat4d[2] != ValuesMat4d[2] || ResultMat4d[3] != ValuesMat4d[3]) return false;
	fmt::print("k");
	if (ResultMat4f[0] != ValuesMat4f[0] || ResultMat4f[1] != ValuesMat4f[1] || ResultMat4f[2] != ValuesMat4f[2] || ResultMat4f[3] != ValuesMat4f[3]) return false;
	fmt::print("l");
	if (ResultMat4i[0] != ValuesMat4i[0] || ResultMat4i[1] != ValuesMat4i[1] || ResultMat4i[2] != ValuesMat4i[2] || ResultMat4i[3] != ValuesMat4i[3]) return false;
	fmt::print("m");
	NetworkBuf.Read(ResultMat3d, 4);
	NetworkBuf.Read(ResultMat3f, 4);
	NetworkBuf.Read(ResultMat3i, 4);
	if (ResultMat3d[0] != ValuesMat3d[0] || ResultMat3d[1] != ValuesMat3d[1] || ResultMat3d[2] != ValuesMat3d[2] || ResultMat3d[3] != ValuesMat3d[3]) return false;
	fmt::print("n");
	if (ResultMat3f[0] != ValuesMat3f[0] || ResultMat3f[1] != ValuesMat3f[1] || ResultMat3f[2] != ValuesMat3f[2] || ResultMat3f[3] != ValuesMat3f[3]) return false;
	fmt::print("o");
	if (ResultMat3i[0] != ValuesMat3i[0] || ResultMat3i[1] != ValuesMat3i[1] || ResultMat3i[2] != ValuesMat3i[2] || ResultMat3i[3] != ValuesMat3i[3]) return false;
	fmt::print("p");
	NetworkBuf.Read(ResultMat2d, 4);
	NetworkBuf.Read(ResultMat2f, 4);
	NetworkBuf.Read(ResultMat2i, 4);
	if (ResultMat2d[0] != ValuesMat2d[0] || ResultMat2d[1] != ValuesMat2d[1] || ResultMat2d[2] != ValuesMat2d[2] || ResultMat2d[3] != ValuesMat2d[3]) return false;
	fmt::print("q");
	if (ResultMat2f[0] != ValuesMat2f[0] || ResultMat2f[1] != ValuesMat2f[1] || ResultMat2f[2] != ValuesMat2f[2] || ResultMat2f[3] != ValuesMat2f[3]) return false;
	fmt::print("r");
	if (ResultMat2i[0] != ValuesMat2i[0] || ResultMat2i[1] != ValuesMat2i[1] || ResultMat2i[2] != ValuesMat2i[2] || ResultMat2i[3] != ValuesMat2i[3]) return false;
	fmt::print("!");
	if (NetworkBuf.ReadUTF<char8_t>((char8_t*)ResultText, sizeof(ResultText)) != sizeof(ValuesText)+1) return false;
	fmt::print("\nTesting: {}\n", ResultText);
	if (strcmp(ResultText, ValuesText) != 0) return false;
	fmt::print("LWByteBuffer Success!\n");
	return true;
}

bool PerformLWQuaternionTest(void) {
	fmt::print("Performing LWQuaternion test:\n");
	LWQuaternionf Testf;
	if (!TestEquality("LWQuaternionf<Identity>", Testf, LWQuaternionf(1.0f, 0.0f, 0.0f, 0.0f))) return false;
	Testf *= LWQuaternionf::FromEuler(0.0f, LW_PI, 0.0f);
	if (!TestEquality("LWQuaternionf<Multiply>", Testf, LWQuaternionf(0.0f, 0.0f, 1.0f, 0.0f))) return false;
	if (!TestEquality("LWQuaternionf<Conjugate>", Testf.Conjugate(), LWQuaternionf(0.0f, 0.0f, -1.0f, 0.0f))) return false;
	if (!TestEquality("LWQuaternionf<SLERP>", LWQuaternionf::SLERP(LWQuaternionf(), Testf, 0.5f), LWQuaternionf(0.70710682f, 0.0f, -0.707106769f, 0.0f))) return false;
	if (!TestEquality("LWQuaternionf<NLERP>", LWQuaternionf::NLERP(LWQuaternionf(), Testf, 0.5f), LWQuaternionf(0.70710682f, 0.0f,  0.707106769f, 0.0f))) return false;
	Testf = LWQuaternionf::NLERP(LWQuaternionf(), Testf, 0.5f);
	if (!PerformTest("LWQuaternionf<Length>", std::bind(&LWQuaternionf::Length, &Testf), 1.0f)) return false;
	if (!PerformTest("LWQuaternionf<LengthSq>", std::bind(&LWQuaternionf::LengthSq, &Testf), 1.0f)) return false;
	LWVector4f Pnt4f = LWVector4f(1.0f, 0.0f, 0.0f, 0.0f);
	LWVector3f Pnt3f = LWVector3f(1.0f, 0.0f, 0.0f);
	LWVector2f Pnt2f = LWVector2f(1.0f, 0.0f);
	Pnt4f = Testf.RotatePoint(Pnt4f);
	Pnt3f = Testf.RotatePoint(Pnt3f);
	Pnt2f = Testf.RotatePoint(Pnt2f);
	if (!TestEquality("LWQuaternionf<RotatePoint4>", Pnt4f, LWVector4f(0.0f, 0.0f, -1.0f, 0.0f))) return false;
	if (!TestEquality("LWQuaternionf<RotatePoint3>", Pnt3f, LWVector3f(0.0f, 0.0f, -1.0f))) return false;
	if (!TestEquality("LWQuaternionf<RotatePoint2>", Pnt2f, LWVector2f(0.0f, 0.0f))) return false;
	if (!TestEquality("LWQuaternionf<ToEuler>", Testf.ToEuler(), LWVector3f(0.0f, LW_PI_2, 0.0f))) return false;
	return true;
}

bool PerformLWSQuaternionTest(void) {
	fmt::print("Performing LWSQuaternion test:\n");
	LWSQuaternionf Testf;
	if (!TestEquality("LWSQuaternionf<Identity>", Testf, LWSQuaternionf(1.0f, 0.0f, 0.0f, 0.0f))) return false;
	Testf *= LWSQuaternionf::FromEuler(0.0f, LW_PI, 0.0f);
	if (!TestEquality("LWSQuaternionf<Multiply>", Testf, LWSQuaternionf(0.0f, 0.0f, 1.0f, 0.0f))) return false;
	if (!TestEquality("LWSQuaternionf<Conjugate>", Testf.Conjugate(), LWSQuaternionf(0.0f, 0.0f, -1.0f, 0.0f))) return false;
	if (!TestEquality("LWSQuaternionf<SLERP>", LWSQuaternionf::SLERP(LWSQuaternionf(), Testf, 0.5f), LWSQuaternionf(0.70710682f, 0.0f, -0.707106769f, 0.0f))) return false;
	if (!TestEquality("LWSQuaternionf<NLERP>", LWSQuaternionf::NLERP(LWSQuaternionf(), Testf, 0.5f), LWSQuaternionf(0.70710682f, 0.0f, 0.707106769f, 0.0f))) return false;
	Testf = LWSQuaternionf::NLERP(LWSQuaternionf(), Testf, 0.5f);
	if (!PerformTest("LWSQuaternionf<Length>", std::bind(&LWSQuaternionf::Length, &Testf), 1.0f)) return false;
	if (!PerformTest("LWSQuaternionf<LengthSq>", std::bind(&LWSQuaternionf::LengthSq, &Testf), 1.0f)) return false;
	LWSVector4f Pnt4f = LWSVector4f(1.0f, 0.0f, 0.0f, 0.0f);
	Pnt4f = Testf.RotatePoint(Pnt4f);
	if (!TestEquality("LWSQuaternionf<RotatePoint>", Pnt4f, LWSVector4f(0.0f, 0.0f, -1.0f, 0.0f))) return false;
	if (!TestEquality("LWSQuaternionf<ToEuler>", Testf.ToEuler(), LWSVector4f(0.0f, LW_PI_2, 0.0f, 0.0f))) return false;

	fmt::print("LWSQuaterniond:\n");
	LWSQuaterniond Testd;
	if (!TestEquality("LWSQuaterniond<Identity>", Testd, LWSQuaterniond(1.0, 0.0, 0.0, 0.0))) return false;
	Testd *= LWSQuaterniond::FromEuler(0.0, LW_PI, 0.0);
	if (!TestEquality("LWSQuaterniond<Multiply>", Testd, LWSQuaterniond(0.0, 0.0, 1.0, 0.0))) return false;
	if (!TestEquality("LWSQuaterniond<Conjugate>", Testd.Conjugate(), LWSQuaterniond(0.0, 0.0, -1.0, 0.0))) return false;
	if (!TestEquality("LWSQuaterniond<SLERP>", LWSQuaterniond::SLERP(LWSQuaterniond(), Testd, 0.5), LWSQuaterniond(0.70710682, 0.0, -0.707106769, 0.0))) return false;
	if (!TestEquality("LWSQuaterniond<NLERP>", LWSQuaterniond::NLERP(LWSQuaterniond(), Testd, 0.5), LWSQuaterniond(0.70710682, 0.0, 0.707106769, 0.0))) return false;
	Testd = LWSQuaterniond::NLERP(LWSQuaterniond(), Testd, 0.5f);
	if (!PerformTest("LWSQuaterniond<Length>", std::bind(&LWSQuaterniond::Length, &Testd), 1.0)) return false;
	if (!PerformTest("LWSQuaterniond<LengthSq>", std::bind(&LWSQuaterniond::LengthSq, &Testd), 1.0)) return false;
	LWSVector4d Pnt4d = LWSVector4d(1.0, 0.0, 0.0, 0.0);
	Pnt4d = Testd.RotatePoint(Pnt4d);
	if (!TestEquality("LWSQuaterniond<RotatePoint>", Pnt4d, LWSVector4d(0.0, 0.0, -1.0, 0.0))) return false;
	if (!TestEquality("LWSQuaterniond<ToEuler>", Testd.ToEuler(), LWSVector4d(0.0, LW_PI_2, 0.0, 0.0))) return false;
	return true;
}

bool PerformLWSMatrixTest(void) {
	fmt::print("Performing LWSMatrix test:\n");
	fmt::print("Testing LWSMatrix4f:\n");
	LWSMatrix4f Trans = LWSMatrix4f(LWSVector4f(1.0f, 2.0f, 3.0f, 4.0f), LWSVector4f(5.0f, 6.0f, 7.0f, 8.0f), LWSVector4f(9.0f, 10.0f, 11.0f, 12.0f), LWSVector4f(13.0f, 14.0f, 15.0f, 16.0f));
	if (!TestEquality("LWSMatrix4f<Transpose>", Trans.Transpose(), LWSMatrix4f(LWSVector4f(1.0f, 5.0f, 9.0f, 13.0f), LWSVector4f(2.0f, 6.0f, 10.0f, 14.0f), LWSVector4f(3.0f, 7.0f, 11.0f, 15.0f), LWSVector4f(4.0f, 8.0f, 12.0f, 16.0f)))) return false;
	if (!TestEquality("LWSMatrix4f<Transpose3x3>", Trans.Transpose3x3(), LWSMatrix4f(LWSVector4f(1.0f, 5.0f, 9.0f, 4.0f), LWSVector4f(2.0f, 6.0f, 10.0f, 8.0f), LWSVector4f(3.0f, 7.0f, 11.0f, 12.0f), LWSVector4f(13.0f, 14.0f, 15.0f, 16.0f)))) return false;
	if (!TestEquality("LWSMatrix4f<Transpose2x2>", Trans.Transpose2x2(), LWSMatrix4f(LWSVector4f(1.0f, 5.0f, 3.0f, 4.0f), LWSVector4f(2.0f, 6.0f, 7.0f, 8.0f), LWSVector4f(9.0f, 10.0f, 11.0f, 12.0f), LWSVector4f(13.0f, 14.0f, 15.0f, 16.0f)))) return false;
	if (!TestEquality("LWSMatrix4f<Column0>", Trans.Column(0), LWSVector4f(1.0f, 5.0f, 9.0f, 13.0f))) return false;
	if (!TestEquality("LWSMatrix4f<Column1>", Trans.Column(1), LWSVector4f(2.0f, 6.0f, 10.0f, 14.0f))) return false;
	if (!TestEquality("LWSMatrix4f<Column2>", Trans.Column(2), LWSVector4f(3.0f, 7.0f, 11.0f, 15.0f))) return false;
	if (!TestEquality("LWSMatrix4f<Column3>", Trans.Column(3), LWSVector4f(4.0f, 8.0f, 12.0f, 16.0f))) return false;
	if (!TestEquality("LWSMatrix4f<Row0>", Trans[0], LWSVector4f(1.0f, 2.0f, 3.0f, 4.0f))) return false;
	if (!TestEquality("LWSMatrix4f<Row1>", Trans[1], LWSVector4f(5.0f, 6.0f, 7.0f, 8.0f))) return false;
	if (!TestEquality("LWSMatrix4f<Row2>", Trans[2], LWSVector4f(9.0f, 10.0f, 11.0f, 12.0f))) return false;
	if (!TestEquality("LWSMatrix4f<Row3>", Trans[3], LWSVector4f(13.0f, 14.0f, 15.0f, 16.0f))) return false;

	float S = sinf(LW_PI * 0.25f);
	float C = cosf(LW_PI * 0.25f);
	LWSMatrix4f InvResult = LWSMatrix4f({ 0.70710682f, 0.70710682f, 0.0f, 0.0f }, { -0.70710682f, 0.70710682f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.70710682f, -2.1213204f, -3.0f, 1.0f });
	LWSMatrix4f LookAtResult = LWSMatrix4f({ -0.7071067691f, 0.0f, 0.7071067691f, 0.0f }, { -0.4082482755f, 0.816496551f, -0.4082482755f, 0.0f }, { -0.5773502588f, -0.5773502588f, -0.5773502588f, 0.0f }, { 5.0f, 5.0f, 5.0f, 1.0f });
	LWSMatrix4f OrthoGLResult = LWSMatrix4f({ 0.200000003f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.200000003f, 0.0f, 0.0f }, { 0.0f, 0.0f, -2.0f, 0.0f }, { -1.0f, -1.0f, -1.0f, 1.0f });
	LWSMatrix4f OrthoDXResult = LWSMatrix4f({ 0.200000003f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.200000003f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f, 0.0f }, { -1.0f, -1.0f, 0.0f, 1.0f });
	LWSMatrix4f FrustumResult = LWSMatrix4f({ 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, -0.0f, 0.0f });
	LWSMatrix4f PerspectiveResult = LWSMatrix4f({ 2.682459354f, 0.0f, 0.0f, 0.0f }, { 0.0f, 2.414213419f, 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, -0.0f, 0.0f });
	
	LWSMatrix4f Mat4f = LWSMatrix4f::RotationX(LW_PI*0.25f);
	
	if (!TestEquality("LWSMatrix4f::RotationX", Mat4f, LWSMatrix4f({ 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, C, -S, 0.0f }, { 0.0f, S, C, 0.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }))) return false;
	Mat4f = LWSMatrix4f::RotationY(LW_PI*0.25f);
	if (!TestEquality("LWSMatrix4f::RotationY", Mat4f, LWSMatrix4f({ C, 0.0f, S, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { -S, 0.0f, C, 0.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }))) return false;
	Mat4f = LWSMatrix4f::RotationZ(LW_PI*0.25f);
	LWSVector4f Testf = LWSVector4f(1.0f, 1.0f, 1.0f, 1.0f);
	if (!TestEquality("LWSMatrix4f::RotationZ", Mat4f, LWSMatrix4f({ C, -S, 0.0f, 0.0f }, { S, C, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }))) return false;
	if (!TestEquality("LWSMatrix4f<Identity>", LWSMatrix4f()*LWSMatrix4f(), LWSMatrix4f())) return false;
	if (!TestEquality("LWSMatrix4f<Multiply>", Mat4f*LWSMatrix4f(), Mat4f)) return false;
	if (!TestEquality("LWSMatrix4f<*LWSVector4>", LWSMatrix4f()*Testf, Testf)) return false;
	if (!TestEquality("LWSMatrix4f<*LWSVector4>", Mat4f*Testf, LWSVector4f(Mat4f.AsMat4()*Testf.AsVec4()))) return false;
	Mat4f *= LWSMatrix4f::Translation(LWSVector4f(1.0f, 2.0f, 3.0f, 1.0f));
	if (!TestEquality("LWSMatrix4<TransformInverse>", Mat4f.TransformInverse(), InvResult)) return false;
	if (!TestEquality("LWSMatrix4f<Inverse>", Mat4f.Inverse(), InvResult)) return false;
	Mat4f = LWSMatrix4f::LookAt(LWSVector4f(5.0f, 5.0f, 5.0f, 1.0f), LWVector4f(6.0f, 6.0f, 6.0f, 1.0f), LWSVector4f(0.0f, 1.0f, 0.0f, 0.0f));
	if (!TestEquality("LWSMatrix4f<LookAt>", Mat4f, LookAtResult)) return false;
	Mat4f = LWSMatrix4f::OrthoGL(0.0f, 10.0f, 0.0f, 10.0f, 0.0f, 1.0f);
	if (!TestEquality("LWSMatrix4f<OrthoGL>", Mat4f, OrthoGLResult)) return false;
	Mat4f = LWSMatrix4f::OrthoDX(0.0f, 10.0f, 0.0f, 10.0f, 0.0f, 1.0f);
	if (!TestEquality("LWSMatrix4f<OrthoDX>", Mat4f, OrthoDXResult)) return false;
	Mat4f = LWSMatrix4f::Frustum(0.0f, 10.0f, 0.0f, 10.0f, 0.0f, 1.0f);
	if (!TestEquality("LWSMatrix4f<Frustum>", Mat4f, FrustumResult)) return false;
	Mat4f = LWSMatrix4f::Perspective(LW_PI_4, 0.9f, 0.0f, 1.0f);
	if (!TestEquality("LWSMatrix4f<Perspective>", Mat4f, PerspectiveResult)) return false;
	Mat4f = LWSMatrix4f(LWSQuaternionf::FromEuler(0.0f, LW_PI_2, 0.0f));
	if (!TestEquality("LWSMatrix4f<Quaternion>", Mat4f, LWSMatrix4f({ 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, {-1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }))) return false;
	Mat4f = LWSMatrix4f(LWSVector4f(2.0f, 2.0f, 2.0f, 0.0f), LWSQuaternionf::FromEuler(0.0f, LW_PI_2, 0.0f), LWSVector4f(1.0f, 2.0f, 3.0f, 1.0f));
	if (!TestEquality("LWSMatrix4f<Scale_Quaternion_Pos>", Mat4f, LWSMatrix4f({ 0.0f, 0.0f, 2.0f, 0.0f }, { 0.0f, 2.0f, 0.0f, 0.0f }, { -2.0f, 0.0f, 0.0f, 0.0f }, { 1.0f, 2.0f, 3.0f, 1.0f }))) return false;

	fmt::print("Testing LWSMatrix4d:\n");	
	LWSMatrix4d Transd = LWSMatrix4d(LWSVector4d(1.0, 2.0, 3.0, 4.0), LWSVector4d(5.0, 6.0, 7.0, 8.0), LWSVector4d(9.0, 10.0, 11.0, 12.0), LWSVector4d(13.0, 14.0, 15.0, 16.0));
	if (!TestEquality("LWSMatrix4d<Transpose>", Transd.Transpose(), LWSMatrix4d(LWSVector4d(1.0f, 5.0, 9.0, 13.0), LWSVector4d(2.0, 6.0, 10.0, 14.0), LWSVector4d(3.0, 7.0, 11.0, 15.0), LWSVector4d(4.0, 8.0, 12.0, 16.0)))) return false;
	if (!TestEquality("LWSMatrix4d<Transpose3x3>", Transd.Transpose3x3(), LWSMatrix4d(LWSVector4d(1.0, 5.0, 9.0, 4.0), LWSVector4d(2.0, 6.0, 10.0, 8.0), LWSVector4d(3.0, 7.0, 11.0, 12.0), LWSVector4d(13.0, 14.0, 15.0, 16.0)))) return false;
	if (!TestEquality("LWSMatrix4d<Transpose2x2>", Transd.Transpose2x2(), LWSMatrix4d(LWSVector4d(1.0, 5.0, 3.0, 4.0), LWSVector4d(2.0, 6.0, 7.0, 8.0), LWSVector4d(9.0, 10.0, 11.0, 12.0), LWSVector4d(13.0, 14.0, 15.0, 16.0)))) return false;
	if (!TestEquality("LWSMatrix4d<Column0>", Transd.Column(0), LWSVector4d(1.0, 5.0, 9.0, 13.0))) return false;
	if (!TestEquality("LWSMatrix4d<Column1>", Transd.Column(1), LWSVector4d(2.0, 6.0, 10.0, 14.0))) return false;
	if (!TestEquality("LWSMatrix4d<Column2>", Transd.Column(2), LWSVector4d(3.0, 7.0, 11.0, 15.0))) return false;
	if (!TestEquality("LWSMatrix4d<Column3>", Transd.Column(3), LWSVector4d(4.0, 8.0, 12.0, 16.0))) return false;
	if (!TestEquality("LWSMatrix4d<Row0>", Transd[0], LWSVector4d(1.0, 2.0, 3.0, 4.0))) return false;
	if (!TestEquality("LWSMatrix4d<Row1>", Transd[1], LWSVector4d(5.0, 6.0, 7.0, 8.0))) return false;
	if (!TestEquality("LWSMatrix4d<Row2>", Transd[2], LWSVector4d(9.0, 10.0, 11.0, 12.0))) return false;
	if (!TestEquality("LWSMatrix4d<Row3>", Transd[3], LWSVector4d(13.0, 14.0, 15.0, 16.0))) return false;

	double Sd = sin(LW_PI * 0.25);
	double Cd = cos(LW_PI * 0.25);
	LWSMatrix4d InvResultd = LWSMatrix4d(LWSVector4d(0.70710682, 0.70710682, 0.0, 0.0),LWSVector4d(-0.70710682, 0.70710682, 0.0, 0.0), LWSVector4d(0.0, 0.0, 1.0, 0.0), LWSVector4d(0.70710682, -2.12132044, -3.0, 1.0));
	LWSMatrix4d LookAtResultd = LWSMatrix4d(LWSVector4d(-0.7071067691, 0.0, 0.7071067691, 0.0), LWSVector4d(-0.4082482755, 0.816496551, -0.4082482755, 0.0), LWSVector4d(-0.5773502588, -0.5773502588, -0.5773502588, 0.0), LWSVector4d(5.0, 5.0, 5.0, 1.0));
	LWSMatrix4d OrthoGLResultd = LWSMatrix4d(LWSVector4d(0.200000003, 0.0, 0.0, 0.0), LWSVector4d(0.0, 0.200000003, 0.0, 0.0), LWSVector4d(0.0, 0.0, -2.0, 0.0), LWSVector4d(-1.0, -1.0, -1.0, 1.0 ));
	LWSMatrix4d OrthoDXResultd = LWSMatrix4d(LWSVector4d(0.200000003, 0.0, 0.0, 0.0), LWSVector4d(0.0, 0.200000003, 0.0, 0.0), LWSVector4d(0.0, 0.0, -1.0, 0.0), LWSVector4d(-1.0, -1.0, 0.0, 1.0 ));
	LWSMatrix4d FrustumResultd = LWSMatrix4d(LWSVector4d(0.0, 0.0, 1.0, 0.0), LWSVector4d(0.0, 0.0, 1.0, 0.0), LWSVector4d(0.0, 0.0, -1.0, -1.0), LWSVector4d(0.0, 0.0, -0.0, 0.0 ));
	LWSMatrix4d PerspectiveResultd = LWSMatrix4d(LWSVector4d(2.682459354, 0.0, 0.0, 0.0), LWSVector4d(0.0, 2.414213419, 0.0, 0.0), LWSVector4d(0.0, 0.0, -1.0, -1.0), LWSVector4d(0.0, 0.0, -0.0, 0.0));

	LWSMatrix4d Mat4d = LWSMatrix4d::RotationX(LW_PI * 0.25);

	if (!TestEquality("LWSMatrix4d::RotationX", Mat4d, LWSMatrix4d(LWSVector4d(1.0, 0.0, 0.0, 0.0), LWSVector4d(0.0, Cd, -Sd, 0.0), LWSVector4d(0.0, Sd, Cd, 0.0), LWSVector4d(0.0, 0.0, 0.0, 1.0)))) return false;
	Mat4d = LWSMatrix4d::RotationY(LW_PI * 0.25);
	if (!TestEquality("LWSMatrix4d::RotationY", Mat4d, LWSMatrix4d(LWSVector4d(Cd, 0.0, Sd, 0.0), LWSVector4d(0.0, 1.0, 0.0, 0.0), LWSVector4d(-Sd, 0.0, Cd, 0.0), LWSVector4d(0.0, 0.0, 0.0, 1.0)))) return false;
	Mat4d = LWSMatrix4d::RotationZ(LW_PI * 0.25);
	LWSVector4d Testd = LWSVector4d(1.0, 1.0, 1.0, 1.0);
	if (!TestEquality("LWSMatrix4d::RotationZ", Mat4d, LWSMatrix4d(LWSVector4d(Cd, -Sd, 0.0, 0.0), LWSVector4d(Sd, Cd, 0.0, 0.0), LWSVector4d(0.0, 0.0, 1.0, 0.0), LWSVector4d(0.0, 0.0, 0.0, 1.0)))) return false;
	if (!TestEquality("LWSMatrix4d<Identity>", LWSMatrix4d() * LWSMatrix4d(), LWSMatrix4d())) return false;
	if (!TestEquality("LWSMatrix4d<Multiply>", Mat4d * LWSMatrix4d(), Mat4d)) return false;
	if (!TestEquality("LWSMatrix4d<*LWSVector4>", LWSMatrix4d() * Testd, Testd)) return false;
	if (!TestEquality("LWSMatrix4d<*LWSVector4>", Mat4d * Testd, LWSVector4d(Mat4d.AsMat4() * Testd.AsVec4()))) return false;
	Mat4d *= LWSMatrix4d::Translation(LWSVector4d(1.0f, 2.0, 3.0, 1.0));
	if (!TestEquality("LWSMatrix4<TransformInverse>", Mat4d.TransformInverse(), InvResultd)) return false;
	if (!TestEquality("LWSMatrix4d<Inverse>", Mat4d.Inverse(), InvResultd)) return false;
	Mat4d = LWSMatrix4d::LookAt(LWSVector4d(5.0, 5.0, 5.0, 1.0), LWVector4d(6.0, 6.0, 6.0, 1.0), LWSVector4d(0.0, 1.0, 0.0, 0.0));
	if (!TestEquality("LWSMatrix4d<LookAt>", Mat4d, LookAtResultd)) return false;
	Mat4d = LWSMatrix4d::OrthoGL(0.0, 10.0, 0.0, 10.0, 0.0, 1.0);
	if (!TestEquality("LWSMatrix4d<OrthoGL>", Mat4d, OrthoGLResultd)) return false;
	Mat4d = LWSMatrix4d::OrthoDX(0.0, 10.0, 0.0, 10.0, 0.0, 1.0);
	if (!TestEquality("LWSMatrix4d<OrthoDX>", Mat4d, OrthoDXResultd)) return false;
	Mat4d = LWSMatrix4d::Frustum(0.0, 10.0, 0.0, 10.0, 0.0, 1.0);
	if (!TestEquality("LWSMatrix4d<Frustum>", Mat4d, FrustumResultd)) return false;
	Mat4d = LWSMatrix4d::Perspective(LW_PI_4, 0.9, 0.0, 1.0);
	if (!TestEquality("LWSMatrix4d<Perspective>", Mat4d, PerspectiveResultd)) return false;
	Mat4d = LWSMatrix4d(LWSQuaterniond::FromEuler(0.0, LW_PI_2, 0.0));
	if (!TestEquality("LWSMatrix4d<Quaternion>", Mat4d, LWSMatrix4d(LWSVector4d(0.0, 0.0, 1.0, 0.0), LWSVector4d(0.0, 1.0, 0.0, 0.0 ), LWSVector4d(-1.0, 0.0, 0.0, 0.0), LWSVector4d(0.0, 0.0, 0.0, 1.0 )))) return false;
	Mat4d = LWSMatrix4d(LWSVector4d(2.0, 2.0, 2.0, 0.0), LWSQuaterniond::FromEuler(0.0, LW_PI_2, 0.0), LWSVector4d(1.0, 2.0, 3.0, 1.0));
	if (!TestEquality("LWSMatrix4d<Scale_Quaternion_Pos>", Mat4d, LWSMatrix4d(LWSVector4d(0.0, 0.0, 2.0, 0.0), LWSVector4d(0.0, 2.0, 0.0, 0.0), LWSVector4d(-2.0, 0.0, 0.0, 0.0), LWSVector4d(1.0, 2.0, 3.0, 1.0)))) return false;

	return true;
}

bool PerformLWMatrixTest(void){
	fmt::print("Performing LWMatrix test:\n");
	fmt::print("Testing LWMatrix4:\n");
	LWMatrix4f Trans = LWMatrix4f(LWVector4f(1.0f, 2.0f, 3.0f, 4.0f), LWVector4f(5.0f, 6.0f, 7.0f, 8.0f), LWVector4f(9.0f, 10.0f, 11.0f, 12.0f), LWVector4f(13.0f, 14.0f, 15.0f, 16.0f));
	if (!TestEquality("LWSMatrix4f<Transpose>", Trans.Transpose(), LWMatrix4f(LWVector4f(1.0f, 5.0f, 9.0f, 13.0f), LWVector4f(2.0f, 6.0f, 10.0f, 14.0f), LWVector4f(3.0f, 7.0f, 11.0f, 15.0f), LWVector4f(4.0f, 8.0f, 12.0f, 16.0f)))) return false;
	if (!TestEquality("LWSMatrix4f<Transpose3x3>", Trans.Transpose3x3(), LWMatrix4f(LWVector4f(1.0f, 5.0f, 9.0f, 4.0f), LWVector4f(2.0f, 6.0f, 10.0f, 8.0f), LWVector4f(3.0f, 7.0f, 11.0f, 12.0f), LWVector4f(13.0f, 14.0f, 15.0f, 16.0f)))) return false;
	if (!TestEquality("LWSMatrix4f<Transpose2x2>", Trans.Transpose2x2(), LWMatrix4f(LWVector4f(1.0f, 5.0f, 3.0f, 4.0f), LWVector4f(2.0f, 6.0f, 7.0f, 8.0f), LWVector4f(9.0f, 10.0f, 11.0f, 12.0f), LWVector4f(13.0f, 14.0f, 15.0f, 16.0f)))) return false;
	float S = sinf(LW_PI*0.25f);
	float C = cosf(LW_PI*0.25f);
	LWMatrix4f InvResult = LWMatrix4f({ 0.70710682f, 0.70710682f, 0.0f, 0.0f }, { -0.70710682f, 0.70710682f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.70710682f, -2.1213204f, -3.0f, 1.0f });
	LWMatrix4f Mat4f = LWMatrix4f::RotationX(LW_PI*0.25f);
	if (!TestEquality("LWMatrix4::RotationX", Mat4f, LWMatrix4f({ 1.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, C, -S, 0.0f }, { 0.0f, S, C, 0.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }))) return false;
	Mat4f = LWMatrix4f::RotationY(LW_PI*0.25f);
	if (!TestEquality("LWMatrix4::RotationY", Mat4f, LWMatrix4f({ C, 0.0f, S, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f }, { -S, 0.0f, C, 0.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }))) return false;
	Mat4f = LWMatrix4f::RotationZ(LW_PI*0.25f);
	if (!TestEquality("LWMatrix4::RotationZ", Mat4f, LWMatrix4f({ C, -S, 0.0f, 0.0f }, { S, C, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }))) return false;
	if (!TestEquality("LWMatrix4<Identity>", LWMatrix4f()*LWMatrix4f(), LWMatrix4f())) return false;
	if (!TestEquality("LWMatrix4<Multiply>", Mat4f*LWMatrix4f(), Mat4f)) return false;
	Mat4f *= LWMatrix4f::Translation(1.0f, 2.0f, 3.0f);
	if (!TestEquality("LWMatrix4<TransformInverse>", Mat4f.TransformInverse(), InvResult)) return false;
	if (!TestEquality("LWMatrix4<Inverse>", Mat4f.Inverse(), InvResult)) return false;

	fmt::print("Testing LWMatrix3:\n");
	LWMatrix3f Mat3f = LWMatrix3f::RotationX(LW_PI*0.25f);
	if (!TestEquality("LWMatrix3::RotationX", Mat3f, LWMatrix3f({ 1.0f, 0.0f, 0.0f }, { 0.0f, C, -S }, { 0.0f, S, C }))) return false;
	Mat3f = LWMatrix3f::RotationY(LW_PI*0.25f);
	if (!TestEquality("LWMatrix3::RotationY", Mat3f, LWMatrix3f({ C, 0.0f, S }, { 0.0f, 1.0f, 0.0f }, { -S, 0.0f, C }))) return false;
	Mat3f = LWMatrix3f::RotationZ(LW_PI*0.25f);
	if (!TestEquality("LWMatrix3::RotationZ", Mat3f, LWMatrix3f({ C, -S, 0.0f }, { S, C, 0.0f }, { 0.0f, 0.0f, 1.0f }))) return false;
	if (!TestEquality("LWMatrix3<Identity>", LWMatrix3f()*LWMatrix3f(), LWMatrix3f())) return false;
	if (!TestEquality("LWMatrix3<Multiply>", Mat3f*LWMatrix3f(), Mat3f)) return false;

	fmt::print("Testing LWMatrix2:\n");
	LWMatrix2f Mat2f = LWMatrix2f::Rotation(LW_PI*0.25f);
	if (!TestEquality("LWMatrix2::Rotation", Mat2f, LWMatrix2f({ C, -S }, { S, C }))) return false;
	if (!TestEquality("LWMatrix2<Identity>", LWMatrix2f()*LWMatrix2f(), LWMatrix2f())) return false;
	if (!TestEquality("LWMatrix2<Multiply>", Mat2f*LWMatrix2f(), Mat2f)) return false;

	fmt::print("Finished LWMatrix test.\n");
	return true;
}

//Helper function for building unicode word+new line index's, used after verification of AdvanceX were working.
template<class IterType>
uint32_t OutputUnicodeIndexs(const char8_t *IndexListName, const IterType &First, std::function<void(IterType &Iter)> AdvanceFunc, std::function<uint32_t(const IterType &Iter)> IndexFunc) {
	fmt::print("const uint32_t {}[] = {{", IndexListName);
	uint32_t Count = 0;
	for (IterType Iter = First; !Iter.AtEnd(); AdvanceFunc(Iter), Count++) {
		fmt::print("{} {}", Count ? ", " : "", IndexFunc(Iter));
	}
	fmt::print("}};\n");
	return Count;
};

//Helper function for getting unicode word + new line index's, used after verification of AdvanceX were working.
template<class Type, size_t TextLen, size_t GTextLen>
void OutputTextIndexs(const Type(&Text)[TextLen], const Type (&GText)[GTextLen], uint32_t Token, uint32_t GToken, const LWUnicodeIterator<Type> &SubString, const LWUnicodeIterator<Type> &GSubString, const LWUnicodeIterator<Type> &TokenList, const LWUnicodeIterator<Type> &GTokenList) {
	LWUnicodeIterator<Type> Iter = LWUnicodeIterator<Type>(Text);
	LWUnicodeGraphemeIterator<Type> GIter = LWUnicodeGraphemeIterator<Type>(GText);
	
	OutputUnicodeIndexs<LWUnicodeIterator<Type>>("WordIndexs", Iter.NextWord(true), [](LWUnicodeIterator<Type> &Iter) { Iter.AdvanceWord(); }, [](const LWUnicodeIterator<Type> &Iter)->uint32_t { return Iter.m_Index; });
	OutputUnicodeIndexs<LWUnicodeIterator<Type>>("LineIndexs", Iter.NextLine(true), [](LWUnicodeIterator<Type> &Iter) { Iter.AdvanceLine(); }, [](const LWUnicodeIterator<Type> &Iter)->uint32_t { return Iter.m_Index; });
	OutputUnicodeIndexs<LWUnicodeIterator<Type>>("TokenIndexs", Iter.NextToken(Token, false), [&Token](LWUnicodeIterator<Type> &Iter) { Iter.AdvanceToken(Token); }, [](const LWUnicodeIterator<Type> &Iter)->uint32_t { return Iter.m_Index; });
	OutputUnicodeIndexs<LWUnicodeIterator<Type>>("SubStringIndexs", Iter.NextSubString(SubString, false), [&SubString](LWUnicodeIterator<Type> &Iter) { Iter.AdvanceSubString(SubString); }, [](const LWUnicodeIterator<Type> &Iter)->uint32_t { return Iter.m_Index; });
	OutputUnicodeIndexs<LWUnicodeIterator<Type>>("rSubStringIndexs", Iter.NextrSubString(SubString, false), [&SubString](LWUnicodeIterator<Type> &Iter) { Iter.AdvancerSubString(SubString); }, [](const LWUnicodeIterator<Type> &Iter)->uint32_t { return Iter.m_Index; });
	OutputUnicodeIndexs<LWUnicodeIterator<Type>>("TokenListIndexs", Iter.NextTokens(TokenList, false), [&TokenList](LWUnicodeIterator<Type> &Iter) { Iter.AdvanceTokens(TokenList); }, [](const LWUnicodeIterator<Type> &Iter)->uint32_t { return Iter.m_Index; });

	OutputUnicodeIndexs<LWUnicodeGraphemeIterator<Type>>("GWordIndexs", GIter.NextWord(true), [](LWUnicodeGraphemeIterator<Type> &Iter) { Iter.AdvanceWord(); }, [](const LWUnicodeGraphemeIterator<Type> &Iter)->uint32_t { return Iter.m_Character; });
	OutputUnicodeIndexs<LWUnicodeGraphemeIterator<Type>>("GLineIndexs", GIter.NextLine(true), [](LWUnicodeGraphemeIterator<Type> &Iter) { Iter.AdvanceLine(); }, [](const LWUnicodeGraphemeIterator<Type> &Iter)->uint32_t { return Iter.m_Character; });
	OutputUnicodeIndexs<LWUnicodeGraphemeIterator<Type>>("GTokenIndexs", GIter.NextToken(GToken, false), [&GToken](LWUnicodeGraphemeIterator<Type> &Iter) { Iter.AdvanceToken(GToken); }, [](const LWUnicodeGraphemeIterator<Type> &Iter)->uint32_t { return Iter.m_Character; });
	OutputUnicodeIndexs<LWUnicodeGraphemeIterator<Type>>("GSubStringIndexs", GIter.NextSubString(GSubString, false), [&GSubString](LWUnicodeGraphemeIterator<Type> &Iter) { Iter.AdvanceSubString(GSubString); }, [](const LWUnicodeGraphemeIterator<Type> &Iter)->uint32_t { return Iter.m_Character; });
	OutputUnicodeIndexs<LWUnicodeGraphemeIterator<Type>>("GrSubStringIndexs", GIter.NextrSubString(GSubString, false), [&GSubString](LWUnicodeGraphemeIterator<Type> &Iter) { Iter.AdvancerSubString(GSubString); }, [](const LWUnicodeGraphemeIterator<Type> &Iter)->uint32_t { return Iter.m_Character; });
	OutputUnicodeIndexs<LWUnicodeGraphemeIterator<Type>>("GTokenListIndexs", GIter.NextTokens(GTokenList, false), [&GTokenList](LWUnicodeGraphemeIterator<Type> &Iter) { Iter.AdvanceTokens(GTokenList); }, [](const LWUnicodeGraphemeIterator<Type> &Iter)->uint32_t { return Iter.m_Character; });
	return;
}

//Helper class for UnicodeTest structers.
template<class Type>
struct UTFData {
	Type *m_Data = nullptr;
	uint32_t m_DataLen = 0;

	template<class UTF8Type, class UTF16Type, class UTF32Type, size_t UTF8Len, size_t UTF16Len, size_t UTF32Len>
	UTFData(const UTF8Type (&UTF8)[UTF8Len], const UTF16Type (&UTF16)[UTF16Len], const UTF32Type (&UTF32)[UTF32Len]) : UTFData(UTF8, UTF16) {
		if (sizeof(Type) != 4) return;
		m_Data = (Type*)UTF32;
		m_DataLen = UTF32Len;
	}

	template<class UTF8Type, class UTF16Type, size_t UTF8Len, size_t UTF16Len>
	UTFData(const UTF8Type(&UTF8)[UTF8Len], const UTF16Type(&UTF16)[UTF16Len]) : UTFData(UTF8) {
		if (sizeof(Type) != 2) return;
		m_Data = (Type*)UTF16;
		m_DataLen = UTF16Len;
	}

	template<class UTF8Type, size_t UTF8Len>
	UTFData(const UTF8Type(&UTF8)[UTF8Len]) {
		if (sizeof(Type) != 1) return;
		m_Data = (Type*)UTF8;
		m_DataLen = UTF8Len;
	}

};

template<class Type>
bool PerformLWUnicodeTest(void) {
	LWAllocator_Default DefAlloc;
	fmt::print("Testing LWUTF{}\n", sizeof(Type) * 8);
	const char8_t ValidText8[] = u8"Hello world, Καλημέρα\r\n\r\nκόσμε, コンニチハ\nგთხოვთ Hello 𐌳 сейчас\nต้องรบราฆ่าฟันจนบรรลัย, ሰማይ ᚳᚹᚫᚦ H₂ 𐀁.";
	const char16_t ValidText16[] = u"Hello world, Καλημέρα\r\n\r\nκόσμε, コンニチハ\nგთხოვთ Hello 𐌳 сейчас\nต้องรบราฆ่าฟันจนบรรลัย, ሰማይ ᚳᚹᚫᚦ H₂ 𐀁.";
	const char32_t ValidText32[] = U"Hello world, Καλημέρα\r\n\r\nκόσμε, コンニチハ\nგთხოვთ Hello 𐌳 сейчас\nต้องรบราฆ่าฟันจนบรรลัย, ሰማይ ᚳᚹᚫᚦ H₂ 𐀁.";
	const char8_t ValidGText8[]  = u8"Hello _ ABg̈C가Dक्षिE\nᄀᄀᄀ각ᆨᆨF___g!ABg̈!.";
	const char16_t ValidGText16[] = u"Hello _ ABg̈C가Dक्षिE\nᄀᄀᄀ각ᆨᆨF___g!ABg̈!.";
	const char32_t ValidGText32[] = U"Hello _ ABg̈C가Dक्षिE\nᄀᄀᄀ각ᆨᆨF___g!ABg̈!.";

	const char8_t CompareText8[] = u8"Hello";  const char16_t CompareText16[] = u"Hello";  const char32_t CompareText32[] = U"Hello";
	const char8_t GCompareText8[] = u8"ABg̈"; const char16_t GCompareText16[] = u"ABg̈"; const char32_t GCompareText32[] = U"ABg̈";
	const char8_t TokenList8[] = u8"ewコᚦ"; const char16_t TokenList16[] = u"ewコᚦ"; const char32_t TokenList32[] = U"ewコᚦ";
	const char8_t GTokenList8[] = u8"g̈!E"; const char16_t GTokenList16[] = u"g̈!E"; const char32_t GTokenList32[] = U"g̈!E";
	const uint8_t MalformedTextA8[] = "\xF0\x80\x80"; //Malformed type 1 where buffer size is smaller than the amount of characters.
	const wchar_t MalformedTextA16[] = L"\xD800";
	const char32_t MalformedTextA32[] = U"\x10FFFF"; //Malformed type 3 where codepoint is outside of standards range of codepoints.
	const uint8_t MalformedTextB8[] = "\xC0\x80\x80\x80\x80"; //Malformed type 2 where non leading code units appear where they shouldn't.
	const wchar_t MalformedTextB16[] = L"\xD800\xDC00\xDC00";
	const uint8_t MalformedTextC8[] = "\xF7\xBF\xBF\xBF"; //Malformed type 3 where codepoint is outside of standards range of codepoints.
	const wchar_t MalformedTextC16[] = L"\xDCFF\xD8FF"; //Malformed type 3 where codeunits is inside the 0xD800-0xE000 gap.
	char8_t ValidText8Res[sizeof(ValidText8)];
	char16_t ValidText16Res[sizeof(ValidText16) / sizeof(char16_t)];
	char32_t ValidText32Res[sizeof(ValidText32) / sizeof(char32_t)];
	char8_t ValidGText8Res[sizeof(ValidGText8)];
	char16_t ValidGText16Res[sizeof(ValidGText16)];
	char32_t ValidGText32Res[sizeof(ValidGText32)];

	UTFData<Type> ValidText = UTFData<Type>(ValidText8, ValidText16, ValidText32);
	UTFData<Type> ValidGText = UTFData<Type>(ValidGText8, ValidGText16, ValidGText32);
	UTFData<Type> MalformedTextA = UTFData<Type>(MalformedTextA8, MalformedTextA16, MalformedTextA32);
	UTFData<Type> MalformedTextB = UTFData<Type>(MalformedTextB8, MalformedTextB16);
	UTFData<Type> MalformedTextC = UTFData<Type>(MalformedTextC8, MalformedTextC16);
	UTFData<Type> CompareText = UTFData<Type>(CompareText8, CompareText16, CompareText32);
	UTFData<Type> GCompareText = UTFData<Type>(GCompareText8, GCompareText16, GCompareText32);
	UTFData<Type> TokenList = UTFData<Type>(TokenList8, TokenList16, TokenList32);
	UTFData<Type> GTokenList = UTFData<Type>(GTokenList8, GTokenList16, GTokenList32);
	UTFData<Type> ValidTextRes = UTFData<Type>(ValidText8Res, ValidText16Res, ValidText32Res);
	UTFData<Type> ValidGTextRes = UTFData<Type>(ValidGText8Res, ValidGText16Res, ValidGText32Res);
	//Used OutputTextIndexs to generate these values, if ValidText is modified, be sure to update these values.
	uint32_t Token = ',';
	uint32_t GToken = '_';
	//OutputTextIndexs<char8_t>(ValidText8, ValidGText8, Token, GToken, CompareText8, GCompareText8, TokenList8, GTokenList8);
	//Codepoint Index's:
	const uint32_t WordIndexs[] = { 0,  6,  13,  25,  32,  38,  45,  51,  53,  60,  84,  88,  93,  96 };
	const uint32_t LineIndexs[] = { 0,  23,  25,  38,  60 };
	const uint32_t TokenIndexs[] = { 11,  30,  82 };
	const uint32_t SubStringIndexs[] = { 0,  45 };
	const uint32_t rSubStringIndexs[] = { 4,  49 };
	const uint32_t TokenListIndexs[] = { 1,  6,  32,  46,  91 };

	//Grapheme Index's
	const uint32_t GWordIndexs[] = { 0,  6,  8,  18 };
	const uint32_t GLineIndexs[] = { 0,  18 };
	const uint32_t GTokenIndexs[] = { 6,  22,  23,  24 };
	const uint32_t GSubStringIndexs[] = { 8,  27 };
	const uint32_t GrSubStringIndexs[] = { 10,  29 };
	const uint32_t GTokenListIndexs[] = { 10,  16,  25,  26,  29,  30 };

	uint32_t ValidGCharLen = 32; //Update if GValidText is modified.
	fmt::print("Testing LWUTF{}Iterator.", sizeof(Type) * 8);
	LWUnicodeIterator<Type> SampleIterator;
	LWUnicodeGraphemeIterator<Type> GSampleIterator;
	uint32_t Chars = 0;
	uint32_t Len = 0;
	uint32_t RawLen = 0;
	if (MalformedTextA.m_DataLen && !TestEquality("LWUnicodeIterator::Create<MalformedA>", LWUnicodeIterator<Type>::Create(SampleIterator, MalformedTextA.m_Data, MalformedTextA.m_DataLen, Len, RawLen), false)) return false;
	if (MalformedTextB.m_DataLen && !TestEquality("LWUnicodeIterator::Create<MalformedB>", LWUnicodeIterator<Type>::Create(SampleIterator, MalformedTextB.m_Data, MalformedTextB.m_DataLen, Len, RawLen), false)) return false;
	if (MalformedTextC.m_DataLen && !TestEquality("LWUnicodeIterator::Create<MalformedC>", LWUnicodeIterator<Type>::Create(SampleIterator, MalformedTextC.m_Data, MalformedTextC.m_DataLen, Len, RawLen), false)) return false;
	if (!TestEquality("LWUnicodeIterator::Create<ValidText>", LWUnicodeIterator<Type>::Create(SampleIterator, ValidText.m_Data, ValidText.m_DataLen, Len, RawLen), true)) return false;
	if (!TestEquality("LWUnicodeIterator::RawLen==ValidTextRawLen", RawLen, ValidText.m_DataLen)) return false;
	if (!TestEquality("LWUnicodeIterator::Len==ValidTextLen", Len + 1, (uint32_t)(sizeof(ValidText32) / sizeof(char32_t)))) return false; /*Len+1 to include null char */
	if (!TestEquality("LWUnicodeIterator::MakeUTF8<Null>", SampleIterator.MakeUTF<char8_t>(nullptr, 0), (uint32_t)sizeof(ValidText8))) return false;
	if (!TestEquality("LWUnicodeIterator::MakeUTF8", SampleIterator.MakeUTF<char8_t>(ValidText8Res, sizeof(ValidText8Res)), (uint32_t)sizeof(ValidText8))) return false;
	if (!TestEquality("MakeUTF8==ValidText8", sizeof(ValidText8), ValidText8Res, ValidText8)) return false;
	if (!TestEquality("LWUnicodeIterator::MakeUTF16<Null>", SampleIterator.MakeUTF<char16_t>(nullptr, 0), (uint32_t)(sizeof(ValidText16) / sizeof(char16_t)))) return false;
	if (!TestEquality("LWUnicodeIterator::MakeUTF16", SampleIterator.MakeUTF<char16_t>(ValidText16Res, sizeof(ValidText16Res) / sizeof(char16_t)), (uint32_t)(sizeof(ValidText16) / sizeof(char16_t)))) return false;
	if (!TestEquality("MakeUTF16==ValidText16", sizeof(ValidText16) / sizeof(char16_t), ValidText16Res, ValidText16)) return false;
	if (!TestEquality("LWUnicodeIterator::MakeUTF32<Null>", SampleIterator.MakeUTF<char32_t>(nullptr, 0), (uint32_t)(sizeof(ValidText32) / sizeof(char32_t)))) return false;
	if (!TestEquality("LWUnicodeIterator::MakeUTF32", SampleIterator.MakeUTF<char32_t>(ValidText32Res, sizeof(ValidText32Res) / sizeof(char32_t)), (uint32_t)(sizeof(ValidText32) / sizeof(char32_t)))) return false;
	if (!TestEquality("MakeUTF32==ValidText32", sizeof(ValidText32) / sizeof(char32_t), ValidText32Res, ValidText32)) return false;
	if (!PerformTest("SampleIterator Compare ValidText", [&SampleIterator, &ValidText]()->bool {
		return SampleIterator.Compare(ValidText.m_Data); }, true)) return false;
	fmt::print("Output: '{}'\n", SampleIterator);
	if (!PerformTest("Forward pre increment Iterating test.", [&SampleIterator, &ValidText32](void)->bool {
		LWUnicodeIterator<Type> Iter = SampleIterator;
		for (; !Iter.AtEnd(); ++Iter)
			if (ValidText32[Iter.m_Index] != *Iter || Iter.m_Index != SampleIterator.Distance(Iter) || Iter.RawIndex() != SampleIterator.RawDistance(Iter)) return false;
		return ValidText32[Iter.m_Index] == *Iter; }, true)) return false;
	if (!PerformTest("Forward post increment iterating test.", [&SampleIterator, &ValidText32](void)->bool {
		LWUnicodeIterator<Type> Iter = SampleIterator;
		for (; !Iter.AtEnd(); Iter++)
			if (ValidText32[Iter.m_Index] != *Iter || Iter.m_Index != SampleIterator.Distance(Iter) || Iter.RawIndex() != SampleIterator.RawDistance(Iter)) return false;
		return ValidText32[Iter.m_Index] == *Iter; }, true)) return false;
	if (!PerformTest("Reverse pre decrement Iterating test.", [&SampleIterator, &ValidText32](void)->bool {
		LWUnicodeIterator<Type> Iter = SampleIterator.NextEnd(); //Advance to the end of the iterator.
		for (;; --Iter) {
			if (ValidText32[Iter.m_Index] != *Iter || Iter.m_Index != SampleIterator.Distance(Iter) || Iter.RawIndex() != SampleIterator.RawDistance(Iter)) return false;
			if (Iter.AtStart()) break;
		}
		return true; }, true)) return false;
	if (!PerformTest("Reverse post decrement iterating test.", [&SampleIterator, &ValidText32](void)->bool {
		LWUnicodeIterator<Type> Iter = SampleIterator.NextEnd();
		for (;; Iter--) {
			if (ValidText32[Iter.m_Index] != *Iter || Iter.m_Index != SampleIterator.Distance(Iter) || Iter.RawIndex() != SampleIterator.RawDistance(Iter)) return false;
			if (Iter.AtStart()) break;
		}
		return true; }, true)) return false;
	if (!PerformTest("AdvanceWord iterating test.", [&WordIndexs, &SampleIterator](void)->bool {
		LWUnicodeIterator<Type> Iter = SampleIterator.NextWord(true);
		uint32_t WordCount = sizeof(WordIndexs) / sizeof(uint32_t);
		uint32_t i = 0;
		for (; !Iter.AtEnd(); Iter.AdvanceWord(), i++) {
			if (Iter.m_Index != WordIndexs[i]) return false;
		}
		return i == WordCount; }, true)) return false;
	if (!PerformTest("AdvanceLine iterating test.", [&LineIndexs, &SampleIterator](void)->bool {
		LWUnicodeIterator<Type> Iter = SampleIterator.NextLine(true);
		uint32_t LineCount = sizeof(LineIndexs) / sizeof(uint32_t);
		uint32_t i = 0;
		for (; !Iter.AtEnd(); Iter.AdvanceLine(), i++) {
			if (Iter.m_Index != LineIndexs[i]) return false;
		}
		return i == LineCount; }, true)) return false;
	if (!PerformTest("AdvanceToken iterating test.", [&TokenIndexs, &SampleIterator, &Token](void)->bool {
		LWUnicodeIterator<Type> Iter = SampleIterator.NextToken(Token, false);
		uint32_t TokenCount = sizeof(TokenIndexs) / sizeof(uint32_t);
		uint32_t i = 0;
		for (; !Iter.AtEnd(); Iter.AdvanceToken(Token), i++) {
			if (Iter.m_Index != TokenIndexs[i]) return false;
		}
		return i == TokenCount; }, true)) return false;
	if (!PerformTest("AdvanceSubString iterating test.", [&SubStringIndexs, &SampleIterator, &CompareText](void)->bool {
		LWUnicodeIterator<Type> SubString = LWUnicodeIterator<Type>(CompareText.m_Data, CompareText.m_DataLen);
		LWUnicodeIterator<Type> Iter = SampleIterator.NextSubString(SubString, false);
		uint32_t SubstringCount = sizeof(SubStringIndexs) / sizeof(uint32_t);
		uint32_t i = 0;
		for (; !Iter.AtEnd(); Iter.AdvanceSubString(SubString), i++) {
			if (Iter.m_Index != SubStringIndexs[i]) return false;
		}
		return i == SubstringCount; }, true)) return false;
	if (!PerformTest("AdvancerSubString iterating test.", [&rSubStringIndexs, &SampleIterator, &CompareText](void)->bool {
		LWUnicodeIterator<Type> SubString = LWUnicodeIterator<Type>(CompareText.m_Data, CompareText.m_DataLen);
		LWUnicodeIterator<Type> Iter = SampleIterator.NextrSubString(SubString, false);
		uint32_t rSubstringCount = sizeof(rSubStringIndexs) / sizeof(uint32_t);
		uint32_t i = 0;
		for (; !Iter.AtEnd(); Iter.AdvancerSubString(SubString), i++) {
			if (Iter.m_Index != rSubStringIndexs[i]) return false;
		}
		return i == rSubstringCount; }, true)) return false;
	if (!PerformTest("AdvanceTokens iterating test.", [&TokenListIndexs, &SampleIterator, &TokenList](void)->bool {
		LWUnicodeIterator<Type> TokenIter = LWUnicodeIterator<Type>(TokenList.m_Data, TokenList.m_DataLen);
		LWUnicodeIterator<Type> Iter = SampleIterator.NextTokens(TokenIter, false);
		uint32_t TokenListCount = sizeof(TokenListIndexs) / sizeof(uint32_t);
		uint32_t i = 0;
		for (; !Iter.AtEnd(); Iter.AdvanceTokens(TokenIter), i++) {
			if (Iter.m_Index != TokenListIndexs[i]) return false;
		}
		return i == TokenListCount; }, true)) return false;
	if (!PerformTest("rAdvanceToken iterating test.", [&TokenIndexs, &SampleIterator, &Token](void)->bool {
		LWUnicodeIterator<Type> Iter = SampleIterator.NextEnd().PrevToken(Token, false);
		uint32_t TokenCount = sizeof(TokenIndexs) / sizeof(uint32_t);
		uint32_t i = 0;
		for (;!Iter.AtStart(); Iter.rAdvanceToken(Token)) {
			if (Iter.m_Index != TokenIndexs[TokenCount - ++i]) return false;
		}
		if (Iter.isToken(Token)) {
			if (Iter.m_Index != TokenIndexs[TokenCount - ++i]) return false;
		}
		return i == TokenCount; }, true)) return false;
	if (!PerformTest("rAdvanceWord iterating test.", [&WordIndexs, &SampleIterator](void)->bool {
		LWUnicodeIterator<Type> Iter = SampleIterator.NextEnd().PrevWord(true);
		uint32_t WordCount = sizeof(WordIndexs) / sizeof(uint32_t);
		uint32_t i = 0;
		for (;; Iter.rAdvanceWord()) {
			if (Iter.m_Index != WordIndexs[WordCount - i++ - 1]) return false;
			if (Iter.AtStart()) break;
		}
		return i == WordCount; }, true)) return false;
	if (!PerformTest("rAdvanceLine iterating test.", [&LineIndexs, &SampleIterator](void)->bool {
		LWUnicodeIterator<Type> Iter = SampleIterator.NextEnd().PrevLine(true);
		uint32_t LineCount = sizeof(LineIndexs) / sizeof(uint32_t);
		uint32_t i = 0;
		for (;; Iter.rAdvanceLine()) {
			if (Iter.m_Index != LineIndexs[LineCount - ++i]) return false;
			if (Iter.AtStart()) break;
		}
		return i == LineCount; }, true)) return false;
	if (!PerformTest("rAdvanceSubString iterating test.", [&SubStringIndexs, &SampleIterator, &CompareText]()->bool {
		LWUnicodeIterator<Type> SubString = LWUnicodeIterator<Type>(CompareText.m_Data, CompareText.m_DataLen);
		LWUnicodeIterator<Type> Iter = SampleIterator.NextEnd().PrevSubString(SubString, false);
		uint32_t SubStringCount = sizeof(SubStringIndexs) / sizeof(uint32_t);
		uint32_t i = 0;
		for (;!Iter.AtStart(); Iter.rAdvanceSubString(SubString)) {
			if (Iter.m_Index != SubStringIndexs[SubStringCount - ++i]) return false;
		}
		if (Iter.isSubString(SubString)) {
			if (Iter.m_Index != SubStringIndexs[SubStringCount - ++i]) return false;
		}
		return i == SubStringCount; }, true)) return false;
	if (!PerformTest("rAdvancerSubString iterating test.", [&rSubStringIndexs, &SampleIterator, &CompareText]()->bool {
		LWUnicodeIterator<Type> SubString = LWUnicodeIterator<Type>(CompareText.m_Data, CompareText.m_DataLen);
		LWUnicodeIterator<Type> Iter = SampleIterator.NextEnd().PrevrSubString(SubString, false);
		uint32_t rSubstringCount = sizeof(rSubStringIndexs) / sizeof(uint32_t);
		uint32_t i = 0;
		for (; !Iter.AtStart(); Iter.rAdvancerSubString(SubString)) {
			//Verifying isrSubString is working correctly since rAdvancerSubString/AdvancerSubString doesn't use it directly.
			if (Iter.m_Index != rSubStringIndexs[rSubstringCount - ++i] || !Iter.isrSubString(SubString)) return false;
		}
		if (Iter.isrSubString(SubString)) {
			if (Iter.m_Index != rSubStringIndexs[rSubstringCount - ++i]) return false;
		}
		return i == rSubstringCount; }, true)) return false;
	if (!PerformTest("rAdvanceToken iterating test.", [&TokenListIndexs, &SampleIterator, &TokenList]()->bool {
		LWUnicodeIterator<Type> TokenListIter = LWUnicodeIterator<Type>(TokenList.m_Data, TokenList.m_DataLen);
		LWUnicodeIterator<Type> Iter = SampleIterator.NextEnd().PrevTokens(TokenListIter, false);
		uint32_t TokenListCount = sizeof(TokenListIndexs) / sizeof(uint32_t);
		uint32_t i = 0;
		for (; !Iter.AtStart(); Iter.rAdvanceTokens(TokenListIter)) {
			if (Iter.m_Index != TokenListIndexs[TokenListCount - ++i]) return false;
		}
		if (Iter.isTokens(TokenListIter)) {
			if (Iter.m_Index != TokenListIndexs[TokenListCount - ++i]) return false;
		}
		return i == TokenListCount; }, true)) return false;
	if (!PerformTest("Copy iterator test.", [&SampleIterator, &ValidTextRes, &CompareText]()->bool {
		LWUnicodeIterator<Type> SubString = LWUnicodeIterator<Type>(CompareText.m_Data, CompareText.m_DataLen);
		LWUnicodeIterator<Type> SubStringEnd = SubString.NextEnd();
		LWUnicodeIterator<Type> Iter = SampleIterator.NextSubString(SubString, false);
		for (; !Iter.AtEnd(); Iter.AdvanceSubString(SubString)) {
			if (Iter.Copy(ValidTextRes.m_Data, ValidTextRes.m_DataLen, SubStringEnd.m_Index) != SubStringEnd.RawIndex() + 1) return false;
			LWUnicodeIterator<Type> CopyIter = LWUnicodeIterator<Type>(ValidTextRes.m_Data, ValidTextRes.m_DataLen);
			if (!SubString.Compare(CopyIter)) return false;
			//Copy Backwards:
			if (Iter.Next(SubStringEnd.m_Index).Copy(ValidTextRes.m_Data, ValidTextRes.m_DataLen, Iter) != SubStringEnd.RawIndex() + 1) return false;
			if (!SubString.Compare(CopyIter)) return false;
		}
		return true; }, true)) return false;
	if (!PerformTest("CompareList test.", [&SampleIterator, &ValidText, &CompareText]()->bool {
		LWUnicodeIterator<Type> A = LWUnicodeIterator<Type>((Type*)"Test case that should fail!");
		LWUnicodeIterator<Type> B = LWUnicodeIterator<Type>(ValidText.m_Data);
		uint32_t Index = SampleIterator.CompareList(A, B);
		if (Index != 1) return false;
		Index = SampleIterator.CompareList((Type*)"abc", (Type*)"def");
		if (Index != -1) return false;
		Index = SampleIterator.NextSubString(LWUnicodeIterator<Type>(CompareText.m_Data, CompareText.m_DataLen), false).CompareListn(5, (Type*)"defgj", CompareText.m_Data);
		if (Index != 1) return false;
		return true; }, true)) return false;
	if (!PerformTest("SplitToken test.", [&SampleIterator, &Token, &TokenIndexs]()->bool {
		const uint32_t IterBufferSize = 32;
		LWUnicodeIterator<Type> IterBuffer[IterBufferSize];
		uint32_t TokenCnt = sizeof(TokenIndexs) / sizeof(uint32_t);
		uint32_t Cnt = SampleIterator.SplitToken(IterBuffer, IterBufferSize, Token);
		if (Cnt != TokenCnt + 1) return false;
		for (uint32_t i = 1; i < Cnt; i++) {
			if (SampleIterator.Distance(IterBuffer[i]) != TokenIndexs[i - 1] + 1) return false;
		}
		return true; }, true)) return false;
	if (!PerformTest("SplitTokenList test.", [&SampleIterator, &TokenList, &TokenListIndexs]()->bool {
		const uint32_t IterBufferSize = 32;
		LWUnicodeIterator<Type> IterBuffer[IterBufferSize];
		uint32_t TokenListCnt = sizeof(TokenListIndexs) / sizeof(uint32_t);
		uint32_t Cnt = SampleIterator.SplitTokenList(IterBuffer, IterBufferSize, LWUnicodeIterator<Type>(TokenList.m_Data));
		if (Cnt != TokenListCnt + 1) return false;
		for (uint32_t i = 1; i < Cnt; i++) {
			if (SampleIterator.Distance(IterBuffer[i]) != TokenListIndexs[i - 1] + 1) return false;
		}
		return true; }, true)) return false;
	if (!PerformTest("SplitWords test.", [&SampleIterator, &WordIndexs]()->bool {
		const uint32_t IterBufferSize = 32;
		LWUnicodeIterator<Type> IterBuffer[IterBufferSize];
		uint32_t WordCnt = sizeof(WordIndexs) / sizeof(uint32_t);
		uint32_t Cnt = SampleIterator.SplitWords(IterBuffer, IterBufferSize);
		if (Cnt != WordCnt) return false;
		for (uint32_t i = 0; i < Cnt; i++) {
			if (SampleIterator.Distance(IterBuffer[i]) != WordIndexs[i]) return false;
		}
		return true; }, true)) return false;
	if (!PerformTest("SplitLines test.", [&SampleIterator, &LineIndexs]()->bool {
		const uint32_t IterBufferSize = 32;
		LWUnicodeIterator<Type> IterBuffer[IterBufferSize];
		uint32_t LineCnt = sizeof(LineIndexs) / sizeof(uint32_t);
		uint32_t Cnt = SampleIterator.SplitLine(IterBuffer, IterBufferSize);
		if (Cnt != LineCnt) return false;
		for (uint32_t i = 0; i < Cnt; i++) {
			if (SampleIterator.Distance(IterBuffer[i]) != LineIndexs[i]) return false;
		}
		
		return true; }, true)) return false;
	fmt::print("LWUTF{0}Iterator Test complete, starting test LWUTF{0}GraphemeIterator\n", sizeof(Type) * 8);
	if (MalformedTextA.m_DataLen && !TestEquality("LWUnicodeGraphemeIterator::Create<MalformedA>", LWUnicodeGraphemeIterator<Type>::Create(GSampleIterator, MalformedTextA.m_Data, MalformedTextA.m_DataLen, Chars, Len, RawLen), false)) return false;
	if (MalformedTextB.m_DataLen && !TestEquality("LWUnicodeGraphemeIterator::Create<MalformedB>", LWUnicodeGraphemeIterator<Type>::Create(GSampleIterator, MalformedTextB.m_Data, MalformedTextB.m_DataLen, Chars, Len, RawLen), false)) return false;
	if (MalformedTextC.m_DataLen && !TestEquality("LWUnicodeGraphemeIterator::Create<MalformedC>", LWUnicodeGraphemeIterator<Type>::Create(GSampleIterator, MalformedTextC.m_Data, MalformedTextC.m_DataLen, Chars, Len, RawLen), false)) return false;
	if (!TestEquality("LWUnicodeGraphemeIterator::Create<ValidGText>", LWUnicodeGraphemeIterator<Type>::Create(GSampleIterator, ValidGText.m_Data, ValidGText.m_DataLen, Chars, Len, RawLen), true)) return false;
	if (!TestEquality("LWUnicodeGraphemeIterator::RawLen==ValidGTextRawLen", RawLen, ValidGText.m_DataLen)) return false;
	if (!TestEquality("LWUnicodeGraphemeIterator::Len==ValidGTextLen", Len + 1, (uint32_t)(sizeof(ValidGText32) / sizeof(char32_t)))) return false; /*Len+1 to include null char */
	if (!TestEquality("LWUnicodeGraphemeIterator::Characters==ValidGChar", Chars, ValidGCharLen)) return false;
	if (!PerformTest("GSampleIterator Compare ValidGText", [&GSampleIterator, &ValidGText]()->bool {
		return GSampleIterator.Compare(ValidGText.m_Data); }, true)) return false;
	//fmt::print("Output: '{}'\n", GSampleIterator);
	if (!PerformTest("Forward pre increment Iterating test.", [&GSampleIterator, &ValidGText32](void)->bool {
		LWUnicodeGraphemeIterator<Type> Iter = GSampleIterator;
		for (uint32_t c = 0; !Iter.AtEnd(); ++Iter, ++c)
			if (ValidGText32[Iter.m_Index] != *Iter || Iter.m_Index != GSampleIterator.Distance(Iter) || Iter.RawIndex() != GSampleIterator.RawDistance(Iter) || Iter.m_Character!=c) return false;
		return ValidGText32[Iter.m_Index] == *Iter; }, true)) return false;
	if (!PerformTest("Forward post increment iterating test.", [&GSampleIterator, &ValidGText32](void)->bool {
		LWUnicodeGraphemeIterator<Type> Iter = GSampleIterator;
		for (uint32_t c = 0; !Iter.AtEnd(); Iter++, ++c)
			if (ValidGText32[Iter.m_Index] != *Iter || Iter.m_Index != GSampleIterator.Distance(Iter) || Iter.RawIndex() != GSampleIterator.RawDistance(Iter) || Iter.m_Character!=c) return false;
		return ValidGText32[Iter.m_Index] == *Iter; }, true)) return false;
	if (!PerformTest("Reverse pre decrement Iterating test.", [&GSampleIterator, &ValidGText32](void)->bool {
		LWUnicodeGraphemeIterator<Type> Iter = GSampleIterator.NextEnd(); //Advance to the end of the iterator.
		uint32_t c = Iter.m_Character;
		for (;; --Iter, --c) {
			if (ValidGText32[Iter.m_Index] != *Iter || Iter.m_Index != GSampleIterator.Distance(Iter) || Iter.RawIndex() != GSampleIterator.RawDistance(Iter) || Iter.m_Character!=c) return false;
			if (Iter.AtStart()) break;
		}
		return true; }, true)) return false;
	if (!PerformTest("Reverse post decrement iterating test.", [&GSampleIterator, &ValidGText32](void)->bool {
		LWUnicodeGraphemeIterator<Type> Iter = GSampleIterator.NextEnd();
		uint32_t c = Iter.m_Character;
		for (;; Iter--, --c) {
			if (ValidGText32[Iter.m_Index] != *Iter || Iter.m_Index != GSampleIterator.Distance(Iter) || Iter.RawIndex() != GSampleIterator.RawDistance(Iter) || Iter.m_Character!=c) return false;
			if (Iter.AtStart()) break;
		}
		return true; }, true)) return false;
	if (!PerformTest("AdvanceWord iterating test.", [&GWordIndexs, &GSampleIterator](void)->bool {
		LWUnicodeGraphemeIterator<Type> Iter = GSampleIterator.NextWord(true);
		uint32_t WordCount = sizeof(GWordIndexs) / sizeof(uint32_t);
		uint32_t i = 0;
		for (; !Iter.AtEnd(); Iter.AdvanceWord(), i++) {
			if (Iter.m_Character != GWordIndexs[i]) return false;
		}
		return i == WordCount; }, true)) return false;
	if (!PerformTest("AdvanceLine iterating test.", [&GLineIndexs, &GSampleIterator](void)->bool {
		LWUnicodeGraphemeIterator<Type> Iter = GSampleIterator.NextLine(true);
		uint32_t LineCount = sizeof(GLineIndexs) / sizeof(uint32_t);
		uint32_t i = 0;
		for (; !Iter.AtEnd(); Iter.AdvanceLine(), i++) {
			if (Iter.m_Character != GLineIndexs[i]) return false;
		}
		return i == LineCount; }, true)) return false;
	if (!PerformTest("AdvanceToken iterating test.", [&GTokenIndexs, &GSampleIterator, &GToken](void)->bool {
		LWUnicodeGraphemeIterator<Type> Iter = GSampleIterator.NextToken(GToken, false);
		uint32_t TokenCount = sizeof(GTokenIndexs) / sizeof(uint32_t);
		uint32_t i = 0;
		for (; !Iter.AtEnd(); Iter.AdvanceToken(GToken), i++) {
			if (Iter.m_Character != GTokenIndexs[i]) return false;
		}
		return i == TokenCount; }, true)) return false;
	if (!PerformTest("AdvanceSubString iterating test.", [&GSubStringIndexs, &GSampleIterator, &GCompareText](void)->bool {
		LWUnicodeIterator<Type> SubString = LWUnicodeIterator<Type>(GCompareText.m_Data, GCompareText.m_DataLen);
		LWUnicodeGraphemeIterator<Type> Iter = GSampleIterator.NextSubString(SubString, false);
		uint32_t SubstringCount = sizeof(GSubStringIndexs) / sizeof(uint32_t);
		uint32_t i = 0;
		for (; !Iter.AtEnd(); Iter.AdvanceSubString(SubString), i++) {
			if (Iter.m_Character != GSubStringIndexs[i]) return false;
		}
		return i == SubstringCount; }, true)) return false;
	if (!PerformTest("AdvancerSubString iterating test.", [&GrSubStringIndexs, &GSampleIterator, &GCompareText](void)->bool {
		LWUnicodeIterator<Type> SubString = LWUnicodeIterator<Type>(GCompareText.m_Data, GCompareText.m_DataLen);
		LWUnicodeGraphemeIterator<Type> Iter = GSampleIterator.NextrSubString(SubString, false);
		uint32_t rSubstringCount = sizeof(GrSubStringIndexs) / sizeof(uint32_t);
		uint32_t i = 0;
		for (; !Iter.AtEnd(); Iter.AdvancerSubString(SubString), i++) {
			if (Iter.m_Character != GrSubStringIndexs[i]) return false;
		}
		return i == rSubstringCount; }, true)) return false;
	if (!PerformTest("AdvanceTokens iterating test.", [&GTokenListIndexs, &GSampleIterator, &GTokenList](void)->bool {
		LWUnicodeIterator<Type> TokenIter = LWUnicodeIterator<Type>(GTokenList.m_Data, GTokenList.m_DataLen);
		LWUnicodeGraphemeIterator<Type> Iter = GSampleIterator.NextTokens(TokenIter, false);
		uint32_t TokenListCount = sizeof(GTokenListIndexs) / sizeof(uint32_t);
		uint32_t i = 0;
		for (; !Iter.AtEnd(); Iter.AdvanceTokens(TokenIter), i++) {
			if (Iter.m_Character != GTokenListIndexs[i]) return false;
		}
		return i == TokenListCount; }, true)) return false;
	if (!PerformTest("rAdvanceToken iterating test.", [&GTokenIndexs, &GSampleIterator, &GToken](void)->bool {
		LWUnicodeGraphemeIterator<Type> Iter = GSampleIterator.NextEnd().PrevToken(GToken, false);
		uint32_t TokenCount = sizeof(GTokenIndexs) / sizeof(uint32_t);
		uint32_t i = 0;
		for (; !Iter.AtStart(); Iter.rAdvanceToken(GToken)) {
			if (Iter.m_Character != GTokenIndexs[TokenCount - ++i]) return false;
		}
		if (Iter.isToken(GToken)) {
			if (Iter.m_Character != GTokenIndexs[TokenCount - ++i]) return false;
		}
		return i == TokenCount; }, true)) return false;
	if (!PerformTest("rAdvanceWord iterating test.", [&GWordIndexs, &GSampleIterator](void)->bool {
		LWUnicodeGraphemeIterator<Type> Iter = GSampleIterator.NextEnd().PrevWord(true);
		uint32_t WordCount = sizeof(GWordIndexs) / sizeof(uint32_t);
		uint32_t i = 0;
		for (;; Iter.rAdvanceWord()) {
			if (Iter.m_Character != GWordIndexs[WordCount - i++ - 1]) return false;
			if (Iter.AtStart()) break;
		}
		return i == WordCount; }, true)) return false;
	if (!PerformTest("rAdvanceLine iterating test.", [&GLineIndexs, &GSampleIterator](void)->bool {
		LWUnicodeGraphemeIterator<Type> Iter = GSampleIterator.NextEnd().PrevLine(true);
		uint32_t LineCount = sizeof(GLineIndexs) / sizeof(uint32_t);
		uint32_t i = 0;
		for (;; Iter.rAdvanceLine()) {
			if (Iter.m_Character != GLineIndexs[LineCount - ++i]) return false;
			if (Iter.AtStart()) break;
		}
		return i == LineCount; }, true)) return false;
	if (!PerformTest("rAdvanceSubString iterating test.", [&GSubStringIndexs, &GSampleIterator, &GCompareText]()->bool {
		LWUnicodeIterator<Type> SubString = LWUnicodeIterator<Type>(GCompareText.m_Data, GCompareText.m_DataLen);
		LWUnicodeGraphemeIterator<Type> Iter = GSampleIterator.NextEnd().PrevSubString(SubString, false);
		uint32_t SubStringCount = sizeof(GSubStringIndexs) / sizeof(uint32_t);
		uint32_t i = 0;
		for (; !Iter.AtStart(); Iter.rAdvanceSubString(SubString)) {
			if (Iter.m_Character != GSubStringIndexs[SubStringCount - ++i]) return false;
		}
		if (Iter.isSubString(SubString)) {
			if (Iter.m_Character != GSubStringIndexs[SubStringCount - ++i]) return false;
		}
		return i == SubStringCount; }, true)) return false;
	if (!PerformTest("rAdvancerSubString iterating test.", [&GrSubStringIndexs, &GSampleIterator, &GCompareText]()->bool {
		LWUnicodeIterator<Type> SubString = LWUnicodeIterator<Type>(GCompareText.m_Data, GCompareText.m_DataLen);
		LWUnicodeGraphemeIterator<Type> Iter = GSampleIterator.NextEnd().PrevrSubString(SubString, false);
		uint32_t rSubstringCount = sizeof(GrSubStringIndexs) / sizeof(uint32_t);
		uint32_t i = 0;
		for (; !Iter.AtStart(); Iter.rAdvancerSubString(SubString)) {
			//Verifying isrSubString is working correctly since rAdvancerSubString/AdvancerSubString doesn't use it directly.
			if (Iter.m_Character != GrSubStringIndexs[rSubstringCount - ++i] || !Iter.isrSubString(SubString)) return false;
		}
		if (Iter.isrSubString(SubString)) {
			if (Iter.m_Character != GrSubStringIndexs[rSubstringCount - ++i]) return false;
		}
		return i == rSubstringCount; }, true)) return false;
	if (!PerformTest("rAdvanceToken iterating test.", [&GTokenListIndexs, &GSampleIterator, &GTokenList]()->bool {
		LWUnicodeIterator<Type> TokenListIter = LWUnicodeIterator<Type>(GTokenList.m_Data, GTokenList.m_DataLen);
		LWUnicodeGraphemeIterator<Type> Iter = GSampleIterator.NextEnd().PrevTokens(TokenListIter, false);
		uint32_t TokenListCount = sizeof(GTokenListIndexs) / sizeof(uint32_t);
		uint32_t i = 0;
		for (; !Iter.AtStart(); Iter.rAdvanceTokens(TokenListIter)) {
			if (Iter.m_Character != GTokenListIndexs[TokenListCount - ++i]) return false;
		}
		if (Iter.isTokens(TokenListIter)) {
			if (Iter.m_Character != GTokenListIndexs[TokenListCount - ++i]) return false;
		}
		return i == TokenListCount; }, true)) return false;
	if (!PerformTest("Copy iterator test.", [&GSampleIterator, &ValidGTextRes, &GCompareText]()->bool {
		LWUnicodeIterator<Type> SubString = LWUnicodeIterator<Type>(GCompareText.m_Data, GCompareText.m_DataLen);
		LWUnicodeIterator<Type> SubStringEnd = SubString.NextEnd();
		LWUnicodeIterator<Type> Iter = GSampleIterator.NextSubString(SubString, false);
		for (; !Iter.AtEnd(); Iter.AdvanceSubString(SubString)) {
			if (Iter.Copy(ValidGTextRes.m_Data, ValidGTextRes.m_DataLen, SubStringEnd.m_Index) != SubStringEnd.RawIndex() + 1) return false;
			LWUnicodeGraphemeIterator<Type> CopyIter = LWUnicodeGraphemeIterator<Type>(ValidGTextRes.m_Data, ValidGTextRes.m_DataLen);
			if (!SubString.Compare(CopyIter)) return false;
			//Copy Backwards:
			if (Iter.Next(SubStringEnd.m_Index).Copy(ValidGTextRes.m_Data, ValidGTextRes.m_DataLen, Iter) != SubStringEnd.RawIndex() + 1) return false;
			if (!SubString.Compare(CopyIter)) return false;
		}
		return true; }, true)) return false;
	if (!PerformTest("CompareList test.", [&GSampleIterator, &ValidGText, &GCompareText]()->bool {
		LWUnicodeGraphemeIterator<Type> A = LWUnicodeGraphemeIterator<Type>((Type*)"Test case that should fail!");
		LWUnicodeGraphemeIterator<Type> B = LWUnicodeGraphemeIterator<Type>(ValidGText.m_Data);
		uint32_t Index = GSampleIterator.CompareList(A, B);
		if (Index != 1) return false;
		Index = GSampleIterator.CompareList((Type*)"abc", (Type*)"def");
		if (Index != -1) return false;
		Index = GSampleIterator.NextSubString(LWUnicodeIterator<Type>(GCompareText.m_Data, GCompareText.m_DataLen), false).CompareListn(4, (Type*)"defg", GCompareText.m_Data);
		if (Index != 1) return false;
		return true; }, true)) return false;
	if (!PerformTest("SplitToken test.", [&GSampleIterator, &GToken, &GTokenIndexs]()->bool {
		const uint32_t IterBufferSize = 32;
		LWUnicodeGraphemeIterator<Type> IterBuffer[IterBufferSize];
		uint32_t TokenCnt = sizeof(GTokenIndexs) / sizeof(uint32_t);
		uint32_t Cnt = GSampleIterator.SplitToken(IterBuffer, IterBufferSize, GToken);
		if (Cnt != TokenCnt + 1) return false;
		for (uint32_t i = 1; i < Cnt; i++) {
			if (GSampleIterator.CharacterDistance(IterBuffer[i]) != GTokenIndexs[i - 1] + 1) return false;
		}
		return true; }, true)) return false;
	if (!PerformTest("SplitTokenList test.", [&GSampleIterator, &GTokenList, &GTokenListIndexs]()->bool {
		const uint32_t IterBufferSize = 32;
		LWUnicodeGraphemeIterator<Type> IterBuffer[IterBufferSize];
		uint32_t TokenListCnt = sizeof(GTokenListIndexs) / sizeof(uint32_t);
		uint32_t Cnt = GSampleIterator.SplitTokenList(IterBuffer, IterBufferSize, LWUnicodeIterator<Type>(GTokenList.m_Data));
		if (Cnt != TokenListCnt + 1) return false;
		for (uint32_t i = 1; i < Cnt; i++) {
			if (GSampleIterator.CharacterDistance(IterBuffer[i]) != GTokenListIndexs[i - 1] + 1) return false;
		}
		return true; }, true)) return false;

	if (!PerformTest("SplitWords test.", [&GSampleIterator, &GWordIndexs]()->bool {
		const uint32_t IterBufferSize = 32;
		LWUnicodeGraphemeIterator<Type> IterBuffer[IterBufferSize];
		uint32_t WordCnt = sizeof(GWordIndexs) / sizeof(uint32_t);
		uint32_t Cnt = GSampleIterator.SplitWords(IterBuffer, IterBufferSize);
		if (Cnt != WordCnt) return false;
		for (uint32_t i = 0; i < Cnt; i++) {
			if (GSampleIterator.CharacterDistance(IterBuffer[i]) != GWordIndexs[i]) return false;
		}
		return true; }, true)) return false;
	if (!PerformTest("SplitLines test.", [&GSampleIterator, &GLineIndexs]()->bool {
		const uint32_t IterBufferSize = 32;
		LWUnicodeGraphemeIterator<Type> IterBuffer[IterBufferSize];
		uint32_t LineCnt = sizeof(GLineIndexs) / sizeof(uint32_t);
		uint32_t Cnt = GSampleIterator.SplitLine(IterBuffer, IterBufferSize);
		if (Cnt != LineCnt) return false;
		for (uint32_t i = 0; i < Cnt; i++) {
			if (GSampleIterator.CharacterDistance(IterBuffer[i]) != GLineIndexs[i]) return false;
		}
		return true; }, true)) return false;
	fmt::print("LWUTF{0}GraphemeIterator test complete, start test LWUTF{0}:\n", sizeof(Type) * 8);
	if (!PerformTest("Create String.", [&SampleIterator, &ValidText, &DefAlloc]()->bool {
		LWUnicode<Type> U;
		if (!LWUnicode<Type>::Create(U, ValidText.m_Data, ValidText.m_DataLen, DefAlloc)) return false;
		if (U != SampleIterator) return false;
		return true; }, true)) return false;
	if (!TestEquality("MemoryLeakCheck", DefAlloc.GetAllocatedBytes(), 0u)) return false;
	if (!PerformTest("Edit String checks.", [&SampleIterator, &ValidText, &DefAlloc]()->bool {
		LWUnicode<Type> U;
		if (!LWUnicode<Type>::Create(U, ValidText.m_Data, ValidText.m_DataLen, DefAlloc)) return false;
		uint32_t Len = U.Length();
		U.Erase(U[0], U[5]);
		if (U != SampleIterator.Next(5)) return false;
		U.Insert(U[0], SampleIterator, SampleIterator.Next(5));
		if (U != SampleIterator) return false;
		LWUnicodeIterator<Type> Pos = U.Insert(U.end(), SampleIterator);
		if (!U.Compare(SampleIterator, Len)) return false;
		if (!Pos.Compare(SampleIterator, Len)) return false;
		return true; }, true)) return false;
	if (!TestEquality("MemoryLeakCheck", DefAlloc.GetAllocatedBytes(), 0u)) return false;
	fmt::print("LWUTF{} test complate.\n", sizeof(Type) * 8);
	return true;
}

bool PerformLWTimerTest(void){
	fmt::print("Testing LWTimer:\nResolution: {}\nTesting 5 second delay:\n", LWTimer::GetResolution());
	uint64_t FirstHR = LWTimer::GetCurrent();
	uint64_t LastHR = FirstHR;
	uint32_t FirstClk = clock();
	for (; (LastHR - FirstHR) < LWTimer::GetResolution() * 5;) LastHR = LWTimer::GetCurrent();
	
	uint32_t LastClk = clock();
	//Add a bit of margin as Clock will not be exactly on the nose.
	if (LastClk - FirstClk>CLOCKS_PER_SEC * 5 + CLOCKS_PER_SEC / 4 || LastClk-FirstClk<CLOCKS_PER_SEC*5-CLOCKS_PER_SEC/4){
		fmt::print("Error with 5 second delay, took: {} While high-frequency timer reported: {}\n", (LastClk - FirstClk), (LastHR - FirstHR));
		return false;
	}
	fmt::print("Delay Time: {}\n", (LastHR - FirstHR));
	LWTimer Timer(LWTimer::GetResolution() / 60, LWTimer::Running);
	fmt::print("Timer Frequency; {}\nTesting 5 second ticks:\n", Timer.GetFrequency());
	FirstHR = LWTimer::GetCurrent();
	LastHR = FirstHR;
	uint32_t Count = 0;
	for (; (LastHR - FirstHR) < LWTimer::GetResolution() * 5; LastHR = LWTimer::GetCurrent()){
		if(Timer.Update(LastHR).isCompleted()) {
			Count++;
			Timer.Reset();
		}
	}
	uint32_t CountDiff = Count > 300 ? Count - 300 : 300 - Count;
	if (CountDiff>1){ //Depending on the os timing slices, it's possible that between 299-301 ticks could have passed, anything beyond that window is considered a fail.
		fmt::print("Error with timing, expected 300 ticks to have passed, instead {} passed.\n", Count);
		fmt::print("Time passed: {}\n", LastHR - FirstHR);
		return false;
	}
	fmt::print("Time passed: {}\n", LastHR - FirstHR);
	fmt::print("LWTimer successfully tested!\n");
	return true;
}

bool PerformLWConcurrentTest(void){
	fmt::print("Beginning Concurrency algorithmn tests.\n");
	fmt::print("Beginning FIFO tests:\n");
	LWConcurrentFIFO<int, 20> Fifo;
	int Count = 0;
	std::thread PushThread([&Fifo](){for (int i = 0; i < 100; i++) while(!Fifo.Push(i)); });
	std::thread ReadThread([&Fifo, &Count](){ 
		int Value = 0; 
		for (int i = 0; i < 100; i++){
			while (!Fifo.Pop(Value));
			Count += Value;
		}
	});
	PushThread.join();
	ReadThread.join();
	if (Count != 4950) return false;
	fmt::print("Finished FIFO Test.\n");
	fmt::print("Finished Concurrent test.\n");
	return true;
}

bool PerformLWCryptoTest(void){
	fmt::print("Beginning LWCrypto test:\n");
	char BufferA[256];
	char BufferB[256];
	const uint32_t EncodeTests = 5;
	char Inputs[][32] = { "any carnal pleasure.", "any carnal pleasure", "any carnal pleasur", "any carnal pleasu", "any carnal pleas" };
	char Outputs[][32] = {"YW55IGNhcm5hbCBwbGVhc3VyZS4=", "YW55IGNhcm5hbCBwbGVhc3VyZQ==", "YW55IGNhcm5hbCBwbGVhc3Vy", "YW55IGNhcm5hbCBwbGVhc3U=", "YW55IGNhcm5hbCBwbGVhcw==" };
	
	for (uint32_t i = 0; i < EncodeTests;i++){
		uint32_t Len = LWCrypto::Base64Encode(Inputs[i], (uint32_t)strlen(Inputs[i]), BufferA, sizeof(BufferA));
		BufferA[Len] = '\0';
		if(!LWUnicodeIterator<char8_t>((char8_t*)BufferA, sizeof(BufferA)).Compare((char8_t*)Outputs[i])) {
			fmt::print("Error with encoding base64: '{}' Received: '{}' Instead of: '{}'\n", Inputs[i], BufferA, Outputs[i]);
			return false;
		} else fmt::print("Encoded: '{}' To: '{}'\n", Inputs[i], BufferA);
		Len = LWCrypto::Base64Decode(BufferA, Len, BufferB, sizeof(BufferB));
		BufferB[Len] = '\0';
		if (!LWUnicodeIterator<char8_t>((char8_t*)BufferB, sizeof(BufferB)).Compare((char8_t*)Inputs[i])) {
			fmt::print("Error with decoding base64: '{}' Received: '{}' Instead of: '{}'\n", BufferA, BufferB, Inputs[i]);
			return false;
		} else fmt::print("Decoded: '{}' To: '{}'\n", BufferA, BufferB);
	}
	fmt::print("Beginning MD5 Hash tests:\n");
	const uint32_t HashCount = 3;
	char HashInputs[][256] = { "", "The quick brown fox jumps over the lazy dog", "The quick brown fox jumps over the lazy dog The quick brown fox jumps over the lazy dog" };
	uint32_t MD5Outputs[] = { 0xd41d8cd9, 0x8f00b204, 0xe9800998, 0xecf8427e, 0x9e107d9d, 0x372bb682, 0x6bd81d35, 0x42a419d6, 0x038aee51, 0x276c48bf, 0x27db1229, 0x9909ae88 };
	uint32_t SHA1Outputs[] = { 0xda39a3ee, 0x5e6b4b0d, 0x3255bfef, 0x95601890, 0xafd80709, 0x2fd4e1c6, 0x7a2d28fc, 0xed849ee1, 0xbb76e739, 0x1b93eb12, 0xd2f76f0b, 0x044c24c7, 0x941d42d2, 0xeddcb34a, 0xb54e1ca0 };
	for (uint32_t i = 0; i < HashCount;i++){
		LWCrypto::HashMD5(HashInputs[i], (uint32_t)strlen(HashInputs[i]), BufferA);
		if (MD5Outputs[i * 4] != *(uint32_t*)BufferA || MD5Outputs[i * 4 + 1] != *(((uint32_t*)BufferA) + 1) || MD5Outputs[i * 4 + 2] != *(((uint32_t*)BufferA) + 2) || MD5Outputs[i * 4 + 3] != *(((uint32_t*)BufferA) + 3)) {
			fmt::print("Hash incorrect: '{}' Expected: '{:x}{:x}{:x}{:x}' Got: '{:x}{:x}{:x}{:x}'\n", HashInputs[i], MD5Outputs[i * 4], MD5Outputs[i * 4 + 1], MD5Outputs[i * 4 + 2], MD5Outputs[i * 4 + 3], BufferA[0], ((uint32_t*)BufferA)[1], ((uint32_t*)BufferA)[2], ((uint32_t*)BufferA)[3]);
			return false;
		} else fmt::print("Hashed: '{}' Got: '{:x}{:x}{:x}{:x}'\n", HashInputs[i], ((uint32_t*)BufferA)[0], ((uint32_t*)BufferA)[1], ((uint32_t*)BufferA)[2], ((uint32_t*)BufferA)[3]);
	}
	fmt::print("MD5 hash passed!\n");
	fmt::print("Beginning SHA-1 Hash tests:\n");
	for (uint32_t i = 0; i < HashCount; i++){
		LWCrypto::HashSHA1(HashInputs[i], (uint32_t)strlen(HashInputs[i]), BufferA);
		if (SHA1Outputs[i * 5] != *(uint32_t*)BufferA || SHA1Outputs[i * 5 + 1] != *(((uint32_t*)BufferA) + 1) || SHA1Outputs[i * 5 + 2] != *(((uint32_t*)BufferA) + 2) || SHA1Outputs[i * 5 + 3] != *(((uint32_t*)BufferA) + 3) || SHA1Outputs[i * 5 + 4] != *(((uint32_t*)BufferA) + 4)) {
			fmt::print("Hash incorrect: '{}' Expected: '{:x}{:x}{:x}{:x}{:x}' Got: '{:x}{:x}{:x}{:x}{:x}'\n", HashInputs[i], SHA1Outputs[i * 5], SHA1Outputs[i * 5 + 1], SHA1Outputs[i * 5 + 2], SHA1Outputs[i * 5 + 3], SHA1Outputs[i * 5 + 4], ((uint32_t*)BufferA)[0], ((uint32_t*)BufferA)[1], ((uint32_t*)BufferA)[2], ((uint32_t*)BufferA)[3], ((uint32_t*)BufferA)[4]);
			return false;
		} else fmt::print("Hashed: '{}' Got: '{:x}{:x}{:x}{:x}{:x}\n", HashInputs[i], ((uint32_t*)BufferA)[0], ((uint32_t*)BufferA)[1], ((uint32_t*)BufferA)[2], ((uint32_t*)BufferA)[3], ((uint32_t*)BufferA)[4]);
	}
	fmt::print("Successfully finished LWCrypto test.\n");
	return true;
}

template<class Type, class Callback>
uint64_t DoWork(uint32_t PartCnt, uint32_t *Count, Type** Array, Callback CB) {
	uint64_t Start = LWTimer::GetCurrent();
	uint32_t k = 0;
	for (uint32_t i = 0; i < PartCnt; i++) {
		uint32_t Len = Count[i];
		for (uint32_t n = 0; n < Len; n++, k++) CB(Array[i][n], k);
	}
	return LWTimer::ToMilliSecond(LWTimer::GetCurrent() - Start);
}

template<class TypeA, class TypeB>
uint32_t GeneratePartitians(uint32_t Count, uint32_t *&PartitionLens, TypeA **&AArray, TypeB **&BArray, LWAllocator &Allocator) {
	const uint32_t AllocatorSafeHeaderSize = 128; //Safety margin for header.
	uint32_t Max = (std::numeric_limits<uint32_t>::max()-AllocatorSafeHeaderSize)/std::max<uint32_t>(sizeof(TypeA), sizeof(TypeB));
	uint32_t PartCnt = (Count/Max);
	if ((Count % Max) != 0) PartCnt++;
	PartitionLens = Allocator.Allocate<uint32_t>(PartCnt);
	AArray = Allocator.Allocate<TypeA*>(PartCnt);
	BArray = Allocator.Allocate<TypeB*>(PartCnt);
	for (uint32_t i = 0; i < PartCnt; i++) {
		PartitionLens[i] = std::min<uint32_t>(Count - i * Max, Max);
		AArray[i] = Allocator.Allocate<TypeA>(PartitionLens[i]);
		BArray[i] = Allocator.Allocate<TypeB>(PartitionLens[i]);
	}
	return PartCnt;
}

template<class TypeA, class TypeB>
void FreePartitions(uint32_t PartitionCount, uint32_t *PartitionLens, TypeA **AArray, TypeB **BArray) {
	for (uint32_t i = 0; i < PartitionCount; i++) {
		LWAllocator::Destroy(AArray[i]);
		LWAllocator::Destroy(BArray[i]);
	}
	LWAllocator::Destroy(AArray);
	LWAllocator::Destroy(BArray);
	LWAllocator::Destroy(PartitionLens);
};

template<class Type>
bool PerformSIMDVec4Test(uint32_t Count, const LWUTF8Iterator &Name, LWAllocator &Allocator) {
	const uint32_t ColumnSize = 16;
	const uint32_t LineSize = ColumnSize * 3 + 4;
	const char Border = '|';
	fmt::print("Performing {} Test:\n", Name);
	fmt::print("Allocating data: {}\n", (sizeof(LWVector4<Type>) * Count + sizeof(LWSVector4<Type>) * Count));
	uint32_t *PartitionLens = nullptr;
	LWVector4<Type> **Vec4 = nullptr;
	LWSVector4<Type> **SVec4 = nullptr;
	uint32_t PartCnt = GeneratePartitians(Count, PartitionLens, Vec4, SVec4, Allocator);
	Type FinalLen = 0;
	Type SFinalLen = 0;

	fmt::print("{:-<{}}\n", "", LineSize);
	fmt::print("{0}{2:^{1}}{0}{3:^{1}}{0}{4:^{1}}{0}\n", Border, ColumnSize, "Function", "Generic Time", "SIMD Time");

	fmt::print("{:-<{}}\n", "", LineSize);
	fmt::print("{0}{2:^{1}}{0}", Border, ColumnSize, "Initialize");
	fmt::print("{2:^{1}}{0}", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, Vec4, [](LWVector4<Type> &Vec, uint32_t i) { Type Val = i & 1 ? -(Type)i : (Type)i;  Vec = LWVector4<Type>(Val); })));
	if (!SVec4) fmt::print("NoArrays");
	if (!SVec4[0]) fmt::print("NoSubArrays");
	fmt::print("{2:^{1}}{0}\n", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, SVec4, [](LWSVector4<Type> &Vec, uint32_t i) { Type Val = i & 1 ? -(Type)i : (Type)i;  Vec = LWSVector4<Type>(Val); })));

	fmt::print("{:-<{}}\n", "", LineSize);
	fmt::print("{0}{2:^{1}}{0}", Border, ColumnSize, "Length");
	fmt::print("{2:^{1}}{0}", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, Vec4, [&FinalLen](LWVector4<Type> &Vec, uint32_t i) { FinalLen += Vec.Length(); })));
	fmt::print("{2:^{1}}{0}\n", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, SVec4, [&SFinalLen](LWSVector4<Type> &Vec, uint32_t i) { SFinalLen += Vec.Length(); })));

	fmt::print("{:-<{}}\n", "", LineSize);
	fmt::print("{0}{2:^{1}}{0}", Border, ColumnSize, "LengthSq");
	fmt::print("{2:^{1}}{0}", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, Vec4, [&FinalLen](LWVector4<Type> &Vec, uint32_t i) { FinalLen += Vec.LengthSquared(); })));
	fmt::print("{2:^{1}}{0}\n", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, SVec4, [&SFinalLen](LWSVector4<Type> &Vec, uint32_t i) { SFinalLen += Vec.LengthSquared(); })));

	fmt::print("{:-<{}}\n", "", LineSize);
	fmt::print("{0}{2:^{1}}{0}", Border, ColumnSize, "Normalize");
	fmt::print("{2:^{1}}{0}", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, Vec4, [](LWVector4<Type> &Vec, uint32_t i) { Vec = Vec.Normalize(); })));
	fmt::print("{2:^{1}}{0}\n", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, SVec4, [](LWSVector4<Type> &Vec, uint32_t i) { Vec = Vec.Normalize(); })));

	fmt::print("{:-<{}}\n", "", LineSize);
	fmt::print("{0}{2:^{1}}{0}", Border, ColumnSize, "Add");
	fmt::print("{2:^{1}}{0}", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, Vec4, [](LWVector4<Type> &Vec, uint32_t i) { Vec += Vec; })));
	fmt::print("{2:^{1}}{0}\n", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, SVec4, [](LWSVector4<Type> &Vec, uint32_t i) { Vec *= Vec; })));

	fmt::print("{:-<{}}\n", "", LineSize);
	fmt::print("{0}{2:^{1}}{0}", Border, ColumnSize, "Multiply");
	fmt::print("{2:^{1}}{0}", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, Vec4, [](LWVector4<Type> &Vec, uint32_t i) {Vec *= Vec; })));
	fmt::print("{2:^{1}}{0}\n", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, SVec4, [](LWSVector4<Type> &Vec, uint32_t i) { Vec *= Vec; })));

	fmt::print("{:-<{}}\n", "", LineSize);
	fmt::print("{0}{2:^{1}}{0}", Border, ColumnSize, "Sub");
	fmt::print("{2:^{1}}{0}", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, Vec4, [](LWVector4<Type> &Vec, uint32_t i) { Vec -= LWVector4<Type>(1); })));
	fmt::print("{2:^{1}}{0}\n", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, SVec4, [](LWSVector4<Type> &Vec, uint32_t i) { Vec -= LWSVector4<Type>(1); })));

	fmt::print("{:-<{}}\n", "", LineSize);
	fmt::print("{0}{2:^{1}}{0}", Border, ColumnSize, "Divide");
	fmt::print("{2:^{1}}{0}", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, Vec4, [](LWVector4<Type> &Vec, uint32_t i) { Vec /= Vec; })));
	fmt::print("{2:^{1}}{0}\n", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, SVec4, [](LWSVector4<Type> &Vec, uint32_t i) { Vec /= Vec; })));
	fmt::print("{:-<{}}\n", "", LineSize);

	FreePartitions(PartCnt, PartitionLens, Vec4, SVec4);
	if (std::is_same<Type, int32_t>()) return true; //Int32_t at extreme values can have incorrect Length Results.
	else {
		fmt::print("{0}{2:^{1}}{0}", Border, ColumnSize, "Sanitized");
		fmt::print("{2:^{1}.5e}{0}", Border, ColumnSize, FinalLen);
		fmt::print("{2:^{1}.5e}{0}\n", Border, ColumnSize, SFinalLen);
		fmt::print("{:-<{}}\n", "", LineSize);
	}
	return FinalLen==SFinalLen;
}

template<class Type>
bool PerformSIMDMat4Test(uint32_t Count, const LWUTF8Iterator &Name, LWAllocator &Allocator) {
	const uint32_t ColumnSize = 32;
	const uint32_t LineSize = ColumnSize*3+4;
	const char Border = '|';
	fmt::print("Performing {} Test:\n", Name);
	fmt::print("Allocating data: {}.\n", (sizeof(LWMatrix4<Type>)*Count + sizeof(LWSMatrix4<Type>)*Count));
	uint32_t *PartitionLens = nullptr;
	LWMatrix4<Type> **Mat4 = nullptr;
	LWSMatrix4<Type> **SMat4 = nullptr;
	uint32_t PartCnt = GeneratePartitians(Count, PartitionLens, Mat4, SMat4, Allocator);
	LWVector4<Type> FinalSum;
	LWSVector4<Type> SFinalSum;
	fmt::print("{:-<{}}\n", "", LineSize);
	fmt::print("{0}{2:^{1}}{0}{3:^{1}}{0}{4:^{1}}{0}\n", Border, ColumnSize, "Function", "Generic Time", "SIMD Time");

	fmt::print("{:-<{}}\n", "", LineSize);
	fmt::print("{0}{2:^{1}}{0}", Border, ColumnSize, "Initialize");
	fmt::print("{2:^{1}}{0}", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, Mat4, [](LWMatrix4<Type> &Mat, uint32_t i) { Type Pitch = -(Type)i * (Type)0.2; Type Yaw = (Type)i * (Type)0.1; Type Roll = (Type)i; Mat = LWMatrix4<Type>::RotationXYZ(Pitch, Yaw, Roll); })));
	fmt::print("{2:^{1}}{0}\n", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, SMat4, [](LWSMatrix4<Type> &Mat, uint32_t i) { Type Pitch = -(Type)i * (Type)0.2; Type Yaw = (Type)i * (Type)0.1; Type Roll = (Type)i; Mat = LWSMatrix4<Type>::RotationXYZ(Pitch, Yaw, Roll); })));

	fmt::print("{:-<{}}\n", "", LineSize);
	fmt::print("{0}{2:^{1}}{0}", Border, ColumnSize, "Transpose");
	fmt::print("{2:^{1}}{0}", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, Mat4, [](LWMatrix4<Type> &Mat, uint32_t i) { Mat = Mat.Transpose(); })));
	fmt::print("{2:^{1}}{0}\n", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, SMat4, [](LWSMatrix4<Type> &Mat, uint32_t i) { Mat = Mat.Transpose(); })));

	fmt::print("{:-<{}}\n", "", LineSize);
	fmt::print("{0}{2:^{1}}{0}", Border, ColumnSize, "TransformInverse");
	fmt::print("{2:^{1}}{0}", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, Mat4, [](LWMatrix4<Type> &Mat, uint32_t i) { Mat = Mat.TransformInverse(); })));
	fmt::print("{2:^{1}}{0}\n", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, SMat4, [](LWSMatrix4<Type> &Mat, uint32_t i) { Mat = Mat.TransformInverse(); })));

	fmt::print("{:-<{}}\n", "", LineSize);
	fmt::print("{0}{2:^{1}}{0}", Border, ColumnSize, "Inverse");
	fmt::print("{2:^{1}}{0}", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, Mat4, [](LWMatrix4<Type> &Mat, uint32_t i) { Mat = Mat.Inverse(); })));
	fmt::print("{2:^{1}}{0}\n", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, SMat4, [](LWSMatrix4<Type> &Mat, uint32_t i) { Mat = Mat.Inverse(); })));

	fmt::print("{:-<{}}\n", "", LineSize);
	fmt::print("{0}{2:^{1}}{0}", Border, ColumnSize, "MultVec4");
	fmt::print("{2:^{1}}{0}", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, Mat4, [&FinalSum](LWMatrix4<Type> &Mat, uint32_t i) { FinalSum += Mat * LWVector4<Type>((Type)1); })));
	fmt::print("{2:^{1}}{0}\n", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, SMat4, [&SFinalSum](LWSMatrix4<Type> &Mat, uint32_t i) { SFinalSum += Mat * LWVector4<Type>((Type)1); })));

	fmt::print("{:-<{}}\n", "", LineSize);
	fmt::print("{0}{2:^{1}}{0}", Border, ColumnSize, "Multiply");
	fmt::print("{2:^{1}}{0}", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, Mat4, [&FinalSum](LWMatrix4<Type> &Mat, uint32_t i) { Mat = Mat * Mat; })));
	fmt::print("{2:^{1}}{0}\n", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, SMat4, [&SFinalSum](LWSMatrix4<Type> &Mat, uint32_t i) { Mat = Mat * Mat; })));

	fmt::print("{:-<{}}\n", "", LineSize);
	fmt::print("{0}{2:^{1}}{0}", Border, ColumnSize, "Add");
	fmt::print("{2:^{1}}{0}", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, Mat4, [&FinalSum](LWMatrix4<Type> &Mat, uint32_t i) { Mat = Mat + Mat; })));
	fmt::print("{2:^{1}}{0}\n", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, SMat4, [&SFinalSum](LWSMatrix4<Type> &Mat, uint32_t i) { Mat = Mat + Mat; })));
	
	fmt::print("{:-<{}}\n", "", LineSize);
	fmt::print("{0}{2:^{1}}{0}", Border, ColumnSize, "Subtract");
	fmt::print("{2:^{1}}{0}", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, Mat4, [&FinalSum](LWMatrix4<Type> &Mat, uint32_t i) { Mat = Mat - Mat; })));
	fmt::print("{2:^{1}}{0}\n", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, SMat4, [&SFinalSum](LWSMatrix4<Type> &Mat, uint32_t i) { Mat = Mat - Mat; })));

	fmt::print("{:-<{}}\n", "", LineSize);
	fmt::print("{0}{2:^{1}}{0}", Border, ColumnSize, "Sanitize");
	fmt::print("{2:^{1}e}{0}", Border, ColumnSize, (FinalSum.x + FinalSum.y + FinalSum.z + FinalSum.w));
	fmt::print("{2:^{1}e}{0}\n", Border, ColumnSize, SFinalSum.Sum4());
	fmt::print("{:-<{}}\n", "", LineSize);
	Type Val = (FinalSum.x + FinalSum.y + FinalSum.z + FinalSum.w);
	Type SVal = SFinalSum.Sum4();
	FreePartitions(PartCnt, PartitionLens, Mat4, SMat4);
	return (Type)abs(Val - SVal) <= (Type)std::numeric_limits<float>::epsilon();
};

template<class Type>
bool PerformSIMDQuatTest(uint32_t Count, const LWUTF8Iterator &Name, LWAllocator &Allocator) {
	const uint32_t ColumnSize = 32;
	const uint32_t LineSize = ColumnSize * 3 + 4;
	const char Border = '|';
	fmt::print("Performing {} Test:\n", Name);
	fmt::print("Allocating data: {}\n", (sizeof(LWQuaternion<Type>) * Count + sizeof(LWSQuaternion<Type>) * Count));

	uint32_t *PartitionLens = nullptr;
	LWQuaternion<Type> **Quat = nullptr;
	LWSQuaternion<Type> **SQuat = nullptr;
	uint32_t PartCnt = GeneratePartitians(Count, PartitionLens, Quat, SQuat, Allocator);
	LWVector4<Type> FinalPnt = LWVector4<Type>(1, 0, 0, 1);
	LWSVector4<Type> SFinalPnt = LWSVector4<Type>(1, 0, 0, 1);

	fmt::print("{:-<{}}\n", "", LineSize);
	fmt::print("{0}{2:^{1}}{0}{3:^{1}}{0}{4:^{1}}{0}\n", Border, ColumnSize, "Function", "Generic Time", "SIMD Time");

	fmt::print("{:-<{}}\n", "", LineSize);
	fmt::print("{0}{2:^{1}}{0}", Border, ColumnSize, "Initialize");
	fmt::print("{2:^{1}}{0}", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, Quat, [](LWQuaternion<Type> &Q, uint32_t i) { Type Pitch = -(Type)i * (Type)0.2; Type Yaw = (Type)i * (Type)0.1; Type Roll = (Type)i; Q = LWQuaternion<Type>::FromEuler(Pitch, Yaw, Roll); })));
	fmt::print("{2:^{1}}{0}\n", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, SQuat, [](LWSQuaternion<Type> &Q, uint32_t i) { Type Pitch = -(Type)i * (Type)0.2; Type Yaw = (Type)i * (Type)0.1; Type Roll = (Type)i; Q = LWSQuaternion<Type>::FromEuler(Pitch, Yaw, Roll); })));

	fmt::print("{:-<{}}\n", "", LineSize);
	fmt::print("{0}{2:^{1}}{0}", Border, ColumnSize, "Multiply");
	fmt::print("{2:^{1}}{0}", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, Quat, [](LWQuaternion<Type> &Q, uint32_t i) { Q *= LWQuaternion<Type>(); })));
	fmt::print("{2:^{1}}{0}\n", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, SQuat, [](LWSQuaternion<Type> &Q, uint32_t i) { Q *= LWSQuaternion<Type>(); })));

	fmt::print("{:-<{}}\n", "", LineSize);
	fmt::print("{0}{2:^{1}}{0}", Border, ColumnSize, "Conjugate");
	fmt::print("{2:^{1}}{0}", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, Quat, [](LWQuaternion<Type> &Q, uint32_t i) { Q = Q.Conjugate(); })));
	fmt::print("{2:^{1}}{0}\n", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, SQuat, [](LWSQuaternion<Type> &Q, uint32_t i) { Q = Q.Conjugate(); })));
 
	fmt::print("{:-<{}}\n", "", LineSize);
	fmt::print("{0}{2:^{1}}{0}", Border, ColumnSize, "RotatePnt4");
	fmt::print("{2:^{1}}{0}", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, Quat, [&FinalPnt](LWQuaternion<Type> &Q, uint32_t i) { FinalPnt = Q.RotatePoint(FinalPnt); })));
	fmt::print("{2:^{1}}{0}\n", Border, ColumnSize, *LWUTF8Iterator::C_View<32>("{}ms", DoWork(PartCnt, PartitionLens, SQuat, [&SFinalPnt](LWSQuaternion<Type> &Q, uint32_t i) { SFinalPnt = Q.RotatePoint(SFinalPnt); })));

	fmt::print("{:-<{}}\n", "", LineSize);
	fmt::print("{0}{2:^{1}}{0}", Border, ColumnSize, "Sanitize");
	fmt::print("{2:^{1}}{0}", Border, ColumnSize, FinalPnt);
	fmt::print("{2:^{1}}{0}\n", Border, ColumnSize, SFinalPnt);
	fmt::print("{:-<{}}\n", "", LineSize);

	FreePartitions(PartCnt, PartitionLens, Quat, SQuat);
	return FinalPnt==SFinalPnt.AsVec4();
}

bool PerformSIMDComparisonTest(uint32_t Count) {
	LWAllocator_Default DefAlloc;
	std::cout << "Performing SIMD Comparison Tests: " << Count << std::endl;
	if (!PerformSIMDVec4Test<float>(Count, "Vector4 Float", DefAlloc)) return false;
	if (!PerformSIMDVec4Test<double>(Count, "Vector4 Double", DefAlloc)) return false;
	if (!PerformSIMDVec4Test<int32_t>(Count, "Vector4 Int", DefAlloc)) return false;
	if (!PerformSIMDMat4Test<float>(Count, "Matrix4 Float", DefAlloc)) return false;
	if (!PerformSIMDMat4Test<double>(Count, "Matrix4 Double", DefAlloc)) return false;
	if (!PerformSIMDQuatTest<float>(Count, "Quaternion Float", DefAlloc)) return false;
	if (!PerformSIMDQuatTest<double>(Count, "Quaternion Double", DefAlloc)) return false;
	return true;
};

int main(int, char **){
	std::cout << "Testing LWFramework core features." << std::endl;
	if (!PerformLWAllocatorTest()) std::cout << "Error with LWAllocator test." << std::endl;
	else if (!PerformLWByteBufferTest()) std::cout << "Error with LWByteBuffer Test." << std::endl;
	else if (!PerformLWByteStreamTest()) std::cout << "Error with LWByteStream test." << std::endl;
	else if (!PerformLWVectorTest()) std::cout << "Error with LWVector test." << std::endl;
	else if (!PerformLWSVectorTest()) std::cout << "Error with LWSVector test." << std::endl;
	else if (!PerformLWMatrixTest()) std::cout << "Error with LWMatrix Test." << std::endl;
	else if (!PerformLWSMatrixTest()) std::cout << "Error with LWSMatrix Test." << std::endl;
	else if (!PerformLWQuaternionTest()) std::cout << "Error with LWQuaternion Test." << std::endl;
	else if (!PerformLWSQuaternionTest()) std::cout << "Error with LWSQuaternion Test." << std::endl;
	else if (!PerformLWUnicodeTest<char8_t>()) std::cout << "Error with LWUTF8 Test." << std::endl;
	else if (!PerformLWUnicodeTest<char16_t>()) std::cout << "Error with LWUTF16 Test." << std::endl;
	else if (!PerformLWUnicodeTest<char32_t>()) std::cout << "Error with LWUTF32 Test." << std::endl;
	else if (!PerformLWConcurrentTest()) std::cout << "Error with LWConcurrent Test." << std::endl;
	else if (!PerformLWTimerTest()) std::cout << "Error with LWTimer Test." << std::endl;
	else if (!PerformLWCryptoTest()) std::cout << "Error with LWCrypto test." << std::endl;
	else if (!PerformSIMDComparisonTest(sizeof(void*)==8? 50000000:500000)) std::cout << "Error with SIMD comparison test." << std::endl;  //can't run as high of test count on x86 otherwise run out of memory.
	else std::cout << "LWFramework core successful test." << std::endl;
	return 0;
}
