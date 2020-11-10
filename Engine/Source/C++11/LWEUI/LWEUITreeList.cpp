#include "LWEUI/LWEUITreeList.h"
#include <LWVideo/LWFont.h>
#include <LWPlatform/LWWindow.h>
#include <LWEAsset.h>
#include <LWEGeometry2D.h>

//LWEUITreeItem
LWEUITreeItem &LWEUITreeItem::SetValue(const LWUTF8Iterator &Value, LWFont *Font, float FontScale) {
	Value.Copy(m_Value, sizeof(m_Value));
	return UpdateTextBounds(Font, FontScale);
}

LWVector2f LWEUITreeItem::GetSize(float Scale, float BorderSize, float MinWidth, float MinHeight) {
	LWVector2f TSize = m_TextSize * Scale + LWVector2f(BorderSize);
	return TSize.Max(LWVector2f(MinWidth, MinHeight));
}

LWEUIMaterial *LWEUITreeItem::GetMaterial(bool isOver, bool isDown, LWEUIMaterial *DefOffMaterial, LWEUIMaterial *DefOverMaterial, LWEUIMaterial *DefDownMaterial) {
	LWEUIMaterial *Mat = m_OffMaterial ? m_OffMaterial : DefOffMaterial;
	if (isOver) {
		if (isDown) Mat = m_DownMaterial ? m_DownMaterial : DefDownMaterial;
		else Mat = m_OverMaterial ? m_OverMaterial : DefOverMaterial;
	}
	return Mat;
}

LWEUITreeItem &LWEUITreeItem::UpdateTextBounds(LWFont *Font, float FontScale) {
	if (!Font) return *this;
	LWVector4f TextSize = Font->MeasureText(m_Value, FontScale);
	m_TextSize = LWVector2f(TextSize.z - TextSize.x, TextSize.y - TextSize.w);
	return *this;
}

LWUTF8Iterator LWEUITreeItem::GetValue(void) const {
	return m_Value;
}

LWUTF8GraphemeIterator LWEUITreeItem::GetValueGrapheme(void) const {
	return m_Value;
}

LWEUITreeItem::LWEUITreeItem(const LWUTF8Iterator &Value, void *UserData, LWFont *Font, float FontScale, LWEUIMaterial *OffMaterial, LWEUIMaterial *OverMaterial, LWEUIMaterial *DownMaterial) : m_UserData(UserData), m_OffMaterial(OffMaterial), m_OverMaterial(OverMaterial), m_DownMaterial(DownMaterial) {
	SetValue(Value, Font, FontScale);
}

//LWEUITreeList

