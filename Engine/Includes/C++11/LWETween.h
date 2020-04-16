#ifndef LWETWEEN_H
#define LWETWEEN_H
#include <LWCore/LWTypes.h>
#include <LWCore/LWVector.h>
#include <LWCore/LWByteBuffer.h>
#include <vector>
#include <algorithm>

template<class Type>
struct LWETweenFrame {
	float m_Time;
	Type m_Value[3];

	static Type LinearTween(const LWETweenFrame<Type> &A, const LWETweenFrame<Type> &B, float p) {
		return A.m_Value[0] + (B.m_Value[0] - A.m_Value[0])*p;
	}

	static Type CubicTween(const LWETweenFrame<Type> &A, const LWETweenFrame<Type> &B, float p) {
		float t2 = p * p;
		float t3 = p * p * p;
		return (2.0f*t3 - 3 * t2 + 1)*A.m_Value[1] + (t3 - 2.0f*t2 + p)*A.m_Value[0] + (-2.0f*t3 + 3.0f*t2)*B.m_Value[1] + (t3 - t2)*B.m_Value[2];
	}

	static void Deserialize(LWETweenFrame<Type> &T, uint32_t ValueCount, LWByteBuffer &Buf) {
		T.m_Time = Buf.Read<float>();
		Buf.Read<Type>(T.m_Value, ValueCount);
		return;
	}

	uint32_t Serialize(LWByteBuffer &Buf, uint32_t ValueCount) {
		uint32_t o = 0;
		o += Buf.Write<float>(m_Time);
		o += Buf.Write<Type>(ValueCount, m_Value);
		return o;
	}

	bool operator < (const LWETweenFrame &Frame) {
		return m_Time < Frame.m_Time;
	}

	bool operator < (float Time) {
		return m_Time < Time;
	}

	LWETweenFrame(float Time, const Type &Value) : m_Time(Time) {
		m_Value[0] = Value;
	}

	LWETweenFrame(float Time, const Type &InTangent, const Type &SplineVertex, const Type &OutTangent) : m_Time(Time) {
		m_Value[0] = InTangent;
		m_Value[1] = SplineVertex;
		m_Value[2] = OutTangent;
	};

	LWETweenFrame() = default;

};

template<class Type>
class LWETween {
public:
	static const uint32_t LINEAR = 0x1a174b80; //LWText::MakeHash("LINEAR"); Linerar interpolation between two frames.
	static const uint32_t STEP = 0xc4977d4f; //LWText::MakeHash("STEP"); no interpolation between two frames.
	static const uint32_t CUBICSPLINE = 0xa7840cfc; //LWText::MakeHash("CUBICSPLINE"); cubic spline interpolation.

	static void Deserialize(LWETween<Type> &T, LWByteBuffer &Buf) {
		uint32_t FrameCnt = Buf.Read<uint32_t>();
		uint32_t Interpolation = Buf.Read<uint32_t>();
		T = LWETween<Type>(Interpolation, FrameCnt);
		uint32_t ValueCnt = Interpolation == CUBICSPLINE ? 3 : 1;
		for (uint32_t i = 0; i < FrameCnt; i++) {
			LWETweenFrame<Type> Frame;
			LWETweenFrame<Type>::Deserialize(Frame, ValueCnt, Buf);
			if (ValueCnt == 3) T.Push(Frame.m_Value[0], Frame.m_Value[1], Frame.m_Value[2], Frame.m_Time);
			else T.Push(Frame.m_Value[0], Frame.m_Time);
		}
		return;
	}

	LWETween &SetInterpolation(uint32_t Interpolation) {
		m_Interpolation = Interpolation;
		return *this;
	}

	bool Push(const Type &Val, float Time) {
		auto Pos = std::lower_bound(m_Frames.begin(), m_Frames.end(), Time);
		m_Frames.emplace(Pos, Time, Val);
		return true;
	}

	bool Push(const Type &InTangent, const Type &SplineVertex, const Type &OutTangent, float Time) {
		auto Pos = std::lower_bound(m_Frames.begin(), m_Frames.end(), Time);
		m_Frames.emplace(Pos, Time, InTangent, SplineVertex, OutTangent);
		return true;
	};

	bool Remove(uint32_t i) {
		uint32_t Count = (uint32_t)m_Frames.size();
		if (i >= Count) return false;
		m_Frames.erase(m_Frames.begin()+i);
		return true;
	}

	LWETween &operator = (LWETween &&O) {
		m_Frames = std::move(O.m_Frames);
		m_Interpolation = O.m_Interpolation;
		return *this;
	}

