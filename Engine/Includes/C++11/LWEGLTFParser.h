#ifndef LWEGLTFPARSER_H
#define LWEGLTFPARSER_H
#include <LWCore/LWTypes.h>
#include <LWCore/LWMatrix.h>
#include <LWCore/LWVector.h>
#include <LWCore/LWByteBuffer.h>
#include <LWVideo/LWImage.h>
#include <LWPlatform/LWFileStream.h>
#include <LWETween.h>
#include <LWEJson.h>
#include <vector>

//Time of GLTF animations is converted to High performance clock time
struct LWEGLTFAnimTween{

	//Constructs a matrix for the given time code, if Loop is set to true then Time will be Time%TotalTime to continously loop the animation.
	LWMatrix4f GetFrame(float Time, bool Loop = false);

	float GetTotalTime(void) const;

	LWEGLTFAnimTween() = default;

	LWETween<LWVector3f> m_Translation;
	LWETween<LWQuaternionf> m_Rotation;
	LWETween<LWVector3f> m_Scale;
};


struct LWEGLTFAccessor {
	enum {
		//Component Type possible values, Based on openGL values, the Byte2-4 are unlikely to be ever used but are added for completness sake.
		Byte = 0,
		UByte,
		Short,
		UShort,
		Int,
		UInt,
		Float,
		Byte2,
		Byte3,
		Byte4,
		Double,

		//Type possible values, indicates how many of ComponentType are to be read.
		Scalar = 0,
		Vec2,
		Vec3,
		Vec4,
		Mat2,
		Mat3,
		Mat4,

		Normalized = 0x1 //Flag indicating the value should be normalized when read out.
	};

	static bool ParseJSON(LWEGLTFAccessor &Buf, LWEJson &J, LWEJObject *Obj);

	LWEGLTFAccessor(uint32_t BufferID, uint32_t Offset, uint32_t ComponentType, uint32_t Count, uint32_t Type, uint32_t Flag);

	LWEGLTFAccessor() = default;

	uint32_t m_BufferID = -1;
	uint32_t m_Offset = 0;
	uint32_t m_ComponentType = 0;
	uint32_t m_Count = 0;
	uint32_t m_Type = 0;
	uint32_t m_Flag = 0;
};

struct LWEGLTFAccessorView {

	/*!< \brief reads from Position the specified component type, normalizes if necessary, then returns the value as type. */
	template<class Type>
	Type GetValueAt(uint8_t *Position){
		if (m_ComponentType == LWEGLTFAccessor::Byte) {
			int8_t Val = *((int8_t*)Position);
			if (m_Normalize) return (Type)(Val / 127);
			return (Type)Val;
		} else if (m_ComponentType == LWEGLTFAccessor::UByte) {
			uint8_t Val = *Position;
			if (m_Normalize) return (Type)(Val / 255);
			return (Type)Val;
		} else if (m_ComponentType == LWEGLTFAccessor::Short) {
			int16_t Val = *((int16_t*)Position);
			if (m_Normalize) return (Type)(Val / 32767);
			return (Type)Val;
		} else if (m_ComponentType == LWEGLTFAccessor::UShort) {
			uint16_t Val = *((uint16_t*)Position);
			if (m_Normalize) return (Type)(Val / 65535);
			return (Type)Val;
		} else if (m_ComponentType == LWEGLTFAccessor::Int) {
			int32_t Val = *((int32_t*)Position);
			if (m_Normalize) return (Type)(Val / 2147483647);
			return (Type)Val;
		} else if (m_ComponentType == LWEGLTFAccessor::UInt) {
			uint32_t Val = *((uint32_t*)Position);
			if (m_Normalize) return (Type)(Val / 4294967295);
			return (Type)Val;
		} else if (m_ComponentType == LWEGLTFAccessor::Float) {
			float Val = *((float*)Position);
			return (Type)Val;
		} else if (m_ComponentType == LWEGLTFAccessor::Double) {
			double Val = *((double*)Position);
			return (Type)Val;
		}
		return (Type)0;
	};

