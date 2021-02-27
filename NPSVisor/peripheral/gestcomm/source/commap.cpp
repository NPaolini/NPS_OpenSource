//---------------- commap.cpp ------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include <float.h>
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "commap.h"
#include "commwn.h"
#include "commidt.h"
//----------------------------------------------------------------------------
PWin* gCommApp::initMainWindow(LPCTSTR title, HINSTANCE hInstance)

{
  commIdentity *cI = getIdentity();
  gestCommWindow* frame = cI->getMainWindow(title, hInstance);

  frame->Attr.w = 250;
  frame->Attr.h = 200;

  return frame;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#if !defined(UNICODE) && !defined(_UNICODE)
//----------------------------------------------------------------------------
// not UNICODE
//----------------------------------------------------------------------------
#define USE_GLOBAL_ARGV
//----------------------------------------------------------------------------
#ifdef USE_GLOBAL_ARGV
//----------------------------------------------------------------------------
#include <dos.h>
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
/**/
typedef unsigned char uchar;
static char *getFirstValid(char* buff, int& len)
{
  while((uchar)(*buff) <= ' ' && len) {
    ++buff;
    --len;
    }
  return buff;
}
//----------------------------------------------------------------------------
static char *getArgv(char* buff, int& len)
{
  bool enclosed = false;
  if(*buff == '\"') {
    ++buff;
    enclosed = true;
    --len;
    }
  if(enclosed)
    while(*buff && *buff != '\"' && len) {
      ++buff;
      --len;
      }
  else
    while(*buff && (uchar)(*buff) > ' ' && len) {
      ++buff;
      --len;
      }
  *buff = 0;
  if(len) {
    --len;
    ++buff;
    }
  return buff;
}
//----------------------------------------------------------------------------
#define MAX_ARGV 20
static int lineToArgv(char* cmdLine, char *argv[])
{
  int len = strlen(cmdLine);
  int argc = 0;
  char* buff = cmdLine;
  while(len && argc < MAX_ARGV) {
    buff = getFirstValid(buff, len);
    argv[argc++] = buff;
    buff = getArgv(buff, len);
    }
  return argc;
}
//----------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------
static int parseArg()
{
  commIdentity *cI = getIdentity();
  if(!cI)
    return 0;
#ifndef USE_GLOBAL_ARGV
/**/
  LPTSTR commandLine = GetCommandLine();
  int len = strlen(commandLine);

  char *cmdLine = new char[len + 2];
  memcpy(cmdLine, commandLine, len + 1);
  cmdLine[len + 1] = 0;

  char *argv[MAX_ARGV];
  memset(argv, 0, sizeof(argv));
  int argc = lineToArgv(cmdLine, argv);
/**/
#else
  int argc = __argc;
  char **argv = __argv;
#endif
  unsigned ok = 0;
  while(--argc >= 0) {
    if(argv[argc][0] == '/' || argv[argc][0] == '-') {
      switch(argv[argc][1]) {
        case 'H':
        case 'h':
          do {
            unsigned long t;
            sscanf_s(argv[argc]+2, "%ul", &t);
            HWND hWnd = reinterpret_cast<HWND>(t);
            cI->setHWND(hWnd);
            ok |= 1;
            } while(false);
          break;
        case 'C':
        case 'c':
          do {
            unsigned long pCom;
            sscanf_s(argv[argc]+2, "%ul", &pCom);
            cI->setComm(pCom);
            ok |= 1 << 1;
            } while(false);
          break;
        default:
          cI->parseArg(argv[argc] + 1);
          break;
        }
      }
    }
#ifndef USE_GLOBAL_ARGV
  LocalFree(commandLine);
  delete []cmdLine;
#endif
  if(!cI->isOkArg(ok)) {
    ::MessageBox(0,"L'applicazione non può essere avviata da sola",
          "Errore: Parametri errati",MB_OK);
    return 0;
    }

  return 1;
}
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
// UNICODE
#include <Shellapi.h>
//----------------------------------------------------------------------------
static int parseArg()
{
  commIdentity *cI = getIdentity();
  if(!cI)
    return 0;

  LPTSTR commandLine = GetCommandLine();
  int argc;
  LPTSTR* argv = CommandLineToArgvW(commandLine, &argc);

  unsigned ok = 0;
  while (--argc >= 0) {
    if (argv[argc][0] == _T('/') || argv[argc][0] == _T('-')) {
      switch (argv[argc][1]) {
      case _T('H'):
      case _T('h'):
        do {
          unsigned long t;
          _stscanf_s(argv[argc] + 2, _T("%ul"), &t);
          HWND hWnd = reinterpret_cast<HWND>(t);
          cI->setHWND(hWnd);
          ok |= 1;
        } while (false);
        break;
      case _T('C'):
      case _T('c'):
        do {
          unsigned long pCom;
          _stscanf_s(argv[argc] + 2, _T("%ul"), &pCom);
          cI->setComm(pCom);
          ok |= 1 << 1;
        } while (false);
        break;
      default:
        cI->parseArg(argv[argc] + 1);
        break;
      }
    }
  }


  LocalFree(argv);

  if(!cI->isOkArg(ok)) {
    ::MessageBox(0, _T("L'applicazione non può essere avviata da sola"),
          _T("Errore: Parametri errati"),MB_OK);
    return 0;
    }

  return 1;
}
#endif
//----------------------------------------------------------------------------
#define SECOND_BEFORE_ABORT 10000
#define TIME_BETWEEN_REPEAT 200
#define REPEAT_BEFORE_ABORT (SECOND_BEFORE_ABORT / TIME_BETWEEN_REPEAT)
//----------------------------------------------------------------------------
static void replaceSlash(LPTSTR buff, size_t len)
{
  for(size_t i = 0; i < len; ++i)
    if(_T('\\') == buff[i] || _T('.') == buff[i])
      buff[i] = _T('_');
}
//----------------------------------------------------------------------------
#define ONLY_ONE
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE, //hPrevInstance,
                     LPSTR,     //lpCmdLine,
                     int       nCmdShow)
{
  if(!parseArg())
    return 0;

  _control87(MCW_EM,MCW_EM);

#ifdef ONLY_ONE
  commIdentity *cI = getIdentity();

  HANDLE hMutex = 0;
  do {
    TCHAR txtMutex[_MAX_PATH] = { 0 };
    GetModuleFileName(0, txtMutex, SIZE_A(txtMutex));
    replaceSlash(txtMutex, SIZE_A(txtMutex));
#if true    // possiamo usare lo stesso driver per più periferiche
    TCHAR idprph[64];
    wsprintf(idprph, _T("_%d"), cI->getNum());
    _tcscat_s(txtMutex, idprph);
#else
    _tcscat_s(txtMutex, cI->getClassName());
#endif
    hMutex = CreateMutex(0, true, txtMutex);
    } while(false);

  if(hMutex && ERROR_ALREADY_EXISTS == GetLastError()) {
    int i;
    for(i = 0; i < REPEAT_BEFORE_ABORT; ++i) {
      HWND hWnd = ::FindWindow(cI->getClassName(), NULL);
      if(hWnd) {
      // se è già avviata, la chiude
        ::SendMessage(hWnd, WM_SYSCOMMAND, SC_CLOSE, 0);
        break;
        }
      SleepEx(TIME_BETWEEN_REPEAT, false);
      }

    // se ha provato a chiuderla cicla per un po' aspettando
    // la chiusura effettiva
    if(REPEAT_BEFORE_ABORT != i) {
      for(i = 0; i < REPEAT_BEFORE_ABORT; ++i) {
        HWND hWnd = ::FindWindow(cI->getClassName(), NULL);
        if(!hWnd)
        // se si è chiusa
          break;
        }
      SleepEx(TIME_BETWEEN_REPEAT, false);
      }

    if(REPEAT_BEFORE_ABORT == i) {
      // se arriva qui significa che non ha trovato la window
      // oppure che non ha risposto in tempo utile e probabilmente
      // è bloccata
      HWND hWnd = ::FindWindow(cI->getClassName(), NULL);
      if(hWnd) {
        DWORD pId;
        GetWindowThreadProcessId(hWnd, &pId);
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0, pId);
        // kill it
        if(hProcess) {
          TerminateProcess(hProcess, 5);
          CloseHandle(hProcess);
          }
        }
      else {
        // ??? il mutex è stato aperto ma non rilasciato
        }
      }
    }
#endif

//  InitCommonControls();
/*
  if(nCmdShow != SW_HIDE)
    nCmdShow = (nCmdShow != SW_SHOWMINNOACTIVE) ?
//            SW_SHOWNORMAL :       // normale
            SW_SHOWMINNOACTIVE :  // o iconizzata
            nCmdShow;
*/
  gCommApp app(hInstance, nCmdShow);

  int result = app.run(_T(" "));

#ifdef ONLY_ONE
  if(hMutex) {
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
    }
#endif

  return result;
}