	LWETween &operator = (const LWETween &O) {
		m_Frames = O.m_Frames;
		m_Interpolation = O.m_Interpolation;
		return *this;
	}

	uint32_t Serialize(LWByteBuffer &Buf) {
		uint32_t ValueCnt = m_Interpolation == CUBICSPLINE ? 3 : 1;
		uint32_t o = 0;
		o += Buf.Write<uint32_t>((uint32_t)m_Frames.size());
		o += Buf.Write<uint32_t>(m_Interpolation);
		for (auto &&Iter : m_Frames) o += Iter.Serialize(Buf, ValueCnt);
		return o;
	}


	float GetValue(float Time, float DefValue) const {
		auto N = std::lower_bound(m_Frames.begin(), m_Frames.end(), Time);
		if (N == m_Frames.end()) {
			if (m_Frames.size() == 0) return DefValue;
			--N;
			return (*N).m_Value;
		} else if (N == m_Frames.begin()) return (*N).m_Value;
		auto C = N - 1;
		float Len = (*N).m_Time - (*C).m_Time;
		float Diff = (*N).m_Value - (*C).m_Value;
		float d = (Time - (*C).m_Time) / Len;
		return (*C).m_Value + Diff * d;
	}

	Type GetValue(float Time, const Type DefValue = Type()) {
		auto RFrame = std::lower_bound(m_Frames.begin(), m_Frames.end(), Time);
		if (RFrame == m_Frames.end()) {
			if (!m_Frames.size()) return DefValue;
			--RFrame;
			return (*RFrame).m_Value[0];
		} else if (RFrame == m_Frames.begin()) return (*RFrame).m_Value[0];
		auto LFrame = RFrame - 1;
		if (m_Interpolation == STEP) return (*LFrame).m_Value[0];
		float Len = (*RFrame).m_Time - (*LFrame).m_Time;
		float Delta = Time - (*LFrame).m_Time;
		float fDelta = fabs(Len) < std::numeric_limits<float>::epsilon() ? 0.0f : Delta / Len;
		if (m_Interpolation == LINEAR) return LWETweenFrame<Type>::LinearTween((*LFrame), (*RFrame), fDelta);
		return LWETweenFrame<Type>::CubicTween((*LFrame), (*RFrame), fDelta);
	}

	LWETweenFrame<Type> &GetFrame(uint32_t i) {
		return m_Frames[i];
	}

	uint32_t GetFrameCount(void) const {
		return (uint32_t)m_Frames.size();
	}

	uint32_t GetInterpolation(void) const {
		return m_Interpolation;
	}

	float GetTotalTime(void) const {
		if (!m_Frames.size()) return 0;
		return m_Frames[m_Frames.size() - 1].m_Time;
	}

	std::vector<LWETweenFrame<Type>> &GetFrames(void) {
		return m_Frames;
	}

	LWETween(uint32_t Interpolation, uint32_t FrameCnt) : m_Interpolation(Interpolation) {
		m_Frames.reserve(FrameCnt);
	}

	LWETween(uint32_t Interpolation) : m_Interpolation(Interpolation) {}

	LWETween(LWETween &&O) : m_Frames(std::move(O.m_Frames)), m_Interpolation(O.m_Interpolation) {}

	LWETween(const LWETween &O) : m_Frames(O.m_Frames), m_Interpolation(O.m_Interpolation) {}

	LWETween() = default;
private:
	std::vector<LWETweenFrame<Type>> m_Frames;
	uint32_t m_Interpolation = LINEAR;
};

template<>
inline LWQuaternionf LWETweenFrame<LWQuaternionf>::LinearTween(const LWETweenFrame<LWQuaternionf> &A, const LWETweenFrame<LWQuaternionf> &B, float p) {
	return LWQuaternionf::SLERP(A.m_Value[0], B.m_Value[0], p);
}

template<>
inline LWQuaterniond LWETweenFrame<LWQuaterniond>::LinearTween(const LWETweenFrame<LWQuaterniond> &A, const LWETweenFrame<LWQuaterniond> &B, float p) {
	return LWQuaterniond::SLERP(A.m_Value[0], B.m_Value[0], (double)p);
}