LWEUITreeList *LWEUITreeList::XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, LWEXMLNode *Style, const LWUTF8Iterator &ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap) {
	char Buffer[256];
	LWAllocator &Allocator = Manager->GetAllocator();
	LWELocalization *Localize = Manager->GetLocalization();
	LWEAssetManager *AM = Manager->GetAssetManager();
	LWEUITreeList *Tree = Allocator.Create<LWEUITreeList>(LWVector4f(0.0f), LWVector4f(0.0f), 0);
	LWEXMLAttribute *StyleAttr = Node->FindAttribute("Style");
	LWEUI::XMLParse(Tree, Node, XML, Manager, Style, ActiveComponentName, ActiveComponent, ActiveComponentNode, StyleMap, ComponentMap);

	LWEXMLAttribute *OverAttr = FindAttribute(Node, Style, "OverMaterial");
	LWEXMLAttribute *DownAttr = FindAttribute(Node, Style, "DownMaterial");
	LWEXMLAttribute *OffAttr = FindAttribute(Node, Style, "OffMaterial");
	LWEXMLAttribute *BackAttr = FindAttribute(Node, Style, "BackgroundMaterial");
	LWEXMLAttribute *FontAttr = FindAttribute(Node, Style, "Font");
	LWEXMLAttribute *MinimumHeightAttr = FindAttribute(Node, Style, "MinimumHeight");
	LWEXMLAttribute *LineThicknessAttr = FindAttribute(Node, Style, "LineThickness");
	LWEXMLAttribute *HighlightMaterialAttr = FindAttribute(Node, Style, "HighlightMaterial");
	LWEXMLAttribute *FontMatAttr = FindAttribute(Node, Style, "FontMaterial");
	LWEXMLAttribute *LineMaterialAttr = FindAttribute(Node, Style, "LineMaterial");
	LWEXMLAttribute *FontScaleAttr = FindAttribute(Node, Style, "FontScale");
	LWEUIMaterial *OverMat = nullptr;
	LWEUIMaterial *DownMat = nullptr;
	LWEUIMaterial *OffMat = nullptr;
	LWEUIMaterial *BackMat = nullptr;
	LWEUIMaterial *FntMat = nullptr;
	LWEUIMaterial *HighMat = nullptr;
	LWEUIMaterial *LineMat = nullptr;
	LWFont *Font = nullptr;

	if (OverAttr) OverMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), OverAttr->GetValue(), ActiveComponent, ActiveComponentNode));
	if (DownAttr) DownMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), DownAttr->GetValue(), ActiveComponent, ActiveComponentNode));
	if (OffAttr) OffMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), OffAttr->GetValue(), ActiveComponent, ActiveComponentNode));
	if (BackAttr) BackMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), BackAttr->GetValue(), ActiveComponent, ActiveComponentNode));
	if (FontMatAttr) FntMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), FontMatAttr->GetValue(), ActiveComponent, ActiveComponentNode));
	if (LineMaterialAttr) LineMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), LineMaterialAttr->GetValue(), ActiveComponent, ActiveComponentNode));
	if (HighlightMaterialAttr) HighMat = Manager->GetMaterial(ParseComponentAttribute(Buffer, sizeof(Buffer), HighlightMaterialAttr->GetValue(), ActiveComponent, ActiveComponentNode));
	if (FontAttr) Font = AM->GetAsset<LWFont>(ParseComponentAttribute(Buffer, sizeof(Buffer), FontAttr->GetValue(), ActiveComponent, ActiveComponentNode));
	if (MinimumHeightAttr) {
		LWUTF8Iterator Res = ParseComponentAttribute(Buffer, sizeof(Buffer), MinimumHeightAttr->GetValue(), ActiveComponent, ActiveComponentNode);
		Tree->SetMinimumHeight((float)atof((const char*)Res()));
	}
	if (LineThicknessAttr) {
		LWUTF8Iterator Res = ParseComponentAttribute(Buffer, sizeof(Buffer), LineThicknessAttr->GetValue(), ActiveComponent, ActiveComponentNode);
		Tree->SetLineThickness((float)atof((const char*)Res()));
	}
	if (FontScaleAttr) {
		LWUTF8Iterator Res = ParseComponentAttribute(Buffer, sizeof(Buffer), FontScaleAttr->GetValue(), ActiveComponent, ActiveComponentNode);
		Tree->SetFontScale((float)atof((const char*)Res()));
	}
	Tree->SetOffMaterial(OffMat).SetOverMaterial(OverMat).SetDownMaterial(DownMat).SetBackgroundMaterial(BackMat).SetFontMaterial(FntMat);
	Tree->SetLineMaterial(LineMat).SetHighlightMaterial(HighMat);
	Tree->SetFont(Font);
	return Tree;
}

