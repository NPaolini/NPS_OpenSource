//-------- pMenuBtn.cpp ---------------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "pMenuBtn.h"
#include "POwnBtnImageStd.h"
#include "pCheckBox.h"
#include "resource.h"
#include "macro_Utils.h"
//-------------------------------------------------------------------
//-------------------------------------------------------------------
#define USE_NEW_FLAT
//-------------------------------------------------------------------
class hilightBtn : public POwnBtnImageStd
{
  private:
    typedef POwnBtnImageStd baseClass;
  public:
    hilightBtn(PWin * parent, uint id, int x, int y, int w, int h,
          PBitmap* image, PBitmap* image2, LPCTSTR text = 0, HINSTANCE hinst = 0) :
    baseClass(parent, id, x, y, w, h, image, wTop, true, text, hinst), old(0),
      lightImg(image2) { init(); }

    hilightBtn(PWin * parent, uint id, const PRect& r, PBitmap* image, PBitmap* image2,
          LPCTSTR text = 0, HINSTANCE hinst = 0) :
    baseClass(parent, id, r, image, wTop, true, text, hinst), old(0),
      lightImg(image2) { init(); }

    hilightBtn(PWin * parent, uint resid, PBitmap* image, PBitmap* image2, HINSTANCE hinst = 0) :
    baseClass(parent, resid, image, wTop, true, hinst), old(0),
      lightImg(image2) { init(); }

    ~hilightBtn();
    bool create();
  protected:
    PBitmap* lightImg;
    PBitmap* old;
    void init();
    void mouseEnter(const POINT& pt, uint flag);
    void mouseLeave(const POINT& pt, uint flag);
    void mouseMove(const POINT& pt, uint flag);

    void restore();
};
//----------------------------------------------------------------------------
hilightBtn::~hilightBtn()
{
  if(old)
    replaceBmp(old);
  delete lightImg;
  destroy();
}
//----------------------------------------------------------------------------
void hilightBtn::init()
{
  Attr.style |= WS_TABSTOP;
  enableCapture();
}
//----------------------------------------------------------------------------
bool hilightBtn::create()
{
#ifdef USE_NEW_FLAT
  setFlat(true);
#endif
  return baseClass::create();
}
//----------------------------------------------------------------------------
void hilightBtn::mouseEnter(const POINT& pt, uint flag)
{
  old = replaceBmp(lightImg);
  InvalidateRect(*this, 0, 0);
  baseClass::mouseEnter(pt, flag);
  SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURS_HAND)));
}
//----------------------------------------------------------------------------
void hilightBtn::mouseLeave(const POINT& pt, uint flag)
{
  restore();
  baseClass::mouseLeave(pt, flag);
}
//----------------------------------------------------------------------------
void hilightBtn::restore()
{
  replaceBmp(old);
  old = 0;
  InvalidateRect(*this, 0, 0);
}
//----------------------------------------------------------------------------
void hilightBtn::mouseMove(const POINT& pt, uint flag)
{
  baseClass::mouseMove(pt, flag);
  SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURS_HAND)));
}
//-------------------------------------------------------------------
#define DB_BTN(id, a, t) { id, a, a##2, t }
#define NEW_BTN new PBitmap(iBtn[i].idBtn, getHInstance())
#define NEW_BTN2 new PBitmap(iBtn[i].idBtn2, getHInstance())
#define TXT_BTN iBtn[i].Text

