#include "LWEGLTFParser.h"
#include <LWPlatform/LWFileStream.h>
#include <LWVideo/LWTexture.h>
#include <LWCore/LWTimer.h>
#include <LWCore/LWByteBuffer.h>
#include <LWCore/LWCrypto.h>
#include <LWCore/LWLogger.h>
#include <functional>

#pragma region LWEGLTFANIMTWEEN
LWMatrix4f LWEGLTFAnimTween::GetFrame(float Time, bool Loop) {
	if (Loop) {
		float Total = GetTotalTime();
		if (Total>0.0f) Time = (float)fmodf(Time,Total);
	}
	LWVector3f Trans = m_Translation.GetValue(Time);
	LWVector3f Scale = m_Scale.GetValue(Time, LWVector3f(1.0f));
	LWQuaternionf Rot = m_Rotation.GetValue(Time);
	LWMatrix4f Mat = LWMatrix4f(Rot, Scale, Trans).Transpose3x3();
	return Mat;
}

float LWEGLTFAnimTween::GetTotalTime(void) const {
	return std::max<float>(std::max<float>(m_Translation.GetTotalTime(), m_Scale.GetTotalTime()), m_Rotation.GetTotalTime());
}

#pragma endregion

#pragma region LWEGLTFACCESSORVIEW
LWEGLTFAccessorView::LWEGLTFAccessorView(uint8_t *Buffer, uint32_t ComponentCount, uint32_t ComponentType, uint32_t Count, bool Normalize) : m_Buffer(Buffer), m_ComponentCount(ComponentCount), m_ComponentType(ComponentType), m_Count(Count), m_Normalize(Normalize) {}
#pragma endregion

#pragma region LWEGLTFBUFFER
bool LWEGLTFBuffer::ParseJSON(LWEGLTFBuffer &Buf, LWEJson &J, LWEJObject *Obj, LWAllocator &Allocator, LWFileStream &FileStream, const char *BinChunk) {
	LWFileStream Stream;
	LWEJObject *JName = Obj->FindChild("name", J);
	LWEJObject *JURI = Obj->FindChild("uri", J);
	LWEJObject *JByteLength = Obj->FindChild("byteLength", J);
	uint8_t *Buffer = nullptr;
	uint32_t Length = 0;
	const uint32_t SupportedEmbedCount = 1;
	const uint32_t DataHash = LWUTF8Iterator(u8"data:").Hash();
	const uint32_t SupportedEmbedFormats[SupportedEmbedCount] = { LWUTF8Iterator(u8"application/octet-stream;base64,").Hash() };
	const uint32_t SupportedTextLengths[SupportedEmbedCount] = { 32 };
	if(!LWLogCriticalIf(JByteLength, "No buffer bytelength found.")) return false;
	if (!JURI) {
		if(!LWLogCriticalIf(BinChunk, "glb does not have a binary chunk.")) return false;
		Length = JByteLength->AsInt();
		Buffer = Allocator.Allocate<uint8_t>(Length);
		std::copy(BinChunk, BinChunk + Length, Buffer);
	} else {
		uint32_t EmbedHash = LWCrypto::HashFNV1A((uint8_t*)JURI->m_Value, 5);
		if (EmbedHash == DataHash) {
			uint32_t i = 0;
			for (; i < SupportedEmbedCount; i++) {
				if (JURI->m_ValueBufferLen < SupportedTextLengths[i] + 5) continue;
				if (LWCrypto::HashFNV1A((uint8_t*)JURI->m_Value + 5, SupportedTextLengths[i]) == SupportedEmbedFormats[i]) break;
			}
			if(!LWLogCriticalIf<256>(i<SupportedEmbedCount, "buffer has unsupported embed type: '{}'", JURI->GetValue())) return false;
			char *Data = JURI->m_Value + 5 + SupportedTextLengths[i];
			uint32_t DataLen = (uint32_t)strlen(Data);
			if (i == 0) {
				Length = LWCrypto::Base64Decode(Data, DataLen, nullptr, 0);
				Buffer = Allocator.Allocate<uint8_t>(Length);
				LWCrypto::Base64Decode(Data, DataLen, (char*)Buffer, Length);
			}
		} else {
			if (!LWFileStream::OpenStream(Stream, JURI->GetValue(), LWFileStream::ReadMode | LWFileStream::BinaryMode, Allocator, &FileStream)) {
				LWLogCritical<256>("could not open buffer file: '{}'", JURI->GetValue());
				return false;
			}
			Length = Stream.Length();
			Buffer = Allocator.Allocate<uint8_t>(Length);
			Stream.Read((char*)Buffer, Length);
		}
	}
	Buf = LWEGLTFBuffer(JName ? JName->GetValue() : LWUTF8Iterator(), Buffer, Length);
	return true;
}

LWEGLTFBuffer &LWEGLTFBuffer::SetName(const LWUTF8Iterator &Name) {
	Name.Copy(m_Name, sizeof(m_Name));
	m_NameHash = LWUTF8I(m_Name).Hash();
	return *this;
}

LWUTF8Iterator LWEGLTFBuffer::GetName(void) const {
	return LWUTF8Iterator(m_Name);
}

LWEGLTFBuffer &LWEGLTFBuffer::operator = (LWEGLTFBuffer &&O) {
	O.GetName().Copy(m_Name, sizeof(m_Name));
	m_NameHash = O.m_NameHash;
	m_Buffer = O.m_Buffer;
	m_Length = O.m_Length;
	O.m_Buffer = nullptr;
	return *this;
}

LWEGLTFBuffer::LWEGLTFBuffer(LWEGLTFBuffer &&O) : m_Buffer(O.m_Buffer), m_Length(O.m_Length), m_NameHash(O.m_NameHash) {
	O.GetName().Copy(m_Name, sizeof(m_Name));
	O.m_Buffer = nullptr;
}

LWEGLTFBuffer::LWEGLTFBuffer(const LWUTF8Iterator &Name, uint8_t *Buffer, uint32_t Length) : m_Buffer(Buffer), m_Length(Length) {
	SetName(Name);
}

LWEGLTFBuffer::~LWEGLTFBuffer() {
	LWAllocator::Destroy(m_Buffer);
}
#pragma endregion

#pragma region LWEGLTFBUFFERVIEW
bool LWEGLTFBufferView::ParseJSON(LWEGLTFBufferView &BufView, LWEJson &J, LWEJObject *Obj) {
	LWEJObject *JBuffer = Obj->FindChild("buffer", J);
	LWEJObject *JByteLength = Obj->FindChild("byteLength", J);
	LWEJObject *JByteOffset = Obj->FindChild("byteOffset", J);
	LWEJObject *JByteStride = Obj->FindChild("byteStride", J);
	if(!LWLogCriticalIf(JBuffer, "Bufferview does not have 'buffer' field.")) return false;
	if(!LWLogCriticalIf(JByteLength, "Bufferview does not have 'byteLength' field.")) return false;

	BufView = LWEGLTFBufferView(JBuffer->AsInt(), JByteOffset ? JByteOffset->AsInt() : 0, JByteLength->AsInt(), JByteStride ? JByteStride->AsInt() : 0);
	return true;
}

LWEGLTFBufferView::LWEGLTFBufferView(uint32_t BufferID, uint32_t Offset, uint32_t Length, uint32_t Stride) : m_BufferID(BufferID), m_Offset(Offset), m_Length(Length), m_Stride(Stride) {}
#pragma endregion

#pragma region LWEGLTFACCESSOR
bool LWEGLTFAccessor::ParseJSON(LWEGLTFAccessor &Buf, LWEJson &J, LWEJObject *Obj) {
	//Values pre recorded from LWText::MakeHash: 
	//                            "Scalar",   "Vec2",     "Vec3",     "Vec4",    "Mat2",      "Mat3",     "Mat4"
	const uint32_t Typehashs[] = { 0x630f585d, 0xc9620e95, 0xc8620d02, 0xc3620523, 0xf41f6dbd, 0xf31f6c2a, 0xee1f644b };
	const uint32_t BaseTypeOffset = 0x1400;
	const uint32_t TypeCnt = 7;
	LWEJObject *JBufferView = Obj->FindChild("bufferView", J);
	LWEJObject *JByteOffset = Obj->FindChild("byteOffset", J);
	LWEJObject *JNormalized = Obj->FindChild("normalized", J);
	LWEJObject *JCount = Obj->FindChild("count", J);
	LWEJObject *JComponentType = Obj->FindChild("componentType", J);
	LWEJObject *JType = Obj->FindChild("type", J);
	if(!LWLogCriticalIf(JCount, "accessor does not have 'count' field.")) return false;
	if(!LWLogCriticalIf(JComponentType, "accessor does not have 'componentType' field.")) return false;
	if(!LWLogCriticalIf(JType, "accessor does not have 'type' field.")) return false;

	uint32_t TypeHash = JType->GetValue().Hash();
	uint32_t Type = 0;
	for (; Type < TypeCnt && TypeHash != Typehashs[Type]; Type++) {}
	uint32_t Flag = 0;
	if (JNormalized) Flag |= JNormalized->AsBoolean() ? Normalized : 0;
	Buf = LWEGLTFAccessor(JBufferView ? JBufferView->AsInt() : -1, JByteOffset ? JByteOffset->AsInt() : 0, JComponentType->AsInt() - BaseTypeOffset, JCount->AsInt(), Type, Flag);
	return true;
}

LWEGLTFAccessor::LWEGLTFAccessor(uint32_t BufferID, uint32_t Offset, uint32_t ComponentType, uint32_t Count, uint32_t Type, uint32_t Flag) : m_BufferID(BufferID), m_Offset(Offset), m_ComponentType(ComponentType), m_Count(Count), m_Type(Type), m_Flag(Flag) {}
#pragma endregion

#pragma region LWEGLTFCAMERAORTHO
bool LWEGLTFCameraOrtho::ParseJSON(LWEGLTFCameraOrtho &Ortho, LWEJson &J, LWEJObject *Obj) {
	LWEJObject *JXMag = Obj->FindChild("xmag", J);
	LWEJObject *JYMag = Obj->FindChild("ymag", J);
	LWEJObject *JZFar = Obj->FindChild("zfar", J);
	LWEJObject *JZNear = Obj->FindChild("znear", J);
	if (!LWLogCriticalIf(JXMag, "ortho camera does not have 'xmag' field.")) return false;
	if (!LWLogCriticalIf(JYMag, "ortho camera does not have 'ymag' field.")) return false;
	if (!LWLogCriticalIf(JZFar, "ortho camera does not have 'zfar' field.")) return false;
	if (!LWLogCriticalIf(JZNear, "ortho camera does not have 'znear' field.")) return false;
	Ortho = LWEGLTFCameraOrtho(JXMag->AsFloat(), JYMag->AsFloat(), JZNear->AsFloat(), JZFar->AsFloat());
	return true;
}

