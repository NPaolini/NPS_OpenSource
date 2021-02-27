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

#include "pFilter.h"
#include "p_manage_ini.h"
#include "pOpensave.h"
#include "p_file.h"
#include "pOwnBtnImageStd.h"
#include "progressBar.h"
#include "connClass.h"
#include "P_Status.h"

//#define SUB_BASE_KEY _T("Software\\NPS_Programs\\sendComm")
//----------------------------------------------------------------------------
#define KEY_STRING_SETUP _T("Setup")
#define OLD_PATH _T("OldPath")
#define DELAY_SEND _T("DelayOnSend")
#define PACKET_SEND _T("PacketDim")
#define INIT_MODEN_STRING _T("InitModem")
#define CONN_TYPE _T("ConnType")
#define OLD_MODEM _T("OldModem")
#define OLD_LISTEN _T("OldListen")
#define OLD_CONNECT _T("OldConnect")
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
inline
bool writeKeyDelay(DWORD value)
{
  return writeKey(DELAY_SEND, value);
}
//----------------------------------------------------------------------------
inline
bool readKeyDelay(DWORD& value, bool writeIfNotFound = true)
{
  return readKey(DELAY_SEND, value, writeIfNotFound);
}
//----------------------------------------------------------------------------
inline
bool writeKeyPacket(DWORD value)
{
  return writeKey(PACKET_SEND, value);
}
//----------------------------------------------------------------------------
inline
bool readKeyPacket(DWORD& value, bool writeIfNotFound = true)
{
  return readKey(PACKET_SEND, value, writeIfNotFound);
}
//----------------------------------------------------------------------------
inline
bool readKeyConnType(DWORD& value, bool writeIfNotFound = true)
{
  return readKey(CONN_TYPE, value, writeIfNotFound);
}
//----------------------------------------------------------------------------
inline
bool writeKeyConnType(DWORD value)
{
  return writeKey(CONN_TYPE, value);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
inline
bool readKeyInitModem(LPTSTR value, size_t lenBuff, bool writeIfNotFound = true)
{
  return readKeyString(INIT_MODEN_STRING, value, lenBuff, writeIfNotFound);
}
//----------------------------------------------------------------------------
inline
bool writeKeyInitModem(LPCTSTR value)
{
  return writeKeyString(INIT_MODEN_STRING, value);
}
//----------------------------------------------------------------------------
inline
bool readKeyOldModem(LPTSTR value, size_t lenBuff, bool writeIfNotFound = true)
{
  return readKeyString(OLD_MODEM, value, lenBuff, writeIfNotFound);
}
//----------------------------------------------------------------------------
inline
bool writeKeyOldModem(LPCTSTR value)
{
  return writeKeyString(OLD_MODEM, value);
}
//----------------------------------------------------------------------------
inline
bool readKeyListen(LPTSTR value, size_t lenBuff, bool writeIfNotFound = true)
{
  return readKeyString(OLD_LISTEN, value, lenBuff, writeIfNotFound);
}
//----------------------------------------------------------------------------
inline
bool writeKeyListen(LPCTSTR value)
{
  return writeKeyString(OLD_LISTEN, value);
}
//----------------------------------------------------------------------------
inline
bool readKeyConnect(LPTSTR value, size_t lenBuff, bool writeIfNotFound = true)
{
  return readKeyString(OLD_CONNECT, value, lenBuff, writeIfNotFound);
}
//----------------------------------------------------------------------------
inline
bool writeKeyConnect(LPCTSTR value)
{
  return writeKeyString(OLD_CONNECT, value);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define PEDIT PEditSend
#define PEDIT2 PEditSel
//----------------------------------------------------------------------------
#define WMC_SEND_EDIT (WM_APP + 99)
//----------------------------------------------------------------------------
class PEditSend : public PEdit
{
  public:
    PEditSend(PWin * parent, uint resid, int textlen = 255, HINSTANCE hinst = 0):
      PEdit(parent, resid, textlen, hinst) {}

  protected:
    virtual bool evChar(WPARAM& key);
  private:
    typedef PEdit baseClass;
};
//-----------------------------------------------------------
bool PEditSend::evChar(WPARAM& key)
{
  if(VK_RETURN == key) {
    PostMessage(*getParent(), WMC_SEND_EDIT, Attr.id, 0);
    shift_click(VK_TAB);
    return true;
    }
  return baseClass::evKeyUp(key);
}
//----------------------------------------------------------------------------
class PEditSel : public PEditSend
{
  public:
    PEditSel(PWin * parent, uint resid, int textlen = 255, HINSTANCE hinst = 0):
      PEditSend(parent, resid, textlen, hinst) {}

  protected:
    virtual bool evChar(WPARAM& key);
  private:
    typedef PEditSend baseClass;
};
//----------------------------------------------------------------------------
bool PEditSel::evChar(WPARAM& key)
{
  SendMessage(*this, EM_SETSEL, 0, (LPARAM)0x7fff);
  return baseClass::evChar(key);
}
//----------------------------------------------------------------------------
paramConn getDefaultparamCom()
{
  paramConn pc;
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
  : baseClass(parent, resId, hInstance), Com(0),  pBuff(16384), Capture(0),
      Brush(CreateSolidBrush(bkgColor)),  showHex(false), currAddr(0),
      hwTips(0), ServerLan(0), dontDelete(false), InfoConn(0), ShowMsgBox(true)
{

  Font = D_FONT(16, 0, 0, _T("courier"));
  PEDIT* dec = new PEDIT(this, IDC_EDIT_DEC);
  dec->setFilter(new myNumbFilter);
  dec->setFont(Font);

  PEDIT* hex = new PEDIT(this, IDC_EDIT_HEX);
  hex->setFilter(new myHexFilter);
  hex->setFont(Font);

  PEDIT* bin = new PEDIT(this, IDC_EDIT_BIN);
  bin->setFilter(new myBinaryFilter);
  bin->setFont(Font);

  bin = new PEDIT2(this, IDC_EDIT_CHAR, 1);
  bin->setFont(Font);

#define IX_BMP_OPENED 6
#define IX_BMP_CLOSED 3

#define MAX_BMP SIZE_A(idBmp)
  int idBmp[] = { IDB_BITMAP_CFG, IDB_BITMAP_SEND, IDB_BITMAP_DONE,
                  IDB_BITMAP_RED_SEM, IDB_BITMAP_CLEAR, IDB_BITMAP_SAVE,
                  IDB_BITMAP_GREEN_SEM };
  Bmp.setDim(MAX_BMP);
  for(uint i = 0; i < MAX_BMP; ++i)
    Bmp[i] = new PBitmap(idBmp[i], getHInstance());


  new POwnBtnImageStd(this, IDC_BUTTON_SETUP, Bmp[0]);

  new myPOwnBtnImageStd(this, IDC_BUTTONOPEN_COM,  Bmp[3]);

  new POwnBtnImageStd(this, IDC_BUTTON_SEND_STRING,  Bmp[1]);
  new POwnBtnImageStd(this, IDC_BUTTON_SEND_BYTE,  Bmp[1]);
  new POwnBtnImageStd(this, IDC_BUTTON_SEND_FILE,  Bmp[1]);

  new POwnBtnImageStd(this, IDCANCEL,  Bmp[2]);

  new POwnBtnImageStd(this, IDC_BUTTON_CLEAR,  Bmp[4]);

  new POwnBtnImageStd(this, IDC_BUTTON_SAVE,  Bmp[5]);

  hinstRichEdit = LoadLibrary(_T("RICHED32.DLL"));
}
//----------------------------------------------------------------------------
mainClient::~mainClient()
{
  safeDeleteP(InfoConn);
  delete Capture;
  if(hwTips)
    DestroyWindow(hwTips);
  destroy();
  DeleteObject(Font);
  DeleteObject(HGDIOBJ(Brush));
  delete Com;
  delete ServerLan;

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
  readKeyDelay(Setting.delay);
  readKeyPacket(Setting.packet);
  DWORD ct = 0;
  readKeyConnType(ct);
  Setting.connType = GET_CT(ct);
  Setting.modemType = GET_MT(ct);
  Setting.lanType = GET_LT(ct);
  Setting.proto = GET_PRT(ct);
  Setting.on485 = GET_485(ct);

  readKeyInitModem(Setting.initModem, SIZE_A(Setting.initModem));


  struct idAndText {
    uint id;
    LPTSTR tips;
    };

  static const idAndText id_txt[] = {
    { IDC_BUTTON_SETUP, _T("Communication Setting") },
    { IDC_BUTTONOPEN_COM, _T("Open/Close communication") },
    { IDC_BUTTON_SEND_STRING, _T("Send content of Window Text") },
    { IDC_BUTTON_SEND_BYTE, _T("Send single byte") },
    { IDC_BUTTON_SEND_FILE, _T("Send the specified file") },
    { IDCANCEL,             _T("Exit program") },
    { IDC_BUTTON_CLEAR,     _T("Clear buffer of received") },
    { IDC_BUTTON_SAVE,        _T("Save buffer to file") },
    { IDC_CHECKBOX_SHOW_HEX, _T("Change the show in Hex Or String") },
    { IDC_CHECKBOX_CAPTURE_TO_FILE, _T("Capture all the receipt in a file") },
    { IDC_CHECKBOX_SHOW_COM, _T("Show additional info for serial connection") },
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
#define MSG_NEW_CLIENT _T("New client connected\r\ndo you want to remove old client?") \
                       _T("\r\n\r\nIf you choose No the new client will disconnect")
//----------------------------------------------------------------------------
void mainClient::addClient(HANDLE_CONN client)
{
  if(Com) {
    if(IDYES == MessageBox(*this, MSG_NEW_CLIENT, _T("One client already connected"), MB_YESNO | MB_ICONINFORMATION)) {
      dontDelete = true;
      delete Com;
      }
    else {
      ServerLan->remove(client);
      return;
      }
    }
  Com = new PConnBase(client);
  if(ShowMsgBox)
    MessageBox(*this, _T("Client Connected"), _T("Server Status"), MB_OK | MB_ICONINFORMATION);
}
//----------------------------------------------------------------------------
LRESULT mainClient::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if(WM_CUSTOM_MSG_CONNECTION == message) {
    switch(LOWORD(wParam)) {
      case CM_MSG_HAS_CHAR:
          readComm(HIWORD(wParam));
        break;

      case CM_BY_LAN_SERVER:
        if(HIWP_ADD_CLIENT == HIWORD(wParam)) {
          addClient((HANDLE_CONN)lParam);
          break;
          }

      case CM_BY_LAN_CLIENT:
      case CM_BY_MODEM:
      case CM_BY_SERIAL_DIRECT:
        showConn((DWORD)HIWORD(wParam), lParam);
        break;
      }
    return 0;
    }

  switch(message) {

    case WM_DESTROY:
      CloseApp(hwnd);
      break;


    case WMC_SEND_EDIT:
      sendString(IDC_EDIT_STRING == wParam ? STRING : VAL);
      break;

    case WM_COMMAND:
      switch(HIWORD(wParam)) {
        case EN_CHANGE:
          updateEdit(LOWORD(wParam));
          break;
        }
      switch(LOWORD(wParam)) {
        case IDC_CHECKBOX_CAPTURE_TO_FILE:
          setCaptureFile();
          break;

        case IDC_CHECKBOX_SHOW_COM:
          openInfoConn();
          break;
        case IDC_CHECKBOX_SHOW_HEX:
          changeShow();
          break;

        case IDC_BUTTON_SEND_FILE:
          sendFile();
          break;
        case IDC_BUTTON_SEND_STRING:
          sendString(STRING);
          break;
        case IDC_BUTTON_SEND_BYTE:
          sendString(VAL);
          break;
        case IDC_BUTTON_CLEAR:
          clearReceived();
          break;
        case IDC_BUTTON_SAVE:
          saveFile();
          break;
        case IDC_BUTTONOPEN_COM:
          openCom();
          break;
        case IDC_BUTTON_SETUP:
          setupCom();
          break;
        case IDC_CHECKBOX_NO_MSGBOX:
          ShowMsgBox = !IS_CHECKED(IDC_CHECKBOX_NO_MSGBOX);
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
void mainClient::openInfoConn()
{
  if(InfoConn) {
    safeDeleteP(InfoConn);
    return;
    }
  InfoConn = new P_Status(this, Com);
  if(!InfoConn->create())
    openInfoConn();
}
//----------------------------------------------------------------------------
void mainClient::switchOpenBmp()
{
  HWND child = GetDlgItem(*this, IDC_BUTTONOPEN_COM);
  PWin* btn = PWin::getWindowPtr(child);
  LPCTSTR title = _T("&Open Comm");
  uint ix = IX_BMP_CLOSED;
  if(Com || ServerLan) {
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
}
//----------------------------------------------------------------------------
#define MAX_NUMBER_LEN _MAX_PATH
class inputNumber : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    inputNumber(LPTSTR number, const setting& set, PWin* parent, uint resId = IDD_DIALOG_NUMBER, HINSTANCE hInstance = 0) :
      baseClass(parent, resId, hInstance), Number(number), Set(set) {}
    virtual ~inputNumber() { destroy(); }

    virtual bool create();
  protected:
    virtual void CmOk();
//    virtual void CmCancel() {}

  private:
    LPTSTR Number;
    const setting& Set;
};
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
bool inputNumber::create()
{
  if(!baseClass::create())
    return false;
  switch(Set.connType) {
    case 1:
      readKeyOldModem(Number, MAX_NUMBER_LEN);
      break;
    case 2:
      if(Set.lanType) {
        SET_TEXT(IDC_STATICTEXT_INFO_NUM, _T("Port to Listen (es. 3000)"));
        readKeyListen(Number, MAX_NUMBER_LEN);
        }
      else {
        SET_TEXT(IDC_STATICTEXT_INFO_NUM, _T("IP Addr,Port to Connect (es. 192.168.0.1,3000)"));
        readKeyConnect(Number, MAX_NUMBER_LEN);
        }
      break;
    }
  SET_TEXT(IDC_EDIT_NUMBER, Number);
  return true;
}
//----------------------------------------------------------------------------
void inputNumber::CmOk()
{
  GET_P_TEXT(IDC_EDIT_NUMBER, Number, MAX_NUMBER_LEN);

  switch(Set.connType) {
    case 1:
      writeKeyOldModem(Number);
      break;
    case 2:
      if(Set.lanType)
        writeKeyListen(Number);
      else
        writeKeyConnect(Number);
      break;
    }
  baseClass::CmOk();
}
//----------------------------------------------------------------------------
bool mainClient::getTelephoneNumber(LPTSTR number)
{
  return IDOK == inputNumber(number, Setting, this).modal();
}
//----------------------------------------------------------------------------
#if 0
void getAddrAndPort(DWORD& addr, DWORD& port, LPCTSTR address)
{
  int p1;
  int p2;
  int p3;
  int p4;
  _stscanf_s(address, _T("%d.%d.%d.%d,%d"), &p1, &p2, &p3, &p4, &port);
  addr = p1 | (p2 << 8) | (p3 << 16) | (p4 << 24);
}
#endif
//----------------------------------------------------------------------------
LPCTSTR getAddrAndPort(DWORD& port, LPTSTR address)
{
  port = 0;
  int len = _tcslen(address);
  for(int i = len - 1; i > 0; --i) {
    if(_T(',') == address[i] || _T(':') == address[i]) {
      address[i] = 0;
      port = _ttoi(address + i + 1);
      break;
      }
    }
  return address;
}
//----------------------------------------------------------------------------
bool mainClient::openCom()
{
  TCHAR title[512] = _T("Test Conn");
  HWND child = GetDlgItem(*this, IDC_BUTTONOPEN_COM);
  PWin* btn = PWin::getWindowPtr(child);
  if(Com || ServerLan) {
    if(ServerLan) {
      delete ServerLan;
      ServerLan = 0;
      }
    if(Com) {
      delete Com;
      Com = 0;
      }
    safeDeleteP(InfoConn);
    ENABLE(IDC_CHECKBOX_SHOW_COM, false);
    }
  else {
    if(2 == Setting.connType) {
      static TCHAR address[MAX_NUMBER_LEN];
      if(!getTelephoneNumber(address))
        return true;
      if(Setting.lanType) {
        DWORD port = _ttoi(address);
        ServerLan = new customServerLanData(*this, Setting.proto);
        if(!ServerLan->run(port)) {
          delete ServerLan;
          ServerLan = 0;
          return false;
          }
        wsprintf(title + _tcslen(title), _T(" - Server Lan listen to %s - %s"),
            address, Setting.proto ? _T("UDP") : _T("TCP"));
        }
      else {
        DWORD port;
        LPCTSTR addr_name = getAddrAndPort(port, address);
        Com = new PConnBase(addr_name, port, *this, Setting.proto);
//        paramConn pc(addr);
//        Com->setParam(pc);
        if(!Com->open()) {
          MessageBox(*this, _T("Unable to open Lan"),
            _T("Error Lan"), MB_OK | MB_ICONSTOP);
          delete Com;
          Com = 0;
          return false;
          }
        wsprintf(title + _tcslen(title), _T(" - Client Lan connected to %s - %s"),
          address, Setting.proto ? _T("UDP") : _T("TCP"));
        }
      }
    else {
      paramConn pc = getReadedParam();
      if(1 == Setting.connType) {
        static TCHAR number[MAX_NUMBER_LEN];
        if(!getTelephoneNumber(number))
          return true;
        Com = new PConnBase(*this, pc, number, Setting.initModem, Setting.modemType);
        if(!Com->open())
          MessageBox(*this, _T("Modem not responce"), _T("Error Modem"), MB_OK | MB_ICONSTOP);
        wsprintf(title + _tcslen(title), _T(" - Modem %s %s"),
              *number ? _T("calling") : _T("waiting"),
              *number ? number : _T(" "));
        }
      else {
        Com = new PConnBase(pc, *this, Setting.on485);
        if(!Com->open()) {
          MessageBox(*this, _T("Unable to open Comm"),
            _T("Error Comm"), MB_OK | MB_ICONSTOP);
          }
        if(Setting.on485)
          wsprintf(title + _tcslen(title), _T(" - Serial(485) connected to Com%d"), pc.getPort() + 1);
        else
          wsprintf(title + _tcslen(title), _T(" - Serial connected to Com%d"), pc.getPort() + 1);
        ENABLE(IDC_CHECKBOX_SHOW_COM, true);
        }
      if(!Com) {
        delete Com;
        Com = 0;
        ENABLE(IDC_CHECKBOX_SHOW_COM, false);
        return false;
        }
      }
    }
  switchOpenBmp();
  SetWindowText(*getParent(), title);
  return true;
}
//----------------------------------------------------------------------------
void mainClient::setupCom()
{
  // meglio chiudere
  if(Com || ServerLan)
    openCom();
//  setting set;
  Setting.par = getReadedParam();
  paramConn oldPc = Setting.par;
  DWORD oldDelay = Setting.delay;
  DWORD oldPacket = Setting.packet;
  if(IDOK == P_Param(Setting, this).modal()) {
    if(Setting.par != oldPc) {
      DWORD dw = Setting.par;
      writeKeySetup(dw);
      }
    if(oldDelay != Setting.delay)
      writeKeyDelay(Setting.delay);
    if(oldPacket != Setting.packet)
      writeKeyPacket(Setting.packet);
    writeKeyInitModem(Setting.initModem);

    DWORD ct = SET_T(Setting.connType, Setting.modemType, Setting.lanType, Setting.proto, Setting.on485);
    writeKeyConnType(ct);
    }
}
//----------------------------------------------------------------------------
void mainClient::clearReceived()
{
  HWND ctrl = GetDlgItem(*this, IDC_EDIT_REC);
  SendMessage(ctrl, WM_SETTEXT, 0, 0);
  pBuff.reset();
  currAddr = 0;
  InvalidateRect(ctrl, 0, 1);
}
//----------------------------------------------------------------------------
void mainClient::showConn(DWORD conn, LPARAM lParam)
{
/*
  static bool inExec;
  if(inExec)
    return;
  inExec = true;
*/
  if(2 == Setting.connType) {
    if(Com || ServerLan) {
      if(ServerLan) {
        if(Com && (!lParam || Com->iAm(lParam))) {
          if(!dontDelete) {
            delete Com;
            Com = 0;
            }
          dontDelete = false;
          if(ShowMsgBox)
            MessageBox(*this, _T("Client disconnected"), _T("Server Status"), MB_OK);
          }
        }
      else {
        openCom();
        if(ShowMsgBox)
          MessageBox(*this, _T("Server disconnected"), _T("Client Status"), MB_OK);
       }
      }
    }
  else if(1 == Setting.connType) {
    #define MSG_(a) { ID_##a, _T(#a) }
    struct  {
      DWORD id;
      LPCTSTR msg;
      } infoMsg[] = {
      MSG_(MODEM_CONNECTED),
      MSG_(MODEM_ERR_RLSD),
      MSG_(MODEM_TIMEOUT),
      };
    for(uint i = 0; i < SIZE_A(infoMsg); ++i)
      if(infoMsg[i].id == conn) {
        TCHAR buff[500];
        wsprintf(buff, _T("%s"), infoMsg[i].msg);
        MessageBox(*this, buff, _T("Connection Status"), MB_OK);
        if(i)
          openCom();
        break;
        }
    }
  else {
    if(!conn) {
      safeDeleteP(InfoConn);
      ENABLE(IDC_CHECKBOX_SHOW_COM, false);
      }
    if(ShowMsgBox) {
      TCHAR buff[500];
      wsprintf(buff, _T("Connection is %s"), conn ? _T("On") : _T("Down"));
      MessageBox(*this, buff, _T("Connection Status"), MB_OK);
      }
    }
//  inExec = false;
}
//----------------------------------------------------------------------------
void mainClient::readComm(int )
{
  if(!Com)
    return;
  static bool inExec;
  if(inExec)
    return;
  inExec = true;

  #define  SIZE_READ 512 * 256
  LPBYTE buff = new BYTE[SIZE_READ];
  Com->reqNotify(false);
  HWND ctrl = GetDlgItem(*this, IDC_EDIT_REC);
  while(true) {
    if(!Com) {
      delete []buff;
      inExec = false;
      return;
      }
    int avail = Com->has_string();
    if(!avail)
      break;
    avail = min(SIZE_READ -1, avail);
    Com->read_string(buff, avail);
    buff[avail] = 0;
    if(Capture)
      Capture->P_write(buff, avail);
    else {
      int currLen = SendMessage(ctrl, WM_GETTEXTLENGTH, 0, 0);
      SendMessage(ctrl, EM_SETSEL, currLen, currLen);
      if(showHex)
        performShowHex(ctrl, buff, avail, currLen);
      else
        performShowChar(ctrl, buff, avail);
      SendMessage(ctrl, EM_SCROLLCARET, 0, 0);
      }
    int nElem = pBuff.getElem();
    if(nElem)
      pBuff.remove(--nElem);
    pBuff.append(buff, avail);
    pBuff[nElem + avail] = 0;
    getAppl()->pumpMessages();
    Sleep(0);
    }
  if(!Capture) {
    InvalidateRect(ctrl, 0, false);
    }
  delete []buff;
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
static void formatRow(LPTSTR row, const BYTE* byte, int len, int addr)
{
  TCHAR right[RIGHT_ROW + 1];
  addr *= DIM_BYTE_DATA;
  wsprintf(row, _T("%06X "), addr);
  TCHAR t[4];
  int i;
  for(i = 0; i < len; ++i) {
    wsprintf(t, _T("%02X"), byte[i]);
    int offsRow = OFFS_HEX + i * 2;
    row[offsRow] = t[0];
    row[offsRow + 1] = t[1];
    if(iscntrl(byte[i]))
      right[i] = _T('.');
    else
      right[i] = byte[i];
    }

  right[i] = _T('\r');
  right[i + 1] = _T('\n');
  right[i + 2] = 0;

  t[0] = _T(' ');
  t[1] = _T(' ');
  for(; i < DIM_BYTE_DATA; ++i) {
    int offsRow = OFFS_HEX + i * 2;
    row[offsRow] = _T(' ');
    row[offsRow + 1] = _T(' ');
    }

  row[LEFT_ROW - 1] = _T(' ');
  for(int i = 0; i < RIGHT_ROW; ++i)
    row[LEFT_ROW + i] = right[i];
  row[LEN_ROW] = 0;
}
//----------------------------------------------------------------------------
static LPCTSTR MSG_SHOW_SEND_FILE = _T("Sending file...\n\rbyte %d of %d");
static LPCTSTR MSG_SHOW_PERFORM = _T("Performing...\n\rbyte %d of %d");
#define BARCOLOR_CHAR RGB(0x3f, 0xff, 0x10)
#define BARCOLOR_HEX  RGB(0x3f, 0x10, 0xff)
#define BARCOLOR_FILE  RGB(0xff, 0x3f, 0x10)
#define MAX_COUNT 50
#define MAX_PERFORM_ONLINE (1024 * 32)
void mainClient::performShowChar(HWND ctrl, const BYTE* buff, int avail)
{
#if 1
  const BYTE* base = buff;
  DWORD count = 0;
  CHARRANGE range = { 0x7ffffff, 0x7ffffff };
  if(avail > MAX_PERFORM_ONLINE) {
    progressBar bar(this, MSG_SHOW_PERFORM, avail, BARCOLOR_CHAR);
    bar.create();
    UpdateWindow(bar);
    getAppl()->pumpMessages();
    BYTE* tmpBuff = new BYTE[avail + 1];
    for(int i = 0; i < avail; ++i) {
      if(!(++count & 0xff)) {
        if(!bar.setCount(i))
          break;
        getAppl()->pumpMessages();
        if(!Com)
          break;
        }
      if('\n' != buff[i] && '\r' != buff[i] && iscntrl(buff[i]))
        tmpBuff[i] = '.';
      else
        tmpBuff[i] = buff[i];
      }
    tmpBuff[avail] = 0;
    SendMessage(ctrl, EM_EXSETSEL, 0, (LPARAM)&range);
    SendMessageA(ctrl, EM_REPLACESEL, 0, (LPARAM)tmpBuff);
    delete []tmpBuff;
    }

  else {
    while(true) {
      int len = strlen((LPCSTR)buff);
      SendMessageA(ctrl, EM_REPLACESEL, 0, (LPARAM)buff);
      avail -= len + 1;
      if(avail <= 0)
        break;
//      int currLen = SendMessage(ctrl, WM_GETTEXTLENGTH, 0, 0);
      SendMessage(ctrl, EM_EXSETSEL, 0, (LPARAM)&range);
      SendMessage(ctrl, EM_REPLACESEL, 0, (LPARAM)_T("."));
      SendMessage(ctrl, EM_EXSETSEL, 0, (LPARAM)&range);
      buff += len + 1;
      if(!len && ++count < MAX_COUNT)
        continue;
      count = 0;
//      if(!bar.setCount(buff - base))
//        break;
      getAppl()->pumpMessages();
      if(!Com)
        break;
      }
    }
#else
  progressBar bar(this, MSG_SHOW_PERFORM, avail, BARCOLOR_CHAR);
  bar.create();
  UpdateWindow(bar);
  getAppl()->pumpMessages();

  const BYTE* base = buff;
  DWORD count = 0;
  CHARRANGE range = { 0x7ffffff, 0x7ffffff };
  if(avail > MAX_PERFORM_ONLINE) {
    BYTE* tmpBuff = new BYTE[avail + 1];
    for(int i = 0; i < avail; ++i) {
      if(!(++count & 0xff)) {
        if(!bar.setCount(i))
          break;
        getAppl()->pumpMessages();
        if(!Com)
          break;
        }
      if('\n' != buff[i] && '\r' != buff[i] && iscntrl(buff[i]))
        tmpBuff[i] = '.';
      else
        tmpBuff[i] = buff[i];
      }
    tmpBuff[avail] = 0;
    SendMessage(ctrl, EM_EXSETSEL, 0, (LPARAM)&range);
    SendMessageA(ctrl, EM_REPLACESEL, 0, (LPARAM)tmpBuff);
    delete []tmpBuff;
    }

  else {
    while(true) {
      int len = strlen((LPCSTR)buff);
      SendMessageA(ctrl, EM_REPLACESEL, 0, (LPARAM)buff);
      avail -= len + 1;
      if(avail <= 0)
        break;
//      int currLen = SendMessage(ctrl, WM_GETTEXTLENGTH, 0, 0);
      SendMessage(ctrl, EM_EXSETSEL, 0, (LPARAM)&range);
      SendMessage(ctrl, EM_REPLACESEL, 0, (LPARAM)_T("."));
      SendMessage(ctrl, EM_EXSETSEL, 0, (LPARAM)&range);
      buff += len + 1;
      if(!len && ++count < MAX_COUNT)
        continue;
      count = 0;
      if(!bar.setCount(buff - base))
        break;
      getAppl()->pumpMessages();
      if(!Com)
        break;
      }
    }
#endif
}
//----------------------------------------------------------------------------
void mainClient::performShowHex(HWND ctrl, const BYTE* buff, int avail, int currLen)
{
  int pos = currLen % LEN_ROW;
  // probabile errore, si pulisce tutto e si reinizia
  if(pos < (OFFS_BYTE + 1) && pos) {
#if 1
    clearReceived();
#else
    SendMessage(ctrl, WM_SETTEXT, 0, 0);
#endif
    pos = 0;
    }
  TCHAR row[LEN_ROW + 1];
  BYTE t[LEN_ROW + 1];
  int initBuff = 0;
  if(pos) {
    t[0] = 0;
    int needed = pos - OFFS_BYTE - DIM_CR_NL;
    int offsBuff = pBuff.getElem() - 1 - needed;
    // ricarica la parte iniziale della riga dal buffer in memoria
    if(offsBuff >= 0) {
      int i;
      for(i = 0; i < needed; ++i)
        t[i] = pBuff[i + offsBuff];
      // prosegue con il buffer di dati nuovi
      for(; i < DIM_BYTE_DATA && initBuff < avail; ++i, ++initBuff)
        t[i] = buff[initBuff];

      formatRow(row, t, i, currAddr - 1);
      // la coppia CR_NL viene considerata un solo carattere
      // nella selezione, quindi la posizione deve essere anticipata
      // di un carattere per ogni riga
#if 0
      int start = currLen - pos - (currLen / LEN_ROW);
#else
      // Invece sembra che venga considerata la coppia, da verificare se sempre o se
      // dipende dalla versione del richedit. Magari si potrebbe caricare la prima riga e verificare se
      // nel buffer c'è solo il NL o entrambi
      int start = currLen - pos; // - (currLen / LEN_ROW);
#endif
      CHARRANGE range = { start, 0x7ffffff };
      SendMessage(ctrl, EM_EXSETSEL, 0, (LPARAM)&range);
      SendMessage(ctrl, EM_REPLACESEL, 0, (LPARAM)row);
      }
    }

  CHARRANGE range = { 0x7ffffff, 0x7ffffff };
  DWORD count = 0;
  if(avail > MAX_PERFORM_ONLINE) {
    progressBar bar(this, MSG_SHOW_PERFORM, avail, BARCOLOR_HEX);
    bar.create();
    UpdateWindow(bar);
    getAppl()->pumpMessages();
    DWORD needBuff = avail - initBuff;
    needBuff /= DIM_BYTE_DATA;
    // non dovrebbe essere necessario, needBuff dovrebbe essere sempre
    // allineato
    ++needBuff;
    needBuff *= LEN_ROW;
    TCHAR* tmpBuff = new TCHAR[needBuff + 1];
    DWORD initTmp = 0;
    for(;initBuff < avail; initBuff += DIM_BYTE_DATA, ++currAddr, initTmp += LEN_ROW) {
      int len = min(avail - initBuff, DIM_BYTE_DATA);
      memcpy(t, buff + initBuff, len);
      formatRow(row, t, len, currAddr);
      for(int i = 0; i < LEN_ROW; ++i)
        tmpBuff[initTmp + i] = row[i];
      if(!(++count & 0x3f)) {
        if(!bar.setCount(initBuff))
          break;
        getAppl()->pumpMessages();
        if(!Com)
          break;
        }
      }
    SendMessage(ctrl, EM_EXSETSEL, 0, (LPARAM)&range);
    SendMessage(ctrl, EM_REPLACESEL, 0, (LPARAM)tmpBuff);
    delete []tmpBuff;
    }
  else {
    for(;initBuff < avail; initBuff += DIM_BYTE_DATA, ++currAddr) {
      int len = min(avail - initBuff, DIM_BYTE_DATA);
      memcpy(t, buff + initBuff, len);
      formatRow(row, t, len, currAddr);
      SendMessage(ctrl, EM_EXSETSEL, 0, (LPARAM)&range);
      SendMessage(ctrl, EM_REPLACESEL, 0, (LPARAM)row);
//      if(!bar.setCount(initBuff))
//        break;
      getAppl()->pumpMessages();
      }
    }
  UpdateWindow(ctrl);
  getAppl()->pumpMessages();
}
//----------------------------------------------------------------------------
static int resolveSpecialCode(BYTE* buff)
{
  int len = strlen((LPCSTR)buff);
  int j = 0;
  for(int i = 0; i < len; ++i) {
    if('\\' == buff[i]) {
      ++i;
      if('\\' == buff[i])
        buff[j++] = '\\';
      else {
        char t[4];
        t[0] = buff[i];
        t[1] = buff[i + 1];
        t[3] = 0;
        int val;
        sscanf_s(t, "%X", &val);
        buff[j++] = (BYTE)val;
        ++i;
        }
      }
    else
      buff[j++] = buff[i];
    }
  return j;
}
//----------------------------------------------------------------------------
bool mainClient::verifyCom()
{
  if(!Com) {
    if(ServerLan) {
      MessageBox(*this, _T("You must wait a connection"),
          _T("Server status"), MB_YESNO | MB_ICONSTOP);
      return false;
      }
    if(IDNO == MessageBox(*this, _T("Do you want to open now?"),
          _T("Comm is not opened"), MB_YESNO | MB_ICONINFORMATION))
      return false;
    if(!openCom())
      return false;
    }
  return true;
}
//----------------------------------------------------------------------------
progressBar* pBar = 0;
//----------------------------------------------------------------------------
bool mainClient::sendBuff(const BYTE* buff, int len, bool useDelay)
{
  const BYTE* p = buff;
  int packet = len;
  if(Setting.packet)
    packet = min(Setting.packet, len);
  while(len > 0) {
    if(useDelay && Setting.delay) {
      for(int i = 0; i < packet; ++i) {
        if(!Com->write_string(p + i, 1))
          return false;
        Sleep(Setting.delay);
        }
      }
    else
      if(Com->write_string(p, packet) != packet)
        return false;
    len -= packet;
    p += packet;
    packet = min(Setting.packet, len);
    if(pBar) {
      if(!pBar->setCount(pBar->getCurr() + packet))
        return false;
      }
    while(true) {
      if(!getAppl()->pumpMessages())
        break;
      }
    Sleep(100);
    }
  return true;
}
//----------------------------------------------------------------------------
void mainClient::sendString(whichSend type)
{
  if(!verifyCom())
    return;
  static LPBYTE buff = 0;
  static uint dim = 0;
  switch(type) {
    case STRING:
      do {
        HWND hw = GetDlgItem(*this, IDC_EDIT_STRING);
        int len = SendMessageA(hw, WM_GETTEXTLENGTH, 0, 0) + 1;
        if(len > 0) {
          if((uint)len > dim) {
            buff = new BYTE[len];
            dim = len;
            }
          SendMessageA(hw, WM_GETTEXT, len, (LPARAM)buff);
          len = resolveSpecialCode(buff);
          sendBuff(buff, len);
          HWND ctrl = GetDlgItem(*this, IDC_EDIT_STRING);
          SendMessage(ctrl, EM_SETSEL, 0, (LPARAM) -1);
          SetFocus(ctrl);
          }
        } while(false);
      break;
    case VAL:
      do {
        int val = GetDlgItemInt(*this, IDC_EDIT_DEC, 0, 0);
        Com->write_string((LPBYTE)&val, 1);
        } while(false);
      break;
    }
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define INI_FILENAME _T("testConn.ini")
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
bool setKeyPath(LPCTSTR keyName, LPCTSTR path, LPCTSTR keyBlock = MAIN_PATH)
{
  myManageIni ini(INI_FILENAME);
  ini.parse();
  ini_param param = { keyName, path };
  ini.addParam(param, keyBlock);
  return ini.save();
}
//----------------------------------------------------------------------------
bool getKeyPath(LPCTSTR keyName, LPTSTR path, LPCTSTR keyBlock = MAIN_PATH)
{
  path[0] = 0;
  myManageIni ini(INI_FILENAME);
  ini.parse();
  LPCTSTR str = ini.getValue(keyName, keyBlock);
  if(str) {
    _tcscpy_s(path, _MAX_PATH, str);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool setKeyParam(LPCTSTR keyName, DWORD value, LPCTSTR keyBlock = MAIN_PATH)
{
  myManageIni ini(INI_FILENAME);
  ini.parse();
  TCHAR t[64];
  wsprintf(t, _T("%lu"), value);
  ini_param param = { keyName, t };
  ini.addParam(param, keyBlock);
  return ini.save();
}
//----------------------------------------------------------------------------
bool getKeyParam(LPCTSTR keyName, DWORD& value, LPCTSTR keyBlock = MAIN_PATH)
{
  myManageIni ini(INI_FILENAME);
  ini.parse();
  LPCTSTR str = ini.getValue(keyName, keyBlock);
  if(str) {
    __int64 t = _tcstoi64(str, 0, 10);
    value = (DWORD)t;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
class myOpenSave : public POpenSave
{
  public:
    myOpenSave(HWND owner) : POpenSave(owner) {}
  protected:
    virtual void setPathFromHistory();
    virtual void savePathToHistory();
};
//----------------------------------------------------------------------------
void myOpenSave::setPathFromHistory()
{
  TCHAR path[_MAX_PATH];
  if(getKeyPath(OLD_PATH, path))
    SetCurrentDirectory(path);
}
//----------------------------------------------------------------------------
void myOpenSave::savePathToHistory()
{
  TCHAR oldPath[_MAX_PATH];
  GetCurrentDirectory(_MAX_PATH, oldPath);
  setKeyPath(OLD_PATH, oldPath);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
LPCTSTR filterExt[] = { _T(".bin"), _T(""), 0 };
//----------------------------------------------------------------------------
LPCTSTR filterOpen =
//   _T("File Bin (*.bin)\0*.bin\0")
  _T("All Files (*.*)\0*.*\0");
//  _T("All Files (*.*)\0")

//----------------------------------------------------------------------------
LPCTSTR filterSaveAs =
   _T("File Bin (*.bin)\0")
    _T("*.bin\0")
  _T("All Files (*.*)\0*.*\0");
//----------------------------------------------------------------------------
bool mainClient::open_File(LPTSTR file)
{
  infoOpenSave Info(filterExt + 1, filterOpen, infoOpenSave::OPEN_F, 0);
  myOpenSave open(*this);

  if(open.run(Info)) {
    LPCTSTR result = open.getFile();
    _tcscpy_s(file, _MAX_PATH, result);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
#define MSG_WARN_DELAY _T("Delay is not null!\r\nDo you want to use it anywhere?")
#define MSG_WARN_DELAY_TITLE _T("Warning")
//----------------------------------------------------------------------------
void mainClient::sendFile()
{
  if(!verifyCom())
    return;
  TCHAR file[_MAX_PATH];
  if(open_File(file)) {
    P_File pf(file, P_READ_ONLY);
    if(pf.P_open() && pf.get_len()) {
      BYTE buff[4096];
      int len = (int)pf.get_len();
      bool useDelay = 0;
      if(Setting.delay) {
        int result = MessageBox(*this, MSG_WARN_DELAY, MSG_WARN_DELAY_TITLE, MB_YESNOCANCEL | MB_ICONSTOP);
        if(IDCANCEL == result)
          return;
        useDelay = IDYES == result;
        }
      progressBar bar(this, MSG_SHOW_SEND_FILE, len, BARCOLOR_FILE);
      bar.create();
      UpdateWindow(bar);
      pBar = &bar;
      getAppl()->pumpMessages();
      int base = len;
      while(len > 0) {
        int toRead = min(sizeof(buff), len);
        int readed = pf.P_read(buff, toRead);
        if(!readed)
          break;
        if(!sendBuff(buff, readed, useDelay))
          break;
        if(!bar.setCount(base - len + readed))
          break;
        len -= readed;
        getAppl()->pumpMessages();
        }
      pBar = 0;
      if(ShowMsgBox) {
        if(!len)
          MessageBox(*this, _T("File Transfered"), _T("Success"), MB_OK | MB_ICONINFORMATION);
        else
          MessageBox(*this, _T("Transfer aborted\r\nFile may be corrupted!"), _T("Error"), MB_OK | MB_ICONSTOP);
        }
      }
    }
}
//----------------------------------------------------------------------------
static int getDec(TCHAR car)
{
  if(car >= _T('0') && car <= _T('9'))
    return car - _T('0');
  if(car >= _T('a') && car <= _T('f'))
    return car - _T('a') + 10;
  if(car >= _T('A') && car <= _T('F'))
    return car - _T('A') + 10;
  return 0;
}
//----------------------------------------------------------------------------
static int hexToInt(LPCTSTR buff)
{
  int code = 0;
  int len = _tcslen(buff);
  for(int i = len - 1, j = 0; i >= 0; --i, ++j) {
    int val = getDec(buff[i]);
    val <<= j * 4;
    code += val;
    }
  return code;
}
//----------------------------------------------------------------------------
static void intToHex(LPTSTR buff, int val)
{
  wsprintf(buff, _T("%02X"), val);
}
//----------------------------------------------------------------------------
static int binToInt(LPCTSTR buff)
{
  int code = 0;
  int len = _tcslen(buff);
  for(int i = len - 1, j = 0; i >= 0; --i, ++j) {
    if(_T('1') == buff[i])
      code |= 1 << j;
    }
  return code;
}
//----------------------------------------------------------------------------
static void intToBin(LPTSTR buff, int val)
{
  #define MAX_L_BIN 8
  for(int i = MAX_L_BIN - 1; i >= 0; --i) {
    if(val & 1)
      buff[i] = _T('1');
    else
      buff[i] = _T('0');
    val >>= 1;
    }
  buff[MAX_L_BIN] = 0;
}
//----------------------------------------------------------------------------
int mainClient::getDec(uint idCtrl)
{
  TCHAR buff[30];
  GetDlgItemText(*this, idCtrl, buff, SIZE_A(buff));

  switch(idCtrl) {
    case IDC_EDIT_CHAR:
      return (int)(BYTE)buff[0];
    case IDC_EDIT_DEC:
      return _ttoi(buff);
    case IDC_EDIT_HEX:
      return hexToInt(buff);
    case IDC_EDIT_BIN:
      return binToInt(buff);
    }
  return 0;
}
//----------------------------------------------------------------------------
void mainClient::setDec(uint idCtrl, int val)
{
  TCHAR buff[30];

  switch(idCtrl) {
    case IDC_EDIT_DEC:
      SetDlgItemInt(*this, idCtrl, val, 0);
      break;
    case IDC_EDIT_HEX:
      intToHex(buff, val);
      SetDlgItemText(*this, idCtrl, buff);
      break;
    case IDC_EDIT_CHAR:
      buff[0] = val;
      buff[1] = 0;
      SetDlgItemText(*this, idCtrl, buff);
      break;
    case IDC_EDIT_BIN:
      intToBin(buff, val);
      SetDlgItemText(*this, idCtrl, buff);
      break;
    }
}
//----------------------------------------------------------------------------
void mainClient::updateEdit(uint idCtrl)
{
  static bool myChange;
  if(myChange)
    return;
  myChange = true;
  uint id[] = { IDC_EDIT_DEC, IDC_EDIT_HEX, IDC_EDIT_BIN, IDC_EDIT_CHAR };
  for(uint i = 0; i < SIZE_A(id); ++i) {
    if(idCtrl == id[i]) {
      int val = getDec(idCtrl);
      for(int j = i - 1; j >= 0; --j)
        setDec(id[j], val);
      for(uint j = i + 1; j < SIZE_A(id); ++j)
        setDec(id[j], val);
      break;
      }
    }
  myChange = false;
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
void mainClient::saveFile()
{
  int nElem = pBuff.getElem();
  if(!nElem) {
    MessageBox(*this, _T("Nothing to Save"), _T("Warning"),
              MB_OK | MB_ICONINFORMATION);
    return;
    }
  infoOpenSave Info(filterExt, filterSaveAs, infoOpenSave::SAVE_AS, 0);
  myOpenSave saveAs(*this);

  if(saveAs.run(Info)) {
    LPCTSTR result = saveAs.getFile();
    P_File saveF(result, P_CREAT);
    --nElem;
    if(saveF.P_open()) {
      if(nElem != saveF.P_write(&pBuff, nElem))
        MessageBox(*this, _T("Unable to write to File"), _T("Error on file"),
              MB_OK | MB_ICONSTOP);
      else
        MessageBox(*this, _T("File Saved"), _T("Success"), MB_OK | MB_ICONINFORMATION);
      }
    else
      MessageBox(*this, _T("Unable to creat File"), _T("Error on file"),
              MB_OK | MB_ICONSTOP);
    }
}
//----------------------------------------------------------------------------
void mainClient::setCaptureFile()
{
  HWND child = GetDlgItem(*this, IDC_CHECKBOX_CAPTURE_TO_FILE);
  if(!child)
    return;
  if(BST_CHECKED == SendMessage(child, BM_GETCHECK, 0, 0)) {
    infoOpenSave Info(filterExt, filterSaveAs, infoOpenSave::SAVE_AS, 0);
    myOpenSave saveAs(*this);

    if(saveAs.run(Info)) {
      LPCTSTR result = saveAs.getFile();
      if(Capture)
        delete Capture;
      Capture = new P_File(result, P_CREAT);
      if(!Capture->P_open()) {
        delete Capture;
        Capture = 0;
        MessageBox(*this, _T("Unable to creat File"), _T("Error on file"),
              MB_OK | MB_ICONSTOP);
        SendMessage(child, BM_SETCHECK, BST_UNCHECKED, 0);
        }
      else
        clearReceived();
      }
    else
      SendMessage(child, BM_SETCHECK, BST_UNCHECKED, 0);
    }
  else {
    delete Capture;
    Capture = 0;
    HWND ctrl = GetDlgItem(*this, IDC_EDIT_REC);
    currAddr = 0;
    int elem = pBuff.getElem();
    if(elem > 1) {
      if(showHex)
        performShowHex(ctrl, &pBuff, elem - 1, 0);
      else
        performShowChar(ctrl, &pBuff, elem - 1);
      }
    }
}
//----------------------------------------------------------------------------
void mainClient::changeShow()
{
  HWND cBox = GetDlgItem(*this, IDC_CHECKBOX_SHOW_HEX);
  if(!cBox)
    return;

  HWND ctrl = GetDlgItem(*this, IDC_EDIT_REC);
  SendMessage(ctrl, WM_SETTEXT, 0, 0);
  currAddr = 0;
  int elem = pBuff.getElem();
  if(BST_CHECKED == SendMessage(cBox, BM_GETCHECK, 0, 0)) {
    showHex = true;
    if(elem)
      performShowHex(ctrl, &pBuff, elem - 1, 0);
    }
  else {
    showHex = false;
    if(elem) {
      const BYTE* tmp = &pBuff;
      performShowChar(ctrl, tmp, elem - 1);
      }
    }
  InvalidateRect(ctrl, 0, 1);
}
//----------------------------------------------------------------------------
void mainClient::CloseApp(HWND /*hwnd*/)
{
  PostQuitMessage(0);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool writeKey(LPCTSTR name, DWORD value)
{
  return setKeyParam(name, value);
}
//----------------------------------------------------------------------------
bool readKey(LPCTSTR name, DWORD& value, bool writeIfNotFound)
{
  if(!getKeyParam(name, value)) {
    if(writeIfNotFound)
      setKeyParam(name, value);
    return false;
    }
  return true;
}
//----------------------------------------------------------------------------
bool writeKeyString(LPCTSTR name, LPCTSTR value)
{
  return setKeyPath(name, value);
}
//----------------------------------------------------------------------------
bool readKeyString(LPCTSTR name, LPTSTR value, size_t lenBuff, bool writeIfNotFound)
{
  if(!getKeyPath(name, value)) {
    if(writeIfNotFound)
      setKeyPath(name, value);
    return false;
    }
  return true;
}
//----------------------------------------------------------------------------

