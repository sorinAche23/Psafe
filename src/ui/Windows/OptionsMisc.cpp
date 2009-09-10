/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// OptionsMisc.cpp : implementation file
//

#include "stdafx.h"
#include "passwordsafe.h"
#include "PWFileDialog.h"
#include "corelib/PwsPlatform.h"
#include "corelib/PWSprefs.h" // for DoubleClickAction enums
#include "os/dir.h"

#if defined(POCKET_PC)
#include "pocketpc/resource.h"
#else
#include "resource.h"
#include "resource3.h"  // String resources
#endif

#include "corelib/corelib.h"

#include "OptionsMisc.h"
#include "Options_PropertySheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsMisc property page

IMPLEMENT_DYNCREATE(COptionsMisc, CPWPropertyPage)

COptionsMisc::COptionsMisc()
  : CPWPropertyPage(COptionsMisc::IDD), m_pToolTipCtrl(NULL)
{
  //{{AFX_DATA_INIT(COptionsMisc)
  //}}AFX_DATA_INIT
}

COptionsMisc::~COptionsMisc()
{
  delete m_pToolTipCtrl;
}

void COptionsMisc::DoDataExchange(CDataExchange* pDX)
{
  CPWPropertyPage::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(COptionsMisc)
  DDX_Check(pDX, IDC_CONFIRMDELETE, m_confirmdelete);
  DDX_Check(pDX, IDC_MAINTAINDATETIMESTAMPS, m_maintaindatetimestamps);
  DDX_Check(pDX, IDC_ESC_EXITS, m_escexits);
  DDX_Control(pDX, IDC_DOUBLE_CLICK_ACTION, m_dblclk_cbox);
  DDX_Check(pDX, IDC_HOTKEY_ENABLE, m_hotkey_enabled);
  // JHF class CHotKeyCtrl not defined under WinCE
#if !defined(POCKET_PC)
  DDX_Control(pDX, IDC_HOTKEY_CTRL, m_hotkey);
#endif
  DDX_Check(pDX, IDC_USEDEFUSER, m_usedefuser);
  DDX_Check(pDX, IDC_QUERYSETDEF, m_querysetdef);
  DDX_Text(pDX, IDC_DEFUSERNAME, m_defusername);
  DDX_Text(pDX, IDC_OTHERBROWSERLOCATION, m_otherbrowserlocation);
  DDX_Text(pDX, IDC_ALTBROWSER_CMDLINE, m_csBrowserCmdLineParms);
  DDX_Text(pDX, IDC_DEFAULTAUTOTYPE, m_csAutotype);
  DDX_Check(pDX, IDC_MINIMIZEONAUTOTYPE, m_minauto);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptionsMisc, CPWPropertyPage)
  //{{AFX_MSG_MAP(COptionsMisc)
  ON_BN_CLICKED(IDC_HOTKEY_ENABLE, OnEnableHotKey)
  ON_BN_CLICKED(IDC_USEDEFUSER, OnUsedefuser)
  ON_BN_CLICKED(IDC_BROWSEFORLOCATION, OnBrowseForLocation)
  ON_CBN_SELCHANGE(IDC_DOUBLE_CLICK_ACTION, OnComboChanged)
  ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL COptionsMisc::OnInitDialog() 
{
  CPWPropertyPage::OnInitDialog();

  // For some reason, MFC calls us twice when initializing.
  // Populate the combo box only once.
  if (m_dblclk_cbox.GetCount() == 0) {
    // ComboBox now sorted - no need to add in English alphabetical order
    int nIndex;
    CString cs_text;

    cs_text.LoadString(IDSC_DCAAUTOTYPE);
    nIndex = m_dblclk_cbox.AddString(cs_text);
    m_dblclk_cbox.SetItemData(nIndex, PWSprefs::DoubleClickAutoType);

    cs_text.LoadString(IDSC_DCABROWSE);
    nIndex = m_dblclk_cbox.AddString(cs_text);
    m_dblclk_cbox.SetItemData(nIndex, PWSprefs::DoubleClickBrowse);

    cs_text.LoadString(IDSC_DCABROWSEPLUS);
    nIndex = m_dblclk_cbox.AddString(cs_text);
    m_dblclk_cbox.SetItemData(nIndex, PWSprefs::DoubleClickBrowsePlus);

    cs_text.LoadString(IDSC_DCACOPYNOTES);
    nIndex = m_dblclk_cbox.AddString(cs_text);
    m_dblclk_cbox.SetItemData(nIndex, PWSprefs::DoubleClickCopyNotes);

    cs_text.LoadString(IDSC_DCACOPYPASSWORD);
    nIndex = m_dblclk_cbox.AddString(cs_text);
    m_dblclk_cbox.SetItemData(nIndex, PWSprefs::DoubleClickCopyPassword);

    cs_text.LoadString(IDSC_DCACOPYPASSWORDMIN);
    nIndex = m_dblclk_cbox.AddString(cs_text);
    m_dblclk_cbox.SetItemData(nIndex, PWSprefs::DoubleClickCopyPasswordMinimize);

    cs_text.LoadString(IDSC_DCACOPYUSERNAME);
    nIndex = m_dblclk_cbox.AddString(cs_text);
    m_dblclk_cbox.SetItemData(nIndex, PWSprefs::DoubleClickCopyUsername);

    cs_text.LoadString(IDSC_DCAVIEWEDIT);
    nIndex = m_dblclk_cbox.AddString(cs_text);
    m_dblclk_cbox.SetItemData(nIndex, PWSprefs::DoubleClickViewEdit);
 
    cs_text.LoadString(IDSC_DCARUN);
    nIndex = m_dblclk_cbox.AddString(cs_text);
    m_dblclk_cbox.SetItemData(nIndex, PWSprefs::DoubleClickRun);

    cs_text.LoadString(IDSC_DCASENDEMAIL);
    nIndex = m_dblclk_cbox.AddString(cs_text);
    m_dblclk_cbox.SetItemData(nIndex, PWSprefs::DoubleClickSendEmail);

    for (int i = 0; i < m_dblclk_cbox.GetCount(); i++) {
      int ival = (int)m_dblclk_cbox.GetItemData(i);
      m_DCA_to_Index[ival] = i;
    }
  }

  if (m_doubleclickaction < PWSprefs::minDCA ||
      m_doubleclickaction > PWSprefs::maxDCA)
    m_doubleclickaction = PWSprefs::DoubleClickCopyPassword;

  m_dblclk_cbox.SetCurSel(m_DCA_to_Index[m_doubleclickaction]);

  // JHF ditto here
#if !defined(POCKET_PC)
  m_hotkey.SetHotKey(LOWORD(m_hotkey_value),HIWORD(m_hotkey_value));
  if (m_hotkey_enabled == FALSE)
    m_hotkey.EnableWindow(FALSE);
#endif

  GetDlgItem(IDC_OTHERBROWSERLOCATION)->SetWindowText(m_csBrowser);

  OnUsedefuser();

  // Tooltips on Property Pages
  EnableToolTips();

  m_pToolTipCtrl = new CToolTipCtrl;
  if (!m_pToolTipCtrl->Create(this, TTS_ALWAYSTIP | TTS_BALLOON | TTS_NOPREFIX)) {
    TRACE(L"Unable To create Property Page ToolTip\n");
    return TRUE;
  }

  // Activate the tooltip control.
  m_pToolTipCtrl->Activate(TRUE);
  m_pToolTipCtrl->SetMaxTipWidth(300);

  // Set the tooltip
  // Note naming convention: string IDS_xxx corresponds to control IDC_xxx
  CString cs_ToolTip;
  cs_ToolTip.LoadString(IDS_MAINTAINDATETIMESTAMPS);
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_MAINTAINDATETIMESTAMPS), cs_ToolTip);
  cs_ToolTip.LoadString(IDS_OTHERBROWSERLOCATION);
  m_pToolTipCtrl->AddTool(GetDlgItem(IDC_OTHERBROWSERLOCATION), cs_ToolTip);

  return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