LWMatrix4f LWEGLTFCameraOrtho::GetMatrix(void) {
	return LWMatrix4f::Ortho(-m_xMag, m_xMag, -m_yMag, m_yMag, m_zNear, m_zFar);
};

LWEGLTFCameraOrtho::LWEGLTFCameraOrtho(float xMag, float yMag, float zNear, float zFar) : m_xMag(xMag), m_yMag(yMag), m_zNear(zNear), m_zFar(zFar) {}
#pragma endregion

#pragma region LWEGLTFCAMERAPERSPECTIVE
bool LWEGLTFCameraPerspective::ParseJSON(LWEGLTFCameraPerspective &Persp, LWEJson &J, LWEJObject *Obj) {
	LWEJObject *JAspect = Obj->FindChild("aspectRatio", J);
	LWEJObject *JFOV = Obj->FindChild("yfov", J);
	LWEJObject *JZNear = Obj->FindChild("znear", J);
	LWEJObject *JZFar = Obj->FindChild("zfar", J);
	if(!LWLogCriticalIf(JZNear, "perspective camera does not have 'znear' field.")) return false;
	if(!LWLogCriticalIf(JFOV, "perspective camera does not have 'yfov' field.")) return false;
	Persp = LWEGLTFCameraPerspective(JFOV->AsFloat(), JZNear->AsFloat());
	if (JAspect) Persp.m_Aspect = JAspect->AsFloat();
	if (JZFar) Persp.m_ZFar = JZFar->AsFloat();
	return true;
}

LWMatrix4f LWEGLTFCameraPerspective::GetMatrix(void) {
	return LWMatrix4f::Perspective(m_FOV, m_Aspect, m_zNear, m_ZFar);
}

LWMatrix4f LWEGLTFCameraPerspective::GetMatrix(float Aspect) {
	return LWMatrix4f::Perspective(m_FOV, Aspect, m_zNear, m_ZFar);
}

LWEGLTFCameraPerspective::LWEGLTFCameraPerspective(float FOV, float zNear) : m_FOV(FOV), m_zNear(zNear) {}
#pragma endregion

#pragma region LWEGLTFCAMERA

bool LWEGLTFCamera::ParseJSON(LWEGLTFCamera &Camera, LWEJson &J, LWEJObject *Obj) {
	LWEJObject *JType = Obj->FindChild("type", J);
	LWEJObject *JName = Obj->FindChild("name", J);
	LWEJObject *JPerspectiveCam = Obj->FindChild("perspective", J);
	LWEJObject *JOrthoCam = Obj->FindChild("orthographic", J);
	if(!LWLogCriticalIf(JType, "camera does not have 'type' field.")) return false;
	uint32_t TypeID = JType->GetValue().Hash();
	Camera = LWEGLTFCamera(JName ? JName->GetValue() : LWUTF8Iterator(), TypeID);
	if (TypeID == perspective){
		if(!LWLogCriticalIf(JPerspectiveCam, "camera is marked perspective with no perspective component.")) return false;
		if (!LWEGLTFCameraPerspective::ParseJSON(Camera.m_Perspective, J, JPerspectiveCam)) return false;
	} else if (TypeID == orthographic) {
		if(!LWLogCriticalIf(JOrthoCam, "camera is marked orthographic with no orthographic component.")) return false;
		if (!LWEGLTFCameraOrtho::ParseJSON(Camera.m_Ortho, J, Obj)) return false;
	} else {
		LWLogCritical<256>("camera type is unknown(or not supported): '{}' ({}).", JType->GetValue(), TypeID);
		return false;
	}
	return true;
}

LWEGLTFCamera &LWEGLTFCamera::SetName(const LWUTF8Iterator &Name) {
	Name.Copy(m_Name, sizeof(m_Name));
	m_NameHash = LWUTF8I(m_Name).Hash();
	return *this;
}

LWUTF8Iterator LWEGLTFCamera::GetName(void) const {
	return LWUTF8Iterator(m_Name);
}

LWMatrix4f LWEGLTFCamera::GetMatrix(void) {
	if (m_CameraType == perspective) return m_Perspective.GetMatrix();
	return m_Ortho.GetMatrix();
}

LWMatrix4f LWEGLTFCamera::GetMatrix(float Aspect) {
	if (m_CameraType == perspective) return m_Perspective.GetMatrix(Aspect);
	return m_Ortho.GetMatrix();
}

LWEGLTFCamera::LWEGLTFCamera(const LWUTF8Iterator &Name, uint32_t CameraType) : m_CameraType(CameraType) {
	SetName(Name);
}
#pragma endregion

#pragma region LWEGLTFATTRIBUTE
bool LWEGLTFAttribute::ParseJSON(LWEGLTFAttribute &Attribute, LWEJson &J, LWEJObject *Obj) {
	Attribute = LWEGLTFAttribute(Obj->AsInt(), Obj->GetName().Hash());
	return true;
}

LWEGLTFAttribute::LWEGLTFAttribute(uint32_t AccessorID, uint32_t AttributeHash) : m_AccessorID(AccessorID), m_AttributeHash(AttributeHash) {}
#pragma endregion


#pragma region LWEGLTFPRIMITIVE

bool LWEGLTFPrimitive::ParseJSON(LWEGLTFPrimitive &Primitive, LWEJson &J, LWEJObject *Obj) {
	LWEJObject *JAttributes = Obj->FindChild("attributes", J);
	LWEJObject *JIndices = Obj->FindChild("indices", J);
	LWEJObject *JMaterial = Obj->FindChild("material", J);
	if(!LWLogCriticalIf(JAttributes, "primitive does not have 'attributes' field.")) return false;
	uint32_t AttributeCnt = std::min<uint32_t>(MaxAttributes, JAttributes->m_Length);
	Primitive = LWEGLTFPrimitive(JMaterial ? JMaterial->AsInt() : -1, JIndices ? JIndices->AsInt() : -1, AttributeCnt);
	for (uint32_t i = 0; i < AttributeCnt; i++) {
		LWEGLTFAttribute::ParseJSON(Primitive.m_AttributeList[i], J, J.GetElement(i, JAttributes));
	}
	return true;
}

uint32_t LWEGLTFPrimitive::FindAttributeAccessor(uint32_t AttributeHash) {
	for (uint32_t i = 0; i < m_AttributeCount; i++) {
		if (m_AttributeList[i].m_AttributeHash == AttributeHash) return m_AttributeList[i].m_AccessorID;
	}
	return -1;
}

LWEGLTFPrimitive::LWEGLTFPrimitive(uint32_t MaterialID, uint32_t IndiceID, uint32_t AttributeCount) : m_MaterialID(MaterialID), m_IndiceID(IndiceID), m_AttributeCount(AttributeCount) {}
#pragma endregion

#pragma region LWEGLTFMesh
bool LWEGLTFMesh::ParseJSON(LWEGLTFMesh &Mesh, LWEJson &J, LWEJObject *Obj) {
	LWEJObject *JPrimitives = Obj->FindChild("primitives", J);
	LWEJObject *JNames = Obj->FindChild("name", J);
	if(!LWLogCriticalIf(JPrimitives, "mesh does not have a primitive list.")) return false;

	Mesh = LWEGLTFMesh(JNames ? JNames->m_Value : "", JPrimitives->m_Length);
	for (uint32_t i = 0; i < JPrimitives->m_Length; i++) {
		LWEGLTFPrimitive Prim;
		if (!LWEGLTFPrimitive::ParseJSON(Prim, J, J.GetElement(i, JPrimitives))) return false;
		Mesh.m_Primitives.push_back(Prim);
	}
	return true;
}

LWEGLTFMesh &LWEGLTFMesh::SetName(const LWUTF8Iterator &Name) {
	Name.Copy(m_Name, sizeof(m_Name));
	m_NameHash = LWUTF8I(m_Name).Hash();
	return *this;
}

LWUTF8Iterator LWEGLTFMesh::GetName(void) const {
	return m_Name;
}

LWEGLTFMesh::LWEGLTFMesh(const LWUTF8Iterator &Name, uint32_t PrimitiveCount) {
	SetName(Name);
	m_Primitives.reserve(PrimitiveCount);
}
#pragma endregion

#pragma region LWEGLTFIMAGE
bool LWEGLTFImage::ParseJSON(LWEGLTFImage &Img, LWEJson &J, LWEJObject *Obj, LWFileStream &Stream) {
	LWEJObject *JName = Obj->FindChild("name", J);
	LWEJObject *JUri = Obj->FindChild("uri", J);
	LWEJObject *JMime = Obj->FindChild("mimeType", J);
	LWEJObject *JBufferView = Obj->FindChild("bufferView", J);
	Img = LWEGLTFImage(JName ? JName->GetValue() : LWUTF8Iterator(), LWUTF8Iterator(), JMime ? JMime->GetValue().Hash() : MimeNone, JBufferView ? JBufferView->AsInt() : -1);
	if (JUri) {
		LWUTF8Iterator Dir, Name;
		LWFileStream::SplitPath(*Stream.GetFilePath(), Dir, Name);
		uint32_t Len = std::min<uint32_t>((uint32_t)fmt::format_to_n(Img.m_URI, sizeof(Img.m_URI), "{}{}", Dir, JUri->GetValue()).size, sizeof(Img.m_URI)-1);
		Img.m_URI[Len] = '\0';
	}
	return true;
}

LWEGLTFImage &LWEGLTFImage::SetName(const LWUTF8Iterator &Name) {
	Name.Copy(m_Name, sizeof(m_Name));
	m_NameHash = LWUTF8I(m_Name).Hash();
	return *this;
}

LWEGLTFImage &LWEGLTFImage::SetURI(const LWUTF8Iterator &URI) {
	URI.Copy(m_URI, sizeof(m_URI));
	return *this;
}

