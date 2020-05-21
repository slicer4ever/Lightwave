#include "LWEUIManager.h"
#include <LWVideo/LWFont.h>
#include <LWPlatform/LWWindow.h>
#include <LWPlatform/LWFileStream.h>
#include <LWCore/LWText.h>
#include <LWCore/LWTimer.h>
#include "LWEAsset.h"
#include "LWEXML.h"
#include "LWEUI/LWEUIButton.h"
#include "LWEUI/LWEUILabel.h"
#include "LWEUI/LWEUIRect.h"
#include "LWEUI/LWEUIListBox.h"
#include "LWEUI/LWEUIScrollBar.h"
#include "LWEUI/LWEUITextInput.h"
#include "LWEUI/LWEUIComponent.h"
#include "LWEUI/LWEUIRichLabel.h"
#include "LWEUI/LWEUITreeList.h"
#include "LWEGeometry2D.h"
#include <map>
#include <cstdarg>
#include <iostream>
#include <algorithm>

LWEUITooltip &LWEUITooltip::Draw(LWEUIFrame &Frame, LWEUIManager &UIMan, float Scale, uint64_t lCurrentTime) {
	const uint64_t ShowFreq = LWTimer::GetResolution();
	LWEUI *UI = m_TooltipedUI;
	if (!m_Font) return *this;
	if (!UI) return *this;
	LWWindow *Wnd = UIMan.GetWindow();
	LWMouse *Mouse = Wnd->GetMouseDevice();
	if (!Mouse) return *this;
	if (Mouse->ButtonDown(LWMouseKey::Left)) m_TooltipSize.x = -1.0f; //Hide tooltip till a new tooltip is needed.
	if (m_TooltipSize.x < 0.0f) return *this;
	if (lCurrentTime - UI->GetOverTime() < ShowFreq) return *this;
	LWVector2f WndSize = Wnd->GetSizef();
	LWVector2f Pos = Mouse->GetPositionf();
	if (Pos.x + m_TooltipSize.x > WndSize.x) Pos.x -= m_TooltipSize.x;
	if (Pos.y + m_TooltipSize.y > WndSize.y) Pos.y -= m_TooltipSize.y;
	float Border = m_BorderSize * Scale;
	Frame.WriteRect(m_BorderMaterial, Pos - Border, m_TooltipSize + Border * 2.0f);
	Frame.WriteRect(m_BackgroundMaterial, Pos, m_TooltipSize);
	m_Font->DrawTextm(UI->GetTooltip(), Pos - LWVector2f(0.0f, m_UnderHang), m_FontScale*Scale, m_FontMaterial ? m_FontMaterial->m_ColorA : LWVector4f(1.0f), &Frame, &LWEUIFrame::WriteFontGlyph);
	return *this;
}

LWEUITooltip &LWEUITooltip::Update(float Scale) {
	if (m_TempTooltipedUI == m_TooltipedUI) return *this;
	if (!m_Font) return *this;
	if (m_TempTooltipedUI) {
		LWVector4f TextSize = m_Font->MeasureText(m_TempTooltipedUI->GetTooltip(), m_FontScale*Scale);
		m_TooltipSize = LWVector2f(TextSize.z - TextSize.x, TextSize.y - TextSize.w);
		m_UnderHang = TextSize.w;
	}
	m_TooltipedUI = m_TempTooltipedUI;
	return *this;
}

bool LWEUINavigation::isPressed(void) const {
	return (m_Flag&Pressed) != 0;
}

bool LWEUINavigation::isBack(void) const {
	return (m_Flag&Back) != 0;
}

bool LWEUINavigation::isEnabled(void) const {
	return (m_Flag&(KeyboardEnabled | GamepadEnabled)) != 0;
}

LWEUINavigation &LWEUINavigation::ProcessUI(LWEUI *UI, const LWVector2f &VisiblePosition, const LWVector2f &VisibleSize, LWEUIManager &UIManager) {
	if (m_Direction.LengthSquared() < 1.0f) return *this;
	if (!UI->isFocusAble()) return *this;
	LWEUI *Focused = UIManager.GetFocusedUI();
	if (Focused == UI) return *this;
	bool hasFocused = Focused != nullptr;
	float PerpWidth = 10.0f;
	if (hasFocused) {
		if (fabs(m_Direction.x) > fabs(m_Direction.y)) PerpWidth = Focused->GetVisibleSize().y;
		else PerpWidth = Focused->GetVisibleSize().x;
	}
	LWVector2f Ctr = VisiblePosition + VisibleSize*0.5f;
	LWVector2f Dir = Ctr - m_Center;
	float D = Dir.Dot(m_Direction);
	float pD = Dir.Dot(m_PerpDirection);
	if (D < 0.0f && hasFocused) return *this;
	if (fabs(pD) >= PerpWidth && hasFocused) return *this;
	if (D < m_ClosestD) {
		m_ClosestD = D;
		m_ClosestUI = UI;
	}
	return *this;
}

LWEUINavigation &LWEUINavigation::Update(LWWindow *Window, LWEUIManager &UIManager){
	static const float Inf = 1000000.0f;
	static const float DeadZone = 0.2f;
	if (!isEnabled()) return *this;
	if (UIManager.isTextInputFocused()) return *this;
	if (m_ClosestUI) UIManager.SetFocused(m_ClosestUI);
	m_ClosestUI = nullptr;
	m_ClosestD = Inf;

	LWEUI *FocusedUI = UIManager.GetFocusedUI();
	LWGamePad *Pad = Window->GetGamepadDevice(0);
	LWKeyboard *KB = Window->GetKeyboardDevice();
	LWVector2f Center = Window->GetSizef()*0.5f;
	if (FocusedUI) Center = FocusedUI->GetVisiblePosition() + FocusedUI->GetVisibleSize()*0.5f;
	uint32_t Flag = (m_Flag & ~(Pressed | Back));
	LWVector2f Dir = LWVector2f();
	bool isGamepadEnabled = (m_Flag&GamepadEnabled);
	bool isKeyboardEnabled = (m_Flag&KeyboardEnabled);
	if (KB && isKeyboardEnabled) {
		if (KB->ButtonPressed(LWKey::S) || KB->ButtonPressed(LWKey::Down)) Dir = LWVector2f(0.0f, -1.0f);
		else if (KB->ButtonPressed(LWKey::W) || KB->ButtonPressed(LWKey::Up)) Dir = LWVector2f(0.0f, 1.0f);
		else if (KB->ButtonPressed(LWKey::A) || KB->ButtonPressed(LWKey::Left)) Dir = LWVector2f(-1.0f, 0.0f);
		else if (KB->ButtonPressed(LWKey::D) || KB->ButtonPressed(LWKey::Right)) Dir = LWVector2f(1.0f, 0.0f);
		if (KB->ButtonPressed(LWKey::Return) || KB->ButtonPressed(LWKey::Space)) Flag |= Pressed;
		if (KB->ButtonPressed(LWKey::Back)) Flag |= Back;
	}
	if (Pad && isGamepadEnabled) {
		if (Pad->GetLeftAxis().LengthSquared() > DeadZone) {
			Dir = Pad->GetLeftAxis().Normalize();
		} else {
			if (Pad->ButtonPressed(LWGamePad::Down)) Dir = LWVector2f(0.0f, -1.0f);
			else if (Pad->ButtonPressed(LWGamePad::Up)) Dir = LWVector2f(0.0f, 1.0f);
			else if (Pad->ButtonPressed(LWGamePad::Left)) Dir = LWVector2f(-1.0f, 0.0f);
			else if (Pad->ButtonPressed(LWGamePad::Right)) Dir = LWVector2f(1.0f, 0.0f);
		}
		if (Pad->ButtonPressed(LWGamePad::A)) Flag |= Pressed;
		if (Pad->ButtonPressed(LWGamePad::B)) Flag |= Back;
	}
	m_Center = Center;
	m_Direction = Dir;
	m_PerpDirection = m_Direction.Perpindicular();
	m_Flag = Flag;
	return *this;
}

LWEUIMaterial &LWEUIMaterial::MakeColors(LWVector4f &TLColor, LWVector4f &BLColor, LWVector4f &TRColor, LWVector4f &BRColor) {
	if (m_FillType == FillFull) {
		TLColor = BLColor = TRColor = BRColor = m_ColorA;
	} else if (m_FillType == FillGradient) {
		TLColor = BLColor = m_ColorA;
		TRColor = BRColor = m_ColorB;
	} else if (m_FillType == FillVGradient) {
		TLColor = TRColor = m_ColorB;
		BLColor = BRColor = m_ColorA;
	}
	return *this;
}

