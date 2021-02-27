//----------- mainDlg.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
//#define ISOLATION_AWARE_ENABLED 1
//#include <windows.h>
//#include <commctrl.h>
#include "precHeader.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#include <stdio.h>
//----------------------------------------------------------------------------
#include "mainDlg.h"
#include "macro_utils.h"
#include "p_matchStream.h"
#include "P_freeList.h"
#include "p_pattern.h"
#include "P_Avl.h"
#include "P_file.h"
#include "p_checkKey.h"
#include "smartPS.h"
#include "pCombobox.h"
#include "pBitmap.h"
#include "popensave.h"
#include "p_manage_ini.h"
#include "progressBar.h"
#include "pedit.h"
#include "pCheckBox.h"
#include "pComboBox.h"
#include "pRadioBtn.h"
//-----------------------------------------------------------
bool manageKey::evKeyUp(UINT& key)
{
  if (VK_DOWN == key || VK_RETURN == key) {
    up_click(VK_TAB);
    return true;
  }
  if (VK_UP == key) {
    up_click(VK_TAB);
    up_click(VK_SHIFT);
    return true;
  }
  return false;
}
//-----------------------------------------------------------
bool manageKey::evKeyDown(UINT& key)
{
  if (VK_DOWN == key || VK_RETURN == key) {
    up_click(key);
    down_click(VK_TAB);
    return true;
  }

  if (VK_UP == key) {
    up_click(VK_UP);
    down_click(VK_SHIFT);
    down_click(VK_TAB);
    return true;
  }
  return false;
}
//-----------------------------------------------------------
bool manageKey::evChar(UINT& key)
{
  if (VK_RETURN == key) {
    click(VK_TAB);
    return true;
  }
  if (VK_ESCAPE == key) {
    return true;
  }
  return false;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
int manageKey::verifyKey()
{
  if (GetKeyState(VK_RETURN) & 0x8000)
    return VK_RETURN;
  if (GetKeyState(VK_DOWN) & 0x8000)
    return VK_DOWN;
  if (GetKeyState(VK_UP) & 0x8000)
    return VK_UP;
  if (GetKeyState(VK_ESCAPE) & 0x8000)
    return VK_ESCAPE;
  return 0;
}
//-----------------------------------------------------------
class myEdit : public PEdit
{
private:
  typedef PEdit baseClass;
public:
  myEdit(PWin* parent, uint idc) : PEdit(parent, idc) {}
  ~myEdit() { destroy(); }
protected:
  virtual int verifyKey() { int res = manageKey::verifyKey(); return res ? res : baseClass::verifyKey(); }
  virtual bool evKeyDown(UINT& key) { return manageKey::evKeyDown(key); }
  virtual bool evKeyUp(UINT& key) { return manageKey::evKeyUp(key); }
  virtual bool evChar(UINT& key) { return manageKey::evChar(key); }
};
//-----------------------------------------------------------
class myCBox : public PComboBox
{
private:
  typedef PComboBox baseClass;
public:
  myCBox(PWin* parent, uint idc) : PComboBox(parent, idc) {}
  ~myCBox() { destroy(); }
protected:
  virtual int verifyKey() { int res = manageKey::verifyKey(); return res ? res : baseClass::verifyKey(); }
  virtual bool evKeyDown(UINT& key) { return manageKey::evKeyDown(key); }
  virtual bool evKeyUp(UINT& key) { return manageKey::evKeyUp(key); }
  virtual bool evChar(UINT& key) { return manageKey::evChar(key); }
};
//-----------------------------------------------------------
class myCheck : public PCheckBox
{
private:
  typedef PCheckBox baseClass;
public:
  myCheck(PWin* parent, uint idc) : PCheckBox(parent, idc) {}
  ~myCheck() { destroy(); }
protected:
  virtual int verifyKey() { int res = manageKey::verifyKey(); return res ? res : baseClass::verifyKey(); }
  virtual bool evKeyDown(UINT& key) { return manageKey::evKeyDown(key); }
  virtual bool evKeyUp(UINT& key) { return manageKey::evKeyUp(key); }
  virtual bool evChar(UINT& key) { return manageKey::evChar(key); }
};
//-----------------------------------------------------------
class myBtn : public PButton
{
private:
  typedef PButton baseClass;
public:
  myBtn(PWin* parent, uint idc) : PButton(parent, idc) {}
  ~myBtn() { destroy(); }
protected:
  virtual int verifyKey() { int res = manageKey::verifyKey(); return res ? res : baseClass::verifyKey(); }
  virtual bool evKeyDown(UINT& key) { return manageKey::evKeyDown(key); }
  virtual bool evKeyUp(UINT& key) { return manageKey::evKeyUp(key); }
  virtual bool evChar(UINT& key) { return manageKey::evChar(key); }
};
//-----------------------------------------------------------
class myRadio : public PRadioButton
{
private:
  typedef PRadioButton baseClass;
public:
  myRadio(PWin* parent, uint idc) : PRadioButton(parent, idc) {}
  ~myRadio() { destroy(); }
protected:
  virtual int verifyKey() { int res = manageKey::verifyKey(); return res ? res : baseClass::verifyKey(); }
  virtual bool evKeyDown(UINT& key) { return manageKey::evKeyDown(key); }
  virtual bool evKeyUp(UINT& key) { return manageKey::evKeyUp(key); }
  virtual bool evChar(UINT& key) { return manageKey::evChar(key); }
};
//----------------------------------------------------------------------------
PDShowVars::PDShowVars(PWin* parent, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance)
{
  new myEdit(this, IDC_EDIT_P1);
  new myEdit(this, IDC_EDIT_P2);
  new myEdit(this, IDC_EDIT_P3);
  new myEdit(this, IDC_EDIT_WIDTH_PICK);
  new myEdit(this, IDC_EDIT_HEIGHT_PICK);
  new myEdit(this, IDC_EDIT_PERC_PHOTO);
  new myEdit(this, IDC_EDIT_WIDTH_PHOTO);
  new myEdit(this, IDC_EDIT_HEIGHT_PHOTO);
  new myEdit(this, IDC_EDIT_PERC_LOGO);
  new myEdit(this, IDC_EDIT_WIDTH_LOGO);
  new myEdit(this, IDC_EDIT_WIDTH_LOGO_CALC);
  new myEdit(this, IDC_EDIT_HEIGHT_LOGO_CALC);
  new myEdit(this, IDC_EDIT_X);
  new myEdit(this, IDC_EDIT_Y);

//  new myCBox(this, IDC_COMBO_POS);
  new myCBox(this, IDC_COMBO_LIST);

  new myCheck(this, IDC_CHECK_REDIM_PHOTO);
  new myCheck(this, IDC_CHECK_REDIM_LOGO);

  new myBtn(this, IDC_RUN);
  new myBtn(this, IDC_DONE);
  new myBtn(this, IDC_BUTTON_LOGO);

  new myRadio(this, IDC_RADIO_USE_PERC);
  new myRadio(this, IDC_RADIO_USE_FIX);
}
//----------------------------------------------------------------------------
PDShowVars::~PDShowVars()
{
  destroy();
}
//----------------------------------------------------------------------------
#define INI_NAME _T("add_banner.ini")
#define BLK_PATH _T("Path")
  #define S_PATH _T("Source")
  #define T_PATH _T("Target")
  #define L_PATH _T("Logo")
#define BLK_OFFS _T("Distance")
  #define X_OFFS _T("X")
  #define Y_OFFS _T("Y")
  #define W_OFFS _T("Position")
#define BLK_REDIM _T("Redim")
  #define REDIM_PHOTO _T("Photo")
  #define REDIM_LOGO _T("Logo")
#define REDIM_PHOTO_PERC _T("Photo_perc")
  #define REDIM_PHOTO_PERC _T("Photo_perc")
  #define REDIM_LOGO_HOW _T("Logo_how")
  #define REDIM_LOGO_PERC _T("Logo_perc")
  #define REDIM_LOGO_WIDTH _T("Logo_width")
//----------------------------------------------------------------------------
bool PDShowVars::create()
{
  PComboBox* cb = new myCBox(this, IDC_COMBO_POS);
  if(!baseClass::create())
    return false;
  LPCTSTR vals[] = { _T("Casuale"), _T("alto-sinistra"), _T("alto-destra") , _T("basso-destra") , _T("basso-sinistra") };
  for (uint i = 0; i < SIZE_A(vals); ++i)
    SendMessage(*cb, CB_ADDSTRING, 0, (LPARAM)vals[i]);
  loadIni();
  enableDimPhoto();
  enableDimLogo();
  showDimLogo();
  HICON icon = LoadIcon(getHInstance(), MAKEINTRESOURCE(IDI_ICON1));
  SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_BIG), LPARAM(icon));
  SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_SMALL), LPARAM(icon));
  return true;
}
//-----------------------------------------------------------------------------
LRESULT PDShowVars::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_RUN:
          run();
          break;
        case IDC_DONE:
          PostQuitMessage(0);
          break;
        case IDC_BUTTON_SOURCE:
          searchFolder(IDC_EDIT_P1);
          break;
        case IDC_BUTTON_TARGET:
          searchFolder(IDC_EDIT_P2);
          break;
        case IDC_BUTTON_LOGO:
          searchFile();
          break;
        case IDC_CHECK_REDIM_PHOTO:
          enableDimPhoto();
          break;
        case IDC_CHECK_REDIM_LOGO:
        case IDC_RADIO_USE_PERC:
        case IDC_RADIO_USE_FIX:
          enableDimLogo();
          break;
        }
      switch (HIWORD(wParam)) {
        case CBN_SELCHANGE:
          if(IDC_COMBO_LIST == LOWORD(wParam))
            enableDimPhotoByList();
          break;
        case EN_CHANGE:
          switch(LOWORD(wParam)) {
            case IDC_EDIT_HEIGHT_PHOTO:
            case IDC_EDIT_PERC_PHOTO:
            case IDC_EDIT_WIDTH_PHOTO:
              calcDim(LOWORD(wParam));
              break;
            case IDC_EDIT_PERC_LOGO:
            case IDC_EDIT_WIDTH_LOGO:
              calcDimLogo();
              break;
            }
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void PDShowVars::calcDimLogo()
{
  int w;
  GET_INT(IDC_EDIT_WIDTH_PICK_LOGO, w);
  if (!w)
    return;
  int h;
  GET_INT(IDC_EDIT_HEIGHT_PICK_LOGO, h);
  if (IS_CHECKED(IDC_RADIO_USE_FIX)) {
    int curr;
    GET_INT(IDC_EDIT_WIDTH_LOGO, curr);
    SET_INT(IDC_EDIT_WIDTH_LOGO_CALC, curr);
    curr = ROUND_REAL((double)curr * h / w);
    SET_INT(IDC_EDIT_HEIGHT_LOGO_CALC, curr);
    }
  else {
    int curr;
    GET_INT(IDC_EDIT_WIDTH_PHOTO, curr);
    if(!curr) {
      forceRefreshList();
      GET_INT(IDC_EDIT_WIDTH_PHOTO, curr);
      }
    if (!curr)
      return;
    int perc;
    GET_INT(IDC_EDIT_PERC_LOGO, perc);
    curr = ROUND_REAL(curr * perc / 100.0);
    SET_INT(IDC_EDIT_WIDTH_LOGO_CALC, curr);
    curr = ROUND_REAL((double)curr * h / w);
    SET_INT(IDC_EDIT_HEIGHT_LOGO_CALC, curr);
    }
}
//----------------------------------------------------------------------------
void PDShowVars::showDimLogo()
{
  TCHAR s[_MAX_PATH];
  GET_TEXT(IDC_EDIT_P3, s);
  PBitmap sbmp(s);
  SIZE sz = sbmp.getSize();
  SET_INT(IDC_EDIT_WIDTH_PICK_LOGO, sz.cx);
  SET_INT(IDC_EDIT_HEIGHT_PICK_LOGO, sz.cy);
  calcDimLogo();
}
//----------------------------------------------------------------------------
void PDShowVars::calcDim(uint idc_from)
{
  static bool inExec;
  if(inExec)
    return;
  inExec = true;
  int w = 0;
  GET_INT(IDC_EDIT_WIDTH_PICK, w);
  int h = 0;
  GET_INT(IDC_EDIT_HEIGHT_PICK, h);
  if(!w || ! h) {
    inExec = false;
    return;
    }
  if (IDC_EDIT_PERC_PHOTO == idc_from) {
    int p = 0;
    GET_INT(IDC_EDIT_PERC_PHOTO, p);
    w = ROUND_REAL(w * p / 100.0);
    h = ROUND_REAL(h * p / 100.0);
    SET_INT(IDC_EDIT_WIDTH_PHOTO, w);
    SET_INT(IDC_EDIT_HEIGHT_PHOTO, h);
    }
  else if (IDC_EDIT_WIDTH_PHOTO == idc_from) {
    int p = 0;
    GET_INT(IDC_EDIT_WIDTH_PHOTO, p);
    p = ROUND_REAL(p * 100.0 / w);
    SET_INT(IDC_EDIT_PERC_PHOTO, p);
    if (p)
      h = ROUND_REAL(h * p / 100.0);
    else
      h = 0;
      SET_INT(IDC_EDIT_HEIGHT_PHOTO, h);
    }
  else if (IDC_EDIT_HEIGHT_PHOTO == idc_from) {
    int p = 0;
    GET_INT(IDC_EDIT_HEIGHT_PHOTO, p);
    p = ROUND_REAL(p * 100.0 / h);
    SET_INT(IDC_EDIT_PERC_PHOTO, p);
    if(p)
      w = ROUND_REAL(w * p / 100.0);
    else
      w = 0;
    SET_INT(IDC_EDIT_WIDTH_PHOTO, w);
    }
  calcDimLogo();
  inExec = false;
}
//----------------------------------------------------------------------------
struct infoBuffer
{
  uint dim;
  LPBYTE buff;
  infoBuffer() : dim(0), buff(0) {}
  ~infoBuffer() { delete []buff; }
  Gdiplus::PropertyItem* getItem() { return (Gdiplus::PropertyItem*)buff; }
};
//----------------------------------------------------------------------------
void checkRotation(PBitmap& sbmp, infoBuffer& ib)
{
  const p_gdiplusBitmap m_pBitmap = sbmp.getManBmp();
  if (!m_pBitmap)
    return;
  uint TotalBuffer = m_pBitmap->GetPropertyItemSize(PropertyTagOrientation);
  if (TotalBuffer > ib.dim) {
    ib.dim = TotalBuffer;
    delete []ib.buff;
    ib.buff = new BYTE[ib.dim];
    }
  Gdiplus::PropertyItem* pItem = ib.getItem();
  int result = m_pBitmap->GetPropertyItem(PropertyTagOrientation, TotalBuffer, pItem);
  if (result)
    return;

  BYTE o = *(BYTE*)(pItem[0].value);
  switch (o) {
  case 2:
    PBitmap::rotateInside(&sbmp, Gdiplus::RotateNoneFlipX);
    break;
  case 3:
    PBitmap::rotateInside(&sbmp, Gdiplus::RotateNoneFlipXY);
    break;
  case 4:
    PBitmap::rotateInside(&sbmp, Gdiplus::RotateNoneFlipY);
    break;
  case 5:
    PBitmap::rotateInside(&sbmp, Gdiplus::Rotate90FlipX);
    break;
  case 6:
    PBitmap::rotateInside(&sbmp, Gdiplus::Rotate90FlipNone);
    break;
  case 7:
    PBitmap::rotateInside(&sbmp, Gdiplus::Rotate90FlipY);
    break;
  case 8:
    PBitmap::rotateInside(&sbmp, Gdiplus::Rotate90FlipXY);
    break;
  }
}
//----------------------------------------------------------------------------
void PDShowVars::enableDimPhotoByList()
{
  HWND hcb = GetDlgItem(*this, IDC_COMBO_LIST);
  int sel = SendMessage(hcb, CB_GETCURSEL, 0, 0);
  if (CB_ERR == sel)
    return;
  TCHAR t[_MAX_PATH];
  int res = SendMessage(hcb, CB_GETLBTEXT, sel, (LPARAM)t);
  if (CB_ERR == res)
    return;
  ENABLE(IDC_EDIT_PERC_PHOTO, true);
  TCHAR s[_MAX_PATH];
  GET_TEXT(IDC_EDIT_P1, s);
  appendPath(s, t);
  PBitmap sbmp(s);
  infoBuffer ib;
  checkRotation(sbmp, ib);
  SIZE sz = sbmp.getSize();
  SET_INT(IDC_EDIT_WIDTH_PICK, sz.cx);
  SET_INT(IDC_EDIT_HEIGHT_PICK, sz.cy);
  calcDim(IDC_EDIT_WIDTH_PHOTO);
}
//----------------------------------------------------------------------------
void PDShowVars::enableDimPhoto()
{
  UINT idc[] = { IDC_EDIT_PERC_PHOTO, IDC_EDIT_WIDTH_PHOTO, IDC_EDIT_HEIGHT_PHOTO, IDC_COMBO_LIST };
  bool enable = IS_CHECKED(IDC_CHECK_REDIM_PHOTO);
  if (!enable) {
    HWND hwcb = GetDlgItem(*this, IDC_COMBO_LIST);
    SendMessage(hwcb, CB_RESETCONTENT, 0, 0);
    SET_INT(IDC_EDIT_WIDTH_PICK, 0);
    SET_INT(IDC_EDIT_HEIGHT_PICK, 0);
  }
  else
    fillListImage();
  for(uint i = 0; i < SIZE_A(idc); ++i)
    ENABLE(idc[i], enable);
  ENABLE(IDC_EDIT_PERC_PHOTO, false);
}
//----------------------------------------------------------------------------
void PDShowVars::enableDimLogo()
{
  UINT idc[] = { IDC_RADIO_USE_PERC, IDC_RADIO_USE_FIX, IDC_EDIT_PERC_LOGO, IDC_EDIT_WIDTH_LOGO };
  bool enable = IS_CHECKED(IDC_CHECK_REDIM_LOGO);
  for(uint i = 0; i < SIZE_A(idc); ++i)
    ENABLE(idc[i], enable);
  if(enable) {
    if(IS_CHECKED(IDC_RADIO_USE_PERC))
      ENABLE(IDC_EDIT_WIDTH_LOGO, false);
    else
      ENABLE(IDC_EDIT_PERC_LOGO, false);
    }
  calcDimLogo();
}
//----------------------------------------------------------------------------
void calcWhere(int where, const SIZE& sz, const SIZE& sz2, int& x, int& y)
{
  if(0 == where)
    where = rand() % 4 + 1;
  switch (where) {
    case 1:
      break;
    case 3:
      y = sz.cy - sz2.cy - y;
    case 2:
      x = sz.cx - sz2.cx - x;
      break;
    case 4:
      y = sz.cy - sz2.cy - y;
      break;
    }
}
//----------------------------------------------------------------------------
struct infoRedim
{
  SIZE szImg;
  SIZE szLogo;
  int resizeLogo; // 0 - > noResize, 1 -> use sz, -> is perc
};
//----------------------------------------------------------------------------
void perform(LPCTSTR sPath, LPCTSTR tPath, LPCTSTR filename, int x, int y, PBitmap& logo, int where, const infoRedim& ir, infoBuffer& ib)
{
  TCHAR s[_MAX_PATH];
  _tcscpy_s(s, sPath);
  appendPath(s, filename);
  PBitmap sbmp(s);
  checkRotation(sbmp, ib);
  SIZE sz = sbmp.getSize();
  if (ir.szImg.cx && ir.szImg.cy) {
    double perc = sz.cx;
    perc /= sz.cy;
    sz.cx = ir.szImg.cx;
    sz.cy = ROUND_REAL(sz.cx / perc);
    }

  const p_gdiplusBitmap m_pBitmap = sbmp.getManBmp();
  if (!m_pBitmap)
    return;
  Gdiplus::Bitmap* newBitmap = new Gdiplus::Bitmap(sz.cx, sz.cy, m_pBitmap->GetPixelFormat());
  Gdiplus::Graphics graphics(newBitmap);
  graphics.DrawImage(m_pBitmap, 0, 0, sz.cx, sz.cy);

  SIZE sz2 = logo.getSize();
  const p_gdiplusBitmap m_pBitmap2 = logo.getManBmp();
  if (!m_pBitmap2)
    return;
  if (ir.resizeLogo) {
    if(ir.resizeLogo > 1) {
#if 1
      double perc = sz2.cx;
      perc /= sz2.cy;
      sz2.cx = ROUND_REAL(sz.cx * ir.resizeLogo / 100.0);
      sz2.cy = ROUND_REAL(sz2.cx * perc);
#else
      int perc = sz2.cx * 100 / sz2.cy;
      sz2.cx = sz.cx * ir.resizeLogo / 100;
      sz2.cy = sz2.cx * perc / 100;
#endif
      }
    else
      sz2 = ir.szLogo;
    }
  calcWhere(where, sz, sz2, x, y);
  graphics.DrawImage(m_pBitmap2, x, y, sz2.cx, sz2.cy);
  PBitmap final(newBitmap);
  _tcscpy_s(s, tPath);
  appendPath(s, filename);
  final.saveAs(s);
}
//-------------------------------------------------------------------
static bool isValidFile(LPCTSTR filename)
{
  TCHAR ext[64];
  _tsplitpath_s(filename, 0, 0, 0, 0, 0, 0, ext, SIZE_A(ext));
  LPCTSTR enabled[] = { _T(".jpg"), _T(".png"), _T(".gif"), _T(".bmp") };
  for (uint i = 0; i < SIZE_A(enabled); ++i)
    if (!_tcsicmp(enabled[i], ext))
      return true;
  return false;
}
//-------------------------------------------------------------------
void fillList(PVect<LPCTSTR>& list, LPCTSTR path)
{
  if (!path)
    return;
  TCHAR old[_MAX_PATH];
  GetCurrentDirectory(SIZE_A(old), old);

  SetCurrentDirectory(path);

  WIN32_FIND_DATA FindFileData;
  TCHAR files[] = _T("*.*");
  HANDLE hf = FindFirstFile(files, &FindFileData);
  if (hf != INVALID_HANDLE_VALUE) {
    do {
      if (!(FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes)) {
        if (isValidFile(FindFileData.cFileName))
          list[list.getElem()] = str_newdup(FindFileData.cFileName);
      }
    } while (FindNextFile(hf, &FindFileData));

    FindClose(hf);
    }
  SetCurrentDirectory(old);
}
//----------------------------------------------------------------------------
LPCTSTR filterExt[] = { _T(".png"), _T(".jpg"), _T(".jpeg"), _T(".gif"), _T(".bmp"), _T(".tif"), _T(".emf"), _T(".wmf"), 0 };
//----------------------------------------------------------------------------
LPCTSTR filterOpen =
_T("File immagini (jfif, png, gif, bmp, tif, emf, wmf)\0")
_T("*.png;*.jpg;*.jpeg;*.gif;*.bmp;*.tif;*.emf;*.wmf\0");
//----------------------------------------------------------------------------
static DWORD lastIx;
//----------------------------------------------------------------------------
bool openFileImg(HWND owner, LPTSTR file)
{
  infoOpenSave Info(filterExt, filterOpen, infoOpenSave::OPEN_F, lastIx, file);
  POpenSave open(owner);

  if (open.run(Info)) {
    lastIx = Info.ixFilter;
    _tcscpy_s(file, _MAX_PATH, open.getFile());
    return true;
  }
  return false;
}
//----------------------------------------------------------------------------
void PDShowVars::loadIni()
{
  TCHAR path[_MAX_PATH];
  GetModuleDirName(SIZE_A(path), path);
  appendPath(path, INI_NAME);
  manageIni mIni(path);
  if (mIni.parse()) {
    LPCTSTR t = mIni.getValue(S_PATH, BLK_PATH);
    if (t)
      SET_TEXT(IDC_EDIT_P1, t);
    t = mIni.getValue(T_PATH, BLK_PATH);
    if (t)
      SET_TEXT(IDC_EDIT_P2, t);
    t = mIni.getValue(L_PATH, BLK_PATH);
    if (t)
      SET_TEXT(IDC_EDIT_P3, t);

    t = mIni.getValue(X_OFFS, BLK_OFFS);
    if (t)
      SET_TEXT(IDC_EDIT_X, t);
    t = mIni.getValue(Y_OFFS, BLK_OFFS);
    if (t)
      SET_TEXT(IDC_EDIT_Y, t);
    t = mIni.getValue(W_OFFS, BLK_OFFS);
    if (t) {
      int sel = _ttoi(t);
      SendMessage(GetDlgItem(*this, IDC_COMBO_POS), CB_SETCURSEL, sel, 0);
      }
    t = mIni.getValue(REDIM_PHOTO, BLK_REDIM);
    if(t)
      if(_T('1') == *t)
        SET_CHECK(IDC_CHECK_REDIM_PHOTO);
    t = mIni.getValue(REDIM_PHOTO_PERC, BLK_REDIM);
    if (t)
      SET_TEXT(IDC_EDIT_WIDTH_PHOTO, t);
//    SET_TEXT(IDC_EDIT_PERC_PHOTO, t);
    t = mIni.getValue(REDIM_LOGO, BLK_REDIM);
    if (t)
      if (_T('1') == *t)
        SET_CHECK(IDC_CHECK_REDIM_LOGO);
    t = mIni.getValue(REDIM_LOGO_HOW, BLK_REDIM);
    if (t) {
      if (_T('0') == *t)
        SET_CHECK(IDC_RADIO_USE_PERC);
      else if(_T('1') == *t)
        SET_CHECK(IDC_RADIO_USE_FIX);
      }
    else
      SET_CHECK(IDC_RADIO_USE_PERC);

    t = mIni.getValue(REDIM_LOGO_PERC, BLK_REDIM);
    if (t)
      SET_TEXT(IDC_EDIT_PERC_LOGO, t);
    t = mIni.getValue(REDIM_LOGO_WIDTH, BLK_REDIM);
    if (t)
      SET_TEXT(IDC_EDIT_WIDTH_LOGO, t);
  }
}
//----------------------------------------------------------------------------
void PDShowVars::saveIni()
{
  TCHAR t[_MAX_PATH];
  GetModuleDirName(SIZE_A(t), t);
  appendPath(t, INI_NAME);
  manageIni mIni(t);
#if 1
  mIni.parse();

  GET_TEXT(IDC_EDIT_P1, t);
  ini_param i_p;
  i_p.name = S_PATH;
  i_p.value = t;
  mIni.addParam(i_p, BLK_PATH);
  GET_TEXT(IDC_EDIT_P2, t);
  i_p.name = T_PATH;
  mIni.addParam(i_p, BLK_PATH);
  GET_TEXT(IDC_EDIT_P3, t);
  i_p.name = L_PATH;
  mIni.addParam(i_p, BLK_PATH);

  GET_TEXT(IDC_EDIT_X, t);
  i_p.name = X_OFFS;
  mIni.addParam(i_p, BLK_OFFS);
  GET_TEXT(IDC_EDIT_Y, t);
  i_p.name = Y_OFFS;
  mIni.addParam(i_p, BLK_OFFS);

  int sel = SendMessage(GetDlgItem(*this, IDC_COMBO_POS), CB_GETCURSEL, 0, 0);
  if (CB_ERR != sel) {
    wsprintf(t, _T("%d"), sel);
    i_p.name = W_OFFS;
    mIni.addParam(i_p, BLK_OFFS);
    }

  t[0] = _T('0');
  if(IS_CHECKED(IDC_CHECK_REDIM_PHOTO))
    t[0] = _T('1');
  t[1] = 0;
  i_p.name = REDIM_PHOTO;
  mIni.addParam(i_p, BLK_REDIM);

  t[0] = _T('0');
  if (IS_CHECKED(IDC_CHECK_REDIM_LOGO))
    t[0] = _T('1');
  t[1] = 0;
  i_p.name = REDIM_LOGO;
  mIni.addParam(i_p, BLK_REDIM);

  GET_TEXT(IDC_EDIT_WIDTH_PHOTO, t);
  i_p.name = REDIM_PHOTO_PERC;
  mIni.addParam(i_p, BLK_REDIM);

  t[0] = _T('?');
  if (IS_CHECKED(IDC_RADIO_USE_PERC))
    t[0] = _T('0');
  else if (IS_CHECKED(IDC_RADIO_USE_FIX))
    t[0] = _T('1');
  t[1] = 0;
  i_p.name = REDIM_LOGO_HOW;
  mIni.addParam(i_p, BLK_REDIM);

  GET_TEXT(IDC_EDIT_PERC_LOGO, t);
  i_p.name = REDIM_LOGO_PERC;
  mIni.addParam(i_p, BLK_REDIM);

  GET_TEXT(IDC_EDIT_WIDTH_LOGO, t);
  i_p.name = REDIM_LOGO_WIDTH;
  mIni.addParam(i_p, BLK_REDIM);


#else
  mIni.flush();

  mIni.createBlock(BLK_PATH);
  GET_TEXT(IDC_EDIT_P1, t);
  ini_param i_p;
  i_p.name = S_PATH;
  i_p.value = t;
  mIni.createParam(i_p, BLK_PATH);
  GET_TEXT(IDC_EDIT_P2, t);
  i_p.name = T_PATH;
  mIni.createParam(i_p, BLK_PATH);
  GET_TEXT(IDC_EDIT_P3, t);
  i_p.name = L_PATH;
  mIni.createParam(i_p, BLK_PATH);

  mIni.createBlock(BLK_OFFS);
  GET_TEXT(IDC_EDIT_X, t);
  i_p.name = X_OFFS;

  mIni.createParam(i_p, BLK_OFFS);
  GET_TEXT(IDC_EDIT_Y, t);
  i_p.name = Y_OFFS;
  mIni.createParam(i_p, BLK_OFFS);

  int sel = SendMessage(GetDlgItem(*this, IDC_COMBO_POS), CB_GETCURSEL, 0, 0);
  if (CB_ERR != sel) {
    wsprintf(t, _T("%d"), sel);
    i_p.name = W_OFFS;
    mIni.createParam(i_p, BLK_OFFS);
    }

  mIni.createBlock(BLK_REDIM);
  t[0] = _T('0');
  if(IS_CHECKED(IDC_CHECK_REDIM_PHOTO))
    t[0] = _T('1');
  t[1] = 0;
  i_p.name = REDIM_PHOTO;
  mIni.createParam(i_p, BLK_REDIM);

  t[0] = _T('0');
  if (IS_CHECKED(IDC_CHECK_REDIM_LOGO))
    t[0] = _T('1');
  t[1] = 0;
  i_p.name = REDIM_LOGO;
  mIni.createParam(i_p, BLK_REDIM);

  GET_TEXT(IDC_EDIT_PERC_PHOTO, t);
  i_p.name = REDIM_PHOTO_PERC;
  mIni.createParam(i_p, BLK_REDIM);

  t[0] = _T('?');
  if (IS_CHECKED(IDC_RADIO_USE_PERC))
    t[0] = _T('0');
  else if (IS_CHECKED(IDC_RADIO_USE_FIX))
    t[0] = _T('1');
  t[1] = 0;
  i_p.name = REDIM_LOGO_HOW;
  mIni.createParam(i_p, BLK_REDIM);

  GET_TEXT(IDC_EDIT_PERC_LOGO, t);
  i_p.name = REDIM_LOGO_PERC;
  mIni.createParam(i_p, BLK_REDIM);

  GET_TEXT(IDC_EDIT_WIDTH_LOGO, t);
  i_p.name = REDIM_LOGO_WIDTH;
  mIni.createParam(i_p, BLK_REDIM);
#endif
  mIni.save();
}
//----------------------------------------------------------------------------
void PDShowVars::fillListImage()
{
  PVect<LPCTSTR> list;
  TCHAR path[_MAX_PATH];
  GET_TEXT(IDC_EDIT_P1, path);
  fillList(list, path);
  HWND hwcb = GetDlgItem(*this, IDC_COMBO_LIST);
  SendMessage(hwcb, CB_RESETCONTENT, 0, 0);
  for (uint i = 0; i < list.getElem(); ++i)
    SendMessage(hwcb, CB_ADDSTRING, 0, (LPARAM)list[i]);
}
//----------------------------------------------------------------------------
void PDShowVars::searchFile()
{
  TCHAR t[_MAX_PATH];
  GET_TEXT(IDC_EDIT_P3, t);
  if (openFileImg(*this, t)) {
    SET_TEXT(IDC_EDIT_P3, t);
    showDimLogo();
    }
}
//----------------------------------------------------------------------------
void PDShowVars::searchFolder(uint idc)
{
  TCHAR t[_MAX_PATH];
  GET_TEXT(idc, t);
  if (PChooseFolder(*this, t)) {
    SET_TEXT(idc, t);
    if(IDC_EDIT_P1 == idc)
      fillListImage();
    }
}
//----------------------------------------------------------------------------
static LPCTSTR MSG_SHOW_SEND_FILE = _T("[%d / %d] %s");
#define BARCOLOR_CHAR RGB(0x3f, 0xff, 0x10)
//----------------------------------------------------------------------------
void PDShowVars::forceRefreshList()
{
  HWND hcb = GetDlgItem(*this, IDC_COMBO_LIST);
  SendMessage(hcb, CB_SETCURSEL, 0, 0);
  enableDimPhotoByList();
}
//----------------------------------------------------------------------------
void PDShowVars::run()
{
  HWND hcb = GetDlgItem(*this, IDC_COMBO_POS);
  int sel = SendMessage(hcb, CB_GETCURSEL, 0, 0);
  if (CB_ERR == sel)
    return;
  saveIni();
  TCHAR s[_MAX_PATH];
  GET_TEXT(IDC_EDIT_P1, s);
  PVect<LPCTSTR> list;
  fillList(list, s);
  int num = list.getElem();
  int x = 10;
  int y = 10;
  GET_INT(IDC_EDIT_X, x);
  GET_INT(IDC_EDIT_Y, y);
  TCHAR t[_MAX_PATH];
  GET_TEXT(IDC_EDIT_P3, t);
  PBitmap slogo(t);
  GET_TEXT(IDC_EDIT_P2, t);
  createDirectoryRecurse(t);
  infoRedim ir = {0};

  if(IS_CHECKED(IDC_CHECK_REDIM_PHOTO)) {
    GET_INT(IDC_EDIT_WIDTH_PHOTO, ir.szImg.cx);
    GET_INT(IDC_EDIT_HEIGHT_PHOTO, ir.szImg.cy);
    if (!ir.szImg.cx || !ir.szImg.cy) {
      forceRefreshList();
      GET_INT(IDC_EDIT_WIDTH_PHOTO, ir.szImg.cx);
      GET_INT(IDC_EDIT_HEIGHT_PHOTO, ir.szImg.cy);
      }
    }
  if (IS_CHECKED(IDC_CHECK_REDIM_LOGO)) {
    if(IS_CHECKED(IDC_RADIO_USE_PERC))
      GET_INT(IDC_EDIT_PERC_LOGO, ir.resizeLogo);
    else {
      ir.resizeLogo = 1;
      GET_INT(IDC_EDIT_WIDTH_LOGO, ir.szLogo.cx);
      SIZE sz = slogo.getSize();
      ir.szLogo.cy = ROUND_REAL((double)ir.szLogo.cx * sz.cy / sz.cx);
      }
    }
  progressBar bar(this, MSG_SHOW_SEND_FILE, list.getElem(), BARCOLOR_CHAR);
  bar.create();
  UpdateWindow(bar);
  PAppl* pa = getAppl();
  if(!pa)
    pa = new PAppl(getHInstance(), 0);
  infoBuffer ib;
  int i;
  for(i = 0; i < num; ++i) {
    if(!bar.setCount(i + 1, list[i]))
      break;
    UpdateWindow(bar);
    perform(s, t, list[i], x, y, slogo, sel, ir, ib);
    pa->pumpMessages();
    delete []list[i];
    }
  if(i < num) {
    for(; i < num; ++i)
      delete []list[i];
    MessageBox(*this, _T("Annullato!"), _T("Aggiunta Logo a foto"), MB_OK | MB_ICONSTOP);
    }
  else
    MessageBox(*this, _T("Finito!"), _T("Aggiunta Logo a foto"), MB_OK | MB_ICONINFORMATION);
}
//------------------------------------------------------------------