#define NEW_BTN_ new PBitmap(iBtn2[i].idBtn, getHInstance())
#define NEW_BTN2_ new PBitmap(iBtn2[i].idBtn2, getHInstance())
#define TXT_BTN_ iBtn2[i].Text
//----------------------------------------------------------------------------
#define COLOR_BAR RGB(0xe0, 0xe0, 0xc0)
#define COLOR_SEL RGB(0xd0, 0xa0, 0x70)
//-----------------------------------------------------------
class myComboBoxEdit : public PEdit
{
  private:
    typedef PEdit baseClass;
  public:
    myComboBoxEdit(uint idbtn, PWin* owner, HWND hwnd, HINSTANCE hInst) : idBtn(idbtn), Owner(owner), baseClass(hwnd, hInst) {}
    ~myComboBoxEdit() { destroy(); }
  protected:
    uint idBtn;
    PWin* Owner;
    virtual bool evChar(WPARAM& key)
    {
      if(VK_RETURN == key)
        PostMessage(*Owner, WM_COMMAND, MAKEWPARAM(idBtn, 0), 0);
      return baseClass::evChar(key);
    }
};
//-----------------------------------------------------------
class myComboBox : public PComboBox
{
  private:
    typedef PComboBox baseClass;
  public:
    myComboBox(uint idbtn, PWin* parent, uint id, const PRect& r, LPCTSTR text = 0, HINSTANCE hinst = 0) :
      idBtn(idbtn), baseClass(parent, id, r, text, hinst) {}
    ~myComboBox() { destroy(); }
    bool create() {
      if(!baseClass::create())
        return false;
      COMBOBOXINFO cbxi;
      cbxi.cbSize = sizeof(cbxi);
      if(GetComboBoxInfo(*this, &cbxi)) {
        HWND hwnd = cbxi.hwndItem;
        if(hwnd)
          new myComboBoxEdit(idBtn, getParent(), hwnd, getHInstance());
        }
      return true;
      }
  protected:
    uint idBtn;
};
//-------------------------------------------------------------------
static PComboBox* makeCbx(PWin* p, uint idc, PRect& r, bool my = false)
{
  int tb[] = { 255 };
  PComboBox* cbx = my ? new myComboBox(IDC_BTN_TIPS, p, idc, r) : new PComboBox(p, idc, r);
/*
  cbx->SetTabStop(SIZE_A(tb), tb, 0);
  cbx->SetColor(0, RGB(255, 255, 96));
  cbx->SetColorSel(RGB(128, 0, 0), RGB(255, 255, 220));
*/
  r.Offset(SIZE_CBX_H + 2, 0);
  return cbx;
}
//-------------------------------------------------------------------
#define DIM_CBX_TIPS 200
//-------------------------------------------------------------------
bool pMenuBtn::create()
{
  PRect r(OFFS_BTN + 10, OFFS_BTN, OFFS_BTN + SIZE_BTN_H + 10, OFFS_BTN + SIZE_BTN_V);
  struct infoBtn
  {
    uint id;
    uint idBtn;
    uint idBtn2;
    LPCTSTR Text;
  };

  infoBtn iBtn[] = {
    DB_BTN(CM_BTN_SETUP, IDB_BITMAP_M_FOLDER, getStringOrDef(IDT_BROWSE, _T("Setup"))),
    DB_BTN(CM_BTN_FULL_SCREEN, IDB_BITMAP_M_ADD, getStringOrDef(IDT_ADD, _T("Full SCreen"))),
    DB_BTN(CM_BTN_MOVE_SCREEN, IDB_BITMAP_M_REM, getStringOrDef(IDT_REM, _T("Move"))),
    DB_BTN(CM_BTN_AUDIO_BASE, IDB_BITMAP_M_COPY, getStringOrDef(IDT_COPY, _T("Sound"))),
    };

  infoBtn iBtn2[] = {
    DB_BTN(CM_BTN_AUDIO_ALERT, IDB_BITMAP_M_PASTE, getStringOrDef(IDT_PASTE, _T("Alert"))),
    DB_BTN(CM_BTN_INFO, IDB_BITMAP_M_UPD, getStringOrDef(IDT_RELOAD, _T("Tips"))),
    DB_BTN(CM_BTN_RESET_TIME, IDB_BITMAP_M_SAVE, getStringOrDef(IDT_SAVE, _T("Reset Timer"))),
    DB_BTN(CM_BTN_TIMER_PAUSE_RESUME, IDB_BITMAP_M_EXPORT, getStringOrDef(IDT_EXPORT, _T("Pause/Resume"))),
//    DB_BTN(CM_BTN_IMPORT, IDB_BITMAP_M_IMPORT, _T("Importa")),
    DB_BTN(CM_BTN_DONE, IDB_BITMAP_M_DONE, getStringOrDef(IDT_DONE, _T("Exit"))),
    };
  POwnBtn::colorRect cr;
  cr.focus = RGB(0x3f, 0x7f,0xff);
  cr.tickness = 3;
  for(uint i = 0; i < SIZE_A(iBtn); ++i) {
    hilightBtn* btn = new hilightBtn(this, iBtn[i].id, r, NEW_BTN, NEW_BTN2, TXT_BTN);
    btn->setColorRect(cr);
    r.Offset(SIZE_BTN_H, 0);
    }
  PRect r2(r);
  r2.right = r2.left + SIZE_SLIDER_H;
  r2.Offset(5, 20);
  infoSlider is = { IDB_SLIDER_SKIN, 0, 0, IDB_SLIDER_DX, infoSlider::sHorzCurrTop, COLOR_BAR, COLOR_SEL };
  Slider = new PSliderWin(this, ICD_SLIDER, r2, is);
  Slider->setLimits(0, 1000);
  r.Offset(SIZE_SLIDER_H, 0);

  for(uint i = 0; i < SIZE_A(iBtn2); ++i) {
    hilightBtn* btn = new hilightBtn(this, iBtn2[i].id, r, NEW_BTN_, NEW_BTN2_, TXT_BTN_);
    btn->setColorRect(cr);
    r.Offset(SIZE_BTN_H, 0);
    }

  r = PRect(OFFS_BTN + 10, OFFS_BTN, OFFS_BTN + SIZE_CBX_H, SIZE_ROW + OFFS_ROW);
  r.Offset(0, OFFS_BTN + SIZE_BTN_V + OFFS_ROW);
  cbxSound = makeCbx(this, IDC_CBX_SOUND, r);
  cbxAlert = makeCbx(this, IDC_CBX_ALERT, r);
  r.Offset(20, 0);
  r.right += DIM_CBX_TIPS;
  cbxTips = makeCbx(this, IDC_CBX_TIPS, r, true);
  cbxTips->Attr.style &= ~CBS_DROPDOWNLIST;
  cbxTips->Attr.style |= CBS_DROPDOWN;
  r.Offset(DIM_CBX_TIPS - 4, 0);
  r.right = r.left + SIZE_ROW + 2;
  hilightBtn* btn = new hilightBtn(this, IDC_BTN_TIPS, r, new PBitmap(IDB_BITMAP_M_UPD_SET, getHInstance()), new PBitmap(IDB_BITMAP_M_UPD_SET2, getHInstance()));

  btn->setColorRect(cr);
  if(!baseClass::create())
    return false;
  fillCbxSound();
  fillCbxAlert();
  fillCbxTips();
  return true;
}
//----------------------------------------------------------------------------
static void addStringToComboBox(HWND hList, LPCTSTR str)
{
  int wid = extent(hList, str);
  int awi = SendMessage(hList, CB_GETDROPPEDWIDTH, 0, 0);
  if(wid > awi && wid < 800)
    ::SendMessage(hList, CB_SETDROPPEDWIDTH, wid, 0);
  ::SendMessage(hList, CB_ADDSTRING, 0, LPARAM(str));
}
//---------------------------------------------------------
static void commonFill(PComboBox* cbx, LPCTSTR key, LPCTSTR ext, LPCTSTR curr)
{
  SendMessage(*cbx, CB_RESETCONTENT, 0, 0);
  TCHAR path[_MAX_PATH];
  getKeyPath(key, path);
  _tcscat_s(path, _T("\\*."));
  _tcscat_s(path, ext);
  WIN32_FIND_DATA FindFileData;
  HANDLE hf = FindFirstFile(path, &FindFileData);
  int sel = -1;
  int ix = 0;
  if(hf != INVALID_HANDLE_VALUE) {
    do {
      if(!(FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes)) {
        if(!_tcsicmp(curr, FindFileData.cFileName))
          sel = ix;
        addStringToComboBox(*cbx, FindFileData.cFileName);
        ++ix;
        }
      } while(FindNextFile(hf, &FindFileData));
    FindClose(hf);
    }
  if(sel >= 0)
    SendMessage(*cbx, CB_SETCURSEL, sel, 0);
}
//---------------------------------------------------------
void pMenuBtn::fillCbxSound()
{
  TCHAR curr[_MAX_PATH];
  getKeyPath(AUDIO_BASE, curr);
  commonFill(cbxSound, AUDIO_BASE_FOLDER, _T("mp3"), curr);
}
//---------------------------------------------------------
void pMenuBtn::fillCbxAlert()
{
  TCHAR curr[_MAX_PATH];
  getKeyPath(AUDIO_ALERT, curr);
  commonFill(cbxAlert, AUDIO_ALERT_FOLDER, _T("mp3"), curr);
}
//---------------------------------------------------------
void pMenuBtn::fillCbxTips()
{
  HWND hwnd = *cbxTips;
  SendMessage(hwnd, CB_RESETCONTENT, 0, 0);
  TCHAR buff[4096];
  TCHAR t[128];
  for(uint i = 1;; ++i) {
    wsprintf(t, _T("%s%d"), TEXT_BASE, i);
    getKeyPath(t, buff);
    if(!*buff)
      break;
    addStringToComboBox(hwnd, buff);
    }
  int sel;
  getKeyParam(TEXT_CURR, (LPDWORD)&sel);
  if(sel)
    SendMessage(hwnd, CB_SETCURSEL, sel -1, 0);
  else
    SendMessage(hwnd, CB_SETCUEBANNER, 0, (LPARAM)_T("No Text"));

}
//---------------------------------------------------------
void pMenuBtn::reload(uint type)
{
  switch(type) {
    case 0:
      fillCbxSound();
      break;
    case 1:
      fillCbxAlert();
      break;
    case 2:
      fillCbxTips();
      break;
    }
}
//-------------------------------------------------------------------
bool pMenuBtn::refreshCbx(uint type, LPTSTR audio)
{
  PComboBox* cbx = 0;
  if(!type) {
    fillCbxSound();
    cbx = cbxSound;
    }
  else {
    fillCbxAlert();
    cbx = cbxAlert;
    }
  uint count = SendMessage(*cbx, CB_GETCOUNT, 0, 0);
  if(!count)
    return false;
  int ix = SendMessage(*cbx, CB_FINDSTRINGEXACT, -1, (LPARAM)audio);
  if(CB_ERR == ix) {
    SendMessage(*cbx, CB_SETCURSEL, 0, 0);
    SendMessage(*cbx, CB_GETLBTEXT, 0, (LPARAM)audio);
    }
  else
    SendMessage(*cbx, CB_SETCURSEL, ix, 0);
  return true;
}
//---------------------------------------------------------
int pMenuBtn::getCurrSel(uint type) // 0 -> audio, 1 -> effect, 2 -> tips
{
  PComboBox* cbx = 0;
  switch(type) {
    case 0:
      cbx = cbxSound;
      break;
    case 1:
      cbx = cbxAlert;
      break;
    case 2:
      cbx = cbxTips;
      break;
    default:
      return - 1;
    }
  return SendMessage(*cbx, CB_GETCURSEL, 0, 0);
}
//---------------------------------------------------------
void pMenuBtn::setCurrSel(int ix, uint type)
{
  PComboBox* cbx = 0;
  switch(type) {
    case 0:
      cbx = cbxSound;
      break;
    case 1:
      cbx = cbxAlert;
      break;
    case 2:
      cbx = cbxTips;
      break;
    default:
      return;
    }
  SendMessage(*cbx, CB_SETCURSEL, ix, 0);
}
//---------------------------------------------------------
void pMenuBtn::getCurrSel(LPTSTR buffer, uint type) // 0 -> audio, 1 -> effect, 2 -> tips
{
  *buffer = 0;
  PComboBox* cbx = 0;
  switch(type) {
    case 0:
      cbx = cbxSound;
      break;
    case 1:
      cbx = cbxAlert;
      break;
    case 2:
      cbx = cbxTips;
      break;
    default:
      return;
    }
  int sel = SendMessage(*cbx, CB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  SendMessage(*cbx, CB_GETLBTEXT, sel, (LPARAM)buffer);
}
//---------------------------------------------------------
bool pMenuBtn::saveCurrTips()
{
  TCHAR buff[4096];
  TCHAR buff2[4096];
  TCHAR t[128];
  if(!GetWindowText(*cbxTips, buff, SIZE_A(buff)))
    return false;
  for(uint i = 1;; ++i) {
    wsprintf(t, _T("%s%d"), TEXT_BASE, i);
    getKeyPath(t, buff2);
    if(!*buff2) {
      setKeyPath(t, buff);
      setKeyParam(TEXT_CURR, i);
      addStringToComboBox(*cbxTips, buff);
      return true;
      }
    if(!_tcsicmp(buff, buff2)) {
      setKeyParam(TEXT_CURR, i);
      return true;
      }
    }
  return false;
}
//---------------------------------------------------------
LRESULT pMenuBtn::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if(WM_CUSTOM_MSG_SLIDER == message) {
    PostMessage(*getParentWin<mainWin>(this), WM_CUSTOM_MSG_SLIDER, wParam, lParam);
    return 0;
    }
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case CM_BTN_SETUP:
        case CM_BTN_FULL_SCREEN:
        case CM_BTN_MOVE_SCREEN:
        case CM_BTN_AUDIO_BASE:
        case CM_BTN_AUDIO_ALERT:
        case CM_BTN_INFO:
        case CM_BTN_RESET_TIME:
        case CM_BTN_TIMER_PAUSE_RESUME:
        case CM_BTN_DONE:
        case IDC_BTN_TIPS:
          PostMessage(*getParentWin<mainWin>(this), WM_CUSTOM_BY_BTN, MAKEWPARAM(LOWORD(wParam), 0), lParam);
          break;
        case IDC_CBX_SOUND:
        case IDC_CBX_ALERT:
        case IDC_CBX_TIPS:
          PostMessage(*getParentWin<mainWin>(this), WM_CUSTOM_BY_BTN, wParam, lParam);
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//---------------------------------------------------------

