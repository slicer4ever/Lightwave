#ifndef LWERENDERER_H
#define LWERENDERER_H
#include <LWCore/LWConcurrent/LWFIFO.h>
#include <LWVideo/LWVideoDriver.h>
#include <LWVideo/LWImage.h>
#include <LWEUIManager.h>
#include <LWEAsset.h>
#include "LWERenderFrame.h"
#include "LWERenderPass.h"
#include "LWERenderTypes.h"
#include "LWELogger.h"
#include <functional>
#include <mutex>

typedef std::function<LWEPass*(LWEXMLNode*, LWEPass*, LWERenderer*, LWEAssetManager*, LWAllocator&)> LWEPassXMLCreateFunc;

struct LWERendererFreeBlock {
	uint32_t m_BlockID;
	uint32_t m_BlockCount;
};

class LWERendererBlockAllocator {
public:
	uint32_t Allocate(uint32_t Blocks);

	void Free(uint32_t BlockID);

	LWERendererBlockAllocator(uint32_t TotalBlocks);

	LWERendererBlockAllocator() = default;
private:
	std::vector<LWERendererFreeBlock> m_FreeList;
	std::unordered_map<uint32_t, uint32_t> m_AllocatedMap;
	uint32_t m_TotalBlocks = 0;
};


//BlockGeometry allows for pooling different mesh's into one large video buffer, allowing for optimizations in how many render calls must be made per frame.
//BlockGeometry separates Position data(must start with a vec4) from vertex attribute data(must be interleaved) to attempt to see a slight performance boost in early-z test.
//Indices all all uint32_t as well.
class LWERendererBlockGeometry {
public:
	static const uint32_t DefaultVerticesPerBlock = 512;
	static const uint32_t DefaultIndicesPerBlock = 256;
	static const uint32_t DefaultMaxVerticeBlocks = 4096;
	static const uint32_t DefaultMaxIndiceBlocks = 4096;
	static const uint64_t VerticeBits = 0xFFFFFFFF;
	static const uint64_t VerticeBitOffset = 0;
	static const uint64_t IndiceBits = 0xFFFFFFFF00000000;
	static const uint64_t IndiceBitOffset = 32;
	static const uint64_t NullID = 0xFFFFFFFFFFFFFFFF;
	
	uint64_t Allocate(uint32_t VerticeCount, uint32_t IndiceCount);

	uint64_t Allocate(uint8_t *VertexPosition, uint8_t *VertexAttributes, uint32_t *Indices, uint32_t VerticeCount, uint32_t IndiceCount, LWERenderer *Renderer, uint32_t Flags = 0);

	bool SubmitLocal(uint64_t ID, uint32_t VerticeCount, uint32_t IndiceCount, LWERenderer *Renderer);

	uint64_t Free(uint64_t ID);

	uint64_t DelayFree(uint64_t ID, LWERenderer *Renderer);

	void Release(LWVideoDriver *Driver);

	uint32_t BuildInputStreams(LWPipeline *Pipeline, LWPipelineInputStream StreamBuffer[LWShader::MaxInputs], LWVideoBuffer *ModelIDBuffer);

	LWERendererBlockGeometry &BuildAllPrimitives(LWERenderer *Renderer);

	LWShaderInput &GetAttribute(uint32_t Idx);

	LWShaderInput &GetPositionAttribute(uint32_t Idx);

	template<class PositionType, class AttributeType>
	bool GetLocalForID(uint64_t ID, PositionType *&VertexPositions, AttributeType *&VertexAttributes, uint32_t *&Indices) {
		uint32_t VertID = ((ID & VerticeBits)>>VerticeBitOffset) * m_VerticesPerBlock;
		uint32_t IndID = ((ID & IndiceBits)>>IndiceBitOffset) * m_IndicesPerBlock;
		VertexPositions = GetLocalVerticePositionsAt<PositionType>(VertID);
		VertexAttributes = GetLocalVerticeAttributesAt<AttributeType>(VertID);
		Indices = GetLocalIndicesAt(IndID);
		return VertexPositions != nullptr;
	}