LWUTF8Iterator LWEGLTFImage::GetName(void) const {
	return m_Name;
}

LWUTF8Iterator LWEGLTFImage::GetURI(void) const {
	return m_URI;
}


LWEGLTFImage::LWEGLTFImage(const LWUTF8Iterator &Name, const LWUTF8Iterator &URI, uint32_t MimeType, uint32_t BufferView) : m_MimeType(MimeType), m_BufferView(BufferView){
	SetName(Name).SetURI(URI);
};
#pragma endregion

#pragma region LWEGLTFTEXTURE
bool LWEGLTFTexture::ParseJSON(LWEGLTFTexture &Tex, LWEJson &J, LWEJObject *Obj) {
	auto ParseSampler = [](LWEJson &J, LWEJObject *Node)->uint32_t {
		const uint32_t GLTF_CLAMP_TO_EDGE = 0x812F;
		const uint32_t GLTF_CLAMP_TO_BORDER = 0x812D;
		const uint32_t GLTF_MIRROR_REPEAT = 0x8370;
		const uint32_t GLTF_REPEAT = 0x2901;
		const uint32_t GLTF_NEAREST = 0x2600;
		const uint32_t GLTF_LINEAR = 0x2601;
		const uint32_t GLTF_NEAREST_MIPMAP_NEAREST = 0x2700;
		const uint32_t GLTF_NEAREST_MIPMAP_LINEAR = 0x2701;
		const uint32_t GLTF_LINEAR_MIPMAP_NEAREST = 0x2702;
		const uint32_t GLTF_LINEAR_MIPMAP_LINEAR = 0x2703;

		uint32_t MinFilter = LWTexture::MinNearestMipmapNearest;
		uint32_t MagFilter = LWTexture::MagNearest;
		uint32_t WrapS = LWTexture::WrapSRepeat;
		uint32_t WrapT = LWTexture::WrapTRepeat;
		uint32_t WrapR = LWTexture::WrapTRepeat;

		LWEJObject *JMagFilter = Node->FindChild("magFilter", J);
		LWEJObject *JMinFilter = Node->FindChild("minFilter", J);
		LWEJObject *JWrapS = Node->FindChild("wrapS", J);
		LWEJObject *JWrapT = Node->FindChild("wrapT", J);
		LWEJObject *JWrapR = Node->FindChild("WrapR", J);
		if (JMagFilter) {
			uint32_t ID = JMagFilter->AsInt();
			if (ID == GLTF_NEAREST) MagFilter = LWTexture::MagNearest;
			else if (ID == GLTF_LINEAR) MagFilter = LWTexture::MagLinear;
		}
		if (JMinFilter) {
			uint32_t ID = JMinFilter->AsInt();
			if (ID == GLTF_NEAREST) MinFilter = LWTexture::MinNearest;
			else if (ID == GLTF_LINEAR) MinFilter = LWTexture::MinLinear;
			else if (ID == GLTF_NEAREST_MIPMAP_NEAREST) MinFilter = LWTexture::MinNearestMipmapNearest;
			else if (ID == GLTF_NEAREST_MIPMAP_LINEAR) MinFilter = LWTexture::MinNearestMipmapLinear;
			else if (ID == GLTF_LINEAR_MIPMAP_NEAREST) MinFilter = LWTexture::MinLinearMipmapNearest;
			else if (ID == GLTF_LINEAR_MIPMAP_LINEAR) MinFilter = LWTexture::MinLinearMipmapLinear;
		}
		if (JWrapS) {
			uint32_t ID = JWrapS->AsInt();
			if (ID == GLTF_CLAMP_TO_EDGE) WrapS = LWTexture::WrapSClampToEdge;
			else if (ID == GLTF_CLAMP_TO_BORDER) WrapS = LWTexture::WrapSClampToBorder;
			else if (ID == GLTF_MIRROR_REPEAT) WrapS = LWTexture::WrapSMirroredRepeat;
			else if (ID == GLTF_REPEAT) WrapS = LWTexture::WrapSRepeat;
		}

		if (JWrapT) {
			uint32_t ID = JWrapT->AsInt();
			if (ID == GLTF_CLAMP_TO_EDGE) WrapT = LWTexture::WrapTClampToEdge;
			else if (ID == GLTF_CLAMP_TO_BORDER) WrapT = LWTexture::WrapTClampToBorder;
			else if (ID == GLTF_MIRROR_REPEAT) WrapT = LWTexture::WrapTMirroredRepeat;
			else if (ID == GLTF_REPEAT) WrapT = LWTexture::WrapTRepeat;
		}
		if (JWrapR) {
			uint32_t ID = JWrapR->AsInt();
			if (ID == GLTF_CLAMP_TO_EDGE) WrapR = LWTexture::WrapRClampToEdge;
			else if (ID == GLTF_CLAMP_TO_BORDER) WrapR = LWTexture::WrapRClampToBorder;
			else if (ID == GLTF_MIRROR_REPEAT) WrapR = LWTexture::WrapRMirroredRepeat;
			else if (ID == GLTF_REPEAT) WrapR = LWTexture::WrapRRepeat;
		}

		return MinFilter | MagFilter | WrapS | WrapT | WrapR;
	};

	auto ParseMSFTtextureddsExtension = [](LWEGLTFTexture &Tex, LWEJson &J, LWEJObject *ExtObj) {
		LWEJObject *JSource = ExtObj->FindChild("source", J);
		if (JSource) Tex.m_ImageID = JSource->AsInt();
	};

	LWEJObject *JSource = Obj->FindChild("source", J);
	LWEJObject *JSampler = Obj->FindChild("sampler", J);
	LWEJObject *JName = Obj->FindChild("name", J);
	LWEJObject *JExtension = Obj->FindChild("extensions", J);


	uint32_t SamplerFlag = LWTexture::MinNearestMipmapNearest | LWTexture::MagNearest | LWTexture::WrapSRepeat | LWTexture::WrapTRepeat | LWTexture::WrapTRepeat;
	if (JSampler) SamplerFlag = ParseSampler(J, JSampler);
	Tex = LWEGLTFTexture(JName ? JName->GetValue() : LWUTF8Iterator(), JSource ? JSource->AsInt() : -1, SamplerFlag);
	if (JExtension) {
		LWEJObject *JMsftExtension = JExtension->FindChild("MSFT_texture_dds", J);
		if (JMsftExtension) ParseMSFTtextureddsExtension(Tex, J, JMsftExtension);
	}
	return true;
}

LWEGLTFTexture &LWEGLTFTexture::SetName(const LWUTF8Iterator &Name) {
	Name.Copy(m_Name, sizeof(m_Name));
	m_NameHash = LWUTF8I(m_Name).Hash();
	return *this;
}

LWUTF8Iterator LWEGLTFTexture::GetName(void) const {
	return m_Name;
}

LWEGLTFTexture::LWEGLTFTexture(const LWUTF8Iterator &Name, uint32_t ImageID, uint32_t SamplerFlag) : m_ImageID(ImageID), m_SamplerFlag(SamplerFlag) {
	SetName(Name);
}
#pragma endregion

#pragma region LWEGLTFTEXTUREINFO

bool LWEGLTFTextureInfo::ParseJSON(LWEGLTFTextureInfo &TexInfo, LWEJson &J, LWEJObject *Obj) {

	auto ParseKHRTextureTransform = [](LWEGLTFTextureInfo &TexInfo, LWEJson &J, LWEJObject *Obj)->bool {
		LWEJObject *JOffset = Obj->FindChild("offset", J);
		LWEJObject *JScale = Obj->FindChild("scale", J);
		LWEJObject *JRotation = Obj->FindChild("rotation", J);
		LWEJObject *JTexCoord = Obj->FindChild("texCoord", J);

		if (JOffset) TexInfo.m_Offset = JOffset->AsVec2f(J);
		if (JScale) TexInfo.m_Scale = JScale->AsVec2f(J);
		if (JRotation) TexInfo.m_Rotation = JRotation->AsFloat();
		if (JTexCoord) TexInfo.m_TexCoord = JTexCoord->AsInt();
		return true;
	};

	LWEJObject *JIndex = Obj->FindChild("index", J);
	LWEJObject *JTexCoord = Obj->FindChild("texCoord", J);
	LWEJObject *JExtensions = Obj->FindChild("extensions", J);
	if(!LWLogCriticalIf(JIndex, "gltf texture info is missing 'index' field.")) return false;

	TexInfo = LWEGLTFTextureInfo(JIndex->AsInt());
	if (JTexCoord) TexInfo.m_TexCoord = JTexCoord->AsInt();
	if (JExtensions) {
		LWEJObject *KHR_Texture_Transform = JExtensions->FindChild("KHR_texture_transform", J);
		if (KHR_Texture_Transform) ParseKHRTextureTransform(TexInfo, J, KHR_Texture_Transform);
	}
	return true;
}

LWEGLTFTextureInfo::LWEGLTFTextureInfo(uint32_t TextureIndex) : m_TextureIndex(TextureIndex) {}
#pragma endregion

#pragma region LWEGLTFMATMETALROUGHNESS

bool LWEGLTFMatMetallicRoughness::ParseJSON(LWEGLTFMatMetallicRoughness &Mat, LWEJson &J, LWEJObject *Obj) {
	LWEJObject *JBaseColorFactor = Obj->FindChild("baseColorFactor", J);
	LWEJObject *JBaseColorTexture = Obj->FindChild("baseColorTexture", J);
	LWEJObject *JMetallicFactor = Obj->FindChild("metallicFactor", J);
	LWEJObject *JRoughnessFactor = Obj->FindChild("roughnessFactor", J);
	LWEJObject *JMetallicRoughnessTexture = Obj->FindChild("metallicRoughnessTexture", J);

	if (JBaseColorFactor) Mat.m_BaseColorFactor = JBaseColorFactor->AsVec4f(J);
	if (JBaseColorTexture) {
		if (!LWEGLTFTextureInfo::ParseJSON(Mat.m_BaseColorTexture, J, JBaseColorTexture)) return false;
	}
	if (JMetallicFactor) Mat.m_MetallicFactor = JMetallicFactor->AsFloat();
	if (JRoughnessFactor) Mat.m_RoughnessFactor = JRoughnessFactor->AsFloat();
	if (JMetallicRoughnessTexture) {
		if (!LWEGLTFTextureInfo::ParseJSON(Mat.m_MetallicRoughnessTexture, J, JMetallicRoughnessTexture)) return false;
	}
	return true;
}

