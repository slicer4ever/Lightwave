#include "UIToolkit.h"
#include <LWEUI/LWEUIButton.h>
#include <LWEUI/LWEUIRect.h>
#include <LWEUI/LWEUILabel.h>

//UIITem
void UIItem::SetVisible(bool Visible) {
	m_UI->SetVisible(Visible);
}

bool UIItem::isVisible(void) const {
	return m_UI->isVisible();
}

UIItem::UIItem(const LWUTF8Iterator &Name, LWEUIManager *UIMan) : m_UI(UIMan->GetNamedUI(Name)) {}

//UIButton
UIBtnLbl::UIBtnLbl(const LWUTF8Iterator &Name, LWEUIManager *UIMan, void *UserData, LWEUIEventCallback ReleasedCallback) : UIItem(Name, UIMan) {
	m_Button = UIMan->GetNamedUI<LWEUIButton>(LWUTF8I::Fmt<128>("{}.Button", Name));
	m_Label = UIMan->GetNamedUI<LWEUILabel>(LWUTF8I::Fmt<128>("{}.Label", Name));

	if (ReleasedCallback) UIMan->RegisterEvent(m_Button, LWEUI::Event_Released, ReleasedCallback, UserData);
}

//UIToggle

void UIToggle::ButtonReleased(LWEUI *, uint32_t, void *UserData) {
	if (isLocked()) return;
	bool State = !isToggled();
	SetToggled(State);
	if (m_ToggledCallback) m_ToggledCallback(*this, State, UserData);
	return;
}


void UIToggle::SetToggled(bool Toggled) {
	if (isLocked()) return;
	m_ToggledUI->SetVisible(Toggled);
	return;
}

void UIToggle::SetLocked(bool Locked) {
	m_LockedUI->SetVisible(Locked);
	return;
}

bool UIToggle::isToggled(void) const {
	return m_ToggledUI->isVisible();
}

bool UIToggle::isLocked(void) const {
	return m_LockedUI->isVisible();
}

UIToggle::UIToggle(const LWUTF8Iterator &Name, LWEUIManager *UIMan, void *UserData, bool Toggled, UIToggleCallback ToggleCallback) : UIItem(Name, UIMan), m_ToggledCallback(ToggleCallback) {
	m_Button = UIMan->GetNamedUI<LWEUIButton>(LWUTF8I::Fmt<128>("{}.Button", Name));
	m_Label = UIMan->GetNamedUI<LWEUILabel>(LWUTF8I::Fmt<128>("{}.Label", Name));
	m_ToggledUI = UIMan->GetNamedUI<LWEUIRect>(LWUTF8I::Fmt<128>("{}.ToggleUI", Name));
	m_LockedUI = UIMan->GetNamedUI<LWEUIRect>(LWUTF8I::Fmt<128>("{}.LockedUI", Name));
	UIMan->RegisterMethodEvent(m_Button, LWEUI::Event_Released, &UIToggle::ButtonReleased, this, UserData);
	SetToggled(Toggled);
}

