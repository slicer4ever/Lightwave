#ifndef LWEJSON
#define LWEJSON
#include <unordered_map>
#include <cstdarg>
#include <LWCore/LWVector.h>
#include <LWCore/LWUnicode.h>
#include "LWETypes.h"

struct LWEJChild {
	uint32_t m_SubNameHash;
	uint32_t m_FullNameHash;
};

struct LWEJObject {
	enum {
		String=0x0,
		Number = 0x1,
		Boolean=0x2,
		Array=0x3,
		Object=0x4,
		Null=0x5
		
	};
	static const uint32_t NameBufferLen = 32;
	static const uint32_t ValueBufferLen = 32;
	char8_t m_NameBuf[NameBufferLen]= {};
	char8_t m_ValueBuf[ValueBufferLen]= {};
	char8_t *m_Name = m_NameBuf;
	char8_t *m_Value = m_ValueBuf;
	uint32_t m_NameBufferLen = NameBufferLen;
	uint32_t m_ValueBufferLen = ValueBufferLen;
	uint32_t m_Hash = 0;
	uint32_t m_ParentHash = 0;
	LWEJChild *m_Children = nullptr;
	uint32_t m_PoolSize = 0;
	uint32_t m_Length = 0;
	uint32_t m_Type = 0;

	/*!< \brief overload of all basic types(float,double,boolean, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t */
	template<class Type>
	LWEJObject &SetTypeValue(LWAllocator &Allocator, Type Val) {
		return *this;
	}

	LWEJObject &SetValue(LWAllocator &Allocator, const LWUTF8Iterator &Value);

	LWEJObject &SetName(LWAllocator &Allocator, const LWUTF8Iterator &Value);

	uint32_t FindChild(const LWUTF8Iterator &ChildName);

	LWEJObject *FindChild(const LWUTF8Iterator &ChildName, LWEJson &J);

	LWEJObject *GetChild(uint32_t i, LWEJson &J);

	LWEJObject &PushChild(LWEJObject *Child, LWAllocator &Allocator);

	LWEJObject &InsertChild(uint32_t Position, LWEJObject *Child, LWAllocator &Allocator);

	LWUTF8Iterator GetName(void) const;

	LWUTF8Iterator GetValue(void) const;

	int32_t AsInt(void) const;

	uint32_t AsUInt(void) const;

	int64_t AsInt64(void) const;

	uint64_t AsUInt64(void) const;

	float AsFloat(void) const;

	double AsDouble(void) const;

	bool AsBoolean(void) const;

	LWVector2f AsVec2f(LWEJson &Js);

	LWVector3f AsVec3f(LWEJson &Js);

	LWVector4f AsVec4f(LWEJson &Js);

	LWQuaternionf AsQuaternionf(LWEJson &Js);

	LWQuaternionf AsQuaternionfr(LWEJson &Js);

	LWMatrix2f AsMat2f(LWEJson &Js);

	LWMatrix3f AsMat3f(LWEJson &Js);
	
	LWMatrix4f AsMat4f(LWEJson &Js);

	LWVector2i AsVec2i(LWEJson &Js);

	LWVector3i AsVec3i(LWEJson &Js);

	LWVector4i AsVec4i(LWEJson &Js);

	uint32_t GetChildrenCount(void) const;

	LWEJObject &operator = (LWEJObject &&O);

	LWEJObject(LWEJObject &&O);

	LWEJObject() = default;

	LWEJObject(const LWUTF8Iterator &Name, const LWUTF8Iterator &Value, uint32_t Type, uint32_t FullHash, uint32_t ParentHash, LWAllocator &Allocator);

	~LWEJObject();

};

class LWEJson {
public:
	enum {
		ElementPoolSize = 64
	};

	static uint32_t EscapeString(const LWUTF8Iterator &String, char8_t *Buffer, uint32_t BufferLen);

	static uint32_t UnEscapeString(const LWUTF8Iterator &String, char8_t *Buffer, uint32_t BufferLen);

	static bool LoadFile(LWEJson &Json, const LWUTF8Iterator &Path, LWAllocator &Allocator, LWEJObject *Parent = nullptr, LWFileStream *ExistingStream = nullptr);

	static bool Parse(LWEJson &JSon, const LWUTF8Iterator &Buffer, LWEJObject *Parent = nullptr);

	uint32_t Serialize(char8_t *Buffer, uint32_t BufferLen, bool Format);

	LWEJObject *MakeObjectElement(const LWUTF8Iterator &Name, LWEJObject *Parent = nullptr);
	
	LWEJObject *MakeArrayElement(const LWUTF8Iterator &Name, LWEJObject *Parent = nullptr);

	LWEJObject *MakeStringElement(const LWUTF8Iterator &Name, const LWUTF8Iterator &Value, LWEJObject *Parent = nullptr);

