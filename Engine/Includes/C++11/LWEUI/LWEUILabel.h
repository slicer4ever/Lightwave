#ifndef LWEUILABEL_H
#define LWEUILABEL_H
#include "LWEUI/LWEUI.h"

class LWEUILabel : public LWEUI {
public:
	static LWEUILabel *XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, const char *ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap);

	virtual LWEUI &UpdateSelf(LWEUIManager *Manager, float Scale, uint64_t lCurrentTime);

	virtual LWEUI &DrawSelf(LWEUIManager *Manager, LWEUIFrame *Frame, float Scale, uint64_t lCurrentTime);

	LWEUILabel &SetText(const LWText &Text);

	LWEUILabel &SetTextf(const char *Format, ...);

	LWEUILabel &SetFont(LWFont *Font);

	LWEUILabel &SetMaterial(LWEUIMaterial *Material);

	LWEUILabel &SetFontScale(float Scale);

	LWFont *GetFont(void);

	LWEUIMaterial *GetMaterial(void);

	float GetFontScale(void) const;

	const LWText &GetText(void) const;

	LWEUILabel(const LWText &Text, LWFont *Font, LWAllocator &Allocator, LWEUIMaterial *Material, const LWVector4f &Position, const LWVector4f &Size, uint32_t Flag);

	LWEUILabel();

	~LWEUILabel();
private:
	LWText m_Text;
	LWFont *m_Font;
	LWEUIMaterial *m_Material;
	float m_FontScale;
	float m_UnderHang;

};


#endif
