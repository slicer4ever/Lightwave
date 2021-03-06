#include "LWVideo/LWVideoDrivers/LWVideoDriver_DirectX11_1.h"
#include "LWVideo/LWTypes.h"
#include "LWVideo/LWImage.h"
#include "LWVideo/LWFrameBuffer.h"
#include "LWVideo/LWPipeline.h"
#include "LWCore/LWVector.h"
#include "LWCore/LWText.h"
#include "LWCore/LWMath.h"
#include "LWPlatform/LWWindow.h"

template<class Type>
struct ViewArray {
	static const uint32_t MaxViews = 32;
	Type *m_Views[MaxViews];
	uint32_t m_First[MaxViews];
	uint32_t m_Length[MaxViews];
	uint32_t m_Count;

	static bool PushLists(Type *View, uint32_t First, uint32_t Length, uint32_t Flag, ViewArray<Type> &VertexList, ViewArray<Type> &GeomList, ViewArray<Type> &PixelList, ViewArray<Type> &ComputeList) {
		if (Flag&LWShaderResource::VertexStage) VertexList.Push(View, First, Length);
		if (Flag&LWShaderResource::GeometryStage) GeomList.Push(View, First, Length);
		if (Flag&LWShaderResource::PixelStage) PixelList.Push(View, First, Length);
		if (Flag&LWShaderResource::ComputeStage) ComputeList.Push(View, First, Length);
		return true;
	};

	bool Push(Type *View, uint32_t First, uint32_t Length) {
		if (m_Count >= MaxViews) return false;
		m_Views[m_Count] = View;
		m_First[m_Count] = First;
		m_Length[m_Count] = Length;
		m_Count++;
		return true;
	}

	void Fill(uint32_t TotalCount, Type *Value, uint32_t First, uint32_t Length) {
		while (m_Count < TotalCount) Push(Value, First, Length);
		return;
	}

	ViewArray() : m_Count(0) {}
};

LWVideoDriver &LWVideoDriver_DirectX11_1::ViewPort(const LWVector4i &Viewport) {
	m_Viewport = Viewport;
	D3D11_VIEWPORT Port = { (float)m_Viewport.x, (float)m_Viewport.y, (float)m_Viewport.z, (float)m_Viewport.w, 0.0f, 1.0f };
	m_Context.m_DXDeviceContext->RSSetViewports(1, &Port);
	return *this;
}

LWShader *LWVideoDriver_DirectX11_1::CreateShader(uint32_t ShaderType, const char *Source, LWAllocator &Allocator, char *CompiledBuffer, char *ErrorBuffer, uint32_t *CompiledBufferLen, uint32_t ErrorBufferLen) {
	const uint32_t ShaderCnt = 4;
	const char *CompileModes[ShaderCnt] = { "vs_5_0", "ps_5_0", "gs_5_0", "cs_5_0" };
	uint32_t CompileFlag = D3D10_SHADER_ENABLE_STRICTNESS;
#if _DEBUG
	CompileFlag |= D3DCOMPILE_DEBUG;
#endif
	ID3D10Blob *Res = nullptr;
	ID3D10Blob *Err = nullptr;

	if (FAILED(D3DCompile(Source, strlen(Source), nullptr, nullptr, nullptr, "main", CompileModes[ShaderType], CompileFlag, 0, &Res, &Err))) {
		if (ErrorBuffer) {
			*ErrorBuffer = '\0';
			strncat(ErrorBuffer, (const char*)Err->GetBufferPointer(), ErrorBufferLen);
			Err->Release();
			PerformErrorAnalysis(Source, ErrorBuffer, ErrorBufferLen);
			return nullptr;
		}
	}
	const char *Compiled = (const char*)Res->GetBufferPointer();
	uint32_t CompiledLen = (uint32_t)Res->GetBufferSize();
	if (CompiledBuffer) {
		*CompiledBuffer = '\0';
		uint32_t Len = std::min<uint32_t>(CompiledLen, *CompiledBufferLen);
		std::copy(Compiled, Compiled + Len, CompiledBuffer);
		*CompiledBufferLen = Len;
	}
	LWShader *Shdr = CreateShaderCompiled(ShaderType, Compiled, CompiledLen, Allocator, ErrorBuffer, ErrorBufferLen);
	Res->Release();
	return Shdr;
}

LWShader *LWVideoDriver_DirectX11_1::CreateShaderCompiled(uint32_t ShaderType, const char *CompiledCode, uint32_t CompiledLen, LWAllocator &Allocator, char *ErrorBuffer, uint32_t ErroBufferLen) {
	LWDirectX11_1ShaderContext Context;
	D3D11_INPUT_ELEMENT_DESC LayoutElements[32];
	DXGI_FORMAT DXFormats[] = { DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32_FLOAT,  DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32B32_UINT, DXGI_FORMAT_R32G32B32_SINT, DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_FLOAT };
	uint32_t ShdrFormats[] = { LWShaderInput::Int,  LWShaderInput::Int, LWShaderInput::Float, LWShaderInput::iVec2,  LWShaderInput::iVec2,  LWShaderInput::Vec2,    LWShaderInput::iVec3,     LWShaderInput::iVec3,     LWShaderInput::Vec3,       LWShaderInput::iVec4,        LWShaderInput::iVec4,        LWShaderInput::Vec4 };

	bool Failed = true;
	if (ShaderType == LWShader::Vertex) Failed = FAILED(m_Context.m_DXDevice->CreateVertexShader(CompiledCode, CompiledLen, nullptr, &Context.m_VertexShader));
	else if (ShaderType == LWShader::Pixel) Failed = FAILED(m_Context.m_DXDevice->CreatePixelShader(CompiledCode, CompiledLen, nullptr, &Context.m_PixelShader));
	else if (ShaderType == LWShader::Geometry) Failed = FAILED(m_Context.m_DXDevice->CreateGeometryShader(CompiledCode, CompiledLen, nullptr, &Context.m_GeometryShader));
	else if (ShaderType == LWShader::Compute) Failed = FAILED(m_Context.m_DXDevice->CreateComputeShader(CompiledCode, CompiledLen, nullptr, &Context.m_ComputeShader));

	if (Failed) return nullptr;
	ID3D11ShaderReflection *Shdr = nullptr;
	Failed = FAILED(D3DReflect(CompiledCode, CompiledLen, IID_ID3D11ShaderReflection, (void**)&Shdr));
	if (Failed) {
		return nullptr;
	}
	D3D11_SHADER_DESC Desc;
	Shdr->GetDesc(&Desc);
	if (ShaderType == LWShader::Vertex) {
		for (uint32_t i = 0; i < Desc.InputParameters; i++) {
			D3D11_SIGNATURE_PARAMETER_DESC ParmDesc;
			Shdr->GetInputParameterDesc(i, &ParmDesc);
			uint32_t ParamOffset = (ParmDesc.Mask == 1 ? 0 : (ParmDesc.Mask <= 3 ? 1 : (ParmDesc.Mask <= 7 ? 2 : 3)));
			uint32_t ParamIdx = ParmDesc.ComponentType == 0 ? 0 : ParmDesc.ComponentType - 1;
			LayoutElements[i].SemanticName = ParmDesc.SemanticName;
			LayoutElements[i].SemanticIndex = ParmDesc.SemanticIndex;
			LayoutElements[i].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			LayoutElements[i].InputSlot = 0;
			LayoutElements[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			LayoutElements[i].InstanceDataStepRate = 0;
			LayoutElements[i].Format = DXFormats[ParamOffset * 3 + ParamIdx];
			Context.m_InputList[i] = { LWText::MakeHash(ParmDesc.SemanticName), ShdrFormats[ParamOffset * 3 + ParamIdx], 1 };
		}
		m_Context.m_DXDevice->CreateInputLayout(LayoutElements, Desc.InputParameters, CompiledCode, CompiledLen, &Context.m_InputLayout);
		Context.m_InputCount = Desc.InputParameters;
	}

	for (uint32_t i = 0; i < Desc.ConstantBuffers; i++) {
		D3D11_SHADER_BUFFER_DESC BufferDesc;
		Shdr->GetConstantBufferByIndex(i)->GetDesc(&BufferDesc);
		if(BufferDesc.Type!=D3D_CT_CBUFFER) continue;
		LWDirectX11_1ShaderResource &Block = Context.m_BlockList[Context.m_BlockCount];
		Block.m_Name = LWText::MakeHash(BufferDesc.Name);
		Block.m_Type = LWPipeline::UniformBlock;
		Block.m_Length = BufferDesc.Size;
		Context.m_BlockCount++;
	}

	for (uint32_t i = 0; i < Desc.BoundResources; i++) {
		D3D11_SHADER_INPUT_BIND_DESC Resource;
		Shdr->GetResourceBindingDesc(i, &Resource);
		if (Resource.Type == D3D_SIT_SAMPLER) continue;
		else if (Resource.Type == D3D_SIT_CBUFFER) continue;

		LWDirectX11_1ShaderResource &Resrc = Context.m_ResourceList[Context.m_ResourceCount];
		uint32_t NameHash = LWText::MakeHash(Resource.Name);
		Resrc.m_Name = NameHash;
		if (Resource.Type == D3D_SIT_TBUFFER || Resource.Type == D3D_SIT_TEXTURE) Resrc.m_Type = LWPipeline::Texture;
		else if (Resource.Type == D3D_SIT_UAV_RWTYPED) Resrc.m_Type = LWPipeline::Image;
		else if (Resource.Type == D3D_SIT_STRUCTURED || Resource.Type == D3D_SIT_BYTEADDRESS) Resrc.m_Type = LWPipeline::TextureBuffer;
		else if (Resource.Type == D3D_SIT_UAV_RWSTRUCTURED || Resource.Type == D3D_SIT_UAV_RWBYTEADDRESS) Resrc.m_Type = LWPipeline::ImageBuffer;
		else std::cout << "Error unknown resource: '" << Resource.Name << "' " << Resource.Type << std::endl;
		Context.m_ResourceCount++;
	}
	Shdr->Release();
	return Allocator.Allocate<LWDirectX11_1Shader>(Context, LWText::MakeHashb(CompiledCode, CompiledLen), ShaderType);
}


LWPipeline *LWVideoDriver_DirectX11_1::CreatePipeline(LWPipeline *Source, LWAllocator &Allocator) {
	LWDirectX11_1PipelineContext Context;
	LWShaderResource ResourceList[LWShader::MaxResources];
	LWShaderResource BlockList[LWShader::MaxBlocks];
	LWShaderInput InputList[LWShader::MaxInputs];
	LWShader *StageList[LWPipeline::StageCount] = { Source->GetShaderStage(0), Source->GetShaderStage(1), Source->GetShaderStage(2) };
	uint32_t ResourceCount = 0;
	uint32_t BlockCount = 0;
	uint32_t InputCount = 0;

	auto ForceInsertResource = [](uint32_t ShaderStage, LWDirectX11_1ShaderResource &Resource, LWShaderResource *List, uint32_t &ListCnt) {
		List[ListCnt++] = LWShaderResource(Resource.m_Name, ShaderStage, Resource.m_Type, Resource.m_Length);
		return;
	};

	auto InsertResource = [](uint32_t ShaderStage, LWDirectX11_1ShaderResource &Resource, LWShaderResource *List, uint32_t &ListCnt) {
		for (uint32_t i = 0; i < ListCnt; i++) {
			if (List[i].m_NameHash == Resource.m_Name) {
				List[i].m_Flag |= ShaderStage;
				return;
			}
		}
		List[ListCnt++] = LWShaderResource(Resource.m_Name, ShaderStage, Resource.m_Type, Resource.m_Length);
		return;
	};

	auto MergeStage = [&ResourceList, &BlockList, &ResourceCount, &BlockCount, &InsertResource](LWDirectX11_1ShaderContext &StageContext, uint32_t StageFlag)->void {
		for (uint32_t i = 0; i < StageContext.m_BlockCount; i++) InsertResource(StageFlag, StageContext.m_BlockList[i], BlockList, BlockCount);
		for (uint32_t i = 0; i < StageContext.m_ResourceCount; i++) InsertResource(StageFlag, StageContext.m_ResourceList[i], ResourceList, ResourceCount);
		return;
	};
	if (Source->isComputePipeline()) {
		LWDirectX11_1ShaderContext &CContext = ((LWDirectX11_1Shader*)StageList[LWPipeline::Compute])->GetContext();

		for (uint32_t i = 0; i < CContext.m_BlockCount; i++) ForceInsertResource(LWShaderResource::ComputeStage, CContext.m_BlockList[i], BlockList, BlockCount);
		for (uint32_t i = 0; i < CContext.m_ResourceCount; i++) ForceInsertResource(LWShaderResource::ComputeStage, CContext.m_ResourceList[i], ResourceList, ResourceCount);
		Context.m_ComputeShader = CContext.m_ComputeShader;
		Context.m_ComputeShader->AddRef();
		return Allocator.Allocate<LWDirectX11_1Pipeline>(Context, StageList, BlockList, ResourceList, nullptr, BlockCount, ResourceCount, 0, LWPipeline::InternalPipeline|LWPipeline::ComputePipeline);
	}
	if (StageList[LWPipeline::Vertex]) {
		LWDirectX11_1ShaderContext &VContext = ((LWDirectX11_1Shader*)StageList[LWPipeline::Vertex])->GetContext();
		for (uint32_t i = 0; i < VContext.m_InputCount; i++) {
			LWDirectX11_1ShaderResource &In = VContext.m_InputList[i];
			InputList[i] = LWShaderInput(In.m_Name, In.m_Type, In.m_Length);
		}
		InputCount = VContext.m_InputCount;
		MergeStage(VContext, LWShaderResource::VertexStage);
		Context.m_VertexShader = VContext.m_VertexShader;
		Context.m_InputLayout = VContext.m_InputLayout;
		Context.m_VertexShader->AddRef();
		Context.m_InputLayout->AddRef();
	}
	if (StageList[LWPipeline::Geometry]) {
		LWDirectX11_1ShaderContext &GContext = ((LWDirectX11_1Shader*)StageList[LWPipeline::Geometry])->GetContext();
		MergeStage(GContext, LWShaderResource::GeometryStage);
		Context.m_GeometryShader = GContext.m_GeometryShader;
		Context.m_GeometryShader->AddRef();
	}
	if (StageList[LWPipeline::Pixel]) {
		LWDirectX11_1ShaderContext &PContext = ((LWDirectX11_1Shader*)StageList[LWPipeline::Pixel])->GetContext();
		MergeStage(PContext, LWShaderResource::PixelStage);
		Context.m_PixelShader = PContext.m_PixelShader;
		Context.m_PixelShader->AddRef();
	}
	return Allocator.Allocate<LWDirectX11_1Pipeline>(Context, StageList, BlockList, ResourceList, InputList, BlockCount, ResourceCount, InputCount, LWPipeline::InternalPipeline);
}


LWVideoDriver &LWVideoDriver_DirectX11_1::ClonePipeline(LWPipeline *Target, LWPipeline *Source) {
	LWVideoDriver::ClonePipeline(Target, Source);
	LWDirectX11_1PipelineContext &SourceCon = ((LWDirectX11_1Pipeline*)Source)->GetContext();
	LWDirectX11_1PipelineContext &TargetCon = ((LWDirectX11_1Pipeline*)Target)->GetContext();
	TargetCon = SourceCon;
	return *this;
}

LWPipeline *LWVideoDriver_DirectX11_1::CreatePipeline(LWShader **ShaderStages, uint64_t Flags, LWAllocator &Allocator) {
	LWDirectX11_1PipelineContext Con;
	LWDirectX11_1Pipeline *P = Allocator.Allocate<LWDirectX11_1Pipeline>(Con, ShaderStages, nullptr, nullptr, nullptr, 0, 0, 0, Flags&~LWPipeline::InternalPipeline);
	if(P) UpdatePipelineStages(P);
	return P;
}

LWTexture *LWVideoDriver_DirectX11_1::CreateTexture1D(uint32_t TextureState, uint32_t PackType, uint32_t Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	//PackTypes:                   RGBA8,                      RGBA8U,                     RGBA16,                         RGBA16U,                        RGBA32,                        RGBA32U,                       RGBA32F,                        RG8,                    RG8U,                   RG16,                     RG16U,                    RG32,                    RG32U,                   RG32F,                    R8,                   R8U,                  R16,                   R16U,                  R32,                  R32U,                 R32F,                  Depth16,               Depth24,                       Depth32,               Depth24Stencil8,          DXT1                   DXT2                   DXT3                   DXT4                   DXT5                   DXT6                   DXT7  
	const DXGI_FORMAT Formats[] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_SNORM, DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R16G16_SNORM, DXGI_FORMAT_R16G16_UNORM, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R16_SNORM, DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_D16_UNORM, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC6H_SF16, DXGI_FORMAT_BC7_UNORM };
	LWDirectX11_1TextureContext Context;
	auto CheckResult = [&Context](HRESULT Res, const char *FuncName)->bool {
		if (SUCCEEDED(Res)) return true;
		std::cout << "CreateTexture1D Error '" << FuncName << "': " << std::hex << Res << std::dec << std::endl;
		if (Context.m_View) Context.m_View->Release();
		if (Context.m_Texture) Context.m_Texture->Release();
		return false;
	};
	bool MakeMipmaps = (TextureState & LWTexture::MakeMipmaps);
	bool Compressed = LWImage::CompressedType(PackType);
	bool DepthTex = LWImage::DepthType(PackType);
	bool RenderTarget = (TextureState&(LWTexture::RenderTarget | LWTexture::RenderBuffer));
	//TextureState = (TextureState & ~LWTexture::MakeMipmaps);
	uint32_t MipLevels = MakeMipmaps ? 0u : (MipmapCnt + 1u);
	uint32_t BindFlags = D3D11_BIND_SHADER_RESOURCE | ((DepthTex || Compressed) ? 0 : D3D11_BIND_UNORDERED_ACCESS) | ((RenderTarget || MakeMipmaps) ? D3D11_BIND_RENDER_TARGET : 0u);

	D3D11_TEXTURE1D_DESC Desc = { Size,
		MipLevels, 1u, Formats[PackType],
		D3D11_USAGE_DEFAULT, BindFlags, 0u,
		MakeMipmaps ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0u };
	D3D11_SHADER_RESOURCE_VIEW_DESC ViewDesc = { Formats[PackType], D3D11_SRV_DIMENSION_TEXTURE1D };
	ViewDesc.Texture1D = { 0u, (uint32_t)-1 };
	
	if (!CheckResult(m_Context.m_DXDevice->CreateTexture1D(&Desc, nullptr, (ID3D11Texture1D**)&Context.m_Texture), "CreateTexture1D")) return nullptr;
	if (!CheckResult(m_Context.m_DXDevice->CreateShaderResourceView(Context.m_Texture, &ViewDesc, &Context.m_View), "CreateShaderResourceView")) return nullptr;
	if (Texels) {
		uint32_t Mips = MakeMipmaps ? 0 : MipmapCnt;
		uint32_t t = 0;
		for (uint32_t i = 0; i <= Mips; i++, t++) {
			uint32_t S = LWImage::MipmapSize1D(Size, i);
			m_Context.m_DXDeviceContext->UpdateSubresource(Context.m_Texture, i, nullptr, Texels[t], LWImage::GetStride(Size, PackType), 0);
		}
	}
	//if (MakeMipmaps) m_Context.m_DXDeviceContext->GenerateMips(Context.m_View);
	return Allocator.Allocate<LWDirectX11_1Texture>(Context, TextureState, PackType, MakeMipmaps ? LWImage::MipmapCount(Size) : MipmapCnt, LWVector3i(Size, 0, 0), LWTexture::Texture1D);
}

