#ifndef LWEUICOMPONENT_H
#define LWEUICOMPONENT_H
#include "LWEUI/LWEUI.h"

class LWEUIComponent : public LWEUI {
public:
	enum {
		MaxComponents = 32
	};
	LWEUIComponent &PushComponent(LWEUI *UI);

	virtual LWEUI &UpdateSelf(LWEUIManager *Manager, float Scale, uint64_t lCurrentTime);

	virtual LWEUI &DrawSelf(LWEUIManager *Manager, LWEUIFrame *Frame, float Scale, uint64_t lCurrentTime);

	LWEUI *GetComponent(uint32_t i);

	uint32_t GetComponentCount(void);

	LWEUIComponent(const LWVector4f &Position, const LWVector4f &Size, uint32_t Flag);

	LWEUIComponent();
private:
	LWEUI *m_ComponentList[MaxComponents];
	uint32_t m_ComponentCount;
};


#endif
