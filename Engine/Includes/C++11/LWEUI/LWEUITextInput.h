#ifndef LWEUITEXTINPUT_H
#define LWEUITEXTINPUT_H
#include "LWEUI/LWEUI.h"

#define LWETEXTINPUT_MAXLINELENGTH 256
#define LWETEXTINPUT_MAXLINES 32

struct LWETextLine {
	char m_Value[LWETEXTINPUT_MAXLINELENGTH];
	uint32_t m_RawLength;
	uint32_t m_CharLength;
};

struct LWETextInputTouchBtn {
	enum {
		ButtonOver = 0x1,
		ButtonDown = 0x2
	};
	LWVector2f m_Position;
	LWVector2f m_Size;
	uint32_t m_Flag;
};

struct LWETextInputCursor {
	uint32_t m_Line = 0;
	uint32_t m_Position = 0;
	uint32_t m_RawPosition = 0;

	LWETextInputCursor(uint32_t Line, uint32_t Position, uint32_t RawPosition);

	//Calculates the Position/RawPosition of the line from the beginning value to the specified position.
	LWETextInputCursor(uint32_t Line, const char *LineBegin, const char *LinePosition);

	LWETextInputCursor() = default;
};

class LWEUITextInput : public LWEUI {
public:

	/*!< \brief parses an TextInput ui element, in addition to LWEUI attributes, LWEUITextInput also takes the following attributes:
		 BorderMaterial: The material to use for the border around the edge of the TextInput.
		 FontMaterial: The material color to be used for the text in the textinput.
		 DefaultMaterial: The material color to be used for the default text in the textinput.
		 SelectMaterial: The material to use when drawing the selection container over top text.
		 TextAreaMaterial: The material to use for the actual textarea the text is drawn on top of.
		 CursorMaterial: The material to use for the cursor.
		 AllowedCharacters: The characters which the textinput is limited to using, if not specified all input's are considered allowed.
		 Font: The named font in LWEUIAssetManager to use for the text area(also used for the touch input buttons)
		 Value: An initial starting value for the text input.
		 Default: Centered text which is drawn when no input has been entered and the text input is not focused.
		 BorderSize: The size of each edge of the border around the textarea.
		 CursorSize: The width of the cursor(the height is calculated automatically), if not specified the default value is 1.
		 MaxLength: The maximum number of characters the textinput can take(if unspecified uses the internal max limits).
		 MaxLines: The maximum number of lines the textinput can have(if unspecified uses the internal max limits).
		 FontScale: The scale the text should be drawn at.
		 DefaultFontScale: The scale the default text should be drawn at(will be equal to FontScale if not set.)
		 OffMaterial: The touch buttons off material, when touch input is being responded to a series of (Copy, Cut, Paste, SelectAll buttons are drawn above the text input.
		 OverMaterial: The touch buttons over material.
		 DownMaterial: The touch buttons down material.
		 ButtonFontScale: The scale for the font of the touch buttons to use.
		 ButtonFontMaterial: The colored material to use for the touch buttons text.
	*/
	static LWEUITextInput *XMLParse(LWEXMLNode *Node, LWEXML *XML, LWEUIManager *Manager, const char *ActiveComponentName, LWEXMLNode *ActiveComponent, LWEXMLNode *ActiveComponentNode, std::map<uint32_t, LWEXMLNode*> &StyleMap, std::map<uint32_t, LWEXMLNode*> &ComponentMap);

	LWEUITextInput &UpdateTouchButtons(LWEUIManager &Manager, float Scale, uint64_t lCurrentTime);

	LWEUITextInput &DrawTouchButtons(LWEUIManager &Manager, LWEUIFrame &Frame, float Scale, uint64_t lCurrentTime);

	virtual LWEUI &UpdateSelf(LWEUIManager &Manager, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime);

	virtual LWEUI &DrawSelf(LWEUIManager &Manager, LWEUIFrame &Frame, float Scale, const LWVector2f &ParentVisiblePos, const LWVector2f &ParentVisibleSize, LWVector2f &VisiblePos, LWVector2f &VisibleSize, uint64_t lCurrentTime);

	virtual void Destroy(void);

	LWEUITextInput &CalculateTouchBtnPositions(float Scale);

	int32_t GetCursorDistance(const LWETextInputCursor &ACursor, const LWETextInputCursor &BCursor, int32_t &RawDistance);

	int32_t MoveCursorBy(LWETextInputCursor &Cursor, int32_t Distance);

	bool MoveCursorUp(LWETextInputCursor &Cursor, float UIScale);

	bool MoveCursorDown(LWETextInputCursor &Cursor, float UIScale);

	bool MoveCursorLeft(LWETextInputCursor &Cursor);

	bool MoveCursorRight(LWETextInputCursor &Cursor);

	bool ProcessKey(LWKey Key, bool ShiftDown, bool CtrlDown, float Scale);

	uint32_t InsertText(const LWText &Text, bool ShiftDown, bool CtrlDown, float Scale);

	uint32_t InsertTextf(const char *Fmt, bool ShiftDown, bool CtrlDown, float Scale, ...);

	bool InsertChar(uint32_t Char, bool ShiftDown, bool CtrlDown, float Scale);

	bool ScrollToCursor(float Scale);

	bool SetScroll(float HoriScroll, float VertScroll, float Scale);

	LWEUITextInput &Clear(void);

	LWEUITextInput &SetAllowedCharacters(const char *Characters);