LWEUIMaterial &LWEUIMaterial::MakeClippedColors(LWVector4f &TLColor, LWVector4f &BLColor, LWVector4f &TRColor, LWVector4f &BRColor, const LWVector4f &ClipRatios) {
	if (m_FillType == FillFull) {
		TLColor = BLColor = TRColor = BRColor = m_ColorA;
	} else{
		LWVector4f Diff = m_ColorB - m_ColorA;
		if (m_FillType == FillGradient) {
			TLColor = BLColor = m_ColorA + Diff * ClipRatios.x;
			TRColor = BRColor = m_ColorA + Diff * ClipRatios.z;
		} else if (m_FillType == FillVGradient) {
			TLColor = TRColor = m_ColorA + Diff * ClipRatios.y;
			BLColor = BRColor = m_ColorA + Diff * ClipRatios.w;
		}
	}
	return *this;
}


LWEUIMaterial::LWEUIMaterial(const LWVector4f &Color) : m_ColorA(Color) {}

LWEUIMaterial::LWEUIMaterial(const LWVector4f &ColorA, const LWVector4f &ColorB, uint32_t FillType) : m_ColorA(ColorA), m_ColorB(ColorB), m_FillType(FillType) {}

LWEUIMaterial::LWEUIMaterial(const LWVector4f &Color, LWTexture *Tex, const LWVector4f &SubRegion) : m_ColorA(Color), m_Texture(Tex), m_SubRegion(SubRegion) {}

LWEUIMaterial::LWEUIMaterial(const LWVector4f &ColorA, const LWVector4f &ColorB, uint32_t FillType, LWTexture *Tex, const LWVector4f &SubRegion) : m_ColorA(ColorA), m_ColorB(ColorB), m_FillType(FillType), m_Texture(Tex), m_SubRegion(SubRegion) {}

uint32_t LWEUIFrame::SetActiveTexture(LWTexture *Texture, bool FontTexture) {
	if (!m_TextureCount) {
		m_VertexCount[m_TextureCount] = 0;
		m_FontTexture[m_TextureCount] = FontTexture;
		m_Textures[m_TextureCount++] = Texture;
		return m_TextureCount-1;
	}
	uint32_t Active = m_TextureCount - 1;
	if (m_Textures[Active] == Texture && m_FontTexture[Active]==FontTexture) return Active;
	if (m_VertexCount[Active] == 0) {
		m_FontTexture[Active] = FontTexture;
		m_Textures[Active] = Texture;
		return Active;
	}
	if (m_TextureCount >= MaxTextures) return ExhaustedTextures;
	m_VertexCount[m_TextureCount] = 0;
	m_FontTexture[m_TextureCount] = FontTexture;
	m_Textures[m_TextureCount++] = Texture;
	return m_TextureCount-1;
}

bool LWEUIFrame::WriteFontGlyph(LWTexture *Texture, const LWVector2f &Position, const LWVector2f &Size, const LWVector4f &TexCoord, const LWVector2f &SignedDistance, const LWVector4f &Color) {
	uint32_t TexID = 0;
	if ((TexID = SetActiveTexture(Texture, true)) == ExhaustedTextures) return false;
	if (!m_Mesh->CanWriteVertices(6)) return false;
	LWVertexUI *V = m_Mesh->GetVertexAt(m_Mesh->WriteVertices(6));
	LWVector2f BtmLeft = Position;
	LWVector2f TopLeft = Position + LWVector2f(0.0f, Size.y);
	LWVector2f BtmRight = Position + LWVector2f(Size.x, 0.0f);
	LWVector2f TopRight = Position + Size;

	LWVector2f BtmLeftTC = LWVector2f(TexCoord.x, TexCoord.w);
	LWVector2f TopLeftTC = LWVector2f(TexCoord.x, TexCoord.y);
	LWVector2f BtmRightTC = LWVector2f(TexCoord.z, TexCoord.w);
	LWVector2f TopRightTC = LWVector2f(TexCoord.z, TexCoord.y);

	*(V + 0) = { LWVector4f(BtmLeft, 0.0f, 1.0f), Color, LWVector4f(BtmLeftTC, SignedDistance) };
	*(V + 1) = { LWVector4f(TopRight, 0.0f, 1.0f), Color, LWVector4f(TopRightTC, SignedDistance) };
	*(V + 2) = { LWVector4f(TopLeft, 0.0f, 1.0f), Color, LWVector4f(TopLeftTC, SignedDistance) };
	*(V + 3) = { LWVector4f(BtmLeft, 0.0f, 1.0f), Color, LWVector4f(BtmLeftTC, SignedDistance) };
	*(V + 4) = { LWVector4f(BtmRight, 0.0f, 1.0f), Color, LWVector4f(BtmRightTC, SignedDistance) };
	*(V + 5) = { LWVector4f(TopRight, 0.0f, 1.0f), Color, LWVector4f(TopRightTC, SignedDistance) };
	m_VertexCount[TexID] += 6;
	return true;
}

bool LWEUIFrame::MakeClipRatios(LWVector4f &RatioRes, const LWVector2f &Pos, const LWVector2f &Size, const LWVector4f &AABB) {
	float Right = Pos.x + Size.x;
	float Top = Pos.y + Size.y;
	if (Pos.x >= (AABB.x + AABB.z) || Top < AABB.y || Right < AABB.x || Pos.y >= (AABB.y + AABB.w)) return false;
	RatioRes.x = std::max<float>((AABB.x - Pos.x) / Size.x, 0.0f);
	RatioRes.z = std::min<float>(((AABB.x + AABB.z) - Pos.x) / Size.x, 1.0f);
	RatioRes.y = std::max<float>((AABB.y - Pos.y) / Size.y, 0.0f);
	RatioRes.w = std::min<float>(((AABB.y + AABB.w) - Pos.y) / Size.y, 1.0f);
	return true;
}

LWEUIFrame &LWEUIFrame::ApplyClipRatios(LWVector2f &TopLeft, LWVector2f &BtmLeft, LWVector2f &TopRight, LWVector2f &BtmRight, const LWVector2f &Pos, const LWVector2f &Size, const LWVector4f &Ratio) {

	float Left = Pos.x + Size.x*Ratio.x;
	float Right = Pos.x + Size.x*Ratio.z;
	float Btm = Pos.y + Size.y*Ratio.w;
	float Top = Pos.y + Size.y*Ratio.y;

	TopLeft = LWVector2f(Left, Top);
	BtmLeft = LWVector2f(Left, Btm);
	TopRight = LWVector2f(Right, Top);
	BtmRight = LWVector2f(Right, Btm);
	return *this;
}

bool LWEUIFrame::WriteClippedRect(LWEUIMaterial *Mat, const LWVector2f &Pos, const LWVector2f &Size, const LWVector4f &AABB) {
	uint32_t TexID = 0;
	LWVector4f Ratio = LWVector4f();
	if (!Mat) return false;
	if (!m_Mesh->CanWriteVertices(6)) return false;
	if (!MakeClipRatios(Ratio, Pos, Size, AABB)) return false;
	if ((TexID = SetActiveTexture(Mat->m_Texture, false)) == ExhaustedTextures) return false;
	LWVector4f SubRegion = Mat->m_SubRegion;
	LWVector2f TexPos = SubRegion.xy();
	LWVector2f TexSize = SubRegion.zw() - TexPos;

	LWVector2f TL, TR, BL, BR;
	LWVector2f TLTex, TRTex, BLTex, BRTex;
	LWVector4f TLClr, TRClr, BLClr, BRClr;

	ApplyClipRatios(TL, BL, TR, BR, Pos, Size, Ratio);
	ApplyClipRatios(TLTex, BLTex, TRTex, BRTex, TexPos, TexSize, Ratio);
	Mat->MakeClippedColors(TLClr, BLClr, TRClr, BRClr, Ratio);

	LWVertexUI *V = m_Mesh->GetVertexAt(m_Mesh->WriteVertices(6));
	*(V + 0) = { LWVector4f(TL, 0.0f, 1.0f), TLClr, LWVector4f(BLTex, 0.0f, 0.0f) };
	*(V + 1) = { LWVector4f(BR, 0.0f, 1.0f), BRClr, LWVector4f(TRTex, 0.0f, 0.0f) };
	*(V + 2) = { LWVector4f(BL, 0.0f, 1.0f), BLClr, LWVector4f(TLTex, 0.0f, 0.0f) };

	*(V + 3) = { LWVector4f(TL, 0.0f, 1.0f), TLClr, LWVector4f(BLTex, 0.0f, 0.0f) };
	*(V + 4) = { LWVector4f(TR, 0.0f, 1.0f), TRClr, LWVector4f(BRTex, 0.0f, 0.0f) };
	*(V + 5) = { LWVector4f(BR, 0.0f, 1.0f), BRClr, LWVector4f(TRTex, 0.0f, 0.0f) };
	m_VertexCount[TexID] += 6;
	return true;
};