	template<class Type>
	uint32_t ReadValues(Type *Values, uint32_t ValueStride, uint32_t Count) {
		const uint32_t TypeSizes[] = { 1, 1, 2, 2, 4, 4, 4, 2, 3, 4, 8 };
		uint32_t C = std::min<uint32_t>(Count, m_Count - m_Position);
		if (!Values) return C;
		uint8_t *bValue = (uint8_t*)Values;
		uint8_t *BPos = m_Buffer + TypeSizes[m_ComponentType] * m_ComponentCount * m_Position;
		for (uint32_t i = 0; i < C; i++) {
			for (uint32_t n = 0; n < m_ComponentCount; n++) {
				Type *V = ((Type*)bValue) + n;
				*V = GetValueAt<Type>(BPos);
				BPos += TypeSizes[m_ComponentType];
			}
			bValue += ValueStride;
		}
		m_Position += C;
		return C;
	}

	LWEGLTFAccessorView(uint8_t *Buffer, uint32_t ComponentCount, uint32_t ComponentType, uint32_t Count, bool Normalize);

	LWEGLTFAccessorView() = default;

	uint8_t *m_Buffer = nullptr;
	uint32_t m_ComponentCount = 0;
	uint32_t m_ComponentType = 0;
	uint32_t m_Count = 0;
	uint32_t m_Position = 0;
	bool m_Normalize = false;
};


struct LWEGLTFBuffer {
	static bool ParseJSON(LWEGLTFBuffer &Buf, LWEJson &J, LWEJObject *Obj, LWAllocator &Allocator, LWFileStream &FileStream, const char *BinChunk);

	LWEGLTFBuffer &SetName(const LWUTF8Iterator &Name);

	LWUTF8Iterator GetName(void) const;

	LWEGLTFBuffer &operator = (LWEGLTFBuffer &&O);

	LWEGLTFBuffer() = default;

	LWEGLTFBuffer(LWEGLTFBuffer &&O);

	LWEGLTFBuffer(const LWUTF8Iterator &Name, uint8_t *Buffer, uint32_t Length);

	~LWEGLTFBuffer();

	char8_t m_Name[256]={};
	uint32_t m_NameHash = LWCrypto::FNV1AHash;
	uint8_t *m_Buffer = nullptr;
	uint32_t m_Length = 0;
};

struct LWEGLTFBufferView {

	static bool ParseJSON(LWEGLTFBufferView &BufView, LWEJson &J, LWEJObject *Obj);

	LWEGLTFBufferView() = default;

	LWEGLTFBufferView(uint32_t BufferID, uint32_t Offset, uint32_t Length, uint32_t Stride);

	uint32_t m_BufferID = -1;
	uint32_t m_Offset = 0;
	uint32_t m_Length = 0;
	uint32_t m_Stride = 0;
};


struct LWEGLTFCameraOrtho {
	static bool ParseJSON(LWEGLTFCameraOrtho &Ortho, LWEJson &J, LWEJObject *Obj);

	LWMatrix4f GetMatrix(void);

	LWEGLTFCameraOrtho(float xMag, float yMag, float zNear, float zFar);

	LWEGLTFCameraOrtho() = default;

	float m_xMag = 0.0f;
	float m_yMag = 0.0f;
	float m_zNear = 0.0f;
	float m_zFar = 0.0f;
};

struct LWEGLTFCameraPerspective {
	static bool ParseJSON(LWEGLTFCameraPerspective &Persp, LWEJson &J, LWEJObject *Obj);

	LWMatrix4f GetMatrix(void);

	LWMatrix4f GetMatrix(float Aspect);

	LWEGLTFCameraPerspective(float FOV, float zNear);

	LWEGLTFCameraPerspective() = default;