LWEGLTFMatMetallicRoughness::LWEGLTFMatMetallicRoughness(const LWVector4f &BaseColorFactor, float MetallicFactor, float RoughnessFactor) : m_BaseColorFactor(BaseColorFactor), m_MetallicFactor(MetallicFactor), m_RoughnessFactor(RoughnessFactor) {}

#pragma endregion

#pragma region LWEGLTFMatSpecularGlossyness

bool LWEGLTFMatSpecularGlossyness::ParseJSON(LWEGLTFMatSpecularGlossyness &Mat, LWEJson &J, LWEJObject *Obj) {
	LWEJObject *JDiffusefactor = Obj->FindChild("diffuseFactor", J);
	LWEJObject *JDiffuseTexture = Obj->FindChild("diffuseTexture", J);
	LWEJObject *JSpecularFactor = Obj->FindChild("specularFactor", J);
	LWEJObject *JGlossinessFactor = Obj->FindChild("glossinessFactor", J);
	LWEJObject *JSpecularGlossinessTexture = Obj->FindChild("specularGlossinessTexture", J);
	if (JDiffusefactor) Mat.m_DiffuseFactor = JDiffusefactor->AsVec4f(J);
	if (JSpecularFactor) Mat.m_SpecularFactor = JSpecularFactor->AsVec3f(J);
	if (JGlossinessFactor) Mat.m_Glossiness = JGlossinessFactor->AsFloat();
	if (JDiffuseTexture) {
		if (!LWEGLTFTextureInfo::ParseJSON(Mat.m_DiffuseTexture, J, JDiffuseTexture)) return false;
	}
	if (JSpecularGlossinessTexture) {
		if (!LWEGLTFTextureInfo::ParseJSON(Mat.m_SpecularGlossyTexture, J, JSpecularGlossinessTexture)) return false;
	}
	return true;
}

LWEGLTFMatSpecularGlossyness::LWEGLTFMatSpecularGlossyness(const LWVector4f &DiffuseFactor, const LWVector3f &SpecularFactor, float Glossiness) : m_DiffuseFactor(DiffuseFactor), m_SpecularFactor(SpecularFactor), m_Glossiness(Glossiness) {}

#pragma endregion

#pragma region LWEGLTFMATERIAL

bool LWEGLTFMaterial::ParseJSON(LWEGLTFMaterial &Mat, LWEJson &J, LWEJObject *Obj) {
	const uint32_t AlphaModeCnt = 3;
	//Hashed with LWText::MakeHash       "Opaque",  "Mask",    "BLEND"
	const uint32_t AlphaModeHashs[AlphaModeCnt] = { 0x63d7221e,0x930d8be9,0x1ba85c38 };

	LWEJObject *JName = Obj->FindChild("name", J);
	LWEJObject *JPBRMetallicRoughness = Obj->FindChild("pbrMetallicRoughness", J);
	LWEJObject *JExtensions = Obj->FindChild("extensions", J);
	LWEJObject *JNormalTexture = Obj->FindChild("normalTexture", J);
	LWEJObject *JOcclusionTexture = Obj->FindChild("occlusionTexture", J);
	LWEJObject *JEmissiveTexture = Obj->FindChild("emissiveTexture", J);
	LWEJObject *JEmissiveFactor = Obj->FindChild("emissiveFactor", J);
	LWEJObject *JAlphaMode = Obj->FindChild("alphaMode", J);
	LWEJObject *JAlphaCutoff = Obj->FindChild("alphaCutoff", J);
	LWEJObject *JDoubleSided = Obj->FindChild("doubleSided", J);
	Mat = LWEGLTFMaterial(JName ? JName->GetValue() : LWUTF8Iterator());
	if (JPBRMetallicRoughness) {
		if (!LWEGLTFMatMetallicRoughness::ParseJSON(Mat.m_MetallicRoughness, J, JPBRMetallicRoughness)) return false;
		Mat.m_Flag = (Mat.m_Flag&~TypeBits) | MetallicRoughness;
	}
	if (JNormalTexture) {
		if (!LWEGLTFTextureInfo::ParseJSON(Mat.m_NormalMapTexture, J, JNormalTexture)) return false;
	}
	if (JOcclusionTexture) {
		if (!LWEGLTFTextureInfo::ParseJSON(Mat.m_OcclusionTexture, J, JOcclusionTexture)) return false;
	}
	if (JEmissiveTexture) {
		if (!LWEGLTFTextureInfo::ParseJSON(Mat.m_EmissiveTexture, J, JEmissiveTexture)) return false;
	}
	if(JEmissiveFactor) Mat.m_EmissiveFactor = LWVector4f(JEmissiveFactor->AsVec3f(J), 1.0f);
	if (JAlphaMode) {
		uint32_t AlphaMode = 0;
		uint32_t Hash = JAlphaMode->GetValue().Hash();
		for (; AlphaMode < AlphaModeCnt && Hash!=AlphaModeHashs[AlphaMode]; AlphaMode++) {}
		if(!LWLogCriticalIf<256>(AlphaMode<AlphaModeCnt, "unknown material alpha mode: '{}'", JAlphaMode->GetValue())) return false;

		Mat.m_Flag |= (AlphaMode << AlphaBitOffset);
	}
	if (JAlphaCutoff) Mat.m_AlphaCutoff = JAlphaCutoff->AsFloat();
	if (JDoubleSided) Mat.m_Flag |= JDoubleSided->AsBoolean() ? DoubleSided : 0;
	if (JExtensions) {
		LWEJObject *JKHR_Materials_Unlit = JExtensions->FindChild("KHR_materials_unlit", J);
		LWEJObject *JKHR_Materials_SpecularGlossiness = JExtensions->FindChild("KHR_materials_pbrSpecularGlossiness", J);
		if (JKHR_Materials_Unlit) {
			Mat.m_Flag = (Mat.m_Flag&~TypeBits) | Unlit;
		}
		if (JKHR_Materials_SpecularGlossiness) {
			if (!LWEGLTFMatSpecularGlossyness::ParseJSON(Mat.m_SpecularGlossy, J, JKHR_Materials_SpecularGlossiness)) return false;
			Mat.m_Flag = (Mat.m_Flag&~TypeBits) | SpecularGlossyness;
		}
	}	
	return true;
}

LWEGLTFMaterial &LWEGLTFMaterial::SetName(const LWUTF8Iterator &Name) {
	Name.Copy(m_Name, sizeof(m_Name));
	m_NameHash = LWUTF8I(m_Name).Hash();
	return *this;
}

uint32_t LWEGLTFMaterial::GetType(void) const {
	return (m_Flag&TypeBits);
}

LWUTF8Iterator LWEGLTFMaterial::GetName(void) const {
	return m_Name;
}

LWEGLTFMaterial::LWEGLTFMaterial(const LWUTF8Iterator &Name) {
	SetName(Name);
}
#pragma endregion

#pragma region LWEGLTFLIGHT

bool LWEGLTFLight::ParseJSON(LWEGLTFLight &L, LWEJson &J, LWEJObject *Obj) {
	LWEJObject *JName = Obj->FindChild("name", J);
	LWEJObject *JColor = Obj->FindChild("color", J);
	LWEJObject *JIntensisty = Obj->FindChild("intensity", J);
	LWEJObject *JSpot = Obj->FindChild("spot", J);
	LWEJObject *JType = Obj->FindChild("type", J);
	LWEJObject *JRange = Obj->FindChild("range", J);
	if (!JType) {
		LWLogCritical<256>("light does not specify type.");
		return false;
	}
	L = LWEGLTFLight(JName ? JName->GetValue() : LWUTF8Iterator(), JType->GetValue().Hash());
	if (JColor) L.m_Color = JColor->AsVec3f(J);
	if (JIntensisty) L.m_Intensity = JIntensisty->AsFloat();
	if (JRange) L.m_Range = JRange->AsFloat();
	if (JSpot) {
		LWEJObject *JInnerConeAngle = JSpot->FindChild("innerConeAngle", J);
		LWEJObject *JOuterConeAngle = JSpot->FindChild("outerConeAngle", J);
		if (JInnerConeAngle) L.m_InnerConeTheta = JInnerConeAngle->AsFloat();
		if (JOuterConeAngle) L.m_OuterConeTheta = JOuterConeAngle->AsFloat();
	}
	return true;
};


LWEGLTFLight &LWEGLTFLight::SetName(const LWUTF8Iterator &Name) {
	Name.Copy(m_Name, sizeof(m_Name));
	m_NameHash = LWUTF8I(m_Name).Hash();
	return *this;
}

LWUTF8Iterator LWEGLTFLight::GetName(void) const {
	return m_Name;
}

LWEGLTFLight::LWEGLTFLight(const LWUTF8Iterator &Name, uint32_t Type) : m_Type(Type) {
	SetName(Name);
}
#pragma endregion

#pragma region LWEGLTFNODE
bool LWEGLTFNode::ParseJSON(LWEGLTFNode &Node, LWEJson &J, LWEJObject *Obj) {
	LWEJObject *JName = Obj->FindChild("name", J);
	LWEJObject *JChildren = Obj->FindChild("children", J);
	LWEJObject *JSkin = Obj->FindChild("skin", J);
	LWEJObject *JMatrix = Obj->FindChild("matrix", J);
	LWEJObject *JMesh = Obj->FindChild("mesh", J);
	LWEJObject *JCamera = Obj->FindChild("camera", J);
	LWEJObject *JRotation = Obj->FindChild("rotation", J);
	LWEJObject *JScale = Obj->FindChild("scale", J);
	LWEJObject *JTranslation = Obj->FindChild("translation", J);
	LWEJObject *JExtensions = Obj->FindChild("extensions", J);
	LWMatrix4f Matrix = LWMatrix4f();
	if (JMatrix) Matrix = JMatrix->AsMat4f(J);
	else {
		LWVector3f Translation = JTranslation ? JTranslation->AsVec3f(J) : LWVector3f();
		LWVector3f Scale = JScale ? JScale->AsVec3f(J) : LWVector3f(1.0f);
		LWQuaternionf Rot = JRotation ? JRotation->AsQuaternionf(J) : LWQuaternionf();
		Matrix = LWMatrix4f(Rot, Scale, Translation).Transpose3x3();
	}
	Node = LWEGLTFNode(JName ? JName->GetValue() : LWUTF8Iterator(), JMesh ? JMesh->AsInt() : -1, JSkin ? JSkin->AsInt() : -1, JCamera ? JCamera->AsInt():-1, JChildren ? JChildren->m_Length : 0, Matrix);
	if (JChildren) {
		for (uint32_t i = 0; i < JChildren->m_Length; i++) {
			LWEJObject *C = J.GetElement(i, JChildren);
			Node.m_Children.push_back(C->AsInt());
		}
	}
	if (JExtensions) {
		LWEJObject *JKHR_Lights_Punctual = JExtensions->FindChild("KHR_lights_punctual", J);
		if (JKHR_Lights_Punctual) {
			LWEJObject *JLight = JKHR_Lights_Punctual->FindChild("light", J);
			if (JLight) Node.m_LightID = JLight->AsInt();
		}
	}
	return true;
}


