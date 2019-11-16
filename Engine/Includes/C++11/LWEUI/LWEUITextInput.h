#ifndef LWEUITEXTINPUT_H
#define LWEUITEXTINPUT_H
#include "LWEUI/LWEUI.h"

#define LWETEXTINPUT_MAXLINELENGTH 256
#define LWETEXTINPUT_MAXLINES 32

struct LWTextLine {
	char m_Value[LWETEXTINPUT_MAXLINELENGTH];
	uint32_t m_RawLength;
	uint32_t m_CharLength;
};


class LWEUITextInput : public LWEUI {
public:
	static LWEUITextInput *XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, const char *ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap);

	virtual LWEUI &UpdateSelf(LWEUIManager *Manager, float Scale, uint64_t lCurrentTime);

	virtual LWEUI &DrawSelf(LWEUIManager *Manager, LWEUIFrame *Frame, float Scale, uint64_t lCurrentTime);

	LWEUITextInput &CalculateBtnPositions(LWVector2f &CopyBtn, LWVector2f &CutBtn, LWVector2f &PasteBtn, LWVector2f &SelectAllBtn, float Scale);

	bool ProcessKey(LWKey Key, bool ShiftDown, bool CtrlDown, float Scale);

	uint32_t InsertText(const LWText &Text, bool ShiftDown, bool CtrlDown, float Scale);

	uint32_t InsertTextf(const char *Fmt, bool ShiftDown, bool CtrlDown, float Scale, ...);

	bool InsertChar(uint32_t Char, bool ShiftDown, bool CtrlDown, float Scale);

	bool ScrollToCursor(float Scale);

	bool SetScroll(float HoriScroll, float VertScroll, float Scale);

	LWEUITextInput &Clear(void);

	LWEUITextInput &SetAllowedCharacters(const char *Characters);

	LWEUITextInput &SetBorderSize(float BorderSize);

	LWEUITextInput &SetCursorSize(float CursorSize);

	LWEUITextInput &SetFontScale(float FontScale);

	LWEUITextInput &SetMaxLength(uint32_t MaxLength);

	LWEUITextInput &SetMaxLines(uint32_t MaxLines);

	LWEUITextInput &SetCursorPosition(uint32_t CursorPosition, uint32_t CursorLine);

	LWEUITextInput &SetBorderMaterial(LWEUIMaterial *BorderMaterial);

	LWEUITextInput &SetFontMaterial(LWEUIMaterial *FontMaterial);

	LWEUITextInput &SetSelectMaterial(LWEUIMaterial *SelectMaterial);

	LWEUITextInput &SetTextAreaMaterial(LWEUIMaterial *TextAreaMaterial);

	LWEUITextInput &SetCursorMaterial(LWEUIMaterial *CursorMaterial);

	LWEUITextInput &SetBtnDownMaterial(LWEUIMaterial *BtnDownMaterial);

	LWEUITextInput &SetBtnOverMaterial(LWEUIMaterial *BtnOverMaterial);

	LWEUITextInput &SetBtnOffMaterial(LWEUIMaterial *BtnOffMaterial);

	LWEUITextInput &SetBtnFontMaterial(LWEUIMaterial *BtnFontMaterial);

	LWEUITextInput &SetBtnFontScale(float FontScale);

	LWEUITextInput &SetFont(LWFont *Font);

	LWEUITextInput &SetSelectRange(uint32_t Position, uint32_t Line, int32_t Length);

	uint32_t GetSelectedText(char *Buffer, uint32_t BufferLen);

	const LWTextLine *GetLine(uint32_t Line) const;

	LWEUIMaterial *GetBorderMaterial(void) const;

	LWEUIMaterial *GetFontMaterial(void) const;

	LWEUIMaterial *GetSelectMaterial(void) const;

	LWEUIMaterial *GetTextAreaMaterial(void) const;

	LWEUIMaterial *GetCursorMaterial(void) const;

	LWFont *GetFont(void) const;

	float GetBorderSize(void) const;

	float GetCursorSize(void) const;

	uint32_t GetMaxLength(void) const;

	uint32_t GetMaxLines(void) const;

	uint32_t GetCurrentLength(void) const;

	uint32_t GetCursorPosition(void) const;

	uint32_t GetCursorRawPosition(void) const;

	uint32_t GetCursorLine(void) const;

	uint32_t GetLineCount(void) const;

	uint32_t GetSelectCusorPosition(void) const;

	uint32_t GetSelectCursorRawPosition(void) const;

	uint32_t GetSelectCusorLine(void) const;

	int32_t GetSelectCursorLength(void) const;

	float GetVerticalScroll(void) const;

	float GetHorizontalScroll(void) const;

	float GetFontScale(void) const;

	const char *GetAllowedCharacters(void) const;

	LWEUITextInput(const LWVector4f &Position, const LWVector4f &Size, uint32_t Flag);

	LWEUITextInput();

	~LWEUITextInput();
private:
	LWTextLine m_Lines[LWETEXTINPUT_MAXLINES];
	char m_AllowedCharacters[LWETEXTINPUT_MAXLINELENGTH];
	LWEUIMaterial *m_BorderMaterial = nullptr;
	LWEUIMaterial *m_FontMaterial = nullptr;
	LWEUIMaterial *m_SelectMaterial = nullptr;
	LWEUIMaterial *m_TextAreaMaterial = nullptr;
	LWEUIMaterial *m_CursorMaterial = nullptr;
	LWEUIMaterial *m_BtnOffMaterial = nullptr;
	LWEUIMaterial *m_BtnOverMaterial = nullptr;
	LWEUIMaterial *m_BtnDownMaterial = nullptr;
	LWEUIMaterial *m_BtnFontMaterial = nullptr;
	LWEUIMaterial *m_CopyMaterial = nullptr;
	LWEUIMaterial *m_CutMaterial = nullptr;
	LWEUIMaterial *m_PasteMaterial = nullptr;
	LWEUIMaterial *m_SelectAllMaterial = nullptr;
	LWFont *m_Font = nullptr;
	LWVector4f m_CopyBtn = LWVector4f();
	LWVector4f m_PasteBtn = LWVector4f();
	LWVector4f m_CutBtn = LWVector4f();
	LWVector4f m_SelectAllBtn = LWVector4f();
	LWVector2f m_ScrollAcceleration = LWVector2f();
	float m_VerticalScroll = 0.0f;
	float m_HorizontalScroll = 0.0f;
	float m_BorderSize = 0.0f;
	float m_CursorSize = 0.0f;
	float m_FontScale = 1.0f;
	float m_FontBtnScale = 1.0f;
	float m_LongestLine = 0.0f;
	uint32_t m_MaxLength = 0xFFFFFFFF;
	uint32_t m_MaxLines = 0xFFFFFFFF;
	uint32_t m_SelectCursorPosition = 0;
	uint32_t m_SelectCursorRawPosition = 0;
	uint32_t m_SelectCursorLine = 0;
	int32_t m_SelectCursorLength = 0;
	uint32_t m_CurrentLength = 0;
	uint32_t m_CursorLine = 0;
	uint32_t m_CursorPosition = 0;
	uint32_t m_CursorRawPosition = 0;
	uint32_t m_LineCount = 1;
};


#endif

