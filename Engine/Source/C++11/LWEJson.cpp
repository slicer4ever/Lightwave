#include "LWEJson.h"
#include <LWCore/LWText.h>
#include <LWPlatform/LWPlatform.h>
#include <LWCore/LWAllocator.h>
#include <LWCore/LWQuaternion.h>
#include <LWCore/LWMatrix.h>
#include <cstring>
#include <functional>
#include <cstdarg>
#include <algorithm>

LWEJObject &LWEJObject::SetValue(LWAllocator &Allocator, const char *Value) {
	uint32_t Len = (uint32_t)strlen(Value) + 1;
	if (Len < m_ValueBufferLen) {
		LWEJson::UnEscapeString(Value, m_Value, m_ValueBufferLen);
	} else {
		char *Old = m_Value;
		char *New = Allocator.AllocateArray<char>(Len);
		LWEJson::UnEscapeString(Value, New, Len);
		m_Value = New;
		m_ValueBufferLen = Len;
		if (Old != m_ValueBuf) LWAllocator::Destroy(Old);
	}
	return *this;
}

LWEJObject &LWEJObject::SetValuef(LWAllocator &Allocator, const char *Fmt, ...) {
	va_list lst;
	va_start(lst, Fmt);
	uint32_t Len = (uint32_t)vsnprintf(nullptr, 0, Fmt, lst)+1;
	va_end(lst);
	va_start(lst, Fmt);
	if (Len < m_ValueBufferLen) {
		vsnprintf(m_Value, m_ValueBufferLen, Fmt, lst);
	} else {
		char *Old = m_Value;
		char *New = Allocator.AllocateArray<char>(Len);
		vsnprintf(New, Len, Fmt, lst);
		m_Value = New;
		m_ValueBufferLen = Len;
		if (Old != m_ValueBuf) LWAllocator::Destroy(Old);
	}
	va_end(lst);
	return *this;
}

LWEJObject &LWEJObject::SetName(LWAllocator &Allocator, const char *Name) {
	uint32_t Len = (uint32_t)strlen(Name) + 1;
	if (Len < m_NameBufferLen) {
		std::copy(Name, Name + Len, m_Name);
	} else {
		char *Old = m_Name;
		char *New = Allocator.AllocateArray<char>(Len);
		std::copy(Name, Name + Len, New);
		m_Name = New;
		m_NameBufferLen = Len;
		if (Old != m_NameBuf) LWAllocator::Destroy(Old);
	}
	return *this;
}

LWEJObject &LWEJObject::SetNamef(LWAllocator &Allocator, const char *Fmt, ...) {
	va_list lst;
	va_start(lst, Fmt);
	uint32_t Len = vsnprintf(nullptr, 0, Fmt, lst) + 1;
	va_end(lst);
	va_start(lst, Fmt);
	if (Len < m_NameBufferLen) {
		vsnprintf(m_Name, m_NameBufferLen, Fmt, lst);
	} else {
		char *Old = m_Name;
		char *New = Allocator.AllocateArray<char>(Len);
		vsnprintf(New, Len, Fmt, lst);
		m_Name = New;
		m_NameBufferLen = Len;
		if (Old != m_ValueBuf) LWAllocator::Destroy(Old);
	}
	va_end(lst);
	return *this;
}

int32_t LWEJObject::AsInt(void) {
	return atoi(m_Value);
}

float LWEJObject::AsFloat(void) {
	return (float)atof(m_Value);
}

bool LWEJObject::AsBoolean(void) {
	return *m_Value == 't' || *m_Value == 'T';
}

uint32_t LWEJObject::FindChild(const char *ChildName) {
	uint32_t NameHash = LWText::MakeHash(ChildName);
	for (uint32_t i = 0; i < m_Length; i++) {
		if (m_Children[i].m_SubNameHash == NameHash) return i;
	}
	return -1;
}

LWEJObject *LWEJObject::FindChild(const char *ChildName, LWEJson &J) {
	uint32_t NameHash = LWText::MakeHash(ChildName);
	for (uint32_t i = 0; i < m_Length; i++) {
		if (m_Children[i].m_SubNameHash == NameHash) return J.Find(m_Children[i].m_FullNameHash);
	}
	return nullptr;
}

LWEJObject &LWEJObject::PushChild(LWEJObject *Child, LWAllocator &Allocator) {
	return InsertChild(m_Length, Child, Allocator);
}

