#include "LWEUI/LWEUIComponent.h"
#include <LWCore/LWVector.h>
#include <LWCore/LWTimer.h>
#include <algorithm>
#include <iostream>

LWEUIComponent *LWEUIComponent::XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, LWEXMLNode *Style, const LWUTF8Iterator &ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode *> &StyleMap, std::map<uint32_t, LWEXMLNode *> &ComponentMap) {
	char8_t Buffer[256];
	char8_t NameBuffer[256]="";
	uint32_t NameHash = Node->GetName().Hash();
	auto Iter = ComponentMap.find(NameHash);
	if (Iter == ComponentMap.end()) {
		fmt::print("Error unknown node: '{}'\n", Node->GetName());
		return nullptr;
	}
	LWEXMLNode *Component = Iter->second;
	LWEXMLAttribute *NameAttr = Node->FindAttribute("Name");
	LWUTF8Iterator ActiveName = NameBuffer;
	if (NameAttr) {
		if (!ActiveComponentName.AtEnd()) snprintf(NameBuffer, sizeof(NameBuffer), "%s.%s", ActiveComponentName(), ParseComponentAttribute(Buffer, sizeof(Buffer), NameAttr->m_Value, ActiveComponent, ActiveComponentNode)());
		else ActiveName = ParseComponentAttribute(Buffer, sizeof(Buffer), NameAttr->GetValue(), ActiveComponent, ActiveComponentNode);
	} else if (!ActiveComponentName.AtEnd()) ActiveName = ActiveComponentName;
	LWEUIComponent *Cmp = Manager->GetAllocator().Create<LWEUIComponent>(LWVector4f(0.0f), LWVector4f(0.0f), 0);
	for (LWEXMLNode *C = XML->NextNode(nullptr, Component); C; C = XML->NextNode(C, Component, true)) {
		LWEUI *S = LWEUI::XMLParseSubNodes(Cmp, C, XML, Manager, ActiveName, Component, Node, StyleMap, ComponentMap);
		if (S) Cmp->PushComponent(S);
	}
	LWEUI::XMLParse(Cmp, Node, XML, Manager, Style, ActiveComponentName, ActiveComponent, ActiveComponentNode, StyleMap, ComponentMap);
	return Cmp;
}

LWEUI &LWEUIComponent::UpdateSelf(LWEUIManager &Manager, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime) {
	bool AutoWidth = (m_Flag&NoAutoWidthSize) == 0;
	bool AutoHeight = (m_Flag&NoAutoHeightSize) == 0;
	bool ScaleSize = (m_Flag&NoScaleSize) == 0;
	if (!AutoWidth && !AutoHeight) return *this;
	if (m_VisibleBounds.x == 0.0f && m_VisibleBounds.y == 0.0f && m_VisibleBounds.z == 0.0f && m_VisibleBounds.w == 0.0f) return *this;
	LWVector4f CBounds = LWVector4f(VisiblePos, VisiblePos + VisibleSize);;

	for (uint32_t i = 0; i < m_ComponentCount; i++) {
		LWVector2f CVisPos = m_ComponentList[i]->GetVisiblePosition();
		LWVector2f CVisSize = m_ComponentList[i]->GetVisibleSize();
		CBounds = MakeNewBounds(CBounds, LWVector4f(CVisPos, CVisPos+CVisSize));
	}
	float iScale = ScaleSize ? 1.0f / Scale : 1.0f;
	LWVector4f Size = m_Size;
	if(AutoWidth) Size.z = (CBounds.z - CBounds.x)*iScale;
	if(AutoHeight) Size.w = (CBounds.w - CBounds.y)*iScale;
	LWVector4f Bounds = MakeVisibleBounds(m_Flag, ParentVisiblePos, ParentVisibleSize, Manager, m_Position, Size, Scale);
	VisiblePos = Bounds.xy();
	VisibleSize = Bounds.zw();
	return *this;
}

LWEUI &LWEUIComponent::DrawSelf(LWEUIManager &Manager, LWEUIFrame &Frame, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime) {
	bool AutoWidth = (m_Flag&NoAutoWidthSize) == 0;
	bool AutoHeight = (m_Flag&NoAutoHeightSize) == 0;
	bool ScaleSize = (m_Flag&NoScaleSize) == 0;

	if (!AutoWidth && !AutoHeight) return *this;
	if (m_VisibleBounds.x == 0.0f && m_VisibleBounds.y == 0.0f && m_VisibleBounds.z == 0.0f && m_VisibleBounds.w == 0.0f) return *this;
	LWVector4f CBounds = LWVector4f(VisiblePos, VisiblePos + VisibleSize);
	for (uint32_t i = 0; i < m_ComponentCount; i++) {
		LWVector2f CVisPos = m_ComponentList[i]->GetVisiblePosition();
		LWVector2f CVisSize = m_ComponentList[i]->GetVisibleSize();
		CBounds = MakeNewBounds(CBounds, LWVector4f(CVisPos, CVisPos+CVisSize));
	}
	float iScale = ScaleSize ? 1.0f / Scale : 1.0f;
	LWVector4f Size = m_Size;
	if (AutoWidth) Size.z = (CBounds.z - CBounds.x)*iScale;
	if (AutoHeight) Size.w = (CBounds.w - CBounds.y)*iScale;
	LWVector4f Bounds = MakeVisibleBounds(m_Flag, ParentVisiblePos, ParentVisibleSize, Manager, m_Position, Size, Scale);
	VisiblePos = Bounds.xy();
	VisibleSize = Bounds.zw();
	
	return *this;
}

void LWEUIComponent::Destroy(void) {
	LWAllocator::Destroy(this);
	return;
}

bool LWEUIComponent::PushComponent(LWEUI *Component) {
	if (m_ComponentCount >= MaxComponentCount) return false;
	m_ComponentList[m_ComponentCount] = Component;
	m_ComponentCount++;
	return true;
}


LWEUIComponent::LWEUIComponent(const LWVector4f &Position, const LWVector4f &Size, uint64_t Flag) : LWEUI(Position, Size, Flag) {}

LWEUIComponent::LWEUIComponent() : LWEUI(LWVector4f(), LWVector4f(), 0) {}
