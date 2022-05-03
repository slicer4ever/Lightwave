#ifndef APP_H
#define APP_H
#include <LWCore/LWTypes.h>
#include <LWPlatform/LWTypes.h>
#include <LWVideo/LWTypes.h>
#include <LWCore/LWLogger.h>
#include <LWEJobQueue.h>
#include <LWEAsset.h>
#include <LWEUIManager.h>
#include <LWECamera.h>
#include <LWERenderer.h>
#include <LWEMesh.h>

struct PrimitiveMaterial {
	LWEGeometryModelData m_Data;
	LWERenderMaterial m_Material;

	PrimitiveMaterial(LWEGLTFParser &P, LWEGLTFMaterial *Mat, const LWSMatrix4f &Transform, LWERenderer *R, LWAllocator &Allocator);

	PrimitiveMaterial() = default;
};

struct Model {
	LWEMesh m_Mesh;
	std::vector<PrimitiveMaterial> m_MaterialList;
};

class App {
public:

	void UpdateJob(LWEJob &J, LWEJobThread &Th, LWEJobQueue &Q, uint64_t lCurrentTime);

	void InputJob(LWEJob &J, LWEJobThread &Th, LWEJobQueue &Q, uint64_t lCurrentTime);

	void RenderJob(LWEJob &J, LWEJobThread &Th, LWEJobQueue &Q, uint64_t lCurrentTime);

	void Run(void);

	bool LoadAssets(const LWUTF8I &Path);

	bool LoadGLTF(const LWUTF8I &Path);

	App(LWAllocator &Allocator);

	~App();
private:
	LWEJobQueue m_JobQueue;
	LWAllocator &m_Allocator;
	LWECamera m_Camera = LWECamera(LWSVector4f(0.0f, 0.0f, 10.0f, 1.0f), LWSVector4f(0.0f, 0.0f,-1.0f, 0.0f), LWSVector4f(0.0f, 1.0f, 0.0f, 0.0f), 1.0f, LW_PI_4, 0.1f, 1000.0f, 0);
	LWWindow *m_Window = nullptr;
	LWVideoDriver *m_VDriver = nullptr;
	LWEAssetManager *m_AssetManager = nullptr;
	LWEUIManager *m_UIManager = nullptr;
	LWERenderer *m_Renderer = nullptr;
	LWEUILabel *m_DebugLabel = nullptr;
	uint32_t m_SwapInterval = 1;
	LWSVector4f m_LightPos = LWSVector4f(0.0f, 0.0f, 10.0f, 1.0f);
	LWSVector4f m_LightDir = LWSVector4f(0.0f, 0.0f, -1.0f, 0.0f);
	std::vector<Model*> m_ModelList;
	LWLoggerTimeMetrics m_InputMetric;
	LWLoggerTimeMetrics m_UpdateMetric;
	LWLoggerTimeMetrics m_FrameMetric;
	LWLoggerTimeMetrics m_RenderMetric;
};

#endif