LWEUI &LWEUITreeList::UpdateSelf(LWEUIManager &Manager, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime) {
	LWWindow *Wnd = Manager.GetWindow();
	LWEUINavigation &Navigator = Manager.GetNavigator();
	const float ScrollScale = 0.25f;

	float LineThick = m_LineThickness * Scale;
	float LineOffset = LineThick * 10.0f;
	float MinWidth = LineOffset;
	float BorderSize = 2.0f*Scale;
	float MinHeight = m_MinimumHeight * Scale;

	bool isNavigationEnabled = Navigator.isEnabled();
	bool isFocused = Manager.GetFocusedUI() == this;
	bool isFocusable = (m_Flag&FocusAble) != 0;
	bool isEditEnabled = m_TreeChangeCallback != nullptr;

	LWMouse *Mouse = Wnd->GetMouseDevice();
	LWTouch *Touch = Wnd->GetTouchDevice();

	bool wasOver = (m_Flag&MouseOver);
	bool wasDown = (m_Flag&MouseLDown);
	uint64_t Flag = (m_Flag&~(MouseOver | MouseLDown)) | (m_TimeOver ? MouseOver : 0);
	bool isOver = (Flag&MouseOver);

	LWVector2f OverPos = LWVector2f(-1000.0f);

	if (Mouse) {
		int32_t Scroll = Mouse->GetScroll();
		if (Scroll && isOver) {
			LWVector2f mScroll = m_Scroll;
			LWVector2f ScrollSize = GetScrollPageSize();
			if (Scroll > 0) mScroll.y -= ScrollSize.y*ScrollScale;
			else mScroll.y += ScrollSize.y*ScrollScale;
			SetScroll(mScroll);
		}
		OverPos = Mouse->GetPositionf();
		if (Mouse->ButtonPressed(LWMouseKey::Left)) m_DragPosition = OverPos;
		if (Mouse->ButtonDown(LWMouseKey::Left)) {
			Flag = Flag | (isOver ? MouseLDown : 0);
			if (isEditEnabled && m_DraggingID==-1) {
				if (m_DragPosition.DistanceSquared(OverPos) > BorderSize*BorderSize) {
					m_DraggingID = m_OverEditID == -1 ? m_OverID : -1;
				}
			}
		} else if (!wasDown) m_DraggingID = -1;
	}

	bool isDown = (Flag&MouseLDown) != 0;
	m_Flag = Flag;

	uint32_t OverID = -1;
	uint32_t OverEditID = -1;
	uint32_t DragDestParentID = -1;
	uint32_t DragDestPrevID = -1;
	LWVector2f MaxScroll = LWVector2f();
	std::function<uint32_t(uint32_t, LWVector2f &)> UpdateItem = [this, &UpdateItem, &Scale, &LineOffset, &LineThick, &MinWidth, &MinHeight, &BorderSize, &OverID, &OverEditID, &OverPos, &DragDestParentID, &DragDestPrevID, &MaxScroll, &isEditEnabled, &isDown](uint32_t ID, LWVector2f &Pos)->uint32_t {
		LWEUITreeItem &Itm = m_List[ID];
		LWVector2f Size = Itm.GetSize(Scale, BorderSize, MinWidth, MinHeight);
		Pos.y -= Size.y;
		Pos.x += LineOffset * 0.5f;
		float InitX = Pos.x;
		Pos.x += LineOffset * 0.5f;
		if (LWEPointInsideAABB(OverPos, Pos, Pos + Size)) OverID = ID;
		Pos.x += Size.x;
		if(isEditEnabled){
			if (m_DraggingID == -1) {
				LWVector2f AddBtnSize = m_EditAddItem.GetSize(Scale, BorderSize, 0.0f, 0.0f);
				LWVector2f DelBtnSize = m_EditAddItem.GetSize(Scale, BorderSize, 0.0f, 0.0f);
				if (LWEPointInsideAABB(OverPos, Pos, Pos + AddBtnSize)) {
					OverID = ID;
					OverEditID = EditIDAdd;
				}
				Pos.x += AddBtnSize.x + BorderSize;
				if (LWEPointInsideAABB(OverPos, Pos, Pos + DelBtnSize)) {
					OverID = ID;
					OverEditID = EditIDDel;
				}
				Pos.x += DelBtnSize.x + BorderSize;
			} else if(m_DraggingID!=ID){
				if (!IsParentOf(m_DraggingID, ID)) {
					if (OverPos.y < Pos.y) {
						if (OverPos.x > Pos.x - Size.x*0.5f) {
							DragDestParentID = ID;
							DragDestPrevID = -1;
						} else {
							DragDestParentID = Itm.m_ParentID;
							DragDestPrevID = ID;
						}
					}
				}
			}
		}
		MaxScroll.x = std::max<float>(Pos.x, MaxScroll.x);
		Pos.y -= BorderSize;
		Pos.x = InitX;
		uint32_t c = Itm.m_FirstChildID;
		while (c != -1) {
			c = UpdateItem(c, Pos);
			Pos.x = InitX;
		}
		return Itm.m_NextID;
	};

	LWVector2f Pos = VisiblePos + LWVector2f(0.0f, VisibleSize.y) - LWVector2f(m_Scroll.x, -m_Scroll.y);
	LWVector2f InitPos = Pos;
	uint32_t c = m_FirstChildID;
	while (c != -1) {
		c = UpdateItem(c, Pos);
		Pos.x = InitPos.x;
	}
	if (isEditEnabled) {
		if (m_DraggingID == -1) {
			LWVector2f AddSize = m_EditAddItem.GetSize(Scale, BorderSize, 0.0f, 0.0f);
			Pos += LWVector2f(BorderSize, -AddSize.y);
			if (LWEPointInsideAABB(OverPos, Pos, Pos + AddSize)) {
				OverEditID = EditIDAdd;
			}
		}
	}

	MaxScroll.y = Pos.y;
	m_MaxScroll = LWVector2f(MaxScroll.x - InitPos.x, InitPos.y - MaxScroll.y);
	m_OverID = OverID;
	m_OverEditID = OverEditID;
	m_DragDestParentID = DragDestParentID;
	m_DragDestPrevID = DragDestPrevID;

	Manager.DispatchEvent(this, Event_MouseOver, isOver && !wasOver);
	Manager.DispatchEvent(this, Event_MouseOver, !isOver && wasOver);
	Manager.DispatchEvent(this, Event_Pressed, isOver && isDown && !wasDown);
	if (Manager.DispatchEvent(this, Event_Released, isOver && !isDown && wasDown)) {
		if (isEditEnabled) {
			LWEUITreeEvent Event = { (uint32_t)-1,0,0,0 };
			if (m_DraggingID != -1) {
				if (m_DragDestParentID != -1 || m_DragDestPrevID != -1) {
					Event = { LWEUITreeEvent::Moving, m_DraggingID, m_DragDestParentID, m_DragDestPrevID };
				}
			} else if (m_OverEditID == EditIDAdd) Event = { LWEUITreeEvent::Adding, m_OverID, 0, 0 };
			else if (m_OverEditID == EditIDDel) Event = { LWEUITreeEvent::Removing, m_OverID, 0, 0 };
			if (Event.m_EventType != -1) m_TreeChangeCallback(*this, Event, Manager, m_UserData);
		}
		if (isFocusable) Manager.SetFocused(this);
	}


	return *this;
}

