#pragma once
// Various WTL extensions that are not fb2k specific and can be reused in other WTL based software

#include <Uxtheme.h>
#include <functional>

#define ATLASSERT_SUCCESS(X) {auto RetVal = (X); ATLASSERT( RetVal ); (void) RetVal; }

#ifdef SubclassWindow // mitigate windowsx.h clash
#undef SubclassWindow
#endif

class NoRedrawScope {
public:
	NoRedrawScope(HWND p_wnd) throw() : m_wnd(p_wnd) {
		m_wnd.SetRedraw(FALSE);
	}
	~NoRedrawScope() throw() {
		m_wnd.SetRedraw(TRUE);
	}
private:
	CWindow m_wnd;
};

class NoRedrawScopeEx {
public:
	NoRedrawScopeEx(HWND p_wnd) throw() : m_wnd(p_wnd) {
		if (m_wnd.IsWindowVisible()) {
			m_active = true;
			m_wnd.SetRedraw(FALSE);
		}
	}
	~NoRedrawScopeEx() throw() {
		if (m_active) {
			m_wnd.SetRedraw(TRUE);
			m_wnd.RedrawWindow(NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_ALLCHILDREN);
		}
	}
	NoRedrawScopeEx(const NoRedrawScopeEx&) = delete;
	void operator=(const NoRedrawScopeEx&) = delete;
private:
	bool m_active = false;
	CWindow m_wnd;
};

class NoRedrawControl {
public:
	CWindow m_wnd;

	void operator++() {
		m_count++;
		if (m_wnd.IsWindowVisible()) {
			m_active = true;
			m_wnd.SetRedraw(FALSE);
		}
	}
	void operator--() {
		if (--m_count == 0 && m_active) {
			m_wnd.SetRedraw(TRUE);
			m_wnd.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
			m_active = false;
		}
	}
	int m_count = 0;
	bool m_active = false;

	NoRedrawControl(HWND wnd = NULL) : m_wnd(wnd) {}
	void operator=(const NoRedrawControl&) = delete;
	NoRedrawControl(const NoRedrawControl&) = delete;
};

LRESULT RelayEraseBkgnd(HWND p_from, HWND p_to, HDC p_dc);
void InjectParentEraseHandler(HWND);
void InjectEraseHandler(HWND, HWND sendTo);
void InjectParentCtlColorHandler(HWND);
void BounceNextDlgCtl(HWND wnd, HWND wndTo);



#define MSG_WM_ERASEBKGND_PARENT() \
	if (uMsg == WM_ERASEBKGND) { \
		lResult = ::RelayEraseBkgnd(hWnd, ::GetParent(hWnd), (HDC)wParam); \
		return TRUE; \
	}

#define MSG_WM_ERASEBKGND_TO(wndTarget) \
	if (uMsg == WM_ERASEBKGND) { \
		lResult = ::RelayEraseBkgnd(hWnd, wndTarget, (HDC)wParam); \
		return TRUE; \
	}

#define MSG_WM_TIMER_EX(timerId, func) \
	if (uMsg == WM_TIMER && (UINT_PTR)wParam == timerId) \
	{ \
		SetMsgHandled(TRUE); \
		func(); \
		lResult = 0; \
		if(IsMsgHandled()) \
			return TRUE; \
	}

#define MESSAGE_HANDLER_SIMPLE(msg, func) \
	if(uMsg == msg) \
	{ \
		SetMsgHandled(TRUE); \
		func(); \
		lResult = 0; \
		if(IsMsgHandled()) \
			return TRUE; \
	}

// void OnSysCommandHelp()
#define MSG_WM_SYSCOMMAND_HELP(func) \
	if (uMsg == WM_SYSCOMMAND && wParam == SC_CONTEXTHELP) \
	{ \
		SetMsgHandled(TRUE); \
		func(); \
		lResult = 0; \
		if(IsMsgHandled()) \
			return TRUE; \
	}

//BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID)
#define END_MSG_MAP_HOOK() \
			break; \
		default: \
			return __super::ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult, dwMsgMapID); \
		} \
		return FALSE; \
	}


// Obsolete, use CImageListManaged instead
class CImageListContainer : public CImageList {
public:
	CImageListContainer() {}
	~CImageListContainer() {Destroy();}

	void operator=(const CImageListContainer&) = delete;
	CImageListContainer(const CImageListContainer&) = delete;
};


template<bool managed> class CThemeT {
public:
	CThemeT(HTHEME source = NULL) : m_theme(source) {}

	~CThemeT() {
		Release();
	}

	HTHEME OpenThemeData(HWND wnd,LPCWSTR classList) {
		Release();
		return m_theme = ::OpenThemeData(wnd, classList);
	}

	void Release() {
		HTHEME releaseme = pfc::replace_null_t(m_theme);
		if (managed && releaseme != NULL) CloseThemeData(releaseme);
	}

	operator HTHEME() const {return m_theme;}
	HTHEME m_theme;
};
typedef CThemeT<false> CThemeHandle;
typedef CThemeT<true> CTheme;


