#ifndef LWEUILISTBOX_H
#define LWEUILISTBOX_H
#include "LWEUI/LWEUI.h"

#define LWELISTBOX_MAXLENGTH 256
#define LWELISTBOX_MAXITEMS 256

struct LWEUIListBoxItem {
	char m_Name[LWELISTBOX_MAXITEMS];
	LWVector2f m_TextSize;
	float m_TextUnderhang;
	void *m_UserData;
	LWEUIMaterial *m_OffMaterial;
	LWEUIMaterial *m_OverMaterial;
	LWEUIMaterial *m_DownMaterial;

	LWUTF8Iterator GetName(void) const;
	
	LWEUIListBoxItem(const LWUTF8Iterator &Name, const LWVector2f &TextSize, float TextUnderHang = 0.0f, void *UserData = nullptr, LWEUIMaterial *OffMaterial = nullptr, LWEUIMaterial *OverMaterial = nullptr, LWEUIMaterial *DownMaterial = nullptr);

	LWEUIListBoxItem() = default;
};

class LWEUIListBox : public LWEUI {
public:
	static const uint32_t NullItem = -1;

	/*!< \brief parses a ListBox, in addition to LWEUI attributes, LWEUIListBox also takes the following attributes:
		  OverMaterial: The material to use when the mouse is over a list item.
		  DownMaterial: The material to use when the mouse is pressed on a list item.
		  OffMaterial: The material to use when the mouse is not over the list item.
		  BackgroundMaterial: The material to use for the background of the list box.
		  Font: Named font in AssetManager to use for the text of each list item.
		  MinimumHeight: A minimum cell height for the list box.
		  BorderSize: The size of the border around the list box(in the same material as BackgroundMaterial).
		  Values: A list of text elements for each item, separated by a '|' symbol.
		  FontMaterial: Uses the color component of the material for what color to draw the text.
		  FontScale: the scale to draw the text of each list item at.
	*/
	static LWEUIListBox *XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, LWEXMLNode *Style, const LWUTF8Iterator &ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap);

	virtual LWEUI &UpdateSelf(LWEUIManager &Manager, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime);

	virtual LWEUI &DrawSelf(LWEUIManager &Manager, LWEUIFrame &Frame, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime);

	virtual void Destroy(void);

	bool PushItem(const LWUTF8Iterator &ItemName, void *UserData, LWEUIMaterial *OffMat = nullptr, LWEUIMaterial *OverMat = nullptr, LWEUIMaterial *DownMat = nullptr);

	bool PushItemf(const char *Fmt, void *UserData, LWEUIMaterial *OffMat, LWEUIMaterial *OverMat, LWEUIMaterial *DownMat, ...);

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

	LWEUIListBox(LWEUIMaterial *BackgroundMaterial, LWEUIMaterial *OffMaterial, LWEUIMaterial *OverMaterial, LWEUIMaterial *DownMaterial, LWEUIMaterial *FontMaterial, LWFont *Font, float MinimumHeight, float BorderSize, const LWVector4f &Position, const LWVector4f &Size, uint64_t Flag);

	~LWEUIListBox();
private:
	LWEUIListBoxItem m_ItemList[LWELISTBOX_MAXITEMS];
	LWEUIMaterial *m_BackgroundMaterial = nullptr;
	LWEUIMaterial *m_OffMaterial = nullptr;
	LWEUIMaterial *m_DownMaterial = nullptr;
	LWEUIMaterial *m_OverMaterial = nullptr;
	LWEUIMaterial *m_FontMaterial = nullptr;
	LWFont *m_Font = nullptr;
	uint32_t m_ItemCount = 0;
	uint32_t m_OverItem = -1;
	float m_FontScale = 1.0f;
	float m_MinimumBoxHeight = 0.0f;
	float m_BorderSize = 0.0f;
	float m_Scroll = 0.0f;
	float m_InitScroll = 0.0f;
	float m_ScrollAcceleration = 0.0f;
};

#endif