LWEJObject &LWEJObject::InsertChild(uint32_t Position, LWEJObject *Child, LWAllocator &Allocator) {
	if (m_Length >= m_PoolSize) {
		uint32_t NextPoolSize = m_PoolSize == 0 ? LWEJson::ElementPoolSize : m_PoolSize * 2;
		LWEJChild *Pool = Allocator.AllocateArray<LWEJChild>(NextPoolSize);
		LWEJChild *OPool = m_Children;
		std::copy(m_Children, m_Children + m_PoolSize, Pool);
		m_Children = Pool;
		m_PoolSize = NextPoolSize;
		LWAllocator::Destroy(OPool);
	}
	std::copy_backward(m_Children + Position, m_Children + m_Length, m_Children + m_Length + 1);
	m_Children[Position].m_SubNameHash = LWText::MakeHash(Child->m_Name);
	m_Children[Position].m_FullNameHash = Child->m_Hash;
	m_Length++;
	return *this;
}

LWVector2f LWEJObject::AsVec2f(LWEJson &Js) {
	LWVector2f Vec;
	if (m_Type == Array) {
		uint32_t Len = std::min<uint32_t>(m_Length, 2);
		float *V = &Vec.x;
		for (uint32_t i = 0; i < Len; i++) {
			LWEJObject *JO = Js[m_Children[i].m_FullNameHash];
			V[i] = JO->AsFloat();
		}
	} else {
		for (uint32_t i = 0; i < m_Length; i++) {
			LWEJObject *JO = Js[m_Children[i].m_FullNameHash];
			if (!JO) continue;
			if (*JO->m_Name == 'x' || *JO->m_Name == 'X') Vec.x = JO->AsFloat();
			if (*JO->m_Name == 'y' || *JO->m_Name == 'Y') Vec.y = JO->AsFloat();
		}
	}
	return Vec;
}

LWVector3f LWEJObject::AsVec3f(LWEJson &Js) {
	LWVector3f Vec;
	if (m_Type == Array) {
		uint32_t Len = std::min<uint32_t>(m_Length, 3);
		float *V = &Vec.x;
		for (uint32_t i = 0; i < Len; i++) {
			LWEJObject *JO = Js[m_Children[i].m_FullNameHash];
			V[i] = JO->AsFloat();
		}
	} else {
		for (uint32_t i = 0; i < m_Length; i++) {
			LWEJObject *JO = Js[m_Children[i].m_FullNameHash];
			if (!JO) continue;
			if (*JO->m_Name == 'x' || *JO->m_Name == 'X') Vec.x = JO->AsFloat();
			if (*JO->m_Name == 'y' || *JO->m_Name == 'Y') Vec.y = JO->AsFloat();
			if (*JO->m_Name == 'z' || *JO->m_Name == 'Z') Vec.z = JO->AsFloat();
		}
	}
	return Vec;
}

LWVector4f LWEJObject::AsVec4f(LWEJson &Js) {
	LWVector4f Vec;
	if (m_Type == Array) {
		uint32_t Len = std::min<uint32_t>(m_Length, 4);
		float *V = &Vec.x;
		for (uint32_t i = 0; i < Len; i++) {
			LWEJObject *JO = Js[m_Children[i].m_FullNameHash];
			V[i] = JO->AsFloat();
		}
	} else {
		for (uint32_t i = 0; i < m_Length; i++) {
			LWEJObject *JO = Js[m_Children[i].m_FullNameHash];
			if (!JO) continue;
			if (*JO->m_Name == 'x' || *JO->m_Name == 'X') Vec.x = JO->AsFloat();
			if (*JO->m_Name == 'y' || *JO->m_Name == 'Y') Vec.y = JO->AsFloat();
			if (*JO->m_Name == 'z' || *JO->m_Name == 'Z') Vec.z = JO->AsFloat();
			if (*JO->m_Name == 'w' || *JO->m_Name == 'W') Vec.w = JO->AsFloat();
		}
	}
	return Vec;
}

LWQuaternionf LWEJObject::AsQuaternionf(LWEJson &Js){
	LWQuaternionf Quat;
	if (m_Type == Array) {
		uint32_t Len = std::min<uint32_t>(m_Length, 4);
		float *V = &Quat.x;
		for (uint32_t i = 0; i < Len; i++) {
			LWEJObject *JO = Js[m_Children[i].m_FullNameHash];
			V[i] = JO->AsFloat();
		}
	} else {
		for (uint32_t i = 0; i < m_Length; i++) {
			LWEJObject *JO = Js[m_Children[i].m_FullNameHash];
			if (!JO) continue;
			if (*JO->m_Name == 'x' || *JO->m_Name == 'X') Quat.x = JO->AsFloat();
			if (*JO->m_Name == 'y' || *JO->m_Name == 'Y') Quat.y = JO->AsFloat();
			if (*JO->m_Name == 'z' || *JO->m_Name == 'Z') Quat.z = JO->AsFloat();
			if (*JO->m_Name == 'w' || *JO->m_Name == 'W') Quat.w = JO->AsFloat();
		}
	}
	return Quat;
}

