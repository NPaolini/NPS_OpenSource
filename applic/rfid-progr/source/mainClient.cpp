//------- mainClient.cpp -----------------------------------------------------
#include "precHeader.h"
#include "wm_custom.h"
#include <stdio.h>
#include "resource.h"
//#ifdef USE_RICH_TEXT
  #include <wtypes.h>
  #include <richedit.h>
//#endif
#include <commctrl.h>
//----------------------------------------------------------------------------
#include "macro_utils.h"
#include "mainClient.h"
#include "p_param.h"
#include "paramConn.h"

#include "p_manage_ini.h"
#include "pOwnBtnImageStd.h"
#include "connClass.h"
#include "p_matchStream.h"
//----------------------------------------------------------------------------
#define KEY_STRING_SETUP _T("Setup")
#define KEY_CODE _T("Code")
//----------------------------------------------------------------------------
class  myPOwnBtnImageStd : public POwnBtnImageStd
{
  private:
    typedef POwnBtnImageStd baseClass;

  public:
    myPOwnBtnImageStd(PWin * parent, uint id, int x, int y, int w, int h,
          PBitmap* image, uint fPos = wLeft, bool autoDelete = false,
          LPCTSTR text = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, x, y, w, h, image, fPos, autoDelete, text, hinst) {}

    myPOwnBtnImageStd(PWin * parent, uint id, const PRect& r, PBitmap* image,
          uint fPos = wLeft, bool autoDelete = false, LPCTSTR text = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, r, image, fPos, autoDelete, text, hinst) {}


    myPOwnBtnImageStd(PWin * parent, uint resid, PBitmap* image,
          uint fPos = wLeft, bool autoDelete = false, HINSTANCE hinst = 0):
        baseClass(parent, resid, image, fPos, autoDelete, hinst) {}