LWTexture *LWVideoDriver_DirectX11_1::CreateTexture2D(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	//PackTypes:                   RGBA8,                      RGBA8U,                     RGBA16,                         RGBA16U,                        RGBA32,                        RGBA32U,                       RGBA32F,                        RG8,                    RG8U,                   RG16,                     RG16U,                    RG32,                    RG32U,                   RG32F,                    R8,                   R8U,                  R16,                   R16U,                  R32,                  R32U,                 R32F,                  Depth16,               Depth24,                          Depth32,                  Depth24Stencil8,                   DXT1                   DXT2                   DXT3                   DXT4                   DXT5                   DXT6                   DXT7  
	const DXGI_FORMAT Formats[] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_SNORM, DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R16G16_SNORM, DXGI_FORMAT_R16G16_UNORM, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R16_SNORM, DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R16_TYPELESS, DXGI_FORMAT_R24G8_TYPELESS,        DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R24G8_TYPELESS,        DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC6H_SF16, DXGI_FORMAT_BC7_UNORM };
	const DXGI_FORMAT VFormats[] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_SNORM, DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R16G16_SNORM, DXGI_FORMAT_R16G16_UNORM, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R16_SNORM, DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R16_UNORM,    DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_R32_FLOAT,    DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC6H_SF16, DXGI_FORMAT_BC7_UNORM };
	LWDirectX11_1TextureContext Context;
	auto CheckResult = [&Context](HRESULT Res, const char *FuncName)->bool {
		if (SUCCEEDED(Res)) return true;
		std::cout << "CreateTexture2D Error '" << FuncName << "': " << std::hex << Res << std::dec << std::endl;
		if (Context.m_View) Context.m_View->Release();
		if (Context.m_Texture) Context.m_Texture->Release();
		return false;
	};

	bool MakeMipmaps = (TextureState&LWTexture::MakeMipmaps);
	bool Compressed = LWImage::CompressedType(PackType);
	bool DepthTex = LWImage::DepthType(PackType);
	bool RenderTarget = (TextureState&(LWTexture::RenderTarget | LWTexture::RenderBuffer));
	TextureState = (TextureState & ~LWTexture::MakeMipmaps);
	uint32_t MipLevels = MakeMipmaps ? 0u : (MipmapCnt + 1u);
	uint32_t BindFlags = D3D11_BIND_SHADER_RESOURCE | ((DepthTex || Compressed) ? 0 : D3D11_BIND_UNORDERED_ACCESS) | ((RenderTarget || MakeMipmaps) ? (DepthTex ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_RENDER_TARGET) : 0u);
	D3D11_TEXTURE2D_DESC Desc = { (uint32_t)Size.x, (uint32_t)Size.y,
		MipLevels, 1u, Formats[PackType], { 1u, 0u},
		D3D11_USAGE_DEFAULT, BindFlags,	0u,	MakeMipmaps ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0u };
	D3D11_SHADER_RESOURCE_VIEW_DESC ViewDesc = { VFormats[PackType], D3D11_SRV_DIMENSION_TEXTURE2D, {0u, (uint32_t)-1} };
	if (!CheckResult(m_Context.m_DXDevice->CreateTexture2D(&Desc, nullptr, (ID3D11Texture2D**)&Context.m_Texture), "CreateTexture2D")) return nullptr;
	if (!CheckResult(m_Context.m_DXDevice->CreateShaderResourceView(Context.m_Texture, &ViewDesc, &Context.m_View), "CreateShaderResourceView")) return nullptr;
	if (Texels) {
		uint32_t Mips = MakeMipmaps ? 0 : MipmapCnt;
		uint32_t t = 0;
		for (uint32_t i = 0; i <= Mips; i++, t++) {
			LWVector2i S = LWImage::MipmapSize2D(Size, i);
			m_Context.m_DXDeviceContext->UpdateSubresource(Context.m_Texture, i, nullptr, Texels[t], LWImage::GetStride(S.x, PackType), 0);
		}
	}
	//if (MakeMipmaps) m_Context.m_DXDeviceContext->GenerateMips(Context.m_View);
	return Allocator.Allocate<LWDirectX11_1Texture>(Context, TextureState, PackType, MakeMipmaps ? LWImage::MipmapCount(Size) : MipmapCnt, LWVector3i(Size, 0), LWTexture::Texture2D);
}

LWTexture *LWVideoDriver_DirectX11_1::CreateTexture3D(uint32_t TextureState, uint32_t PackType, const LWVector3i &Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	//PackTypes:                   RGBA8,                      RGBA8U,                     RGBA16,                         RGBA16U,                        RGBA32,                        RGBA32U,                       RGBA32F,                        RG8,                    RG8U,                   RG16,                     RG16U,                    RG32,                    RG32U,                   RG32F,                    R8,                   R8U,                  R16,                   R16U,                  R32,                  R32U,                 R32F,                  Depth16,               Depth24,                       Depth32,               Depth24Stencil8,          DXT1                   DXT2                   DXT3                   DXT4                   DXT5                   DXT6                   DXT7  
	const DXGI_FORMAT Formats[] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_SNORM, DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R16G16_SNORM, DXGI_FORMAT_R16G16_UNORM, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R16_SNORM, DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_D16_UNORM, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC6H_SF16, DXGI_FORMAT_BC7_UNORM };
	LWDirectX11_1TextureContext Context;
	auto CheckResult = [&Context](HRESULT Res, const char *FuncName)->bool {
		if (SUCCEEDED(Res)) return true;
		std::cout << "CreateTexture3D Error '" << FuncName << "': " << std::hex << Res << std::dec << std::endl;
		if (Context.m_View) Context.m_View->Release();
		if (Context.m_Texture) Context.m_Texture->Release();
		return false;
	};
	bool MakeMipmaps = (TextureState&LWTexture::MakeMipmaps);
	bool Compressed = LWImage::CompressedType(PackType);
	bool DepthTex = LWImage::DepthType(PackType);
	bool RenderTarget = (TextureState&(LWTexture::RenderTarget|LWTexture::RenderBuffer));
	//TextureState = (TextureState & ~LWTexture::MakeMipmaps);
	uint32_t MipLevels = MakeMipmaps ? 0u : (MipmapCnt + 1u);
	uint32_t BindFlags = D3D11_BIND_SHADER_RESOURCE | ((DepthTex || Compressed) ? 0 : D3D11_BIND_UNORDERED_ACCESS) | ((RenderTarget || MakeMipmaps) ? D3D11_BIND_RENDER_TARGET : 0u);
	D3D11_TEXTURE3D_DESC Desc = { (uint32_t)Size.x, (uint32_t)Size.y, (uint32_t)Size.z,
		MipLevels, Formats[PackType],
		D3D11_USAGE_DEFAULT, BindFlags, 0u,
		MakeMipmaps ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0u };
	D3D11_SHADER_RESOURCE_VIEW_DESC ViewDesc = { Formats[PackType], D3D11_SRV_DIMENSION_TEXTURE3D,{ 0u, (uint32_t)-1 } };
	if (!CheckResult(m_Context.m_DXDevice->CreateTexture3D(&Desc, nullptr, (ID3D11Texture3D**)&Context.m_Texture), "CreateTexture3D")) return nullptr;
	if (!CheckResult(m_Context.m_DXDevice->CreateShaderResourceView(Context.m_Texture, &ViewDesc, &Context.m_View), "CreateShaderResourceView")) return nullptr;
	if (Texels) {
		uint32_t Mips = MakeMipmaps ? 0 : MipmapCnt;
		uint32_t t = 0;
		for (uint32_t i = 0; i <= Mips; i++, t++) {
			LWVector3i S = LWImage::MipmapSize3D(Size, i);
			m_Context.m_DXDeviceContext->UpdateSubresource(Context.m_Texture, i, nullptr, Texels[t], LWImage::GetStride(S.x, PackType), LWImage::GetLength2D(LWVector2i(S.x, S.y), PackType));
		}
	}
	//if (MakeMipmaps) m_Context.m_DXDeviceContext->GenerateMips(Context.m_View);
	return Allocator.Allocate<LWDirectX11_1Texture>(Context, TextureState, PackType, MakeMipmaps ? LWImage::MipmapCount(Size) : MipmapCnt, Size, LWTexture::Texture3D);
}