	uint8_t *GetLocalVerticePositionsAt(uint32_t Index);

	template<class Type>
	Type *GetLocalVerticePositionsAt(uint32_t Index) {
		return (Type*)GetLocalVerticePositionsAt(Index);
	}

	uint8_t *GetLocalVerticeAttributesAt(uint32_t Index);

	template<class Type>
	Type *GetLocalVerticeAttributesAt(uint32_t Index) {
		return (Type*)GetLocalVerticeAttributesAt(Index);
	}

	uint32_t *GetLocalIndicesAt(uint32_t Index);

	uint32_t GetAttributeCount(void) const;

	uint32_t GetPositionAttributeCount(void) const;

	uint32_t FindAttribute(const LWUTF8Iterator &Name, bool Verbose = true) const;

	uint32_t FindAttribute(uint32_t NameHash, bool Verbose = true) const;

	uint32_t FindPositionAttribute(const LWUTF8Iterator &Name, bool Verbose = true) const;

	uint32_t FindPositionAttribute(uint32_t NameHash, bool Verbose = true) const;

	uint32_t GetVerticeTypeSize(void) const;

	uint32_t GetPositionTypeSize(void) const;

	uint32_t GetAttributeTypeSize(void) const;

	uint32_t GetNameHash(void) const;

	//Remaps primitive type's to Geometry attributes(where Applicable, TexCoord, Normal, Tangent), also create's triangle indice list if primitive doesn't supply indice's(unless block has no indice buffer, in which case the vice-versa is performed).
	//Returns id for the primitive
	uint64_t UploadPrimitive(const LWEStaticVertice *VerticePos, const LWEVerticePBR *VerticeAttr, uint32_t VerticeCount, uint32_t *IndiceList, uint32_t IndiceCount, LWERenderer *Renderer);

	uint64_t GetBoxPrimitive(LWERenderer *Renderer = nullptr);

	uint64_t GetSpherePrimitive(LWERenderer *Renderer = nullptr);

	uint64_t GetConePrimtive(LWERenderer *Renderer = nullptr);

	uint64_t GetCapsulePrimitive(LWERenderer *Renderer = nullptr);

	uint64_t GetCylinderPrimitive(LWERenderer *Renderer = nullptr);

	uint64_t GetDomePrimitive(LWERenderer *Renderer = nullptr);

	uint64_t GetPlanePrimitive(LWERenderer *Renderer = nullptr);

	LWVideoBuffer *GetIndiceBuffer(void);

	LWERendererBlockGeometry(LWVideoDriver *Driver, LWAllocator &Allocator, uint32_t NameHash, LWShaderInput *PositionLayout, uint32_t PositionCount, LWShaderInput *AttributeLayout, uint32_t AttributeCount, bool LocalCopy, uint32_t VerticesPerBlock = DefaultVerticesPerBlock, uint32_t MaxVerticeBlocks = DefaultMaxIndiceBlocks, uint32_t IndicesPerBlock = DefaultIndicesPerBlock, uint32_t MaxIndiceBlocks = DefaultMaxIndiceBlocks);

	LWERendererBlockGeometry() = default;
	
	friend class LWERenderer;

	friend struct LWEGeometryBucket;
private:
	bool UploadTo(uint64_t ID, uint8_t *VertexPosition, uint8_t *VertexAttributes, uint32_t *Indices, uint32_t VerticeCount, uint32_t IndiceCount, LWVideoDriver *Driver);

	LWIndirectIndice MakeDrawCall(uint64_t ID, uint32_t InstanceOffset = 0, uint32_t Offset = 0, uint32_t Count = 0);