	float m_FOV = 0.0f;
	float m_zNear = 0.0f;
	float m_ZFar = 100000.0f; //Set to "infinite" by default.
	float m_Aspect = 1.0f;
};

struct LWEGLTFCamera {
	static const uint32_t perspective = 0xfe832c15; //LWText::MakeHash("perspective");
	static const uint32_t orthographic = 0xe3f05fab; //LWText::MakeHash("orthographic");
	
	static bool ParseJSON(LWEGLTFCamera &Camera, LWEJson &J, LWEJObject *Obj);

	LWEGLTFCamera &SetName(const LWUTF8Iterator &Name);

	LWUTF8Iterator GetName(void) const;

	LWMatrix4f GetMatrix(void);

	LWMatrix4f GetMatrix(float Aspect);

	LWEGLTFCamera(const LWUTF8Iterator &Name, uint32_t CameraType);

	LWEGLTFCamera() = default;

	char8_t m_Name[256]={};
	LWEGLTFCameraOrtho m_Ortho;
	LWEGLTFCameraPerspective m_Perspective;
	uint32_t m_NameHash = LWCrypto::FNV1AHash;
	uint32_t m_CameraType = 0;
};



struct LWEGLTFAttribute {
	static const uint32_t POSITION = 0x7808e88a; //LWText::MakeHash("POSITION");
	static const uint32_t NORMAL = 0x3c329992; //LWText::MakeHash("NORMAL");
	static const uint32_t TANGENT = 0x999442e2; //LWText::MakeHash("TANGENT");
	static const uint32_t TEXCOORD_0 = 0x444ab1fc; //LWText::MakeHash("TEXCOORD_0");
	static const uint32_t TEXCOORD_1 = 0x454ab38f; //LWText::MakeHash("TEXCOORD_1");
	static const uint32_t COLOR_0 = 0xcc6c094f; //LWText::MakeHash("COLOR_0");
	static const uint32_t JOINTS_0 = 0xe254449b; //LWText::MakeHash("JOINTS_0");
	static const uint32_t WEIGHTS_0 = 0x23950019; //LWText::MakeHash("WEIGHTS_0");

	static bool ParseJSON(LWEGLTFAttribute &Attribute, LWEJson &J, LWEJObject *Obj);

	LWEGLTFAttribute() = default;

	LWEGLTFAttribute(uint32_t AccessorID, uint32_t AttributeHash);

	uint32_t m_AccessorID = -1;
	uint32_t m_AttributeHash = 0;
};

struct LWEGLTFPrimitive {
	static const uint32_t MaxAttributes = 8;

	static bool ParseJSON(LWEGLTFPrimitive &Primitive, LWEJson &J, LWEJObject *Obj);

	//returns -1 if not found, otherwise returns the attributes index.
	uint32_t FindAttributeAccessor(uint32_t AttributeHash);

	LWEGLTFPrimitive(uint32_t MaterialID, uint32_t IndiceID, uint32_t AttributeCount);

	LWEGLTFPrimitive() = default;

	LWEGLTFAttribute m_AttributeList[MaxAttributes];
	uint32_t m_AttributeCount = 0;
	uint32_t m_MaterialID = -1;
	uint32_t m_IndiceID = -1;

};

struct LWEGLTFMesh {
	static bool ParseJSON(LWEGLTFMesh &Mesh, LWEJson &J, LWEJObject *Obj);

	LWEGLTFMesh &SetName(const LWUTF8Iterator &Name);

	LWUTF8Iterator GetName(void) const;

	LWEGLTFMesh(const LWUTF8Iterator &Name, uint32_t PrimitiveCount);

	LWEGLTFMesh() = default;

	char8_t m_Name[256]={};
	uint32_t m_NameHash = LWCrypto::FNV1AHash;
	std::vector<LWEGLTFPrimitive> m_Primitives;
};

