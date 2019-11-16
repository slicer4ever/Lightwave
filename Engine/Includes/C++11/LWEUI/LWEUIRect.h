#ifndef LWEUIRECT_H
#define LWEUIRECT_H
#include "LWEUI/LWEUI.h"

class LWEUIRect : public LWEUI {
public:
	static LWEUIRect *XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, const char *ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap);

	virtual LWEUI &UpdateSelf(LWEUIManager *Manager, float Scale, uint64_t lCurrentTime);

	virtual LWEUI &DrawSelf(LWEUIManager *Manager, LWEUIFrame *Frame, float Scale, uint64_t lCurrentTime);

	LWEUIRect &SetMaterial(LWEUIMaterial *Material);

	/*!< \brief set's a visible rotation to the rectangle rotated around it's center point, this does not change the actual dimension of the rectangle, and is only a visual effect.
		 \param Theta input expected as radians.
	*/
	LWEUIRect &SetTheta(float Theta);

	LWEUIMaterial *GetMaterial(void);

	float GetTheta(void) const;

	LWEUIRect(LWEUIMaterial *Material, const LWVector4f &Position, const LWVector4f &Size, uint32_t Flag);

	LWEUIRect();

	~LWEUIRect();
private:
	LWEUIMaterial *m_Material;
	float m_Theta;
};


#endif