	std::mutex m_AllocateLock;
	std::unordered_map<uint64_t, std::pair<uint32_t, uint32_t>> m_CountMap;
	LWShaderInput m_PositionLayout[LWShader::MaxInputs]; //Input layout for binding to pipeline, when making debug primitives, the first vec4 must be Position, everything else will be 0'd out.
	LWShaderInput m_AttributeLayout[LWShader::MaxInputs]; //Attribute layout for binding to pipeline, When making debug primitives, Normal, TexCoord, Tangent are all supported name's, all other attributes will be set to 0.
	LWERendererBlockAllocator m_VerticeBlocks;
	LWERendererBlockAllocator m_IndiceBlocks;
	LWVideoBuffer *m_VertexPositionBuffer = nullptr;
	LWVideoBuffer *m_VertexAttributesBuffer = nullptr;
	LWVideoBuffer *m_IndiceBuffer = nullptr;
	uint32_t m_PositionTypeSize = 0;
	uint32_t m_AttributeTypeSize = 0;
	uint32_t m_VerticesPerBlock = 0;
	uint32_t m_IndicesPerBlock = 0;
	uint32_t m_AttributeCount = 0;
	uint32_t m_PositionCount = 0;
	uint32_t m_NameHash = LWUTF8I::EmptyHash;
	uint64_t m_BoxPrimitive = NullID;
	uint64_t m_SpherePrimitive = NullID;
	uint64_t m_ConePrimitive = NullID;
	uint64_t m_CapsulePrimitive = NullID;
	uint64_t m_CylinderPrimitive = NullID;
	uint64_t m_DomePrimitive = NullID;
	uint64_t m_PlanePrimitive = NullID;

};

struct LWERenderPendingBuffer {
	uint8_t *m_Data = nullptr;
	uint32_t m_BufferType = 0;
	uint32_t m_BufferUsage = 0;
	uint32_t m_Offset = 0;
	uint32_t m_TypeSize = 0;
	uint32_t m_Count = 0;

	LWERenderPendingBuffer(uint8_t *Data, uint32_t BufferType, uint32_t BufferUsage, uint32_t TypeSize, uint32_t Count, uint32_t Offset = 0);

	LWERenderPendingBuffer() = default;
};

struct LWERendererPendingBlockGeometry {
	uint8_t *m_VertexPosition = nullptr;
	uint8_t *m_VertexAttributes = nullptr;
	uint32_t *m_Indices = nullptr;
	uint32_t m_VerticeCount = 0;
	uint32_t m_IndiceCount = 0;

	LWERendererPendingBlockGeometry(uint8_t *VertexPosition, uint8_t *VertexAttributes, uint32_t *Indices, uint32_t VerticeCount, uint32_t IndiceCount);

	LWERendererPendingBlockGeometry() = default;
};

struct LWERenderTextureProps {
	uint32_t m_TextureState = 0;
	uint32_t m_PackType = -1; //-1 packtype means not to create this texture.
	uint32_t m_TextureType = LWTexture::Texture2D;
	uint32_t m_Layers = 1;
	uint32_t m_Samples = 4; //MS textures

	LWERenderTextureProps(uint32_t TextureState);

	LWERenderTextureProps(uint32_t TextureState, uint32_t PackType, uint32_t TextureType = LWTexture::Texture2D, uint32_t Layers = 1, uint32_t Samples = 4);

	LWERenderTextureProps() = default;
};

struct LWERenderPendingTexture {
	LWImage *m_Image = nullptr;
	LWVector2f m_DynamicSize;
	LWVector2i m_StaticSize;
	uint32_t m_NamedDynamic = LWUTF8I::EmptyHash;
	LWERenderTextureProps m_TexProps;

	bool isDynamicSized(void) const;

	LWERenderPendingTexture(LWImage *Image, uint32_t TextureState);

	LWERenderPendingTexture(const LWVector2i &StaticSize, const LWERenderTextureProps &Props);

	LWERenderPendingTexture(const LWVector2f &DynamicSize, const LWERenderTextureProps &Props, uint32_t NamedDynamic = LWUTF8I::EmptyHash);

	LWERenderPendingTexture() = default;
};

struct LWERenderFramebufferTexture {
	LWBitField32(NamedLayer, 16, 0); 
	LWBitField32(NamedFace, 8, NamedLayerBitsOffset + 16);
	LWBitField32(NamedMipmap, 7, NamedFaceBitsOffset + 8);
	static const uint32_t FrameBufferName = 0x80000000; //if framebuffer is set, layer is used to refer to the attachment point being referenced.

