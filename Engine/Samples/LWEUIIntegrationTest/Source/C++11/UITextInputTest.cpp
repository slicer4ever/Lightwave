#include "UITextInputTest.h"
#include <LWEUI/LWEUITextInput.h>
#include <LWEUI/LWEUILabel.h>
#include <LWEUI/LWEUIScrollBar.h>
#include <LWEUI/LWEUIRichLabel.h>
#include <LWEUI/LWEUIListBox.h>

void UITextInputTest::PasswordToggled(UIToggle &, bool Toggled, void*) {
	m_TextDialog->SetFlag((m_TextDialog->GetFlag() & ~LWEUITextInput::PasswordField) | (Toggled ? LWEUITextInput::PasswordField : 0));
	return;
}

void UITextInputTest::ScrollBarChanged(LWEUI *SB, uint32_t, void *UserData) {
	LWEUIManager *UIMan = (LWEUIManager*)UserData;
	float hScroll = m_TextDialog->GetHorizontalScroll();
	float vScroll = m_TextDialog->GetVerticalScroll();
	if (SB == m_HoriScrollBar) hScroll = m_HoriScrollBar->GetScroll();
	else vScroll = m_VertScrollBar->GetScroll();
	m_TextDialog->SetScroll(hScroll, vScroll, UIMan->GetScale());
	return;
}

void UITextInputTest::TextDialogChanged(LWEUI *, uint32_t, void *UserData) {
	char8_t Buffer[1024];
	LWEUIManager *UIMan = (LWEUIManager*)UserData;
	m_CharacterCountLbl->SetText(LWUTF8I::Fmt<128>("{}/{}", m_TextDialog->GetCurrentLength(), m_TextDialog->GetMaxLength()));
	m_LineCountLbl->SetText(LWUTF8I::Fmt<128>("{}/{}", m_TextDialog->GetLineCount(), m_TextDialog->GetMaxLines()));
	float hMaxScroll = m_TextDialog->GetHorizontalMaxScroll();
	float vMaxScroll = m_TextDialog->GetVerticalMaxScroll(UIMan->GetScale());
	float hSize = m_TextDialog->GetVisibleSize().x;
	float vSize = m_TextDialog->GetVisibleSize().y;
	m_HoriScrollBar->SetVisible(hMaxScroll > hSize);
	m_HoriScrollBar->SetMaxScroll(hMaxScroll).SetScrollSize(hSize).SetScroll(m_TextDialog->GetHorizontalScroll());
	m_VertScrollBar->SetVisible(vMaxScroll > vSize);
	m_VertScrollBar->SetMaxScroll(vMaxScroll).SetScrollSize(vSize).SetScroll(m_TextDialog->GetVerticalScroll());
	m_TextDialog->GetTextRange(m_TextDialog->MakeCursorAt(0, 0), sizeof(Buffer), Buffer, sizeof(Buffer), false);
	m_RichLabel->SetText(Buffer);
	return;
}

void UITextInputTest::ClearBtn(LWEUI*, uint32_t, void *UserData) {
	m_TextDialog->Clear();
	TextDialogChanged(nullptr, 0, UserData);
}


void UITextInputTest::TreeChangedCallback(LWEUITreeList &Tree, LWEUITreeEvent &Event, LWEUIManager &, void *) {
	if (Event.m_EventType == LWEUITreeEvent::Adding) {
		uint32_t ID = Tree.InsertItemAt("LALA", nullptr, Event, *m_Allocator);
		Tree.SetItemValue(ID, LWUTF8I::Fmt<32>("Tree.{}", ID));
	} else if (Event.m_EventType == LWEUITreeEvent::Moving) {
		Tree.MoveItemTo(Event);
		Tree.Prune(*m_Allocator);
	} else if (Event.m_EventType == LWEUITreeEvent::Removing) {
		Tree.RemoveItemAt(Event);
		Tree.Prune(*m_Allocator);
	}
	return;
}

void UITextInputTest::ListBoxReleased(LWEUI*, uint32_t, void*) {
	uint32_t OverID = m_ListBox->GetItemOver();
	if (OverID == -1) return;
	LWEUIListBoxItem *Item = m_ListBox->GetItem(OverID);
	m_Tree->InsertChildLast(Item->GetName(), nullptr, -1, *m_Allocator);
	return;
}

void UITextInputTest::RichTextCallback(LWEUIRichLabel &, LWEUITextStyle &Style, uint32_t EventID, LWEUIManager&) {
	if (EventID == LWEUI::Event_Released) {
		m_ListBox->PushItem(Style.m_Iterator, nullptr);
	}
	return;
}

UITextInputTest::UITextInputTest(const LWUTF8Iterator &Name, LWEUIManager *UIMan, LWAllocator *Allocator) : UIItem(Name, UIMan), m_Allocator(Allocator) {	
	UIToggle::MakeMethod(m_PasswordFieldTgl, LWUTF8I::Fmt<128>("{}.PasswordTgl", Name), UIMan, nullptr, false, &UITextInputTest::PasswordToggled, this);
	UIBtnLbl::MakeMethod(m_ClearBtn, LWUTF8I::Fmt<128>("{}.ClearBtn", Name), UIMan, UIMan, &UITextInputTest::ClearBtn, this);
	m_TextDialog = UIMan->GetNamedUI<LWEUITextInput>(LWUTF8I::Fmt<128>("{}.TextDialog", Name));
	m_CharacterCountLbl = UIMan->GetNamedUI<LWEUILabel>(LWUTF8I::Fmt<128>("{}.CharacterCountLbl", Name));
	m_LineCountLbl = UIMan->GetNamedUI<LWEUILabel>(LWUTF8I::Fmt<128>("{}.LineCountLbl", Name));
	m_RichLabel = UIMan->GetNamedUI<LWEUIRichLabel>(LWUTF8I::Fmt<128>("{}.RichLbl", Name));
	m_HoriScrollBar = UIMan->GetNamedUI<LWEUIScrollBar>(LWUTF8I::Fmt<128>("{}.HoriScrollBar", Name));
	m_VertScrollBar = UIMan->GetNamedUI<LWEUIScrollBar>(LWUTF8I::Fmt<128>("{}.VertScrollBar", Name));
	m_Tree = UIMan->GetNamedUI<LWEUITreeList>(LWUTF8I::Fmt<128>("{}.Tree", Name));
	m_ListBox = UIMan->GetNamedUI<LWEUIListBox>(LWUTF8I::Fmt<128>("{}.ListBox", Name));
	for (uint32_t i = 0; i < 10; i++) m_RichLabel->RegisterMethodCallback(i, &UITextInputTest::RichTextCallback, this);
	UIMan->RegisterMethodEvent(m_HoriScrollBar, LWEUI::Event_Changed, &UITextInputTest::ScrollBarChanged, this, UIMan);
	UIMan->RegisterMethodEvent(m_VertScrollBar, LWEUI::Event_Changed, &UITextInputTest::ScrollBarChanged, this, UIMan);
	UIMan->RegisterMethodEvent(m_TextDialog, LWEUI::Event_Changed, &UITextInputTest::TextDialogChanged, this, UIMan);
	UIMan->RegisterMethodEvent(m_ListBox, LWEUI::Event_Released, &UITextInputTest::ListBoxReleased, this, UIMan);
	m_Tree->SetChangeCallbackMethod(&UITextInputTest::TreeChangedCallback, this, nullptr);
	TextDialogChanged(m_TextDialog, 0, UIMan);
	m_HoriScrollBar->SetVisible(false);
	m_VertScrollBar->SetVisible(false);
}