class CCheckBox : public CButton {
public:
	void ToggleCheck(bool state) {SetCheck(state ? BST_CHECKED : BST_UNCHECKED);}
	bool IsChecked() const {return GetCheck() == BST_CHECKED;}

	CCheckBox(HWND hWnd = NULL) : CButton(hWnd) { }
	CCheckBox & operator=(HWND wnd) {m_hWnd = wnd; return *this; }
};

class CEditPPHooks : public CWindowImpl<CEditPPHooks, CEdit> {
public:
	bool HandleCtrlA = true, NoEscSteal = false, NoEnterSteal = false, WantAllKeys = false;

	std::function<void ()> onEnterKey;
	std::function<void ()> onEscKey;
	
	CEditPPHooks(CMessageMap * hookMM = nullptr, int hookMMID = 0) : m_hookMM(hookMM), m_hookMMID(hookMMID) {}

	BEGIN_MSG_MAP_EX(CEditPPHooks)
		MSG_WM_KEYDOWN(OnKeyDown)
		MSG_WM_CHAR(OnChar)
		MSG_WM_GETDLGCODE(OnEditGetDlgCode)

		if ( m_hookMM != nullptr ) {

			CHAIN_MSG_MAP_ALT_MEMBER( ( * m_hookMM ), m_hookMMID );

		}

	END_MSG_MAP()

	static void DeleteLastWord( CEdit wnd, bool bForward = false );
private:
	static bool isSpecial( wchar_t c ) {
		return (unsigned) c < ' ';
	}
	static bool isWordDelimiter( wchar_t c ) {
		return c == ' ' || c == ',' || c == '.' || c == ';' || c == ':';
	}
	void OnChar(UINT nChar, UINT, UINT nFlags) {
		if (m_suppressChar != 0) {
			if (nChar == m_suppressChar) return;
		}
		if (m_suppressScanCode != 0) {
			UINT code = nFlags & 0xFF;
			if (code == m_suppressScanCode) return;
		}
		SetMsgHandled(FALSE);
	}
	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
		(void)nRepCnt;
		m_suppressChar = 0;
		m_suppressScanCode = 0;
		if (HandleCtrlA) {
			if (nChar == 'A') {
				if (GetHotkeyModifierFlags() == MOD_CONTROL) {
					m_suppressScanCode = nFlags & 0xFF;
					this->SetSelAll(); return;
				}
			}
			if ( nChar == VK_BACK ) {
				if (GetHotkeyModifierFlags() == MOD_CONTROL) {
					m_suppressScanCode = nFlags & 0xFF;
					DeleteLastWord( *this ) ; return;
				}
			}
			if ( nChar == VK_DELETE ) {
				if (GetHotkeyModifierFlags() == MOD_CONTROL) {
					m_suppressScanCode = nFlags & 0xFF;
					DeleteLastWord( *this, true ) ; return;
				}
			}
			if ( nChar == VK_RETURN && onEnterKey ) {
				m_suppressChar = nChar;
				onEnterKey(); return;
			}
			if ( nChar == VK_ESCAPE && onEscKey ) {
				m_suppressChar = nChar;
				onEscKey(); return;
			}
		}
		SetMsgHandled(FALSE);
	}
	UINT OnEditGetDlgCode(LPMSG lpMsg) {
		if (WantAllKeys) return DLGC_WANTALLKEYS;
		if (lpMsg == NULL) {
			SetMsgHandled(FALSE); return 0;
		} else {
			switch(lpMsg->message) {
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
				switch(lpMsg->wParam) {
				case VK_ESCAPE:
					if (onEscKey) {
						return DLGC_WANTMESSAGE;
					}
					SetMsgHandled(!!NoEscSteal);
					return 0;
				case VK_RETURN:
					if (onEnterKey) {
						return DLGC_WANTMESSAGE;
					}
					SetMsgHandled(!!NoEnterSteal);
					return 0;
				default:
					SetMsgHandled(FALSE); return 0;
				}
			default:
				SetMsgHandled(FALSE); return 0;

			}
		}
	}
	UINT m_suppressChar = 0, m_suppressScanCode = 0;
	CMessageMap * const m_hookMM;
	const int m_hookMMID;
};