LWTexture *LWVideoDriver_DirectX11_1::CreateTextureCubeMap(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	//PackTypes:                   RGBA8,                      RGBA8U,                     RGBA16,                         RGBA16U,                        RGBA32,                        RGBA32U,                       RGBA32F,                        RG8,                    RG8U,                   RG16,                     RG16U,                    RG32,                    RG32U,                   RG32F,                    R8,                   R8U,                  R16,                   R16U,                  R32,                  R32U,                 R32F,                  Depth16,               Depth24,                       Depth32,               Depth24Stencil8,          DXT1                   DXT2                   DXT3                   DXT4                   DXT5                   DXT6                   DXT7  
	const DXGI_FORMAT Formats[] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_SNORM, DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R16G16_SNORM, DXGI_FORMAT_R16G16_UNORM, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R16_SNORM, DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R16_TYPELESS, DXGI_FORMAT_R24G8_TYPELESS,        DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R24G8_TYPELESS,        DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC6H_SF16, DXGI_FORMAT_BC7_UNORM };
	const DXGI_FORMAT VFormats[] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_SNORM, DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R16G16_SNORM, DXGI_FORMAT_R16G16_UNORM, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R16_SNORM, DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R16_UNORM,    DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_R32_FLOAT,    DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC6H_SF16, DXGI_FORMAT_BC7_UNORM };
	LWDirectX11_1TextureContext Context;
	auto CheckResult = [&Context](HRESULT Res, const char *FuncName)->bool {
		if (SUCCEEDED(Res)) return true;
		std::cout << "CreateTextureCubeMap Error '" << FuncName << "': " << std::hex << Res << std::dec << std::endl;
		if (Context.m_View) Context.m_View->Release();
		if (Context.m_Texture) Context.m_Texture->Release();
		return false;
	};
	bool MakeMipmaps = (TextureState&LWTexture::MakeMipmaps);
	bool Compressed = LWImage::CompressedType(PackType);
	bool DepthTex = LWImage::DepthType(PackType);
	bool RenderTarget = (TextureState&(LWTexture::RenderTarget|LWTexture::RenderBuffer));
	//TextureState = (TextureState & ~LWTexture::MakeMipmaps);
	uint32_t MipLevels = MakeMipmaps ? 0u : (MipmapCnt + 1u);
	uint32_t BindFlags = D3D11_BIND_SHADER_RESOURCE | ((DepthTex || Compressed) ? 0 : D3D11_BIND_UNORDERED_ACCESS) | ((RenderTarget || MakeMipmaps) ? (DepthTex ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_RENDER_TARGET) : 0u);
	uint32_t RenderFlags = (TextureState&LWTexture::RenderBuffer) == 0 ? 0 : (D3D11_DSV_READ_ONLY_DEPTH | D3D11_DSV_READ_ONLY_STENCIL);
	D3D11_TEXTURE2D_DESC Desc = { (uint32_t)Size.x, (uint32_t)Size.y,
		MipLevels, 6u, Formats[PackType],{ 1u, 0u },
		D3D11_USAGE_DEFAULT, BindFlags,
		0u,	D3D11_RESOURCE_MISC_TEXTURECUBE | (MakeMipmaps ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0u) };
	D3D11_SHADER_RESOURCE_VIEW_DESC ViewDesc = { VFormats[PackType], D3D11_SRV_DIMENSION_TEXTURECUBE };// , { 0u, (uint32_t)-1 }
	ViewDesc.TextureCube = { 0u, (uint32_t)-1 };
	
	if (!CheckResult(m_Context.m_DXDevice->CreateTexture2D(&Desc, nullptr, (ID3D11Texture2D**)&Context.m_Texture), "CreateTexture2D")) return nullptr;
	if (!CheckResult(m_Context.m_DXDevice->CreateShaderResourceView(Context.m_Texture, &ViewDesc, &Context.m_View), "CreateShaderResourceView")) return nullptr;
	
	if (Texels) {
		uint32_t Mips = MakeMipmaps ? 0 : MipmapCnt;
		uint32_t t = 0;
		for (uint32_t n = 0; n < 6; n++) {
			for (uint32_t i = 0; i <= Mips; i++, t++) {
				LWVector2i S = LWImage::MipmapSize2D(Size, i);
				m_Context.m_DXDeviceContext->UpdateSubresource(Context.m_Texture, t, nullptr, Texels ? Texels[t] : nullptr, LWImage::GetStride(S.x, PackType), 0);
			}
		}
	}
	//if (MakeMipmaps) m_Context.m_DXDeviceContext->GenerateMips(Context.m_View);
	return Allocator.Allocate<LWDirectX11_1Texture>(Context, TextureState, PackType, MakeMipmaps ? LWImage::MipmapCount(Size) : MipmapCnt, LWVector3i(Size, 0), LWTexture::TextureCubeMap);
}


LWTexture *LWVideoDriver_DirectX11_1::CreateTexture2DMS(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Samples, LWAllocator &Allocator) {
	//PackTypes:                   RGBA8,                      RGBA8U,                     RGBA16,                         RGBA16U,                        RGBA32,                        RGBA32U,                       RGBA32F,                        RG8,                    RG8U,                   RG16,                     RG16U,                    RG32,                    RG32U,                   RG32F,                    R8,                   R8U,                  R16,                   R16U,                  R32,                  R32U,                 R32F,                  Depth16,               Depth24,                          Depth32,                  Depth24Stencil8,                   DXT1                   DXT2                   DXT3                   DXT4                   DXT5                   DXT6                   DXT7  
	const DXGI_FORMAT Formats[] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_SNORM, DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R16G16_SNORM, DXGI_FORMAT_R16G16_UNORM, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R16_SNORM, DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R16_TYPELESS, DXGI_FORMAT_R24G8_TYPELESS,        DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R24G8_TYPELESS,        DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC6H_SF16, DXGI_FORMAT_BC7_UNORM };
	const DXGI_FORMAT VFormats[] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_SNORM, DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R16G16_SNORM, DXGI_FORMAT_R16G16_UNORM, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R16_SNORM, DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R16_UNORM,    DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_R32_FLOAT,    DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC6H_SF16, DXGI_FORMAT_BC7_UNORM };
	LWDirectX11_1TextureContext Context;
	auto CheckResult = [&Context](HRESULT Res, const char *FuncName)->bool {
		if (SUCCEEDED(Res)) return true;
		std::cout << "CreateTexture2DMS Error '" << FuncName << "': " << std::hex << Res << std::dec << std::endl;
		if (Context.m_View) Context.m_View->Release();
		if (Context.m_Texture) Context.m_Texture->Release();
		return false;
	};
	TextureState = (TextureState & ~LWTexture::MakeMipmaps); //Can't have mipmaps.
	bool Compressed = LWImage::CompressedType(PackType);
	bool DepthTex = LWImage::DepthType(PackType);
	bool RenderTarget = (TextureState & (LWTexture::RenderTarget | LWTexture::RenderBuffer));
	uint32_t BindFlags = D3D11_BIND_SHADER_RESOURCE | ((RenderTarget) ? (DepthTex ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_RENDER_TARGET) : 0u);
	D3D11_TEXTURE2D_DESC Desc = { (uint32_t)Size.x, (uint32_t)Size.y, 1u, 1u, Formats[PackType], {Samples, 0u}, D3D11_USAGE_DEFAULT, BindFlags, 0u, 0u };
	D3D11_SHADER_RESOURCE_VIEW_DESC ViewDesc = { VFormats[PackType], D3D11_SRV_DIMENSION_TEXTURE2DMS, {0u} };
	if (!CheckResult(m_Context.m_DXDevice->CreateTexture2D(&Desc, nullptr, (ID3D11Texture2D**)&Context.m_Texture), "CreateTexture2D")) return nullptr;
	if (!CheckResult(m_Context.m_DXDevice->CreateShaderResourceView(Context.m_Texture, &ViewDesc, &Context.m_View), "CreateShaderResourceView")) return nullptr;
	return Allocator.Allocate<LWDirectX11_1Texture>(Context, TextureState, PackType, Samples, LWVector3i(Size, 0), LWTexture::Texture2DMS);
}

LWTexture *LWVideoDriver_DirectX11_1::CreateTexture1DArray(uint32_t TextureState, uint32_t PackType, uint32_t Size, uint32_t Layers, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	//PackTypes:                   RGBA8,                      RGBA8U,                     RGBA16,                         RGBA16U,                        RGBA32,                        RGBA32U,                       RGBA32F,                        RG8,                    RG8U,                   RG16,                     RG16U,                    RG32,                    RG32U,                   RG32F,                    R8,                   R8U,                  R16,                   R16U,                  R32,                  R32U,                 R32F,                  Depth16,               Depth24,                       Depth32,               Depth24Stencil8,          DXT1                   DXT2                   DXT3                   DXT4                   DXT5                   DXT6                   DXT7  
	const DXGI_FORMAT Formats[] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_SNORM, DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R16G16_SNORM, DXGI_FORMAT_R16G16_UNORM, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R16_SNORM, DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_D16_UNORM, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC6H_SF16, DXGI_FORMAT_BC7_UNORM };
	LWDirectX11_1TextureContext Context;
	auto CheckResult = [&Context](HRESULT Res, const char *FuncName)->bool {
		if (SUCCEEDED(Res)) return true;
		std::cout << "CreateTexture1DArray Error '" << FuncName << "': " << std::hex << Res << std::dec << std::endl;
		if (Context.m_View) Context.m_View->Release();
		if (Context.m_Texture) Context.m_Texture->Release();
		return false;
	};
	bool MakeMipmaps = (TextureState&LWTexture::MakeMipmaps);
	bool Compressed = LWImage::CompressedType(PackType);
	bool DepthTex = LWImage::DepthType(PackType);
	bool RenderTarget = (TextureState&(LWTexture::RenderTarget | LWTexture::RenderBuffer));
	//TextureState = (TextureState & ~LWTexture::MakeMipmaps);

	uint32_t MipLevels = MakeMipmaps ? 0u : (MipmapCnt + 1u);
	uint32_t BindFlags = D3D11_BIND_SHADER_RESOURCE | ((DepthTex || Compressed) ? 0 : D3D11_BIND_UNORDERED_ACCESS) | ((RenderTarget || MakeMipmaps) ? D3D11_BIND_RENDER_TARGET : 0u);

	D3D11_TEXTURE1D_DESC Desc = { Size,
		MipLevels, Layers, Formats[PackType],
		D3D11_USAGE_DEFAULT, BindFlags, 0u,
		MakeMipmaps ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0u };
	D3D11_SHADER_RESOURCE_VIEW_DESC ViewDesc = { Formats[PackType], D3D11_SRV_DIMENSION_TEXTURE1DARRAY };
	ViewDesc.Texture1DArray = { 0u, (uint32_t)-1, 0u, Layers };
	
	if (!CheckResult(m_Context.m_DXDevice->CreateTexture1D(&Desc, nullptr, (ID3D11Texture1D**)&Context.m_Texture), "CreateTexture1D")) return nullptr;
	if (!CheckResult(m_Context.m_DXDevice->CreateShaderResourceView(Context.m_Texture, &ViewDesc, &Context.m_View), "CreateShaderResourceView")) return nullptr;
	if (Texels) {
		uint32_t Mips = MakeMipmaps ? 0 : MipmapCnt;
		uint32_t t = 0;
		for (uint32_t l = 0; l < Layers; l++) {
			for (uint32_t i = 0; i <= Mips; i++, t++) {
				uint32_t S = LWImage::MipmapSize1D(Size, i);
				m_Context.m_DXDeviceContext->UpdateSubresource(Context.m_Texture, t, nullptr, Texels[t], LWImage::GetStride(Size, PackType), 0);
			}
		}
	}
	//if (MakeMipmaps) m_Context.m_DXDeviceContext->GenerateMips(Context.m_View);
	return Allocator.Allocate<LWDirectX11_1Texture>(Context, TextureState, PackType, MakeMipmaps ? LWImage::MipmapCount(Size) : MipmapCnt, LWVector3i(Size, Layers, 0), LWTexture::Texture1DArray);

}