    void chgBmp(PBitmap* newbmp) { Bmp = newbmp; }
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool writeKey(LPCTSTR name, DWORD value);
//----------------------------------------------------------------------------
bool readKey(LPCTSTR name, DWORD& value, bool writeIfNotFound = true);
//----------------------------------------------------------------------------
bool writeKeyString(LPCTSTR name, LPCTSTR value);
bool readKeyString(LPCTSTR name, LPTSTR value, size_t lenBuff, bool writeIfNotFound);
//----------------------------------------------------------------------------
/* il codice è memorizzato:
    3B  -> header -> nps
    10B -> codice numerico -> 0000000000
    3B  -> versione -> 001
  per compatibilità future, non viene memorizzata la dimensione del codice letto poi dal marcatempo
  quindi qualunque codice si immetta, max 10 caratteri viene scritto nella card, sempre allineato a dx
*/
inline
bool writeKeyCode(LPCTSTR value)
{
  return writeKeyString(KEY_CODE, value);
}
//----------------------------------------------------------------------------
inline
bool readKeyCode(LPTSTR value, size_t lenBuff, bool writeIfNotFound = true)
{
  return readKeyString(KEY_CODE, value, lenBuff, writeIfNotFound);
}
//----------------------------------------------------------------------------
inline
bool writeKeySetup(DWORD value)
{
  return writeKey(KEY_STRING_SETUP, value);
}
//----------------------------------------------------------------------------
inline
bool readKeySetup(DWORD& value, bool writeIfNotFound = true)
{
  return readKey(KEY_STRING_SETUP, value, writeIfNotFound);
}
//----------------------------------------------------------------------------
paramConn getDefaultparamCom()
{
  paramConn pc;
  pc.b.rts_cts = paramConn::ON_RTS_CTS;
  pc.b.dtr_dsr = paramConn::ON_DTR_DSR;
  return pc;
}
//----------------------------------------------------------------------------
static paramConn getReadedParam()
{
  paramConn pc = getDefaultparamCom();
  DWORD dwParam = pc;
  readKeySetup(dwParam);
  return paramConn(dwParam);
}
//----------------------------------------------------------------------------
HINSTANCE hinstRichEdit;
//----------------------------------------------------------------------------
#define YELLOW_COLOR  RGB(0xff,0xff,0xcf)
#define bkgColor YELLOW_COLOR
//----------------------------------------------------------------------------
mainClient::mainClient(PWin* parent, uint resId,  HINSTANCE hInstance)
  : baseClass(parent, resId, hInstance), Com(0), Brush(CreateSolidBrush(bkgColor)),  currAddr(0),
      hwTips(0), dontDelete(false)
{

  Font = D_FONT(16, 0, 0, _T("courier"));

#define IX_BMP_OPENED 4
#define IX_BMP_CLOSED 2

#define MAX_BMP SIZE_A(idBmp)
  int idBmp[] = { IDB_BITMAP_CFG, IDB_BITMAP_DONE,
                  IDB_BITMAP_RED_SEM, IDB_BITMAP_CLEAR,
                  IDB_BITMAP_GREEN_SEM, IDB_RESET };
  Bmp.setDim(MAX_BMP);
  for(uint i = 0; i < MAX_BMP; ++i)
    Bmp[i] = new PBitmap(idBmp[i], getHInstance());


  new POwnBtnImageStd(this, IDC_BUTTON_SETUP, Bmp[0]);

  new myPOwnBtnImageStd(this, IDC_BUTTONOPEN_COM,  Bmp[2]);

  new POwnBtnImageStd(this, IDCANCEL,  Bmp[1]);

  new POwnBtnImageStd(this, IDC_BUTTON_CLEAR,  Bmp[3]);
  new POwnBtnImageStd(this, IDC_BUTTON_RESET,  Bmp[5]);


  hinstRichEdit = LoadLibrary(_T("RICHED32.DLL"));
}
//----------------------------------------------------------------------------
mainClient::~mainClient()
{
  if(hwTips)
    DestroyWindow(hwTips);
  destroy();
  DeleteObject(Font);
  DeleteObject(HGDIOBJ(Brush));
  delete Com;

  if(hinstRichEdit) {
    FreeLibrary(hinstRichEdit);
    hinstRichEdit = 0;
    }
  int nElem = Bmp.getElem();
  for(int i = 0; i < nElem; ++i)
    delete Bmp[i];
}
//----------------------------------------------------------------------------
#define GET_CT(a) ((a) & 3)
#define GET_MT(a) (((a) >> 2) & 1)
#define GET_LT(a) (((a) >> 3) & 1)
#define GET_PRT(a) (((a) >> 4) & 1)
#define GET_485(a) (((a) >> 5) & 1)
//----------------------------------------------------------------------------
#define SET_T(c, m, l, p, _485) ((c) | ((m) << 2) | ((l) << 3) | ((p) << 4) | ((_485) << 5))
//----------------------------------------------------------------------------
bool mainClient::create()
{
  if(!baseClass::create()) {
    return false;
    }
  HWND hwnd = GetDlgItem(*this, IDC_EDIT_REC);
  SendMessage(hwnd, EM_SETTEXTMODE, TM_PLAINTEXT, 0);
  SendMessage(hwnd, EM_SETBKGNDCOLOR, 0, bkgColor);
  SendMessage(hwnd, WM_SETFONT, (WPARAM)Font, true);
  LONG style = GetWindowLong(hwnd, GWL_STYLE);
  SetWindowLong(hwnd, GWL_STYLE, (style & ~(WS_SIZEBOX)) | WS_BORDER);

  SendMessage(GetDlgItem(*this, IDC_EDIT_STRING), WM_SETFONT, (WPARAM)Font, true);


  struct idAndText {
    uint id;
    LPTSTR tips;
    };

  static const idAndText id_txt[] = {
    { IDC_BUTTON_SETUP, _T("Communication Setting") },
    { IDC_BUTTONOPEN_COM, _T("Open/Close communication") },
    { IDCANCEL,             _T("Exit program") },
    { IDC_BUTTON_CLEAR,     _T("Clear buffer of received") },
    { IDC_BUTTON_RESET, _T("Reset Master Card (the next one becames the new master)") },
    };

#ifndef TTS_BALLOON
 #define TTS_BALLOON 0x40
#endif
  hwTips = CreateWindow(TOOLTIPS_CLASS, 0, TTS_ALWAYSTIP | TTS_BALLOON,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        0, 0, getHInstance(), 0);

  if(hwTips) {
    TOOLINFO ti;
    for(uint i = 0; i < SIZE_A(id_txt); ++i) {
      ti.cbSize = sizeof(TOOLINFO);
      ti.uFlags = TTF_IDISHWND;
      ti.hwnd = *this;
      ti.hinst = getHInstance();
      ti.uId = (UINT_PTR)GetDlgItem(*this, id_txt[i].id);
      ti.lpszText = id_txt[i].tips;
      if(!SendMessage(hwTips, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti))
        break;
      }
    }
  checkEnable();
  return true;
}
//----------------------------------------------------------------------------
bool mainClient::preProcessMsg(MSG& msg)
{
  switch (msg.message) {
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
      if(hwTips) {
        MSG msg2 = msg;
        SendMessage(hwTips, TTM_RELAYEVENT, 0, (LPARAM) (LPMSG) &msg2);
        }
      break;
    }
  if(!toBool(IsDialogMessage(getHandle(), &msg)))
    return baseClass::preProcessMsg(msg);
  return true;
}
//----------------------------------------------------------------------------
LRESULT mainClient::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if(WM_CUSTOM_MSG_CONNECTION == message) {
    switch(LOWORD(wParam)) {
      case CM_MSG_HAS_CHAR:
          readComm(HIWORD(wParam));
        break;
      }
    return 0;
    }

