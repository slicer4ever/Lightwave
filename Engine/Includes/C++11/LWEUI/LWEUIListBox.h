#ifndef LWEUILISTBOX_H
#define LWEUILISTBOX_H
#include "LWEUI/LWEUI.h"

#define LWELISTBOX_MAXLENGTH 256
#define LWELISTBOX_MAXITEMS 256

struct LWEUIListBoxItem {
	char m_Name[LWELISTBOX_MAXITEMS];
	LWVector2f m_TextSize;
	void *m_UserData;
	LWEUIMaterial *m_OffMaterial;
	LWEUIMaterial *m_OverMaterial;
	LWEUIMaterial *m_DownMaterial;
};

class LWEUIListBox : public LWEUI {
public:
	static LWEUIListBox *XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, const char *ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap);

	virtual LWEUI &UpdateSelf(LWEUIManager *Manager, float Scale, uint64_t lCurrentTime);

	virtual LWEUI &DrawSelf(LWEUIManager *Manager, LWEUIFrame *Frame, float Scale, uint64_t lCurrentTime);

	bool PushItem(const LWText &ItemName, void *UserData, LWEUIMaterial *OffMat = nullptr, LWEUIMaterial *OverMat = nullptr, LWEUIMaterial *DownMat = nullptr);

	bool Clear(void);

	bool RemoveItem(uint32_t i);

	LWEUIListBox &SetBackgroundMaterial(LWEUIMaterial *Mat);

	LWEUIListBox &SetOverMaterial(LWEUIMaterial *Mat);

	LWEUIListBox &SetOffMaterial(LWEUIMaterial *Mat);

	LWEUIListBox &SetDownMaterial(LWEUIMaterial *Mat);

	LWEUIListBox &SetFontMaterial(LWEUIMaterial *Mat);

	LWEUIListBox &SetFont(LWFont *Font);

	LWEUIListBox &SetScroll(float Scroll, float Scale=1.0f);

	LWEUIListBox &SetFontScale(float FontScale);

	LWEUIListBox &SetMinimumHeightSize(float MinimumHeight);

	LWEUIListBox &SetBorderSize(float BorderSize);

	LWEUIListBoxItem *GetItem(uint32_t i);

	uint32_t GetItemCount(void) const;

	uint32_t GetItemOver(void) const;

	float GetScroll(void) const;

	float GetBorderSize(void) const;

	LWFont *GetFont(void);

	LWEUIMaterial *GetBackgroundMaterial(void);

	LWEUIMaterial *GetOffMaterial(void);

	LWEUIMaterial *GetDownMaterial(void);

	LWEUIMaterial *GetOverMaterial(void);

	LWEUIMaterial *GetFontMaterial(void);

	float GetFontScale(void) const;

	float GetScrollPageSize(void) const;

	float GetScrollMaxSize(float Scale=1.0f) const;

	float GetCellHeight(float Scale = 1.0f) const;

	LWEUIListBox(LWEUIMaterial *BackgroundMaterial, LWEUIMaterial *OffMaterial, LWEUIMaterial *OverMaterial, LWEUIMaterial *DownMaterial, LWEUIMaterial *FontMaterial, LWFont *Font, float MinimumHeight, float BorderSize, const LWVector4f &Position, const LWVector4f &Size, uint32_t Flag);

	~LWEUIListBox();
private:
	LWEUIListBoxItem m_ItemList[LWELISTBOX_MAXITEMS];
	LWEUIMaterial *m_BackgroundMaterial;
	LWEUIMaterial *m_OffMaterial;
	LWEUIMaterial *m_DownMaterial;
	LWEUIMaterial *m_OverMaterial;
	LWEUIMaterial *m_FontMaterial;
	LWFont *m_Font;
	uint32_t m_ItemCount;
	uint32_t m_OverItem;
	float m_FontScale = 1.0f;
	float m_MinimumBoxHeight;
	float m_BorderSize;
	float m_Scroll;
	float m_InitScroll;
	float m_ScrollAcceleration;
};

#endif