	uint32_t m_NameHash = LWUTF8I::EmptyHash; //If name is non-empty, the texture will be searched for.
	uint32_t m_NamedFlags = 0;
	LWERenderTextureProps m_TexProps;

	uint32_t GetNamedMipmap(void) const;

	uint32_t GetNamedLayer(void) const;

	uint32_t GetNamedFace(void) const;

	bool isFramebufferName(void) const;

	LWERenderFramebufferTexture(const LWUTF8Iterator &Name, uint32_t Mipmap=0, uint32_t Layer=0, uint32_t Face=0, bool bIsFramebufferName = false);

	LWERenderFramebufferTexture(uint32_t NameHash, uint32_t Mipmap=0, uint32_t Layer=0, uint32_t Face=0, bool bIsFramebufferName = false);

	LWERenderFramebufferTexture(const LWERenderTextureProps &Props);

	LWERenderFramebufferTexture() = default;
};

struct LWERenderPendingFrameBuffer {	
	LWVector2f m_DynamicSize;
	LWVector2i m_StaticSize;
	uint32_t m_NamedDynamic = LWUTF8I::EmptyHash;

	LWERenderFramebufferTexture m_Textures[LWFrameBuffer::Count];

	bool isDynamicSized(void) const;

	LWERenderPendingFrameBuffer(const LWVector2i &StaticSize);

	LWERenderPendingFrameBuffer(const LWVector2f &DynamicSize, uint32_t NamedDynamic = LWUTF8I::EmptyHash);

	LWERenderPendingFrameBuffer() = default;
};

struct LWERenderPendingResource {
	LWBitField32(Type, 2, 0);
	static const uint32_t NoDiscard = 0x4; //If NoDiscard is raised, then the data/Image are not destroyed once upload is complete.
	static const uint32_t DestroyResource = 0x8; //The renderer reference to the object this resource refers to is destroyed.
	static const uint32_t WriteOverlap = 0x10; //If buffer already exists, then this buffer will update a portion of that buffer.

	static const uint32_t tBuffer = 0;
	static const uint32_t tTexture = 1;
	static const uint32_t tFrameBuffer = 2;
	static const uint32_t tBlockGeometry = 3;

	bool isNoDiscard(void) const;

	bool isDestroyResource(void) const;

	bool isWriteOverlap(void) const;//if buffer type, then if the buffer exists, will attempt to write to a partial portion of the buffer.

	void Finished(void); //clean's up the internal data if NoDiscard is not set.

	uint32_t GetType(void) const;

	LWERenderPendingResource(uint32_t ID, uint32_t Flag, const LWERenderPendingBuffer &Buffer);

	LWERenderPendingResource(uint32_t ID, uint32_t Flag, const LWERenderPendingTexture &Texture);

	LWERenderPendingResource(uint32_t ID, uint32_t Flag, const LWERenderPendingFrameBuffer &FrameBuffer);

	LWERenderPendingResource(uint32_t ID, const LWUTF8Iterator &Name, uint32_t Flag, const LWERenderPendingBuffer &Buffer);

	LWERenderPendingResource(uint32_t ID, const LWUTF8Iterator &Name, uint32_t Flag, const LWERenderPendingTexture &Texture);

	LWERenderPendingResource(uint32_t ID, const LWUTF8Iterator &Name, uint32_t Flag, const LWERenderPendingFrameBuffer &FrameBuffer);

	LWERenderPendingResource(uint64_t ID, const LWUTF8Iterator &Name, uint32_t Flag, const LWERendererPendingBlockGeometry &BlockGeom);

	LWERenderPendingResource(uint32_t ID, uint32_t NameHash, uint32_t Flag, const LWERenderPendingBuffer &Buffer);

	LWERenderPendingResource(uint32_t ID, uint32_t NameHash, uint32_t Flag, const LWERenderPendingTexture &Texture);

	LWERenderPendingResource(uint32_t ID, uint32_t NameHash, uint32_t Flag, const LWERenderPendingFrameBuffer &FrameBuffer);