	template<class Type>
	LWEJObject *MakeValueElement(const LWUTF8Iterator &Name, Type Value, LWEJObject *Parent = nullptr) {
		LWEJObject *Obj = MakeElement(Name, Parent);
		if(Obj) Obj->SetTypeValue(m_Allocator, Value);
		return Obj;
	}

	LWEJObject *PushArrayObjectElement(LWEJObject *Parent = nullptr);

	LWEJObject *PushArrayArrayElement(LWEJObject *Parent = nullptr);

	LWEJObject *PushArrayStringElement(const LWUTF8Iterator &Value, LWEJObject *Parent = nullptr);

	template<class Type>
	LWEJObject *PushArrayValueElement(Type Value, LWEJObject *Parent = nullptr) {
		uint32_t Len = Parent ? Parent->m_Length : m_Length;
		LWEJObject *Obj = MakeElement(LWUTF8C_View<32>("[{}]", Len), Parent);
		if (Obj) Obj->SetTypeValue(m_Allocator, Value);
		return Obj;
	}

	LWEJObject *MakeElement(const LWUTF8Iterator &Name, LWEJObject *Parent = nullptr);

	LWEJson &PushRootElement(LWEJObject *Object);

	LWEJson &InsertRootElement(uint32_t Position, LWEJObject *Object);

	LWEJson &SetType(uint32_t Type);

	LWEJObject *GetElement(uint32_t i, LWEJObject *Parent = nullptr);

	uint32_t GetLength(void);

	uint32_t GetType(void);

	LWEJObject *operator[](const LWUTF8Iterator &Name);

	LWEJObject *operator[](uint32_t NameHash);

	LWEJObject *Find(const LWUTF8Iterator &Name);

	LWEJObject *Find(uint32_t NameHash);

	LWAllocator &GetAllocator(void);

	LWEJson(LWAllocator &Allocator);

	~LWEJson();
private:
	LWAllocator &m_Allocator;
	uint32_t *m_Elements = nullptr;
	uint32_t m_PoolSize = 0;
	uint32_t m_Length = 0;
	uint32_t m_Type = LWEJObject::Object;
	std::unordered_map<uint32_t, LWEJObject> m_ObjectMap;
};

// @cond

template<>
inline LWEJObject &LWEJObject::SetTypeValue<bool>(LWAllocator &Allocator, bool Value) {
	m_Type = Boolean;
	return SetValue(Allocator, Value ? u8"true" : u8"false");
}

template<>
inline LWEJObject &LWEJObject::SetTypeValue<int8_t>(LWAllocator &Allocator, int8_t Value) {
	m_Type = Number;
	return SetValue(Allocator, LWUTF8C_View<16>(u8"{}", Value));
}

template<>
inline LWEJObject &LWEJObject::SetTypeValue<uint8_t>(LWAllocator &Allocator, uint8_t Value) {
	m_Type = Number;
	return SetValue(Allocator, LWUTF8C_View<16>(u8"{}", Value));
}

template<>
inline LWEJObject &LWEJObject::SetTypeValue<int16_t>(LWAllocator &Allocator, int16_t Value) {
	m_Type = Number;
	return SetValue(Allocator, LWUTF8C_View<16>(u8"{}", Value));
}

template<>
inline LWEJObject &LWEJObject::SetTypeValue<uint16_t>(LWAllocator &Allocator, uint16_t Value) {
	m_Type = Number;
	return SetValue(Allocator, LWUTF8C_View<16>(u8"{}", Value));
}

template<>
inline LWEJObject &LWEJObject::SetTypeValue<int32_t>(LWAllocator &Allocator, int32_t Value) {
	m_Type = Number;
	return SetValue(Allocator, LWUTF8C_View<16>(u8"{}", Value));
}

template<>
inline LWEJObject &LWEJObject::SetTypeValue<uint32_t>(LWAllocator &Allocator, uint32_t Value) {
	m_Type = Number;
	return SetValue(Allocator, LWUTF8C_View<16>(u8"{}", Value));
}

template<>
inline LWEJObject &LWEJObject::SetTypeValue<int64_t>(LWAllocator &Allocator, int64_t Value) {
	m_Type = Number;
	return SetValue(Allocator, LWUTF8C_View<16>(u8"{}", Value));
}

template<>
inline LWEJObject &LWEJObject::SetTypeValue<uint64_t>(LWAllocator &Allocator, uint64_t Value) {
	m_Type = Number;
	return SetValue(Allocator, LWUTF8C_View<16>(u8"{}", Value));
}

template<>
inline LWEJObject &LWEJObject::SetTypeValue<float>(LWAllocator &Allocator, float Value) {
	m_Type = Number;
	return SetValue(Allocator, LWUTF8C_View<16>(u8"{}", Value));
}

template<>
inline LWEJObject &LWEJObject::SetTypeValue<double>(LWAllocator &Allocator, double Value) {
	m_Type = Number;
	return SetValue(Allocator, LWUTF8C_View<16>(u8"{}", Value));
}


// @endcond

#endif