template<>
inline LWQuaternionf LWETweenFrame<LWQuaternionf>::CubicTween(const LWETweenFrame<LWQuaternionf> &A, const LWETweenFrame<LWQuaternionf> &B, float p) {
	float t2 = p * p;
	float t3 = p * p * p;
	LWVector4f aInTan = LWVector4f(A.m_Value[0].x, A.m_Value[0].y, A.m_Value[0].z, A.m_Value[0].w);
	LWVector4f aSpline = LWVector4f(A.m_Value[1].x, A.m_Value[1].y, A.m_Value[1].z, A.m_Value[1].w);
	LWVector4f bSpline = LWVector4f(B.m_Value[1].x, B.m_Value[1].y, B.m_Value[1].z, B.m_Value[1].w);
	LWVector4f bOutTan = LWVector4f(B.m_Value[2].x, B.m_Value[2].y, B.m_Value[2].z, B.m_Value[2].w);

	LWVector4f Res = (2.0f*t3 - 3 * t2 + 1)*aSpline + (t3 - 2.0f*t2 + p)*aInTan + (-2.0f*t3 + 3.0f*t2)*bSpline + (t3 - t2)*bOutTan;
	return LWQuaternionf(Res.w, Res.x, Res.y, Res.z).Normalize();
}

template<>
inline LWQuaterniond LWETweenFrame<LWQuaterniond>::CubicTween(const LWETweenFrame<LWQuaterniond> &A, const LWETweenFrame<LWQuaterniond> &B, float p) {
	float t2 = p * p;
	float t3 = p * p * p;
	LWVector4d aInTan = LWVector4d(A.m_Value[0].x, A.m_Value[0].y, A.m_Value[0].z, A.m_Value[0].w);
	LWVector4d aSpline = LWVector4d(A.m_Value[1].x, A.m_Value[1].y, A.m_Value[1].z, A.m_Value[1].w);
	LWVector4d bSpline = LWVector4d(B.m_Value[1].x, B.m_Value[1].y, B.m_Value[1].z, B.m_Value[1].w);
	LWVector4d bOutTan = LWVector4d(B.m_Value[2].x, B.m_Value[2].y, B.m_Value[2].z, B.m_Value[2].w);

	LWVector4d Res = (2.0f*t3 - 3 * t2 + 1)*aSpline + (t3 - 2.0f*t2 + p)*aInTan + (-2.0f*t3 + 3.0f*t2)*bSpline + (t3 - t2)*bOutTan;
	return LWQuaterniond(Res.w, Res.x, Res.y, Res.z).Normalize();
}


template<>
inline void LWETweenFrame<LWVector2i>::Deserialize(LWETweenFrame<LWVector2i> &T, uint32_t ValueCnt, LWByteBuffer &Buf) {
	T.m_Time = Buf.Read<float>();
	Buf.ReadVec2<int32_t>(T.m_Value, ValueCnt);
	return;
}

template<>
inline void LWETweenFrame<LWVector2f>::Deserialize(LWETweenFrame<LWVector2f> &T, uint32_t ValueCnt, LWByteBuffer &Buf) {
	T.m_Time = Buf.Read<float>();
	Buf.ReadVec2<float>(T.m_Value, ValueCnt);
	return;
}

template<>
inline void LWETweenFrame<LWVector2d>::Deserialize(LWETweenFrame<LWVector2d> &T, uint32_t ValueCnt, LWByteBuffer &Buf) {
	T.m_Time = Buf.Read<float>();
	Buf.ReadVec2<double>(T.m_Value, ValueCnt);
	return;
}

template<>
inline void LWETweenFrame<LWVector3i>::Deserialize(LWETweenFrame<LWVector3i> &T, uint32_t ValueCnt, LWByteBuffer &Buf) {
	T.m_Time = Buf.Read<float>();
	Buf.ReadVec3<int32_t>(T.m_Value, ValueCnt);
	return;
}

template<>
inline void LWETweenFrame<LWVector3f>::Deserialize(LWETweenFrame<LWVector3f> &T, uint32_t ValueCnt, LWByteBuffer &Buf) {
	T.m_Time = Buf.Read<float>();
	Buf.ReadVec3<float>(T.m_Value, ValueCnt);
	return;
}

template<>
inline void LWETweenFrame<LWVector3d>::Deserialize(LWETweenFrame<LWVector3d> &T, uint32_t ValueCnt, LWByteBuffer &Buf) {
	T.m_Time = Buf.Read<float>();
	Buf.ReadVec3<double>(T.m_Value, ValueCnt);
	return;
}

template<>
inline void LWETweenFrame<LWVector4i>::Deserialize(LWETweenFrame<LWVector4i> &T, uint32_t ValueCnt, LWByteBuffer &Buf) {
	T.m_Time = Buf.Read<float>();
	Buf.ReadVec4<int32_t>(T.m_Value, ValueCnt);
	return;
}

