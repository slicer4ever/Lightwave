#ifndef LWEUIADVLABEL_H
#define LWEUIADVLABEL_H
#include "LWEUI/LWEUI.h"
#include <unordered_map>

struct LWEUITextStyle {
	LWVector4f m_ColorMult = LWVector4f(1.0f); //Text color multiplier with the LWEUIMaterial ColorA.
	LWVector4f m_BackgroundColorMult = LWVector4f(0.0f); //Text background color multiplier with the LWEUIMaterial ColorA.
	LWVector4f m_Bounds = LWVector4f(); //Raw bounds of the text.
	LWVector4f m_VisibileBounds = LWVector4f(); //Visible bounds of the text.
	float m_Scale = 1.0f;
	uint32_t m_Offset = 0;
	uint32_t m_Length = 0;
	uint32_t m_CallbackID = -1; //Callback id when the mouse over's/off/presses events.
	uint64_t m_Flag = 0;
};

typedef std::function<void(LWEUIRichLabel &, LWEUITextStyle &, uint32_t, LWEUIManager &)> LWEUIRichLabelCallback;

class LWEUIRichLabel : public LWEUI {
public:
	static const uint32_t MinimumBufferSize = 64;

	/*!< \brief parses an RichLabel, which support different coloring, and sizing of the text, in addition to LWEUI attributes, LWEUIAdvLabel also takes the following attributes:
		  Font: Named font in AssetManager to use for the label.
		  Value: the text to contain and draw.
		  ValueSrc: Alternative value loaded from the specified file source, and appears in it's entirety.
		  Material: Taking the color component as the default color to draw the text in(Texture component of the material is ignored.)
		  Scale: How much to scale the font text by.
	*/
	static LWEUIRichLabel *XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, const char *ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap);

	virtual LWEUI &UpdateSelf(LWEUIManager &Manager, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime);

	virtual LWEUI &DrawSelf(LWEUIManager &Manager, LWEUIFrame &Frame, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime);

	virtual void Destroy(void);

	template<class Method, class Obj>
	LWEUIRichLabel &RegisterMethodCallback(uint32_t CallbackID, Method Callback, Obj *O) {
		return RegisterCallback(CallbackID, std::bind(Callback, O, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	}

	LWEUIRichLabel &RegisterCallback(uint32_t CallbackID, LWEUIRichLabelCallback Callback);

	LWEUIRichLabel &UnregisterCallback(uint32_t CallbackID);

	/*!< \brief set's the text, and parses for rich text tags. The following tags are processed:
		  [#Hex]Set's the color Multiplier with material for the text color.
		  [B#Hex]Set's the Background color multiplier for the text color.
		  [Number]Set's the scale of the text.
		  [$Number]Registers the text callback id which if a callback is set with that id will call the callback when the mouse goes over/off/presses/released(+right+middle variants).
		  [/]Ends the current callback id.
	*/
	LWEUIRichLabel &SetText(const LWText &Text);

	LWEUIRichLabel &SetTextf(const char *Format, ...);

	LWEUIRichLabel &SetFont(LWFont *Font);

	LWEUIRichLabel &SetMaterial(LWEUIMaterial *Material);

	LWEUIRichLabel &SetFontScale(float Scale);

	LWFont *GetFont(void);

	LWEUIMaterial *GetMaterial(void);

	float GetFontScale(void) const;

	const LWEUITextStyle &GetTextStyle(uint32_t i) const;

	uint32_t GetStyleCount(void) const;

	LWVector2f GetLineSize(uint32_t i) const;

	uint32_t GetLineCount(void) const;
	
	const char *GetText(void) const;

	LWEUIRichLabel(const LWText &Text, LWFont *Font, LWAllocator &Allocator, LWEUIMaterial *Material, const LWVector4f &Position, const LWVector4f &Size, uint64_t Flag);

	LWEUIRichLabel();

	~LWEUIRichLabel();
private:
	std::vector<LWEUITextStyle> m_StyleList;
	std::vector<LWVector2f> m_LineSizes;
	std::unordered_map<uint32_t, LWEUIRichLabelCallback> m_CallbackMap;
	LWVector2f m_TextSize;
	char *m_TextBuffer;
	LWFont *m_Font;
	LWEUIMaterial *m_Material;
	uint32_t m_BufferLength;
	float m_FontScale;
	float m_Overhang;

};


#endif
