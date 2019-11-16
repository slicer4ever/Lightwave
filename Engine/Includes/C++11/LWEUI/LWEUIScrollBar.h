#ifndef LWEUISCROLLBAR_H
#define LWEUISCROLLBAR_H
#include "LWEUI/LWEUI.h"

class LWEUIScrollBar : public LWEUI {
public:
	static LWEUIScrollBar *XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, const char *ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap);

	virtual LWEUI &UpdateSelf(LWEUIManager *Manager, float Scale, uint64_t lCurrentTime);

	virtual LWEUI &DrawSelf(LWEUIManager *Manager, LWEUIFrame *Frame, float Scale, uint64_t lCurrentTime);

	LWEUIScrollBar &SetBarOffMaterial(LWEUIMaterial *Material);

	LWEUIScrollBar &SetBarDownMaterial(LWEUIMaterial *Material);

	LWEUIScrollBar &SetBarOverMaterial(LWEUIMaterial *Material);

	LWEUIScrollBar &SetBackgroundMaterial(LWEUIMaterial *Material);

	LWEUIScrollBar &SetScroll(float Scroll);

	LWEUIScrollBar &SetMaxScroll(float MaxScroll);

	LWEUIScrollBar &SetScrollSize(float ScrollSize);

	LWEUIMaterial *GetBarOffMaterial(void);

	LWEUIMaterial *GetBarDownMaterial(void);

	LWEUIMaterial *GetBarOverMaterial(void);

	LWEUIMaterial *GetBackgroundMaterial(void);

	float GetScroll(void);

	float GetMaxScroll(void);

	float GetScrollSize(void);

	LWEUIScrollBar(LWEUIMaterial *BarOffMaterial, LWEUIMaterial *BarOverMaterial, LWEUIMaterial *BarDownMaterial, LWEUIMaterial *BackgroundMaterial, float MaxScroll, float ScrollSize, const LWVector4f &Position, const LWVector4f &Size, uint32_t Flag);

	~LWEUIScrollBar();
private:
	LWEUIMaterial *m_BarOffMaterial;
	LWEUIMaterial *m_BarOverMaterial;
	LWEUIMaterial *m_BarDownMaterial;
	LWEUIMaterial *m_BackgroundMaterial;
	float m_InitialScroll;
	float m_Scroll;
	float m_MaxScroll;
	float m_ScrollSize;
};

#endif