	LWERenderPendingResource(uint64_t ID, uint32_t NameHash, uint32_t Flag, const LWERendererPendingBlockGeometry &BlockGeom);

	LWERenderPendingResource(uint32_t ID, const LWERenderPendingBuffer &Buffer); //Raises the DestroyResource flag(will not destroy BlockGeometry), Second parameter is only for type overloading.

	LWERenderPendingResource(uint32_t ID, const LWERenderPendingTexture &Texture); //Raises the DestroyResource flag(will not destroy BlockGeometry), Second parameter is only for type overloading.

	LWERenderPendingResource(uint32_t ID, const LWERenderPendingFrameBuffer &FrameBuffer); //Raises the DestroyResource flag(will not destroy BlockGeometry), Second parameter is only for type overloading.

	LWERenderPendingResource(uint64_t ID, uint32_t NameHash); //Raises the destroy flag for BlockedGeometry to release the specified ID chunks.

	LWERenderPendingResource() = default;

	//(non-trival copy constructors prevent using union to save a bit of memory).
	LWERenderPendingBuffer m_Buffer;
	LWERenderPendingTexture m_Texture;
	LWERenderPendingFrameBuffer m_FrameBuffer;
	LWERendererPendingBlockGeometry m_BlockGeometry;

	uint64_t m_ID = 0;
	uint32_t m_NameHash = LWUTF8I::EmptyHash;
	uint32_t m_Flag = 0;
};


struct LWERendererDynamicFramebuffer {
	uint32_t m_ID; //ID for
	LWVector2f m_DynamicSize;
	uint32_t m_NamedDynamic = LWUTF8I::EmptyHash;
	LWERenderFramebufferTexture m_Textures[LWFrameBuffer::Count];
};

struct LWERendererDynamicTexture {
	uint32_t m_ID;
	LWVector2f m_DynamicSize;
	uint32_t m_NamedDynamic = LWUTF8I::EmptyHash;
	LWERenderTextureProps m_Propertys;
};

struct LWERendererDynamicScalar {
	LWVector2f m_Size;
	uint32_t m_DynamicID = 0;
};

class LWERenderer {
public:
	static const uint32_t FrameBuffer = 3;
	static const uint32_t MaxPendingResources = 512;
	static const uint32_t FBAttachmentIDBit = 0x80000000;

	//Add's built-in pass type's to the function map used during the renderer creation process.
	static void GenerateDefaultXMLPasses(std::unordered_map<uint32_t, LWEPassXMLCreateFunc> &PassMap);

	/*!< \brief Parse's xml size: (x(%)|y(%)|NamedDynamic, if % is included in parameters then DynamicSize is set(and the texture/framebuffer will automatically be adjusted when either the window size is changed, or if NamedDynamic is set then when the Named dynamic size has changed), whereas static size is set if not included. */
	bool ParseXMLSizeAttribute(LWEXMLAttribute *SizeAttribute, LWVector2i &StaticSize, LWVector2f &DynamicSize, uint32_t &NamedDynamic);

	/*!< \brief parse's following attributes: Name(Required), Size(Required), Samples(all texture's are created as Multi-sampled textures with this number of samples). Color, Color1, Color2, Color3, Color4, Color5, Color6, Depth
	*	ColorX+Depth parameters:
	*		if named parameter is a texture: 'TextureName:LayerIdx:MipmapIdx:Face(+X, -X, +Y, -Z, +Z, -Z)' omitting a parameter will default to 0 for that parameter.
	*		if named parameter is another framebuffer: 'FramebufferName:(Color(X)/Depth)'
	*       if parameter is not named, a Texture2D image is created with the following parameters: 'PackType(see LWImage for options):TextureState(see list of state's available in LWTexture, if LWTexture::RenderTarget/RenderBuffer is omitted, RenderTarget is automatically included.)' */
	bool ParseXMLFrameBuffer(LWEXMLNode *Node);