bool LWEUIFrame::WriteClippedText(LWEUIMaterial *Mat, const LWText &Text, LWFont *Fnt, const LWVector2f &Pos, float Scale, const LWVector4f &AABB) {
	uint32_t TexID = 0;
	if (!Fnt) return false;
	LWVector4f Clr = Mat ? Mat->m_ColorA : LWVector4f(0.0f, 0.0f, 0.0f, 1.0f);
	Fnt->DrawClippedTextm(Text, Pos, Scale, Clr, AABB, this, &LWEUIFrame::WriteFontGlyph);
	return true;
};

bool LWEUIFrame::WriteRect(LWEUIMaterial *Mat, const LWVector2f &Pos, const LWVector2f &Size) {
	uint32_t TexID = 0;
	if (!Mat) return false;
	if (!m_Mesh->CanWriteVertices(6)) return false;
	if ((TexID = SetActiveTexture(Mat->m_Texture, false)) == ExhaustedTextures) return false;

	LWVector4f TLClr, BLClr, TRClr, BRClr;
	Mat->MakeColors(TLClr, BLClr, TRClr, BRClr);

	LWVector2f TL = LWVector2f(Pos.x, Pos.y + Size.y);
	LWVector2f TR = LWVector2f(Pos.x + Size.x, Pos.y + Size.y);
	LWVector2f BL = LWVector2f(Pos.x, Pos.y);
	LWVector2f BR = LWVector2f(Pos.x + Size.x, Pos.y);

	LWVector4f SubRegion = Mat->m_SubRegion;
	LWVector2f TLTex = SubRegion.xy();
	LWVector2f BLTex = LWVector2f(SubRegion.x, SubRegion.w);
	LWVector2f TRTex = LWVector2f(SubRegion.z, SubRegion.y);
	LWVector2f BRTex = SubRegion.zw();

	LWVertexUI *V = m_Mesh->GetVertexAt(m_Mesh->WriteVertices(6));
	*(V + 0) = { LWVector4f(TL, 0.0f, 1.0f), TLClr, LWVector4f(TLTex, 0.0f, 0.0f) };
	*(V + 1) = { LWVector4f(BL, 0.0f, 1.0f), BLClr, LWVector4f(BLTex, 0.0f, 0.0f) };
	*(V + 2) = { LWVector4f(BR, 0.0f, 1.0f), BRClr, LWVector4f(BRTex, 0.0f, 0.0f) };

	*(V + 3) = { LWVector4f(BR, 0.0f, 1.0f), BRClr, LWVector4f(BRTex, 0.0f, 0.0f) };
	*(V + 4) = { LWVector4f(TR, 0.0f, 1.0f), TRClr, LWVector4f(TRTex, 0.0f, 0.0f) };
	*(V + 5) = { LWVector4f(TL, 0.0f, 1.0f), TLClr, LWVector4f(TLTex, 0.0f, 0.0f) };
	m_VertexCount[TexID] += 6;
	return true;
}

bool LWEUIFrame::WriteRect(LWEUIMaterial *Mat, const LWVector2f &Pos, const LWVector2f &Size, float Theta) {
	uint32_t TexID = 0;
	if (!Mat) return false;
	if (!m_Mesh->CanWriteVertices(6)) return false;
	if ((TexID = SetActiveTexture(Mat->m_Texture, false)) == ExhaustedTextures) return false;

	LWVector4f TLClr, BLClr, TRClr, BRClr;
	Mat->MakeColors(TLClr, BLClr, TRClr, BRClr);


	LWVector2f hSize = Size * 0.5f;
	LWVector2f Rot = LWVector2f::MakeTheta(Theta);
	LWVector2f TL = Rot.Rotate(LWVector2f(-hSize.x,-hSize.y)) + Pos + hSize;
	LWVector2f TR = Rot.Rotate(LWVector2f( hSize.x,-hSize.y)) + Pos + hSize;
	LWVector2f BL = Rot.Rotate(LWVector2f(-hSize.x, hSize.y)) + Pos + hSize;
	LWVector2f BR = Rot.Rotate(LWVector2f( hSize.x, hSize.y)) + Pos + hSize;

	LWVector4f SubRegion = Mat->m_SubRegion;
	LWVector2f TLTex = SubRegion.xy();
	LWVector2f BLTex = LWVector2f(SubRegion.x, SubRegion.w);
	LWVector2f TRTex = LWVector2f(SubRegion.z, SubRegion.y);
	LWVector2f BRTex = SubRegion.zw();

	LWVertexUI *V = m_Mesh->GetVertexAt(m_Mesh->WriteVertices(6));
	*(V + 0) = { LWVector4f(TL, 0.0f, 1.0f), TLClr, LWVector4f(TLTex, 0.0f, 0.0f) };
	*(V + 1) = { LWVector4f(BL, 0.0f, 1.0f), BLClr, LWVector4f(BLTex, 0.0f, 0.0f) };
	*(V + 2) = { LWVector4f(BR, 0.0f, 1.0f), BRClr, LWVector4f(BRTex, 0.0f, 0.0f) };

	*(V + 3) = { LWVector4f(BR, 0.0f, 1.0f), BRClr, LWVector4f(BRTex, 0.0f, 0.0f) };
	*(V + 4) = { LWVector4f(TR, 0.0f, 1.0f), TRClr, LWVector4f(TRTex, 0.0f, 0.0f) };
	*(V + 5) = { LWVector4f(TL, 0.0f, 1.0f), TLClr, LWVector4f(TLTex, 0.0f, 0.0f) };

	m_VertexCount[TexID] += 6;
	return true;
}

bool LWEUIFrame::WriteLine(LWEUIMaterial *Mat, const LWVector2f &APos, const LWVector2f &BPos, float Thickness) {
	uint32_t TexID = 0;
	if (!Mat) return false;
	if (!m_Mesh->CanWriteVertices(6)) return false;
	if ((TexID = SetActiveTexture(Mat->m_Texture, false)) == ExhaustedTextures) return false;
	LWVector2f Dir = BPos - APos;
	LWVector2f nDir = Dir.Normalize();
	LWVector2f Perp = nDir.Perpindicular()*Thickness;

	LWVector2f TL = APos + Perp;
	LWVector2f BL = APos - Perp;
	LWVector2f TR = BPos + Perp;
	LWVector2f BR = BPos - Perp;

	LWVector4f SubRegion = Mat->m_SubRegion;
	LWVector2f TLTex = SubRegion.xy();
	LWVector2f BLTex = LWVector2f(SubRegion.x, SubRegion.w);
	LWVector2f TRTex = LWVector2f(SubRegion.z, SubRegion.y);
	LWVector2f BRTex = SubRegion.zw();

	LWVector4f TLClr, BLClr, TRClr, BRClr;
	Mat->MakeColors(TLClr, BLClr, TRClr, BRClr);

	LWVertexUI *V = m_Mesh->GetVertexAt(m_Mesh->WriteVertices(6));
	*(V + 0) = { LWVector4f(TL, 0.0f, 1.0f), TLClr, LWVector4f(TLTex, 0.0f, 0.0f) };
	*(V + 1) = { LWVector4f(BL, 0.0f, 1.0f), BLClr, LWVector4f(BLTex, 0.0f, 0.0f) };
	*(V + 2) = { LWVector4f(BR, 0.0f, 1.0f), BRClr, LWVector4f(BRTex, 0.0f, 0.0f) };

	*(V + 3) = { LWVector4f(BR, 0.0f, 1.0f), BRClr, LWVector4f(BRTex, 0.0f, 0.0f) };
	*(V + 4) = { LWVector4f(TR, 0.0f, 1.0f), TRClr, LWVector4f(TRTex, 0.0f, 0.0f) };
	*(V + 5) = { LWVector4f(TL, 0.0f, 1.0f), TLClr, LWVector4f(TLTex, 0.0f, 0.0f) };
	m_VertexCount[TexID] += 6;
	return true;
}