LWEGLTFNode &LWEGLTFNode::SetName(const LWUTF8Iterator &Name) {
	Name.Copy(m_Name, sizeof(m_Name));
	m_NameHash = LWUTF8I(m_Name).Hash();
	return *this;
}

LWUTF8Iterator LWEGLTFNode::GetName(void) const {
	return m_Name;
}

LWEGLTFNode::LWEGLTFNode(const LWUTF8Iterator &Name, uint32_t MeshID, uint32_t SkinID, uint32_t CameraID, uint32_t ChildrenCnt, const LWMatrix4f &TransformMatrix) : m_MeshID(MeshID), m_SkinID(SkinID), m_CameraID(CameraID), m_TransformMatrix(TransformMatrix){
	SetName(Name);
	m_Children.reserve(ChildrenCnt);
}

#pragma endregion

#pragma region LWEGLTFSCENE
bool LWEGLTFScene::ParseJSON(LWEGLTFScene &Scene, LWEJson &J, LWEJObject *Obj) {
	LWEJObject *JName = Obj->FindChild("name", J);
	LWEJObject *JNodes = Obj->FindChild("nodes", J);
	Scene = LWEGLTFScene(JName ? JName->GetValue() : LWUTF8Iterator(), JNodes ? JNodes->m_Length : 0);
	for (uint32_t i = 0; i < JNodes->m_Length; i++) {
		LWEJObject *C = J.GetElement(i, JNodes);
		Scene.m_NodeList.push_back(C->AsInt());
	}
	return true;
}

LWEGLTFScene &LWEGLTFScene::SetName(const LWUTF8Iterator &Name) {
	Name.Copy(m_Name, sizeof(m_Name));
	m_NameHash = LWUTF8I(m_Name).Hash();
	return *this;
}

LWUTF8Iterator LWEGLTFScene::GetName(void) const {
	return m_Name;
}

LWEGLTFScene::LWEGLTFScene(const LWUTF8Iterator &Name, uint32_t NodeCnt) {
	SetName(Name);
	m_NodeList.reserve(NodeCnt);
}

#pragma endregion

#pragma region LWEGLTFSKIN
bool LWEGLTFSkin::ParseJSON(LWEGLTFSkin &Skin, LWEJson &J, LWEJObject *Obj) {
	LWEJObject *JName = Obj->FindChild("name", J);
	LWEJObject *JInverseBindMatrices = Obj->FindChild("inverseBindMatrices", J);
	LWEJObject *JJoints = Obj->FindChild("joints", J);
	LWEJObject *JSkeleton = Obj->FindChild("skeleton", J);
	if(!LWLogCriticalIf(JJoints, "skin is missing 'joints' field.")) return false;

	Skin = LWEGLTFSkin(JName ? JName->GetValue() : LWUTF8Iterator(), JJoints->m_Length, JInverseBindMatrices ? JInverseBindMatrices->AsInt() : -1, JSkeleton ? JSkeleton->AsInt() : -1);
	for (uint32_t i = 0; i < JJoints->m_Length; i++) {
		LWEJObject *JJnt = J.GetElement(i, JJoints);
		Skin.m_JointList.push_back(JJnt->AsInt());
	}
	return true;
}


LWEGLTFSkin &LWEGLTFSkin::SetName(const LWUTF8Iterator &Name) {
	Name.Copy(m_Name, sizeof(m_Name));
	m_NameHash = LWUTF8I(m_Name).Hash();
	return *this;
}

LWUTF8Iterator LWEGLTFSkin::GetName(void) const {
	return m_Name;
}

LWEGLTFSkin::LWEGLTFSkin(const LWUTF8Iterator &Name, uint32_t JointCnt, uint32_t InverseBindMatrices, uint32_t SkeletonNode) : m_InverseBindMatrices(InverseBindMatrices), m_SkeletonNode(SkeletonNode) {
	SetName(Name);
	m_JointList.reserve(JointCnt);
}
#pragma endregion

#pragma region LWEGLTFANIMCHANNEL
bool LWEGLTFAnimChannel::ParseJSON(LWEGLTFAnimChannel &Channel, LWEJson &J, LWEJObject *Obj, LWEJObject *AnimObj) {
	auto ParseSampler = [](LWEGLTFAnimChannel &Channel, LWEJson &J, LWEJObject *Obj)->bool {
		LWEJObject *JInput = Obj->FindChild("input", J);
		LWEJObject *JOutput = Obj->FindChild("output", J);
		LWEJObject *JInterpolation = Obj->FindChild("interpolation", J);
		if(!LWLogCriticalIf(JInput, "Animation sampler is missing 'input' field.")) return false;
		if(!LWLogCriticalIf(JOutput, "Animation sampler is missing 'output' field.")) return false;

		Channel.m_InputID = JInput->AsInt();
		Channel.m_OutputID = JOutput->AsInt();
		if (JInterpolation) Channel.m_Interpolation = JInterpolation->GetValue().Hash();
		return true;
	};

	auto ParseTarget = [](LWEGLTFAnimChannel &Channel, LWEJson &J, LWEJObject *Obj)->bool {
		LWEJObject *JNode = Obj->FindChild("node", J);
		LWEJObject *JPath = Obj->FindChild("path", J);
		if(!LWLogCriticalIf(JPath, "Animation channel is missing 'path' field.")) return false;

		Channel.m_Path = JPath->GetValue().Hash();
		if (JNode) Channel.m_Node = JNode->AsInt();
		return true;
	};

	LWEJObject *JSampler = Obj->FindChild("sampler", J);
	LWEJObject *JTarget = Obj->FindChild("target", J);
	if(!LWLogCriticalIf(JSampler, "Animation channel is missing 'sampler' field.")) return false;
	if(!LWLogCriticalIf(JTarget, "Animation channel is missing 'target' field.")) return false;

	LWEJObject *JAnimSamplers = AnimObj->FindChild("samplers", J);
	if(!LWLogCriticalIf(JAnimSamplers, "Animation is missing 'samplers' field.")) return false;

	LWEJObject *JASampler = J.GetElement(JSampler->AsInt(), JAnimSamplers);
	if(!LWLogCriticalIf<256>(JASampler, "Animation sampler index is outside of bounds: {}", JSampler->AsInt())) return false;

	if (!ParseSampler(Channel, J, JASampler)) return false;
	if (!ParseTarget(Channel, J, JTarget)) return false;
	return true;
}

LWEGLTFAnimChannel::LWEGLTFAnimChannel(uint32_t InputID, uint32_t OutputID, uint32_t Interpolation, uint32_t Node, uint32_t Path) : m_InputID(InputID), m_OutputID(OutputID), m_Interpolation(Interpolation), m_Node(Node), m_Path(Path) {}
#pragma endregion

#pragma region LWEGLTFANIMATION
bool LWEGLTFAnimation::ParseJSON(LWEGLTFAnimation &Anim, LWEJson &J, LWEJObject *Obj) {
	LWEJObject *JName = Obj->FindChild("name", J);
	LWEJObject *JChannels = Obj->FindChild("channels", J);
	if(!LWLogCriticalIf(JChannels, "Animation is missing 'channels' field.")) return false;

	Anim = LWEGLTFAnimation(JName ? JName->GetValue() : LWUTF8Iterator(), JChannels->m_Length);
	for (uint32_t i = 0; i < JChannels->m_Length; i++) {
		LWEGLTFAnimChannel Channel;
		LWEJObject *JChannel = J.GetElement(i, JChannels);
		if (!LWEGLTFAnimChannel::ParseJSON(Channel, J, JChannel, Obj)) return false;
		Anim.m_Channels.push_back(Channel);
	}
	return true;
}

LWEGLTFAnimation &LWEGLTFAnimation::SetName(const LWUTF8Iterator &Name) {
	Name.Copy(m_Name, sizeof(m_Name));
	m_NameHash = LWUTF8I(m_Name).Hash();
	return *this;
}

LWUTF8Iterator LWEGLTFAnimation::GetName(void) const {
	return m_Name;
}

LWEGLTFAnimation::LWEGLTFAnimation(const LWUTF8Iterator &Name, uint32_t ChannelCnt) {
	SetName(Name);
	m_Channels.reserve(ChannelCnt);
}
#pragma endregion

bool LWEGLTFParser::LoadFile(LWEGLTFParser &Parser, const LWUTF8Iterator &Path, LWAllocator &Allocator) {
	uint32_t ExtensionID = LWFileStream::IsExtensions(Path, "gltf", "glb");
	if (ExtensionID == 0) return LoadFileGLTF(Parser, Path, Allocator);
	else if (ExtensionID == 1) return LoadFileGLB(Parser, Path, Allocator);
	return false;
}

