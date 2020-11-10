#ifndef UITEXTINPUTTEST_H
#define UITEXTINPUTTEST_H
#include "UIToolkit.h"
#include <LWEUI/LWEUIRichLabel.h>
#include <LWEUI/LWEUITreeList.h>

struct UITextInputTest : public UIItem {

	void PasswordToggled(UIToggle &, bool Toggled, void*);

	void TextDialogChanged(LWEUI *, uint32_t, void *);

	void ScrollBarChanged(LWEUI *SB, uint32_t, void*);

	void ClearBtn(LWEUI*, uint32_t, void*);

	void TreeChangedCallback(LWEUITreeList &, LWEUITreeEvent &Event, LWEUIManager &, void *);

	void RichTextCallback(LWEUIRichLabel &, LWEUITextStyle &, uint32_t, LWEUIManager&);

	void ListBoxReleased(LWEUI*, uint32_t, void*);

	UITextInputTest(const LWUTF8Iterator &Name, LWEUIManager *UIMan, LWAllocator *Allocator);

	UITextInputTest() = default;

	UIToggle m_PasswordFieldTgl;
	UIBtnLbl m_ClearBtn;
	LWAllocator *m_Allocator = nullptr;
	LWEUIScrollBar * m_VertScrollBar = nullptr;
	LWEUIScrollBar *m_HoriScrollBar = nullptr;
	LWEUITextInput *m_TextDialog = nullptr;
	LWEUIRichLabel *m_RichLabel = nullptr;
	LWEUITreeList *m_Tree = nullptr;
	LWEUIListBox *m_ListBox = nullptr;
	LWEUILabel *m_CharacterCountLbl = nullptr;
	LWEUILabel *m_LineCountLbl = nullptr;
};

#endif