LWQuaternionf LWEJObject::AsQuaternionfr(LWEJson &Js) {
	LWQuaternionf Quat;
	if (m_Type == Array) {
		uint32_t Len = std::min<uint32_t>(m_Length, 4);
		float *V = &Quat.x;
		for (uint32_t i = 0; i < Len; i++) {
			LWEJObject *JO = Js[m_Children[i].m_FullNameHash];
			V[i] = JO->AsFloat();
		}
		Quat = LWQuaternionf(Quat.x, Quat.y, Quat.z, Quat.w);
	} else {
		for (uint32_t i = 0; i < m_Length; i++) {
			LWEJObject *JO = Js[m_Children[i].m_FullNameHash];
			if (!JO) continue;
			if (*JO->m_Name == 'x' || *JO->m_Name == 'X') Quat.x = JO->AsFloat();
			if (*JO->m_Name == 'y' || *JO->m_Name == 'Y') Quat.y = JO->AsFloat();
			if (*JO->m_Name == 'z' || *JO->m_Name == 'Z') Quat.z = JO->AsFloat();
			if (*JO->m_Name == 'w' || *JO->m_Name == 'W') Quat.w = JO->AsFloat();
		}
	}
	return Quat;
}

LWMatrix2f LWEJObject::AsMat2f(LWEJson &Js) {
	LWMatrix2f Mat;
	if (m_Type == Array) {
		uint32_t Len = std::min<uint32_t>(m_Length, 4);
		float *V = &Mat.m_Rows[0].x;
		for (uint32_t i = 0; i < Len; i++) {
			LWEJObject *JO = Js[m_Children[i].m_FullNameHash];
			V[i] = JO->AsFloat();
		}
	}
	return Mat;
}

LWMatrix3f LWEJObject::AsMat3f(LWEJson &Js) {
	LWMatrix3f Mat;
	if (m_Type == Array) {
		uint32_t Len = std::min<uint32_t>(m_Length, 9);
		float *V = &Mat.m_Rows[0].x;
		for (uint32_t i = 0; i < Len; i++) {
			LWEJObject *JO = Js[m_Children[i].m_FullNameHash];
			V[i] = JO->AsFloat();
		}
	}
	return Mat;
}

LWMatrix4f LWEJObject::AsMat4f(LWEJson &Js) {
	LWMatrix4f Mat;
	if (m_Type == Array) {
		uint32_t Len = std::min<uint32_t>(m_Length, 16);
		float *V = &Mat.m_Rows[0].x;
		for (uint32_t i = 0; i < Len; i++) {
			LWEJObject *JO = Js[m_Children[i].m_FullNameHash];
			V[i] = JO->AsFloat();
		}
	}
	return Mat;
}

LWVector2i LWEJObject::AsVec2i(LWEJson &Js) {
	LWVector2i Vec;
	if (m_Type == Array) {
		uint32_t Len = std::min<uint32_t>(m_Length, 2);
		int32_t *V = &Vec.x;
		for (uint32_t i = 0; i < Len; i++) {
			LWEJObject *JO = Js[m_Children[i].m_FullNameHash];
			V[i] = JO->AsInt();
		}
	} else {
		for (uint32_t i = 0; i < m_Length; i++) {
			LWEJObject *JO = Js[m_Children[i].m_FullNameHash];
			if (!JO) continue;
			if (*JO->m_Name == 'x' || *JO->m_Name == 'X') Vec.x = JO->AsInt();
			if (*JO->m_Name == 'y' || *JO->m_Name == 'Y') Vec.y = JO->AsInt();
		}
	}
	return Vec;
}

LWVector3i LWEJObject::AsVec3i(LWEJson &Js) {
	LWVector3i Vec;
	if (m_Type == Array) {
		uint32_t Len = std::min<uint32_t>(m_Length, 3);
		int32_t *V = &Vec.x;
		for (uint32_t i = 0; i < Len; i++) {
			LWEJObject *JO = Js[m_Children[i].m_FullNameHash];
			V[i] = JO->AsInt();
		}
	} else {
		for (uint32_t i = 0; i < m_Length; i++) {
			LWEJObject *JO = Js[m_Children[i].m_FullNameHash];
			if (!JO) continue;
			if (*JO->m_Name == 'x' || *JO->m_Name == 'X') Vec.x = JO->AsInt();
			if (*JO->m_Name == 'y' || *JO->m_Name == 'Y') Vec.y = JO->AsInt();
			if (*JO->m_Name == 'z' || *JO->m_Name == 'Z') Vec.z = JO->AsInt();
		}
	}
	return Vec;
}

