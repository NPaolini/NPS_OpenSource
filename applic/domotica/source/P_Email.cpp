//---------- P_Email.cpp -----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "P_Email.h"
#include "pOwnBtnImageStd.h"
#include "macro_utils.h"
#include "base64.h"
#include "p_param.h"
#include "p_param_v.h"
//----------------------------------------------------------------------------
P_Email::P_Email(PConnBase* conn, PWin* parent, uint resId, HINSTANCE hinstance)
:
    baseClass(conn, parent, resId, hinstance)
{
  #define MAX_BMP SIZE_A(idBmp)
  int idBmp[] = { IDB_BITMAP_SEND, IDB_BITMAP_DONE };
  Bmp.setDim(MAX_BMP);
  for(uint i = 0; i < MAX_BMP; ++i)
    Bmp[i] = new PBitmap(idBmp[i], getHInstance());

  new POwnBtnImageStd(this, IDOK, Bmp[0]);
  new POwnBtnImageStd(this, IDCANCEL, Bmp[1]);
}
//----------------------------------------------------------------------------
P_Email::~P_Email()
{
  int nElem = Bmp.getElem();
  for(int i = 0; i < nElem; ++i)
    delete Bmp[i];
}
//----------------------------------------------------------------------------
void P_Email::refreshBase64(UINT idc1, UINT idc2)
{
  char buff[256];
  char b64[256];
  GetDlgItemTextA(*this, idc1, buff, SIZE_A(buff));
  uint dim = base64_encode(b64, buff, strlen(buff));
  b64[dim] = 0;
  SetDlgItemTextA(*this, idc2, b64);
}
//----------------------------------------------------------------------------
void P_Email::loadCurr()
{
  char buff[4096] = "cmd=getemail:#";
  connSend cs(Conn);
  cs.send(buff, strlen(buff));
  DWORD readed = storeUntil(Conn, '#', buff, SIZE_A(buff));
  buff[readed] = 0;
  parseBuff(buff);
}
//----------------------------------------------------------------------------
void P_Email::appendText(LPSTR buff, size_t dim, LPCSTR l, UINT idc, LPCSTR prefx)
{
  char t[265];
  GetDlgItemTextA(*this, idc, t, SIZE_A(t));
  if(prefx)
    strcat_s(buff, dim, prefx);
  strcat_s(buff, dim, l);
  strcat_s(buff, dim, ",");
  strcat_s(buff, dim, t);
}
//----------------------------------------------------------------------------
void P_Email::BNClickedOk()
{
  char buff[4096] = "cmd=email:";

  struct infoE { LPCSTR l; UINT idc; };
  infoE ie[] = {
    { "s", IDC_EDIT_SMTP },
    { "p", IDC_EDIT_EMAIL_PORT },
    { "f", IDC_EDIT_EMAIL_FROM },
    { "t", IDC_EDIT_EMAIL_TO },
    { "u", IDC_EDIT_EMAIL_USER_64 },
    { "w", IDC_EDIT_EMAIL_PSW_64 }
    };

  for(uint i = 0; i < SIZE_A(ie); ++i)
    appendText(buff, SIZE_A(buff), ie[i].l, ie[i].idc, i ? "," : "");
  strcat_s(buff, "#");

  connSend cs(Conn);
  cs.send(buff, strlen(buff));
  Sleep(2000);
  loadCurr();
}
//----------------------------------------------------------------------------
bool P_Email::create()
{
  if(!baseClass::create())
    return false;
  Conn->passThrough(cReset, 0);
  loadCurr();
  return true;
}
//----------------------------------------------------------------------------
LRESULT P_Email::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDOK:
          BNClickedOk();
          break;
        case IDCANCEL:
          EndDialog(hwnd, LOWORD(wParam));
          return 0;
        case IDC_EDIT_EMAIL_USER:
          if(EN_CHANGE == HIWORD(wParam))
            refreshBase64(IDC_EDIT_EMAIL_USER, IDC_EDIT_EMAIL_USER_64);
          break;
        case IDC_EDIT_EMAIL_PSW:
          if(EN_CHANGE == HIWORD(wParam))
            refreshBase64(IDC_EDIT_EMAIL_PSW, IDC_EDIT_EMAIL_PSW_64);
          break;
        }
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define MATCH_EMAIL "r=getemail:"
#define DIM_MATCH_EMAIL (strlen(MATCH_EMAIL))
//----------------------------------------------------------------------------
#define SET_TEXT_A(idc, txt) SetDlgItemTextA(*this, idc, txt)
//----------------------------------------------------------------------------
void P_Email::setText_and_64(LPCSTR v, UINT idc1, UINT idc2)
{
  SET_TEXT_A(idc1, v);
  char t[256];
  uint l = base64_decode(t, v, strlen(v));
  t[l] = 0;
  SET_TEXT_A(idc2, t);
}
//----------------------------------------------------------------------------
void P_Email::parsePair(LPCSTR l, LPCSTR v)
{
  switch(*l) {
    case 's':
      SET_TEXT_A(IDC_EDIT_SMTP, v);
      break;
    case 'p':
      SET_TEXT_A(IDC_EDIT_EMAIL_PORT, v);
      break;
    case 'f':
      SET_TEXT_A(IDC_EDIT_EMAIL_FROM, v);
      break;
    case 't':
      SET_TEXT_A(IDC_EDIT_EMAIL_TO, v);
      break;
    case 'u':
      setText_and_64(v, IDC_EDIT_EMAIL_USER_64, IDC_EDIT_EMAIL_USER);
      break;
    case 'w':
      setText_and_64(v, IDC_EDIT_EMAIL_PSW_64, IDC_EDIT_EMAIL_PSW);
      break;
    }
}
//----------------------------------------------------------------------------
void P_Email::parseBuff(LPCSTR buff)
{
  int pos = checkPos(buff, MATCH_EMAIL, DIM_MATCH_EMAIL);
  if(pos < 0)
    return;
  pvvCharA target;
  uint n = splitParamA(target, buff + pos);
  if(n != 12)
    return;
  for(uint i = 0; i < n; i += 2)
    parsePair(&target[i], &target[i + 1]);
}
//----------------------------------------------------------------------------