struct LWEGLTFImage {
	static const uint32_t MimeNone = 0;
	static const uint32_t MimeImageJpeg = 0xe88a7c5d; //LWText::MakeHash("image/jpeg");
	static const uint32_t MimeImagePng = 0xb00abf3a; //LWText::MakeHash("image/png");
	static const uint32_t MimeImageDDS = 0x846e6e23; //LWText::MakeHash("image/vnd-ms.dds");

	static bool ParseJSON(LWEGLTFImage &Img, LWEJson &J, LWEJObject *Obj, LWFileStream &Stream);

	LWEGLTFImage &SetName(const LWUTF8Iterator &Name);

	LWEGLTFImage &SetURI(const LWUTF8Iterator &URI);

	LWUTF8Iterator GetName(void) const;

	LWUTF8Iterator GetURI(void) const;

	LWEGLTFImage() = default;

	LWEGLTFImage(const LWUTF8Iterator &Name, const LWUTF8Iterator &URI, uint32_t MimeType, uint32_t BufferView);

	char8_t m_Name[256]={};
	char8_t m_URI[256]={};
	uint32_t m_NameHash = LWCrypto::FNV1AHash;
	uint32_t m_BufferView = -1;
	uint32_t m_MimeType = 0;
};

struct LWEGLTFTexture {
	static bool ParseJSON(LWEGLTFTexture &Tex, LWEJson &J, LWEJObject *Obj);

	LWEGLTFTexture &SetName(const LWUTF8Iterator &Name);

	LWUTF8Iterator GetName(void) const;

	LWEGLTFTexture(const LWUTF8Iterator &Name, uint32_t ImageID, uint32_t SamplerFlag);

	LWEGLTFTexture() = default;

	char8_t m_Name[256]={};
	uint32_t m_NameHash=LWCrypto::FNV1AHash;
	uint32_t m_ImageID = 0;
	uint32_t m_SamplerFlag = 0;
};

struct LWEGLTFTextureInfo {
	static bool ParseJSON(LWEGLTFTextureInfo &TexInfo, LWEJson &J, LWEJObject *Obj);

	LWEGLTFTextureInfo(uint32_t TextureIndex);

	LWEGLTFTextureInfo() = default;

	uint32_t m_TextureIndex = 0;
	uint32_t m_TexCoord = 0;
	LWVector2f m_Offset = LWVector2f(0.0f);
	LWVector2f m_Scale = LWVector2f(1.0f);
	float m_Rotation = 0.0f;
};

struct LWEGLTFMatMetallicRoughness {
	static bool ParseJSON(LWEGLTFMatMetallicRoughness &Mat, LWEJson &J, LWEJObject *Obj);

	LWEGLTFMatMetallicRoughness(const LWVector4f &BaseColorFactor, float MetallicFactor, float RoughnessFactor);

	LWEGLTFMatMetallicRoughness() = default;

	LWEGLTFTextureInfo m_BaseColorTexture;
	LWEGLTFTextureInfo m_MetallicRoughnessTexture;
	LWVector4f m_BaseColorFactor = LWVector4f(1.0f);
	float m_MetallicFactor = 1.0f;
	float m_RoughnessFactor = 1.0f;
};

struct LWEGLTFMatSpecularGlossyness {
	static bool ParseJSON(LWEGLTFMatSpecularGlossyness &Mat, LWEJson &J, LWEJObject *Obj);

	LWEGLTFMatSpecularGlossyness(const LWVector4f &DiffuseFactor, const LWVector3f &SpecularFactor, float Glossiness);

	LWEGLTFMatSpecularGlossyness() = default;

	LWEGLTFTextureInfo m_DiffuseTexture;
	LWEGLTFTextureInfo m_SpecularGlossyTexture;
	LWVector4f m_DiffuseFactor = LWVector4f(1.0f);
	LWVector3f m_SpecularFactor = LWVector3f(1.0f);
	float m_Glossiness = 1.0f;
};

