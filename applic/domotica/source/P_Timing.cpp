//---------- P_Timing.cpp -----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "P_Timing.h"
#include "pOwnBtnImageStd.h"
#include "macro_utils.h"
#include "base64.h"
#include "p_param.h"
#include "p_param_v.h"
//----------------------------------------------------------------------------
P_Timing::P_Timing(PConnBase* conn, PWin* parent, uint resId, HINSTANCE hinstance)
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
P_Timing::~P_Timing()
{
  int nElem = Bmp.getElem();
  for(int i = 0; i < nElem; ++i)
    delete Bmp[i];
}
//----------------------------------------------------------------------------
void appendText(HWND owner, LPSTR buff, size_t dim, UINT idc, LPCSTR prefx)
{
  char tmp[256];
  GetDlgItemTextA(owner, idc, tmp, SIZE_A(tmp));
  if(prefx)
    strcat_s(buff, dim, prefx);
  strcat_s(buff, dim, tmp);
}
//----------------------------------------------------------------------------
void P_Timing::loadCurrTiming()
{
  char buff[4096] = "cmd=gettiming:#";
  connSend cs(Conn);
  cs.send(buff, strlen(buff));
  DWORD readed = storeUntil(Conn, '#', buff, SIZE_A(buff));
  buff[readed] = 0;
  parseTimingAlarm(buff);
}
//----------------------------------------------------------------------------
void P_Timing::loadCurrTime()
{
  char buff[2048] = "cmd=getdate:";
  connSend cs(Conn);
  cs.send(buff, strlen(buff));
  DWORD readed = storeUntil(Conn, '#', buff, SIZE_A(buff));
  buff[readed] = 0;
  parseDatetime(buff);
}
//----------------------------------------------------------------------------
void P_Timing::syncTime()
{
  SYSTEMTIME st;
  GetLocalTime(&st);
  char buff[2048] = "cmd=date:";
  int dow = st.wDayOfWeek;
  if(!dow)
    dow = 7;
  wsprintfA(buff + strlen(buff), "%d,%d,%d,%d#cmd=time:%d,%d,%d#",
    dow, st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond);
  connSend cs(Conn);
  cs.send(buff, strlen(buff));
  loadCurrTime();
}
//----------------------------------------------------------------------------
void P_Timing::BNClickedOk()
{
  char buff[2048] = "cmd=timing:";
  UINT idc[] = { IDC_EDIT_AL_TIME_1, IDC_EDIT_AL_TIME_2, IDC_EDIT_AL_TIME_3, IDC_EDIT_AL_TIME_4, IDC_EDIT_ENDRUN_TIME };
  for(uint i = 0; i < SIZE_A(idc); ++i)
    appendText(*this, buff, SIZE_A(buff), idc[i], i ? "," : "");
  strcat_s(buff, "#");

  connSend cs(Conn);
  cs.send(buff, strlen(buff));
  loadCurrTiming();
}
//----------------------------------------------------------------------------
bool P_Timing::create()
{
  if(!baseClass::create())
    return false;
  Conn->passThrough(cReset, 0);
  loadCurrTiming();
  loadCurrTime();
  return true;
}
//----------------------------------------------------------------------------
LRESULT P_Timing::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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
        case IDC_BUTTON_CURR_TIME:
          loadCurrTime();
          break;
        case IDC_BUTTON_SYNC_TIME:
          syncTime();
          break;
        }
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define MATCH_TIMING "r=gettiming:"
#define MATCH_SYNC "r=getdate:"
#define DIM_MATCH_TIMING (strlen(MATCH_TIMING))
#define DIM_MATCH_SYNC (strlen(MATCH_SYNC))
//----------------------------------------------------------------------------
#define SET_TEXT_A(idc, txt) SetDlgItemTextA(*this, idc, txt)
//----------------------------------------------------------------------------
void P_Timing::parseDatetime(LPCSTR buff)
{
  int pos = checkPos(buff, MATCH_SYNC, DIM_MATCH_SYNC);
  if(pos < 0)
    return;
  char t[256];
  LPCSTR p = buff + pos;
  for(int i = 0; i < SIZE_A(t); ++i) {
    t[i] = p[i];
    if(!p[i])
      break;
    }
  SET_TEXT_A(IDC_EDIT_CURR_TIME, t);
}
//----------------------------------------------------------------------------
void P_Timing::parseTimingAlarm(LPCSTR buff)
{
  int pos = checkPos(buff, MATCH_TIMING, DIM_MATCH_TIMING);
  if(pos < 0)
    return;
  pvvCharA target;
  uint n = splitParamA(target, buff + pos);
  if(n > 5)
    return;
  SET_TEXT_A(IDC_EDIT_AL_TIME_1, &target[0]);
  SET_TEXT_A(IDC_EDIT_AL_TIME_2, &target[1]);
  SET_TEXT_A(IDC_EDIT_AL_TIME_3, &target[2]);
  SET_TEXT_A(IDC_EDIT_AL_TIME_4, &target[3]);
  SET_TEXT_A(IDC_EDIT_ENDRUN_TIME, &target[4]);
}
//----------------------------------------------------------------------------