LWTexture *LWVideoDriver_DirectX11_1::CreateTexture2DArray(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Layers, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	//PackTypes:                    RGBA8,                      RGBA8U,                     RGBA16,                         RGBA16U,                        RGBA32,                        RGBA32U,                       RGBA32F,                        RG8,                    RG8U,                   RG16,                     RG16U,                    RG32,                    RG32U,                   RG32F,                    R8,                   R8U,                  R16,                   R16U,                  R32,                  R32U,                 R32F,                  Depth16,               Depth24,                          Depth32,                  Depth24Stencil8,                   DXT1                   DXT2                   DXT3                   DXT4                   DXT5                   DXT6                   DXT7  
	const DXGI_FORMAT Formats[] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_SNORM, DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R16G16_SNORM, DXGI_FORMAT_R16G16_UNORM, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R16_SNORM, DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R16_TYPELESS, DXGI_FORMAT_R24G8_TYPELESS,        DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R24G8_TYPELESS,        DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC6H_SF16, DXGI_FORMAT_BC7_UNORM };
	const DXGI_FORMAT VFormats[] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_SNORM, DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R16G16_SNORM, DXGI_FORMAT_R16G16_UNORM, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R16_SNORM, DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R16_UNORM,    DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_R32_FLOAT,    DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC6H_SF16, DXGI_FORMAT_BC7_UNORM };
	LWDirectX11_1TextureContext Context;
	auto CheckResult = [&Context](HRESULT Res, const char *FuncName)->bool {
		if (SUCCEEDED(Res)) return true;
		std::cout << "CreateTexture2DArray Error '" << FuncName << "': " << std::hex << Res << std::dec << std::endl;
		if (Context.m_View) Context.m_View->Release();
		if (Context.m_Texture) Context.m_Texture->Release();
		return false;
	};

	bool MakeMipmaps = (TextureState&LWTexture::MakeMipmaps);
	bool Compressed = LWImage::CompressedType(PackType);
	bool DepthTex = LWImage::DepthType(PackType);
	bool RenderTarget = (TextureState&(LWTexture::RenderTarget|LWTexture::RenderBuffer));
	//TextureState = (TextureState & ~LWTexture::MakeMipmaps);

	uint32_t MipLevels = MakeMipmaps ? 0u : (MipmapCnt + 1u);
	uint32_t BindFlags = D3D11_BIND_SHADER_RESOURCE | ((DepthTex || Compressed) ? 0 : D3D11_BIND_UNORDERED_ACCESS) | ((RenderTarget || MakeMipmaps) ? (DepthTex ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_RENDER_TARGET) : 0u);

	D3D11_TEXTURE2D_DESC Desc = { (uint32_t)Size.x, (uint32_t)Size.y,
		MipLevels, Layers, Formats[PackType], { 1u, 0u},
		D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | BindFlags,
		0u,
		MakeMipmaps ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0u };


	D3D11_SHADER_RESOURCE_VIEW_DESC ViewDesc = { VFormats[PackType], D3D11_SRV_DIMENSION_TEXTURE2DARRAY};
	ViewDesc.Texture2DArray = { 0u, (uint32_t)-1, 0u, Layers };

	if (!CheckResult(m_Context.m_DXDevice->CreateTexture2D(&Desc, nullptr, (ID3D11Texture2D**)&Context.m_Texture), "CreateTexture2D")) return nullptr;
	if (!CheckResult(m_Context.m_DXDevice->CreateShaderResourceView(Context.m_Texture, &ViewDesc, &Context.m_View), "CreateShaderResourceView")) return nullptr;
	
	if (Texels) {
		uint32_t Mips = MakeMipmaps ? 0 : MipmapCnt;
		uint32_t t = 0;
		for (uint32_t l = 0; l < Layers; l++) {
			for (uint32_t i = 0; i <= Mips; i++, t++) {
				LWVector2i S = LWImage::MipmapSize2D(Size, i);
				m_Context.m_DXDeviceContext->UpdateSubresource(Context.m_Texture, t, nullptr, Texels ? Texels[t] : nullptr, LWImage::GetStride(Size.x, PackType), 0);
			}
		}
	}
	//if (MakeMipmaps) m_Context.m_DXDeviceContext->GenerateMips(Context.m_View);
	return Allocator.Allocate<LWDirectX11_1Texture>(Context, TextureState, PackType, MakeMipmaps ? LWImage::MipmapCount(Size) : MipmapCnt, LWVector3i(Size, Layers), LWTexture::Texture2DArray);
}

LWTexture *LWVideoDriver_DirectX11_1::CreateTextureCubeArray(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Layers, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	//PackTypes:                   RGBA8,                      RGBA8U,                     RGBA16,                         RGBA16U,                        RGBA32,                        RGBA32U,                       RGBA32F,                        RG8,                    RG8U,                   RG16,                     RG16U,                    RG32,                    RG32U,                   RG32F,                    R8,                   R8U,                  R16,                   R16U,                  R32,                  R32U,                 R32F,                  Depth16,               Depth24,                       Depth32,               Depth24Stencil8,          DXT1                   DXT2                   DXT3                   DXT4                   DXT5                   DXT6                   DXT7  
	const DXGI_FORMAT Formats[] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_SNORM, DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R16G16_SNORM, DXGI_FORMAT_R16G16_UNORM, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R16_SNORM, DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R16_TYPELESS, DXGI_FORMAT_R24G8_TYPELESS,        DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R24G8_TYPELESS,        DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC6H_SF16, DXGI_FORMAT_BC7_UNORM };
	const DXGI_FORMAT VFormats[] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_SNORM, DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R16G16_SNORM, DXGI_FORMAT_R16G16_UNORM, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R16_SNORM, DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R16_UNORM,    DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_R32_FLOAT,    DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC6H_SF16, DXGI_FORMAT_BC7_UNORM };
	LWDirectX11_1TextureContext Context;
	auto CheckResult = [&Context](HRESULT Res, const char *FuncName)->bool {
		if (SUCCEEDED(Res)) return true;
		std::cout << "CreateTextureCubeArray Error '" << FuncName << "': " << std::hex << Res << std::dec << std::endl;
		if (Context.m_View) Context.m_View->Release();
		if (Context.m_Texture) Context.m_Texture->Release();
		return false;
	};
	bool MakeMipmaps = (TextureState&LWTexture::MakeMipmaps);
	bool Compressed = LWImage::CompressedType(PackType);
	bool DepthTex = LWImage::DepthType(PackType);
	bool RenderTarget = (TextureState&(LWTexture::RenderTarget | LWTexture::RenderBuffer));
	//TextureState = (TextureState & ~LWTexture::MakeMipmaps);

	uint32_t MipLevels = MakeMipmaps ? 0u : (MipmapCnt + 1u);
	uint32_t BindFlags = D3D11_BIND_SHADER_RESOURCE | ((DepthTex || Compressed) ? 0 : D3D11_BIND_UNORDERED_ACCESS) | ((RenderTarget || MakeMipmaps) ? (DepthTex ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_RENDER_TARGET) : 0u);

	D3D11_TEXTURE2D_DESC Desc = { (uint32_t)Size.x, (uint32_t)Size.y,
		MipLevels, 6u*Layers, Formats[PackType],{ 1u, 0u },
		D3D11_USAGE_DEFAULT, BindFlags,
		0u,	D3D11_RESOURCE_MISC_TEXTURECUBE | (MakeMipmaps ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0u) };
	D3D11_SHADER_RESOURCE_VIEW_DESC ViewDesc = { VFormats[PackType], D3D11_SRV_DIMENSION_TEXTURECUBEARRAY };
	ViewDesc.TextureCubeArray = { 0u, (uint32_t)-1, 0u, Layers };

	if (!CheckResult(m_Context.m_DXDevice->CreateTexture2D(&Desc, nullptr, (ID3D11Texture2D**)&Context.m_Texture), "CreateTexture2D")) return nullptr;
	if (!CheckResult(m_Context.m_DXDevice->CreateShaderResourceView(Context.m_Texture, &ViewDesc, &Context.m_View), "CreateShaderResourceView")) return nullptr;

	if (Texels) {
		uint32_t Mips = MakeMipmaps ? 0 : MipmapCnt;
		uint32_t t = 0;
		for(uint32_t l=0;l<Layers*6;l++){
			for (uint32_t i = 0; i <= Mips; i++, t++) {
				LWVector2i S = LWImage::MipmapSize2D(Size, i);
				m_Context.m_DXDeviceContext->UpdateSubresource(Context.m_Texture, t, nullptr, Texels ? Texels[t] : nullptr, LWImage::GetStride(S.x, PackType), 0);
			}
		}
	}
	//if (MakeMipmaps) m_Context.m_DXDeviceContext->GenerateMips(Context.m_View);
	return Allocator.Allocate<LWDirectX11_1Texture>(Context, TextureState, PackType, MakeMipmaps ? LWImage::MipmapCount(Size) : MipmapCnt, LWVector3i(Size, Layers), LWTexture::TextureCubeMapArray);

}


LWTexture *LWVideoDriver_DirectX11_1::CreateTexture2DMSArray(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Samples, uint32_t Layers, LWAllocator &Allocator) {
	//PackTypes:                   RGBA8,                      RGBA8U,                     RGBA16,                         RGBA16U,                        RGBA32,                        RGBA32U,                       RGBA32F,                        RG8,                    RG8U,                   RG16,                     RG16U,                    RG32,                    RG32U,                   RG32F,                    R8,                   R8U,                  R16,                   R16U,                  R32,                  R32U,                 R32F,                  Depth16,               Depth24,                          Depth32,                  Depth24Stencil8,                   DXT1                   DXT2                   DXT3                   DXT4                   DXT5                   DXT6                   DXT7  
	const DXGI_FORMAT Formats[] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_SNORM, DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R16G16_SNORM, DXGI_FORMAT_R16G16_UNORM, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R16_SNORM, DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R16_TYPELESS, DXGI_FORMAT_R24G8_TYPELESS,        DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R24G8_TYPELESS,        DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC6H_SF16, DXGI_FORMAT_BC7_UNORM };
	const DXGI_FORMAT VFormats[] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_SNORM, DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R16G16_SNORM, DXGI_FORMAT_R16G16_UNORM, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R16_SNORM, DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R16_UNORM,    DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_R32_FLOAT,    DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC6H_SF16, DXGI_FORMAT_BC7_UNORM };
	LWDirectX11_1TextureContext Context;
	auto CheckResult = [&Context](HRESULT Res, const char *FuncName)->bool {
		if (SUCCEEDED(Res)) return true;
		std::cout << "CreateTexture2DMSArray Error '" << FuncName << "': " << std::hex << Res << std::dec << std::endl;
		if (Context.m_View) Context.m_View->Release();
		if (Context.m_Texture) Context.m_Texture->Release();
		return false;
	};
	TextureState = (TextureState & ~LWTexture::MakeMipmaps); //Can't have mipmaps.
	bool Compressed = LWImage::CompressedType(PackType);
	bool DepthTex = LWImage::DepthType(PackType);
	bool RenderTarget = (TextureState & (LWTexture::RenderTarget | LWTexture::RenderBuffer));
	uint32_t BindFlags = D3D11_BIND_SHADER_RESOURCE | ((RenderTarget) ? (DepthTex ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_RENDER_TARGET) : 0u);
	D3D11_TEXTURE2D_DESC Desc = { (uint32_t)Size.x, (uint32_t)Size.y, 1u, Layers, Formats[PackType], {Samples, 0u}, D3D11_USAGE_DEFAULT, BindFlags, 0u, 0u };
	D3D11_SHADER_RESOURCE_VIEW_DESC ViewDesc = { VFormats[PackType], D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY, {0u, Layers} };
	if (!CheckResult(m_Context.m_DXDevice->CreateTexture2D(&Desc, nullptr, (ID3D11Texture2D**)&Context.m_Texture), "CreateTexture2D")) return nullptr;
	if (!CheckResult(m_Context.m_DXDevice->CreateShaderResourceView(Context.m_Texture, &ViewDesc, &Context.m_View), "CreateShaderResourceView")) return nullptr;
	return Allocator.Allocate<LWDirectX11_1Texture>(Context, TextureState, PackType, Samples, LWVector3i(Size, Layers), LWTexture::Texture2DMSArray);
}

