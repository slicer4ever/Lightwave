#ifndef LWEUIADVLABEL_H
#define LWEUIADVLABEL_H
#include "LWEUI/LWEUI.h"

struct LWETextPart {
	char m_Text[256];
	LWVector4f m_ColorMult;
	float m_Scale;
	bool m_LineEnd;
};

class LWEUIAdvLabel : public LWEUI {
public:
	enum {
		MaxParts = 16
	};
	static LWEUIAdvLabel *XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, const char *ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap);

	virtual LWEUI &UpdateSelf(LWEUIManager *Manager, float Scale, uint64_t lCurrentTime);

	virtual LWEUI &DrawSelf(LWEUIManager *Manager, LWEUIFrame *Frame, float Scale, uint64_t lCurrentTime);

	LWEUIAdvLabel &SetText(const LWText &Text);

	LWEUIAdvLabel &SetTextf(const char *Format, ...);

	LWEUIAdvLabel &SetFont(LWFont *Font);

	LWEUIAdvLabel &SetMaterial(LWEUIMaterial *Material);

	LWEUIAdvLabel &SetFontScale(float Scale);

	LWFont *GetFont(void);

	LWEUIMaterial *GetMaterial(void);

	float GetFontScale(void) const;

	const LWETextPart &GetTextPart(uint32_t i) const;

	uint32_t GetTextPartCount(void) const;

	LWEUIAdvLabel(const LWText &Text, LWFont *Font, LWEUIMaterial *Material, const LWVector4f &Position, const LWVector4f &Size, uint32_t Flag);

	LWEUIAdvLabel();

	~LWEUIAdvLabel();
private:
	LWETextPart m_TextParts[MaxParts];
	float m_LineLength[MaxParts];
	LWFont *m_Font;
	LWEUIMaterial *m_Material;
	uint32_t m_TextPartCnt;
	float m_FontScale;
	float m_UnderHang;

};


#endif