LWVector4i LWEJObject::AsVec4i(LWEJson &Js){
	LWVector4i Vec;
	if (m_Type == Array) {
		uint32_t Len = std::min<uint32_t>(m_Length, 4);
		int32_t *V = &Vec.x;
		for (uint32_t i = 0; i < Len; i++) {
			LWEJObject *JO = Js[m_Children[i].m_FullNameHash];
			V[i] = JO->AsInt();
		}
	} else {
		for (uint32_t i = 0; i < m_Length; i++) {
			LWEJObject *JO = Js[m_Children[i].m_FullNameHash];
			if (!JO) continue;
			if (*JO->m_Name == 'x' || *JO->m_Name == 'X') Vec.x = JO->AsInt();
			if (*JO->m_Name == 'y' || *JO->m_Name == 'Y') Vec.y = JO->AsInt();
			if (*JO->m_Name == 'z' || *JO->m_Name == 'Z') Vec.z = JO->AsInt();
			if (*JO->m_Name == 'w' || *JO->m_Name == 'W') Vec.w = JO->AsInt();
		}
	}
	return Vec;
}

LWEJObject &LWEJObject::operator=(LWEJObject &&O) {
	std::copy(O.m_NameBuf, O.m_NameBuf+sizeof(m_NameBuf), m_NameBuf);
	std::copy(O.m_ValueBuf, O.m_ValueBuf+sizeof(m_ValueBuf), m_ValueBuf);
	m_Name = O.m_Name == O.m_NameBuf ? m_NameBuf : O.m_Name;
	m_Value = O.m_Value == O.m_ValueBuf ? m_ValueBuf : O.m_Value;
	m_NameBufferLen = O.m_NameBufferLen;
	m_ValueBufferLen = O.m_ValueBufferLen;
	m_Hash = O.m_Hash;
	m_ParentHash = O.m_ParentHash;
	m_Children = O.m_Children;
	m_PoolSize = O.m_PoolSize;
	m_Length = O.m_Length;
	m_Type = O.m_Type;

	O.m_Name = O.m_NameBuf;
	O.m_Value = O.m_ValueBuf;
	O.m_Children = nullptr;
	return *this;
}

LWEJObject::LWEJObject(LWEJObject &&O) : m_NameBufferLen(O.m_NameBufferLen), m_ValueBufferLen(O.m_ValueBufferLen), m_Hash(O.m_Hash), m_ParentHash(O.m_ParentHash), m_Children(O.m_Children), m_PoolSize(O.m_PoolSize), m_Length(O.m_Length), m_Type(O.m_Type) {
	std::copy(O.m_NameBuf, O.m_NameBuf + sizeof(m_NameBuf), m_NameBuf);
	std::copy(O.m_ValueBuf, O.m_ValueBuf + sizeof(m_ValueBuf), m_ValueBuf);
	m_Name = O.m_Name == O.m_NameBuf ? m_NameBuf : O.m_Name;
	m_Value = O.m_Value == O.m_ValueBuf ? m_ValueBuf : O.m_Value;

	O.m_Name = O.m_NameBuf;
	O.m_Value = O.m_ValueBuf;
	O.m_Children = nullptr;
}

LWEJObject::LWEJObject(const char *Name, const char *Value, uint32_t Type, uint32_t FullHash, uint32_t ParentHash, LWAllocator &Allocator) : m_Hash(FullHash), m_ParentHash(ParentHash), m_Type(Type) {
	SetName(Allocator, Name);
	SetValue(Allocator, Value);
}

LWEJObject::~LWEJObject() {
	if (m_Name != m_NameBuf) LWAllocator::Destroy(m_Name);
	if (m_Value != m_ValueBuf) LWAllocator::Destroy(m_Value);
	LWAllocator::Destroy(m_Children);
}


uint32_t LWEJson::EscapeString(const char *String, char *Buffer, uint32_t BufferLen) {
	char *B = Buffer;
	char *L = B + BufferLen;
	uint32_t o = 0;
	for (const char *S = String; *S; S++) {
		o += 2;
		if (*S == '\b') {
			if (B != L) *B++ = '\\';
			if (B != L) *B++ = 'b';
		} else if (*S == '\f') {
			if (B != L) *B++ = '\\';
			if (B != L) *B++ = 'f';
		} else if (*S == '\n') {
			if (B != L) *B++ = '\\';
			if (B != L) *B++ = 'n';
		} else if (*S == '\r') {
			if (B != L) *B++ = '\\';
			if (B != L) *B++ = 'r';
		} else if (*S == '\t') {
			if (B != L) *B++ = '\\';
			if (B != L) *B++ = 't';
		} else if (*S == '\"') {
			if (B != L) *B++ = '\\';
			if (B != L) *B++ = '\"';
		} else if (*S == '\\') {
			if (B != L) *B++ = '\\';
			if (B != L) *B++ = '\\';
		} else {
			if(B!=L) *B++ = *S;
			o -= 1;
		}
	}
	if (B != L) *B = '\0';
	o++;
	return o;
}

