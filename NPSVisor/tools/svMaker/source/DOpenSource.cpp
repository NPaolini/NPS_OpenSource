//------- DOpenSource.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <shlobj.h>
#include <commdlg.h>
//----------------------------------------------------------------------------
#include "dChoosePath.h"
#include "macro_utils.h"
#include "pOpensave.h"
#include "common.h"
#include "sizer.h"
#include "dManageStdMsg.h"
#include "POwnBtnImageStd.h"
#include "PSplitWin.h"
#include "PMemo.h"
//-------------------------------------------------------------------
class exp_BaseContainer : public PSplitWin
{
  private:
    typedef PSplitWin baseClass;
  public:
    exp_BaseContainer(PWin* parent, uint id, LPCTSTR tit);
    ~exp_BaseContainer();
    bool create();

    PWin* getEdit() { return Ed; }
    bool preProcessMsg(MSG& msg);

  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    HBRUSH evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType);
    PControl* Tit;
    PControl* Ed;
    HBRUSH bkgTit;
};
//-------------------------------------------------------------------
//-------------------------------------------------------------------
class PDialogBtn : public PDialog
{
  private:
    typedef PDialog baseClass;
  public:
    PDialogBtn(PWin* parent, uint id, HINSTANCE hInst = 0);
    virtual ~PDialogBtn() { destroy(); }
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};
//-------------------------------------------------------------------
class myStatic : public PStatic
{
  private:
    typedef PStatic baseClass;
  public:
    myStatic(PWin* parent, PWin* ed, uint id, const PRect& r, LPCTSTR title = 0, HINSTANCE hinst = 0) :
      baseClass(parent, id, r, title, hinst), Ed(ed)
    {
      Attr.style |= SS_NOTIFY;
      Attr.style &= ~WS_TABSTOP;
    }
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    PWin* Ed;
};
//-------------------------------------------------------------------
class mySplitWin : public PSplitWin
{
  public:
    mySplitWin(PWin* parent, uint id, uint perc = 500, uint width = 3, uint style = PSplitter::PSW_HORZ, HINSTANCE hInst = 0) :
      PSplitWin(parent, id, perc, width, style, hInst) { Attr.style |= WS_CHILD ; }
    virtual ~mySplitWin() { destroy(); }
};
//----------------------------------------------------------------------------
class POpenSource : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    POpenSource(PWin* parent, LPCTSTR filename, dataProject& data, uint resId = IDD_DIALOG_OPEN_SOURCE, HINSTANCE hinstance = 0);
    virtual ~POpenSource() { destroy(); }
  public:
    virtual bool create();

  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CmOk();

  private:
    LPCTSTR Filename;
    dataProject& Data;
    void fillEdit(HWND hwed, setOfString& set);
    void saveToFile(P_File& pf, HWND hwed);
    void savePerc();
    PSplitter* Split;
    exp_BaseContainer* bodyClear;
    exp_BaseContainer* bodyCrypt;
    PDialogBtn* topBtn;
};
//-------------------------------------------------------------------
void openPageFileSource(PWin* parent, LPCTSTR filename, dataProject& data)
{
  POpenSource(parent, filename, data).modal();
}
//-------------------------------------------------------------------
LRESULT myStatic::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_LBUTTONUP:
      SetFocus(*Ed);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-------------------------------------------------------------------
#define BGK_TIT RGB(255, 255, 180)
#define H_TIT 22
#define ID_EDIT 255
#define ID_STATIC 355
#define H_TOP 36