  switch(message) {

    case WM_DESTROY:
      CloseApp(hwnd);
      break;

    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_CLEAR:
          clearReceived();
          break;
        case IDC_BUTTONOPEN_COM:
          openCom();
          break;
        case IDC_BUTTON_SETUP:
          setupCom();
          break;
        case IDC_BUTTON_RESET:
          sendReset();
          break;
        }
      break;
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLORSCROLLBAR:
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
//----------------------------------------------------------------------------
void mainClient::checkEnable()
{
  ENABLE(IDC_BUTTON_RESET, toBool(Com));
}
//----------------------------------------------------------------------------
void mainClient::sendReset()
{
  if(Com) {
    BYTE t[] = "####";
    Com->write_string(t, sizeof(t));
    }
}
//----------------------------------------------------------------------------
void mainClient::switchOpenBmp()
{
  HWND child = GetDlgItem(*this, IDC_BUTTONOPEN_COM);
  PWin* btn = PWin::getWindowPtr(child);
  LPCTSTR title = _T("&Open Comm");
  uint ix = IX_BMP_CLOSED;
  if(Com) {
    ix = IX_BMP_OPENED;
    title = _T("&Close Comm");
    }
  if(btn)
    btn->setCaption(title);
  else
    SetWindowText(child, title);

  myPOwnBtnImageStd *iBtn = dynamic_cast<myPOwnBtnImageStd*>(btn);
  if(iBtn) {
    iBtn->chgBmp(Bmp[ix]);
    InvalidateRect(child, 0, 0);
    }
  checkEnable();
}
//----------------------------------------------------------------------------
#define SET_TEXT(idc, txt) \
    SetDlgItemText(*this, idc, txt)
//----------------------------------------------------------------------------
#define SET_INT(idc, val) \
    SetDlgItemInt(*this, idc, (val), 1)
//----------------------------------------------------------------------------
#define GET_P_TEXT(idc, txt, len) \
    GetDlgItemText(*this, idc, txt, len)
//----------------------------------------------------------------------------
#define GET_TEXT(idc, txt) \
    GET_P_TEXT(idc, txt, SIZE_A(txt))
//----------------------------------------------------------------------------
bool mainClient::openCom()
{
  TCHAR title[512] = _T("RFID Progr");
  HWND child = GetDlgItem(*this, IDC_BUTTONOPEN_COM);
  PWin* btn = PWin::getWindowPtr(child);
  if(Com) {
    if(Com) {
      delete Com;
      Com = 0;
      }
    }
  else {
    paramConn pc = getReadedParam();
    Com = new PConnBase(pc, *this, false);
    if(!Com->open()) {
      MessageBox(*this, _T("Unable to open Comm"), _T("Error Comm"), MB_OK | MB_ICONSTOP);
      delete Com;
      Com = 0;
      return false;
      }
    else
      wsprintf(title + _tcslen(title), _T(" - Serial connected to Com%d"), pc.getPort() + 1);
    }
  switchOpenBmp();
  SetWindowText(*getParent(), title);
  return true;
}
//----------------------------------------------------------------------------
void mainClient::setupCom()
{
    // meglio chiudere
  if(Com)
    openCom();
  Setting.par = getReadedParam();
  TCHAR code[64] = _T("nps0000000000001");
  readKeyCode(code, SIZE_A(Setting.Code));
  _tcscpy_s(Setting.Code, code);
  if(IDOK == P_Param(Setting, this).modal()) {
    DWORD dw = Setting.par;
    writeKeySetup(dw);
    writeKeyCode(Setting.Code);
    }
}
//----------------------------------------------------------------------------
static LPTSTR flushNotCode(LPTSTR p)
{
  while(*p) {
    switch(*p) {
      case _T(';'):
      case _T(','):
        break;
      default:
        if(*p > _T(' '))
          return p;
      }
    ++p;
    }
  return 0;
}
//----------------------------------------------------------------------------
void mainClient::sendCode()
{
  int len = GET_TEXTLENGHT(IDC_EDIT_STRING);
  if(!len)
    return;
  LPTSTR buff = new TCHAR[len + 2];
  GetDlgItemText(*this, IDC_EDIT_STRING, buff, len + 1);
  BYTE code[64];
  for(uint i = 0; i < DIM_FULL_CODE; ++i)
    code[i] = (BYTE)Setting.Code[i];
  TCHAR t[64];
  LPTSTR p = buff;
  p = flushNotCode(p);
  uint i = 0;
  while(*p && _istdigit(*p))
    t[i++] = *p++;
  if(!i)
    return;
  t[i] = 0;
  for(uint j = 0; j < i; ++j)
    code[OFFS_VER - i + j] = (BYTE)t[j];
  code[DIM_FULL_CODE] = 0;
  Com->write_string(code, DIM_FULL_CODE + 1);
  p = flushNotCode(p);
  SET_TEXT(IDC_EDIT_STRING, p);
  delete []buff;
}
//----------------------------------------------------------------------------
void mainClient::clearReceived()
{
HWND ctrl = GetDlgItem(*this, IDC_EDIT_REC);
  SendMessage(ctrl, WM_SETTEXT, 0, 0);
  currAddr = 0;
  InvalidateRect(ctrl, 0, 1);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void mainClient::readComm(int )
{
  if(!Com)
    return;
  static bool inExec;
  if(inExec)
    return;
  inExec = true;

  #define  SIZE_READ (512 * 2)
  BYTE buff[SIZE_READ];
  Com->reqNotify(false);
  HWND ctrl = GetDlgItem(*this, IDC_EDIT_REC);
  while(true) {
    if(!Com) {
      inExec = false;
      return;
      }
    int avail = Com->has_string();
    if(!avail)
      break;
    Sleep(100);
    avail = Com->has_string();
    avail = min(SIZE_READ -1, avail);
    Com->read_string(buff, avail);
    buff[avail] = 0;
    int currLen = SendMessage(ctrl, WM_GETTEXTLENGTH, 0, 0);
    SendMessage(ctrl, EM_SETSEL, currLen, currLen);
    performShowChar(ctrl, buff, avail);
    SendMessage(ctrl, EM_SCROLLCARET, 0, 0);
    getAppl()->pumpMessages();
    Sleep(0);
    }
  InvalidateRect(ctrl, 0, false);
  if(Com->has_string())
    PostMessage(*this, WM_CUSTOM_MSG_CONNECTION, MAKEWPARAM(CM_MSG_HAS_CHAR, 1), 0);
  else if(Com)
    Com->reqNotify(true);
  inExec = false;
}
//----------------------------------------------------------------------------
#define DIM_ADDR 6
#define DIM_BYTE_DATA 8
#define DIM_HEX_DATA (DIM_BYTE_DATA * 2)
#define DIM_CR_NL 2
#define DIM_SPACE 1
//----------------------------------------------------------------------------
#define LEFT_ROW \
    (DIM_ADDR + DIM_SPACE + \
     DIM_HEX_DATA + DIM_SPACE)

#define RIGHT_ROW  (DIM_BYTE_DATA + DIM_CR_NL)

#define LEN_ROW (LEFT_ROW + RIGHT_ROW)
//----------------------------------------------------------------------------
#define OFFS_HEX  (DIM_ADDR + DIM_SPACE)
#define OFFS_BYTE (DIM_ADDR + DIM_SPACE + DIM_HEX_DATA + DIM_SPACE)
#define END_BYTE (OFFS_BYTE + DIM_BYTE_DATA + DIM_CR_NL)
//----------------------------------------------------------------------------
static LPCTSTR MSG_SHOW_PERFORM = _T("Performing...\n\rbyte %d of %d");
#define BARCOLOR_CHAR RGB(0x3f, 0xff, 0x10)
#define BARCOLOR_HEX  RGB(0x3f, 0x10, 0xff)
#define BARCOLOR_FILE  RGB(0xff, 0x3f, 0x10)
#define MAX_COUNT 50
#define MAX_PERFORM_ONLINE (1024 * 32)
void mainClient::performShowChar(HWND ctrl, const BYTE* buff, int avail)
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
    if(!Com)
      break;
    }
  if(checkSendCode(pb, len))
    sendCode();
}
//----------------------------------------------------------------------------
HBRUSH mainClient::evCtlColor(HDC hdc, HWND /*hWndChild*/, UINT ctlType)
{
  if(WM_CTLCOLOREDIT == ctlType) {
    SetTextColor(hdc, RGB(0, 0, 0));
    SetBkColor(hdc, bkgColor);
    return (Brush);
    }
  return 0;
}
//----------------------------------------------------------------------------
void mainClient::CloseApp(HWND /*hwnd*/)
{
  PostQuitMessage(0);
}
//----------------------------------------------------------------------------
//-----------------------------------------------------------
#define INI_FILENAME _T("RFID-Progr.ini")
#define MAIN_PATH _T("\\Main")
//----------------------------------------------------------------------------
class myManageIni : public manageIni
{
  private:
    typedef manageIni baseClass;
  public:
    myManageIni(LPCTSTR filename);
};
//----------------------------------------------------------------------------
static void makeModulePath(LPTSTR target, size_t dim,  LPCTSTR filename)
{
  LPCTSTR pf = filename + _tcslen(filename) - 1;
  while(pf > filename) {
    if(_T('\\') == *pf || _T('/') == *pf) {
      ++pf;
      break;
      }
    --pf;
    }
  GetModuleDirName(dim, target);
  appendPath(target, pf);
}
//----------------------------------------------------------------------------
myManageIni::myManageIni(LPCTSTR filename) : baseClass(filename)
{
  TCHAR t[_MAX_PATH];
  makeModulePath(t, SIZE_A(t), filename);
  delete []Filename;
  Filename = str_newdup(t);
}
//----------------------------------------------------------------------------
bool writeKeyString(LPCTSTR keyName, LPCTSTR value)
{
  myManageIni ini(INI_FILENAME);
  ini.parse();
  ini_param param = { keyName, value };
  ini.addParam(param, MAIN_PATH);
  return ini.save();
}
//----------------------------------------------------------------------------
bool readKeyString(LPCTSTR keyName, LPTSTR value, size_t lenBuff, bool writeIfNotFound)
{
  value[0] = 0;
  myManageIni ini(INI_FILENAME);
  ini.parse();
  LPCTSTR str = ini.getValue(keyName, MAIN_PATH);
  if(str) {
    _tcscpy_s(value, lenBuff, str);
    return true;
    }
  if(writeIfNotFound)
    writeKeyString(keyName, value);
  return false;
}
//----------------------------------------------------------------------------
bool writeKey(LPCTSTR keyName, DWORD value)
{
  myManageIni ini(INI_FILENAME);
  ini.parse();
  TCHAR t[64];
  wsprintf(t, _T("%lu"), value);
  ini_param param = { keyName, t };
  ini.addParam(param, MAIN_PATH);
  return ini.save();
}
//----------------------------------------------------------------------------
bool readKey(LPCTSTR keyName, DWORD& value, bool writeIfNotFound)
{
  myManageIni ini(INI_FILENAME);
  ini.parse();
  LPCTSTR str = ini.getValue(keyName, MAIN_PATH);
  if(str) {
    __int64 t = _tcstoi64(str, 0, 10);
    value = (DWORD)t;
    return true;
    }
  if(writeIfNotFound)
    writeKey(keyName, value);
  return false;
}
//----------------------------------------------------------------------------
//-----------------------------------------------------------
#define MAX_DIM_CHECK 24
class consumer : public p_matchStream<BYTE, MAX_DIM_CHECK>
{
private:
  typedef p_matchStream<BYTE, MAX_DIM_CHECK> baseClass;
public:
  consumer(LPCBYTE buff, uint len) : Buff(buff), Len(len), curr(0) { }
protected:
  virtual bool has_data();
  virtual void read_one_data(BYTE& buff);
private:
  LPCBYTE Buff;
  uint Len;
  uint curr;
};
//-----------------------------------------------------------
bool consumer::has_data()
{
  return curr < Len;
}
//-----------------------------------------------------------
void consumer::read_one_data(BYTE& buff)
{
  buff = Buff[curr];
  ++curr;
}
//----------------------------------------------------------------------------
#define MATCH_PROGR (BYTE*)"byte):##"
#define DIM_MATCH (strlen((LPCSTR)MATCH_PROGR))

bool mainClient::checkSendCode(const BYTE* buff, int avail)
{
  consumer cons(buff, avail);
  return cons.checkMatch(MATCH_PROGR, DIM_MATCH) >= 0;
}

