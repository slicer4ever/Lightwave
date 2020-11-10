#include "LWEJson.h"
#include <LWPlatform/LWPlatform.h>
#include <LWCore/LWAllocator.h>
#include <LWCore/LWQuaternion.h>
#include <LWPlatform/LWFileStream.h>
#include <LWCore/LWMatrix.h>
#include <cstring>
#include <functional>
#include <cstdarg>
#include <algorithm>

LWEJObject &LWEJObject::SetValue(LWAllocator &Allocator, const LWUTF8Iterator &Value) {
	uint32_t Len = Value.RawDistance(Value.NextEnd())+1;
	if (Len <= m_ValueBufferLen) {
		LWEJson::UnEscapeString(Value, m_Value, m_ValueBufferLen);
	} else {
		char8_t *Old = m_Value;
		char8_t *New = Allocator.Allocate<char8_t>(Len);
		uint32_t nLen = LWEJson::UnEscapeString(Value, New, Len);
		m_Value = New;
		m_ValueBufferLen = Len;
		if (Old != m_ValueBuf) LWAllocator::Destroy(Old);
	}
	return *this;
}

LWEJObject &LWEJObject::SetName(LWAllocator &Allocator, const LWUTF8Iterator &Name) {
	uint32_t Len = Name.RawDistance(Name.NextEnd())+1;
	if (Len <= m_NameBufferLen) {
		Name.Copy(m_Name, m_NameBufferLen);
	} else {
		char *Old = m_Name;
		char *New = Allocator.Allocate<char>(Len);
		Name.Copy(New, Len);
		m_Name = New;
		m_NameBufferLen = Len;
		if (Old != m_NameBuf) LWAllocator::Destroy(Old);
	}
	return *this;
}

LWUTF8Iterator LWEJObject::GetName(void) const {
	return LWUTF8Iterator(m_Name);
}