uint32_t LWEJson::UnEscapeString(const char *String, char *Buffer, uint32_t BufferLen) {
	char *B = Buffer;
	char *L = Buffer + BufferLen;
	uint32_t o = 0;
	for (const char *S = String; *S; S++) {
		if (*S == '\\') {
			S++;
			if (B != L) {
				if (*S == 'b') *B++ = '\b';
				else if (*S == 'f') *B++ = '\f';
				else if (*S == 'n') *B++ = '\n';
				else if (*S == 'r') *B++ = '\r';
				else if (*S == 't') *B++ = '\t';
				else if (*S == '\"') *B++ = '\"';
				else if (*S == '\\') *B++ = '\\';
			}
		} else {
			if (B != L) *B++ = *S;
		}
		o++;
	}
	if (B != L) *B = '\0';
	o++;
	return o;
}

bool LWEJson::Parse(LWEJson &JSon, const char *Buffer, LWEJObject *Parent) {
	char StaticDataBuffer[1024 * 32];
	char *DataBuffer = StaticDataBuffer;
	uint32_t DataBufferLen = sizeof(StaticDataBuffer);

	auto OutputLineError = [](const char *Buffer, const char *P, const char *Error)->const char* {
		uint32_t Line = 0;
		if (P) {
			for (const char *C = Buffer; C != P && *C; C++) {
				if (*C == '\n') Line++;
			}
			std::cout << "JSON parse error line " << Line << ": " << Error << std::endl;
		} else std::cout << "JSON parse error(unknown line): " << Error << std::endl;
		return nullptr;
	};

	auto MakeDataBuffer = [&StaticDataBuffer, &DataBuffer, &DataBufferLen, &JSon](uint32_t NewLength)->char* {
		if (NewLength > DataBufferLen) {
			char *Old = DataBuffer;
			DataBuffer = JSon.GetAllocator().AllocateArray<char>(NewLength);
			if (Old != StaticDataBuffer) LWAllocator::Destroy(Old);
		}

		return DataBuffer;
	};

	auto CopyBufferToToken = [&MakeDataBuffer](const char *P, char Token)->const char* {
		const char *V = P;
		for (; *V; V++) {
			if (*V == Token) break;
			if (*V == '\\') {
				if (*(V + 1) == Token) V++;
			}
		}
		if (!*V) return nullptr; //Keep consistency with LWText functions.
		uint32_t Len = (uint32_t)(uintptr_t)(V - P);
		char *Buf = MakeDataBuffer(Len + 1);
		std::copy(P, V, Buf);
		Buf[Len] = '\0';
		return V;
	};

	auto CopyBufferToTokens = [&MakeDataBuffer](const char *P, const char *Tokens)->const char* {
		const char *V = P;
		bool Found = false;
		for (; V; V = LWText::Next(V)) {
			uint32_t VC = LWText::GetCharacter(V);
			for (const char *T = Tokens; T && !Found; T = LWText::Next(T)) Found = VC == LWText::GetCharacter(T);
			if (Found) break;
		}
		if (!V) return nullptr;
		uint32_t Len = (uint32_t)(uintptr_t)(V - P);
		char *Buf = MakeDataBuffer(Len + 1);
		std::copy(P, V, Buf);
		Buf[Len] = '\0';
		return V;
	};

	std::function<const char*(const char *, const char *, LWEJObject *, LWEJson &)> ParseElement = [&ParseElement, &CopyBufferToToken, &CopyBufferToTokens, &OutputLineError, &DataBuffer](const char *Buffer, const char *P, LWEJObject *Obj, LWEJson &Js)->const char* {
		P = LWText::NextWord(P, true);
		if (!P) return OutputLineError(Buffer, P, "Invalid syntax detected.");
		uint32_t i = 0;
		uint32_t ObjHash = Obj ? Obj->m_Hash : 0;
		if (*P == '{') {
			if (Obj) Obj->m_Type = LWEJObject::Object;
			else Js.SetType(LWEJObject::Object);
			for (P = LWText::NextWord(P + 1, true); P && *P && *P != '}'; P++) {
				P = LWText::NextWord(P, true);
				if (!P || *P != '\"') return OutputLineError(Buffer, P, "Invalid syntax detected.");
				P = CopyBufferToToken(P + 1, '\"');
				if (!P) return OutputLineError(Buffer, P, "Invalid syntax detected.");
				P = LWText::NextWord(P + 1, true);
				if (!P || *P != ':') return OutputLineError(Buffer, P, "Invalid syntax detected.");
				LWEJObject *O = Js.MakeElement(DataBuffer, Obj);
				if (!O) return OutputLineError(Buffer, P, "Error: Internal name collision occurred.");
				Obj = Js.Find(ObjHash);
				P = ParseElement(Buffer, P + 1, O, Js);
				if (!P) return nullptr;
				P = LWText::NextWord(P + 1, true);
				if (!P) return OutputLineError(Buffer, P, "Error: Invalid syntax detected.");
				if (*P == '}') break;
				if (*P != ',') return OutputLineError(Buffer, P, "Error: Invalid syntax detected.");
			}
		} else if (*P == '[') {
			if (Obj) Obj->m_Type = LWEJObject::Array;
			else Js.SetType(LWEJObject::Array);
			for (P = LWText::NextWord(P + 1, true); P && *P && *P != ']'; P++) {
				P = LWText::NextWord(P, true);
				if (!P || *P == ',') return OutputLineError(Buffer, P, "Invalid syntax detected.");
				if (*P == ']') break;
				LWEJObject *O = Js.MakeElementf("%s[%d]", Obj, Obj ? Obj->m_Name : "", i++);
				if (!O) return OutputLineError(Buffer, P, "Error: Internal name collision occurred.");
				Obj = Js.Find(ObjHash);
				P = ParseElement(Buffer, P, O, Js);
				if (!P) return nullptr;
				P = LWText::NextWord(P + 1, true);
				if (!P) return OutputLineError(Buffer, P, "Error: Invalid syntax detected.");
				if (*P == ']') break;
				if (*P != ',') return OutputLineError(Buffer, P, "Error: Invalid syntax detected.");
			}
		} else if (*P == '\"') {
			if (!Obj) return OutputLineError(Buffer, P, "Error: Invalid syntax detected.");
			P = CopyBufferToToken(P + 1, '\"');
			if (!P) return OutputLineError(Buffer, P, "Error: Invalid syntax detected.");
			Obj->SetValue(Js.GetAllocator(), DataBuffer);
			Obj->m_Type = LWEJObject::String;
		} else {
			if (!Obj) return OutputLineError(Buffer, P, "Error: Invalid syntax detected.");
			P = CopyBufferToTokens(P, ", }]\n\r");
			if (!P) return OutputLineError(Buffer, P, "Error: Invalid syntax detected.");
			Obj->SetValue(Js.GetAllocator(), DataBuffer);
			if (!strncasecmp(Obj->m_Value, "true", 4) || !strncasecmp(Obj->m_Value, "false", 5)) {
				Obj->m_Type = LWEJObject::Boolean;
			} else if (!strncasecmp(Obj->m_Value, "null", 4)) {
				Obj->m_Type = LWEJObject::Null;
			} else Obj->m_Type = LWEJObject::Number;
			P--;
		}
		return P;
	};
	const char *Res = ParseElement(Buffer, Buffer, Parent, JSon);
	if (DataBuffer != StaticDataBuffer) LWAllocator::Destroy(DataBuffer);
	return Res != nullptr;
}

