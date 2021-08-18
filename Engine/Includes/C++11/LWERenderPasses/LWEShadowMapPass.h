#ifndef LWESHADOWMAPPASS_H
#define LWESHADOWMAPPASS_H
#include "LWERenderPasses/LWEGeometryPass.h"

//LWEShadowMapPass
class LWEShadowMapPass : public LWEGeometryPass {
public:
	LWBitField32(CubeFaceIndex, 3, 28);
	static const uint32_t CubeIndex = 0x80000000;

	/*!< \brief parse's shadowmap pass, creating the following named Textures in renderer: {PassName}TexArray, {PassName}CubeArray.
	*	 Attributes ArraySize(Required, parses as Size, size of each array slice), CubeSize(Optional, size of each cubemap face for point lights, if not included point lights are ignored as possible shadow casters), Buckets(Required, The number of shadow buckets allowed to be used, note for point lights shadow casting, 6 buckets will be used(The bucket child nodes should not be specified with ShadowMapPass node)).  CascadeCount(number of maps to use for directional shadows).
	*    Pipeline(Required, default pipeline used for rendering all opaque objects).
	*/
	static LWEPass *ParseXML(LWEXMLNode *Node, LWEPass *Pass, LWERenderer *Renderer, LWEAssetManager *AssetManager, LWAllocator &Allocator);

	uint32_t RenderPass(LWERenderFrame &Frame, LWEGeometryRenderable RenderableList[LWEMaxGeometryBuckets][LWEMaxBucketSize], std::pair<uint32_t, uint32_t> RenderableCount[LWEMaxGeometryBuckets], LWERenderer *Renderer, LWVideoDriver *Driver, uint32_t SubPassIndex);

	LWEShadowMapPass &SetShadowIDs(uint32_t ShadowTexArrayID, uint32_t ShadowArrayFBID, uint32_t ShadowCubeArrayID, uint32_t ShadowCubeFBID);

	LWEShadowMapPass &SetCascadeCount(uint32_t CascadeCount);

	LWEShadowMapPass &SetPassBitID(uint32_t PassID);

	//Sorts the frame's shadow casting lights in front to back, will add each castor equal to the number of buckets.  if a castor will require more buckets then what is remaining, then it is ignored. */
	LWEShadowMapPass &InitializeShadowCastorBuckets(LWERenderFrame &Frame);

	uint32_t GetPassBitID(void) const;

	LWEShadowMapPass(uint32_t ShadowMapCount);

	LWEShadowMapPass() = default;

protected:
	uint32_t m_FBTargetIndex[LWEMaxGeometryBuckets] = {};
	uint32_t m_ShadowArrayFBID = 0;
	uint32_t m_ShadowCubeFBID = 0;
	uint32_t m_ShadowTexArrayID = 0; //For directional/spot lights.
	uint32_t m_ShadowCubeArrayID = 0; //For point lights.
	uint32_t m_CascadeCount = 3; //Number of buckets to use for a directional light source.
	uint32_t m_PassBitID = 0;
};

#endif