LWUTF8Iterator LWEJObject::GetValue(void) const {
	return LWUTF8Iterator(m_Value);
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

uint32_t LWEJObject::FindChild(const LWUTF8Iterator &ChildName) {
	uint32_t Hash = ChildName.Hash();
	for (uint32_t i = 0; i < m_Length; i++) {
		if (m_Children[i].m_SubNameHash == Hash) return i;
	}
	return -1;
}

LWEJObject *LWEJObject::FindChild(const LWUTF8Iterator &ChildName, LWEJson &J) {
	uint32_t Hash = ChildName.Hash();
	for (uint32_t i = 0; i < m_Length; i++) {
		if (m_Children[i].m_SubNameHash == Hash) return J.Find(m_Children[i].m_FullNameHash);
	}
	return nullptr;
}

LWEJObject &LWEJObject::PushChild(LWEJObject *Child, LWAllocator &Allocator) {
	return InsertChild(m_Length, Child, Allocator);
}

LWEJObject &LWEJObject::InsertChild(uint32_t Position, LWEJObject *Child, LWAllocator &Allocator) {
	if (m_Length >= m_PoolSize) {
		uint32_t NextPoolSize = m_PoolSize == 0 ? LWEJson::ElementPoolSize : m_PoolSize * 2;
		LWEJChild *Pool = Allocator.Allocate<LWEJChild>(NextPoolSize);
		LWEJChild *OPool = m_Children;
		std::copy(m_Children, m_Children + m_PoolSize, Pool);
		m_Children = Pool;
		m_PoolSize = NextPoolSize;
		LWAllocator::Destroy(OPool);
	}
	std::copy_backward(m_Children + Position, m_Children + m_Length, m_Children + m_Length + 1);
	m_Children[Position].m_SubNameHash = LWUTF8Iterator(Child->m_Name).Hash();
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

LWEJObject::LWEJObject(const LWUTF8Iterator &Name, const LWUTF8Iterator &Value, uint32_t Type, uint32_t FullHash, uint32_t ParentHash, LWAllocator &Allocator) : m_Hash(FullHash), m_ParentHash(ParentHash), m_Type(Type) {
	SetName(Allocator, Name);
	SetValue(Allocator, Value);
}

LWEJObject::~LWEJObject() {
	if (m_Name != m_NameBuf) LWAllocator::Destroy(m_Name);
	if (m_Value != m_ValueBuf) LWAllocator::Destroy(m_Value);
	LWAllocator::Destroy(m_Children);
}


uint32_t LWEJson::EscapeString(const LWUTF8Iterator &String, char8_t *Buffer, uint32_t BufferLen) {
	char *B = Buffer;
	char *BL = B + std::min<uint32_t>(BufferLen-1, BufferLen);
	uint32_t o = 0;
	LWUTF8Iterator C = String;
	for(;!C.AtEnd(); ++C) {
		uint32_t cp = *C;
		o += 2;
		if (cp == '\b') {
			if (B != BL) *B++ = '\\';
			if (B != BL) *B++ = 'b';
		}else if (cp == '\f') {
			if (B != BL) *B++ = '\\';
			if (B != BL) *B++ = 'f';
		} else if (cp == '\n') {
			if (B != BL) *B++ = '\\';
			if (B != BL) *B++ = 'n';
		} else if (cp == '\r') {
			if (B != BL) *B++ = '\\';
			if (B != BL) *B++ = 'r';
		} else if (cp == '\t') {
			if (B != BL) *B++ = '\\';
			if (B != BL) *B++ = 't';
		} else if (cp == '\"') {
			if (B != BL) *B++ = '\\';
			if (B != BL) *B++ = '\"';
		} else if (cp == '\\') {
			if (B != BL) *B++ = '\\';
			if (B != BL) *B++ = '\\';
		} else {
			uint32_t r = LWUTF8Iterator::CodePointUnitSize(cp);
			if (B + r <= BL) LWUTF8Iterator::EncodeCodePoint(B, (uint32_t)(uintptr_t)(BL - B), cp);
			B = std::min<char8_t*>(B + r, BL);
			o = (o+r) - 2;
		}
	}
	if (BufferLen) *B = '\0';
	return o+1;
}

uint32_t LWEJson::UnEscapeString(const LWUTF8Iterator &String, char8_t *Buffer, uint32_t BufferLen) {
	char *B = Buffer;
	char *BL = Buffer + std::min<uint32_t>(BufferLen-1, BufferLen);
	uint32_t o = 0;
	LWUTF8Iterator C = String;
	for(;!C.AtEnd();++C) {
		uint32_t cp = *C;
		if(cp=='\\') {
			++C;
			uint32_t scp = *C;
			o++;
			if(B!=BL) {
				if (scp == 'b') *B++ = '\b';
				else if (scp == 'f') *B++ = '\f';
				else if (scp == 'n') *B++ = '\n';
				else if (scp == 'r') *B++ = '\r';
				else if (scp == 't') *B++ = '\t';
				else if (scp == '\"') *B++ = '\"';
				else if (scp == '\\') *B++ = '\\';
				else o--;
			}
		} else {
			uint32_t r = LWUTF8Iterator::CodePointUnitSize(cp);
			if (B <= BL) LWUTF8Iterator::EncodeCodePoint(B, (uint32_t)(uintptr_t)(BL - B), cp);
			B = std::min<char8_t*>(B + r, BL);
			o += r;
		}
	}
	if (BufferLen) *B = '\0';
	return o+1;
}

bool LWEJson::LoadFile(LWEJson &Json, const LWUTF8Iterator &Path, LWAllocator &Allocator, LWEJObject *Parent, LWFileStream *ExistingStream) {
	LWFileStream Stream;
	if (!LWFileStream::OpenStream(Stream, Path, LWFileStream::BinaryMode | LWFileStream::ReadMode, Allocator, ExistingStream)) return false;
	uint32_t Len = Stream.Length() + 1;
	char8_t *B = Allocator.Allocate<char8_t>(Len);
	Stream.ReadText(B, Len);
	bool Res = Parse(Json, B, Parent);
	LWAllocator::Destroy(B);
	return Res;
}


bool LWEJson::Parse(LWEJson &JSon, const LWUTF8Iterator &Source, LWEJObject *Parent) {
	auto OutputLineError = [](const LWUTF8Iterator &Source, const LWUTF8Iterator &P, const LWUTF8Iterator &Error)->LWUTF8Iterator {
		fmt::print("Line {}: {}\n", LWUTF8Iterator::CountLines(LWUTF8Iterator(Source, P)), Error);
		return LWUTF8Iterator();
	};

	std::function<LWUTF8Iterator(const LWUTF8Iterator &, const LWUTF8Iterator &, LWEJObject *, LWEJson &)> ParseElement = [&ParseElement, &OutputLineError](const LWUTF8Iterator &Source, const LWUTF8Iterator &P, LWEJObject *Obj, LWEJson &Js)->LWUTF8Iterator {
		LWUTF8Iterator C = P.NextWord(true);
		if (C.AtEnd()) return OutputLineError(Source, C, u8"Unexpected end of stream.");
		uint32_t i = 0;
		uint32_t ObjHash = Obj ? Obj->m_Hash : 0;
		if(*C=='{') {
			if (Obj) Obj->m_Type = LWEJObject::Object;
			else Js.SetType(LWEJObject::Object);
			for((++C).AdvanceWord(true); !C.AtEnd() && *C!='}'; (++C).AdvanceWord(true)) {
				if (C.AtEnd() || *C != '\"') return OutputLineError(Source, C, "Invalid syntax detected. Expected a \" token.");
				LWUTF8Iterator NameBegin = C + 1;
				LWUTF8Iterator NameEnd = NameBegin.NextToken('\"', false);
				if (NameEnd.AtEnd()) return OutputLineError(Source, C, "Invalid syntax detected. Expected a \" token.");
				C = (NameEnd + 1).NextWord(true);
				if (C.AtEnd() || *C != ':') return OutputLineError(Source, C, "Invalid syntax detected. Expected a : token. ");
				LWEJObject *O = Js.MakeElement(LWUTF8Iterator(NameBegin, NameEnd), Obj);
				if (!O) return OutputLineError(Source, C, "Error: Internal name collision occurred.");
				Obj = Js.Find(ObjHash);
				C = ParseElement(Source, C + 1, O, Js);
				if (C.AtEnd()) return C;
				C = (C + 1).NextWord(true);
				if (C.AtEnd()) return OutputLineError(Source, C, "Error: Unexpected end of stream.");
				if (*C == '}') break;
				if (*C != ',') return OutputLineError(Source, C, "Error: Expected a , token.");
			}
		} else if (*C == '[') {
			if (Obj) Obj->m_Type = LWEJObject::Array;
			else Js.SetType(LWEJObject::Array);
			for ((++C).AdvanceWord(true); !C.AtEnd() && *C != ']'; (C++).AdvanceWord(true)) {
				if (C.AtEnd() || *C == ',') return OutputLineError(Source, C, "Invalid syntax detected. Expected a , token.");
				if (*C == ']') break;
				LWEJObject *O = Js.MakeElement(LWUTF8I::Fmt<256>("{}[{}]", Obj ? Obj->GetName() : LWUTF8I(), i++), Obj);
				if (!O) return OutputLineError(Source, C, "Error: Internal name collision occurred.");
				Obj = Js.Find(ObjHash);
				C = ParseElement(Source, C, O, Js);
				if (C.AtEnd()) return C;
				C = (C + 1).AdvanceWord(true);
				if (C.AtEnd()) return OutputLineError(Source, C, "Error: Unexpected end of stream.");
				if (*C == ']') break;
				if (*C != ',') return OutputLineError(Source, C, "Error: Invalid syntax detected. Expected a , token.");
			}
		} else if (*C == '\"') {
			if (!Obj) return OutputLineError(Source, C, "Error: Unexpected \" Token.");
			LWUTF8Iterator StrEnd = (++C).NextToken('\"', false);
			if (StrEnd.AtEnd()) return OutputLineError(Source, C, "Error: Unexpected end of stream.");
			Obj->SetValue(Js.GetAllocator(), LWUTF8Iterator(C, StrEnd));
			Obj->m_Type = LWEJObject::String;
			C = StrEnd;
		} else {
			if (!Obj) return OutputLineError(Source, C, "Error: Invalid syntax detected.");
			LWUTF8Iterator ValueEnd = C.NextTokens(u8", }]\n\r");
			if (ValueEnd.AtEnd()) return OutputLineError(Source, C, "Error: Unexpected end of stream.");
			Obj->SetValue(Js.GetAllocator(), LWUTF8Iterator(C, ValueEnd));
			if (C.Compare(u8"true", 4) || C.Compare(u8"TRUE", 4) || C.Compare(u8"false", 5) || C.Compare(u8"FALSE", 5)) {
				Obj->m_Type = LWEJObject::Boolean;
			} else if (C.Compare(u8"null", 4)) {
				Obj->m_Type = LWEJObject::Null;
			} else Obj->m_Type = LWEJObject::Number;
			C = (ValueEnd-1);
		}
		return C;
	};
	return ParseElement(Source, Source, Parent, JSon).isInitialized();
}

uint32_t LWEJson::Serialize(char8_t *Buffer, uint32_t BufferLen, bool Format) {
	std::function<uint32_t(char *, uint32_t , LWEJson &, LWEJObject &, uint32_t , bool, bool )> SerializeObjectFmt = [&SerializeObjectFmt](char8_t *Buffer, uint32_t BufferLen, LWEJson &Js, LWEJObject &Obj, uint32_t Depth, bool Last, bool WriteName)->uint32_t {
		uint32_t o = 0;
		uint32_t dm = Depth * 2;
		char Buf[512];

		if(WriteName) o += snprintf((char*)Buffer + o, BufferLen - o, "%*s\"%s\": ", dm, "", Obj.m_Name);
		if (Obj.m_Type == LWEJObject::Array) {
			o += snprintf((char*)Buffer + o, BufferLen - o, "[");
			for (uint32_t i = 0; i < Obj.m_Length; i++) {
				LWEJObject *Jobj = Js.Find(Obj.m_Children[i].m_FullNameHash);
				if(!Jobj) continue;
				o += SerializeObjectFmt(Buffer + o, BufferLen - o, Js, *Jobj, Depth + 1, i == Obj.m_Length - 1, false);
			}
			o += snprintf((char*)Buffer + o, BufferLen-o, "]");
		} else if (Obj.m_Type == LWEJObject::Object) {
			o += snprintf((char*)Buffer + o, BufferLen - o, "{\n");
			for (uint32_t i = 0; i < Obj.m_Length; i++) {
				LWEJObject *Jobj = Js.Find(Obj.m_Children[i].m_FullNameHash);
				if(!Jobj) continue;
				o += SerializeObjectFmt(Buffer + o, BufferLen - o, Js, *Jobj, Depth + 1, i == Obj.m_Length-1, true);
			}
			o += snprintf((char*)Buffer + o, BufferLen-o, "%*s}", dm, "");
		} else if (Obj.m_Type == LWEJObject::String) {
			EscapeString(Obj.m_Value, Buf, sizeof(Buf));
			o += snprintf((char*)Buffer + o, BufferLen - o, "\"%s\"", Buf);
		} else {
			o += snprintf((char*)Buffer + o, BufferLen - o, "%s", Obj.m_Value);
		}
		if (!Last) o += snprintf((char*)Buffer + o, BufferLen - o, ",");
		o += snprintf((char*)Buffer + o, BufferLen - o, "\n");
		return o;
	};

	std::function<uint32_t(char *, uint32_t, LWEJson &, LWEJObject &, bool, bool)> SerializeObject = [&SerializeObject](char8_t *Buffer, uint32_t BufferLen, LWEJson &Js, LWEJObject &Obj, bool Last, bool WriteName) {
		uint32_t o = 0;
		char Buf[512];

		if (WriteName) o += snprintf((char*)Buffer + o, BufferLen - o, "\"%s\":", Obj.m_Name);
		if (Obj.m_Type == LWEJObject::Array) {
			o += snprintf((char*)Buffer + o, BufferLen - o, "[");
			for (uint32_t i = 0; i < Obj.m_Length; i++) {
				LWEJObject *Jobj = Js.Find(Obj.m_Children[i].m_FullNameHash);
				if (!Jobj) continue;
				o += SerializeObject(Buffer + o, BufferLen - o, Js, *Jobj, i == Obj.m_Length - 1, false);
			}
			o += snprintf((char*)Buffer + o, BufferLen - o, "]");
		} else if (Obj.m_Type == LWEJObject::Object) {
			o += snprintf((char*)Buffer + o, BufferLen - o, "{");
			for (uint32_t i = 0; i < Obj.m_Length; i++) {
				LWEJObject *Jobj = Js.Find(Obj.m_Children[i].m_FullNameHash);
				if (!Jobj) continue;
				o += SerializeObject(Buffer + o, BufferLen - o, Js, *Jobj, i == Obj.m_Length - 1, true);
			}
			o += snprintf((char*)Buffer + o, BufferLen - o, "}");
		} else if (Obj.m_Type == LWEJObject::String) {
			EscapeString(Obj.m_Value, Buf, sizeof(Buf));
			o += snprintf((char*)Buffer + o, BufferLen - o, "\"%s\"", Buf);
		} else {
			o += snprintf((char*)Buffer + o, BufferLen - o, "%s", Obj.m_Value);
		}
		if (!Last) o += snprintf((char*)Buffer + o, BufferLen - o, ",");
		return o;
	};

	uint32_t o = 0;

	if (Format) {
		if (m_Type == LWEJObject::Array) o += snprintf((char*)Buffer + o, BufferLen - o, "[");
		else o += snprintf((char*)Buffer + o, BufferLen - o, "{\n");
		for (uint32_t i = 0; i < m_Length; i++) {
			LWEJObject *J = Find(m_Elements[i]);
			if(!J) continue;
			o += SerializeObjectFmt(Buffer + o, BufferLen - o, *this, *J, 1, i == (m_Length - 1), m_Type != LWEJObject::Array);
		}
	} else {
		if (m_Type == LWEJObject::Array) o += snprintf((char*)Buffer + o, BufferLen - o, "[");
		else o += snprintf((char*)Buffer + o, BufferLen - o, "{");
		for (uint32_t i = 0; i < m_Length; i++) {
			LWEJObject *J = Find(m_Elements[i]);
			if (!J) continue;
			o += SerializeObject(Buffer + o, BufferLen - o, *this, *J, i == (m_Length - 1), m_Type!=LWEJObject::Array);
		}
	}
	if (m_Type == LWEJObject::Array) o += snprintf((char*)Buffer + o, BufferLen - o, "]");
	else o += snprintf((char*)Buffer + o, BufferLen - o, "}");
	return o;
}


LWEJObject *LWEJson::MakeObjectElement(const LWUTF8Iterator &Name, LWEJObject *Parent) {
	LWEJObject *Obj = MakeElement(Name, Parent);
	if (Obj) Obj->m_Type = LWEJObject::Object;
	return Obj;
}

LWEJObject *LWEJson::MakeArrayElement(const LWUTF8Iterator &Name, LWEJObject *Parent) {
	LWEJObject *Obj = MakeElement(Name, Parent);
	if (Obj) Obj->m_Type = LWEJObject::Array;
	return Obj;
}

LWEJObject *LWEJson::MakeStringElement(const LWUTF8Iterator &Name, const LWUTF8Iterator &Value, LWEJObject *Parent) {
	LWEJObject *Obj = MakeElement(Name, Parent);
	if (Obj) Obj->SetValue(m_Allocator, Value);
	return Obj;
}

LWEJObject *LWEJson::PushArrayObjectElement(LWEJObject *Parent) {
	uint32_t Len = Parent ? Parent->m_Length : m_Length;
	LWEJObject *Obj = MakeElement(LWUTF8Iterator::C_View<16>(u8"[{}]", Len), Parent);
	if (Obj) Obj->m_Type = LWEJObject::Object;
	return Obj;
}

LWEJObject *LWEJson::PushArrayArrayElement(LWEJObject *Parent){
	uint32_t Len = Parent ? Parent->m_Length : m_Length;
	LWEJObject *Obj = MakeElement(LWUTF8Iterator::C_View<16>(u8"[{}]", Len), Parent);
	if (Obj) Obj->m_Type = LWEJObject::Array;
	return Obj;
}

LWEJObject *LWEJson::PushArrayStringElement(const LWUTF8Iterator &Value, LWEJObject *Parent){
	uint32_t Len = Parent ? Parent->m_Length : m_Length;
	LWEJObject *Obj = MakeElement(LWUTF8Iterator::C_View<16>(u8"[{}]", Len), Parent);
	if (Obj) Obj->SetValue(m_Allocator, Value);
	return Obj;
}

LWEJObject *LWEJson::MakeElement(const LWUTF8Iterator &Name, LWEJObject *Parent) {
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
	snprintf(FullNameBuffer + o, sizeof(FullNameBuffer) - o, "%s", *Name.c_str<256>());
	uint32_t ParentHash = Parent ? Parent->m_Hash : 0;
	uint32_t FullHash = LWUTF8Iterator(FullNameBuffer).Hash();
	//LWEJObject O = LWEJObject(Name, "", 0, LWText::MakeHash(FullNameBuffer), ParentHash);

	//std::pair<uint32_t, LWEJObject> p(O.m_Hash, O);
	auto Res = m_ObjectMap.emplace(FullHash, LWEJObject(Name, "", 0, FullHash, ParentHash, m_Allocator));
	if (!Res.second) {
		fmt::print("JSON name collision: '{}'\n", FullNameBuffer);
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
		uint32_t *Pool = m_Allocator.Allocate<uint32_t>(NextPoolSize);
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

LWEJObject *LWEJson::operator[](const LWUTF8Iterator &Name) {
	return Find(Name.Hash());
}

LWEJObject *LWEJson::operator[](uint32_t NameHash) {
	return Find(NameHash);
}

LWEJObject *LWEJson::Find(const LWUTF8Iterator &Name) {
	return Find(Name.Hash());
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