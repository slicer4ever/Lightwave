#include "LWVideo/LWVideoDrivers/LWVideoDriver_Vulkan.h"
#include <iostream>

bool LWVideoDriver_Vulkan::DebugResult(const char *ErrorMessage, VkResult Result, bool Verbose) {
	VkResult ErrorList[] = { VK_SUCCESS, VK_NOT_READY, VK_TIMEOUT, VK_EVENT_SET, VK_EVENT_RESET, VK_INCOMPLETE,
		VK_ERROR_OUT_OF_HOST_MEMORY, VK_ERROR_OUT_OF_DEVICE_MEMORY, VK_ERROR_INITIALIZATION_FAILED,
		VK_ERROR_DEVICE_LOST, VK_ERROR_MEMORY_MAP_FAILED, VK_ERROR_LAYER_NOT_PRESENT, VK_ERROR_EXTENSION_NOT_PRESENT,
		VK_ERROR_FEATURE_NOT_PRESENT, VK_ERROR_INCOMPATIBLE_DRIVER, VK_ERROR_TOO_MANY_OBJECTS, VK_ERROR_FORMAT_NOT_SUPPORTED,
		VK_ERROR_FRAGMENTED_POOL, VK_ERROR_UNKNOWN, VK_ERROR_OUT_OF_DEVICE_MEMORY, VK_ERROR_INVALID_EXTERNAL_HANDLE,
		VK_ERROR_FRAGMENTATION, VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS, VK_ERROR_SURFACE_LOST_KHR,
		VK_ERROR_NATIVE_WINDOW_IN_USE_KHR, VK_SUBOPTIMAL_KHR, VK_ERROR_OUT_OF_DATE_KHR,
		VK_ERROR_INCOMPATIBLE_DISPLAY_KHR, VK_ERROR_VALIDATION_FAILED_EXT, VK_ERROR_INVALID_SHADER_NV,
		VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT, VK_ERROR_NOT_PERMITTED_EXT,
		VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT, VK_ERROR_OUT_OF_POOL_MEMORY_KHR,
		VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR, VK_ERROR_FRAGMENTATION_EXT,
		VK_ERROR_INVALID_DEVICE_ADDRESS_EXT, VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR,
		VK_RESULT_BEGIN_RANGE, VK_RESULT_END_RANGE, VK_RESULT_RANGE_SIZE, VK_RESULT_MAX_ENUM };
	const char *ErrorNames[] = { "SUCCESS", "NOT READY", "TIMEOUT", "EVENT SET", "EVENT RESET", "INCOMPLETE",
		"OUT OF HOST MEMORY", "OUT OF DEVICE MEMORY", "INITIALIZATION FAILED", "DEVICE LOST", "MEMORY MAP FAILED",
		"LAYER NOT PRESENT", "FEATURE NOT PRESENT", "INCOMPATIBLE DRIVER", "TOO MANY OBJECTS", "FORMAT NOT SUPPORTED",
		"FRAGMENTED POOL", "UNKNOWN", "OUT OF DEVICE MEMORY", "INVALID EXTERNAL HANDLE", "FRAGMENTATION", "INVALID OPAQUE CAPTURE ADDRESS",
		"SURFACE LOST KHR", "NATIVE WINDOW IN USE KHR", "SUBOPTIMAL KHR", "OUT OF DATE KHR", "INCOMPATIBLE DISPLAY KHR",
		"VALIDATION FAILED EXT", "INVALID SHADER NV", "INVALID DRM FORMAT MODIFIER PLANE LAYOUT EXT", "NOT PERMITTED EXT",
		"FULL SCREEN EXCLUSIVE MODE LOST EXT", "OUT OF POOL MEMORY KHR", "INVALID EXTERNAL HANDLE KHR", "FRAGMENTATION EXT",
		"INVALID DEVICE ADDRESS EXT", "INVALID OPAQUE CAPTURE ADDRESS KHR", "BEGIN RANGE", "END RANGE", "RANGE SIZE", "MAX ENUM" };
	const uint32_t UnknownID = 17;
	const uint32_t ErrorCount = sizeof(ErrorList) / sizeof(VkResult);
	if(Result!=VK_SUCCESS){
		uint32_t i = 0;
		for (; i < ErrorCount; i++) {
			if (ErrorList[i] == Result) break;
		}
		if (i >= ErrorCount) i = UnknownID;
		if (Verbose) std::cout << ErrorMessage << ": " << ErrorNames[i] << std::endl;
		return false;
	}
	return true;
}