bool LWEUIFrame::WriteClippedLine(LWEUIMaterial *Mat, const LWVector2f &APos, const LWVector2f &BPos, float Thickness, const LWVector4f &AABB) {
	uint32_t TexID = 0;
	if (!Mat) return false;
	if (!m_Mesh->CanWriteVertices(6)) return false;
	if ((TexID = SetActiveTexture(Mat->m_Texture, false)) == ExhaustedTextures) return false;
	LWVector2f AAMin = AABB.xy();
	LWVector2f AAMax = AAMin + AABB.zw();
	float Min = 0.0f;
	float Max = 1.0f;
	if (!LWEPointInsideAABB(APos, AAMin, AAMax) || !LWEPointInsideAABB(BPos, AAMin, AAMax)) {
		if (!LWERayAABBIntersect(APos, BPos, AAMin, AAMax, &Min, &Max)) return false;
		Min = std::max<float>(Min, 0.0f);
		Max = std::min<float>(Max, 1.0f);
	}
	LWVector4f Ratio = LWVector4f(Min, 0.0f, Max, 1.0f);
	LWVector2f Dir = BPos - APos;
	LWVector2f nDir = Dir.Normalize();
	LWVector2f A = APos + Dir * Min;
	LWVector2f B = APos + Dir * Max;

	LWVector2f Perp = nDir.Perpindicular()*Thickness;

	LWVector2f TL = A + Perp;
	LWVector2f BL = A - Perp;
	LWVector2f TR = B + Perp;
	LWVector2f BR = B - Perp;

	LWVector4f TLClr, BLClr, TRClr, BRClr;
	Mat->MakeClippedColors(TLClr, BLClr, TRClr, BRClr, Ratio);

	LWVector4f SubRegion = Mat->m_SubRegion;
	LWVector2f TexPos = SubRegion.xy();
	LWVector2f TexSize = SubRegion.zw() - TexPos;
	LWVector2f TLTex, BLTex, TRTex, BRTex;
	ApplyClipRatios(TLTex, BLTex, TRTex, BRTex, TexPos, TexSize, Ratio);

	LWVertexUI *V = m_Mesh->GetVertexAt(m_Mesh->WriteVertices(6));
	*(V + 0) = { LWVector4f(TL, 0.0f, 1.0f), TLClr, LWVector4f(TLTex, 0.0f, 0.0f) };
	*(V + 1) = { LWVector4f(BL, 0.0f, 1.0f), BLClr, LWVector4f(BLTex, 0.0f, 0.0f) };
	*(V + 2) = { LWVector4f(BR, 0.0f, 1.0f), BRClr, LWVector4f(BRTex, 0.0f, 0.0f) };

	*(V + 3) = { LWVector4f(BR, 0.0f, 1.0f), BRClr, LWVector4f(BRTex, 0.0f, 0.0f) };
	*(V + 4) = { LWVector4f(TR, 0.0f, 1.0f), TRClr, LWVector4f(TRTex, 0.0f, 0.0f) };
	*(V + 5) = { LWVector4f(TL, 0.0f, 1.0f), TLClr, LWVector4f(TLTex, 0.0f, 0.0f) };
	m_VertexCount[TexID] += 6;
	return true;
}

uint32_t LWEUIFrame::WriteVertices(uint32_t VertexCount) {
	if (!m_TextureCount) return 0xFFFFFFFF;
	if (!m_Mesh->CanWriteVertices(VertexCount)) return 0xFFFFFFFF;
	uint32_t Active = m_TextureCount - 1;
	m_VertexCount[Active] += VertexCount;

	return m_Mesh->WriteVertices(VertexCount);
}

LWEUIFrame &LWEUIFrame::operator=(LWEUIFrame &&F) {
	m_Mesh = F.m_Mesh;
	m_TextureCount = F.m_TextureCount;
	for (uint32_t i = 0; i < m_TextureCount; i++) {
		m_Textures[i] = F.m_Textures[i];
		m_FontTexture[i] = F.m_FontTexture[i];
		m_VertexCount[i] = F.m_VertexCount[i];
	}
	return *this;
}

LWEUIFrame &LWEUIFrame::operator=(LWEUIFrame &F) {
	m_Mesh = F.m_Mesh;
	m_TextureCount = F.m_TextureCount;
	for (uint32_t i = 0; i < m_TextureCount; i++) {
		m_Textures[i] = F.m_Textures[i];
		m_FontTexture[i] = F.m_FontTexture[i];
		m_VertexCount[i] = F.m_VertexCount[i];
	}
	return *this;
}

LWEUIFrame::LWEUIFrame(LWEUIFrame &&F) : m_Mesh(F.m_Mesh), m_TextureCount(F.m_TextureCount) {
	for (uint32_t i = 0; i < m_TextureCount; i++) {
		m_Textures[i] = F.m_Textures[i];
		m_FontTexture[i] = F.m_FontTexture[i];
		m_VertexCount[i] = F.m_VertexCount[i];
	}
}

LWEUIFrame::LWEUIFrame(LWEUIFrame &F) : m_Mesh(F.m_Mesh), m_TextureCount(F.m_TextureCount) {
	for (uint32_t i = 0; i < m_TextureCount; i++) {
		m_Textures[i] = F.m_Textures[i];
		m_FontTexture[i] = F.m_FontTexture[i];
		m_VertexCount[i] = F.m_VertexCount[i];
	}
}

LWEUIFrame::LWEUIFrame(LWMesh<LWVertexUI> *Mesh) : m_Mesh(Mesh), m_TextureCount(0) {}

LWEUIFrame::LWEUIFrame() : m_Mesh(nullptr), m_TextureCount(0) {}

const char *LWEUIManager::GetVertexShaderSource(void) {
	static const char VertexSource[] = ""\
		"#module Vertex DirectX11_1\n"\
		"cbuffer UIUniform{\n"\
		"	float4x4 Matrix;\n"\
		"};\n"\
		"struct Vertex {\n"\
		"	float4 Position : POSITION;\n"\
		"	float4 Color : COLOR;\n"\
		"	float4 TexCoord : TEXCOORD;\n"\
		"};\n"\
		"struct Pixel {\n"\
		"	float4 Position : SV_POSITION;\n"\
		"	float4 Color : COLOR0;\n"\
		"	float4 TexCoord : TEXCOORD0;\n"\
		"};\n"\
		"Pixel main(Vertex In) {\n"\
		"	Pixel O;\n"\
		"	O.Position = mul(Matrix, In.Position);\n"\
		"	O.Color = In.Color;\n"\
		"	O.TexCoord = In.TexCoord;\n"\
		"	return O;\n"\
		"}\n"\
		"#module Vertex OpenGL3_3 OpenGL4_5\n"\
		"#version 330\n"\
		"layout(std140) uniform UIUniform {\n"\
		"	mat4 Matrix;\n"\
		"};\n"\
		"in vec4 Position | 0;\n"\
		"in vec4 Color | 1;\n"\
		"in vec4 TexCoord | 2;\n"\
		"out vec4 pColor;\n"\
		"out vec4 pTexCoord;\n"\
		"void main(void) {\n"\
		"	gl_Position = Matrix*Position;\n"\
		"	pColor = Color;\n"\
		"	pTexCoord = TexCoord;\n"\
		"}\n"\
		"#module Vertex OpenGL2_1\n"\
		"attribute vec4 Position;\n"\
		"attribute vec4 Color;\n"\
		"attribute vec4 TexCoord;\n"\
		"varying vec4 pColor;\n"\
		"varying vec4 pTexCoord;\n"\
		"#block UIUniform\n"\
		"uniform mat4 Matrix;\n"\
		"void main(void) {\n"\
		"	gl_Position = Matrix*Position;\n"\
		"	pColor = Color;\n"\
		"	pTexCoord = TexCoord;\n"\
		"}\n"\
		"#module Vertex OpenGLES2\n"\
		"attribute highp vec4 Position;\n"\
		"attribute lowp vec4 Color;\n"\
		"attribute lowp vec4 TexCoord;\n"\
		"varying lowp vec4 pColor;\n"\
		"varying lowp vec4 pTexCoord;\n"\
		"#block UIUniform\n"\
		"uniform highp mat4 Matrix;\n"\
		"void main(void) {\n"\
		"	gl_Position = Matrix*Position;\n"\
		"	pColor = Color;\n"\
		"	pTexCoord = TexCoord;\n"\
		"}\n";
	return VertexSource;
}

const char *LWEUIManager::GetTextureShaderSource(void) {
	static const char TextureSource[] = ""\
		"#module Pixel DirectX11_1\n"\
		"struct Pixel {\n"\
		"	float4 Position : SV_POSITION;\n"\
		"	float4 Color : COLOR0;\n"\
		"	float4 TexCoord : TEXCOORD0;\n"\
		"};\n"\
		"Texture2D Tex;\n"\
		"SamplerState TexSampler;\n"\
		"float4 main(Pixel In) : SV_TARGET{\n"\
		"	return In.Color*Tex.Sample(TexSampler, In.TexCoord.xy);\n"\
		"}\n"\
		"#module Pixel OpenGL3_3 OpenGL4_5\n"\
		"#version 330\n"\
		"uniform sampler2D Tex;\n"\
		"in vec4 pColor;\n"\
		"in vec4 pTexCoord;\n"\
		"out vec4 p_Color;\n"\
		"void main(void) {\n"\
		"	p_Color = texture(Tex, pTexCoord.xy)*pColor;\n"\
		"}\n"\
		"#module Pixel OpenGL2_1\n"\
		"uniform sampler2D Tex;\n"\
		"varying vec4 pColor;\n"\
		"varying vec4 pTexCoord;\n"\
		"void main(void) {\n"\
		"	gl_FragColor = texture2D(Tex, pTexCoord.xy)*pColor;\n"\
		"}\n"\
		"#module Pixel OpenGLES2\n"\
		"uniform sampler2D Tex;\n"\
		"varying lowp vec4 pColor;\n"\
		"varying lowp vec4 pTexCoord;\n"\
		"void main(void) {\n"\
		"	gl_FragColor = texture2D(Tex, pTexCoord.xy)*pColor;\n"\
		"}\n";
		return TextureSource;
}