LWEUI &LWEUITreeList::DrawSelf(LWEUIManager &Manager, LWEUIFrame &Frame, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime) {
	float LineThick = m_LineThickness * Scale;
	float LineOffset = LineThick * 10.0f;
	float MinWidth = LineOffset;
	float BorderSize = 2.0f*Scale;
	float MinHeight = m_MinimumHeight * Scale;
	if (!m_Font) return *this;
	bool isDown = (m_Flag&MouseLDown) != 0;
	bool isEditEnabled = m_TreeChangeCallback != nullptr;

	LWVector4f AABB = LWVector4f(VisiblePos, VisibleSize);
	std::function<uint32_t(uint32_t, LWVector2f &)> DrawItem = [this, &DrawItem, &Scale, &LineOffset, &LineThick, &MinWidth, &MinHeight, &BorderSize, &AABB, &isDown, &isEditEnabled, &Frame](uint32_t ID, LWVector2f &Pos)->uint32_t {
		LWEUITreeItem &Itm = m_List[ID];
		LWVector2f Size = Itm.GetSize(Scale, BorderSize, MinWidth, MinHeight);
		float hHeight = Size.y*0.5f;
		Pos.y -= hHeight;
		Frame.WriteClippedLine(m_LineMaterial, Pos, Pos + LWVector2f(LineOffset, 0.0f), LineThick, AABB);
		Pos.x += LineOffset*0.5f;
		LWVector2f InitPos = Pos;
		Pos.x += LineOffset * 0.5f;
		Pos.y -= hHeight;
		Frame.WriteClippedRect(Itm.GetMaterial((m_OverID==ID && m_OverEditID==-1) || (m_DraggingID==ID), isDown || (m_DraggingID==ID), m_OffMaterial, m_OverMaterial, m_DownMaterial), Pos, Size, AABB);
		m_Font->DrawClippedTextm(Itm.m_Value, Pos + LWVector2f(BorderSize*0.5f), m_FontScale*Scale, m_FontMaterial ? m_FontMaterial->m_ColorA : LWVector4f(1.0f), AABB, &Frame, &LWEUIFrame::WriteFontGlyph);
		
		if (isEditEnabled) {
			if (m_DraggingID == -1) {
				Pos.x += Size.x + BorderSize;
				LWVector2f AddSize = m_EditAddItem.GetSize(Scale, BorderSize, 0.0f, 0.0f);
				LWVector2f DelSize = m_EditDelItem.GetSize(Scale, BorderSize, 0.0f, 0.0f);
				Frame.WriteClippedRect(m_EditAddItem.GetMaterial(m_OverID == ID && m_OverEditID == EditIDAdd, isDown, m_OffMaterial, m_OverMaterial, m_DownMaterial), Pos, AddSize, AABB);
				m_Font->DrawClippedTextm(m_EditAddItem.m_Value, Pos + LWVector2f(BorderSize*0.5f), m_FontScale*Scale, m_FontMaterial ? m_FontMaterial->m_ColorA : LWVector4f(1.0f), AABB, &Frame, &LWEUIFrame::WriteFontGlyph);
				Pos.x += AddSize.x + BorderSize;
				Frame.WriteClippedRect(m_EditDelItem.GetMaterial(m_OverID == ID && m_OverEditID == EditIDDel, isDown, m_OffMaterial, m_OverMaterial, m_DownMaterial), Pos, AddSize, AABB);
				m_Font->DrawClippedTextm(m_EditDelItem.m_Value, Pos + LWVector2f(BorderSize*0.5f), m_FontScale*Scale, m_FontMaterial ? m_FontMaterial->m_ColorA : LWVector4f(1.0f), AABB, &Frame, &LWEUIFrame::WriteFontGlyph);
			} else {
				if (m_DragDestParentID == ID && m_DragDestPrevID == -1) {
					Frame.WriteClippedLine(m_HighlightMaterial, Pos + LWVector2f(Size.x*0.5f, 0.0f), Pos + LWVector2f(Size.x*1.5f, 0.5f), LineThick, AABB);
				} else if (m_DragDestPrevID == ID) {
					Frame.WriteClippedLine(m_HighlightMaterial, Pos, Pos + LWVector2f(Size.x, 0.0f), LineThick, AABB);
				}
			}
		}

		Pos.y -= BorderSize;
		Pos.x = InitPos.x;
		LWVector2f LastPos = InitPos;
		uint32_t c = Itm.m_FirstChildID;
		while (c != -1) {
			LWEUITreeItem &CItm = m_List[c];
			LastPos = Pos;
			LastPos.y -= CItm.GetSize(Scale, BorderSize, MinWidth, MinHeight).y*0.5f;
			c = DrawItem(c, Pos);
			Pos.x = InitPos.x;
		}
		if(LastPos.y!=InitPos.y){
			InitPos.x += LineThick * 0.5f;
			LastPos.x = InitPos.x;
			Frame.WriteClippedLine(m_LineMaterial, InitPos, LastPos, LineThick, AABB);
		}
		return Itm.m_NextID;
	};

	Frame.WriteRect(m_BackgroundMaterial, VisiblePos, VisibleSize);
	LWVector2f Pos = VisiblePos + LWVector2f(0.0f, VisibleSize.y) - LWVector2f(m_Scroll.x, -m_Scroll.y);

	LWVector2f InitPos = Pos;
	LWVector2f LastPos = Pos;
	uint32_t c = m_FirstChildID;
	while (c != -1) {
		LWEUITreeItem &Itm = m_List[c];
		LastPos = Pos;
		LastPos.y -= Itm.GetSize(Scale, BorderSize, MinWidth, MinHeight).y*0.5f;
		c = DrawItem(c, Pos);
		Pos.x = InitPos.x;
	}
	if(LastPos.y!=InitPos.y){
		InitPos.x += LineThick * 0.5f;
		LastPos.x = InitPos.x;
		Frame.WriteClippedLine(m_LineMaterial, InitPos, LastPos, LineThick, AABB);
	} 
	if(isEditEnabled){
		if (m_DraggingID == -1) {
			LWVector2f AddSize = m_EditAddItem.GetSize(Scale, BorderSize, 0.0f, 0.0f);
			Pos += LWVector2f(BorderSize, -AddSize.y);
			Frame.WriteClippedRect(m_EditAddItem.GetMaterial(m_OverID == -1 && m_OverEditID == EditIDAdd, isDown, m_OffMaterial, m_OverMaterial, m_DownMaterial), Pos, AddSize, AABB);
			m_Font->DrawClippedTextm(m_EditAddItem.m_Value, Pos + LWVector2f(BorderSize*0.5f), m_FontScale*Scale, m_FontMaterial ? m_FontMaterial->m_ColorA : LWVector4f(1.0f), AABB, &Frame, &LWEUIFrame::WriteFontGlyph);
		}
	}
	return *this;
}

