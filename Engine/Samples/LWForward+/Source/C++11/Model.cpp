#include "Model.h"
#include <LWCore/LWAllocator.h>
#include <LWCore/LWByteBuffer.h>
#include <LWPlatform/LWFileStream.h>
#include <cstring>
#include <algorithm>
#include <functional>


Model &Model::SetFlag(uint32_t Flag) {
	m_Flag = Flag;
	return *this;
}

bool Model::PushPrimitive(Primitive &P) {
	if (m_PrimitiveCount >= MaxPrimitives) return false;
	if (!m_PrimitiveCount) {
		m_AABBMin = P.m_AABBMin;
		m_AABBMax = P.m_AABBMax;
	} else {
		m_AABBMin = m_AABBMin.Min(P.m_AABBMin);
		m_AABBMax = m_AABBMax.Max(P.m_AABBMax);
	}
	m_Primitives[m_PrimitiveCount] = P;
	m_PrimitiveCount++;
	return true;
}

uint32_t Model::GetFlag(void) const {
	return m_Flag;
}

Primitive &Model::GetPrimitive(uint32_t i) {
	return m_Primitives[i];
}

uint32_t Model::GetPrimitiveCount(void) const {
	return m_PrimitiveCount;
}

LWVector3f Model::GetAABBMin(void) const {
	return m_AABBMin;
}

LWVector3f Model::GetAABBMax(void) const {
	return m_AABBMax;
}

Model::Model() {}

Model::~Model() {}