	/*!< \brief parse's the following nodes(Texture1D, Texture2D, Texture3D, TextureCubemap, Texture1DArray, Texture2DArray, TextureCubeArray, Texture2DMS, Texture2DMSArray) with the following attributes: Name(Required), Size(Required), PackType(Required, see LWImage::PackType for options), TextureState(Default: 0, or'd list of LWTexture state's, if the texture is to be used by a framebuffer, it must also have RenderTarget set), Layers(default: 1, z size for 3D textures, or number of layers for array textures), Samples(default: 4, for MS texture's this is the number of samples to use, must be in 2^n.) */
	bool ParseXMLTexture(LWEXMLNode *Node);
	
	/*!< \brief parse's the following nodes(VertexBuffer, UniformBuffer, Index16Buffer, Index32Buffer, ImageBuffer, IndirectBuffer) with the following attributes: Name(Required), TypeSize(Required, size of each element), Count(Required, total number of elements), Usage(Default: Static, see LWVideoBuffer for all Usage types). LocalCopy flag is added with a no-value attribute "LocalCopy". */
	bool ParseXMLVideoBuffer(LWEXMLNode *Node);

	/*!< \brief add's a named dynamic scalar to use. (Node: DynamicScalar, Attributes: Name(Required), Width(#), Height(#)), note if the DynamicScalar already exists, then the Width+Height are ignored unless a non-valued FORCE attribute is set(this is due to the assumption a setting file has loaded it's own values) */
	bool ParseXMLNamedDynamicScalar(LWEXMLNode *Node);

	/*!< \brief parse's blocked geometry buffer: (Attributes: Name(Required),  TypeSize(Semi-Required, byte size of each vertice) Type(If included, TypeSize+Attribute/PositionMap are not required,  supported named Types are 'VertexStatic', 'VertexSkeleton', which will generate the relevant maps),  VerticesPerBlock(Optional, number of vertices in each block of memory). MaxVerticeBlocks(Number of vertice blocks, Total Vertice buffer size = TypeSize*VerticesPerBlock*MaxVerticeBlocks)  IndicesPerBlock(Number of indice's in a block, each indice is a 32 bit unsigned integer).  MaxIndiceBlocks(Number of indice's block Total indice yte buffer size = 4*IndicesPerBlock*MaxIndiceBlocks) BuildPrimitives(Value-less attribute that indicates primitives should be generated for this blocked buffer) Debug(Value-less attribute that indicates block is used for RenderFrame debugging geometry calls) Local(Value-less attribute that indicates all vertex buffer's should have a local copy for writing to).
	*	 -blocked geometry has sub nodes:
	*		AttributeMap - Layout of attributes, this is required for debug primitives to add Normal/Texcoord if present, layout is exactly like how LWShader input layout works
	*		PositionMap - Layout of position attributes, if this is omitted then Vec4 Position is defaulted.
	*/
	bool ParseXMLBlockGeometry(LWEXMLNode *Node);

	//Userdata must be the FuncMap that was populated by user-types and GenerateDefaultXMLPasses.
	bool ParseXML(LWEXMLNode *Node, void *UserData, LWEXML*);

	LWERenderFrame *BeginFrame(void);

	void EndFrame(void);

	bool SizeChanged(void);

	void NamedDynamicChanged(void);

	void ApplyFrame(LWERenderFrame &Frame);

	void Render(float dTime, uint32_t SwapInterval, uint64_t lCurrentTime, uint64_t PendingResourceMaxPerFrameTime);

	void ProcessPendingResources(uint64_t MaxPerFrameTime);

	LWERendererBlockGeometry *CreateBlockedGeometry(const LWUTF8I &Name, LWShaderInput *PositionLayout, uint32_t PositionLayoutCount, LWShaderInput *AttributeLayout, uint32_t AttributeLayoutCount, bool LocalCopy = false, uint32_t VerticesPerBlock = LWERendererBlockGeometry::DefaultVerticesPerBlock, uint32_t MaxVerticeBlocks = LWERendererBlockGeometry::DefaultMaxIndiceBlocks, uint32_t IndicesPerBlock = LWERendererBlockGeometry::DefaultIndicesPerBlock, uint32_t MaxIndiceBlocks = LWERendererBlockGeometry::DefaultMaxIndiceBlocks);