template<>
inline void LWETweenFrame<LWVector4f>::Deserialize(LWETweenFrame<LWVector4f> &T, uint32_t ValueCnt, LWByteBuffer &Buf) {
	T.m_Time = Buf.Read<float>();
	Buf.ReadVec4<float>(T.m_Value, ValueCnt);
	return;
}

template<>
inline void LWETweenFrame<LWVector4d>::Deserialize(LWETweenFrame<LWVector4d> &T, uint32_t ValueCnt, LWByteBuffer &Buf) {
	T.m_Time = Buf.Read<float>();
	Buf.ReadVec4<double>(T.m_Value, ValueCnt);
	return;
}

template<>
inline void LWETweenFrame<LWMatrix2i>::Deserialize(LWETweenFrame<LWMatrix2i> &T, uint32_t ValueCnt, LWByteBuffer &Buf) {
	T.m_Time = Buf.Read<float>();
	Buf.ReadMat2<int32_t>(T.m_Value, ValueCnt);
	return;
}

template<>
inline void LWETweenFrame<LWMatrix2f>::Deserialize(LWETweenFrame<LWMatrix2f> &T, uint32_t ValueCnt, LWByteBuffer &Buf) {
	T.m_Time = Buf.Read<float>();
	Buf.ReadMat2<float>(T.m_Value, ValueCnt);
	return;
}

template<>
inline void LWETweenFrame<LWMatrix2d>::Deserialize(LWETweenFrame<LWMatrix2d> &T, uint32_t ValueCnt, LWByteBuffer &Buf) {
	T.m_Time = Buf.Read<float>();
	Buf.ReadMat2<double>(T.m_Value, ValueCnt);
	return;
}

template<>
inline void LWETweenFrame<LWMatrix3i>::Deserialize(LWETweenFrame<LWMatrix3i> &T, uint32_t ValueCnt, LWByteBuffer &Buf) {
	T.m_Time = Buf.Read<float>();
	Buf.ReadMat3<int32_t>(T.m_Value, ValueCnt);
	return;
}

template<>
inline void LWETweenFrame<LWMatrix3f>::Deserialize(LWETweenFrame<LWMatrix3f> &T, uint32_t ValueCnt, LWByteBuffer &Buf) {
	T.m_Time = Buf.Read<float>();
	Buf.ReadMat3<float>(T.m_Value, ValueCnt);
	return;
}

template<>
inline void LWETweenFrame<LWMatrix3d>::Deserialize(LWETweenFrame<LWMatrix3d> &T, uint32_t ValueCnt, LWByteBuffer &Buf) {
	T.m_Time = Buf.Read<float>();
	Buf.ReadMat3<double>(T.m_Value, ValueCnt);
	return;
}

template<>
inline void LWETweenFrame<LWMatrix4i>::Deserialize(LWETweenFrame<LWMatrix4i> &T, uint32_t ValueCnt, LWByteBuffer &Buf) {
	T.m_Time = Buf.Read<float>();
	Buf.ReadMat4<int32_t>(T.m_Value, ValueCnt);

	return;
}

template<>
inline void LWETweenFrame<LWMatrix4f>::Deserialize(LWETweenFrame<LWMatrix4f> &T, uint32_t ValueCnt, LWByteBuffer &Buf) {
	T.m_Time = Buf.Read<float>();
	Buf.ReadMat4<float>(T.m_Value, ValueCnt);

	return;
}

template<>
inline void LWETweenFrame<LWMatrix4d>::Deserialize(LWETweenFrame<LWMatrix4d> &T, uint32_t ValueCnt, LWByteBuffer &Buf) {
	T.m_Time = Buf.Read<float>();
	Buf.ReadMat4<double>(T.m_Value, ValueCnt);

	return;
}

template<>
inline void LWETweenFrame<LWQuaternionf>::Deserialize(LWETweenFrame<LWQuaternionf> &T, uint32_t ValueCnt, LWByteBuffer &Buf) {
	T.m_Time = Buf.Read<float>();
	Buf.ReadQuaternion<float>(T.m_Value, ValueCnt);
	return;
}

template<>
inline void LWETweenFrame<LWQuaterniond>::Deserialize(LWETweenFrame<LWQuaterniond> &T, uint32_t ValueCnt, LWByteBuffer &Buf) {
	T.m_Time = Buf.Read<float>();
	Buf.ReadQuaternion<double>(T.m_Value, ValueCnt);
	return;
}


#endif