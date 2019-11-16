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

LWEUI &LWEUIRect::UpdateSelf(LWEUIManager *Manager, float Scale, uint64_t lCurrentTime) {

	LWWindow *Wnd = Manager->GetWindow();
	LWMouse *Mouse = Wnd->GetMouseDevice();
	LWTouch *Touch = Wnd->GetTouchDevice();
	uint32_t Flag = m_Flag;
	m_Flag &= ~MouseOver;
	if (Mouse) {
		LWVector2i MP = Mouse->GetPosition();
		LWVector2f MousePnt = LWVector2f((float)MP.x, (float)MP.y);
		bool Over = MousePnt.x >= m_VisiblePosition.x && MousePnt.x <= m_VisiblePosition.x + m_VisibleSize.x && MousePnt.y >= m_VisiblePosition.y && MousePnt.y <= m_VisiblePosition.y + m_VisibleSize.y;
		if (Over) {
			Manager->DispatchEvent(this, Event_TempOverInc);
			m_Flag |= MouseOver;
		}
	}
	if (Touch) {
		for (uint32_t i = 0; i < Touch->GetPointCount(); i++) {
			auto Pnt = Touch->GetPoint(i);
			LWVector2f TouchPnt = LWVector2f((float)Pnt->m_Position.x, (float)Pnt->m_Position.y);
			bool Over = TouchPnt.x + Pnt->m_Size >= m_VisiblePosition.x && TouchPnt.x - Pnt->m_Size <= m_VisiblePosition.x + m_VisibleSize.x && TouchPnt.y + Pnt->m_Size >= m_VisiblePosition.y && TouchPnt.y - Pnt->m_Size <= m_VisiblePosition.y + m_VisibleSize.y;
			if (Over) {
				Manager->DispatchEvent(this, Event_TempOverInc | (i << Event_OverOffset));
				m_Flag |= MouseOver;
			}
		}
	}
	if (m_Flag&MouseOver && (Flag&MouseOver) == 0) Manager->DispatchEvent(this, Event_MouseOver);
	if ((m_Flag&MouseOver) == 0 && Flag&MouseOver) Manager->DispatchEvent(this, Event_MouseOff);
	return *this;
}

LWEUI &LWEUIRect::DrawSelf(LWEUIManager *Manager, LWEUIFrame *Frame, float Scale, uint64_t lCurrentTime) {
	auto DrawRect = [](LWEUIMaterial *Mat, const LWVector2f &Pos, const LWVector2f &Size, float Theta, LWEUIFrame *F)->bool {
		if (!Mat) return false;
		if (!F->SetActiveTexture(Mat->m_Texture, false)) return false;
		LWVector4f SubRegion = Mat->m_SubRegion;
		LWVector2f hSize = Size * 0.5f;
		LWVector2f SubTL = LWVector2f(SubRegion.x, SubRegion.w);
		LWVector2f SubBR = LWVector2f(SubRegion.z, SubRegion.y);
		LWVector2f hSubSize = (SubBR - SubTL)*0.5f;
		uint32_t c = LWVertexUI::WriteRect(F->m_Mesh, Pos + hSize, hSize, Theta, Mat->m_Color, SubTL + hSubSize, hSubSize);
		F->m_VertexCount[F->m_TextureCount - 1] += c;
		return c != 0;
	};

	DrawRect(m_Material, m_VisiblePosition, m_VisibleSize, m_Theta, Frame);
	return *this;
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

LWEUIRect::LWEUIRect(LWEUIMaterial *Material, const LWVector4f &Position, const LWVector4f &Size, uint32_t Flag) : LWEUI(Position, Size, Flag), m_Material(Material), m_Theta(0.0f) {}

LWEUIRect::LWEUIRect() : LWEUI(LWVector4f(), LWVector4f(), 0), m_Material(nullptr), m_Theta(0.0f) {}

LWEUIRect::~LWEUIRect() {}
