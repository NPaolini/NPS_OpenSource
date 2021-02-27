//------ WrapSquidServer.cpp -------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "WrapSquidServer.h"
#include "queueLog.h"
#include "p_manage_ini.h"
#include "base64.h"
//----------------------------------------------------------------------------
static logQueue LogQueue;
logQueue& getLogQueue() { return LogQueue; }
//----------------------------------------------------------------------------
myChild::myChild(PWin* parent, uint id, HANDLE_CONN hc, HINSTANCE hInst) :
    baseClass(parent, id, hc, hInst), hChildStd_IN_Rd(0), hChildStd_IN_Wr(0),
    hChildStd_OUT_Rd(0), hChildStd_OUT_Wr(0), Initializated(false), progr_type(0),
    closing(false), Obfuscated(false)
{}
//-------------------------------------------------------------------
static bool IsValidHandle(HANDLE h)
{
  DWORD f;
  return h && h != INVALID_HANDLE_VALUE && GetHandleInformation(h, &f);
}
//-------------------------------------------------------------------
static void CloseHandleSafe(HANDLE h)
{
  if(IsValidHandle(h))
    CloseHandle(h);
}
//-------------------------------------------------------------------
#define CTRL(x) (#x[0]-'a'+1)
myChild::~myChild()
{
  closing = true;
  char end[2] = { CTRL(d), CTRL(z) };
  perform_write(end, 2);
  destroy();
  CloseHandleSafe(hChildStd_IN_Rd);
  CloseHandleSafe(hChildStd_IN_Wr);
  CloseHandleSafe(hChildStd_OUT_Rd);
  CloseHandleSafe(hChildStd_OUT_Wr);
  delete []progr_type;
}
//-------------------------------------------------------------------
bool myChild::create()
{
  return baseClass::create();
}
//-------------------------------------------------------------------
static bool readUntil(PConnBase* conn, char c)
{
  DWORD count = 0;
  while(conn->isConnected()) {
    if(conn->has_string()) {
      BYTE t;
      conn->read_string(&t, 1);
      if(c == t)
        return true;
      }
    else {
      if (100 <= ++count)
        break;
      Sleep(10);
      }
    }
  return false;
}
//-------------------------------------------------------------------
static DWORD storeUntil(PConnBase* conn, char c1, char c2, LPSTR buff, DWORD len)
{
  DWORD count = 0;
  DWORD pos = 0;
  while(conn->isConnected()) {
    if(conn->has_string()) {
      BYTE t;
      conn->read_string(&t, 1);
      if(c1 == t || c2 == t) {
        Sleep(0);
        conn->passThrough(cReset, 0);
        return pos;
        }
      buff[pos++] = t;
      if(pos == len)
        return pos;
      }
    else {
      if(100 <= ++count)
        break;
      Sleep(10);
      }
    }
  return 0;
}
//-------------------------------------------------------------------
#define TIME_TO_LIVE (1000 * 60 * 10)
//-------------------------------------------------------------------
bool myChild::th_readConn()
{
  if(closing || !Conn->isConnected())
    return false;
  if(!Conn->has_string())
    return true;

  char buff[BUFF_SIZE];
  DWORD readed = storeUntil(Conn, '\n', '\r', buff, sizeof(buff) - 1);
  if(readed) {
    while(readed && (unsigned char)buff[readed - 1] <= ' ')
      --readed;
    buff[readed] = 0;
    if(!Initializated) {
      if(init_send_ok(buff, readed)) {
        PostMessage(*this, WM_FROM_CLIENT, MAKEWPARAM(CM_KILL_TIMER, 0), 0);
        PostMessage(*getParent(), WM_FROM_CLIENT, MAKEWPARAM(CM_OK_CLIENT, 0), (LPARAM)this);
        }
      else
        PostMessage(*getParent(), WM_FROM_CLIENT, MAKEWPARAM(CM_REMOVE_CLIENT, 0), (LPARAM)this);
      }
    else {
      perform_wrap(buff, readed);
      //idTimer = SetTimer(*this, 555, TIME_TO_LIVE, 0);
      }
    }
  return true;
}
//-------------------------------------------------------------------
bool myChild::th_writeConn()
{
  return true;
}
//-------------------------------------------------------------------
/*
 il file è nella forma:
 [listen_port]
 port=2000
 log_access=1
 [basic]
 progr=basic.exe
 ;param=
 obfuscated=1|0
 [ntlm]
 progr=ntlm.exe
 ;param=
 [negotiate]
 progr=negotiate.exe
 ;param=
 [external]
 progr=ext_ad_group.exe
 param=-G
*/
//-------------------------------------------------------------------
bool myChild::createProcessChild(LPTSTR szCmdline)
{
  PROCESS_INFORMATION piProcInfo;
  STARTUPINFO siStartInfo;
  BOOL bSuccess = FALSE;

// Set up members of the PROCESS_INFORMATION structure.

  ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

// Set up members of the STARTUPINFO structure.
// This structure specifies the STDIN and STDOUT handles for redirection.

  ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
  siStartInfo.cb = sizeof(STARTUPINFO);
  siStartInfo.hStdError = hChildStd_OUT_Wr;
  siStartInfo.hStdOutput = hChildStd_OUT_Wr;
  siStartInfo.hStdInput = hChildStd_IN_Rd;
  siStartInfo.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
  siStartInfo.wShowWindow = SW_HIDE;

// Create the child process.

  bSuccess = CreateProcess(NULL,
      szCmdline,     // command line
      NULL,          // process security attributes
      NULL,          // primary thread security attributes
      TRUE,          // handles are inherited
      0,             // creation flags
      NULL,          // use parent's environment
      NULL,          // use parent's current directory
      &siStartInfo,  // STARTUPINFO pointer
      &piProcInfo);  // receives PROCESS_INFORMATION

   // If an error occurs, exit the application.
  if(!bSuccess)
    return false;
  else {
    // Close handles to the child process and its primary thread.
    // Some applications might keep these handles to monitor the status
    // of the child process, for example.

    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);
    }
  return true;
}
//-------------------------------------------------------------------
bool myChild::init_send_ok(LPSTR buff, DWORD len)
{
  Initializated = false;
  TCHAR tmp[_MAX_PATH];
  GetModuleDirName(SIZE_A(tmp), tmp);
  appendPath(tmp, INI_FILE);
  manageIni ini(tmp);
  if(!ini.parse())
    return false;
  TCHAR b2[_MAX_PATH];
  copyStrZ(b2, buff);
  LPCTSTR p = ini.getValue(KEY_INI_PROGR, b2);
  if(!p)
    return false;
  GetModuleDirName(SIZE_A(tmp), tmp);
  appendPath(tmp, p);
  if(!P_File::P_exist(tmp))
    return false;
  p = ini.getValue(KEY_INI_PARAM, b2);
  if(p) {
    _tcscat_s(tmp, _T(" "));
    _tcscat_s(tmp, p);
    }
  p = ini.getValue(KEY_INI_OBFUSCATED, b2);
  if(p) {
    Obfuscated = _ttoi(p);
    }
  progr_type = str_newdup(buff);
  SECURITY_ATTRIBUTES saAttr;
  saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
  saAttr.bInheritHandle = TRUE;
  saAttr.lpSecurityDescriptor = NULL;

  if(!CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, BUFF_SIZE))
    return false;
  if(!SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
    return false;

  if(!CreatePipe(&hChildStd_IN_Rd, &hChildStd_IN_Wr, &saAttr, BUFF_SIZE))
    return false;
  if(!SetHandleInformation(hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0) )
    return false;

  logQueue& q = getLogQueue();
  pDataSend pds = q.getFree();
  if(pds) {
    pds->type = 1;
    strcpy_s(pds->buff, progr_type);
    strcat_s(pds->buff, "\n");
    q.Add(pds);
    }
  if(createProcessChild(tmp)) {
    Initializated = true;
    char resp[256] =  "ok-";
    strcat_s(resp, buff);
    strcat_s(resp, "\n");
    Conn->write_string((LPCBYTE)resp, strlen(resp));
    pds = q.getFree();
    if(pds) {
      pds->type = 2;
      strcpy_s(pds->buff, resp);
      q.Add(pds);
      }
    Sleep(100);
    }
  return Initializated;
}
//-------------------------------------------------------------------
static bool findNL(LPCSTR b, DWORD r)
{
#if 0
  return '\n' == b[r - 1] || '\r' == b[r - 1];
#else
  for(int i = (int)r - 1; i >= 0; --i)
    if('\n' == b[i] || '\r' == b[i])
      return true;
  return false;
#endif
}
//-------------------------------------------------------------------
bool myChild::perform_write(LPSTR buff, DWORD len)
{
  if(!IsValidHandle(hChildStd_IN_Wr))
    return false;
  DWORD dwWritten = 0;
  DWORD pos = 0;
  do {
    DWORD w;
    BOOL bSuccess = WriteFile(hChildStd_IN_Wr, buff + pos, len - pos, &w, NULL);
    if(!bSuccess || 0 == w)
      return false;
    dwWritten += w;
    if(dwWritten == len)
      break;
    pos += w;
    Sleep(50);
    } while(true);
  return true;
}
//-------------------------------------------------------------------
bool myChild::perform_read(LPSTR buff, DWORD szBuff, DWORD& dwRead)
{
  if(!IsValidHandle(hChildStd_OUT_Rd))
    return false;
  int pos = 0;
  dwRead = 0;
  do {
    DWORD r;
    BOOL bSuccess = ReadFile(hChildStd_OUT_Rd, buff + pos , szBuff - pos, &r, NULL);
    if(!bSuccess || 0 == r)
      return false;
    dwRead += r;
    if(findNL(buff + pos, r))
      break;
    pos += r;
    Sleep(50);
    } while(dwRead < szBuff);
  while(dwRead) {
    if((unsigned char)buff[dwRead - 1] <= ' ')
      --dwRead;
    else
      break;
    }
  buff[dwRead++] = '\n';
  buff[dwRead] = 0;
  return true;
}
//-------------------------------------------------------------------
DWORD decode64(LPSTR target, LPSTR source, DWORD len)
{
  uint res = base64_decode(target, source, len);
  target[res] = 0;
  return res;
}
//-------------------------------------------------------------------
void myChild::addInfoType(LPSTR buff, size_t dim)
{
  SYSTEMTIME st;
  GetLocalTime(&st);
  wsprintfA(buff, "%4d-%02d-%02d %02d.%02d.%02d %s[%d] - ", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, progr_type, Attr.id);
}
//-------------------------------------------------------------------
bool myChild::perform_wrap(LPSTR buff, DWORD len)
{
  char buff2[BUFF_SIZE];
  len = decode64(buff2, buff, len);
  buff2[len++] = '\n';
  buff2[len] = 0;
  bool bSuccess = perform_write(buff2, len);
  logQueue& q = getLogQueue();
  pDataSend pds = q.getFree();
  if(pds) {
    pds->type = 1 + Obfuscated * 10;
    addInfoType(pds->buff, SIZE_A(pds->buff));
    if(Obfuscated)
      strcat_s(pds->buff, buff);
    else
      strcat_s(pds->buff, buff2);
    q.Add(pds);
    }
  if(bSuccess) {
    DWORD dwRead = 0;
    bSuccess = perform_read(buff2, sizeof(buff2) - 4, dwRead);
    Conn->write_string((LPCBYTE)buff2, dwRead);
    pds = q.getFree();
    if(pds) {
      pds->type = 2;
      addInfoType(pds->buff, SIZE_A(pds->buff));
      strcat_s(pds->buff, buff2);
      q.Add(pds);
      }
    return true;
    }
  return false;
}

