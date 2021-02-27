//------- mainClient.cpp -----------------------------------------------------
#include "precHeader.h"
#include "wm_custom.h"
#include <stdio.h>
#include "resource.h"
#include <wtypes.h>
#include <richedit.h>
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
#include "P_Lan.h"
#include "P_backup.h"
#include "P_Timing.h"
#include "P_Email.h"
#include "P_User.h"
#include "P_Log.h"
#include "P_Site.h"
//----------------------------------------------------------------------------
#define KEY_STRING_SETUP _T("Setup")
#define KEY_CODE _T("Code")
#define KEY_DELAY _T("Delay_send")
#define KEY_PACKET _T("Packet_send")
#define KEY_WIFI _T("UseWiFi")
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
inline
bool writeKeyDelay(DWORD value)
{
  return writeKey(KEY_DELAY, value);
}
//----------------------------------------------------------------------------
inline
bool readKeyDelay(DWORD& value, bool writeIfNotFound = true)
{
  return readKey(KEY_DELAY, value, writeIfNotFound);
}
//----------------------------------------------------------------------------
inline
bool writeKeyPacket(DWORD value)
{
  return writeKey(KEY_PACKET, value);
}
//----------------------------------------------------------------------------
inline
bool writeKeyWiFi(bool value)
{
  return writeKey(KEY_WIFI, value);
}
//----------------------------------------------------------------------------
inline
bool readKeyPacket(DWORD& value, bool writeIfNotFound = true)
{
  return readKey(KEY_PACKET, value, writeIfNotFound);
}
//----------------------------------------------------------------------------
inline
bool readKeyWiFi(bool& value, bool writeIfNotFound = true)
{
  DWORD t = value;
  bool success = readKey(KEY_WIFI, t, writeIfNotFound);
  value = t;
  return success;
}
//----------------------------------------------------------------------------
paramConn getDefaultparamCom()
{
  paramConn pc;
  pc.b.baud = paramConn::_115200;
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
#define YELLOW_COLOR  RGB(0xff,0xff,0xcf)
#define bkgColor YELLOW_COLOR
//----------------------------------------------------------------------------
mainClient::mainClient(PWin* parent, uint resId,  HINSTANCE hInstance)
  : baseClass(parent, resId, hInstance), Conn(0), hwTips(0)
{
#define IX_BMP_CLOSED 1
#define IX_BMP_OPENED 10


#define MAX_BMP SIZE_A(idBmp)
  int idBmp[] = { ID_IMAGE_TIMING, ID_IMAGE_RED_SEM, ID_IMAGE_LAN, ID_IMAGE_SAVE, ID_IMAGE_SAVE, ID_IMAGE_EMAIL,
                  ID_IMAGE_LOG, ID_IMAGE_USERS, ID_IMAGE_SETUP, ID_IMAGE_DONE, ID_IMAGE_GREEN_SEM };

#define MAX_BTN SIZE_A(idBtn)
  int idBtn[] = { IDC_BUTTON_TIMING, IDC_BUTTONOPEN_COM, IDC_BUTTON_LAN, IDC_BUTTON_BACKUP, IDC_BUTTON_BACKUP_SITE,
                  IDC_BUTTON_EMAIL, IDC_BUTTON_LOG, IDC_BUTTON_USERS, IDC_BUTTON_SETUP, IDCANCEL };
  Bmp.setDim(MAX_BMP);
  for(uint i = 0; i < MAX_BTN; ++i) {
    Bmp[i] = new PBitmap(idBmp[i], getHInstance(), RT_RCDATA);
    POwnBtnImageStd* btn;
    if(IDC_BUTTONOPEN_COM == idBtn[i])
      btn = new myPOwnBtnImageStd(this, idBtn[i], Bmp[i]);
    else
      btn = new POwnBtnImageStd(this, idBtn[i], Bmp[i]);
    btn->setImgStyle(POwnBtnImageStd::eisScaled);
    }
  Bmp[MAX_BTN] = new PBitmap(idBmp[MAX_BTN], getHInstance(), RT_RCDATA);
}
//----------------------------------------------------------------------------
mainClient::~mainClient()
{
  if(hwTips)
    DestroyWindow(hwTips);
  destroy();
  delete Conn;

  int nElem = Bmp.getElem();
  for(int i = 0; i < nElem; ++i)
    delete Bmp[i];
}
//----------------------------------------------------------------------------
bool mainClient::create()
{
  if(!baseClass::create()) {
    return false;
    }
  struct idAndText {
    uint id;
    LPTSTR tips;
    };

  static const idAndText id_txt[] = {
    { IDC_BUTTON_SETUP, _T("Serial Setting") },
  { IDC_BUTTON_TIMING, _T("Timings:\r\n(Date-time, Alarm, Roll-up shutter, ...)") },
    { IDC_BUTTONOPEN_COM, _T("Open Serial Comunication") },
    { IDC_BUTTON_BACKUP, _T("Backup/Restore EEPROM") },
    { IDC_BUTTON_BACKUP_SITE, _T("Download/Upload Site files") },
    { IDC_BUTTON_LAN,  _T("Lan Setting") },
    { IDC_BUTTON_EMAIL, _T("Email") },
    { IDC_BUTTON_LOG,   _T("Logs") },
    { IDC_BUTTON_USERS, _T("Users") },
    { IDCANCEL,   _T("Done") },
    };

#ifndef TTS_BALLOON
 #define TTS_BALLOON 0x40
#endif
  hwTips = CreateWindow(TOOLTIPS_CLASS, 0, TTS_ALWAYSTIP | TTS_BALLOON,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        0, 0, getHInstance(), 0);

  if(hwTips) {
  SendMessage(hwTips, TTM_SETMAXTIPWIDTH, 0, 100);
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
void mainClient::checkEnable()
{
  int idBtn[] = { IDC_BUTTON_TIMING, IDC_BUTTON_LAN, IDC_BUTTON_BACKUP, IDC_BUTTON_BACKUP_SITE,
                  IDC_BUTTON_EMAIL, IDC_BUTTON_LOG, IDC_BUTTON_USERS };
  for(uint i = 0; i < SIZE_A(idBtn); ++i)
    ENABLE(idBtn[i], toBool(Conn));
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
//          readComm(HIWORD(wParam));
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
        case IDC_BUTTON_SETUP:
          setupCom();
          break;
        case IDC_BUTTONOPEN_COM:
          openConn();
          break;
        case IDC_BUTTON_BACKUP:
          P_backup(Conn, this).modal();
          break;
        case IDC_BUTTON_BACKUP_SITE:
          P_Site(Conn, this).modal();
          break;
        case IDC_BUTTON_LAN:
          P_Lan(Conn, this).modal();
          break;
        case IDC_BUTTON_TIMING:
          P_Timing(Conn, this).modal();
          break;
        case IDC_BUTTON_EMAIL:
          P_Email(Conn, this).modal();
          break;
        case IDC_BUTTON_LOG:
          P_Log(Conn, this).modal();
          break;
        case IDC_BUTTON_USERS:
          P_User(Conn, this).modal();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void fillSetting(setting& s)
{
  s.par = getReadedParam();
  TCHAR code[64] = _T("nps0000000000001");
  readKeyCode(code, SIZE_A(s.Code));
  _tcscpy_s(s.Code, code);
  readKeyDelay(s.delay);
  readKeyPacket(s.packet);
  readKeyWiFi(s.useWiFi);
}
//----------------------------------------------------------------------------
void saveSetting(const setting& s)
{
  DWORD dw = s.par;
  writeKeySetup(dw);
  writeKeyCode(s.Code);
  writeKeyDelay(s.delay);
  writeKeyPacket(s.packet);
  writeKeyWiFi(s.useWiFi);
}
//----------------------------------------------------------------------------
void mainClient::setupCom()
{
    // meglio chiudere
  if(Conn)
    openConn();
  fillSetting(Setting);
  if(IDOK == P_Param(Setting, this).modal())
    saveSetting(Setting);
}
//----------------------------------------------------------------------------
#define MATCH_WAIT_MSG (LPBYTE)"Welcome to Home Automation by NPS"
#define MATCH_DIM_WAIT (sizeof(MATCH_WAIT_MSG) + 1)

#define MAX_WAIT_TIMEOUT (20 * 1000)
#define WAIT_TIME 50
#define WAIT_REPEAT (MAX_WAIT_TIMEOUT / WAIT_TIME)
//----------------------------------------------------------------------------
class checkConnConsumer : public p_matchStream<BYTE, MATCH_DIM_WAIT>
{
  private:
    typedef p_matchStream<BYTE, MAX_DIM_CHECK> baseClass;
  public:
    checkConnConsumer(PConnBase* conn) : Conn(conn) { }
  protected:
    virtual bool has_data();
    virtual void read_one_data(BYTE& buff);
  private:
    PConnBase* Conn;
};
//-----------------------------------------------------------
bool checkConnConsumer::has_data()
{
  for(uint i = 0; i < WAIT_REPEAT; ++i) {
    if(Conn->has_string())
      return true;
    if(!Conn->isConnected())
      return false;
    Sleep(WAIT_TIME);
    }
  return false;
}
//-----------------------------------------------------------
void checkConnConsumer::read_one_data(BYTE& buff)
{
  Conn->read_string(&buff, 1);
}
//----------------------------------------------------------------------------
//#define MY_DEBUG
#ifdef MY_DEBUG
  #define _MAKECONN_ new PConnBase(_T("127.0.0.1"), 2000, *this, false);
#else
  #define _MAKECONN_ new PConnBase(pc, *this, false);
#endif
//----------------------------------------------------------------------------
bool mainClient::openConn()
{
  TCHAR title[512] = _T("Domotica by NPS");
  HWND child = GetDlgItem(*this, IDC_BUTTONOPEN_COM);
  PWin* btn = PWin::getWindowPtr(child);
  if(Conn) {
    delete Conn;
    Conn = 0;
    }
  else {
    paramConn pc = getReadedParam();
    Conn = _MAKECONN_
    if(!Conn->open()) {
      MessageBox(*this, _T("Unable to open Comm"), _T("Error Comm"), MB_OK | MB_ICONSTOP);
      delete Conn;
      Conn = 0;
      return false;
      }
    wsprintf(title + _tcslen(title), _T(" - Serial connected to Com%d"), pc.getPort() + 1);
    checkConnConsumer ccc(Conn);
    ccc.checkMatch(MATCH_WAIT_MSG, MATCH_DIM_WAIT);
    }
  switchOpenBmp();
  SendMessage(*getParent(), WM_SETTEXT, 0, (LPARAM)title);
  return true;
}
//----------------------------------------------------------------------------
void mainClient::CloseApp(HWND /*hwnd*/)
{
  PostQuitMessage(0);
}
//----------------------------------------------------------------------------
void mainClient::switchOpenBmp()
{
  HWND child = GetDlgItem(*this, IDC_BUTTONOPEN_COM);
  PWin* btn = PWin::getWindowPtr(child);
  uint ix = IX_BMP_CLOSED;
  if (Conn) {
    ix = IX_BMP_OPENED;
  }

  myPOwnBtnImageStd *iBtn = dynamic_cast<myPOwnBtnImageStd*>(btn);
  if (iBtn) {
    iBtn->chgBmp(Bmp[ix]);
    InvalidateRect(child, 0, 0);
  }
  TOOLINFO ti;
  ti.cbSize = sizeof(TOOLINFO);
  ti.uFlags = TTF_IDISHWND;
  ti.hwnd = *this;
  ti.hinst = getHInstance();
  ti.uId = (UINT_PTR)GetDlgItem(*this, IDC_BUTTONOPEN_COM);
  LPTSTR tips = _T("Open Serial Comunication");
  if (Conn)
    tips = _T("Close Serial Comunication");
  ti.lpszText = tips;
  SendMessage(hwTips, TTM_UPDATETIPTEXT, 0, (LPARAM)(LPTOOLINFO)&ti);
  checkEnable();
}
//----------------------------------------------------------------------------
//-----------------------------------------------------------
#define INI_FILENAME _T("domotica.ini")
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
