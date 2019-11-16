#ifndef LWEUIBUTTON_H
#define LWEUIBUTTON_H
#include "LWEUI/LWEUI.h"

class LWEUIButton : public LWEUI {
public:
	static LWEUIButton *XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, const char *ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap);

	virtual LWEUI &UpdateSelf(LWEUIManager *Manager, float Scale, uint64_t lCurrentTime);

	virtual LWEUI &DrawSelf(LWEUIManager *Manager, LWEUIFrame *Frame, float Scale, uint64_t lCurrentTime);

	LWEUIButton &SetOverMaterial(LWEUIMaterial *OverMaterial);

	LWEUIButton &SetOffMaterial(LWEUIMaterial *OffMaterial);

	LWEUIButton &SetDownMaterial(LWEUIMaterial *DownMaterial);

	LWEUIMaterial *GetOverMaterial(void);

	LWEUIMaterial *GetOffMaterial(void);

	LWEUIMaterial *GetDownMaterial(void);

	LWEUIButton(LWEUIMaterial *OverMaterial, LWEUIMaterial *OffMaterial, LWEUIMaterial *DownMaterial, const LWVector4f &Position, const LWVector4f &Size, uint32_t Flag);

	~LWEUIButton();

private:
	LWEUIMaterial *m_OverMaterial;
	LWEUIMaterial *m_OffMaterial;
	LWEUIMaterial *m_DownMaterial;
};
#endif
