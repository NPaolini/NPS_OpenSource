//----------- make_keydat.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <process.h>
//----------------------------------------------------------------------------
#include "macro_utils.h"
#include "make_keydat.h"
#include "base64.h"
#include "pOpensave.h"
#include "p_file.h"
#include "PMemo.h"
//----------------------------------------------------------------------------
make_keydat::make_keydat(PWin* parent, HINSTANCE hinstance) :
          baseClass(parent, IDD_CLIENT_SV, hinstance), Font(0)
{
}
//----------------------------------------------------------------------------
make_keydat::~make_keydat()
{
  destroy();
  if(Font)
    DeleteObject(Font);
}
//----------------------------------------------------------------------------
bool make_keydat::create()
{
  PMemo* ed1 = new PMemo(this, IDC_EDIT_ENCODE);
  PMemo* ed2 = new PMemo(this, IDC_EDIT_DECODE);
  if(!baseClass::create())
    return false;

  NONCLIENTMETRICSA ncm;
  ZeroMemory(&ncm, sizeof(ncm));
  uint sz = sizeof(ncm);// - sizeof(ncm.iPaddedBorderWidth);
  ncm.cbSize = sz;
  int height = 12;
  if (SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, sz, &ncm, 0))
    height = -ncm.lfMessageFont.lfHeight;
//  else
//    DisplayErrorString(GetLastError());
  Font = D_FONT(height, 0, 0, _T("courier"));
  ed1->setFont(Font);
  ed2->setFont(Font);

  HICON hicon = LoadIcon(getHInstance(), MAKEINTRESOURCE(IDI_ICON_NP));
  if(hicon) {
    SendMessage(*this, WM_SETICON, ICON_BIG, (LPARAM)hicon);
    SendMessage(*this, WM_SETICON, ICON_SMALL, (LPARAM)hicon);
    }
  SET_CHECK(IDC_RADIO_ENC_TXT);
  SET_CHECK(IDC_RADIO_DEC_TXT);
  SET_INT(IDC_EDIT_SPLIT, 72);
  enableDisable(false);
  enableDisable(true);
  return true;
}
//----------------------------------------------------------------------------
LRESULT make_keydat::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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
void make_keydat::enableDisable(bool dec)
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
void make_keydat::chooseFile(uint idc_edit)
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
void make_keydat::allocLoadBuff(infoBuff& ib, uint idc, bool byFile, bool need_trim)
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
      if(need_trim) {
        LPSTR t = new char[ib.dim + 1];
        pf.P_read(t, ib.dim);
        uint j = 0;
        for(uint i = 0; i < ib.dim; ++i)
          if((unsigned)t[i] > ' ')
            ib.buff[j++] = t[i];
        ib.dim = j;
        delete []t;
        }
      else
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
#define DIM_BLOCK 76
//----------------------------------------------------------------------------
void make_keydat::saveBuff(infoBuff& ib, uint idc, bool byFile, uint len_split)
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
      if(len_split) {
        uint nBlock = ib.dim / len_split;
        LPSTR pb = ib.buff;
        for(uint i = 0; i < nBlock; ++i, pb += len_split) {
          pf.P_write(pb, len_split);
          pf.P_write("\r\n", 2);
          }
        if(ib.dim % len_split)   {
          pf.P_write(pb, ib.dim % len_split);
          pf.P_write("\r\n", 2);
          }
        }
      else
        pf.P_write(ib.buff, ib.dim);
      MessageBox(*this, t, _T("File salvato"), MB_OK | MB_ICONINFORMATION);
      }
    }
  else {
    HWND hed = GetDlgItem(*this, idc);
    if(len_split) {
      uint nBlock = ib.dim / len_split;
      infoBuff ib2(ib.dim + nBlock * 2);
      LPSTR pb = ib.buff;
      LPSTR pb2 = ib2.buff;
#if 1
      split_encoded(pb2, pb, len_split);
#else
      for(uint i = 0; i < nBlock; ++i, pb += len_split) {
        CopyMemory(pb2, pb, len_split);
        pb2 += len_split;
        *pb2++ = '\r';
        *pb2++ = '\n';
        }
      if(ib.dim % len_split)   {
        CopyMemory(pb2, pb, ib.dim % len_split);
        pb2 += ib.dim % len_split;
        }
      *pb2 = 0;
#endif
      SetWindowTextA(hed, pb2);
      }
    else
      SetWindowTextA(hed, ib.buff);
  }
}
//----------------------------------------------------------------------------
#define NO_TRIM false
#define NO_SPLIT 0
#define YES_TRIM true
//----------------------------------------------------------------------------
void make_keydat::encode()
{
  infoBuff ibS;
  uint idcS = IS_CHECKED(IDC_RADIO_ENC_FILE) ? IDC_EDIT_ENC_FILE : IDC_EDIT_ENCODE;
  allocLoadBuff(ibS, idcS, IS_CHECKED(IDC_RADIO_ENC_FILE), NO_TRIM);

  infoBuff ibT((ibS.dim / 3 + 1) * 4 + 1);
  ibT.dim = base64_encode(ibT.buff, ibS.buff, ibS.dim);

  uint idcT = IS_CHECKED(IDC_RADIO_DEC_FILE) ? IDC_EDIT_DEC_FILE : IDC_EDIT_DECODE;
  uint len_split = 0;
  GET_INT(IDC_EDIT_SPLIT, len_split);
  saveBuff(ibT, idcT, IS_CHECKED(IDC_RADIO_DEC_FILE), len_split);
}
//----------------------------------------------------------------------------
void make_keydat::decode()
{
  infoBuff ibS;
  uint idcS = IS_CHECKED(IDC_RADIO_DEC_FILE) ? IDC_EDIT_DEC_FILE : IDC_EDIT_DECODE;
  allocLoadBuff(ibS, idcS, IS_CHECKED(IDC_RADIO_DEC_FILE), YES_TRIM);

  infoBuff ibT((ibS.dim / 4 + 1) * 3 + 1);
  ibT.dim = base64_decode(ibT.buff, ibS.buff, ibS.dim);

  uint idcT = IS_CHECKED(IDC_RADIO_ENC_FILE) ? IDC_EDIT_ENC_FILE : IDC_EDIT_ENCODE;
  saveBuff(ibT, idcT, IS_CHECKED(IDC_RADIO_ENC_FILE), NO_SPLIT);
}
//----------------------------------------------------------------------------