LWVideoDriver &LWVideoDriver_Vulkan::ClearColor(uint32_t Color) {
	return *this;
}

LWVideoDriver &LWVideoDriver_Vulkan::ClearDepth(float Depth) {
	return *this;
}

LWVideoDriver &LWVideoDriver_Vulkan::ClearStencil(uint8_t Stencil) {
	return *this;
}

LWVideoDriver &LWVideoDriver_Vulkan::ViewPort(const LWVector4i &Viewport) {
	return *this;
}

bool LWVideoDriver_Vulkan::SetRasterState(uint64_t Flags, float Bias, float SlopedScaleBias) {
	return false;
}

bool LWVideoDriver_Vulkan::SetFrameBuffer(LWFrameBuffer *Buffer) {
	return false;
}

bool LWVideoDriver_Vulkan::SetPipeline(LWPipeline *Pipeline, LWVideoBuffer *VertexBuffer, LWVideoBuffer *IndiceBuffer, uint32_t VertexStride, uint32_t Offset) {
	return false;
}

LWShader *LWVideoDriver_Vulkan::CreateShader(uint32_t ShaderType, const char *Source, LWAllocator &Allocator, char *CompiledBuffer, char *ErrorBuffer, uint32_t *CompiledBufferLen, uint32_t ErrorBufferLen) {
	return nullptr;
}

LWShader *LWVideoDriver_Vulkan::CreateShaderCompiled(uint32_t ShaderType, const char *CompiledCode, uint32_t CompiledCodeLen, LWAllocator &Allocator, char *ErrorBuffer, uint32_t ErroBufferLen) {
	return nullptr;
}

LWPipeline *LWVideoDriver_Vulkan::CreatePipeline(LWPipeline *Source, LWAllocator &Allocator) {
	return nullptr;
}

LWVideoDriver &LWVideoDriver_Vulkan::ClonePipeline(LWPipeline *Target, LWPipeline *Source) {
	return *this;
}

LWTexture *LWVideoDriver_Vulkan::CreateTexture1D(uint32_t TextureState, uint32_t PackType, uint32_t Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	return nullptr;
}

LWTexture *LWVideoDriver_Vulkan::CreateTexture2D(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	return nullptr;
}

LWTexture *LWVideoDriver_Vulkan::CreateTexture3D(uint32_t TextureState, uint32_t PackType, const LWVector3i &Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	return nullptr;
}

LWTexture *LWVideoDriver_Vulkan::CreateTextureCubeMap(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	return nullptr;
}

LWTexture *LWVideoDriver_Vulkan::CreateTexture1DArray(uint32_t TextureState, uint32_t PackType, uint32_t Size, uint32_t Layers, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	return nullptr;
}

LWTexture *LWVideoDriver_Vulkan::CreateTexture2DArray(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Layers, uint8_t **Texels, uint32_t MipmapCnt, LWAllocator &Allocator) {
	return nullptr;
}

LWTexture *LWVideoDriver_Vulkan::CreateTextureCubeArray(uint32_t TextureState, uint32_t PackType, const LWVector2i &Size, uint32_t Layers, uint8_t **Texels, uint32_t MapmapCnt, LWAllocator &Allocator) {
	return nullptr;
}

LWVideoBuffer *LWVideoDriver_Vulkan::CreateVideoBuffer(uint32_t Type, uint32_t UsageFlag, uint32_t TypeSize, uint32_t Length, LWAllocator &Allocator, const uint8_t *Buffer) {
	return nullptr;
}

LWPipeline *LWVideoDriver_Vulkan::CreatePipeline(LWShader **Stages, uint64_t Flag, LWAllocator &Allocator) {
	return nullptr;
}

LWFrameBuffer *LWVideoDriver_Vulkan::CreateFrameBuffer(const LWVector2i &Size, LWAllocator &Allocator) {
	return nullptr;
}