struct LWEGLTFMaterial {
	enum {
		AlphaOpaque = 0x0, //Material is fully opaque
		AlphaMask = 0x1, //Material should be discarded based on the alphaCutoff value.
		AlphaBlend = 0x2, //Material should be blended normally.
		AlphaBits = 0x3, //Bits to get the alpha blending mode.
		AlphaBitOffset = 0x0, //bit offset to get the 0 indexed alpha blending mode.
		DoubleSided = 0x4, //Flag indicating the material is double sided and front face culling should be disabled.

		MetallicRoughness = 0x0, //Type is default metallic roughness material.
		Unlit = 0x8, //Type is unlit material (Unlit Color texture+Color factor is placed in metallic roughness BaseColor/BaseColorFactor).
		SpecularGlossyness = 0x10, //Type is specular glossyness material.
		TypeBits = 0x18, //Bits to get the type of material.
		TypeBitOffset = 0x4 //Bit offset to get the 0 indexed type.
		
	};

	static bool ParseJSON(LWEGLTFMaterial &Mat, LWEJson &J, LWEJObject *Obj);

	LWEGLTFMaterial &SetName(const LWUTF8Iterator &Name);

	uint32_t GetType(void) const;

	LWUTF8Iterator GetName(void) const;

	LWEGLTFMaterial(const LWUTF8Iterator &Name);

	LWEGLTFMaterial() = default;

	char8_t m_Name[256]={};
	uint32_t m_NameHash = LWCrypto::FNV1AHash;
	LWEGLTFMatMetallicRoughness m_MetallicRoughness;
	LWEGLTFMatSpecularGlossyness m_SpecularGlossy;
	LWEGLTFTextureInfo m_NormalMapTexture;
	LWEGLTFTextureInfo m_OcclusionTexture;
	LWEGLTFTextureInfo m_EmissiveTexture;
	LWVector4f m_EmissiveFactor = LWVector4f(0.0f, 0.0f, 0.0f, 1.0f);
	float m_AlphaCutoff = 0.5f;
	uint32_t m_Flag = 0;
	
};

struct LWEGLTFLight {
	static const uint32_t POINT = 0x18ae6c91;//LWText::MakeHash("point");
	static const uint32_t DIRECTIONAL = 0x10f7c207;//LWText::MakeHash("directional");
	static const uint32_t SPOT = 0x2305f67d;//LWText::MakeHash("spot");

	static bool ParseJSON(LWEGLTFLight &L, LWEJson &J, LWEJObject *Obj);
	
	LWEGLTFLight &SetName(const LWUTF8Iterator &Name);

	LWUTF8Iterator GetName(void) const;

	LWEGLTFLight(const LWUTF8Iterator &Name, uint32_t Type);

	LWEGLTFLight() = default;

	char8_t m_Name[256] = "";
	uint32_t m_NameHash = LWCrypto::FNV1AHash;
	LWVector3f m_Color = LWVector3f(1.0f);
	float m_Intensity = 1.0f;
	float m_InnerConeTheta = 0.0f;
	float m_OuterConeTheta = LW_PI_4;
	uint32_t m_Type;
	float m_Range = 1000.0f;
};

struct LWEGLTFNode {
	static bool ParseJSON(LWEGLTFNode &Node, LWEJson &J, LWEJObject *Obj);

	LWEGLTFNode &SetName(const LWUTF8Iterator &Name);

	LWUTF8Iterator GetName(void) const;

	LWEGLTFNode(const LWUTF8Iterator &Name, uint32_t MeshID, uint32_t SkinID, uint32_t CameraID, uint32_t ChildrenCnt, const LWMatrix4f &TransformMatrix);

	LWEGLTFNode() = default;
	
	char8_t m_Name[256]={};
	uint32_t m_NameHash = LWCrypto::FNV1AHash;
	std::vector<uint32_t> m_Children;
	LWMatrix4f m_TransformMatrix;
	uint32_t m_ParentID = -1;
	uint32_t m_CameraID = -1;
	uint32_t m_LightID = -1;
	uint32_t m_MeshID = -1;
	uint32_t m_SkinID = -1;
	uint32_t m_NodeID = 0;

};