void LWEUITreeList::Destroy(void) {
	LWAllocator::Destroy(this);
	return;
}

LWEUITreeList &LWEUITreeList::SetItemValue(uint32_t ID, const LWUTF8Iterator &Value) {
	m_List[ID].SetValue(Value, m_Font, m_FontScale);
	return *this;
}

uint32_t LWEUITreeList::InsertItemAt(const LWUTF8Iterator &Value, void *UserData, const LWEUITreeEvent &Event, LWAllocator &Allocator, LWEUIMaterial *OffMaterial, LWEUIMaterial *OverMaterial, LWEUIMaterial *DownMaterial) {
	return InsertItemAt(Value, UserData, Event.m_SourceID, -1, Allocator, OffMaterial, OverMaterial, DownMaterial);
}

uint32_t LWEUITreeList::InsertItemAt(const LWUTF8Iterator &Value, void *UserData, uint32_t ParentID, uint32_t PrevID, LWAllocator &Allocator, LWEUIMaterial *OffMaterial, LWEUIMaterial *OverMaterial, LWEUIMaterial *DownMaterial) {
	if (m_ListLength >= m_ListBufferSize) {
		LWAllocator::Destroy(m_OldList);
		m_OldList = m_List;
		LWEUITreeItem *L = Allocator.Allocate<LWEUITreeItem>(m_ListBufferSize + BatchSize);
		std::copy(m_List, m_List + m_ListLength, L);
		m_List = L;
		m_ListBufferSize += BatchSize;
	}	
	uint32_t ID = m_ListLength;
	m_List[ID] = LWEUITreeItem(Value, UserData, m_Font, m_FontScale, OffMaterial, OverMaterial, DownMaterial);
	MoveItemTo(ID, ParentID, PrevID);
	m_ListLength++;
	return ID;
}