bool LWVideoDriver_Vulkan::UpdateTexture(LWTexture *Texture) {
	return false;
}

bool LWVideoDriver_Vulkan::UpdateTexture1D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, uint32_t Position, uint32_t Size) {
	return false;
}

bool LWVideoDriver_Vulkan::UpdateTexture2D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, const LWVector2i &Position, const LWVector2i &Size) {
	return false;
}

bool LWVideoDriver_Vulkan::UpdateTexture3D(LWTexture *Texture, uint32_t MipmapLevel, void *Texels, const LWVector3i &Position, const LWVector3i &Size) {
	return false;
}

bool LWVideoDriver_Vulkan::UpdateTextureCubeMap(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Face, void *Texels, const LWVector2i &Position, const LWVector2i &Size) {
	return false;
}

bool LWVideoDriver_Vulkan::UpdateTexture1DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, void *Texels, uint32_t Position, uint32_t Size) {
	return false;
}

bool LWVideoDriver_Vulkan::UpdateTexture2DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, void *Texels, const LWVector2i &Position, const LWVector2i &Size) {
	return false;
}

bool LWVideoDriver_Vulkan::UpdateTextureCubeArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint32_t Face, void *Texels, const LWVector2i &Position, const LWVector2i &Size) {
	return false;
}

bool LWVideoDriver_Vulkan::UpdateVideoBuffer(LWVideoBuffer *VideoBuffer, const uint8_t *Buffer, uint32_t Length) {
	return false;
}

bool LWVideoDriver_Vulkan::DownloadTexture1D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver_Vulkan::DownloadTexture2D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver_Vulkan::DownloadTexture3D(LWTexture *Texture, uint32_t MipmapLevel, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver_Vulkan::DownloadTextureCubeMap(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Face, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver_Vulkan::DownloadTexture1DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver_Vulkan::DownloadTexture2DArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver_Vulkan::DownloadTextureCubeArray(LWTexture *Texture, uint32_t MipmapLevel, uint32_t Layer, uint32_t Face, uint8_t *Buffer) {
	return false;
}

bool LWVideoDriver_Vulkan::DownloadVideoBuffer(LWVideoBuffer *VBuffer, uint8_t *Buffer, uint32_t Offset, uint32_t Length) {
	return false;
}

LWVideoDriver &LWVideoDriver_Vulkan::DestroyPipeline(LWPipeline *Pipeline) {
	return *this;
}

LWVideoDriver &LWVideoDriver_Vulkan::DestroyVideoBuffer(LWVideoBuffer *Buffer) {
	return *this;
}

LWVideoDriver &LWVideoDriver_Vulkan::DestroyShader(LWShader *Shader) {
	return *this;
}

LWVideoDriver &LWVideoDriver_Vulkan::DestroyTexture(LWTexture *Texture) {
	return *this;
}

LWVideoDriver &LWVideoDriver_Vulkan::DestroyFrameBuffer(LWFrameBuffer *FrameBuffer) {
	return *this;
}

LWVideoDriver &LWVideoDriver_Vulkan::DrawBuffer(LWPipeline *Pipeline, int32_t DrawMode, LWVideoBuffer *InputBlock, LWVideoBuffer *IndexBuffer, uint32_t Count, uint32_t VertexStride, uint32_t Offset) {
	return *this;
}

LWVideoDriver &LWVideoDriver_Vulkan::DrawInstancedBuffer(LWPipeline *Pipeline, int32_t DrawMode, LWVideoBuffer *InputBlock, LWVideoBuffer *IndexBuffer, uint32_t Count, uint32_t VertexStride, uint32_t InstanceCount, uint32_t Offset) {
	return *this;
}

LWVideoDriver &LWVideoDriver_Vulkan::Dispatch(LWPipeline *Pipeline, const LWVector3i &GroupDimension) {
	return *this;
}

LWVulkan_Context &LWVideoDriver_Vulkan::GetContext(void) {
	return m_Context;
}

LWVideoDriver_Vulkan::LWVideoDriver_Vulkan(LWWindow *Window, LWVulkan_Context &Context, uint32_t UniformBlockSize) : LWVideoDriver(Window, LWVideoDriver::Vulkan, UniformBlockSize), m_Context(Context) {
}

