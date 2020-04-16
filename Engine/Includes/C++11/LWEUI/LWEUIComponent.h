#ifndef LWEUICOMPONENT_H
#define LWEUICOMPONENT_H
#include "LWEUI/LWEUI.h"

class LWEUIComponent : public LWEUI {
public:
	static const uint32_t MaxComponentCount = 16;

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