bool LWEGLTFParser::LoadFileGLB(LWEGLTFParser &Parser, const LWUTF8Iterator &Path, LWAllocator &Allocator) {
	const uint32_t MagicID = 0x46546C67;
	const uint32_t ChunkJSON = 0x4E4F534A;
	const uint32_t ChunkBIN = 0x004E4942;
	const uint32_t SupportedVersion = 2;
	struct Header {
		uint32_t m_Magic;
		uint32_t m_Version;
		uint32_t m_Length;
	};
	struct Chunk {
		uint32_t m_Length;
		uint32_t m_Type;
		uint32_t m_Position;
	};

	auto ReadHeader = [](Header &H, LWByteBuffer &Buf)->bool {
		H.m_Magic = Buf.Read<uint32_t>();
		H.m_Version = Buf.Read<uint32_t>();
		H.m_Length = Buf.Read<uint32_t>();
		return true;
	};

	auto ReadChunk = [](Chunk &C, LWByteBuffer &Buf)->bool {
		C.m_Length = Buf.Read<uint32_t>();
		C.m_Type = Buf.Read<uint32_t>();
		C.m_Position = Buf.GetPosition();
		return true;
	};

	LWFileStream Stream;
	if(!LWLogCriticalIf<256>(LWFileStream::OpenStream(Stream, Path, LWFileStream::ReadMode|LWFileStream::BinaryMode, Allocator), "Could not open glb file: '{}'", Path)) return false;

	uint32_t Len = Stream.Length();
	if (Len < sizeof(Header)) return false;
	char8_t *Buffer = Allocator.Allocate<char8_t>(Len);
	Stream.Read(Buffer, Len);
	LWByteBuffer Buf = LWByteBuffer((int8_t*)Buffer, Len);
	
	Header H;
	ReadHeader(H, Buf);
	if (!LWLogCriticalIf<256>(H.m_Magic == MagicID, "glb magic header incorrect: '{}'", Path)) {
		LWAllocator::Destroy(Buffer);
		return false;
	}
	if (!LWLogCriticalIf<256>(H.m_Version == SupportedVersion, "glb has version: {} while glb parser only supports version {} for file: '{}'.", H.m_Version, SupportedVersion, Path)) {
		LWAllocator::Destroy(Buffer);
		return false;
	}
	//We only support 1 json and then 1 bin chunk(if attached).
	Chunk jsonChunk;
	Chunk binChunk;
	char *binChunkBuf = nullptr;
	ReadChunk(jsonChunk, Buf);
	if (!LWLogCriticalIf<256>(jsonChunk.m_Type == ChunkJSON, "glb is formatted incorrectly: '{}'", Path)) {
		LWAllocator::Destroy(Buffer);
		return false;
	}
	if ((Buf.GetPosition() + jsonChunk.m_Length) != Len) {
		Buf.Seek(jsonChunk.m_Length);
		ReadChunk(binChunk, Buf);
		binChunkBuf = Buffer + binChunk.m_Position;
	}
	LWEJson J(Allocator);
	if (!LWLogCriticalIf<256>(LWEJson::Parse(J, Buffer + jsonChunk.m_Position), "glb failed to parse gltf: '{}'", Path)) {
		LWAllocator::Destroy(Buffer);
		return false;
	}
	bool bResult = ParseJSON(Parser, J, Stream, binChunkBuf, Allocator);
	LWAllocator::Destroy(Buffer);
	return bResult;
}

bool LWEGLTFParser::LoadFileGLTF(LWEGLTFParser &Parser, const LWUTF8Iterator &Path, LWAllocator &Allocator) {

	LWFileStream Stream;
	if (!LWLogCriticalIf<256>(LWFileStream::OpenStream(Stream, Path, LWFileStream::ReadMode | LWFileStream::BinaryMode, Allocator), "Could not open gltf file: '{}'", Path)) return false;

	uint32_t Len = Stream.Length();
	char8_t *Buffer = Allocator.Allocate<char8_t>(Len);
	Stream.Read(Buffer, Len);
	LWEJson J(Allocator);
	if(!LWLogCriticalIf<256>(LWEJson::Parse(J, Buffer), "Failed to parse gltf file: '{}'", Path)) {
		LWAllocator::Destroy(Buffer);
		return false;
	}
	LWAllocator::Destroy(Buffer);
	return ParseJSON(Parser, J, Stream, nullptr, Allocator);
}

bool LWEGLTFParser::ParseJSON(LWEGLTFParser &Parser, LWEJson &J, LWFileStream &Stream, const char *BinChunk, LWAllocator &Allocator) {
	LWEJObject *JBuffers = J.Find("buffers");
	LWEJObject *JBufferViews = J.Find("bufferViews");
	LWEJObject *JAccessors = J.Find("accessors");
	LWEJObject *JImages = J.Find("images");
	LWEJObject *JTextures = J.Find("textures");
	LWEJObject *JMaterials = J.Find("materials");
	LWEJObject *JCameras = J.Find("cameras");
	LWEJObject *JMeshs = J.Find("meshes");
	LWEJObject *JSkins = J.Find("skins");
	LWEJObject *JAnims = J.Find("animations");
	LWEJObject *JNodes = J.Find("nodes");
	LWEJObject *JScenes = J.Find("scenes");
	LWEJObject *JScene = J.Find("scene");
	LWEJObject *JExtensions = J.Find("extensions");

	LWEJObject *JLights = nullptr;
	if (JExtensions) {
		LWEJObject *JKHR_Lights_Punctual = JExtensions->FindChild("KHR_lights_punctual", J);
		if (JKHR_Lights_Punctual) JLights = JKHR_Lights_Punctual->FindChild("lights", J);
	}

	Parser.SetDefaultScene(JScene ? JScene->AsInt() : -1);

	if (!LWLogCriticalIf<256>(JMeshs, "parsing glb '{}': No meshes structure found.", Stream.GetFilePath())) return false;
	if (!LWLogCriticalIf<256>(JNodes, "parsing glb '{}': No nodes structure found.", Stream.GetFilePath())) return false;
	if (!LWLogCriticalIf<256>(JScenes, "parsing glb '{}': No scenes structure found.", Stream.GetFilePath())) return false;

	if (JBuffers) {
		for (uint32_t i = 0; i < JBuffers->m_Length; i++) {
			LWEGLTFBuffer Buf;
			LWEJObject *JB = J.GetElement(i, JBuffers);
			if (!LWEGLTFBuffer::ParseJSON(Buf, J, JB, Allocator, Stream, BinChunk)) return false;
			Parser.PushBuffer(Buf);
		}
	}
	if (JBufferViews) {
		for (uint32_t i = 0; i < JBufferViews->m_Length; i++) {
			LWEGLTFBufferView BufView;
			LWEJObject *JBV = J.GetElement(i, JBufferViews);
			if (!LWEGLTFBufferView::ParseJSON(BufView, J, JBV)) return false;
			Parser.PushBufferView(BufView);
		}
	}

	if (JAccessors) {
		for (uint32_t i = 0; i < JAccessors->m_Length; i++) {
			LWEGLTFAccessor Acc;
			LWEJObject *JAcc = J.GetElement(i, JAccessors);
			if (!LWEGLTFAccessor::ParseJSON(Acc, J, JAcc)) return false;
			Parser.PushAccessor(Acc);
		}
	}

	if (JImages) {
		for (uint32_t i = 0; i < JImages->m_Length; i++) {
			LWEGLTFImage Img;
			LWEJObject *JImg = J.GetElement(i, JImages);
			if (!LWEGLTFImage::ParseJSON(Img, J, JImg, Stream)) return false;
			Parser.PushImage(Img);
		}
	}
	if (JTextures) {
		for (uint32_t i = 0; i < JTextures->m_Length; i++) {
			LWEGLTFTexture Tex;
			LWEJObject *JTex = J.GetElement(i, JTextures);
			if (!LWEGLTFTexture::ParseJSON(Tex, J, JTex)) return false;
			Parser.PushTexture(Tex);
		}
	}
	if (JMaterials) {
		for (uint32_t i = 0; i < JMaterials->m_Length; i++) {
			LWEGLTFMaterial Mat;
			LWEJObject *JMat = J.GetElement(i, JMaterials);
			if (!LWEGLTFMaterial::ParseJSON(Mat, J, JMat)) return false;
			Parser.PushMaterial(Mat);
		}
	}
	if (JCameras) {
		for (uint32_t i = 0; i < JCameras->m_Length; i++) {
			LWEGLTFCamera Cam;
			LWEJObject *JCam = J.GetElement(i, JCameras);
			if (!LWEGLTFCamera::ParseJSON(Cam, J, JCam)) return false;
			Parser.PushCamera(Cam);
		}
	}
	if (JSkins) {
		for (uint32_t i = 0; i < JSkins->m_Length; i++) {
			LWEGLTFSkin Skin;
			LWEJObject *JSkin = J.GetElement(i, JSkins);
			if (!LWEGLTFSkin::ParseJSON(Skin, J, JSkin)) return false;
			Parser.PushSkin(Skin);
		}
	}
	if (JAnims) {
		for (uint32_t i = 0; i < JAnims->m_Length; i++) {
			LWEGLTFAnimation Anim;
			LWEJObject *JAnim = J.GetElement(i, JAnims);
			if (!LWEGLTFAnimation::ParseJSON(Anim, J, JAnim)) return false;
			Parser.PushAnimation(Anim);
		}
	}
	if (JLights) {
		for (uint32_t i = 0; i < JLights->m_Length; i++) {
			LWEGLTFLight L;
			LWEJObject *JLight = J.GetElement(i, JLights);
			if (!LWEGLTFLight::ParseJSON(L, J, JLight)) return false;
			Parser.PushLight(L);
		}
	}
	for (uint32_t i = 0; i < JMeshs->m_Length; i++) {
		LWEGLTFMesh Mesh;
		LWEJObject *JMesh = J.GetElement(i, JMeshs);
		if (!LWEGLTFMesh::ParseJSON(Mesh, J, JMesh)) return false;
		Parser.PushMesh(Mesh);
	}

	for (uint32_t i = 0; i < JNodes->m_Length; i++) {
		LWEGLTFNode Node;
		LWEJObject *JNode = J.GetElement(i, JNodes);
		if (!LWEGLTFNode::ParseJSON(Node, J, JNode)) return false;
		Node.m_NodeID = i;
		Parser.PushNode(Node);
	}
	uint32_t NodeCnt = Parser.GetNodeCount();
	//Fix parents.
	for (uint32_t i = 0; i < NodeCnt; i++) {
		LWEGLTFNode *Node = Parser.GetNode(i);
		for (auto &&Iter : Node->m_Children) {
			LWEGLTFNode *CNode = Parser.GetNode(Iter);
			CNode->m_ParentID = i;
		}
	}

	for (uint32_t i = 0; i < JScenes->m_Length; i++) {
		LWEGLTFScene Scene;
		LWEJObject *JScene = J.GetElement(i, JScenes);
		if (!LWEGLTFScene::ParseJSON(Scene, J, JScene)) return false;
		Parser.PushScene(Scene);
	}
	return true;
}