uint32_t LWEUITreeList::InsertChildFirst(const LWUTF8Iterator &Value, void *UserData, uint32_t ParentID, LWAllocator &Allocator, LWEUIMaterial *OffMaterial, LWEUIMaterial *OverMaterial, LWEUIMaterial *DownMaterial) {
	return InsertItemAt(Value, UserData, ParentID, -1, Allocator, OffMaterial, OverMaterial, DownMaterial);
}

uint32_t LWEUITreeList::InsertChildLast(const LWUTF8Iterator &Value, void *UserData, uint32_t ParentID, LWAllocator &Allocator, LWEUIMaterial *OffMaterial, LWEUIMaterial *OverMaterial, LWEUIMaterial *DownMaterial) {
	uint32_t PrevID = m_LastChildID;
	if (ParentID != -1) PrevID = m_List[ParentID].m_LastChildID;
	return InsertItemAt(Value, UserData, ParentID, PrevID, Allocator, OffMaterial, OverMaterial, DownMaterial);
}

LWEUITreeList &LWEUITreeList::RemoveItemAt(uint32_t ID) {
	LWEUITreeItem &Itm = m_List[ID];
	if (Itm.m_ParentID!=-1) {
		LWEUITreeItem &Parent = m_List[Itm.m_ParentID];
		if (Parent.m_FirstChildID == ID) {
			Parent.m_FirstChildID = Itm.m_NextID;
		}
		if (Parent.m_LastChildID == ID) {
			Parent.m_LastChildID = Itm.m_PrevID;
		}
	} else {
		if (m_FirstChildID == ID) {
			m_FirstChildID = Itm.m_NextID;
		}
		if (m_LastChildID == ID) {
			m_LastChildID = Itm.m_PrevID;
		}
	}
	if (Itm.m_PrevID != -1) {
		LWEUITreeItem &Prev = m_List[Itm.m_PrevID];
		Prev.m_NextID = Itm.m_NextID;
	}
	if (Itm.m_NextID != -1) {
		LWEUITreeItem &Next = m_List[Itm.m_NextID];
		Next.m_PrevID = Itm.m_PrevID;
	}
	return *this;
}

LWEUITreeList &LWEUITreeList::RemoveItemAt(const LWEUITreeEvent &Event) {
	return RemoveItemAt(Event.m_SourceID);
}

/*!< \brief moves child to new location, with it's children in tact. */
LWEUITreeList &LWEUITreeList::MoveItemTo(uint32_t SourceID, uint32_t ParentID, uint32_t PrevID) {
	LWEUITreeItem &Itm = m_List[SourceID];
	RemoveItemAt(SourceID);
	Itm.m_ParentID = ParentID;
	Itm.m_PrevID = PrevID;
	Itm.m_NextID = -1;
	if (ParentID != -1) {
		LWEUITreeItem &Parent = m_List[ParentID];
		if (PrevID == -1) {
			if (Parent.m_FirstChildID != -1) {
				LWEUITreeItem &PFirst = m_List[Parent.m_FirstChildID];
				PFirst.m_PrevID = SourceID;
				Itm.m_NextID = Parent.m_FirstChildID;
			}
			Parent.m_FirstChildID = SourceID;
		}
		if (PrevID == Parent.m_LastChildID) Parent.m_LastChildID = SourceID;
	} else {
		if (PrevID == -1) {
			if (m_FirstChildID != -1) {
				LWEUITreeItem &PFirst = m_List[m_FirstChildID];
				PFirst.m_PrevID = SourceID;
				Itm.m_NextID = m_FirstChildID;
			}
			m_FirstChildID = SourceID;
		}
		if (PrevID == m_LastChildID) m_LastChildID = SourceID;
	}
	if (PrevID != -1) {
		LWEUITreeItem &Prev = m_List[PrevID];
		if (Prev.m_NextID != -1) {
			LWEUITreeItem &PNext = m_List[Prev.m_NextID];
			PNext.m_PrevID = SourceID;
			Itm.m_NextID = Prev.m_NextID;
		}
		Prev.m_NextID = SourceID;
	}
	return *this;
}

