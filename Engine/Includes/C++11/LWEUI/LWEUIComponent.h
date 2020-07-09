#ifndef LWEUICOMPONENT_H
#define LWEUICOMPONENT_H
#include "LWEUI/LWEUI.h"

class LWEUIComponent : public LWEUI {
public:
	static const uint32_t MaxComponentCount = 16;

	static LWEUIComponent *XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, LWEXMLNode *Style, const char *ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap);

	virtual LWEUI &UpdateSelf(LWEUIManager &Manager, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime);

	virtual LWEUI &DrawSelf(LWEUIManager &Manager, LWEUIFrame &Frame, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime);

	virtual void Destroy(void);

	bool PushComponent(LWEUI *Component);

	LWEUIComponent(const LWVector4f &Position, const LWVector4f &Size, uint64_t Flag);

	LWEUIComponent();
private:

	LWEUI *m_ComponentList[MaxComponentCount];
	uint32_t m_ComponentCount = 0;
};


#endif
