#ifndef MODEL_H
#define MODEL_H
#include <LWCore/LWVector.h>
#include <LWCore/LWQuaternion.h>
#include <LWCore/LWMatrix.h>
#include <LWVideo/LWMesh.h>

class TextureID;

struct Vertice {
	LWVector4f m_Position;
	LWVector4f m_TexCoord;
	LWVector4f m_Tangent;
	LWVector4f m_Normal;
	LWVector4f m_BoneWeights;
	LWVector4i m_BoneIndices;

	Vertice(const LWVector4f &Position) : m_Position(Position) {}

	Vertice() : m_Position(LWVector4f(0.0f, 0.0f, 0.0f, 1.0f)) {} //Setup Vertice so w is 1 if that component isn't specified in the gltf.
};

struct Primitive {
	LWMesh<Vertice> *m_Geometry = nullptr;
	uint32_t m_MaterialID = 0;
	LWVector3f m_AABBMin;
	LWVector3f m_AABBMax;

	Primitive() = default;
};

class Model {
public:
	enum {
		MaxPrimitives = 256
	};

	Model &SetFlag(uint32_t Flag);

	bool PushPrimitive(Primitive &P);

	uint32_t GetFlag(void) const;

	Primitive &GetPrimitive(uint32_t i);

	uint32_t GetPrimitiveCount(void) const;

	LWVector3f GetAABBMin(void) const;

	LWVector3f GetAABBMax(void) const;

	Model();

	~Model();
private:
	Primitive m_Primitives[MaxPrimitives];
	LWVector3f m_AABBMin;
	LWVector3f m_AABBMax;
	uint32_t m_PrimitiveCount = 0;
	uint32_t m_Flag;
};

#endif