class CEditNoEscSteal : public CEdit {
public:
	void SubclassWindow(HWND wnd) {
		this->Attach(wnd);
		SubclassThisWindow(wnd);
	}
	static void SubclassThisWindow(HWND wnd) {
		SetWindowSubclass(wnd, proc, 0, 0);
	}
private:
	static LRESULT CALLBACK proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR) {
		if ( uMsg == WM_GETDLGCODE ) {
			auto lpMsg = reinterpret_cast<LPMSG>(lParam);
			if (lpMsg != NULL) {
				switch(lpMsg->message) {
				case WM_KEYDOWN:
				case WM_SYSKEYDOWN:
					switch(lpMsg->wParam) {
					case VK_ESCAPE:
						return 0;
					}
				}
			}
		}
		return DefSubclassProc(hWnd, uMsg, wParam, lParam);
	}
};

class CEditNoEnterEscSteal : public CEdit {
public:
	void SubclassWindow(HWND wnd) {
		this->Attach(wnd);
		SubclassThisWindow(wnd);
	}
	static void SubclassThisWindow(HWND wnd) {
		SetWindowSubclass(wnd, proc, 0, 0);
	}
private:
	static LRESULT CALLBACK proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR) {
		if ( uMsg == WM_GETDLGCODE ) {
			auto lpMsg = reinterpret_cast<LPMSG>(lParam);
			if (lpMsg != NULL) {
				switch(lpMsg->message) {
				case WM_KEYDOWN:
				case WM_SYSKEYDOWN:
					switch(lpMsg->wParam) {
					case VK_RETURN:
					case VK_ESCAPE:
						return 0;
					}
				}
			}
		}
		return DefSubclassProc(hWnd, uMsg, wParam, lParam);
	}
};



class CWindowClassUnregisterScope {
public:
	CWindowClassUnregisterScope() : name() {}
	const TCHAR * name;
	void Set(const TCHAR * n) {ATLASSERT( name == NULL ); name = n; }
	bool IsActive() const {return name != NULL;}
	void CleanUp() {
		const TCHAR * n = name; name = NULL;
		if (n != NULL) ATLASSERT_SUCCESS( UnregisterClass(n, (HINSTANCE)&__ImageBase) );
	}
	~CWindowClassUnregisterScope() {CleanUp();}
};


// CWindowRegisteredT
// Minimalistic wrapper for registering own window classes that can be created by class name, included in dialogs and such.
// Usage:
// class myClass : public CWindowRegisteredT<myClass> {...};
// Call myClass::Register() before first use
template<typename TClass, typename TBaseClass = CWindow>
class CWindowRegisteredT : public TBaseClass, public CMessageMap {
public:
	static UINT GetClassStyle() {
		return CS_VREDRAW | CS_HREDRAW;
	}
	static HCURSOR GetCursor() {
		return ::LoadCursor(NULL, IDC_ARROW);
	}

	BEGIN_MSG_MAP_EX(CWindowRegisteredT)
	END_MSG_MAP()


	static void Register() {
		static CWindowClassUnregisterScope scope;
		if (!scope.IsActive()) {
			WNDCLASS wc = {};
			wc.style = TClass::GetClassStyle();
			wc.cbWndExtra = sizeof(void*);
			wc.lpszClassName = TClass::GetClassName();
			wc.lpfnWndProc = myWindowProc;
			wc.hInstance = (HINSTANCE)&__ImageBase;
			wc.hCursor = TClass::GetCursor();
			ATLASSERT_SUCCESS( RegisterClass(&wc) != 0 );
			scope.Set(wc.lpszClassName);
		}
	}
protected:
	virtual ~CWindowRegisteredT() {}
private:
	static LRESULT CALLBACK myWindowProc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp) {
		TClass * i = NULL;
		if (msg == WM_NCCREATE) {
			i = new TClass;
			i->Attach(wnd);
			::SetWindowLongPtr(wnd, 0, reinterpret_cast<LONG_PTR>(i));
		} else {
			i = reinterpret_cast<TClass*>( ::GetWindowLongPtr(wnd, 0) );
		}
		LRESULT r;
		if (i == NULL || !i->ProcessWindowMessage(wnd, msg, wp, lp, r)) r = ::DefWindowProc(wnd, msg, wp, lp);
		if (msg == WM_NCDESTROY) {
			::SetWindowLongPtr(wnd, 0, 0);
			delete i;
		}
		return r;
	}
};




class CSRWlock {
public:
	CSRWlock() { }
	
	static bool HaveAPI() { return true; }

	void EnterShared() {
		AcquireSRWLockShared( & theLock );
	}
	void EnterExclusive() {
		AcquireSRWLockExclusive( & theLock );
	}
	void LeaveShared() {
		ReleaseSRWLockShared( & theLock );
	}
	void LeaveExclusive() {
		ReleaseSRWLockExclusive( &theLock );
	}

private:
	CSRWlock(const CSRWlock&) = delete;
	void operator=(const CSRWlock&) = delete;

	SRWLOCK theLock = {};
};