LWVideoBuffer *LWVideoDriver_DirectX11_1::CreateVideoBuffer(uint32_t Type, uint32_t UsageFlag, uint32_t TypeSize, uint32_t Length, LWAllocator &Allocator, const uint8_t *Buffer) {
	uint32_t DBinds[] = { D3D11_BIND_VERTEX_BUFFER, D3D11_BIND_CONSTANT_BUFFER, D3D11_BIND_INDEX_BUFFER, D3D11_BIND_INDEX_BUFFER, D3D11_BIND_SHADER_RESOURCE };
	uint32_t MiscFlags[] = { 0u, 0u, 0u, 0u, D3D11_RESOURCE_MISC_BUFFER_STRUCTURED };
	uint32_t VideoID = 0;
	uint32_t UsageID = (UsageFlag&LWVideoBuffer::UsageFlag);
	uint32_t RawLength = TypeSize * Length;
	D3D11_BUFFER_DESC Desc;
	if (UsageID == LWVideoBuffer::PersistentMapped) return nullptr; //Not implemented.
	else if (UsageID == LWVideoBuffer::Static) {
		Desc = { RawLength, D3D11_USAGE_IMMUTABLE, DBinds[Type], 0u, MiscFlags[Type], TypeSize };
	} else if (UsageID == LWVideoBuffer::WriteDiscardable) {
		Desc = { RawLength, D3D11_USAGE_DYNAMIC, DBinds[Type], D3D11_CPU_ACCESS_WRITE, MiscFlags[Type], TypeSize };
	} else if (UsageID == LWVideoBuffer::WriteNoOverlap) {
		Desc = { RawLength, D3D11_USAGE_DYNAMIC, DBinds[Type], D3D11_CPU_ACCESS_WRITE, MiscFlags[Type], TypeSize };
	} else if (UsageID == LWVideoBuffer::Readable) {
		Desc = { RawLength, D3D11_USAGE_IMMUTABLE, DBinds[Type], D3D11_CPU_ACCESS_READ, MiscFlags[Type], TypeSize };
	} else if (UsageID == LWVideoBuffer::GPUResource) {
		Desc = { RawLength, D3D11_USAGE_DEFAULT, DBinds[Type] | D3D11_BIND_UNORDERED_ACCESS, 0u, MiscFlags[Type], TypeSize };
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC SDesc = { DXGI_FORMAT_UNKNOWN, D3D11_SRV_DIMENSION_BUFFEREX, {0u, Length} };
	D3D11_UNORDERED_ACCESS_VIEW_DESC UDesc = { DXGI_FORMAT_UNKNOWN, D3D11_UAV_DIMENSION_BUFFER, {0u, Length} };
	D3D11_SUBRESOURCE_DATA Data = { Buffer, 0, 0 };

	ID3D11Buffer *B = nullptr;
	HRESULT Res = m_Context.m_DXDevice->CreateBuffer(&Desc, Buffer ? &Data : nullptr, &B);

	LWDirectX11_1BufferContext Con = { B, nullptr, nullptr };
	if (FAILED(Res)) {
		std::cout << "Failed to create video buffer: " << Res << std::endl;
		return nullptr;
	}
	if ((Desc.BindFlags&D3D11_BIND_SHADER_RESOURCE) != 0) {
		Res = m_Context.m_DXDevice->CreateShaderResourceView(B, &SDesc, &Con.m_SView);
		if (FAILED(Res)) {
			std::cout << "Failed to create SView: " << Res << std::endl;
			B->Release();
			return nullptr;
		}
	}
	if ((Desc.BindFlags&D3D11_BIND_UNORDERED_ACCESS) != 0) {
		Res = m_Context.m_DXDevice->CreateUnorderedAccessView(B, &UDesc, &Con.m_UView);
		if (FAILED(Res)) {
			std::cout << "Failed to create UView: " << Res << std::endl;
			if (Con.m_SView) Con.m_SView->Release();
			B->Release();
			return nullptr;
		}
	}
	return Allocator.Allocate<LWDirectX11_1Buffer>(Buffer, &Allocator, TypeSize, Length, UsageFlag | Type, Con);
}

LWFrameBuffer *LWVideoDriver_DirectX11_1::CreateFrameBuffer(const LWVector2i &Size, LWAllocator &Allocator) {
	return Allocator.Allocate<LWDirectX11_1FrameBuffer>(Size);
}

bool LWVideoDriver_DirectX11_1::UpdateTexture(LWTexture *Texture) {
	D3D11_FILTER DXFilters[] = { D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT, D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR, D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT, D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT, D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR, D3D11_FILTER_MIN_MAG_MIP_LINEAR };
	D3D11_FILTER DXCompareFuncs[] = { D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT, D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT, D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT, D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT, D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR, D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR, D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT, D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT, D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT,	D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT, D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR, D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR };
	D3D11_COMPARISON_FUNC DXCompareModes[] = { D3D11_COMPARISON_NEVER, D3D11_COMPARISON_ALWAYS, D3D11_COMPARISON_LESS, D3D11_COMPARISON_EQUAL, D3D11_COMPARISON_LESS_EQUAL, D3D11_COMPARISON_GREATER, D3D11_COMPARISON_GREATER_EQUAL, D3D11_COMPARISON_NOT_EQUAL };
	D3D11_TEXTURE_ADDRESS_MODE DXWrapModes[] = { D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_BORDER, D3D11_TEXTURE_ADDRESS_MIRROR, D3D11_TEXTURE_ADDRESS_WRAP };
	if (!Texture->isDirty()) return false;
	uint32_t TextureState = Texture->GetTextureState();
	uint32_t SamplerHash = (TextureState & (LWTexture::MinFilterFlag | LWTexture::MagFilterFlag | LWTexture::WrapSFilterFlag | LWTexture::WrapTFilterFlag | LWTexture::WrapRFilterFlag | LWTexture::CompareFuncFlag | LWTexture::CompareModeFlag | LWTexture::DepthReadFlag));
	bool MakeMips = (TextureState & LWTexture::MakeMipmaps)!=0;

	auto Iter = m_Context.m_SamplerMap.find(SamplerHash);
	ID3D11SamplerState *SampleState = nullptr;
	if (Iter != m_Context.m_SamplerMap.end()) SampleState = Iter->second;
	else {
		uint32_t Filter = (TextureState&LWTexture::MinFilterFlag) | (TextureState&LWTexture::MagFilterFlag);
		uint32_t WrapS = (TextureState&LWTexture::WrapSFilterFlag) >> LWTexture::WrapSFilterBitOffset;
		uint32_t WrapT = (TextureState&LWTexture::WrapTFilterFlag) >> LWTexture::WrapTFilterBitOffset;
		uint32_t WrapR = (TextureState&LWTexture::WrapRFilterFlag) >> LWTexture::WrapRFilterBitOffset;
		uint32_t CFunc = (TextureState&LWTexture::CompareFuncFlag) >> LWTexture::CompareFuncBitOffset;
		uint32_t CMode = (TextureState&LWTexture::CompareModeFlag) >> LWTexture::CompareModeBitOffset;

		D3D11_SAMPLER_DESC SamplerDesc;
		SamplerDesc.Filter = CMode ? DXCompareFuncs[Filter] : DXFilters[Filter];
		SamplerDesc.AddressU = DXWrapModes[WrapS];
		SamplerDesc.AddressV = DXWrapModes[WrapT];
		SamplerDesc.AddressW = DXWrapModes[WrapR];
		SamplerDesc.MinLOD = -FLT_MAX;
		SamplerDesc.MaxLOD = FLT_MAX;
		SamplerDesc.MipLODBias = 0.0f;
		SamplerDesc.MaxAnisotropy = 1;
		SamplerDesc.ComparisonFunc = DXCompareModes[CFunc];
		SamplerDesc.BorderColor[0] = SamplerDesc.BorderColor[1] = SamplerDesc.BorderColor[2] = SamplerDesc.BorderColor[3] = 1.0f;
		if (FAILED(m_Context.m_DXDevice->CreateSamplerState(&SamplerDesc, &SampleState))) {
			Texture->ClearDirty();
			return true;
		}
		std::pair<uint32_t, ID3D11SamplerState*> p = { SamplerHash, SampleState };
		m_Context.m_SamplerMap.insert(p);
	}
	LWDirectX11_1Texture *Tex = (LWDirectX11_1Texture*)Texture;
	LWDirectX11_1TextureContext &TexCon = Tex->GetContext();
	if (MakeMips) m_Context.m_DXDeviceContext->GenerateMips(TexCon.m_View);
	TexCon.m_Sampler = SampleState;
	Texture->ClearDirty();
	return true;
}

bool LWVideoDriver_DirectX11_1::UpdateTexture1D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, uint32_t Position, uint32_t Size) {
	if (!UpdateTexture(Texture)) return false;
	if (!Texels) return true;
	uint32_t PackType = Texture->GetPackType();
	LWDirectX11_1TextureContext &Con = ((LWDirectX11_1Texture*)Texture)->GetContext();
	D3D11_BOX B = { Position, 0u, 0u, Position + Size, 1u, 1u };
	m_Context.m_DXDeviceContext->UpdateSubresource(Con.m_Texture, MipmapLevel, &B, Texels, LWImage::GetStride(Size, PackType), 0);
	return true;
}

bool LWVideoDriver_DirectX11_1::UpdateTexture2D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, const LWVector2i &Position, const LWVector2i &Size) {
	if (!UpdateTexture(Texture)) return false;
	if (!Texels) return true;
	uint32_t PackType = Texture->GetPackType();
	LWDirectX11_1TextureContext &Con = ((LWDirectX11_1Texture*)Texture)->GetContext();
	D3D11_BOX B = { (uint32_t)Position.x, (uint32_t)Position.y, 0u, (uint32_t)(Position.x + Size.x), (uint32_t)(Position.y + Size.y), 1u };
	m_Context.m_DXDeviceContext->UpdateSubresource(Con.m_Texture, MipmapLevel, &B, Texels, LWImage::GetStride(Size.x, PackType), 0);
	return true;
}

bool LWVideoDriver_DirectX11_1::UpdateTexture3D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, const LWVector3i &Position, const LWVector3i &Size) {
	if (!UpdateTexture(Texture)) return false;
	if (!Texels) return true;
	uint32_t PackType = Texture->GetPackType();
	LWDirectX11_1TextureContext &Con = ((LWDirectX11_1Texture*)Texture)->GetContext();
	D3D11_BOX B = { (uint32_t)Position.x, (uint32_t)Position.y, (uint32_t)Position.z, (uint32_t)(Position.x + Size.x), (uint32_t)(Position.y + Size.y), (uint32_t)(Position.z + Size.z) };
	m_Context.m_DXDeviceContext->UpdateSubresource(Con.m_Texture, MipmapLevel, &B, Texels, LWImage::GetStride(Size.x, PackType), LWImage::GetLength2D(LWVector2i(Size.x, Size.y), PackType));
	return true;
}

bool LWVideoDriver_DirectX11_1::UpdateTextureCubeMap(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Face, void *Texels, const LWVector2i &Position, const LWVector2i &Size) {
	if (!UpdateTexture(Texture)) return false;
	if (!Texels) return true;
	uint32_t PackType = Texture->GetPackType();
	LWDirectX11_1TextureContext &Con = ((LWDirectX11_1Texture*)Texture)->GetContext();
	D3D11_BOX B = { (uint32_t)Position.x,  (uint32_t)Position.y, 0u,  (uint32_t)(Position.x + Size.x),  (uint32_t)(Position.y + Size.y), 0u };
	uint32_t MipCnt = Texture->GetMipmapCount();
	m_Context.m_DXDeviceContext->UpdateSubresource(Con.m_Texture, MipCnt*Face + MipmapLevel, &B, Texels, LWImage::GetStride(Size.x, PackType), 0);
	return true;
}

bool LWVideoDriver_DirectX11_1::UpdateTexture1DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, void *Texels, uint32_t Position, uint32_t Size) {
	if (!UpdateTexture(Texture)) return false;
	if (!Texels) return true;
	uint32_t PackType = Texture->GetPackType();
	LWDirectX11_1TextureContext &Con = ((LWDirectX11_1Texture*)Texture)->GetContext();
	D3D11_BOX B = { (uint32_t)Position, 0u, 0u, (uint32_t)(Position + Size), 1u, 1u };
	uint32_t MipCnt = Texture->GetMipmapCount();
	m_Context.m_DXDeviceContext->UpdateSubresource(Con.m_Texture, MipCnt*Layer + MipmapLevel, &B, Texels, LWImage::GetStride(Size, PackType), 0);
	return true;
}

bool LWVideoDriver_DirectX11_1::UpdateTexture2DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, void *Texels, const LWVector2i &Position, const LWVector2i &Size) {
	if (!UpdateTexture(Texture)) return false;
	if (!Texels) return true;
	uint32_t PackType = Texture->GetPackType();
	LWDirectX11_1TextureContext &Con = ((LWDirectX11_1Texture*)Texture)->GetContext();
	D3D11_BOX B = { (uint32_t)Position.x, (uint32_t)Position.y, 0u, (uint32_t)(Position.x + Size.x), (uint32_t)(Position.y + Size.y), 1u };
	uint32_t MipCnt = Texture->GetMipmapCount();
	m_Context.m_DXDeviceContext->UpdateSubresource(Con.m_Texture, MipCnt*Layer + MipmapLevel, &B, Texels, LWImage::GetStride(Size.x, PackType), 0);
	return true;
}

bool LWVideoDriver_DirectX11_1::UpdateTextureCubeArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint32_t Face, void *Texels, const LWVector2i &Position, const LWVector2i &Size) {
	if (!UpdateTexture(Texture)) return false;
	if (!Texels) return true;
	uint32_t PackType = Texture->GetPackType();
	LWDirectX11_1TextureContext &Con = ((LWDirectX11_1Texture*)Texture)->GetContext();
	D3D11_BOX B = { (uint32_t)Position.x, (uint32_t)Position.y, 0u, (uint32_t)(Position.x + Size.x), (uint32_t)(Position.y + Size.y), 1u };
	uint32_t MipCnt = Texture->GetMipmapCount();
	m_Context.m_DXDeviceContext->UpdateSubresource(Con.m_Texture, MipCnt*(Layer * 6) + (MipCnt*Face) + MipmapLevel, &B, Texels, LWImage::GetStride(Size.x, PackType), 0);
	return true;
}