#define ID_BOTTOM 201
//-------------------------------------------------------------------
exp_BaseContainer::exp_BaseContainer(PWin* parent, uint id, LPCTSTR tit) :
    baseClass(parent, id, 100, 3, PSplitter::PSW_VERT),
    Tit(0), Ed(0), bkgTit(0)
{
  Ed = new PMemo(this, ID_EDIT + id, PRect(0, 0, 10, 10));
  Ed->Attr.style |= ES_NOHIDESEL | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_HSCROLL | WS_TABSTOP;
  Tit = new myStatic(this, Ed, ID_STATIC + id, PRect(0, 0, 10, 10), tit);

  setWChild(Tit, Ed);
  PSplitter* split = getSplit();
  split->setLock(PSplitter::PSL_FIRST);
  split->setDim1(H_TIT);
  PBitmap bmp(IDB_BAR, getHInstance());
  bkgTit = CreatePatternBrush(bmp);
}
//-------------------------------------------------------------------
exp_BaseContainer::~exp_BaseContainer()
{
  destroy();
  DeleteObject(bkgTit);
}
//-------------------------------------------------------------------
bool exp_BaseContainer::create()
{
  Attr.style |= WS_CLIPCHILDREN;
  Attr.exStyle |= WS_EX_CONTROLPARENT;
  if(!baseClass::create())
    return false;
  Tit->setFont(D_FONT(22, 0, 0, _T("Comic Sans MS")));

  Ed->setFont(D_FONT(18, 0, 0, _T("Courier")));

  return true;
}
//----------------------------------------------------------------------------
bool exp_BaseContainer::preProcessMsg(MSG& msg)
{
  if(!toBool(IsDialogMessage(getHandle(), &msg)))
    return baseClass::preProcessMsg(msg);
  return true;
}
//-------------------------------------------------------------------
LRESULT exp_BaseContainer::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(HIWORD(wParam)) {
        case EN_SETFOCUS:
          SetFocus(*Ed);
          InvalidateRect(*Tit, 0, 0);
          break;
        case EN_KILLFOCUS:
          InvalidateRect(*Tit, 0, 0);
          break;
        }
      break;
    case WM_CTLCOLORSTATIC:
      do {
        HBRUSH result = evCtlColor((HDC)wParam, (HWND)lParam, message);
        if(result)
          return (LRESULT)result;
        } while(false);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//------------------------------------------------------------------------------