uint32_t LWEJson::Serialize(char *Buffer, uint32_t BufferLen, bool Format) {

	std::function<uint32_t(char *, uint32_t , LWEJson &, LWEJObject &, uint32_t , bool, bool )> SerializeObjectFmt = [&SerializeObjectFmt](char *Buffer, uint32_t BufferLen, LWEJson &Js, LWEJObject &Obj, uint32_t Depth, bool Last, bool WriteName)->uint32_t {
		uint32_t o = 0;
		uint32_t dm = Depth * 2;
		char Buf[512];

		if(WriteName) o += snprintf(Buffer + o, BufferLen - o, "%*s\"%s\": ", dm, "", Obj.m_Name);
		if (Obj.m_Type == LWEJObject::Array) {
			o += snprintf(Buffer + o, BufferLen - o, "[");
			for (uint32_t i = 0; i < Obj.m_Length; i++) {
				LWEJObject *Jobj = Js.Find(Obj.m_Children[i].m_FullNameHash);
				if(!Jobj) continue;
				o += SerializeObjectFmt(Buffer + o, BufferLen - o, Js, *Jobj, Depth + 1, i == Obj.m_Length - 1, false);
			}
			o += snprintf(Buffer + o, BufferLen-o, "]");
		} else if (Obj.m_Type == LWEJObject::Object) {
			o += snprintf(Buffer + o, BufferLen - o, "{\n");
			for (uint32_t i = 0; i < Obj.m_Length; i++) {
				LWEJObject *Jobj = Js.Find(Obj.m_Children[i].m_FullNameHash);
				if(!Jobj) continue;
				o += SerializeObjectFmt(Buffer + o, BufferLen - o, Js, *Jobj, Depth + 1, i == Obj.m_Length-1, true);
			}
			o += snprintf(Buffer + o, BufferLen-o, "%*s}", dm, "");
		} else if (Obj.m_Type == LWEJObject::String) {
			EscapeString(Obj.m_Value, Buf, sizeof(Buf));
			o += snprintf(Buffer + o, BufferLen - o, "\"%s\"", Buf);
		} else {
			o += snprintf(Buffer + o, BufferLen - o, "%s", Obj.m_Value);
		}
		if (!Last) o += snprintf(Buffer + o, BufferLen - o, ",");
		o += snprintf(Buffer + o, BufferLen - o, "\n");
		return o;
	};

	std::function<uint32_t(char *, uint32_t, LWEJson &, LWEJObject &, bool, bool)> SerializeObject = [&SerializeObject](char *Buffer, uint32_t BufferLen, LWEJson &Js, LWEJObject &Obj, bool Last, bool WriteName) {
		uint32_t o = 0;
		char Buf[512];

		if (WriteName) o += snprintf(Buffer + o, BufferLen - o, "\"%s\":", Obj.m_Name);
		if (Obj.m_Type == LWEJObject::Array) {
			o += snprintf(Buffer + o, BufferLen - o, "[");
			for (uint32_t i = 0; i < Obj.m_Length; i++) {
				LWEJObject *Jobj = Js.Find(Obj.m_Children[i].m_FullNameHash);
				if (!Jobj) continue;
				o += SerializeObject(Buffer + o, BufferLen - o, Js, *Jobj, i == Obj.m_Length - 1, false);
			}
			o += snprintf(Buffer + o, BufferLen - o, "]");
		} else if (Obj.m_Type == LWEJObject::Object) {
			o += snprintf(Buffer + o, BufferLen - o, "{");
			for (uint32_t i = 0; i < Obj.m_Length; i++) {
				LWEJObject *Jobj = Js.Find(Obj.m_Children[i].m_FullNameHash);
				if (!Jobj) continue;
				o += SerializeObject(Buffer + o, BufferLen - o, Js, *Jobj, i == Obj.m_Length - 1, true);
			}
			o += snprintf(Buffer + o, BufferLen - o, "}");
		} else if (Obj.m_Type == LWEJObject::String) {
			EscapeString(Obj.m_Value, Buf, sizeof(Buf));
			o += snprintf(Buffer + o, BufferLen - o, "\"%s\"", Buf);
		} else {
			o += snprintf(Buffer + o, BufferLen - o, "%s", Obj.m_Value);
		}
		if (!Last) o += snprintf(Buffer + o, BufferLen - o, ",");
		return o;
	};

	uint32_t o = 0;

	if (Format) {
		if (m_Type == LWEJObject::Array) o += snprintf(Buffer + o, BufferLen - o, "[");
		else o += snprintf(Buffer + o, BufferLen - o, "{\n");
		for (uint32_t i = 0; i < m_Length; i++) {
			LWEJObject *J = Find(m_Elements[i]);
			if(!J) continue;
			o += SerializeObjectFmt(Buffer + o, BufferLen - o, *this, *J, 1, i == (m_Length - 1), m_Type != LWEJObject::Array);
		}
	} else {
		if (m_Type == LWEJObject::Array) o += snprintf(Buffer + o, BufferLen - o, "[");
		else o += snprintf(Buffer + o, BufferLen - o, "{");
		for (uint32_t i = 0; i < m_Length; i++) {
			LWEJObject *J = Find(m_Elements[i]);
			if (!J) continue;
			o += SerializeObject(Buffer + o, BufferLen - o, *this, *J, i == (m_Length - 1), m_Type!=LWEJObject::Array);
		}
	}
	if (m_Type == LWEJObject::Array) o += snprintf(Buffer + o, BufferLen - o, "]");
	else o += snprintf(Buffer + o, BufferLen - o, "}");
	return o;
}