// COptionsMisc message handlers

void COptionsMisc::OnEnableHotKey() 
{
  // JHF : no hotkeys on WinCE
#if !defined(POCKET_PC)
  if (((CButton*)GetDlgItem(IDC_HOTKEY_ENABLE))->GetCheck() == 1) {
    GetDlgItem(IDC_HOTKEY_CTRL)->EnableWindow(TRUE);
    GetDlgItem(IDC_HOTKEY_CTRL)->SetFocus();
  } else
    GetDlgItem(IDC_HOTKEY_CTRL)->EnableWindow(FALSE);
#endif
}

void COptionsMisc::OnComboChanged()
{
  int nIndex = m_dblclk_cbox.GetCurSel();
  m_doubleclickaction = m_dblclk_cbox.GetItemData(nIndex);
}

void COptionsMisc::OnUsedefuser()
{
  if (((CButton*)GetDlgItem(IDC_USEDEFUSER))->GetCheck() == 1) {
    GetDlgItem(IDC_DEFUSERNAME)->EnableWindow(TRUE);
    GetDlgItem(IDC_STATIC_USERNAME)->EnableWindow(TRUE);
    GetDlgItem(IDC_QUERYSETDEF)->EnableWindow(FALSE);
  } else {
    GetDlgItem(IDC_DEFUSERNAME)->EnableWindow(FALSE);
    GetDlgItem(IDC_STATIC_USERNAME)->EnableWindow(FALSE);
    GetDlgItem(IDC_QUERYSETDEF)->EnableWindow(TRUE);
  }
}

