//----------- make_crypt.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <process.h>
//----------------------------------------------------------------------------
#include "macro_utils.h"
#include "make_crypt.h"
#include "pOpensave.h"
#include "p_file.h"
//----------------------------------------------------------------------------
make_crypt::make_crypt(PWin* parent, HINSTANCE hinstance) :
          baseClass(parent, IDD_CLIENT_SV, hinstance)
{
}
//----------------------------------------------------------------------------
make_crypt::~make_crypt()
{
  destroy();
}
//----------------------------------------------------------------------------
bool make_crypt::create()
{
  if(!baseClass::create())
    return false;
  HICON hicon = LoadIcon(getHInstance(), MAKEINTRESOURCE(IDI_ICON_NP));
  if(hicon) {
    SendMessage(*this, WM_SETICON, ICON_BIG, (LPARAM)hicon);
    SendMessage(*this, WM_SETICON, ICON_SMALL, (LPARAM)hicon);
    }
  SET_CHECK(IDC_RADIO_ENC_TXT);
  SET_CHECK(IDC_RADIO_DEC_TXT);
  enableDisable(false);
  enableDisable(true);
  return true;
}
//----------------------------------------------------------------------------
LRESULT make_crypt::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_ENCODE:
          encode();
          break;
        case IDC_DECODE:
          decode();
          break;
        case IDC_BUTTON_ENC_FILE:
          chooseFile(IDC_EDIT_ENC_FILE);
          break;
        case IDC_BUTTON_DEC_FILE:
          chooseFile(IDC_EDIT_DEC_FILE);
          break;
        case IDC_RADIO_ENC_TXT:
        case IDC_RADIO_ENC_FILE:
          enableDisable(false);
          break;
        case IDC_RADIO_DEC_TXT:
        case IDC_RADIO_DEC_FILE:
          enableDisable(true);
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void make_crypt::enableDisable(bool dec)
{
  uint idcEw[] = { IDC_EDIT_ENCODE, IDC_EDIT_DECODE };
  uint idcEf[] = { IDC_EDIT_ENC_FILE, IDC_EDIT_DEC_FILE };
  uint idcB[] = { IDC_BUTTON_ENC_FILE, IDC_BUTTON_DEC_FILE };

  uint idcCheck[] = { IDC_RADIO_ENC_TXT, IDC_RADIO_DEC_TXT };
  bool enable = IS_CHECKED(idcCheck[dec]);
  ENABLE(idcEw[dec], enable);
  ENABLE(idcEf[dec], !enable);
  ENABLE(idcB[dec], !enable);
}
//----------------------------------------------------------------------------
class myOpenSave : public POpenSave
{
  public:
    myOpenSave(HWND owner) : POpenSave(owner) {  }
    ~myOpenSave() {  }
  protected:
    static TCHAR Path[_MAX_PATH];
    virtual void setPathFromHistory();
    virtual void savePathToHistory();
};
//----------------------------------------------------------------------------
TCHAR myOpenSave::Path[_MAX_PATH] = _T(".\\");
//----------------------------------------------------------------------------
void myOpenSave::setPathFromHistory()
{
  SetCurrentDirectory(Path);
}
//----------------------------------------------------------------------------
void myOpenSave::savePathToHistory()
{
  GetCurrentDirectory(_MAX_PATH, Path);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
LPCTSTR filterOpenExt[] = { _T(".*"), 0 };
//----------------------------------------------------------------------------
LPCTSTR filterOpen =
  _T("All files (*.*)\0")
  _T("*.*\0");
//----------------------------------------------------------------------------
bool openFile(HWND owner, LPTSTR file)
{
  static DWORD lastIx;
  infoOpenSave Info(filterOpenExt, filterOpen, infoOpenSave::OPEN_NO_EXIST, lastIx);
  myOpenSave open(owner);

  if(open.run(Info)) {
    lastIx = Info.ixFilter;
    _tcscpy_s(file, _MAX_PATH, open.getFile());
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
void make_crypt::chooseFile(uint idc_edit)
{
  static TCHAR path[_MAX_PATH];
  TCHAR t[_MAX_PATH];
  GetDlgItemText(*this, idc_edit, t, SIZE_A(t));
  if(*t)
    _tcscpy_s(path, t);
  if(openFile(*this, path))
    SetDlgItemText(*this, idc_edit, path);
}
//----------------------------------------------------------------------------
void make_crypt::allocLoadBuff(infoBuff& ib, uint idc, bool byFile)
{
  if(byFile) {
    TCHAR t[_MAX_PATH];
    GET_TEXT(idc, t);
    lTrim(trim(t));
    if(!*t) {
      chooseFile(idc);
      GET_TEXT(idc, t);
      lTrim(trim(t));
      if(!*t)
        return;
      }
    P_File pf(t, P_READ_ONLY);
    if(pf.P_open()) {
      ib.dim = (uint)pf.get_len();
      ib.buff = new char[ib.dim + 1];
      pf.P_read(ib.buff, ib.dim);
      ib.buff[ib.dim] = 0;
      }
    }
  else {
    HWND hed = GetDlgItem(*this, idc);
    ib.dim = GetWindowTextLength(hed);
    ib.buff = new char[ib.dim + 2];
    GetWindowTextA(hed, ib.buff, ib.dim + 1);
    }
}
//----------------------------------------------------------------------------
void make_crypt::saveBuff(infoBuff& ib, uint idc, bool byFile)
{
  if(byFile) {
    TCHAR t[_MAX_PATH];
    GET_TEXT(idc, t);
    lTrim(trim(t));
    if(!*t) {
      chooseFile(idc);
      GET_TEXT(idc, t);
      lTrim(trim(t));
      if(!*t)
        return;
      }
    P_File pf(t, P_CREAT);
    if(pf.P_open()) {
      pf.P_write(ib.buff, ib.dim);
      MessageBox(*this, t, _T("File salvato"), MB_OK | MB_ICONINFORMATION);
      }
    }
  else {
    HWND hed = GetDlgItem(*this, idc);
    SetWindowTextA(hed, ib.buff);
    }
}
//----------------------------------------------------------------------------
void make_crypt::common_code(infoBuff& ib)
{
  char key[256];
  GetWindowTextA(GetDlgItem(*this, IDC_EDIT_KEY), key, sizeof(key));
  if(!key[0]) {
    key[0] = '?';
    key[1] = 0;
    SetWindowText(GetDlgItem(*this, IDC_EDIT_KEY), _T("?"));
    }
  int step = 0;
  GET_INT(IDC_EDIT_KEY_STEP, step);
  if(step <= 0) {
    step = 1;
    SET_INT(IDC_EDIT_KEY_STEP, 1);
    }
  cryptBuffer((LPBYTE)ib.buff, ib.dim, (LPCBYTE)key, strlen(key), step);
}
//----------------------------------------------------------------------------
void make_crypt::encode()
{
  infoBuff ib;
  uint idcS = IS_CHECKED(IDC_RADIO_ENC_FILE) ? IDC_EDIT_ENC_FILE : IDC_EDIT_ENCODE;
  allocLoadBuff(ib, idcS, IS_CHECKED(IDC_RADIO_ENC_FILE));
  common_code(ib);

  uint idcT = IS_CHECKED(IDC_RADIO_DEC_FILE) ? IDC_EDIT_DEC_FILE : IDC_EDIT_DECODE;
  saveBuff(ib, idcT, IS_CHECKED(IDC_RADIO_DEC_FILE));
}
//----------------------------------------------------------------------------
void make_crypt::decode()
{
  infoBuff ib;
  uint idcS = IS_CHECKED(IDC_RADIO_DEC_FILE) ? IDC_EDIT_DEC_FILE : IDC_EDIT_DECODE;
  allocLoadBuff(ib, idcS, IS_CHECKED(IDC_RADIO_DEC_FILE));
  common_code(ib);

  uint idcT = IS_CHECKED(IDC_RADIO_ENC_FILE) ? IDC_EDIT_ENC_FILE : IDC_EDIT_ENCODE;
  saveBuff(ib, idcT, IS_CHECKED(IDC_RADIO_ENC_FILE));
}
//----------------------------------------------------------------------------