LWEJObject *LWEJson::MakeElement(const char *Name, LWEJObject *Parent) {
	const uint32_t MaxParents = 128;
	char FullNameBuffer[1024];
	LWEJObject *ParentTable[MaxParents];
	uint32_t ParentCnt = 0;
	LWEJObject *P = Parent;
	while (P) {
		if (ParentCnt >= MaxParents) return nullptr;
		if (P->m_Type != LWEJObject::Array) ParentTable[ParentCnt++] = P;
		P = P->m_ParentHash == 0 ? nullptr : Find(P->m_ParentHash);
	}
	uint32_t o = 0;
	while (ParentCnt) {
		o += snprintf(FullNameBuffer + o, sizeof(FullNameBuffer) - o, "%s.", ParentTable[ParentCnt - 1]->m_Name);
		ParentCnt--;
	}
	snprintf(FullNameBuffer + o, sizeof(FullNameBuffer) - o, "%s", Name);
	uint32_t ParentHash = Parent ? Parent->m_Hash : 0;
	uint32_t FullHash = LWText::MakeHash(FullNameBuffer);
	//LWEJObject O = LWEJObject(Name, "", 0, LWText::MakeHash(FullNameBuffer), ParentHash);

	//std::pair<uint32_t, LWEJObject> p(O.m_Hash, O);
	auto Res = m_ObjectMap.emplace(FullHash, LWEJObject(Name, "", 0, FullHash, ParentHash, m_Allocator));
	if (!Res.second) {
		std::cout << "JSON name collision: '" << FullNameBuffer << "'" << std::endl;
		return nullptr;
	}
	LWEJObject &R = Res.first->second;
	if (Parent) Parent->PushChild(&R, m_Allocator);
	else PushRootElement(&R);
	return &R;
}