const char *LWEUIManager::GetColorShaderSource(void) {
	static const char ColorSource[] = ""\
		"#module Pixel DirectX11_1\n"\
		"struct Pixel {\n"\
		"	float4 Position : SV_POSITION;\n"\
		"	float4 Color : COLOR0;\n"\
		"	float4 TexCoord : TEXCOORD0;\n"\
		"};\n"\
		"float4 main(Pixel In) : SV_TARGET{\n"\
		"	return In.Color;\n"\
		"}\n"\
		"#module Pixel OpenGL3_3 OpenGL4_5\n"\
		"#version 330\n"\
		"in vec4 pColor;\n"\
		"in vec4 pTexCoord;\n"\
		"out vec4 p_Color;\n"\
		"void main(void) {\n"\
		"	p_Color = pColor;\n"\
		"}\n"\
		"#module Pixel OpenGL2_1\n"\
		"varying vec4 pColor;\n"\
		"varying vec4 pTexCoord;\n"\
		"void main(void) {\n"\
		"	gl_FragColor = pColor;\n"\
		"}\n"\
		"#module Pixel OpenGLES2\n"\
		"varying lowp vec4 pColor;\n"\
		"varying lowp vec4 pTexCoord;\n"\
		"void main(void) {\n"\
		"	gl_FragColor = pColor;\n"\
		"}\n";
	return ColorSource;
}

const char *LWEUIManager::GetYUVTextureShaderSource(void) {
	static const char YUVSource[] = ""\
	"#module Pixel DirectX11_1\n"\
	"	struct Pixel {\n"\
	"	float4 Position : SV_POSITION;\n"\
	"	float4 Color : COLOR0;\n"\
	"	float4 TexCoordA : TEXCOORD0;\n"\
	"	float4 TexCoordB : TEXCOORD1;\n"\
	"};\n"\
	"Texture2D Tex;\n"\
	"SamplerState TexSampler;\n"\
	"float4 DecodeYUV(float2 TexCoordY, float2 TexCoordU, float2 TexCoordV) {\n"\
	"	float Y = Tex.Sample(TexSampler, TexCoordY).r;\n"\
	"	float U = Tex.Sample(TexSampler, TexCoordU).r - 0.5f;\n"\
	"	float V = Tex.Sample(TexSampler, TexCoordV).r - 0.5f;\n"\
	"	float R = Y + 1.402f*V;\n"\
	"	float G = Y - 0.344f*U - 0.714f*V;\n"\
	"	float B = Y + 1.722f*U;\n"\
	"	return float4(R, G, B, 1.0f);\n"\
	"}\n"\
	"float4 main(Pixel In) : SV_TARGET{\n"\
	"	return DecodeYUV(In.TexCoordA.xy, In.TexCoordA.zw, In.TexCoordB.xy)*In.Color;\n"\
	"}\n"\
	"#module Pixel OpenGL3_3 OpenGL4_5\n"\
	"#version 330\n"\
	"in vec4 pColor;\n"\
	"in vec4 pTexCoordA;\n"\
	"in vec4 pTexCoordB;\n"\
	"uniform sampler2D Tex;\n"\
	"out vec4 p_Color | 0 | Output;\n"\
	"vec4 DecodeYUV(vec2 TexCoordY, vec2 TexCoordU, vec2 TexCoordV) {\n"\
	"	float Y = texture2D(Tex, TexCoordY).r;\n"\
	"	float U = texture2D(Tex, TexCoordU).r - 0.5f;\n"\
	"	float V = texture2D(Tex, TexCoordV).r - 0.5f;\n"\
	"	float R = Y + 1.402f*V;\n"\
	"	float G = Y - 0.344f*U - 0.714f*V;\n"\
	"	float B = Y + 1.722f*U;\n"\
	"	return vec4(R, G, B, 1.0f);\n"\
	"}\n"\
	"void main(void) {\n"\
	"	p_Color = DecodeYUV(pTexCoordA.xy, pTexCoordA.zw, pTexCoordB.xy)*pColor;\n"\
	"}\n"\
	"#module Pixel OpenGL2_1\n"\
	"varying vec4 pColor;\n"\
	"varying vec4 pTexCoordA;\n"\
	"varying vec4 pTexCoordB;\n"\
	"uniform sampler2D Tex;\n"\
	"vec4 DecodeYUV(vec2 TexCoordY, vec2 TexCoordU, vec2 TexCoordV) {\n"\
	"	float Y = texture(Tex, TexCoordY).r;\n"\
	"	float U = texture(Tex, TexCoordU).r - 0.5f;\n"\
	"	float V = texture(Tex, TexCoordV).r - 0.5f;\n"\
	"	float R = Y + 1.402f*V;\n"\
	"	float G = Y - 0.344f*U - 0.714f*V;\n"\
	"	float B = Y + 1.722f*U;\n"\
	"	return vec4(R, G, B, 1.0f);\n"\
	"}\n"\
	"void main(void) {\n"\
	"	gl_FragColor = DecodeYUV(pTexCoordA.xy, pTexCoordA.zw, pTexCoordB.xy)*pColor;\n"\
	"}\n"\
	"#module Pixel OpenGLES2\n"\
	"varying lowp vec4 pColor;\n"\
	"varying lowp vec4 pTexCoordA;\n"\
	"varying lowp vec4 pTexCoordB;\n"\
	"uniform sampler2D Tex;\n"\
	"vec4 DecodeYUV(vec2 TexCoordY, vec2 TexCoordU, vec2 TexCoordV) {\n"\
	"	float Y = texture(Tex, TexCoordY).r;\n"\
	"	float U = texture(Tex, TexCoordU).r - 0.5f;\n"\
	"	float V = texture(Tex, TexCoordV).r - 0.5f;\n"\
	"	float R = Y + 1.402f*V;\n"\
	"	float G = Y - 0.344f*U - 0.714f*V;\n"\
	"	float B = Y + 1.722f*U;\n"\
	"	return vec4(R, G, B, 1.0f);\n"\
	"}\n"\
	"void main(void) {\n"\
	"	gl_FragColor = DecodeYUV(pTexCoordA.xy, pTexCoordA.zw, pTexCoordB.xy)*pColor;\n"\
	"}\n";
	return YUVSource;
}

