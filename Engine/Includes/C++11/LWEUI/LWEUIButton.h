#ifndef LWEUIBUTTON_H
#define LWEUIBUTTON_H
#include "LWEUI/LWEUI.h"

class LWEUIButton : public LWEUI {
public:


	/*!< \brief parses an button label, in addition to LWEUI attributes, LWEUIButton also takes the following attributes:
		 OverMaterial: The material to use when the button is being moused over(or in navigation mode is focused).
		 DownMaterial: The material to use when the button is being pressed on.
		 OffMaterial: The material to use when the button is not being moused over or pressed on.
	*/
	static LWEUIButton *XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, LWEXMLNode *Style, const char *ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap);

	virtual LWEUI &UpdateSelf(LWEUIManager &Manager, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime);

	virtual LWEUI &DrawSelf(LWEUIManager &Manager, LWEUIFrame &Frame, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime);

	virtual void Destroy(void);

	LWEUIButton &SetOverMaterial(LWEUIMaterial *OverMaterial);

	LWEUIButton &SetOffMaterial(LWEUIMaterial *OffMaterial);

	LWEUIButton &SetDownMaterial(LWEUIMaterial *DownMaterial);

	LWEUIMaterial *GetOverMaterial(void);

	LWEUIMaterial *GetOffMaterial(void);

	LWEUIMaterial *GetDownMaterial(void);

	LWEUIButton(LWEUIMaterial *OverMaterial, LWEUIMaterial *OffMaterial, LWEUIMaterial *DownMaterial, const LWVector4f &Position, const LWVector4f &Size, uint64_t Flag);

	~LWEUIButton();

private:
	LWEUIMaterial *m_OverMaterial;
	LWEUIMaterial *m_OffMaterial;
	LWEUIMaterial *m_DownMaterial;
};
#endif