struct LWEGLTFScene {
	static bool ParseJSON(LWEGLTFScene &Scene, LWEJson &J, LWEJObject *Obj);

	LWEGLTFScene &SetName(const LWUTF8Iterator &Name);

	LWUTF8Iterator GetName(void) const;

	LWEGLTFScene(const LWUTF8Iterator &Name, uint32_t NodeCnt);

	LWEGLTFScene() = default;

	char8_t m_Name[256]={};
	uint32_t m_NameHash = LWCrypto::FNV1AHash;
	std::vector<uint32_t> m_NodeList;
};

struct LWEGLTFSkin {
	static bool ParseJSON(LWEGLTFSkin &Skin, LWEJson &J, LWEJObject *Obj);

	LWEGLTFSkin &SetName(const LWUTF8Iterator &Name);

	LWUTF8Iterator GetName(void) const;

	LWEGLTFSkin(const LWUTF8Iterator &Name, uint32_t JointCnt, uint32_t InverseBindMatrices, uint32_t SkeletonNode);

	LWEGLTFSkin() = default;

	char8_t m_Name[256] = "";
	uint32_t m_NameHash = LWCrypto::FNV1AHash;
	uint32_t m_InverseBindMatrices = -1;
	uint32_t m_SkeletonNode = -1;
	std::vector<uint32_t> m_JointList;
};

struct LWEGLTFAnimChannel {
	static const uint32_t LINEAR = 0x1a174b80; //LWText::MakeHash("LINEAR");
	static const uint32_t STEP = 0xc4977d4f; //LWText::MakeHash("STEP");
	static const uint32_t CUBICSPLINE = 0xa7840cfc; //LWText::MakeHash("CUBICSPLINE");
	static const uint32_t translation = 0xcbd2d62c; //LWText::MakeHash("translation");
	static const uint32_t rotation = 0x21ac415f; //LWText::MakeHash("rotation");
	static const uint32_t scale = 0x82971c71; //LWText::MakeHash("scale");
	//static const uint32_t weights = 0xb6d9f7fe; //LWText::MakeHash("weights");  Morph targets are not supported at this time.

	static bool ParseJSON(LWEGLTFAnimChannel &Channel, LWEJson &J, LWEJObject *Obj, LWEJObject *AnimObj);

	LWEGLTFAnimChannel(uint32_t InputID, uint32_t OutputID, uint32_t Interpolation, uint32_t Node, uint32_t Path);

	LWEGLTFAnimChannel() = default;

	uint32_t m_InputID = -1; //Interpolation input accessor.
	uint32_t m_OutputID = -1; //Output values for each input.
	uint32_t m_Interpolation = LINEAR;
	uint32_t m_Node = -1;
	uint32_t m_Path = 0;

};

struct LWEGLTFAnimation {

	static bool ParseJSON(LWEGLTFAnimation &Anim, LWEJson &J, LWEJObject *Obj);

	LWEGLTFAnimation &SetName(const LWUTF8Iterator &Name);

	LWUTF8Iterator GetName(void) const;

	LWEGLTFAnimation(const LWUTF8Iterator &Name, uint32_t ChannelCnt);

	LWEGLTFAnimation() = default;

	char8_t m_Name[256] = "";
	uint32_t m_NameHash = LWCrypto::FNV1AHash;
	std::vector<LWEGLTFAnimChannel> m_Channels;
};


class LWEGLTFParser {
public:
	static bool LoadFile(LWEGLTFParser &Parser, const LWUTF8Iterator &Path, LWAllocator &Allocator);

	static bool LoadFileGLB(LWEGLTFParser &Parser, const LWUTF8Iterator &Path, LWAllocator &Allocator);

