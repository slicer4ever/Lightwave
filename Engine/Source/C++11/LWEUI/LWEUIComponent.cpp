#include "LWEUI/LWEUIComponent.h"
#include <LWCore/LWVector.h>
#include <LWCore/LWTimer.h>
#include <algorithm>
#include <iostream>

LWEUIComponent &LWEUIComponent::PushComponent(LWEUI *UI) {
	if (m_ComponentCount >= MaxComponents) return *this;
	m_ComponentList[m_ComponentCount++] = UI;
	return *this;
}

LWEUI &LWEUIComponent::UpdateSelf(LWEUIManager *Manager, float Scale, uint64_t lCurrentTime) {
	auto CalculateBounds = [](LWEUI *UI, LWVector4f &Bounds, bool First) {
		LWVector4f VisBounds = UI->GetVisibleBounds();
		if (First) Bounds = VisBounds;
		else {
			Bounds.x = std::min<float>(Bounds.x, VisBounds.x);
			Bounds.y = std::min<float>(Bounds.y, VisBounds.y);
			Bounds.z = std::max<float>(Bounds.z, VisBounds.x);
			Bounds.w = std::max<float>(Bounds.w, VisBounds.y);
		}
	
	};
	if ((m_Flag&NoAutoSize) != 0) return *this;
	
	LWVector4f Bounds;
	for (uint32_t i = 0; i < m_ComponentCount; i++) CalculateBounds(m_ComponentList[i], Bounds, i == 0);
	float IScale = (m_Flag&NoScaleSize) ? 1.0f : 1.0f / Scale;
	m_Size.z = (Bounds.z - Bounds.x)*IScale;
	m_Size.w = (Bounds.w - Bounds.y)*IScale;
	return *this;
}

LWEUI &LWEUIComponent::DrawSelf(LWEUIManager *Manager, LWEUIFrame *Frame, float Scale, uint64_t lCurrentTime) {
	return *this;
}

LWEUI *LWEUIComponent::GetComponent(uint32_t i) {
	return m_ComponentList[i];
}

uint32_t LWEUIComponent::GetComponentCount(void) {
	return 0;
}

LWEUIComponent::LWEUIComponent(const LWVector4f &Position, const LWVector4f &Size, uint32_t Flag) : LWEUI(Position, Size, Flag), m_ComponentCount(0) {}

LWEUIComponent::LWEUIComponent() : LWEUI(LWVector4f(), LWVector4f(), 0), m_ComponentCount(0) {}
