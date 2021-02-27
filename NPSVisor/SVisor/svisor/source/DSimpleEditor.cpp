//--------- DSimpleEditor.cpp ------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "DSimpleEditor.h"
#include "1.h"
#include "p_base.h"
#include "sizer.h"
//----------------------------------------------------------------------------
#define BORDER 5
#define W_BTN 120
#define H_BTN 30
//----------------------------------------------------------------------------
class CR_Edit2 : public svEdit
{
  public:
    CR_Edit2(PWin * parent, uint id, const PRect& r,
              LPCTSTR text = 0, int textlen = 0, HINSTANCE hinst = 0) :
         svEdit(parent, id, r, text, textlen, hinst) {}
    ~CR_Edit2() { destroy(); }
  protected:
    int verifyKey()  { return false; }
    virtual bool evChar(UINT& key) { return false; }
    bool evKeyDown(UINT& key) { return false; }

};
//----------------------------------------------------------------------------
DSimpleEditor::DSimpleEditor(PWin* parent, LPCTSTR filename, uint resId, HINSTANCE hinst)
:
    baseClass(parent, resId, hinst), Filename(filename)
{
  PRect r(R__X(BORDER), R__Y(H_BTN + BORDER + 2), 160, 120);
  Text = new CR_Edit2(this, IDC_BUTTON_F1 * 2, r, 0,(DWORD)-1);
  Text->Attr.style |= ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL | WS_VSCROLL;

  r = PRect(0, 0, R__X(W_BTN), R__Y(H_BTN));
  r.MoveTo(R__X(BORDER), R__Y(BORDER));
  OkBtn = new POwnBtnImageStd(this, IDC_BUTTON_F1, r, new PBitmap(IDB_OK, getHInstance()), POwnBtnImageStd::wLeft, true );
  r.Offset(R__X(2) + r.Width(), 0);
  CancBtn = new POwnBtnImageStd(this, IDC_BUTTON_F2, r, new PBitmap(IDB_CANC, getHInstance()), POwnBtnImageStd::wLeft, true );
}
//----------------------------------------------------------------------------
DSimpleEditor::~DSimpleEditor()
{
  destroy();
}
//----------------------------------------------------------------------------
#define DIVS 6
//----------------------------------------------------------------------------
#define LT(a) ((a) / DIVS)
#define RB(a) ((a) - (a) / DIVS)
//----------------------------------------------------------------------------
bool DSimpleEditor::create()
{
  if(!baseClass::create())
    return false;
  int w = sizer::getWidth();
  int h = sizer::getHeight();
  PRect r(LT(w), LT(h), RB(w), RB(h));
  setWindowPos(0, r, SWP_NOZORDER);

  PRect rT;
  GetWindowRect(*Text, rT);
  MapWindowPoints(HWND_DESKTOP, *this, (LPPOINT)(LPRECT)rT, 2);
  GetClientRect(*this, r);
  rT.right = r.right - R__X(BORDER);// - GetSystemMetrics(SM_CXBORDER) * 3;
  rT.bottom = r.bottom - R__Y(BORDER);// - GetSystemMetrics(SM_CYBORDER) * 3;
  Text->setWindowPos(0, rT, SWP_NOZORDER | SWP_NOMOVE);

#if 1
  uint dim;
  LPTSTR buff = openFile(Filename, dim);
  if(buff) {
    SetWindowText(*Text, buff);
    delete []buff;
    }
#else
  P_File pf(Filename, P_READ_ONLY);
  if(pf.P_open()) {
    lUDimF len = (lUDimF)pf.get_len();
    LPTSTR buff = new TCHAR[len + 1];
    pf.P_read(buff, len);
    buff[len] = 0;
    SetWindowText(*Text, buff);
    delete []buff;
    }
#endif
  smartPointerConstString sps(getStringByLangGlob(ID_DIALOG_OK));
  OkBtn->setCaption(sps);
  sps = getStringByLangGlob(ID_DIALOG_CANC);
  CancBtn->setCaption(sps);

  HFONT font = DEF_FONT(R__Y(16));
  Text->setFont(font);
  OkBtn->setFont(font);
  CancBtn->setFont(font, true);

  TCHAR file[_MAX_PATH];
  _tsplitpath_s(Filename, 0, 0, 0, 0, file, SIZE_A(file), 0, 0);
  SetWindowText(*this, file);
  return true;
}
//----------------------------------------------------------------------------
bool DSimpleEditor::BNClicked_F1()
{
  int len = GetWindowTextLength(*Text);
  if(len <= 0)
    return false;
  P_File pf(Filename, P_CREAT);
  if(!pf.P_open())
    return false;
  LPTSTR buff = new TCHAR[len + 2];
  GetWindowText(*Text, buff, len + 1);
  pf.P_writeString(buff);

  return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
LRESULT DSimpleEditor::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDOK:
        case IDC_BUTTON_F1:
          if(BNClicked_F1())
            EndDialog(hwnd, IDOK);
          break;
        case IDCANCEL:
        case IDC_BUTTON_F2:
          EndDialog(hwnd, IDCANCEL);
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}