LWEGLTFScene *LWEGLTFParser::BuildSceneOnlyList(uint32_t SceneID, std::vector<uint32_t> &NodeList, std::vector<uint32_t> &MeshList, std::vector<uint32_t> &SkinList, std::vector<uint32_t> &LightList, std::vector<uint32_t> &MaterialList, std::vector<uint32_t> &TextureList, std::vector<uint32_t> &ImageList) {
	LWEGLTFScene *Scene = GetScene(SceneID);
	if (!Scene) return nullptr;

	auto PushListItem = [](std::vector<uint32_t> &List, uint32_t ID)->bool {
		for (auto &&Iter : List) {
			if (Iter == ID) return false;
		}
		List.push_back(ID);
		return true;
	};

	auto EvaluateTexture = [this, &PushListItem, &ImageList, &TextureList](uint32_t TextureID)->bool {
		LWEGLTFTexture *Tex = GetTexture(TextureID);
		if (!Tex) return false;
		PushListItem(TextureList, TextureID);
		LWEGLTFImage *Img = GetImage(Tex->m_ImageID);
		if (!Img) return false;
		PushListItem(ImageList, Tex->m_ImageID);
		return true;
	};

	auto EvaluateMaterial = [this, &PushListItem, &EvaluateTexture, &MaterialList](uint32_t MaterialID)->bool {
		LWEGLTFMaterial *Mat = GetMaterial(MaterialID);
		if (!Mat) return false;
		PushListItem(MaterialList, MaterialID);
		EvaluateTexture(Mat->m_MetallicRoughness.m_BaseColorTexture.m_TextureIndex);
		EvaluateTexture(Mat->m_MetallicRoughness.m_MetallicRoughnessTexture.m_TextureIndex);
		EvaluateTexture(Mat->m_SpecularGlossy.m_DiffuseTexture.m_TextureIndex);
		EvaluateTexture(Mat->m_SpecularGlossy.m_SpecularGlossyTexture.m_TextureIndex);
		EvaluateTexture(Mat->m_NormalMapTexture.m_TextureIndex);
		EvaluateTexture(Mat->m_OcclusionTexture.m_TextureIndex);
		EvaluateTexture(Mat->m_EmissiveTexture.m_TextureIndex);
		return true;
	};

	std::function<void(uint32_t)> EvaluateNode = [this, &EvaluateNode, &PushListItem, &EvaluateMaterial, &NodeList, &MeshList, &SkinList, &LightList](uint32_t NodeID)->void {
		PushListItem(NodeList, NodeID);
		LWEGLTFNode *Node = GetNode(NodeID);
		LWEGLTFMesh *Mesh = GetMesh(Node->m_MeshID);
		LWEGLTFSkin *Skin = GetSkin(Node->m_SkinID);
		LWEGLTFLight *Light = GetLight(Node->m_LightID);
		if (Mesh) {
			PushListItem(MeshList, Node->m_MeshID);
			for (auto &&Iter : Mesh->m_Primitives) EvaluateMaterial(Iter.m_MaterialID);
		}
		if (Skin) {
			PushListItem(SkinList, Node->m_SkinID);
		}
		if (Light) {
			PushListItem(LightList, Node->m_LightID);
		}
		for (auto &&Iter : Node->m_Children) EvaluateNode(Iter);
		return;
	};
	for (auto &&Iter : Scene->m_NodeList) EvaluateNode(Iter);
	return Scene;
}

bool LWEGLTFParser::CreateAccessorView(LWEGLTFAccessorView &View, uint32_t AccessorID) {
	const uint32_t ComponentSizes[] = { 1,2,3,4, 4, 9, 16 };
	LWEGLTFAccessor *Accessor = GetAccessor(AccessorID);
	if (!Accessor) return false;
	LWEGLTFBufferView *BufView = GetBufferView(Accessor->m_BufferID);
	if (!BufView) return false;
	LWEGLTFBuffer *Buf = GetBuffer(BufView->m_BufferID);
	if (!Buf) return false;
	View = LWEGLTFAccessorView(Buf->m_Buffer + Accessor->m_Offset + BufView->m_Offset, ComponentSizes[Accessor->m_Type], Accessor->m_ComponentType, Accessor->m_Count, (Accessor->m_Flag&LWEGLTFAccessor::Normalized) != 0);
	return true;
}

float LWEGLTFParser::BuildNodeAnimation(LWEGLTFAnimTween &Animation, uint32_t NodeID) {
	uint64_t Resolution = LWTimer::GetResolution();
	LWEGLTFAnimChannel *TranslationChannel = nullptr;
	LWEGLTFAnimChannel *ScaleChannel = nullptr;
	LWEGLTFAnimChannel *RotationChannel = nullptr;
	LWEGLTFNode *Node = GetNode(NodeID);
	LWMatrix4f Transform = Node ? Node->m_TransformMatrix : LWMatrix4f();
	LWVector3f DefScale = LWVector3f(1.0f);
	LWVector3f DefTranslation = LWVector3f();
	LWQuaternionf DefRotation = LWQuaternionf();
	Transform.Decompose(DefScale, DefRotation, DefTranslation, true);
	for (auto &&Iter : m_Animations) {
		for (auto &&Chnls : Iter.m_Channels) {
			if(Chnls.m_Node!=NodeID) continue;
			if (Chnls.m_Path == LWEGLTFAnimChannel::translation) TranslationChannel = &Chnls;
			else if (Chnls.m_Path == LWEGLTFAnimChannel::rotation) RotationChannel = &Chnls;
			else if (Chnls.m_Path == LWEGLTFAnimChannel::scale) ScaleChannel = &Chnls;
		}
	}
	if (TranslationChannel) {
		if (!BuildChannelTween<LWVector3f>(Animation.m_Translation, *TranslationChannel)) TranslationChannel = nullptr;
	}
	if(!TranslationChannel) Animation.m_Translation.Push(DefTranslation, 0);
	if (RotationChannel) {
		if (!BuildChannelTween<LWQuaternionf>(Animation.m_Rotation, *RotationChannel)) RotationChannel = nullptr;
	}
	if (!RotationChannel) Animation.m_Rotation.Push(DefRotation, 0);
	if(ScaleChannel){
		if (!BuildChannelTween<LWVector3f>(Animation.m_Scale, *ScaleChannel)) ScaleChannel = nullptr;
	}
	if (!ScaleChannel) {
		Animation.m_Scale.Push(DefScale, 0);
	}
	return Animation.GetTotalTime();
}

float LWEGLTFParser::BuildNodeAnimation(LWEGLTFAnimTween &Animation, uint32_t AnimationID, uint32_t NodeID) {
	LWEGLTFAnimChannel *TranslationChannel = nullptr;
	LWEGLTFAnimChannel *ScaleChannel = nullptr;
	LWEGLTFAnimChannel *RotationChannel = nullptr;
	LWEGLTFNode *Node = GetNode(NodeID);
	LWMatrix4f Transform = Node ? Node->m_TransformMatrix : LWMatrix4f();
	LWVector3f DefScale = LWVector3f(1.0f);
	LWVector3f DefTranslation = LWVector3f();
	LWQuaternionf DefRotation = LWQuaternionf();
	Transform.Decompose(DefScale, DefRotation, DefTranslation, true);
	LWEGLTFAnimation *Anim = GetAnimation(AnimationID);

	if (Anim) {
		for (auto &&Chnls : Anim->m_Channels) {
			if (Chnls.m_Node != NodeID) continue;
			if (Chnls.m_Path == LWEGLTFAnimChannel::translation) TranslationChannel = &Chnls;
			else if (Chnls.m_Path == LWEGLTFAnimChannel::rotation) RotationChannel = &Chnls;
			else if (Chnls.m_Path == LWEGLTFAnimChannel::scale) ScaleChannel = &Chnls;
		}
	}
	if (TranslationChannel) {
		if (!BuildChannelTween<LWVector3f>(Animation.m_Translation, *TranslationChannel)) TranslationChannel = nullptr;
	}
	if (!TranslationChannel) Animation.m_Translation.Push(DefTranslation, 0);
	if (RotationChannel) {
		if (!BuildChannelTween<LWQuaternionf>(Animation.m_Rotation, *RotationChannel)) RotationChannel = nullptr;
	}
	if (!RotationChannel) Animation.m_Rotation.Push(DefRotation, 0);
	if (ScaleChannel) {
		if (!BuildChannelTween<LWVector3f>(Animation.m_Scale, *ScaleChannel)) ScaleChannel = nullptr;
	}
	if (!ScaleChannel) {
		Animation.m_Scale.Push(DefScale, 0);
	}
	return Animation.GetTotalTime();
}

LWMatrix4f LWEGLTFParser::GetNodeWorldTransform(uint32_t NodeID) {
	LWEGLTFNode *N = GetNode(NodeID);
	if (N->m_ParentID == -1) return N->m_TransformMatrix;
	return N->m_TransformMatrix*GetNodeWorldTransform(N->m_ParentID);
}

bool LWEGLTFParser::LoadImage(LWImage &Image, uint32_t ImageID, LWAllocator &Allocator) {
	LWEGLTFImage *Img = GetImage(ImageID);
	if (!Img) return false;
	if (Img->m_BufferView != -1) {
		LWEGLTFBufferView *BufView = GetBufferView(Img->m_BufferView);
		LWEGLTFBuffer *Buf = GetBuffer(BufView->m_BufferID);
		LWByteBuffer BB = LWByteBuffer((int8_t*)Buf->m_Buffer + BufView->m_Offset, BufView->m_Length, 0);
		if (Img->m_MimeType == LWEGLTFImage::MimeImagePng) return LWImage::LoadImagePNG(Image, BB, Allocator);
		else if (Img->m_MimeType == LWEGLTFImage::MimeImageDDS) return LWImage::LoadImageDDS(Image, BB, Allocator);
		LWLogCritical<256>("unsupported mime type: {:#x}", Img->m_MimeType);
		return false;
	}
	if(*Img->m_URI){
		return LWLogCriticalIf<256>(LWImage::LoadImage(Image, Img->m_URI, Allocator), "Could not load image at: '{}'", Img->m_URI);
	}
	return false;
}