LRESULT COptionsMisc::OnQuerySiblings(WPARAM wParam, LPARAM lParam)
{
  UpdateData(TRUE);

  // Security asked for DoubleClickAction value
  switch (wParam) {
    case COptions_PropertySheet::PP_GET_DCA:
      {
      int * pDCA = (int *)lParam;
      ASSERT(pDCA != NULL);
      *pDCA = (int)m_doubleclickaction;
      return 1L;
      }
    default:
      break;
  }
  return 0L;
}

BOOL COptionsMisc::OnApply() 
{
  UpdateData(TRUE);

  // JHF ditto
#if !defined(POCKET_PC)
  WORD wVirtualKeyCode, wModifiers;
  m_hotkey.GetHotKey(wVirtualKeyCode, wModifiers);
  DWORD v = wVirtualKeyCode | (wModifiers << 16);
  m_hotkey_value = v;
#endif
  m_csBrowser = m_otherbrowserlocation;

  // Go ask Security for ClearClipboardOnMinimize value
  BOOL bClearClipboardOnMinimize;
  if (QuerySiblings(COptions_PropertySheet::PP_GET_CCOM,
                    (LPARAM)&bClearClipboardOnMinimize) == 0L) {
    // Security not loaded - get from Prefs
    bClearClipboardOnMinimize = 
        PWSprefs::GetInstance()->GetPref(PWSprefs::ClearClipboardOnMinimize);
  }

  if (m_doubleclickaction == PWSprefs::DoubleClickCopyPasswordMinimize &&
      bClearClipboardOnMinimize) {
    AfxMessageBox(IDS_MINIMIZECONFLICT);

    // Are we the current page, if not activate this page
    COptions_PropertySheet *pPS = (COptions_PropertySheet *)GetParent();
    if (pPS->GetActivePage() != (CPWPropertyPage *)this)
      pPS->SetActivePage(this);

    m_dblclk_cbox.SetFocus();
    return FALSE;
  }

  return CPWPropertyPage::OnApply();
}

void COptionsMisc::OnBrowseForLocation()
{
  CString cs_initiallocation, cs_title;
  INT_PTR rc;

  if (m_csBrowser.IsEmpty())
    cs_initiallocation = L"C:\\";
  else {
    std::wstring path = m_csBrowser;
    std::wstring drive, dir, name, ext;
    pws_os::splitpath(path, drive, dir, name, ext);
    path = pws_os::makepath(drive, dir, L"", L"");
    cs_initiallocation = path.c_str();
  }

  CPWFileDialog fd(TRUE, NULL, NULL,
                   OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_DONTADDTORECENT | 
                      OFN_HIDEREADONLY | OFN_PATHMUSTEXIST,
                   CString(MAKEINTRESOURCE(IDS_FDF_PR_ALL)),
                   this);

  cs_title.LoadString(IDS_SELECTBROWSER);
  fd.m_ofn.lpstrTitle = cs_title;
  fd.m_ofn.lpstrInitialDir = cs_initiallocation;

  rc = fd.DoModal();
  if (rc == IDOK) {
    m_csBrowser = fd.GetPathName();
    GetDlgItem(IDC_OTHERBROWSERLOCATION)->SetWindowText(m_csBrowser);
  }
}

// Override PreTranslateMessage() so RelayEvent() can be
// called to pass a mouse message to CPWSOptions's
// tooltip control for processing.
BOOL COptionsMisc::PreTranslateMessage(MSG* pMsg)
{
  if (m_pToolTipCtrl != NULL)
    m_pToolTipCtrl->RelayEvent(pMsg);

  return CPWPropertyPage::PreTranslateMessage(pMsg);
}