bool LWVideoDriver_DirectX11_1::DownloadTexture1D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver_DirectX11_1::DownloadTexture2D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver_DirectX11_1::DownloadTexture3D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver_DirectX11_1::DownloadTextureCubeMap(LWTexture *Texture, uint32_t Face, uint32_t MipmapLevel, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver_DirectX11_1::DownloadTexture1DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver_DirectX11_1::DownloadTexture2DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver_DirectX11_1::DownloadTextureCubeArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint32_t Face, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver_DirectX11_1::DownloadVideoBuffer(LWVideoBuffer *VBuffer, uint8_t *Buffer, uint32_t Offset, uint32_t Length) {
	return false;
}

bool LWVideoDriver_DirectX11_1::UpdateVideoBuffer(LWVideoBuffer *VideoBuffer, const uint8_t *Buffer, uint32_t Length) {
	LWDirectX11_1Buffer *VB = (LWDirectX11_1Buffer*)VideoBuffer;
	LWDirectX11_1BufferContext &Con = VB->GetContext();
	ID3D11Buffer *Buf = Con.m_Buffer;
	uint32_t UsageID = VideoBuffer->GetFlag()&LWVideoBuffer::UsageFlag;
	if (UsageID != LWVideoBuffer::WriteDiscardable && UsageID != LWVideoBuffer::WriteNoOverlap) return false;
	D3D11_MAP UpdateType = D3D11_MAP_WRITE_DISCARD;
	if (UsageID == LWVideoBuffer::WriteNoOverlap) UpdateType = D3D11_MAP_WRITE_NO_OVERWRITE;
	D3D11_MAPPED_SUBRESOURCE M;
	HRESULT Res = m_Context.m_DXDeviceContext->Map(Buf, 0, UpdateType, 0, &M);
	if (FAILED(Res)) {
		std::cout << "Error Map: " << Res << std::endl;
		return false;
	}
	std::copy(Buffer, Buffer + Length, (uint8_t*)M.pData);
	m_Context.m_DXDeviceContext->Unmap(Buf, 0);
	return true;
}

LWVideoDriver &LWVideoDriver_DirectX11_1::DestroyVideoBuffer(LWVideoBuffer *Buffer) {
	LWDirectX11_1Buffer *VB = (LWDirectX11_1Buffer*)Buffer;
	LWDirectX11_1BufferContext &Con = VB->GetContext();
	Con.m_Buffer->Release();
	if (Con.m_SView) Con.m_SView->Release();
	if (Con.m_UView) Con.m_UView->Release();
	LWAllocator::Destroy(VB);
	return *this;
}

LWVideoDriver &LWVideoDriver_DirectX11_1::DestroyPipeline(LWPipeline *Pipeline) {
	LWDirectX11_1Pipeline *P = (LWDirectX11_1Pipeline*)Pipeline;
	if (P->isInternalPipeline()) {
		LWDirectX11_1PipelineContext &PipelineContext = P->GetContext();
		if (PipelineContext.m_VertexShader) PipelineContext.m_VertexShader->Release();
		if (PipelineContext.m_GeometryShader) PipelineContext.m_GeometryShader->Release();
		if (PipelineContext.m_PixelShader) PipelineContext.m_PixelShader->Release();
		if (PipelineContext.m_ComputeShader) PipelineContext.m_ComputeShader->Release();
		if (PipelineContext.m_InputLayout) PipelineContext.m_InputLayout->Release();
	}
	LWAllocator::Destroy(P);
	return *this;
}

LWVideoDriver &LWVideoDriver_DirectX11_1::DestroyShader(LWShader *Shader) {
	LWDirectX11_1Shader *S = (LWDirectX11_1Shader*)Shader;
	LWDirectX11_1ShaderContext &ShaderContext = S->GetContext();
	uint32_t ShaderType = Shader->GetShaderType();

	if (ShaderType == LWShader::Vertex) ShaderContext.m_VertexShader->Release();
	else if (ShaderType == LWShader::Geometry) ShaderContext.m_GeometryShader->Release();
	else if (ShaderType == LWShader::Pixel) ShaderContext.m_PixelShader->Release();
	else if (ShaderType == LWShader::Compute) ShaderContext.m_ComputeShader->Release();
	if (ShaderContext.m_InputLayout) ShaderContext.m_InputLayout->Release();
	LWAllocator::Destroy(S);
	return *this;
}

LWVideoDriver &LWVideoDriver_DirectX11_1::DestroyTexture(LWTexture *Texture) {
	LWDirectX11_1Texture *Tex = (LWDirectX11_1Texture*)Texture;
	LWDirectX11_1TextureContext &Con = Tex->GetContext();

	for (auto &&Iter : Con.m_RenderTargetViewList) Iter.second->Release();
	for (auto &&Iter : Con.m_DepthStencilViewList) Iter.second->Release();
	for (auto &&Iter : Con.m_UnorderedViewList) Iter.second->Release();
	Con.m_View->Release();
	Con.m_Texture->Release();
	LWAllocator::Destroy(Tex);
	return *this;
}

LWVideoDriver &LWVideoDriver_DirectX11_1::DestroyFrameBuffer(LWFrameBuffer *FrameBuffer) {
	LWDirectX11_1FrameBuffer *F = (LWDirectX11_1FrameBuffer*)FrameBuffer;
	LWAllocator::Destroy(F);
	return *this;
}

bool LWVideoDriver_DirectX11_1::SetFrameBuffer(LWFrameBuffer *FrameBuffer, bool ChangeViewport) {
	if (!LWVideoDriver::SetFrameBuffer(FrameBuffer, ChangeViewport)) return false;
	ID3D11RenderTargetView *ColorViews[LWFrameBuffer::ColorCount];
	ID3D11DepthStencilView *DepthView = m_Context.m_BackBufferDepthStencilView;
	ColorViews[0] = m_Context.m_BackBuffer;
	std::fill(ColorViews + 1, ColorViews + LWFrameBuffer::ColorCount, nullptr);
	if (FrameBuffer) {
		for (uint32_t i = 0; i < LWFrameBuffer::ColorCount; i++) {
			auto &Slot = FrameBuffer->GetAttachment(LWFrameBuffer::Color0 + i);
			if (!Slot.m_Source) ColorViews[i] = nullptr;
			else {
				auto &Con = ((LWDirectX11_1Texture*)Slot.m_Source)->GetContext();
				ColorViews[i] = Con.GetRenderTargetView(Slot.m_Layer, Slot.m_Face, Slot.m_Mipmap, Slot.m_Source, m_Context);
			}
		}
		auto &DepthSlot = FrameBuffer->GetAttachment(LWFrameBuffer::Depth);
		if (!DepthSlot.m_Source) DepthView = nullptr;
		else {
			auto &DepthCon = ((LWDirectX11_1Texture*)DepthSlot.m_Source)->GetContext();
			DepthView = DepthCon.GetDepthStencilView(DepthSlot.m_Layer, DepthSlot.m_Face, DepthSlot.m_Mipmap, DepthSlot.m_Source, m_Context);
		}
	}
	m_Context.m_DXDeviceContext->OMSetRenderTargets(LWFrameBuffer::ColorCount, ColorViews, DepthView);
	return true;
}


bool LWVideoDriver_DirectX11_1::SetRasterState(uint64_t Flags, float Bias, float SlopedScaleBias){
	if (!LWVideoDriver::SetRasterState(Flags, Bias, SlopedScaleBias)) return false;
	uint32_t StateHash = MakeRasterStateHash(Flags, Bias, SlopedScaleBias);
	auto Iter = m_Context.m_RasterMap.find(StateHash);
	if (Iter == m_Context.m_RasterMap.end()) {
		const int32_t Depth24Scalar = 0xFFFFFF;
		D3D11_COMPARISON_FUNC CompFuncs[] = { D3D11_COMPARISON_ALWAYS, D3D11_COMPARISON_NEVER, D3D11_COMPARISON_LESS, D3D11_COMPARISON_GREATER, D3D11_COMPARISON_LESS_EQUAL, D3D11_COMPARISON_GREATER_EQUAL };
		D3D11_BLEND BlendFuncs[] = { D3D11_BLEND_ZERO, D3D11_BLEND_ONE, D3D11_BLEND_SRC_COLOR, D3D11_BLEND_DEST_COLOR, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_INV_SRC_COLOR, D3D11_BLEND_INV_DEST_COLOR, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_INV_DEST_ALPHA };
		D3D11_BLEND ABlendFuncs[] = { D3D11_BLEND_ZERO, D3D11_BLEND_ONE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_INV_DEST_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_INV_DEST_ALPHA };
		D3D11_STENCIL_OP StencilOps[] = { D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_ZERO, D3D11_STENCIL_OP_REPLACE, D3D11_STENCIL_OP_INCR_SAT, D3D11_STENCIL_OP_DECR_SAT, D3D11_STENCIL_OP_INCR, D3D11_STENCIL_OP_DECR, D3D11_STENCIL_OP_INVERT };
		D3D11_FILL_MODE FillModes[] = { D3D11_FILL_SOLID, D3D11_FILL_WIREFRAME };
		D3D11_CULL_MODE CullModes[] = { D3D11_CULL_NONE, D3D11_CULL_FRONT, D3D11_CULL_BACK };

		uint64_t SourceBlend = (Flags&LWPipeline::BLEND_SRC_BITS) >> LWPipeline::BLEND_SRC_BITOFFSET;
		uint64_t DestBlend = (Flags&LWPipeline::BLEND_DST_BITS) >> LWPipeline::BLEND_DST_BITOFFSET;
		uint64_t FillMode = (Flags&LWPipeline::FILL_MODE_BITS) >> LWPipeline::FILL_MODE_BITOFFSET;
		uint64_t CullMode = (Flags&LWPipeline::CULL_BITS) >> LWPipeline::CULL_BITOFFSET;
		uint64_t DepthCompareFunc = (Flags&LWPipeline::DEPTH_COMPARE_BITS) >> LWPipeline::DEPTH_COMPARE_BITOFFSET;
		uint64_t StencilCompareFunc = (Flags&LWPipeline::STENCIL_COMPARE_BITS) >> LWPipeline::STENCIL_COMPARE_BITOFFSET;
		uint64_t SFailOp = (Flags&LWPipeline::STENCIL_OP_SFAIL_BITS) >> LWPipeline::STENCIL_OP_SFAIL_BITOFFSET;
		uint64_t DFailOp = (Flags&LWPipeline::STENCIL_OP_DFAIL_BITS) >> LWPipeline::STENCIL_OP_DFAIL_BITOFFSET;
		uint64_t PassOp = (Flags&LWPipeline::STENCIL_OP_PASS_BITS) >> LWPipeline::STENCIL_OP_PASS_BITOFFSET;
		uint64_t StencilReadMask = (Flags&LWPipeline::STENCIL_READMASK_BITS) >> LWPipeline::STENCIL_READMASK_BITOFFSET;
		uint64_t StencilWriteMask = (Flags&LWPipeline::STENCIL_WRITEMASK_BITS) >> LWPipeline::STENCIL_WRITEMASK_BITOFFSET;
		bool doDepthBias = (Flags&LWPipeline::DEPTH_BIAS);
		if (!doDepthBias) Bias = SlopedScaleBias = 0.0f;

		D3D11_RASTERIZER_DESC1 RastDesc = { FillModes[FillMode], CullModes[CullMode],
											true, (int32_t)(Depth24Scalar*Bias), 0.0f, SlopedScaleBias, true, false, true, false, 0 };
		D3D11_DEPTH_STENCIL_DESC DepthDesc = { (Flags&(LWPipeline::DEPTH_TEST)) != 0,
												((Flags&(LWPipeline::No_Depth)) == 0 ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO),
												CompFuncs[DepthCompareFunc], (Flags&LWPipeline::STENCIL_TEST) != 0,
												(uint8_t)StencilReadMask,
												(uint8_t)StencilWriteMask,
												{StencilOps[SFailOp], StencilOps[DFailOp], StencilOps[PassOp], CompFuncs[StencilCompareFunc] },
												{StencilOps[SFailOp], StencilOps[DFailOp], StencilOps[PassOp], CompFuncs[StencilCompareFunc] } };

		D3D11_BLEND_DESC BlendDesc;
		BlendDesc.AlphaToCoverageEnable = false;
		BlendDesc.IndependentBlendEnable = false;
		BlendDesc.RenderTarget[0].BlendEnable = (Flags&(LWPipeline::BLENDING));
		BlendDesc.RenderTarget[0].SrcBlend = BlendFuncs[SourceBlend];
		BlendDesc.RenderTarget[0].DestBlend = BlendFuncs[DestBlend];
		BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].SrcBlendAlpha = ABlendFuncs[SourceBlend];
		BlendDesc.RenderTarget[0].DestBlendAlpha = ABlendFuncs[DestBlend];
		BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		BlendDesc.RenderTarget[0].RenderTargetWriteMask =
			((Flags&LWPipeline::No_ColorR) == 0 ? D3D11_COLOR_WRITE_ENABLE_RED : 0) |
			((Flags&LWPipeline::No_ColorG) == 0 ? D3D11_COLOR_WRITE_ENABLE_GREEN : 0) |
			((Flags&LWPipeline::No_ColorB) == 0 ? D3D11_COLOR_WRITE_ENABLE_BLUE : 0) |
			((Flags&LWPipeline::No_ColorA) == 0 ? D3D11_COLOR_WRITE_ENABLE_ALPHA : 0);
		for (uint32_t i = 1; i < 8; i++) BlendDesc.RenderTarget[i] = BlendDesc.RenderTarget[0];
		LWDirectX11_1RasterContext RContext;
		if (FAILED(m_Context.m_DXDevice->CreateRasterizerState1(&RastDesc, &RContext.m_RasterState))) return nullptr;
		if (FAILED(m_Context.m_DXDevice->CreateDepthStencilState(&DepthDesc, &RContext.m_DepthStencilState))) {
			RContext.m_RasterState->Release();
			return false;
		}
		if (FAILED(m_Context.m_DXDevice->CreateBlendState(&BlendDesc, &RContext.m_BlendState))) {
			RContext.m_RasterState->Release();
			RContext.m_DepthStencilState->Release();
			return false;
		}
		auto Res = m_Context.m_RasterMap.emplace(StateHash, RContext);
		if (!Res.second) {
			RContext.m_RasterState->Release();
			RContext.m_DepthStencilState->Release();
			RContext.m_BlendState->Release();
			return false;
		}
		Iter = Res.first;
	}
	uint64_t StencilValue = (Flags&LWPipeline::STENCIL_REF_VALUE_BITS) >> LWPipeline::STENCIL_REF_VALUE_BITOFFSET;
	LWDirectX11_1RasterContext &RasterContext = Iter->second;
	m_Context.m_DXDeviceContext->RSSetState(RasterContext.m_RasterState);
	m_Context.m_DXDeviceContext->OMSetBlendState(RasterContext.m_BlendState, nullptr, 0xFFFFFFFF);
	m_Context.m_DXDeviceContext->OMSetDepthStencilState(RasterContext.m_DepthStencilState, (uint32_t)StencilValue);
	return true;
}