LWEGLTFParser &LWEGLTFParser::SetDefaultScene(uint32_t SceneID) {
	m_DefaultSceneID = SceneID;
	return *this;
}

bool LWEGLTFParser::PushBuffer(LWEGLTFBuffer &Buf) {
	m_Buffers.push_back(std::move(Buf));
	return true;
}

bool LWEGLTFParser::PushBufferView(LWEGLTFBufferView &BufView) {
	m_BufferViews.push_back(std::move(BufView));
	return true;
}

bool LWEGLTFParser::PushAccessor(LWEGLTFAccessor &Accessor) {
	m_Accessors.push_back(std::move(Accessor));
	return true;
}

bool LWEGLTFParser::PushImage(LWEGLTFImage &Image) {
	m_Images.push_back(std::move(Image));
	return true;
}

bool LWEGLTFParser::PushTexture(LWEGLTFTexture &Texture) {
	m_Textures.push_back(std::move(Texture));
	return true;
}

bool LWEGLTFParser::PushMaterial(LWEGLTFMaterial &Material) {
	m_Materials.push_back(std::move(Material));
	return true;
}

bool LWEGLTFParser::PushMesh(LWEGLTFMesh &Mesh) {
	m_Meshs.push_back(std::move(Mesh));
	return true;
}

bool LWEGLTFParser::PushLight(LWEGLTFLight &Light) {
	m_Lights.push_back(std::move(Light));
	return true;
}

bool LWEGLTFParser::PushNode(LWEGLTFNode &Node) {
	m_Nodes.push_back(std::move(Node));
	return true;
}

bool LWEGLTFParser::PushSkin(LWEGLTFSkin &Skin) {
	m_Skins.push_back(std::move(Skin));
	return true;
}

bool LWEGLTFParser::PushAnimation(LWEGLTFAnimation &Animation) {
	m_Animations.push_back(std::move(Animation));
	return true;
}

bool LWEGLTFParser::PushCamera(LWEGLTFCamera &Camera) {
	m_Cameras.push_back(std::move(Camera));
	return true;
}

bool LWEGLTFParser::PushScene(LWEGLTFScene &Scene) {
	m_Scenes.push_back(std::move(Scene));
	return true;
}

uint32_t LWEGLTFParser::FindBuffer(const LWUTF8Iterator &Name) {
	uint32_t NameHash = Name.Hash();
	uint32_t Len = (uint32_t)m_Buffers.size();
	for (uint32_t i = 0; i < Len; i++) {
		if (NameHash == m_Buffers[i].m_NameHash) return i;
	}
	return -1;
}

uint32_t LWEGLTFParser::FindMesh(const LWUTF8Iterator &Name) {
	uint32_t NameHash = Name.Hash();	
	uint32_t Len = (uint32_t)m_Meshs.size();
	for (uint32_t i = 0; i < Len; i++) {
		if (NameHash == m_Meshs[i].m_NameHash) return i;
	}
	return -1;
}

uint32_t LWEGLTFParser::FindImage(const LWUTF8Iterator &Name) {
	uint32_t NameHash = Name.Hash();
	uint32_t Len = (uint32_t)m_Images.size();
	for (uint32_t i = 0; i < Len; i++) {
		if (NameHash == m_Images[i].m_NameHash) return i;
	}
	return -1;
}

uint32_t LWEGLTFParser::FindTexture(const LWUTF8Iterator &Name) {
	uint32_t NameHash = Name.Hash();
	uint32_t Len = (uint32_t)m_Textures.size();
	for (uint32_t i = 0; i < Len; i++) {
		if (NameHash == m_Textures[i].m_NameHash) return i;
	}
	return -1;
}

uint32_t LWEGLTFParser::FindMaterial(const LWUTF8Iterator &Name) {
	uint32_t NameHash = Name.Hash();
	uint32_t Len = (uint32_t)m_Materials.size();
	for (uint32_t i = 0; i < Len; i++) {
		if (NameHash == m_Materials[i].m_NameHash) return i;
	}
	return -1;
}

uint32_t LWEGLTFParser::FindLight(const LWUTF8Iterator &Name) {
	uint32_t NameHash = Name.Hash();
	uint32_t Len = (uint32_t)m_Lights.size();
	for (uint32_t i = 0; i < Len; i++) {
		if (NameHash == m_Lights[i].m_NameHash) return i;
	}
	return -1;
}

uint32_t LWEGLTFParser::FindNode(const LWUTF8Iterator &Name) {
	uint32_t NameHash = Name.Hash();
	uint32_t Len = (uint32_t)m_Nodes.size();
	for (uint32_t i = 0; i < Len; i++) {
		if (NameHash == m_Nodes[i].m_NameHash) return i;
	}
	return -1;
}

uint32_t LWEGLTFParser::FindScene(const LWUTF8Iterator &Name) {
	uint32_t NameHash = Name.Hash();
	uint32_t Len = (uint32_t)m_Scenes.size();
	for (uint32_t i = 0; i < Len; i++) {
		if (NameHash == m_Scenes[i].m_NameHash) return i;
	}
	return -1;
}

uint32_t LWEGLTFParser::FindSkin(const LWUTF8Iterator &Name) {
	uint32_t NameHash = Name.Hash();
	uint32_t Len = (uint32_t)m_Skins.size();
	for (uint32_t i = 0; i < Len; i++) {
		if (NameHash == m_Skins[i].m_NameHash) return i;
	}
	return -1;
}

uint32_t LWEGLTFParser::FindCamera(const LWUTF8Iterator &Name) {
	uint32_t NameHash = Name.Hash();
	uint32_t Len = (uint32_t)m_Cameras.size();
	for (uint32_t i = 0; i < Len; i++) {
		if (NameHash == m_Cameras[i].m_NameHash) return i;
	}
	return -1;
}

uint32_t LWEGLTFParser::FindAnimation(const LWUTF8Iterator &Name) {
	uint32_t NameHash = Name.Hash();
	uint32_t Len = (uint32_t)m_Animations.size();
	for (uint32_t i = 0; i < Len; i++) {
		if (NameHash == m_Animations[i].m_NameHash) return i;
	}
	return -1;
}

LWEGLTFBuffer *LWEGLTFParser::GetBuffer(uint32_t i) {
	if (i >= m_Buffers.size()) return nullptr;
	return &m_Buffers[i];
}

LWEGLTFBufferView *LWEGLTFParser::GetBufferView(uint32_t i) {
	if (i >= m_BufferViews.size()) return nullptr;
	return &m_BufferViews[i];
}

LWEGLTFAccessor *LWEGLTFParser::GetAccessor(uint32_t i) {
	if (i >= m_Accessors.size()) return nullptr;
	return &m_Accessors[i];
}

LWEGLTFImage *LWEGLTFParser::GetImage(uint32_t i) {
	if (i >= m_Images.size()) return nullptr;
	return &m_Images[i];
}

LWEGLTFTexture *LWEGLTFParser::GetTexture(uint32_t i) {
	if (i >= m_Textures.size()) return nullptr;
	return &m_Textures[i];
}

LWEGLTFMaterial *LWEGLTFParser::GetMaterial(uint32_t i) {
	if (i >= m_Materials.size()) return nullptr;
	return &m_Materials[i];
}

LWEGLTFAnimation *LWEGLTFParser::GetAnimation(uint32_t i) {
	if (i >= m_Animations.size()) return nullptr;
	return &m_Animations[i];
}

LWEGLTFSkin *LWEGLTFParser::GetSkin(uint32_t i) {
	if (i >= m_Skins.size()) return nullptr;
	return &m_Skins[i];
}

LWEGLTFMesh *LWEGLTFParser::GetMesh(uint32_t i) {
	if (i >= m_Meshs.size()) return nullptr;
	return &m_Meshs[i];
}

LWEGLTFLight *LWEGLTFParser::GetLight(uint32_t i) {
	if (i >= m_Lights.size()) return nullptr;
	return &m_Lights[i];
}

LWEGLTFNode *LWEGLTFParser::GetNode(uint32_t i) {
	if (i >= m_Nodes.size()) return nullptr;
	return &m_Nodes[i];
}

LWEGLTFCamera *LWEGLTFParser::GetCamera(uint32_t i) {
	if (i >= m_Cameras.size()) return nullptr;
	return &m_Cameras[i];
}

LWEGLTFScene *LWEGLTFParser::GetScene(uint32_t i) {
	if (i >= m_Scenes.size()) return nullptr;
	return &m_Scenes[i];
}

uint32_t LWEGLTFParser::GetBufferCount(void) const {
	return (uint32_t)m_Buffers.size();
}

uint32_t LWEGLTFParser::GetBufferViewCount(void) const {
	return (uint32_t)m_BufferViews.size();
}

uint32_t LWEGLTFParser::GetAccessorCount(void) const {
	return (uint32_t)m_Accessors.size();
}

uint32_t LWEGLTFParser::GetImageCount(void) const {
	return (uint32_t)m_Images.size();
}

uint32_t LWEGLTFParser::GetAnimationCount(void) const {
	return (uint32_t)m_Animations.size();
}

uint32_t LWEGLTFParser::GetTextureCount(void) const {
	return (uint32_t)m_Textures.size();
}

uint32_t LWEGLTFParser::GetMaterialCount(void) const {
	return (uint32_t)m_Materials.size();
}

uint32_t LWEGLTFParser::GetMeshCount(void) const {
	return (uint32_t)m_Meshs.size();
}

uint32_t LWEGLTFParser::GetLightCount(void) const {
	return (uint32_t)m_Lights.size();
}

uint32_t LWEGLTFParser::GetNodeCount(void) const {
	return (uint32_t)m_Nodes.size();
}

uint32_t LWEGLTFParser::GetSkinCount(void) const {
	return (uint32_t)m_Skins.size();
}

uint32_t LWEGLTFParser::GetSceneCount(void) const {
	return (uint32_t)m_Scenes.size();
}

uint32_t LWEGLTFParser::GetCameraCount(void) const {
	return (uint32_t)m_Cameras.size();
}

uint32_t LWEGLTFParser::GetDefaultSceneID(void) const {
	return m_DefaultSceneID;
}

LWEGLTFParser &LWEGLTFParser::ClearBuffers(void) {
	for (auto &&Iter : m_Buffers) {
		LWAllocator::Destroy(Iter.m_Buffer);
		Iter.m_Buffer = nullptr;
	}
	return *this;
}