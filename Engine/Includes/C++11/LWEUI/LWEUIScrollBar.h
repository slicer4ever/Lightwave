#ifndef LWEUISCROLLBAR_H
#define LWEUISCROLLBAR_H
#include "LWEUI/LWEUI.h"

class LWEUIScrollBar : public LWEUI {
public:

	/*!< \brief parses an scrollbar ui element, in addition to LWEUI attributes, LWEUIScrollBar also takes the following attributes:
		 OverMaterial: The material to use when the bar is being moused over(or in navigation mode is focused).
		 DownMaterial: The material to use when the bar is being pressed on.
		 OffMaterial: The material to use when the bar is not being moused over or pressed on.
		 BackgroundMaterial: The material to use for the entire scroll bar element.
		 Scroll: The current scroll position of the bar.
		 MaxScroll: The maximum scroll length.
		 ScrollSize: The size of the scrollbar relative to the MaxScroll, this is then multiplied by the entire size of the scrollbar to calculate the bar's size.
	*/
	static LWEUIScrollBar *XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, LWEXMLNode *Style, const LWUTF8Iterator &ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap);

	virtual LWEUI &UpdateSelf(LWEUIManager &Manager, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime);

	virtual LWEUI &DrawSelf(LWEUIManager &Manager, LWEUIFrame &Frame, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime);

	virtual void Destroy(void);

	LWEUIScrollBar &SetBarOffMaterial(LWEUIMaterial *Material);

	LWEUIScrollBar &SetBarDownMaterial(LWEUIMaterial *Material);

	LWEUIScrollBar &SetBarOverMaterial(LWEUIMaterial *Material);

	LWEUIScrollBar &SetBackgroundMaterial(LWEUIMaterial *Material);

	LWEUIScrollBar &SetScroll(float Scroll);

	LWEUIScrollBar &SetMaxScroll(float MaxScroll);

	LWEUIScrollBar &SetScrollSize(float ScrollSize);

	LWEUIScrollBar &SetDirection(bool isHorizontal);

	LWEUIMaterial *GetBarOffMaterial(void);

	LWEUIMaterial *GetBarDownMaterial(void);

	LWEUIMaterial *GetBarOverMaterial(void);

	LWEUIMaterial *GetBackgroundMaterial(void);

	bool isHorizontal(void) const;

	bool isVertical(void) const;

	float GetScroll(void) const;

	float GetMaxScroll(void) const;

	float GetScrollSize(void) const;

	LWEUIScrollBar(LWEUIMaterial *BarOffMaterial, LWEUIMaterial *BarOverMaterial, LWEUIMaterial *BarDownMaterial, LWEUIMaterial *BackgroundMaterial, float MaxScroll, float ScrollSize, const LWVector4f &Position, const LWVector4f &Size, uint64_t Flag);

	~LWEUIScrollBar();
private:
	LWEUIMaterial *m_BarOffMaterial = nullptr;
	LWEUIMaterial *m_BarOverMaterial = nullptr;
	LWEUIMaterial *m_BarDownMaterial = nullptr;
	LWEUIMaterial *m_BackgroundMaterial = nullptr;
	float m_InitialScroll = 0.0f;
	float m_Scroll = 0.0f;
	float m_MaxScroll;
	float m_ScrollSize;
};

#endif