	LWEUITextInput &SetDefaultText(const char *Text);

	LWEUITextInput &SetBorderSize(float BorderSize);

	LWEUITextInput &SetCursorSize(float CursorSize);

	LWEUITextInput &SetFontScale(float FontScale);

	LWEUITextInput &SetMaxLength(uint32_t MaxLength);

	LWEUITextInput &SetMaxLines(uint32_t MaxLines);

	LWEUITextInput &SetCursorPosition(uint32_t CursorPosition, uint32_t CursorLine);

	LWEUITextInput &SetBorderMaterial(LWEUIMaterial *BorderMaterial);

	LWEUITextInput &SetFontMaterial(LWEUIMaterial *FontMaterial);

	LWEUITextInput &SetDefaultMaterial(LWEUIMaterial *DefMaterial);

	LWEUITextInput &SetSelectMaterial(LWEUIMaterial *SelectMaterial);

	LWEUITextInput &SetTextAreaMaterial(LWEUIMaterial *TextAreaMaterial);

	LWEUITextInput &SetCursorMaterial(LWEUIMaterial *CursorMaterial);

	LWEUITextInput &SetBtnDownMaterial(LWEUIMaterial *BtnDownMaterial);

	LWEUITextInput &SetBtnOverMaterial(LWEUIMaterial *BtnOverMaterial);

	LWEUITextInput &SetBtnOffMaterial(LWEUIMaterial *BtnOffMaterial);

	LWEUITextInput &SetBtnFontMaterial(LWEUIMaterial *BtnFontMaterial);

	LWEUITextInput &SetBtnFontScale(float FontScale);

	LWEUITextInput &SetDefaultFontScale(float FontScale);

	LWEUITextInput &SetFont(LWFont *Font);

	LWEUITextInput &SetSelectRange(uint32_t Position, uint32_t Line, int32_t Length);

	uint32_t GetSelectedText(char *Buffer, uint32_t BufferLen);

	const LWETextLine *GetLine(uint32_t Line) const;

	LWEUIMaterial *GetBorderMaterial(void) const;

	LWEUIMaterial *GetFontMaterial(void) const;

	LWEUIMaterial *GetSelectMaterial(void) const;

	LWEUIMaterial *GetDefaultMaterial(void) const;

	LWEUIMaterial *GetTextAreaMaterial(void) const;

	LWEUIMaterial *GetCursorMaterial(void) const;

	LWFont *GetFont(void) const;

	float GetBorderSize(void) const;

	float GetCursorSize(void) const;

	uint32_t GetMaxLength(void) const;

	uint32_t GetMaxLines(void) const;

	uint32_t GetCurrentLength(void) const;

	const LWETextInputCursor &GetCursor(void) const;

	const LWETextInputCursor &GetSelectCursor(void) const;

	LWETextInputCursor &GetCursor(void);

	LWETextInputCursor &GetSelectCursor(void);

	uint32_t GetLineCount(void) const;

	int32_t GetSelectCursorLength(void) const;

	float GetVerticalScroll(void) const;

	float GetHorizontalScroll(void) const;

	float GetFontScale(void) const;

	float GetDefaultFontScale(void) const;

	bool isPasswordField(void) const;

	const char *GetAllowedCharacters(void) const;

	const char *GetDefaultText(void) const;

	LWEUITextInput(const LWVector4f &Position, const LWVector4f &Size, uint64_t Flag);

	~LWEUITextInput();
private:
	LWETextLine m_Lines[LWETEXTINPUT_MAXLINES];
	char m_AllowedCharacters[LWETEXTINPUT_MAXLINELENGTH];
	char m_DefaultText[LWETEXTINPUT_MAXLINELENGTH];
	LWEUIMaterial *m_BorderMaterial = nullptr;
	LWEUIMaterial *m_FontMaterial = nullptr;
	LWEUIMaterial *m_SelectMaterial = nullptr;
	LWEUIMaterial *m_TextAreaMaterial = nullptr;
	LWEUIMaterial *m_CursorMaterial = nullptr;
	LWEUIMaterial *m_DefaultMaterial = nullptr;
	LWEUIMaterial *m_BtnOffMaterial = nullptr;
	LWEUIMaterial *m_BtnOverMaterial = nullptr;
	LWEUIMaterial *m_BtnDownMaterial = nullptr;
	LWEUIMaterial *m_BtnFontMaterial = nullptr;
	LWFont *m_Font = nullptr;
	LWETextInputTouchBtn m_CopyBtn;
	LWETextInputTouchBtn m_PasteBtn;
	LWETextInputTouchBtn m_CutBtn;
	LWETextInputTouchBtn m_SelectAllBtn;
	LWETextInputCursor m_Cursor;
	LWETextInputCursor m_SelectCursor;
	LWVector2f m_ScrollAcceleration = LWVector2f();
	float m_VerticalScroll = 0.0f;
	float m_HorizontalScroll = 0.0f;
	float m_BorderSize = 0.0f;
	float m_CursorSize = 0.0f;
	float m_FontScale = 1.0f;
	float m_DefaultFontScale = 1.0f;
	float m_FontBtnScale = 1.0f;
	float m_LongestLine = 0.0f;
	uint32_t m_MaxLength = 0xFFFFFFFF;
	uint32_t m_MaxLines = 0xFFFFFFFF;
	int32_t m_SelectCursorLength = 0;
	uint32_t m_CurrentLength = 0;
	uint32_t m_LineCount = 1;
};


#endif