bool LWVideoDriver_DirectX11_1::SetPipeline(LWPipeline *Pipeline, LWVideoBuffer *VertexBuffer, LWVideoBuffer *IndiceBuffer, uint32_t VerticeStride, uint32_t Offset) {
	bool Update = LWVideoDriver::SetPipeline(Pipeline, VertexBuffer, IndiceBuffer, VerticeStride, Offset);
	ViewArray<ID3D11ShaderResourceView> VertexResourceViews;
	ViewArray<ID3D11ShaderResourceView> GeomResourceViews;
	ViewArray<ID3D11ShaderResourceView> PixelResourceViews;
	ViewArray<ID3D11ShaderResourceView> ComputeResourceViews;
	ViewArray<ID3D11SamplerState> VertexSamplerViews;
	ViewArray<ID3D11SamplerState> GeomSamplerViews;
	ViewArray<ID3D11SamplerState> PixelSamplerViews;
	ViewArray<ID3D11SamplerState> ComputeSamplerViews;
	ViewArray<ID3D11Buffer> VertexConstantViews;
	ViewArray<ID3D11Buffer> GeomConstantViews;
	ViewArray<ID3D11Buffer> PixelConstantViews;
	ViewArray<ID3D11Buffer> ComputeConstantViews;
	ViewArray<ID3D11UnorderedAccessView> ComputeUAVViews;
	LWDirectX11_1Pipeline *P = (LWDirectX11_1Pipeline*)Pipeline;
	LWDirectX11_1PipelineContext &PipeContext = P->GetContext();

	uint32_t BlockCnt = Pipeline->GetBlockCount();
	uint32_t ResourceCnt = Pipeline->GetResourceCount();
	uint32_t ComputeUAVBound = m_Context.m_BoundComputeUnorderedCount;

	for (uint32_t i = 0; i < BlockCnt; i++) {
		LWShaderResource &Block = Pipeline->GetBlock(i);
		auto *VB = (LWDirectX11_1Buffer*)Block.m_Resource;
		ID3D11Buffer *Buf = nullptr;
		uint32_t BufOffset = Block.m_Offset*16;
		uint32_t BufLen = Block.GetLength() / 16;
		BufLen = (BufLen % 16 == 0 ? BufLen : (BufLen + (16 - BufLen % 16)));
		if (VB) {
			Buf = VB->GetContext().m_Buffer;
			LWVideoDriver::UpdateVideoBuffer(VB);
		}
		ViewArray<ID3D11Buffer>::PushLists(Buf, BufOffset, BufLen, Block.m_Flag, VertexConstantViews, GeomConstantViews, PixelConstantViews, ComputeConstantViews);
	}
	for (uint32_t i = 0; i < ResourceCnt; i++) {
		LWShaderResource &Resrc = Pipeline->GetResource(i);
		auto *T = (LWDirectX11_1Texture*)Resrc.m_Resource;
		auto *B = (LWDirectX11_1Buffer*)Resrc.m_Resource;
		uint32_t TypeID = Resrc.GetTypeID();
		ID3D11ShaderResourceView *SView = nullptr;
		ID3D11UnorderedAccessView *UView = nullptr;
		ID3D11SamplerState *Sampler = nullptr;
		if ((TypeID == LWPipeline::Texture || TypeID == LWPipeline::Image) && T) {
			Update = UpdateTexture(T) || Update;
			auto &TexCon = T->GetContext();
			SView = TexCon.m_View;
			UView = TexCon.GetUnorderedAccessView(0, 0, 0, T, m_Context);
			Sampler = TexCon.m_Sampler;
		} else if ((TypeID == LWPipeline::TextureBuffer || TypeID == LWPipeline::ImageBuffer) && B) {
			LWVideoDriver::UpdateVideoBuffer(B);
			auto &BufCon = B->GetContext();
			SView = BufCon.m_SView;
			UView = BufCon.m_UView;
		}
		if (TypeID == LWPipeline::Texture) {
			ViewArray<ID3D11SamplerState>::PushLists(Sampler, 0, 0, Resrc.m_Flag, VertexSamplerViews, GeomSamplerViews, PixelSamplerViews, ComputeSamplerViews);
			ViewArray<ID3D11ShaderResourceView>::PushLists(SView, 0, 0, Resrc.m_Flag, VertexResourceViews, GeomResourceViews, PixelResourceViews, ComputeResourceViews);
		} else if (TypeID == LWPipeline::TextureBuffer) {
			ViewArray<ID3D11ShaderResourceView>::PushLists(SView, 0, 0, Resrc.m_Flag, VertexResourceViews, GeomResourceViews, PixelResourceViews, ComputeResourceViews);
		} else {
			if (Resrc.m_Flag&LWShaderResource::ComputeStage) ComputeUAVViews.Push(UView, 0, 0);
			else ViewArray<ID3D11ShaderResourceView>::PushLists(SView, 0, 0, Resrc.m_Flag, VertexResourceViews, GeomResourceViews, PixelResourceViews, ComputeResourceViews);
		}
	}
	if (VertexBuffer) {
		LWVideoDriver::UpdateVideoBuffer(VertexBuffer);
		ID3D11Buffer *VBuffer = ((LWDirectX11_1Buffer*)VertexBuffer)->GetContext().m_Buffer;
		uint32_t o = 0;
		m_Context.m_DXDeviceContext->IASetVertexBuffers(0, 1, &VBuffer, &VerticeStride, &o);
	}
	if (IndiceBuffer) {
		LWVideoDriver::UpdateVideoBuffer(IndiceBuffer);
		ID3D11Buffer *IBuffer = ((LWDirectX11_1Buffer*)IndiceBuffer)->GetContext().m_Buffer;
		m_Context.m_DXDeviceContext->IASetIndexBuffer(IBuffer, IndiceBuffer->GetType() == LWVideoBuffer::Index16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
	}
	if (!Update) return false;
	m_Context.m_BoundComputeUnorderedCount = ComputeUAVViews.m_Count;
	ComputeUAVViews.Fill(ComputeUAVBound, nullptr, 0, 0);
	if (ComputeUAVViews.m_Count) m_Context.m_DXDeviceContext->CSSetUnorderedAccessViews(0, ComputeUAVViews.m_Count, ComputeUAVViews.m_Views, nullptr);
	
	m_Context.m_DXDeviceContext->VSSetConstantBuffers1(0, VertexConstantViews.m_Count, VertexConstantViews.m_Views, VertexConstantViews.m_First, VertexConstantViews.m_Length);
	m_Context.m_DXDeviceContext->GSSetConstantBuffers1(0, GeomConstantViews.m_Count, GeomConstantViews.m_Views, GeomConstantViews.m_First, GeomConstantViews.m_Length);
	m_Context.m_DXDeviceContext->PSSetConstantBuffers1(0, PixelConstantViews.m_Count, PixelConstantViews.m_Views, PixelConstantViews.m_First, PixelConstantViews.m_Length);
	m_Context.m_DXDeviceContext->CSSetConstantBuffers1(0, ComputeConstantViews.m_Count, ComputeConstantViews.m_Views, ComputeConstantViews.m_First, ComputeConstantViews.m_Length);

	m_Context.m_DXDeviceContext->VSSetShaderResources(0, VertexResourceViews.m_Count, VertexResourceViews.m_Views);
	m_Context.m_DXDeviceContext->VSSetSamplers(0, VertexSamplerViews.m_Count, VertexSamplerViews.m_Views);

	m_Context.m_DXDeviceContext->GSSetShaderResources(0, GeomResourceViews.m_Count, GeomResourceViews.m_Views);
	m_Context.m_DXDeviceContext->GSSetSamplers(0, GeomSamplerViews.m_Count, GeomSamplerViews.m_Views);

	m_Context.m_DXDeviceContext->PSSetShaderResources(0, PixelResourceViews.m_Count, PixelResourceViews.m_Views);
	m_Context.m_DXDeviceContext->PSSetSamplers(0, PixelSamplerViews.m_Count, PixelSamplerViews.m_Views);

	m_Context.m_DXDeviceContext->CSSetShaderResources(0, ComputeResourceViews.m_Count, ComputeResourceViews.m_Views);
	m_Context.m_DXDeviceContext->CSSetSamplers(0, ComputeSamplerViews.m_Count, ComputeSamplerViews.m_Views);

	m_Context.m_DXDeviceContext->VSSetShader(PipeContext.m_VertexShader, nullptr, 0);
	m_Context.m_DXDeviceContext->PSSetShader(PipeContext.m_PixelShader, nullptr, 0);
	m_Context.m_DXDeviceContext->GSSetShader(PipeContext.m_GeometryShader, nullptr, 0);
	m_Context.m_DXDeviceContext->CSSetShader(PipeContext.m_ComputeShader, nullptr, 0);
	m_Context.m_DXDeviceContext->IASetInputLayout(PipeContext.m_InputLayout);

	return true;
}

LWVideoDriver &LWVideoDriver_DirectX11_1::ClearColor(uint32_t Color) {
	SetFrameBuffer(m_ActiveFrameBuffer, false);
	LWVector4f Clr = LWUNPACK_COLORVEC4f(Color);
	ID3D11RenderTargetView *RViews[LWFrameBuffer::ColorCount];
	std::fill(RViews, RViews + LWFrameBuffer::ColorCount, nullptr);
	RViews[0] = m_Context.m_BackBuffer;
	if (m_ActiveFrameBuffer) {
		for (uint32_t i = 0; i < LWFrameBuffer::ColorCount; i++) {
			auto &Slot = m_ActiveFrameBuffer->GetAttachment(LWFrameBuffer::Color0 + i);
			if (!Slot.m_Source) RViews[i] = nullptr;
			else RViews[i] = ((LWDirectX11_1Texture*)Slot.m_Source)->GetContext().GetRenderTargetView(Slot.m_Layer, Slot.m_Face, Slot.m_Mipmap, Slot.m_Source, m_Context);
		}
	}
	for (uint32_t i = 0; i < LWFrameBuffer::ColorCount; i++) {
		if (RViews[i]) m_Context.m_DXDeviceContext->ClearRenderTargetView(RViews[i], &Clr.x);
	}
	return *this;
}

LWVideoDriver &LWVideoDriver_DirectX11_1::ClearColor(const LWVector4f &Color) {
	SetFrameBuffer(m_ActiveFrameBuffer, false);
	ID3D11RenderTargetView *RViews[LWFrameBuffer::ColorCount];
	std::fill(RViews, RViews + LWFrameBuffer::ColorCount, nullptr);
	RViews[0] = m_Context.m_BackBuffer;
	if (m_ActiveFrameBuffer) {
		for (uint32_t i = 0; i < LWFrameBuffer::ColorCount; i++) {
			auto &Slot = m_ActiveFrameBuffer->GetAttachment(LWFrameBuffer::Color0 + i);
			if (!Slot.m_Source) RViews[i] = nullptr;
			else RViews[i] = ((LWDirectX11_1Texture*)Slot.m_Source)->GetContext().GetRenderTargetView(Slot.m_Layer, Slot.m_Face, Slot.m_Mipmap, Slot.m_Source, m_Context);
		}
	}
	for (uint32_t i = 0; i < LWFrameBuffer::ColorCount; i++) {
		if (RViews[i]) m_Context.m_DXDeviceContext->ClearRenderTargetView(RViews[i], &Color.x);
	}
	return *this;
}


LWVideoDriver &LWVideoDriver_DirectX11_1::ClearDepth(float Depth) {
	SetFrameBuffer(m_ActiveFrameBuffer);
	ID3D11DepthStencilView *View = m_Context.m_BackBufferDepthStencilView;
	if (m_ActiveFrameBuffer) {
		auto &Slot = m_ActiveFrameBuffer->GetAttachment(LWFrameBuffer::Depth);
		if (!Slot.m_Source) View = nullptr;
		else View = ((LWDirectX11_1Texture*)Slot.m_Source)->GetContext().GetDepthStencilView(Slot.m_Layer, Slot.m_Face, Slot.m_Mipmap, Slot.m_Source, m_Context);
	}
	if(View) m_Context.m_DXDeviceContext->ClearDepthStencilView(View, D3D11_CLEAR_DEPTH, Depth, 0);
	return *this;
}

LWVideoDriver &LWVideoDriver_DirectX11_1::ClearStencil(uint8_t Stencil) {
	SetFrameBuffer(m_ActiveFrameBuffer);
	ID3D11DepthStencilView *View = m_Context.m_BackBufferDepthStencilView;
	if (m_ActiveFrameBuffer) {
		auto &Slot = m_ActiveFrameBuffer->GetAttachment(LWFrameBuffer::Depth);
		if (!Slot.m_Source) View = nullptr;
		else View = ((LWDirectX11_1Texture*)Slot.m_Source)->GetContext().GetDepthStencilView(Slot.m_Layer, Slot.m_Face, Slot.m_Mipmap, Slot.m_Source, m_Context);
	}
	if(View) m_Context.m_DXDeviceContext->ClearDepthStencilView(View, D3D11_CLEAR_STENCIL, 0.0f, Stencil);
	return *this;
}

LWVideoDriver &LWVideoDriver_DirectX11_1::DrawBuffer(LWPipeline *Pipeline, int32_t DrawMode, LWVideoBuffer *InputBlock, LWVideoBuffer *IndexBuffer, uint32_t Count, uint32_t VertexStride, uint32_t Offset) {
	//Points, LineStrip, Lines, TriangleStrip, Triangles
	const uint32_t DDrawModes[] = { D3D11_PRIMITIVE_TOPOLOGY_POINTLIST, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, D3D11_PRIMITIVE_TOPOLOGY_LINELIST, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST };
	SetFrameBuffer(m_ActiveFrameBuffer);
	SetPipeline(Pipeline, InputBlock, IndexBuffer, VertexStride, Offset);
	m_Context.m_DXDeviceContext->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)DDrawModes[DrawMode]);	
	if (IndexBuffer) {
		m_Context.m_DXDeviceContext->DrawIndexed(Count, Offset, 0);
	} else m_Context.m_DXDeviceContext->Draw(Count, Offset);
	return *this;
}

