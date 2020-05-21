#include "LWEUI/LWEUIRect.h"
#include "LWEAsset.h"
#include "LWELocalization.h"
#include <LWPlatform/LWWindow.h>
#include <LWVideo/LWTexture.h>

LWEUIRect *LWEUIRect::XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, const char *ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap) {
	char Buffer[256];
	char SBuffer[1024 * 32];
	LWAllocator *Allocator = Manager->GetAllocator();
	LWELocalization *Localize = Manager->GetLocalization();
	LWEUIRect *Rect = Allocator->Allocate<LWEUIRect>(nullptr, LWVector4f(0.0f), LWVector4f(0.0f), 0);
	LWXMLAttribute *StyleAttr = Node->FindAttribute("Style");
	LWEXMLNode *Style = nullptr;
	if (StyleAttr) {
		auto Iter = StyleMap.find(LWText::MakeHash(ParseComponentAttribute(Buffer, sizeof(Buffer), StyleAttr->m_Value, ActiveComponent, ActiveComponentNode)));
		if (Iter != StyleMap.end()) Style = Iter->second;
	}
	LWEUI::XMLParse(Rect, Node, XML, Manager, Style, ActiveComponentName, ActiveComponent, ActiveComponentNode, StyleMap, ComponentMap);
	LWXMLAttribute *MatAttr = FindAttribute(Node, Style, "Material");
	LWXMLAttribute *ThetaAttr = FindAttribute(Node, Style, "Theta");
	LWEUIMaterial *Mat = nullptr;
	float Theta = 0.0f;
	if (MatAttr) {
		const char *Res = ParseComponentAttribute(Buffer, sizeof(Buffer), MatAttr->m_Value, ActiveComponent, ActiveComponentNode);
		if (Localize) Res = Localize->ParseLocalization(SBuffer, sizeof(SBuffer), Res);
		Mat = Manager->GetMaterial(Res);
	}
	if(ThetaAttr) Theta = (float)atof(ThetaAttr->m_Value) * LW_DEGTORAD;

	Rect->SetMaterial(Mat).SetTheta(Theta);
	return Rect;
}

LWEUI &LWEUIRect::UpdateSelf(LWEUIManager &Manager, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime) {
	bool wasOver = (m_Flag&MouseOver);
	m_Flag = (m_Flag&~MouseOver) | (m_TimeOver ? MouseOver : 0);
	bool isOver = (m_Flag&MouseOver);
	Manager.DispatchEvent(this, Event_MouseOver, isOver && !wasOver);
	Manager.DispatchEvent(this, Event_MouseOff, !isOver && wasOver);
	return *this;
}

LWEUI &LWEUIRect::DrawSelf(LWEUIManager &Manager, LWEUIFrame &Frame, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime) {
	Frame.WriteRect(m_Material, VisiblePos, VisibleSize, m_Theta);
	return *this;
}

void LWEUIRect::Destroy(void) {
	LWAllocator::Destroy(this);
	return;
}

LWEUIRect &LWEUIRect::SetMaterial(LWEUIMaterial *Material) {
	if (m_Flag&SizeToTexture && m_Material && m_Material->m_Texture) {
		LWVector2i TexSize = m_Material->m_Texture->Get2DSize();
		LWVector2f SubSize = LWVector2f(m_Material->m_SubRegion.z - m_Material->m_SubRegion.x, m_Material->m_SubRegion.w - m_Material->m_SubRegion.y)*LWVector2f((float)TexSize.x, (float)TexSize.y);
		m_Size.z -= SubSize.x;
		m_Size.w -= SubSize.y;
	}
	m_Material = Material;
	if (m_Flag&SizeToTexture && m_Material && m_Material->m_Texture) {
		LWVector2i TexSize = m_Material->m_Texture->Get2DSize();
		LWVector2f SubSize = LWVector2f(m_Material->m_SubRegion.z - m_Material->m_SubRegion.x, m_Material->m_SubRegion.w - m_Material->m_SubRegion.y)*LWVector2f((float)TexSize.x, (float)TexSize.y);
		m_Size.z += SubSize.x;
		m_Size.w += SubSize.y;
	}
	return *this;
}

LWEUIRect &LWEUIRect::SetTheta(float Theta) {
	m_Theta = Theta;
	return *this;
}

LWEUIMaterial *LWEUIRect::GetMaterial(void) {
	return m_Material;
}

float LWEUIRect::GetTheta(void) const {
	return m_Theta;
}

LWEUIRect::LWEUIRect(LWEUIMaterial *Material, const LWVector4f &Position, const LWVector4f &Size, uint64_t Flag) : LWEUI(Position, Size, Flag), m_Material(Material), m_Theta(0.0f) {}

LWEUIRect::LWEUIRect() : LWEUI(LWVector4f(), LWVector4f(), 0), m_Material(nullptr), m_Theta(0.0f) {}

LWEUIRect::~LWEUIRect() {}