HBRUSH exp_BaseContainer::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  LONG id = GetWindowLong(hWndChild, GWL_ID);
  if(WM_CTLCOLORSTATIC == ctlType) {
    if(*Ed == GetFocus()) {
      SetBkMode(hdc, TRANSPARENT);
      return bkgTit;
      }
    SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
    return GetSysColorBrush(COLOR_BTNFACE);
    }
  return 0;
}
//-------------------------------------------------------------------
PDialogBtn::PDialogBtn(PWin* parent, uint id, HINSTANCE hInst) :
  baseClass(parent, id, hInst)
{
  int idBmp[] = { IDB_SAVE, IDB_DONE };
  int idBtn[] = { IDOK, IDCANCEL };

  for(uint i = 0; i < SIZE_A(idBtn); ++i) {
    PBitmap* bmp = new PBitmap(idBmp[i], getHInstance());
    new POwnBtnImageStd(this, idBtn[i], bmp, POwnBtnImageStd::wLeft, true);
    }
  Attr.exStyle |= WS_EX_CONTROLPARENT;
}
//-------------------------------------------------------------------
LRESULT PDialogBtn::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDOK:
        case IDCANCEL:
          do {
            POpenSource* m = getParentWin<POpenSource>(this);
            PostMessage(*m, WM_COMMAND, MAKEWPARAM(LOWORD(wParam), 0), 0);
            } while(false);
          break;
        }
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
static bool getDWORD(LPCTSTR skey, DWORD& target)
{
  myManageIni ini(SVMAKER_INI_FILE);
  ini.parse();
  LPCTSTR value = ini.getValue(skey, OPEN_SOURCE_PATH);
  if(value) {
    __int64 t = _tcstoi64(value, 0, 10);
    target = (DWORD)t;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
static bool setDWORD(LPCTSTR skey, DWORD source)
{
  myManageIni ini(SVMAKER_INI_FILE);
  ini.parse();
  TCHAR t[64];
  wsprintf(t, _T("%lu"), source);
  ini_param param = { skey, t };
  ini.addParam(param, OPEN_SOURCE_PATH);
  return ini.save();
}
//----------------------------------------------------------------------------
POpenSource::POpenSource(PWin* parent, LPCTSTR filename, dataProject& data, uint resId, HINSTANCE hinstance) :
      baseClass(parent, resId, hinstance), Filename(filename), Data(data), Split(0), bodyClear(0), bodyCrypt(0), topBtn(0)
{
  topBtn = new PDialogBtn(this, IDD_BUTTON);
  int perc = 500;
  DWORD split = 0;
  getDWORD(SPLIT_PATH, split);
  if(split)
    perc = split;

  PSplitWin* bottom = new mySplitWin(this, ID_BOTTOM, perc);

  Split = new PSplitter(this, topBtn, bottom, 100, 1, PSplitter::PSW_VERT);
  Split->setLock(PSplitter::PSL_FIRST);
  Split->setDrawStyle(PSplitter::PSD_NONE);
  Split->setDim1(H_TOP);
  bodyClear = new exp_BaseContainer(bottom, 10, _T("Not encrypted"));
  bodyCrypt = new exp_BaseContainer(bottom, 11, _T("Encrypted"));
  bottom->setWChild(bodyClear, bodyCrypt);
}
//----------------------------------------------------------------------------
extern void loadOneSet(setOfString& set, dataProject& datap, LPCTSTR filename);
//----------------------------------------------------------------------------
#define DIM_BUFF_EDIT_ROW (0xfff0)
void POpenSource::fillEdit(HWND hwed, setOfString& set)
{
  SetWindowText(hwed, _T(""));
  if(!set.setFirst())
    return;
  SendMessage(hwed, EM_LIMITTEXT, 0, 0);
  LPTSTR buff = new TCHAR[DIM_BUFF_EDIT_ROW];
  do {
    wsprintf(buff, _T("%d,"), set.getCurrId());
    uint len = _tcslen(buff);
    translateFromCRNL(buff + len, set.getCurrString());
    _tcscat_s(buff, DIM_BUFF_EDIT_ROW, _T("\r\n"));
    SendMessage(hwed, EM_REPLACESEL, 0, (LPARAM)buff);
    } while(set.setNext());
  delete []buff;
}
//-----------------------------------------------------------
static void checkOut(int& x, int& y)
{
  int width = GetSystemMetrics(SM_CXSCREEN);
  int height = GetSystemMetrics(SM_CYSCREEN);

  if(x < 0)
    x = 0;
  else if(x > width - 100)
    x = width - 100;
  if(y < 0)
    y = 0;
  else if(y > height - 100)
    y = height - 100;
}
//----------------------------------------------------------------------------
bool POpenSource::create()
{
  if(!baseClass::create())
    return false;
  {
  setOfString set;
  loadOneSet(set, Data, Filename);
  fillEdit(*bodyClear->getEdit(), set);
  }
  setOfString set;
  TCHAR file[_MAX_PATH] = _T("#");
  _tcscat_s(file, Filename);
  loadOneSet(set, Data, file);
  fillEdit(*bodyCrypt->getEdit(), set);


  int w = GetSystemMetrics(SM_CXFULLSCREEN);
  int h = GetSystemMetrics(SM_CYFULLSCREEN);

  PRect r(0, 0, w * 3 / 4, h * 3 / 4);
  r.MoveTo((w - r.right) / 2, (h - r.bottom) / 2);
  SetWindowPos(*this, 0, r.left, r.top, r.Width(), r.Height(), SWP_NOZORDER);
  DWORD coords = 0;
  getDWORD(OPEN_SOURCE_POS, coords);
  if(coords && (DWORD)-1 != coords) {
    r.left = (short)LOWORD(coords);
    r.top = (short)HIWORD(coords);
    checkOut(r.left, r.top);
    getDWORD(OPEN_SOURCE_DIM, coords);
    if(coords && (DWORD)-1 != coords) {
      r.right = (short)LOWORD(coords) + r.left;
      r.bottom = (short)HIWORD(coords) + r.top;
      }
    SetWindowPos(*this, 0, r.left, r.top, r.Width(), r.Height(), SWP_NOZORDER);
    }
  else if((DWORD)-1 == coords)
    ShowWindow(*this, SW_MAXIMIZE);
  return true;
}
//----------------------------------------------------------------------------
void POpenSource::savePerc()
{
  PSplitWin* bottom = (PSplitWin*)Split->getW2();
  int perc = bottom->getSplitter()->getPerc();
  setDWORD(SPLIT_PATH, perc);
}
//----------------------------------------------------------------------------
static bool needMaximized()
{
  DWORD coords = 0;
  getDWORD(OPEN_SOURCE_POS, coords);
  return (DWORD)-1 == coords;
}
//----------------------------------------------------------------------------
bool isMaximized(HWND hwnd)
{
  WINDOWPLACEMENT wp;
  ZeroMemory(&wp, sizeof(wp));
  wp.length = sizeof(wp);
  if(GetWindowPlacement(hwnd, &wp))
    return SW_SHOWMAXIMIZED == (SW_SHOWMAXIMIZED & wp.showCmd); // MaximizedWin;
  return false;
}
//-------------------------------------------------------------------
LRESULT POpenSource::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_DESTROY:
      if(isMaximized(hwnd)) {
        setDWORD(OPEN_SOURCE_POS, (DWORD)-1);
        }
      else {
        PRect r;
        GetWindowRect(hwnd, r);
        DWORD t = MAKELONG(r.left, r.top);
        setDWORD(OPEN_SOURCE_POS, t);
        t = MAKELONG(r.Width(), r.Height());
        setDWORD(OPEN_SOURCE_DIM, t);
        }
      savePerc();
      break;
    }
  if(Split)
    Split->windowProcSplit(hwnd, message, wParam, lParam);
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void POpenSource::saveToFile(P_File& pf, HWND hwed)
{
  LPTSTR buff = new TCHAR[DIM_BUFF_EDIT_ROW];
  uint nLine = SendMessage(hwed, EM_GETLINECOUNT, 0, 0);
  for(uint i = 0; i < nLine; ++i) {
    buff[0] = DIM_BUFF_EDIT_ROW;
    int len = SendMessage(hwed, EM_GETLINE, i, (LPARAM)buff);
    buff[len] = 0;
    _tcscat_s(buff, DIM_BUFF_EDIT_ROW, _T("\r\n"));
    writeStringChkUnicode(pf, buff);
    }
  delete []buff;
}
//----------------------------------------------------------------------------
void POpenSource::CmOk()
{
  TCHAR crypt[_MAX_PATH];
  _tcscpy_s(crypt, Data.newPath);
  appendPath(crypt, Data.systemPath);
  appendPath(crypt, Data.getSubPath());

  TCHAR clear[_MAX_PATH];
  _tcscpy_s(clear, crypt);

  appendPath(crypt, _T("#"));
  _tcscat_s(crypt, Filename);

  appendPath(clear, Filename);
  _tcscat_s(clear, PAGE_EXT);
  _tcscat_s(crypt, PAGE_EXT);

  do {
    P_File pfCrypt(crypt);
    pfCrypt.appendback();

    P_File pfClear(clear);
    pfClear.appendback();
    } while(false);

  manageCryptPage mcp;
  bool crypted = mcp.isCrypted();
  if(crypted)
    mcp.makePathCrypt(crypt);

  P_File pfCrypt(crypt, P_CREAT);
  P_File pfClear(clear, P_CREAT);

  if(pfCrypt.P_open()) {
    if(pfClear.P_open()) {
      saveToFile(pfClear, *bodyClear->getEdit());
      saveToFile(pfCrypt, *bodyCrypt->getEdit());
      if(crypted) {
        _tcscpy_s(crypt, Data.newPath);
        appendPath(crypt, Data.systemPath);
        appendPath(crypt, Data.getSubPath());
        appendPath(crypt, _T("#"));
        _tcscat_s(crypt, Filename);
        _tcscat_s(crypt, PAGE_EXT);
        mcp.releaseFileCrypt(crypt);
        }
      }
    }
  baseClass::CmOk();
}
//----------------------------------------------------------------------------