LWVideoDriver &LWVideoDriver_DirectX11_1::DrawInstancedBuffer(LWPipeline *Pipeline, int32_t DrawMode, LWVideoBuffer *InputBlock, LWVideoBuffer *IndexBuffer, uint32_t Count, uint32_t VertexStride, uint32_t InstanceCount, uint32_t Offset) {
	//Points, LineStrip, Lines, TriangleStrip, Triangles
	uint32_t DDrawModes[] = { D3D11_PRIMITIVE_TOPOLOGY_POINTLIST, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, D3D11_PRIMITIVE_TOPOLOGY_LINELIST, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST };
	
	SetFrameBuffer(m_ActiveFrameBuffer);
	SetPipeline(Pipeline, InputBlock, IndexBuffer, VertexStride, Offset);
	m_Context.m_DXDeviceContext->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY)DDrawModes[DrawMode]);
	if (IndexBuffer) {
		m_Context.m_DXDeviceContext->DrawIndexedInstanced(Count, InstanceCount, Offset,  0, 0);
	} else m_Context.m_DXDeviceContext->DrawInstanced(Count, InstanceCount, Offset, 0);
	return *this;
}

LWVideoDriver &LWVideoDriver_DirectX11_1::Dispatch(LWPipeline *Pipeline, const LWVector3i &GroupDimension) {
	SetFrameBuffer(m_ActiveFrameBuffer);
	SetPipeline(Pipeline, nullptr, nullptr, 0, 0);
	m_Context.m_DXDeviceContext->Dispatch(GroupDimension.x, GroupDimension.y, GroupDimension.z);
	return *this;
}

LWVideoDriver &LWVideoDriver_DirectX11_1::Present(uint32_t SwapInterval) {
	m_Context.m_DXSwapChain->Present(SwapInterval, 0);
	return *this;
}

LWDirectX11_1Context &LWVideoDriver_DirectX11_1::GetContext(void) {
	return m_Context;
}

LWVideoDriver_DirectX11_1::LWVideoDriver_DirectX11_1(LWWindow *Window, LWDirectX11_1Context &Context, uint32_t UniformBlockSize) : LWVideoDriver(Window, DirectX11_1, UniformBlockSize) {
	m_Context = Context;
}


uint32_t LWDirectX11_1TextureContext::MakeHash(uint32_t Layer, uint32_t Face, uint32_t MipmapLevel) {
	return (Layer | (MipmapLevel << 16) | (Face<<28));
}

ID3D11RenderTargetView *LWDirectX11_1TextureContext::GetRenderTargetView(uint32_t Layer, uint32_t Face, uint32_t MipmapLevel, LWTexture *Tex, LWDirectX11_1Context &DriverContext) {
	const DXGI_FORMAT Formats[] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_SNORM, DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R16G16_SNORM, DXGI_FORMAT_R16G16_UNORM, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R16_SNORM, DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R16_UNORM,    DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_R32_FLOAT,    DXGI_FORMAT_R24_UNORM_X8_TYPELESS, DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC6H_SF16, DXGI_FORMAT_BC7_UNORM };
	uint32_t Hash = MakeHash(Layer, Face, MipmapLevel);
	auto Iter = m_RenderTargetViewList.find(Hash);
	if (Iter != m_RenderTargetViewList.end()) return Iter->second;
	uint32_t PackType = Tex->GetPackType();
	uint32_t TexType = Tex->GetType();
	LWVector3i Size = Tex->Get3DSize();
	uint32_t Mipmaps = Tex->GetMipmapCount();
	D3D11_RENDER_TARGET_VIEW_DESC Desc;
	ID3D11RenderTargetView *View = nullptr;
	Desc.Format = Formats[Tex->GetPackType()];
	if (TexType == LWTexture::Texture1D) {
		Desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1D;
		Desc.Texture1D = { MipmapLevel };
	} else if (TexType == LWTexture::Texture2D) {
		Desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		Desc.Texture2D = { MipmapLevel };
	} else if (TexType == LWTexture::Texture3D) {
		Desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
		Desc.Texture3D = { MipmapLevel, 0, (uint32_t)-1 };
	} else if (TexType == LWTexture::TextureCubeMap) {
		Layer += Face;
		Desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		Desc.Texture2DArray = { MipmapLevel, Layer, 1 };
	} else if (TexType == LWTexture::Texture1DArray) {
		Desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE1DARRAY;
		Desc.Texture1DArray = { MipmapLevel, Layer, 1 };
	} else if (TexType == LWTexture::Texture2DArray) {
		Desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		Desc.Texture2DArray = { MipmapLevel, Layer, 1 };
	} else if (TexType == LWTexture::TextureCubeMapArray) {
		Layer = Layer * 6 + Face;
		Desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		Desc.Texture2DArray = { MipmapLevel, Layer, 1 };
	} else if (TexType == LWTexture::Texture2DMS) {
		Desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
	} else if (TexType == LWTexture::Texture2DMSArray) {
		Desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
		Desc.Texture2DMSArray = { Layer, 1 };
	}
	HRESULT Res = DriverContext.m_DXDevice->CreateRenderTargetView(m_Texture, &Desc, &View);
	if (FAILED(Res)) {
		std::cout << "Error CreateRenderTargetView: " << std::hex << Res << std::dec << std::endl;
		return nullptr;
	}
	auto ResE = m_RenderTargetViewList.emplace(Hash, View);
	if (!ResE.second) {
		std::cout << "Error RenderTargetViewList.emplace" << std::endl;
		View->Release();
		return nullptr;
	}
	return View;
}

ID3D11DepthStencilView *LWDirectX11_1TextureContext::GetDepthStencilView(uint32_t Layer, uint32_t Face, uint32_t MipmapLevel, LWTexture *Tex, LWDirectX11_1Context &DriverContext) {

	const DXGI_FORMAT Formats[] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_SNORM, DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R16G16_SNORM, DXGI_FORMAT_R16G16_UNORM, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R16_SNORM, DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_D16_UNORM,    DXGI_FORMAT_D24_UNORM_S8_UINT,     DXGI_FORMAT_D32_FLOAT,    DXGI_FORMAT_D24_UNORM_S8_UINT,     DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC6H_SF16, DXGI_FORMAT_BC7_UNORM };
	uint32_t Hash = MakeHash(Layer, Face, MipmapLevel);
	auto Iter = m_DepthStencilViewList.find(Hash);
	if (Iter != m_DepthStencilViewList.end()) return Iter->second;
	uint32_t PackType = Tex->GetPackType();
	uint32_t TexType = Tex->GetType();
	LWVector3i Size = Tex->Get3DSize();
	uint32_t Mipmaps = Tex->GetMipmapCount();
	D3D11_DEPTH_STENCIL_VIEW_DESC Desc;
	ID3D11DepthStencilView *View = nullptr;
	Desc.Format = Formats[Tex->GetPackType()];
	Desc.Flags = (Tex->GetTextureState()&LWTexture::RenderBuffer) == 0 ? 0 : D3D11_DSV_READ_ONLY_DEPTH | D3D11_DSV_READ_ONLY_STENCIL;
	if (TexType == LWTexture::Texture1D) {
		Desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1D;
		Desc.Texture1D = { MipmapLevel };
	} else if (TexType == LWTexture::Texture2D || TexType==LWTexture::Texture2DMS) {
		Desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		Desc.Texture2D = { MipmapLevel };
	} else if (TexType == LWTexture::TextureCubeMap) {
		Layer += Face;
		Desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		Desc.Texture2DArray = { MipmapLevel, Layer, 1 };
	} else if (TexType == LWTexture::Texture1DArray) {
		Desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE1DARRAY;
		Desc.Texture1DArray = { MipmapLevel, Layer, 1 };
	} else if (TexType == LWTexture::Texture2DArray) {
		Desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		Desc.Texture2DArray = { MipmapLevel, Layer, 1 };
	} else if (TexType == LWTexture::TextureCubeMapArray) {
		Layer = Layer * 6 + Face;
		Desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		Desc.Texture2DArray = { MipmapLevel, Layer, 1 };
	} else if (TexType == LWTexture::Texture2DMS) {
		Desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	}else if (TexType == LWTexture::Texture2DMSArray){
		Desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
		Desc.Texture2DMSArray = { Layer, 1 };
	} else return nullptr;
	HRESULT Res = DriverContext.m_DXDevice->CreateDepthStencilView(m_Texture, &Desc, &View);
	if (FAILED(Res)) {
		std::cout << "Error CreateDepthStencilView: " << std::hex << Res << std::dec << std::endl;
		return nullptr;
	}
	auto ResE = m_DepthStencilViewList.emplace(Hash, View);
	if (!ResE.second) {
		std::cout << "Error DepthStencilViewList.emplace" << std::endl;
		View->Release();
		return nullptr;
	}
	return View;
}

ID3D11UnorderedAccessView *LWDirectX11_1TextureContext::GetUnorderedAccessView(uint32_t Layer, uint32_t Face, uint32_t MipmapLevel, LWTexture *Tex, LWDirectX11_1Context &DriverContext) {
	const DXGI_FORMAT Formats[] = { DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R16G16B16A16_SNORM, DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R32G32B32A32_SINT, DXGI_FORMAT_R32G32B32A32_UINT, DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R8G8_UNORM, DXGI_FORMAT_R16G16_SNORM, DXGI_FORMAT_R16G16_UNORM, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R8_UNORM, DXGI_FORMAT_R16_SNORM, DXGI_FORMAT_R16_UNORM, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_D16_UNORM,    DXGI_FORMAT_D24_UNORM_S8_UINT,     DXGI_FORMAT_D32_FLOAT,    DXGI_FORMAT_D24_UNORM_S8_UINT,     DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_BC6H_SF16, DXGI_FORMAT_BC7_UNORM };
	uint32_t Hash = MakeHash(Layer, Face, MipmapLevel);
	auto Iter = m_UnorderedViewList.find(Hash);
	if (Iter != m_UnorderedViewList.end()) return Iter->second;
	uint32_t PackType = Tex->GetPackType();
	uint32_t TexType = Tex->GetType();
	LWVector3i Size = Tex->Get3DSize();
	uint32_t Mipmaps = Tex->GetMipmapCount();
	bool Compressed = LWImage::CompressedType(PackType);
	bool DepthTex = LWImage::DepthType(PackType);
	if (Compressed || DepthTex) return nullptr;
	D3D11_UNORDERED_ACCESS_VIEW_DESC Desc;
	ID3D11UnorderedAccessView *View = nullptr;
	Desc.Format = Formats[Tex->GetPackType()];
	if (TexType == LWTexture::Texture1D) {
		Desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1D;
		Desc.Texture1D = { MipmapLevel };
	} else if (TexType == LWTexture::Texture2D) {
		Desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		Desc.Texture2D = { MipmapLevel };
	} else if (TexType == LWTexture::Texture3D) {
		Desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
		Desc.Texture3D = { MipmapLevel, 0u, (uint32_t)Size.z };
	} else if (TexType == LWTexture::TextureCubeMap) {
		Layer += Face;
		Desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
		Desc.Texture2DArray = { MipmapLevel, Layer, 1 };
	} else if (TexType == LWTexture::Texture1DArray) {
		Desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1DARRAY;
		Desc.Texture1DArray = { MipmapLevel, Layer, 1 };
	} else if (TexType == LWTexture::Texture2DArray) {
		Desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
		Desc.Texture2DArray = { MipmapLevel, Layer, 1 };
	} else if (TexType == LWTexture::TextureCubeMapArray) {
		Layer = Layer * 6 + Face;
		Desc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
		Desc.Texture2DArray = { MipmapLevel, Layer, 1 };
	} else return nullptr;
	HRESULT Res = DriverContext.m_DXDevice->CreateUnorderedAccessView(m_Texture, &Desc, &View);
	if (FAILED(Res)) {
		std::cout << "Error CreateUnorderedAccessView: " << std::hex << Res << std::dec << std::endl;
		return nullptr;
	}
	auto ResE = m_UnorderedViewList.emplace(Hash, View);
	if (!ResE.second) {
		std::cout << "Error UnorderedViewList.emplace" << std::endl;
		View->Release();
		return nullptr;
	}
	return View;
}