	LWERendererBlockGeometry *CreateBlockedGeometry(uint32_t NameHash, LWShaderInput *PositionLayout, uint32_t PositionLayoutCount, LWShaderInput *AttributeLayout, uint32_t AttributeLayoutCount, bool LocalCopy = false, uint32_t VerticesPerBlock = LWERendererBlockGeometry::DefaultVerticesPerBlock, uint32_t MaxVerticeBlocks = LWERendererBlockGeometry::DefaultMaxIndiceBlocks, uint32_t IndicesPerBlock = LWERendererBlockGeometry::DefaultIndicesPerBlock, uint32_t MaxIndiceBlocks = LWERendererBlockGeometry::DefaultMaxIndiceBlocks);

	LWERenderer &SetAssetManager(LWEAssetManager *AssetManager);

	//Must be set before FinalizePasses is called, otherwise each frame will need to be updated.
	LWERenderer &SetDebugGeometry(LWERendererBlockGeometry *DebugGeometry);

	//Change's or add's the dynamic size of the specified DynamicScalar, if changing then the Dirty flag is raised.
	void SetNamedDynamicSize(const LWUTF8Iterator &Name, const LWVector2f &Size);

	void SetNamedDynamicSize(uint32_t NameHash, const LWVector2f &Size);

	//if Resource.m_ID = 0, then generates a new id, and returns that id.  returning 0 means failed to add resource to pending list.
	uint32_t PushPendingResource(const LWERenderPendingResource &Resource);

	uint32_t NextTextureID(void);

	uint32_t NextVideoBufferID(void);

	uint32_t NextFramebufferID(void);

	LWFrameBuffer *GetFrameBuffer(uint32_t ID);

	LWVideoBuffer *GetVideoBuffer(uint32_t ID);

	LWTexture *GetTexture(uint32_t ID);

	LWEPass *GetPass(uint32_t Idx);

	template<class Type>
	Type *GetPass(uint32_t Idx) {
		return (Type*)m_PassList[Idx];
	}

	uint32_t FindNamedFrameBuffer(const LWUTF8Iterator &Name, bool Verbose = true);

	uint32_t FindNamedFrameBuffer(uint32_t NameHash, bool Verbose = true);

	uint32_t FindNamedVideoBuffer(const LWUTF8Iterator &Name, bool Verbose = true);

	uint32_t FindNamedVideoBuffer(uint32_t NameHash, bool Verbose = true);

	uint32_t FindNamedTexture(const LWUTF8Iterator &Name, bool Verbose = true);

	uint32_t FindNamedTexture(uint32_t NameHash, bool Verbose = true);

	LWERendererBlockGeometry *FindNamedBlockGeometryMap(const LWUTF8Iterator &Name, bool Verbose = true);

	LWERendererBlockGeometry *FindNamedBlockGeometryMap(uint32_t NameHash, bool Verbose = true);

	LWEPass *FindNamedPass(const LWUTF8Iterator &Name, bool Verbose = true);

	LWEPass *FindNamedPass(uint32_t NameHash, bool Verbose = true);

	const LWERendererDynamicScalar *FindDynamicScalar(const LWUTF8Iterator &Name, bool Verbose = true) const;

	const LWERendererDynamicScalar *FindDynamicScalar(uint32_t NameHash, bool Verbose = true) const;

	bool FinalizePasses(void);

	template<class Type>
	Type *FindNamedPass(const LWUTF8Iterator &Name, bool Verbose = true) {
		return (Type*)FindNamedPass(Name, Verbose);
	}

	template<class Type>
	Type *FindNamedPass(uint32_t NameHash, bool Verbose = true) {
		return (Type*)FindNamedPass(NameHash, Verbose);
	}

	LWAllocator &GetAllocator(void);

	LWVideoBuffer *GetPPScreenVertices(void);

	LWVideoBuffer *GetFrameGlobalDataBuffer(void);

	LWVideoBuffer *GetFramePassDataBuffer(void);