//-------------------------------------------------------------------
myServerConn::myServerConn(PWin* parent, uint id, uint port, HINSTANCE hInst) :
    baseClass(parent, id, port, hInst), pfLog(0)
{}
//-------------------------------------------------------------------
myServerConn::myServerConn(PWin* parent, uint id, const PVect<WORD>& port, HINSTANCE hInst) :
    baseClass(parent, id, port, hInst), pfLog(0)
{}
//-------------------------------------------------------------------
myServerConn::~myServerConn() { destroy(); delete pfLog; }
//-------------------------------------------------------------------
#define LOG_FILE_N _T("wrap_squid_log")
#define LOG_FILE_E _T(".txt")
#define LOG_FILE _T("wrap_squid_log.txt")
//-------------------------------------------------------------------
static LPCSTR getInfo(int ix)
{
  switch (ix) {
  case 1:
    return "Recv";
  case 2:
    return "Send";
  default:
    return "???";
  }
}
//-------------------------------------------------------------------
#define MAX_DIM_LOG (1024 * 1024 * 5)
//-------------------------------------------------------------------
static P_File* rotate(P_File* pf)
{
	if (pf->get_len() < MAX_DIM_LOG)
		return pf;

	TCHAR path[_MAX_PATH];
	GetModuleDirName(SIZE_A(path), path);
	appendPath(path, LOG_FILE_N);
	TCHAR date[64];
	SYSTEMTIME st;
	GetLocalTime(&st);
	wsprintf(date, _T("%4d-%02d-%02d %02d.%02d.%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	_tcscat_s(path, date);
	_tcscat_s(path, LOG_FILE_E);
	TCHAR oldpath[_MAX_PATH];
	_tcscpy_s(oldpath, pf->get_name());
	pf->P_close();
	pf->P_rename(path);
	delete pf;
	pf = new P_File(oldpath);
	pf->P_open();
	return pf;
}
//-------------------------------------------------------------------
void myServerConn::log(pDataSend pds)
{
  logQueue& q = getLogQueue();
  if(!pfLog) {
    TCHAR path[_MAX_PATH];
    GetModuleDirName(SIZE_A(path), path);
    appendPath(path, LOG_FILE);
    pfLog = new P_File(path);
    if(!pfLog->P_open()) {
      delete pfLog;
      pfLog = 0;
      q.setFree(pds);
      return;
      }
    }
  pfLog = rotate(pfLog);
  pfLog->P_seek(0, SEEK_END_);
  char t[64];
  bool obf = pds->type > 10;
  int ix = pds->type % 10;
  wsprintfA(t, "%s - ", getInfo(ix));
  pfLog->P_write(t, strlen(t));
  if(obf || 2 == ix)
    strcat_s(pds->buff, "\n");
  pfLog->P_write(pds->buff, strlen(pds->buff));
  q.setFree(pds);
}
//-------------------------------------------------------------------
LRESULT  myServerConn::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_FROM_QUEUE:
      switch (wParam) {
        case 2:
          log((pDataSend)lParam);
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}

