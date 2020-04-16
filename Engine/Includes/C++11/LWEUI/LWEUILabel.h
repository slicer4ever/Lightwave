#ifndef LWEUILABEL_H
#define LWEUILABEL_H
#include "LWEUI/LWEUI.h"

class LWEUILabel : public LWEUI {
public:
	/*!< \brief parses a label, in addition to LWEUI attributes, LWEUILabel also takes the following attributes:
		  Font: Named font in AssetManager to use for the label.
		  Value: the text to contain and draw.
		  ValueSrc: Alternative value loaded from the specified file source, and appears in it's entirety.
		  Material: Taking the color component as the default color to draw the text in(Texture component of the material is ignored.)
		  Scale: How much to scale the font text by.
	*/

	static LWEUILabel *XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, const char *ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap);

	virtual LWEUI &UpdateSelf(LWEUIManager &Manager, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime);

	virtual LWEUI &DrawSelf(LWEUIManager &Manager, LWEUIFrame &Frame, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime);

	virtual void Destroy(void);

	LWEUILabel &SetText(const LWText &Text);

	LWEUILabel &SetTextf(const char *Format, ...);

	LWEUILabel &SetFont(LWFont *Font);

	LWEUILabel &SetMaterial(LWEUIMaterial *Material);

	LWEUILabel &SetFontScale(float Scale);

	LWFont *GetFont(void);

	LWEUIMaterial *GetMaterial(void);

	float GetFontScale(void) const;

	const LWText &GetText(void) const;

	LWEUILabel(const LWText &Text, LWFont *Font, LWAllocator &Allocator, LWEUIMaterial *Material, const LWVector4f &Position, const LWVector4f &Size, uint64_t Flag);

	LWEUILabel();

	~LWEUILabel();
private:
	LWText m_Text;
	LWFont *m_Font;
	LWEUIMaterial *m_Material;
	LWVector2f m_TextSize;
	float m_FontScale;
	float m_UnderHang;

};


#endif