LWEUITreeList &LWEUITreeList::MoveItemTo(const LWEUITreeEvent &Event) {
	return MoveItemTo(Event.m_SourceID, Event.m_DestParentID, Event.m_DestPrevID);
}

LWEUITreeList &LWEUITreeList::Prune(LWAllocator &Allocator) {
	LWEUITreeItem *NewList = Allocator.Allocate<LWEUITreeItem>(m_ListBufferSize);
	uint32_t NewListLen = 0;
	std::function<uint32_t(uint32_t, uint32_t)> ProcessNode = [&NewList, &NewListLen, this, &ProcessNode](uint32_t ID, uint32_t ParentID)->uint32_t {
		LWEUITreeItem &Itm = m_List[ID];
		uint32_t nID = NewListLen++;
		LWEUITreeItem &nItm = NewList[nID];
		nItm = LWEUITreeItem(Itm.m_Value, Itm.m_UserData, m_Font, m_FontScale, Itm.m_OffMaterial, Itm.m_OverMaterial, Itm.m_DownMaterial);

		nItm.m_ParentID = ParentID;

		uint32_t c = Itm.m_FirstChildID;
		uint32_t PrevNID = -1;
		while (c != -1) {
			LWEUITreeItem &CItm = m_List[c];
			uint32_t cnID = ProcessNode(c, nID);
			LWEUITreeItem &NCItm = NewList[cnID];
			NCItm.m_PrevID = PrevNID;
			if (PrevNID == -1) nItm.m_FirstChildID = cnID;
			else {
				LWEUITreeItem &NCPrev = NewList[PrevNID];
				NCPrev.m_NextID = cnID;
			}
			PrevNID = cnID;
			c = CItm.m_NextID;
		}
		nItm.m_LastChildID = PrevNID;
		return nID;
	};
	uint32_t c = m_FirstChildID;
	uint32_t PrevID = -1;
	while (c != -1) {
		LWEUITreeItem &CItm = m_List[c];
		uint32_t ID = ProcessNode(c, -1);
		LWEUITreeItem &NCItm = NewList[ID];
		NCItm.m_PrevID = PrevID;
		if (PrevID == -1) m_FirstChildID = ID;
		else {
			LWEUITreeItem &NCPrev = NewList[PrevID];
			NCPrev.m_NextID = ID;
		}
		PrevID = ID;
		c = CItm.m_NextID;
	}
	m_LastChildID = PrevID;
	LWAllocator::Destroy(m_OldList);
	m_OldList = m_List;
	m_List = NewList;
	m_ListLength = NewListLen;
	return *this;
}

bool LWEUITreeList::IsParentOf(uint32_t SourceID, uint32_t ChildID) {
	uint32_t p = ChildID;
	while (p != -1) {
		LWEUITreeItem &PItm = m_List[p];
		if (PItm.m_ParentID == SourceID) return true;
		p = PItm.m_ParentID;
	}
	return false;
}

LWEUITreeList &LWEUITreeList::Clear(void) {
	m_ListLength = 0;
	m_FirstChildID = m_LastChildID = -1;
	return *this;
}

LWEUITreeList &LWEUITreeList::SetChangeCallback(LWEUITreeChangeCallback Callback, void *UserData) {
	m_TreeChangeCallback = Callback;
	m_UserData = UserData;
	return *this;
}

LWEUITreeList &LWEUITreeList::SetBackgroundMaterial(LWEUIMaterial *Material) {
	m_BackgroundMaterial = Material;
	return *this;
}

LWEUITreeList &LWEUITreeList::SetFontMaterial(LWEUIMaterial *Material) {
	m_FontMaterial = Material;
	return *this;
}

LWEUITreeList &LWEUITreeList::SetOffMaterial(LWEUIMaterial *Material) {
	m_OffMaterial = Material;
	return *this;
}

LWEUITreeList &LWEUITreeList::SetOverMaterial(LWEUIMaterial *Material) {
	m_OverMaterial = Material;
	return *this;
}

LWEUITreeList &LWEUITreeList::SetDownMaterial(LWEUIMaterial *Material) {
	m_DownMaterial = Material;
	return *this;
}