	static bool LoadFileGLTF(LWEGLTFParser &Parser, const LWUTF8Iterator &Path, LWAllocator &Allocator);

	static bool ParseJSON(LWEGLTFParser &Parser, LWEJson &J, LWFileStream &Stream, const char *BinChunk, LWAllocator &Allocator);

	LWEGLTFParser &SetDefaultScene(uint32_t SceneID);

	/*!< \brief constructs a unique list of meshes, skin's, material's, and Image's only used by the specified scene. */
	LWEGLTFScene *BuildSceneOnlyList(uint32_t SceneID, std::vector<uint32_t> &NodeList, std::vector<uint32_t> &MeshList, std::vector<uint32_t> &SkinList, std::vector<uint32_t> &LightList, std::vector<uint32_t> &MaterialList, std::vector<uint32_t> &TextureList, std::vector<uint32_t> &ImageList);

	/*!< \brief constructs a tween of type for the passed in channel. */
	template<class Type>
	bool BuildChannelTween(LWETween<Type> &Tween, LWEGLTFAnimChannel &Channel) {
		Tween = LWETween<Type>(Channel.m_Interpolation);
		LWEGLTFAccessorView InputView;
		LWEGLTFAccessorView OutputView;
		if (!CreateAccessorView(InputView, Channel.m_InputID)) return false;
		if (!CreateAccessorView(OutputView, Channel.m_OutputID)) return false;
		for (uint32_t i = 0; i < InputView.m_Count; i++) {
			float Time = 0.0f;
			Type Vals[3];
			InputView.ReadValues<float>(&Time, 0, 1);
			if (Channel.m_Interpolation == LWEGLTFAnimChannel::CUBICSPLINE) {
				OutputView.ReadValues<float>(&Vals[0].x, sizeof(Type), 3);
				Tween.Push(Vals[0], Vals[1], Vals[2], Time);
			} else {
				OutputView.ReadValues<float>(&Vals[0].x, 0, 1);
				Tween.Push(Vals[0], Time);
			}
		}
		return true;
	};

	//Returns the length of the animation, if no animation channels then the Animation will be populated with a single frame of the bound world translation/rotation/scaling, so can still be used if neccessary.  This function searchs all animations.
	float BuildNodeAnimation(LWEGLTFAnimTween &Animation, uint32_t NodeID);

	//Returns the length of the animation, if no animation channels in the specified animationID is found, then the animation will be populated with a single frame of the bound world translation/rotation/scaling, so can still be used if necessary.
	float BuildNodeAnimation(LWEGLTFAnimTween &Animation, uint32_t AnimationID, uint32_t NodeID);

	//Calculates the world transform for the specified node.
	LWMatrix4f GetNodeWorldTransform(uint32_t NodeID);

	//Attempts to load an image if it's a lightwave supported format.
	bool LoadImage(LWImage &Image, uint32_t ImageID, LWAllocator &Allocator);

	bool CreateAccessorView(LWEGLTFAccessorView &View, uint32_t AccessorID);

	bool PushBuffer(LWEGLTFBuffer &Buf);

	bool PushBufferView(LWEGLTFBufferView &BufView);

	bool PushAccessor(LWEGLTFAccessor &Accessor);

	bool PushImage(LWEGLTFImage &Image);

	bool PushTexture(LWEGLTFTexture &Texture);

	bool PushMaterial(LWEGLTFMaterial &Material);

	bool PushLight(LWEGLTFLight &Light);

	bool PushAnimation(LWEGLTFAnimation &Animation);

	bool PushCamera(LWEGLTFCamera &Camera);

	bool PushMesh(LWEGLTFMesh &Mesh);

	bool PushNode(LWEGLTFNode &Node);

	bool PushSkin(LWEGLTFSkin &Skin);

	bool PushScene(LWEGLTFScene &Scene);

	//Returns index of buffer, or -1 if not found.
	uint32_t FindBuffer(const LWUTF8Iterator &Name);

