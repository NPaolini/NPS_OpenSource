//---------- P_Log.cpp -----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "P_Log.h"
#include "pOwnBtnImageStd.h"
#include "macro_utils.h"
#include "base64.h"
#include "p_param.h"
#include "p_param_v.h"
#include <richedit.h>
//----------------------------------------------------------------------------
HINSTANCE hinstRichEdit;
//----------------------------------------------------------------------------
#define YELLOW_COLOR  RGB(0xff,0xff,0xcf)
#define bkgColor YELLOW_COLOR
//----------------------------------------------------------------------------
P_Log::P_Log(PConnBase* conn, PWin* parent, uint resId, HINSTANCE hinstance)
:
    baseClass(conn, parent, resId, hinstance)
{
  #define MAX_BMP SIZE_A(idBmp)
  int idBmp[] = { IDB_BITMAP_LOAD, IDB_BITMAP_CANC, IDB_BITMAP_DONE };
  Bmp.setDim(MAX_BMP);
  for(uint i = 0; i < MAX_BMP; ++i)
    Bmp[i] = new PBitmap(idBmp[i], getHInstance());

  new POwnBtnImageStd(this, IDC_BUTTON_LOAD, Bmp[0]);
  new POwnBtnImageStd(this, IDC_BUTTON_DELETE, Bmp[1]);
  new POwnBtnImageStd(this, IDCANCEL, Bmp[2]);
  hinstRichEdit = LoadLibrary(_T("RICHED32.DLL"));
}
//----------------------------------------------------------------------------
P_Log::~P_Log()
{
  if(hinstRichEdit) {
    FreeLibrary(hinstRichEdit);
    hinstRichEdit = 0;
    }
  int nElem = Bmp.getElem();
  for(int i = 0; i < nElem; ++i)
    delete Bmp[i];
}
//----------------------------------------------------------------------------
//static enum eFile { eL_Email = 1, eL_Access, eL_Alarm };
//----------------------------------------------------------------------------
int P_Log::getCurrSel()
{
  if(IS_CHECKED(IDC_RADIO_EMAIL))
    return 1;
  if(IS_CHECKED(IDC_RADIO_ACCESS))
    return 2;
  if(IS_CHECKED(IDC_RADIO_ALARM))
    return 3;
  return 0;
}
//----------------------------------------------------------------------------
static bool confirmDelete(PWin* w)
{
  return IDYES == MessageBox(*w, _T("Do you confirm deleting the log file?"), _T("Log management"), MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING);
}
//----------------------------------------------------------------------------
void P_Log::loadLog()
{
  clearReceived();
  char buff[256] = "";
  int ix = getCurrSel();
  if(!ix)
    return;
  wsprintfA(buff, "cmd=showlog:%d#", ix);
  connSend cs(Conn);
  cs.send(buff, strlen(buff));
}
//----------------------------------------------------------------------------
void P_Log::delLog()
{
  if(!confirmDelete(this))
    return;
  clearReceived();
  char buff[256] = "";
  int ix = getCurrSel();
  if(!ix)
    return;
  wsprintfA(buff, "cmd=remlog:%d#", ix);
  connSend cs(Conn);
  cs.send(buff, strlen(buff));
}
//----------------------------------------------------------------------------
bool P_Log::create()
{
  if(!baseClass::create())
    return false;
  Conn->passThrough(cReset, 0);
  HWND hwnd = GetDlgItem(*this, IDC_EDIT_REC);
  SendMessage(hwnd, EM_SETTEXTMODE, TM_PLAINTEXT, 0);
  LONG style = GetWindowLong(hwnd, GWL_STYLE);
  SetWindowLong(hwnd, GWL_STYLE, (style & ~(WS_SIZEBOX)) | WS_BORDER);
  SET_CHECK(IDC_RADIO_EMAIL);
  return true;
}
//----------------------------------------------------------------------------
LRESULT P_Log::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDCANCEL:
          EndDialog(hwnd, LOWORD(wParam));
          return 0;
        case IDC_BUTTON_LOAD:
          loadLog();
          break;
        case IDC_BUTTON_DELETE:
          delLog();
          break;
        }
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void P_Log::clearReceived()
{
  HWND ctrl = GetDlgItem(*this, IDC_EDIT_REC);
  SendMessage(ctrl, WM_SETTEXT, 0, 0);
  InvalidateRect(ctrl, 0, 1);
}
//----------------------------------------------------------------------------
void P_Log::readConn(WORD )
{
  static bool inExec;
  if(inExec)
    return;
  inExec = true;

  #define  SIZE_READ (512 * 2)
  BYTE buff[SIZE_READ];
  Conn->reqNotify(false);
  HWND ctrl = GetDlgItem(*this, IDC_EDIT_REC);
  while(true) {
    if(!Conn)
      break;
    int avail = Conn->has_string();
    if(!avail)
      break;
    Sleep(100);
    avail = Conn->has_string();
    avail = min(SIZE_READ -1, avail);
    Conn->read_string(buff, avail);
    buff[avail] = 0;
    int currLen = SendMessage(ctrl, WM_GETTEXTLENGTH, 0, 0);
    SendMessage(ctrl, EM_SETSEL, currLen, currLen);
    performShowChar(ctrl, buff, avail);
    SendMessage(ctrl, EM_SCROLLCARET, 0, 0);
    getAppl()->pumpMessages();
    Sleep(0);
    }
  InvalidateRect(ctrl, 0, false);
  if(Conn) {
    if(Conn->has_string())
      PostMessage(*this, WM_CUSTOM_MSG_CONNECTION, MAKEWPARAM(CM_MSG_HAS_CHAR, 1), 0);
    else
      Conn->reqNotify(true);
    }
  inExec = false;
}
//----------------------------------------------------------------------------
#define MAX_COUNT 50
//----------------------------------------------------------------------------
void P_Log::performShowChar(HWND ctrl, const BYTE* buff, int avail)
{
  const BYTE* base = buff;
  DWORD count = 0;
  CHARRANGE range = { 0x7ffffff, 0x7ffffff };
  LPCBYTE pb = buff;

  int len = strlen((LPCSTR)buff);
  while(true) {
    SendMessageA(ctrl, EM_REPLACESEL, 0, (LPARAM)buff);
    avail -= len + 1;
    getAppl()->pumpMessages();
    if(avail <= 0)
      break;
    SendMessage(ctrl, EM_EXSETSEL, 0, (LPARAM)&range);
    SendMessage(ctrl, EM_REPLACESEL, 0, (LPARAM)_T("."));
    SendMessage(ctrl, EM_EXSETSEL, 0, (LPARAM)&range);
    buff += len + 1;
    if(!len && ++count < MAX_COUNT)
      continue;
    count = 0;
    getAppl()->pumpMessages();
    }
}
//----------------------------------------------------------------------------