	LWVideoBuffer *GetFrameModelDataBuffer(void);

	LWVideoBuffer *GetFrameBoneDataBuffer(void);

	LWVideoBuffer *GetFrameLightDataBuffer(void);

	LWERendererBlockGeometry *GetDebugGeometry(void);

	const LWELoggerTimeMetrics &GetApplyFrameMetric(void) const;

	const LWELoggerTimeMetrics &GetRenderFrameMetric(void) const;

	const LWELoggerTimeMetrics &GetPendingFrameMetric(void) const;

	LWEAssetManager *GetAssetManager(void);

	LWVideoDriver *GetVideoDriver(void);

	uint32_t GetPassCount(void) const;

	float GetTime(void) const;

	LWERenderer(LWVideoDriver *Driver, LWEAssetManager *AssetManager, LWAllocator &Allocator);

	~LWERenderer();
protected:
	LWTexture *CreateTexture(const LWVector2i &TexSize, LWERenderTextureProps &Props);

	LWFrameBuffer *CreateFramebuffer(const LWVector2i &FramebufferSize, LWERenderFramebufferTexture TextureList[LWFrameBuffer::Count]);

	LWFrameBuffer *DestroyFrameBuffer(LWFrameBuffer *FB);

	LWEGeometryRenderable m_GeometryRendableList[LWEMaxGeometryBuckets][LWEMaxBucketSize];
	std::pair<uint32_t, uint32_t> m_GeometryRendableCount[LWEMaxGeometryBuckets];
	LWConcurrentFIFO<LWERenderPendingResource, MaxPendingResources> m_PendingResources;
	LWERenderFrame m_Frames[FrameBuffer];
	std::vector<LWEPass*> m_PassList;
	LWAllocator &m_Allocator;
	LWVideoDriver *m_Driver = nullptr;
	LWEAssetManager *m_AssetManager = nullptr;
	LWERendererBlockGeometry *m_DebugGeometry = nullptr;

	LWVideoBuffer *m_PPScreenVertices = nullptr;
	LWVideoBuffer *m_FrameGlobalDataBuffer = nullptr;
	LWVideoBuffer *m_FramePassDataBuffer = nullptr;
	LWVideoBuffer *m_FrameModelDataBuffer = nullptr;
	LWVideoBuffer *m_FrameBoneDataBuffer = nullptr;
	LWVideoBuffer *m_FrameLightDataBuffer = nullptr;

	std::unordered_map<uint32_t, LWVideoBuffer*> m_BufferMap;
	std::unordered_map<uint32_t, LWTexture*> m_TextureMap;
	std::unordered_map<uint32_t, LWFrameBuffer*> m_FramebufferMap;

	std::unordered_map<uint32_t, uint32_t> m_NamedBufferMap;
	std::unordered_map<uint32_t, uint32_t> m_NamedTextureMap;
	std::unordered_map<uint32_t, uint32_t> m_NamedFramebufferMap;
	std::unordered_map<uint32_t, LWERendererBlockGeometry*> m_NamedBlockGeometryMap;
	std::vector<LWERendererDynamicFramebuffer> m_DynamicFramebufferMap;
	std::vector<LWERendererDynamicTexture> m_DynamicTextureMap;
	std::unordered_map<uint32_t, LWERendererDynamicScalar> m_NamedDynamicMap;

	std::atomic<uint32_t> m_NextBufferID = 1;
	std::atomic<uint32_t> m_NextTextureID = 1;
	std::atomic<uint32_t> m_NextFramebufferID = 1;
	std::atomic<uint32_t> m_WriteFrame = 0;
	std::atomic<uint32_t> m_ReadFrame = 0;
	LWELoggerTimeMetrics m_ApplyFrameMetric;
	LWELoggerTimeMetrics m_RenderFrameMetric;
	LWELoggerTimeMetrics m_PendingFrameMetric;
	uint32_t m_NamedDynamicID = 0;
	float m_Time = 0.0f;
	bool m_SizeChanged = true;
	bool m_NamedDynamicChanged = false;
};

#endif