bool LWEUIManager::XMLParser(LWEXMLNode *Node, void *UserData, LWEXML *X) {
	std::map<uint32_t, LWEXMLNode*> StyleMap;
	std::map<uint32_t, LWEXMLNode*> ComponentMap;
	uint32_t StyleCount = 0;
	LWEUIManager *Manager = (LWEUIManager*)UserData;

	auto ParseColor = [](LWXMLAttribute *ColorAttr)->LWVector4f {
		LWVector4f Color;
		const char *C = LWText::NextWord(ColorAttr->m_Value, true);
		if (*C == '#') {
			uint32_t Val = 0;
			sscanf(C, "#%x", &Val);
			Color.x = (float)((Val >> 24) & 0xFF) / 255.0f;
			Color.y = (float)((Val >> 16) & 0xFF) / 255.0f;
			Color.z = (float)((Val >> 8) & 0xFF) / 255.0f;
			Color.w = (float)((Val) & 0xFF) / 255.0f;
		} else sscanf(ColorAttr->m_Value, "%f|%f|%f|%f", &Color.x, &Color.y, &Color.z, &Color.w);
		return Color;
	};

	auto ParseMaterial = [&ParseColor](LWEXMLNode *Node, LWEUIManager *Man)->bool {
		const char *FillNames[] = { "Full","Gradient", "VGradient" };
		uint32_t FillValues[] = { LWEUIMaterial::FillFull, LWEUIMaterial::FillGradient, LWEUIMaterial::FillVGradient };
		uint32_t FillValueCnt = sizeof(FillValues) / sizeof(uint32_t);

		LWXMLAttribute *NameAttr = Node->FindAttribute("Name");
		LWXMLAttribute *ColorAttr = Node->FindAttribute("Color");
		LWXMLAttribute *ColorAAttr = Node->FindAttribute("ColorA");
		LWXMLAttribute *ColorBAttr = Node->FindAttribute("ColorB");
		LWXMLAttribute *FillAttr = Node->FindAttribute("Fill");
		LWXMLAttribute *TexAttr = Node->FindAttribute("Texture");
		LWXMLAttribute *SubRegionAttr = Node->FindAttribute("SubRegion");
		if (!NameAttr) return false;
		LWVector4f ColorA = LWVector4f(1.0f);
		LWVector4f ColorB = LWVector4f(1.0f);
		LWTexture *Tex = nullptr;
		LWVector4f SubRegion = LWVector4f(0.0f, 0.0f, 1.0f, 1.0f);
		uint32_t FillMode = LWEUIMaterial::FillFull;
		if (ColorAttr) ColorA = ParseColor(ColorAttr);
		if (ColorAAttr) ColorA = ParseColor(ColorAAttr);
		if (ColorBAttr) ColorB = ParseColor(ColorBAttr);
		if (FillAttr) {
			const char *C = LWText::NextWord(FillAttr->m_Value, true);
			uint32_t FillType = LWText::CompareMultiplea(C, FillValueCnt, FillNames);
			if (FillType == -1) {
				std::cout << "Material has unknown fill type: '" << FillAttr->m_Value << "'" << std::endl;
			} else {
				FillMode = FillType;
			}
		}
		if (TexAttr) Tex = Man->GetAssetManager()->GetAsset<LWTexture>(TexAttr->m_Value);
		if (SubRegionAttr && Tex) {
			LWVector2i TexSize = Tex->Get2DSize();
			LWVector4i Region = LWVector4i(0, 0, TexSize);
			sscanf(SubRegionAttr->m_Value, "%d|%d|%d|%d", &Region.x, &Region.y, &Region.z, &Region.w);
			SubRegion.x = ((float)Region.x + 0.5f) / (float)TexSize.x;
			SubRegion.y = ((float)Region.y + 0.5f) / (float)TexSize.y;
			SubRegion.z = ((float)(Region.x +Region.z) - 0.5f) / (float)TexSize.x;
			SubRegion.w = ((float)(Region.y + Region.w) - 0.5f) / (float)TexSize.y;
		}
		return Man->InsertMaterial(NameAttr->m_Value, ColorA, ColorB, FillMode, Tex, SubRegion) != nullptr;
	};

	auto ParseStyle = [](LWEXMLNode *Node, LWEUIManager *Man, std::map<uint32_t, LWEXMLNode*> &StyleMap) {
		const uint32_t NameHash = LWText::MakeHash("Name");
		const uint32_t StyleHash = LWText::MakeHash("Style");
		LWXMLAttribute *NameAttr = Node->FindAttribute("Name");
		if (!NameAttr) return;
		StyleMap.emplace(LWText::MakeHash(NameAttr->m_Value), Node);	
		LWXMLAttribute *StyleAttr = Node->FindAttribute("Style");
		if (!StyleAttr) return;
		auto Iter = StyleMap.find(LWText::MakeHash(StyleAttr->m_Value));
		Node->RemoveAttribute(StyleAttr);
		if (Iter == StyleMap.end()) return;
		LWEXMLNode *N = Iter->second;
		for (uint32_t i = 0; i < N->m_AttributeCount; i++) {
			LWXMLAttribute &Attr = N->m_Attributes[i];
			uint32_t AttrHash = LWText::MakeHash(Attr.m_Name);
			if (AttrHash == NameHash || AttrHash == StyleHash) continue;
			if (Node->FindAttribute(Attr.m_Name)) continue;
			Node->PushAttribute(Attr.m_Name, Attr.m_Value);
		}
		return;
	};

	auto ParseUIDPIScale = [](LWEXMLNode *Node, LWEUIManager *Man) {
		LWXMLAttribute *DPIAttr = Node->FindAttribute("DPI");
		LWXMLAttribute *ScaleAttr = Node->FindAttribute("Scale");
		if (!DPIAttr || !ScaleAttr) return;
		uint32_t DPI = (uint32_t)atoi(DPIAttr->m_Value);
		float s = (float)atof(ScaleAttr->m_Value);
		Man->PushDPIScale(DPI, s);
		return;
	};

	auto ParseUIResScale = [](LWEXMLNode *Node, LWEUIManager *Man) {

		LWXMLAttribute *WidthAttr = Node->FindAttribute("Width");
		LWXMLAttribute *HeightAttr = Node->FindAttribute("Height");
		LWXMLAttribute *ScaleAttr = Node->FindAttribute("Scale");
		if (!WidthAttr || !HeightAttr || !ScaleAttr) return;
		int32_t w = atoi(WidthAttr->m_Value);
		int32_t h = atoi(HeightAttr->m_Value);
		float s = (float)atof(ScaleAttr->m_Value);
		Man->PushScreenScale(LWVector2i(w, h), s);
		return;
	};

	auto ParseComponent = [](LWEXMLNode *Node, LWEUIManager *Man, std::map<uint32_t, LWEXMLNode*> &ComponentMap) {
		LWXMLAttribute *NameAttr = Node->FindAttribute("Name");
		if (!NameAttr) return;
		ComponentMap.insert(std::pair<uint32_t, LWEXMLNode*>(LWText::MakeHash(NameAttr->m_Value), Node));
		return;
	};

	auto ParseInclude = [](LWEXMLNode *Node, LWEXMLNode *Parent, LWEUIManager *Man, LWEXML *X) {
		LWXMLAttribute *SrcAttr = Node->FindAttribute("Src");
		if (!SrcAttr) {
			SrcAttr = Node->FindAttribute("Source");
			if(!SrcAttr) return;
		}
		if (!LWEXML::LoadFile(*X, *Man->GetAllocator(), SrcAttr->m_Value, true, Parent, Node)) {
			std::cout << "Error reading include file: '" << SrcAttr->m_Value << "'" << std::endl;
			return;
		}
		return;
	};

	auto ParseTooltip = [](LWEXMLNode *Node, LWEUIManager *Man, LWEXML *X) {
		LWXMLAttribute *FontAttr = Node->FindAttribute("Font");
		LWXMLAttribute *FontMatAttr = Node->FindAttribute("FontMaterial");
		LWXMLAttribute *BorderMatAttr = Node->FindAttribute("BorderMaterial");
		LWXMLAttribute *BackgroundMatAttr = Node->FindAttribute("BackgroundMaterial");
		LWXMLAttribute *BorderSizeAttr = Node->FindAttribute("BorderSize");
		LWXMLAttribute *FontScaleAttr = Node->FindAttribute("FontScale");
		LWEUITooltip &TT = Man->GetTooltipDecoration();
		if (FontAttr) TT.m_Font = Man->GetAssetManager()->GetAsset<LWFont>(FontAttr->m_Value);
		if (FontMatAttr) TT.m_FontMaterial = Man->GetMaterial(FontMatAttr->m_Value);
		if (BorderMatAttr) TT.m_BorderMaterial = Man->GetMaterial(BorderMatAttr->m_Value);
		if (BackgroundMatAttr) TT.m_BackgroundMaterial = Man->GetMaterial(BackgroundMatAttr->m_Value);
		if (BorderSizeAttr) TT.m_BorderSize = (float)atof(BorderSizeAttr->m_Value);
		if (FontScaleAttr) TT.m_FontScale = (float)atof(FontScaleAttr->m_Value);
		return;
	};

	for (LWEXMLNode *C = X->NextNode(nullptr, Node); C; C = X->NextNode(C, Node, true)) {
		uint32_t Idx = LWText::CompareMultiple(C->m_Name, 7, "Material", "Style", "UIScale", "DPIScale", "Component", "Include", "Tooltip");
		
		if (Idx == 0) ParseMaterial(C, Manager);
		else if (Idx == 1) ParseStyle(C, Manager, StyleMap);
		else if (Idx == 2) ParseUIResScale(C, Manager);
		else if (Idx == 3) ParseUIDPIScale(C, Manager);
		else if (Idx == 4) ParseComponent(C, Manager, ComponentMap);
		else if (Idx == 5) ParseInclude(C, Node, Manager, X);
		else if (Idx == 6) ParseTooltip(C, Manager, X);
		else{
			LWEUI::XMLParseSubNodes(nullptr, C, X, Manager, "", nullptr, nullptr, StyleMap, ComponentMap);
		}
	}
	return true;
}

