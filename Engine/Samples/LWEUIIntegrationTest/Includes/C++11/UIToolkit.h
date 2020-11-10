#ifndef UITOOLKIT_H
#define UITOOLKIT_H
#include <LWEUIManager.h>
#include <LWEUI/LWEUI.h>
#include <LWCore/LWUnicode.h>

struct UIToggle;

typedef std::function<void(UIToggle &, bool, void*)> UIToggleCallback;

struct UIItem {

	virtual void SetVisible(bool Visible);

	bool isVisible(void) const;

	UIItem(const LWUTF8Iterator &Name, LWEUIManager *UIMan);

	UIItem() = default;

	LWEUI *m_UI = nullptr;
};

struct UIBtnLbl : public UIItem{

	template<class Method, class Obj>
	static void MakeMethod(UIBtnLbl &Btn, const LWUTF8Iterator &Name, LWEUIManager *UIMan, void *UserData, Method M, Obj O) {
		new (&Btn)UIBtnLbl(Name, UIMan, UserData, std::bind(M, O, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		return;
	}

	UIBtnLbl(const LWUTF8Iterator &Name, LWEUIManager *UIMan, void *UserData, LWEUIEventCallback ReleasedCallback);

	UIBtnLbl() = default;

	LWEUIButton *m_Button = nullptr;
	LWEUILabel *m_Label = nullptr;
};

struct UIToggle : public UIItem{

	template<class Method, class Obj>
	static void MakeMethod(UIToggle &Tgl, const LWUTF8Iterator &Name, LWEUIManager *UIMan, void *UserData, bool Toggled, Method M, Obj O) {
		new (&Tgl)UIToggle(Name, UIMan, UserData, Toggled, std::bind(M, O, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		return;
	}

	void ButtonReleased(LWEUI *, uint32_t, void *UserData);

	void SetToggled(bool Toggled);

	void SetLocked(bool Locked);

	bool isToggled(void) const;

	bool isLocked(void) const;

	UIToggle(const LWUTF8Iterator &Name, LWEUIManager *UIMan, void *UserData, bool Toggled, UIToggleCallback ToggleCallback);

	UIToggle() = default;

	UIToggleCallback m_ToggledCallback = nullptr;
	LWEUIButton *m_Button = nullptr;
	LWEUILabel *m_Label = nullptr;
	LWEUIRect *m_ToggledUI = nullptr;
	LWEUIRect *m_LockedUI = nullptr;
};

#endif