LWEUITreeList &LWEUITreeList::SetLineMaterial(LWEUIMaterial *Material) {
	m_LineMaterial = Material;
	return *this;
}

LWEUITreeList &LWEUITreeList::SetHighlightMaterial(LWEUIMaterial *Material) {
	m_HighlightMaterial = Material;
	return *this;
}

LWEUITreeList &LWEUITreeList::SetLineThickness(float LineThickness) {
	m_LineThickness = LineThickness;
	return *this;
}

LWEUITreeList &LWEUITreeList::SetFont(LWFont *Font) {
	m_Font = Font;
	std::function<void(uint32_t)> UpdateNodeTextBounds = [&UpdateNodeTextBounds, this](uint32_t ID) {
		LWEUITreeItem &Itm = m_List[ID];
		Itm.UpdateTextBounds(m_Font, m_FontScale);
		uint32_t c = Itm.m_FirstChildID;
		while (c != -1) {
			LWEUITreeItem &CItm = m_List[c];
			UpdateNodeTextBounds(c);
			c = CItm.m_NextID;
		}
		return;
	};
	
	uint32_t c = m_FirstChildID;
	while (c != -1) {
		LWEUITreeItem &CItm = m_List[c];
		UpdateNodeTextBounds(c);
		c = CItm.m_NextID;
	};
	m_EditAddItem.SetValue("+", m_Font, m_FontScale);
	m_EditDelItem.SetValue("-", m_Font, m_FontScale);
	return *this;
}

LWEUITreeList &LWEUITreeList::SetScroll(const LWVector2f &Scroll) {
	m_Scroll = Scroll.Min(m_MaxScroll - GetScrollPageSize()).Max(LWVector2f());
	return *this;
}

LWEUITreeList &LWEUITreeList::SetFontScale(float FontScale) {
	m_FontScale = FontScale;
	return SetFont(m_Font);
}

LWEUITreeList &LWEUITreeList::SetMinimumHeight(float MinHeight) {
	m_MinimumHeight = MinHeight;
	return *this;
}

LWEUIMaterial *LWEUITreeList::GetBackgroundMaterial(void) {
	return m_BackgroundMaterial;
}

LWEUIMaterial *LWEUITreeList::GetFontMaterial(void) {
	return m_FontMaterial;
}

LWEUIMaterial *LWEUITreeList::GetOffMaterial(void) {
	return m_OffMaterial;
}

LWEUIMaterial *LWEUITreeList::GetOverMaterial(void) {
	return m_OverMaterial;
}

LWEUIMaterial *LWEUITreeList::GetDownMaterial(void) {
	return m_DownMaterial;
}

LWEUIMaterial *LWEUITreeList::GetLineMaterial(void) {
	return m_LineMaterial;
}

LWEUIMaterial *LWEUITreeList::GetHighlightMaterial(void) {
	return m_HighlightMaterial;
}

uint32_t LWEUITreeList::GetOverID(void) const {
	if (m_OverEditID != -1 || m_DraggingID!=-1) return -1;
	return m_OverID;
}

LWEUITreeItem *LWEUITreeList::GetOverItem(void) {
	if (m_OverID == -1) return nullptr;
	return &m_List[m_OverID];
}

LWFont *LWEUITreeList::GetFont(void) {
	return m_Font;
}

float LWEUITreeList::GetLineThickness(void) const {
	return m_LineThickness;
}

float LWEUITreeList::GetMinimumHeight(void) const {
	return m_MinimumHeight;
}

float LWEUITreeList::GetFontScale(void) const {
	return m_FontScale;
}

LWVector2f LWEUITreeList::GetScroll(void) const {
	return m_Scroll;
}

LWVector2f LWEUITreeList::GetMaxScroll(void) const {
	return m_MaxScroll;
}

LWVector2f LWEUITreeList::GetScrollPageSize(void) const {
	return m_VisibleSize;
}

uint32_t LWEUITreeList::GetFirstChildID(void) const {
	return m_FirstChildID;
}

uint32_t LWEUITreeList::GetLastChildID(void) const {
	return m_LastChildID;
}

LWEUITreeItem &LWEUITreeList::GetItem(uint32_t ID) {
	return m_List[ID];
}

const LWEUITreeItem &LWEUITreeList::GetItem(uint32_t ID) const {
	return m_List[ID];
}

LWEUITreeList::LWEUITreeList(const LWVector4f &Position, const LWVector4f &Size, uint64_t Flags) : LWEUI(Position, Size, Flags) {

}