LWEUIManager &LWEUIManager::Update(const LWVector2f &Position, const LWVector2f &Size, float Scale, uint64_t lCurrentTime) {
	m_VisiblePosition = Position;
	m_VisibleSize = Size;
	std::fill(m_TempCount, m_TempCount + LWTouch::MaxTouchPoints, 0);
	m_Tooltip.m_TempTooltipedUI = nullptr;
	m_LastScale = Scale;
	bool OnlyFocusedTIBox = false;
	LWKeyboard *Keyboard = m_Window->GetKeyboardDevice();

	if (m_FocusedUI) {
		LWEUITextInput *TI = dynamic_cast<LWEUITextInput*>(m_FocusedUI);
		if (TI && m_Window->GetFlag()&LWWindow::KeyboardPresent) {
			LWVector4f KeyboardDim = m_Window->GetKeyboardLayout();
			LWVector2f RemainSize = LWVector2f((float)m_Window->GetSize().x, (float)m_Window->GetSize().y) - LWVector2f(KeyboardDim.z, KeyboardDim.w);
			LWVector2f TextBoxPos;
			TextBoxPos.x = KeyboardDim.x + RemainSize.x*0.5f - TI->GetVisibleSize().x*0.5f;
			TextBoxPos.x = std::max<float>(TextBoxPos.x, KeyboardDim.x + RemainSize.x*0.1f);
			TextBoxPos.y = KeyboardDim.y + KeyboardDim.w + RemainSize.y*0.5f - TI->GetVisibleSize().y*0.5f;
			TextBoxPos.y = std::min<float>(TextBoxPos.y + TI->GetVisibleSize().y - RemainSize.y*0.1f, TextBoxPos.y + TI->GetVisibleSize().y);
			TI->SetVisiblePosition(TextBoxPos);

			TI->UpdateSelf(*this, Scale, LWVector2f(), LWVector2f(), TI->GetVisiblePosition(), TI->GetVisibleSize(), lCurrentTime);
			OnlyFocusedTIBox = true;
		}
	}
	if(!OnlyFocusedTIBox){
		for (LWEUI *C = m_FirstUI; C; C = C->GetNext()) {
			C->Update(*this, Scale, m_VisiblePosition, m_VisibleSize, true, lCurrentTime);
		}
		if (Keyboard) {
			LWEUI *N = m_FocusedUI;
			if (Keyboard->ButtonPressed(LWKey::Tab)) {
				if (Keyboard->ButtonDown(LWKey::LShift)) {
					for (LWEUI *C = GetNext(nullptr); C; C = GetNext(C, C->isInvisible())) {
						if (C->isInvisible()) continue;
						if (C == m_FocusedUI) break;
						if (C->isTabAble()) N = C;
					}
				} else {
					for (N = GetNext(N); N; N = GetNext(N, N->isInvisible())) {
						if(N->isInvisible()) continue;
						if(N->isTabAble()) break;
					}
				}
			}
			if (N != m_FocusedUI) SetFocused(N);
		}
	}
	std::copy(m_TempCount, m_TempCount + LWTouch::MaxTouchPoints, m_OverCount);
	m_Tooltip.Update(Scale);
	m_Navigator.Update(m_Window, *this);
	return *this;
}

LWEUIManager &LWEUIManager::Update(float Scale, uint64_t lCurrentTime) {
	LWVector2i WndSize = m_Window->GetSize();
	return Update(LWVector2f(0.0f), LWVector2f((float)WndSize.x, (float)WndSize.y), Scale, lCurrentTime);
}

LWEUIManager &LWEUIManager::Update(uint64_t lCurrentTime) {
	LWVector2i WndSize = m_Window->GetSize();
	float S = FindScaleForSize(WndSize);
	return Update(LWVector2f(0.0f), LWVector2f((float)WndSize.x, (float)WndSize.y), S, lCurrentTime);
}

LWEUIManager &LWEUIManager::Draw(LWEUIFrame &Frame, float Scale, uint64_t lCurrentTime) {

	bool OnlyFocusedTIBox = false;
	if (m_FocusedUI) {
		LWEUITextInput *TI = dynamic_cast<LWEUITextInput*>(m_FocusedUI);
		if(TI && m_Window->isVirtualKeyboardPresent()){
			TI->DrawSelf(*this, Frame, Scale, LWVector2f(), LWVector2f(), TI->GetVisiblePosition(), TI->GetVisibleSize(), lCurrentTime);
			OnlyFocusedTIBox = true;
		}
	}
	if (!OnlyFocusedTIBox) {
		uint32_t i = 0;
		for (LWEUI *C = m_FirstUI; C; C = C->GetNext()) {
			C->Draw(*this, Frame, Scale, m_VisiblePosition, m_VisibleSize, lCurrentTime);
		}
	}
	m_Tooltip.Draw(Frame, *this, Scale, lCurrentTime);
	return *this;
}

LWEUIManager &LWEUIManager::Draw(LWEUIFrame &Frame, uint64_t lCurrentTime) {
	return Draw(Frame, m_LastScale, lCurrentTime);
}

LWEUI *LWEUIManager::GetNext(LWEUI *Current, bool SkipChildren) {
	if (!Current) return m_FirstUI;
	if (Current->GetFirstChild() && !SkipChildren) return Current->GetFirstChild();
	if (!Current->GetNext()) {
		if (Current->GetParent()) return GetNext(Current->GetParent(), true);
	}
	return Current->GetNext();
}

LWEUIManager &LWEUIManager::InsertUIAfter(LWEUI *UI, LWEUI *Parent, LWEUI *Prev) {
	UI->SetParent(Parent);
	if (!Prev) {
		if (!Parent) {
			UI->SetNext(m_FirstUI);
			m_FirstUI = UI;
			if (!m_LastUI) m_LastUI = m_FirstUI;
		} else {
			UI->SetNext(Parent->GetFirstChild());
			Parent->SetFirstChild(UI);
			if (!Parent->GetLastChild()) Parent->SetLastChild(UI);
		}
	} else {
		UI->SetParent(Prev->GetParent()).SetNext(Prev->GetNext());
		Prev->SetNext(UI);
		if (Prev->GetParent()) {
			if (Parent->GetLastChild() == Prev) Parent->SetLastChild(UI);
		} else {
			if (m_LastUI == Prev) m_LastUI = UI;
		}
	}
	return *this;
}

LWEUIManager &LWEUIManager::SetNavigationMode(bool Enabled, bool GamepadEnabled, bool KeyboardEnabled) {
	m_Navigator.m_Flag = 
		(m_Navigator.m_Flag&~(LWEUINavigation::GamepadEnabled | LWEUINavigation::KeyboardEnabled)) |
		((Enabled && KeyboardEnabled) ? LWEUINavigation::KeyboardEnabled : 0) |
		((Enabled && GamepadEnabled) ? LWEUINavigation::GamepadEnabled : 0);
	return *this;
}

LWEUIManager &LWEUIManager::RemoveUI(LWEUI *UI, bool Destroy) {
	LWEUI *PrevUI = nullptr;
	LWEUI *Parent = UI->GetParent();

	for (LWEUI *U = Parent?Parent->GetFirstChild():m_FirstUI; U != UI; PrevUI = U, U = U->GetNext()) {}
	if (Parent) {
		if (!PrevUI) Parent->SetFirstChild(UI->GetNext());
		else PrevUI->SetNext(UI->GetNext());
		
		if (Parent->GetLastChild() == UI) Parent->SetLastChild(PrevUI);
	} else {
		if (!PrevUI) m_FirstUI = UI->GetNext();
		else PrevUI->SetNext(UI->GetNext());

		if (m_LastUI == UI) m_LastUI = PrevUI;
	}

	if (Destroy) UI->Destroy();
	return *this;
}

bool LWEUIManager::RegisterEvent(LWEUI *UI, uint32_t EventCode, LWEUIEventCallback Callback, void *UserData) {
	if (!UI) return false;
	return UI->RegisterEvent(EventCode, Callback, UserData);
}

bool LWEUIManager::RegisterEvent(const LWText &UIName, uint32_t EventCode, LWEUIEventCallback Callback, void *UserData) {
	LWEUI *UI = GetNamedUI(UIName);
	if (!UI) return false;
	return RegisterEvent(UI, EventCode, Callback, UserData);
}

bool LWEUIManager::UnregisterEvent(LWEUI *UI, uint32_t EventCode) {
	if (!UI) return false;
	return UI->UnregisterEvent(EventCode);
}

bool LWEUIManager::UnregisterEvent(const LWText &UIName, uint32_t EventCode) {
	LWEUI *UI = GetNamedUI(UIName);
	if (!UI) return false;
	return UnregisterEvent(UI, EventCode);
}

bool LWEUIManager::DispatchEvent(LWEUI *Dispatchee, uint32_t EventCode, bool DoDispatch) {
	if (!DoDispatch) return false;
	uint32_t ECode = EventCode&LWEUI::Event_Flags;
	uint32_t TouchIdx = (EventCode&LWEUI::Event_OverIdx) >> LWEUI::Event_OverOffset;
	if (ECode == LWEUI::Event_TempOverInc && !Dispatchee->isIgnoringOverCount()) {
		if (Dispatchee->HasTooltip()) m_Tooltip.m_TempTooltipedUI = Dispatchee;
		m_TempCount[TouchIdx]++;
		return true;
	}
	Dispatchee->DispatchEvent(ECode);
	return true;
}

bool LWEUIManager::DispatchEvent(const char *DispatcheeName, uint32_t EventCode, bool DoDispatch) {
	if (!DoDispatch) return false;
	auto Iter = m_NameMap.find(LWText::MakeHash(DispatcheeName));
	if (Iter == m_NameMap.end()) return false;
	return DispatchEvent(Iter->second, EventCode);
}