	//Returns index of Mesh, or -1 if not found.
	uint32_t FindMesh(const LWUTF8Iterator &Name);

	//Returns index of Image, or -1 if not found.
	uint32_t FindImage(const LWUTF8Iterator &Name);

	//Returns index of Texture, or -1 if not found.
	uint32_t FindTexture(const LWUTF8Iterator &Name);

	//Returns index of Material, or -1 if not found.
	uint32_t FindMaterial(const LWUTF8Iterator &Name);

	//Returns index of Light, or -1 if not found.
	uint32_t FindLight(const LWUTF8Iterator &Name);

	//Returns index of Node, or -1 if not found.
	uint32_t FindNode(const LWUTF8Iterator &Name);

	//Returns index of Scene, or -1 if not found.
	uint32_t FindScene(const LWUTF8Iterator &Name);

	//Returns index of Skin, or -1 if not found.
	uint32_t FindSkin(const LWUTF8Iterator &Name);

	//Returns index of Animation, or -1 if not found.
	uint32_t FindAnimation(const LWUTF8Iterator &Name);

	//Returns index of camera, or -1 if not found.
	uint32_t FindCamera(const LWUTF8Iterator &Name);

	LWEGLTFBuffer *GetBuffer(uint32_t i);

	LWEGLTFBufferView *GetBufferView(uint32_t i);

	LWEGLTFAccessor *GetAccessor(uint32_t i);

	LWEGLTFImage *GetImage(uint32_t i);

	LWEGLTFTexture *GetTexture(uint32_t i);

	LWEGLTFMaterial *GetMaterial(uint32_t i);

	LWEGLTFAnimation *GetAnimation(uint32_t i);

	LWEGLTFCamera *GetCamera(uint32_t i);

	LWEGLTFLight *GetLight(uint32_t i);

	LWEGLTFSkin *GetSkin(uint32_t i);

	LWEGLTFMesh *GetMesh(uint32_t i);

	LWEGLTFNode *GetNode(uint32_t i);

	LWEGLTFScene *GetScene(uint32_t i);

	uint32_t GetBufferCount(void) const;

	uint32_t GetBufferViewCount(void) const;

	uint32_t GetAccessorCount(void) const;

	uint32_t GetImageCount(void) const;

	uint32_t GetTextureCount(void) const;

	uint32_t GetMaterialCount(void) const;

	uint32_t GetMeshCount(void) const;

	uint32_t GetNodeCount(void) const;

	uint32_t GetSceneCount(void) const;

	uint32_t GetAnimationCount(void) const;

	uint32_t GetSkinCount(void) const;

	uint32_t GetLightCount(void) const;

	uint32_t GetCameraCount(void) const;

	uint32_t GetDefaultSceneID(void) const;

	/*!< \brief deletes all loaded buffers data, once this is done all AccessorViews will be invalid, and calls to them will cause a crash. */
	LWEGLTFParser &ClearBuffers(void);

	LWEGLTFParser() = default;
private:
	std::vector<LWEGLTFBuffer> m_Buffers;
	std::vector<LWEGLTFBufferView> m_BufferViews;
	std::vector<LWEGLTFAccessor> m_Accessors;
	std::vector<LWEGLTFImage> m_Images;
	std::vector<LWEGLTFTexture> m_Textures;
	std::vector<LWEGLTFMaterial> m_Materials;
	std::vector<LWEGLTFLight> m_Lights;
	std::vector<LWEGLTFMesh> m_Meshs;
	std::vector<LWEGLTFNode> m_Nodes;
	std::vector<LWEGLTFScene> m_Scenes;
	std::vector<LWEGLTFAnimation> m_Animations;
	std::vector<LWEGLTFCamera> m_Cameras;
	std::vector<LWEGLTFSkin> m_Skins;
	uint32_t m_DefaultSceneID = -1;

};

#endif