LWEJson &LWEJson::PushRootElement(LWEJObject *Object) {
	return InsertRootElement(m_Length, Object);
}

LWEJson &LWEJson::InsertRootElement(uint32_t Position, LWEJObject *Object) {
	if (m_Length >= m_PoolSize) {
		uint32_t NextPoolSize = m_PoolSize == 0 ? LWEJson::ElementPoolSize : m_PoolSize * 2;
		uint32_t *Pool = m_Allocator.AllocateArray<uint32_t>(NextPoolSize);
		uint32_t *OPool = m_Elements;
		std::copy(m_Elements, m_Elements + m_PoolSize, Pool);
		m_Elements = Pool;
		m_PoolSize = NextPoolSize;
		LWAllocator::Destroy(OPool);
	}
	std::copy_backward(m_Elements + Position, m_Elements + m_Length, m_Elements + m_Length + 1);
	m_Elements[Position] = Object->m_Hash;
	m_Length++;
	return *this;
	
}

LWEJObject *LWEJson::MakeElementf(const char *Fmt, LWEJObject *Parent, ...) {
	char Buffer[256];
	va_list lst;
	va_start(lst, Parent);
	vsnprintf(Buffer, sizeof(Buffer), Fmt, lst);
	va_end(lst);
	return MakeElement(Buffer, Parent);
}

LWEJson &LWEJson::SetType(uint32_t Type) {
	m_Type = Type;
	return *this;
}

LWEJObject *LWEJson::GetElement(uint32_t i, LWEJObject *Parent) {
	if (Parent && i < Parent->m_Length) return Find(Parent->m_Children[i].m_FullNameHash);
	if (!Parent && i < m_Length) return Find(m_Elements[i]);
	return nullptr;
}

uint32_t LWEJson::GetLength(void) {
	return m_Length;
}

uint32_t LWEJson::GetType(void) {
	return m_Type;
}

LWEJObject *LWEJson::operator[](const char *Name) {
	uint32_t Hash = LWText::MakeHash(Name);
	return Find(Hash);
}

LWEJObject *LWEJson::operator[](uint32_t NameHash) {
	return Find(NameHash);
}

LWEJObject *LWEJson::Findf(const char *Fmt, ...) {
	char Buffer[256];
	va_list lst;
	va_start(lst, Fmt);
	vsnprintf(Buffer, sizeof(Buffer), Fmt, lst);
	va_end(lst);
	return Find(Buffer);
}

LWEJObject *LWEJson::Find(const char *Name) {
	uint32_t Hash = LWText::MakeHash(Name);
	return Find(Hash);
}

LWEJObject *LWEJson::Find(uint32_t NameHash) {
	auto Iter = m_ObjectMap.find(NameHash);
	if (Iter == m_ObjectMap.end()) return nullptr;
	return &Iter->second;
}

LWAllocator &LWEJson::GetAllocator(void) {
	return m_Allocator;
}

LWEJson::LWEJson(LWAllocator &Allocator) : m_Allocator(Allocator) {}

LWEJson::~LWEJson() {
	LWAllocator::Destroy(m_Elements);
}