bool LWEUIManager::DispatchEventf(const char *DispathceeNameFmt, uint32_t EventCode, bool DoDispatch, ...) {
	if (!DoDispatch) return false;
	char Buffer[256];
	va_list lst;
	va_start(lst, EventCode);
	vsnprintf(Buffer, sizeof(Buffer), DispathceeNameFmt, lst);
	va_end(lst);
	return DispatchEvent(Buffer, EventCode);
}

LWEUIManager &LWEUIManager::SetFocused(LWEUI *UI) {
	if (m_FocusedUI == UI) return *this;
	LWEUI *PrevFoc = m_FocusedUI;
	m_FocusedUI = UI;
	if (PrevFoc) {
		DispatchEvent(PrevFoc, LWEUI::Event_LostFocus);
		if (dynamic_cast<LWEUITextInput*>(PrevFoc)) m_Window->CloseKeyboard();
		
	}
	if (m_FocusedUI) {
		DispatchEvent(m_FocusedUI, LWEUI::Event_Focused);
		if (dynamic_cast<LWEUITextInput*>(m_FocusedUI)) {
			if(m_FocusedUI->GetFlag()&LWEUI::TouchEnabled) m_Window->OpenKeyboard();
		}
	}
	return *this;
}

bool LWEUIManager::PushScreenScale(const LWVector2i &Resolution, float Scale) {
	if (m_ResScaleCount >= MaxScreenScales) return false;
	uint32_t Area = Resolution.x*Resolution.y;
	uint32_t i = 0;
	for (; i < m_ResScaleCount; i++) {
		if (m_ResScaleMap[i].m_ScreenArea >= Area) break;
	}
	std::copy_backward(m_ResScaleMap + i, m_ResScaleMap + m_ResScaleCount, m_ResScaleMap + (m_ResScaleCount - 1));
	m_ResScaleMap[i].m_ScreenArea = Area;
	m_ResScaleMap[i].m_Scale = Scale;
	m_ResScaleCount++;
	return true;
}

bool LWEUIManager::PushDPIScale(uint32_t DPI, float Scale) {
	if (m_DPIScaleCount >= MaxDPIScales) return false;
	uint32_t i = 0;
	for (; i < m_DPIScaleCount; i++) {
		if (m_DPIScaleMap[i].m_ScreenDPI >= DPI) break;
	}
	std::copy_backward(m_DPIScaleMap + i, m_DPIScaleMap + m_DPIScaleCount, m_DPIScaleMap + (m_DPIScaleCount - 1));
	m_DPIScaleMap[i].m_ScreenDPI = DPI;
	m_DPIScaleMap[i].m_Scale = Scale;
	m_DPIScaleCount++;
	return true;
}

float LWEUIManager::FindScaleForSize(const LWVector2i &Size) {
	float ScreenScale = m_ResScaleCount ? m_ResScaleMap[m_ResScaleCount-1].m_Scale : 1.0f;
	float DPIScale = m_CachedDPIScale;
	uint32_t Area = Size.x*Size.y;
	for (uint32_t i = 1; i < m_ResScaleCount; i++) {
		if (Area < m_ResScaleMap[i].m_ScreenArea) {
			ScreenScale = m_ResScaleMap[i - 1].m_Scale;
			break;
		}
	}
	if (m_CachedDPIScale == 0.0f) {
		DPIScale = m_DPIScaleCount ? m_DPIScaleMap[m_DPIScaleCount-1].m_Scale : 1.0f;
		for (uint32_t i = 1; i < m_DPIScaleCount; i++) {
			if (m_DPIScaleMap[i].m_ScreenDPI > m_ScreenDPI) {
				uint32_t DPILen = m_DPIScaleMap[i].m_ScreenDPI - m_DPIScaleMap[i - 1].m_ScreenDPI;
				if (!DPILen) {
					DPIScale = m_DPIScaleMap[i].m_Scale;
				} else {
					float v = 1.0f-(float)(m_DPIScaleMap[i].m_ScreenDPI - m_ScreenDPI) / (float)DPILen;
					DPIScale = m_DPIScaleMap[i - 1].m_Scale + v * (m_DPIScaleMap[i].m_Scale - m_DPIScaleMap[i - 1].m_Scale);
				}
				break;
			}
		}
		m_CachedDPIScale = DPIScale;
	}
	return ScreenScale*DPIScale;
}

bool LWEUIManager::InsertNamedUI(const LWText &Name, LWEUI *UI) {
	auto Res = m_NameMap.insert(std::pair<uint32_t, LWEUI*>(Name.GetHash(), UI));
	return Res.second;
}

bool LWEUIManager::isTextInputFocused(void) {
	return dynamic_cast<LWEUITextInput*>(m_FocusedUI) != nullptr;
}

LWEUI *LWEUIManager::GetNamedUI(const LWText &Name) {
	auto Iter = m_NameMap.find(Name.GetHash());
	if (Iter == m_NameMap.end()) std::cout << "Could not find ui: '" << Name.GetCharacters() << "'" << std::endl;
	return Iter == m_NameMap.end() ? nullptr : Iter->second;
}

LWEUI *LWEUIManager::GetNamedUIf(const char *Format, ...) {
	char Buffer[256];
	va_list lst;
	va_start(lst, Format);
	vsnprintf(Buffer, sizeof(Buffer), Format, lst);
	va_end(lst);
	return GetNamedUI(Buffer);
}

LWEUIMaterial *LWEUIManager::InsertMaterial(const LWText &Name, const LWVector4f &ColorA, const LWVector4f &ColorB, uint32_t FillMode, LWTexture *Texture, const LWVector4f &SubRegion) {
	if (m_MaterialCount >= MaxMaterials) return nullptr;
	LWEUIMaterial *Mat = m_MaterialTable + m_MaterialCount;
	*Mat = LWEUIMaterial(ColorA, ColorB, FillMode, Texture, SubRegion);
	auto Res = m_MatTable.emplace(Name.GetHash(), Mat);
	if (!Res.second) return nullptr;
	m_MaterialCount++;
	return Mat;
}

LWEUIMaterial *LWEUIManager::GetMaterial(const LWText &Name) {
	auto Iter = m_MatTable.find(Name.GetHash());
	return Iter == m_MatTable.end() ? nullptr : Iter->second;
}

LWVector2f LWEUIManager::GetVisibleSize(void) const {
	return m_VisibleSize;
}

LWVector2f LWEUIManager::GetVisiblePosition(void) const {
	return m_VisiblePosition;
}

LWELocalization *LWEUIManager::GetLocalization(void) {
	return m_Localization;
}

LWEAssetManager *LWEUIManager::GetAssetManager(void) {
	return m_AssetManager;
}

LWAllocator *LWEUIManager::GetAllocator(void) {
	return m_Allocator;
}

LWWindow *LWEUIManager::GetWindow(void) {
	return m_Window;
}

LWEUI *LWEUIManager::GetFirstUI(void) {
	return m_FirstUI;
}

LWEUI *LWEUIManager::GetLastUI(void) {
	return m_LastUI;
}

LWEUI *LWEUIManager::GetFocusedUI(void) {
	return m_FocusedUI;
}

LWEUITooltip &LWEUIManager::GetTooltipDecoration(void) {
	return m_Tooltip;
}

LWEUINavigation &LWEUIManager::GetNavigator(void) {
	return m_Navigator;
}

uint32_t LWEUIManager::GetOverCount(uint32_t PointerIdx) const{
	return m_OverCount[PointerIdx];
}

bool LWEUIManager::isNavigationModeEnabled(void) const {
	return m_Navigator.isEnabled();
}

float LWEUIManager::GetLastScale(void) const {
	return m_LastScale;
}

uint32_t LWEUIManager::GetScreenDPI(void) const{
	return m_ScreenDPI;
}

LWEUIManager::LWEUIManager(LWWindow *Window, uint32_t ScreenDPI, LWAllocator *Allocator, LWELocalization *Localization, LWEAssetManager *AssetManager) : m_Window(Window), m_AssetManager(AssetManager), m_Allocator(Allocator), m_Localization(Localization), m_FirstUI(nullptr), m_LastUI(nullptr), m_FocusedUI(nullptr), m_LastScale(1.0f), m_MaterialCount(0), m_EventCount(0), m_ScreenDPI(ScreenDPI), m_ResScaleCount(0), m_DPIScaleCount(0), m_CachedDPIScale(0.0f) {
	memset(m_OverCount, 0, sizeof(m_OverCount));
}

LWEUIManager::~LWEUIManager() {
	std::function<void(LWEUI *U)> RecursiveDestroy;
	RecursiveDestroy = [&RecursiveDestroy](LWEUI *N) {
		for (LWEUI *C = N->GetFirstChild(), *K = C ? C->GetNext() : C; C; C = K, K = K ? K->GetNext() : K) {
			RecursiveDestroy(C);
		}
		N->Destroy();
	};
	for (LWEUI *C = m_FirstUI, *K = C ? C->GetNext() : C; C; C = K, K = K ? K->GetNext() : K) {
		RecursiveDestroy(C);
	}
}



