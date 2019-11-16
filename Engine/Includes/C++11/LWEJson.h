#ifndef LWEJSON
#define LWEJSON
#include <unordered_map>
#include <LWCore/LWVector.h>
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
	char m_NameBuf[NameBufferLen];
	char m_ValueBuf[ValueBufferLen];
	char *m_Name = m_NameBuf;
	char *m_Value = m_ValueBuf;
	uint32_t m_NameBufferLen = NameBufferLen;
	uint32_t m_ValueBufferLen = ValueBufferLen;
	uint32_t m_Hash = 0;
	uint32_t m_ParentHash = 0;
	LWEJChild *m_Children = nullptr;
	uint32_t m_PoolSize = 0;
	uint32_t m_Length = 0;
	uint32_t m_Type = 0;

	LWEJObject &SetValue(LWAllocator &Allocator, const char *Value);

	LWEJObject &SetValuef(LWAllocator &Allocator, const char *Fmt, ...);

	LWEJObject &SetName(LWAllocator &Allocator, const char *Value);

	LWEJObject &SetNamef(LWAllocator &Allocator, const char *Fmt, ...);

	uint32_t FindChild(const char *ChildName);

	LWEJObject *FindChild(const char *ChildName, LWEJson &J);

	LWEJObject &PushChild(LWEJObject *Child, LWAllocator &Allocator);

	LWEJObject &InsertChild(uint32_t Position, LWEJObject *Child, LWAllocator &Allocator);

	int32_t AsInt(void);

	float AsFloat(void);

	bool AsBoolean(void);

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

	LWEJObject &operator = (LWEJObject &&O);

	LWEJObject(LWEJObject &&O);

	LWEJObject() = default;

	LWEJObject(const char *Name, const char *Value, uint32_t Type, uint32_t FullHash, uint32_t ParentHash, LWAllocator &Allocator);

	~LWEJObject();

};

class LWEJson {
public:
	enum {
		ElementPoolSize = 64
	};

	static uint32_t EscapeString(const char *String, char *Buffer, uint32_t BufferLen);

	static uint32_t UnEscapeString(const char *String, char *Buffer, uint32_t BufferLen);

	static bool Parse(LWEJson &JSon, const char *Buffer, LWEJObject *Parent = nullptr);

	uint32_t Serialize(char *Buffer, uint32_t BufferLen, bool Format);

	LWEJObject *MakeElement(const char *Name, LWEJObject *Parent = nullptr);

	LWEJObject *MakeElementf(const char *Fmt, LWEJObject *Parent, ...);

	LWEJson &PushRootElement(LWEJObject *Object);

	LWEJson &InsertRootElement(uint32_t Position, LWEJObject *Object);

	LWEJson &SetType(uint32_t Type);

	LWEJObject *GetElement(uint32_t i, LWEJObject *Parent = nullptr);

	uint32_t GetLength(void);

	uint32_t GetType(void);

	LWEJObject *operator[](const char *Name);

	LWEJObject *operator[](uint32_t NameHash);

	LWEJObject *Findf(const char *Fmt, ...);

	LWEJObject *Find(const char *Name);

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

#endif
