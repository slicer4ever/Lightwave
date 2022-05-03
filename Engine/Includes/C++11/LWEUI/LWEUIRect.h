#ifndef LWEUIRECT_H
#define LWEUIRECT_H
#include "LWEUI/LWEUI.h"

class LWEUIRect : public LWEUI {
public:

	/*!< \brief parses an rect ui element, in addition to LWEUI attributes, LWEUIRect also takes the following attributes:
		 Material: The material to use when drawing the rectangle(no material can be specified if the UIRect is to be used just for positioning/sizing child elements).
		 Theta: Angle in radians the rect is angled.
	*/
	static LWEUIRect *XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, LWEXMLNode *Style, const LWUTF8Iterator &ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap);

	virtual LWEUI &UpdateSelf(LWEUIManager &Manager, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime);

	virtual LWEUI &DrawSelf(LWEUIManager &Manager, LWEUIFrame &Frame, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime);

	virtual void Destroy(void);

	LWEUIRect &SetMaterial(LWEUIMaterial *Material);

	/*!< \brief set's a visible rotation to the rectangle rotated around it's center point, this does not change the actual dimension of the rectangle, and is only a visual effect.
		 \param Theta input expected as radians.
	*/
	LWEUIRect &SetTheta(float Theta);

	LWEUIMaterial *GetMaterial(void);

	float GetTheta(void) const;

	LWEUIRect(LWEUIMaterial *Material, const LWVector4f &Position, const LWVector4f &Size, uint64_t Flag);

	LWEUIRect();

	~LWEUIRect();
private:
	LWEUIMaterial *m_Material = nullptr;
	float m_Theta = 0.0f;
};


#endif
