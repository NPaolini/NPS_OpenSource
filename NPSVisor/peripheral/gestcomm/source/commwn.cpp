//------------ commwn.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "commidt.h"
#include "commwn.h"

#include <stdio.h>

#include "hdrmsg.h"
#include "prect.h"

#include "PServerDriver.h"
//----------------------------------------------------------------------------
#define ID_BUTTON 100

#define _REG(a) const UINT WM_##a = RegisterWindowMessage(_T("WM_") _T(#a))
_REG(MAIN_PRG);
_REG(CUSTOM);
//----------------------------------------------------------------------------
gestCommWindow::gestCommWindow(LPCTSTR title, HINSTANCE hinst)
:
  PMainWin(title, hinst), countRec(0), countSend(0), countTot(0), gComm(0),
    toggled(false), LastErr(gestComm::OpenComErr), MustClose(0), idTimer(0),
    Started(false), forceDirtyOnReceive(0), canManageBit(2), w_active(0),
    consecutiveError(0), repeatWaitOnInitError(0), firstShow(true)


{
    Reset = new PButton(this, ID_BUTTON, 10, 100, 80, 30, _T("&Stop"), hinst);
}
//----------------------------------------------------------------------------
gestCommWindow::~gestCommWindow()
{
  destroy();
}
//----------------------------------------------------------------------------
void gestCommWindow::Destroy(HWND hwnd)
{
  static bool ok = false;
  if(!ok) {
    ok = true;
    if(idTimer) {
      KillTimer(hwnd, idTimer);
      idTimer = 0;
      }
    UINT p;

    if(!MustClose)
      p = MSG_I_AM_CLOSING;
    else if(MustClose < 0)
      p = MSG_INIT_ERR;
    else
      p = MSG_OK_CLOSING;

    commIdentity *cI = getIdentity();
    cI->sendMessage(hwnd, cI->get_WM(),
                  MAKEWPARAM(MSG_CLOSING, p), (LPARAM)hwnd, SEND_TO_ALL, true);

//    if(!toggled)
//      toggle(ID_BUTTON);
    if(gComm)
      delete gComm;
    gComm = 0;
    }
}
//----------------------------------------------------------------------------
LPCTSTR gestCommWindow::getClassName() const
{
  commIdentity *cI = getIdentity();
  return cI->getClassName();
}
//----------------------------------------------------------------------------
#define SHOW_ON_BAR
//----------------------------------------------------------------------------
bool gestCommWindow::create()
{
  Attr.style &= ~WS_SYSMENU;
  Attr.exStyle = WS_EX_TOOLWINDOW | // rende la barra più piccola
#ifdef SHOW_ON_BAR
                 // il flag precedente non fa apparire l'icona sulla
                 // barra dei task, va forzata con questo
                  WS_EX_APPWINDOW;
#else
                  0;
#endif
  if(!PMainWin::create())
    return false;
  HICON icon = LoadIcon(getHInstance(), MAKEINTRESOURCE(1));
  if(icon) {
    SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_BIG), LPARAM(icon));
    SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_SMALL), LPARAM(icon));
    }
  commIdentity *cI = getIdentity();
  ::SendMessage(cI->getHSuperVisor(), cI->get_WM(), MAKEWPARAM(MSG_HWND, 0),
                  reinterpret_cast<LPARAM>(getHandle()));
  if(useTimer())
    SetTimer(getHandle(), idTimer = 1, getTimerTick(), 0);
  else
    processCommEvent();
  PRect r;
  GetWindowRect(cI->getHSuperVisor(), r);
  uint offs = cI->getNum() * 10;
  Attr.x = r.left + offs;
  Attr.y = r.top + offs;
  return true;
}
//----------------------------------------------------------------------------
uint gestCommWindow::getTimerTick() const
{
  if(gComm)
    return gComm->getTimerTick();
  return 200;
}
//----------------------------------------------------------------------------
#define MAX_ERR 20
//static PRect invalid(10,10,200,100);
#define SECOND_TO_WAIT 20
//----------------------------------------------------------------------------
bool gestCommWindow::init_gComm()
{
  if(repeatWaitOnInitError) {
    if(--repeatWaitOnInitError)
      return false;
    }
  Started = false;
  if(!getHandle() || MustClose)
    return false;
  commIdentity *cI = getIdentity();
  if(cI->needHide())
    ShowWindow(*this, SW_HIDE);
  if(gComm)
    delete gComm;
  LastErr = gestComm::OpenComErr;
  gComm = cI->getGestComm(this);
  if(!gComm)
    return false;
  gestComm::ComResult result = gComm->Init();
  if(gestComm::NoErr <= result) {
    // invia l'ok alla comunicazione
    commIdentity *cI = getIdentity();
    ::PostMessage(cI->getHSuperVisor(), cI->get_WM(), MAKEWPARAM(MSG_IS_READY, 1), SEND_INFO_FLAGS);
    Started = true;

    if(idTimer) {
      KillTimer(getHandle(), idTimer);
      idTimer = 0;
      }
    if(useTimer())
      SetTimer(getHandle(), idTimer = 1, getTimerTick(), 0);
    repeatWaitOnInitError = 0;
    return true;
    }
  delete gComm;
  gComm = 0;
  if(gestComm::OpenComErr == result || gestComm::OpenFileErr == result) {
    if(!repeatWaitOnInitError)
      repeatWaitOnInitError = (SECOND_TO_WAIT * 1000) / getTimerTick();
//    CloseApp(-1);
//    return false;
    }
  LastErr = result;
  ++countTot;
  InvalidateRect(getHandle(), 0, false);
//  InvalidateRect(getHandle(), invalid, false);
  return false;
}
//----------------------------------------------------------------------------
gestComm::ComResult gestCommWindow::action(gestComm::ComResult last, bool setCount)
{
  if(MustClose) // viene controllato prima di entrare qui || !gComm)
    return gestComm::NoErr;
  commIdentity *cI = getIdentity();
  switch(LastErr) {
    case gestComm::NoErr:
    case gestComm::OpenComErr:
    case gestComm::OpenFileErr:
    case gestComm::WriteFileErr:
    case gestComm::ReadFileErr:
    case gestComm::noAction:
      break;
    case gestComm::SendErr:
      gComm->reset();
      if(setCount)
        ++countSend;
      break;
    case gestComm::TimeOut:
    case gestComm::RecErr:
      gComm->reset();
      if(setCount)
        ++countRec;
      break;
    case gestComm::doResponce:
      do {
      // vedere specifiche nell'header mycom_bs.h
        WORD resp = gComm->getLastsResponce();
        last = (gestComm::ComResult)HIGH_SRESP(resp);
        resp = LOW_SRESP(resp);
        // se necessita di una risposta
        if(NO != resp)
          cI->sendMessage(*this, cI->get_WM(), MAKEWPARAM(MSG_RESPONCE, resp),
                gComm->getLastData(), gComm->getLastIdReq(), true);
        } while(0);
      break;
    case gestComm::HasData:
      do {
        DWORD data = gComm->getLastData();
        // in caso di comunicazione diretta usa il svisor sulla
        // stessa macchina
        ::PostMessage(cI->getHSuperVisor(), cI->get_WM(),
            MAKEWPARAM(MSG_INIT_HAS_DATA, gComm->getLastsResponce()), data);
        last = gComm->receive();
        ::PostMessage(cI->getHSuperVisor(), cI->get_WM(),
            MAKEWPARAM(MSG_HAS_DATA, 0), last >= gestComm::NoErr);
      } while(false);
      break;
    case gestComm::ReqData:
      do {
        DWORD data = gComm->getLastData();
        // in caso di comunicazione diretta usa il svisor sulla
        // stessa macchina
        DWORD result = ::SendMessage(cI->getHSuperVisor(), cI->get_WM(),
                MAKEWPARAM(MSG_REQ_DATA, gComm->getLastsResponce()), data);
      // lasciamo che sia la periferica a decidere
        last = gComm->send(result);
        ::PostMessage(cI->getHSuperVisor(), cI->get_WM(),
                MAKEWPARAM(MSG_END_REQ_DATA, 0), last >= gestComm::NoErr);
        } while(false);
      break;
    }
  return last;
}
//----------------------------------------------------------------------------
// occorre controllare ogni volta che gComm sia ancora valido,
// se si elaborano messaggi durante una qualsiasi attesa,
// potrebbe essere stato distrutto
void gestCommWindow::processCommEvent()
{
    static bool inExec;
    if(!getHandle() || MustClose) {
      inExec = false;
      return;
      }
    if(inExec)
      return;
    inExec = true;
    if(gComm) {
      bool needMore = true;
#if 1
      int count = (int)getMoreCycleCount();
      while(count-- > 0 && needMore) {
#else
      int count = 0;
      while(needMore) {
        if(count++ > (int)getMoreCycleCount())
          break;
#endif
        if(!gComm)
          break;
        // notifica al driver della periferica di non inviare altre notifiche
        // di dati pronti
        bool last = gComm->reqNotify(false);
        int totCount = countRec + countSend;
        do {
          gComm->resetDirty();
          LastErr = beforeRun();
          if(!gComm)
            break;

          if(gestComm::NoErr <= LastErr)
            LastErr = gComm->SendCommand();
          if(!gComm)
            break;
          LastErr = action(LastErr);

          LastErr = gComm->ConfirmCommand(LastErr);
          if(!gComm)
            break;
          LastErr = action(LastErr);

          LastErr = gComm->DoData(LastErr);
          if(!gComm)
            break;
          LastErr = action(LastErr);

          LastErr = gComm->write(LastErr);
          if(!gComm)
            break;
          LastErr = action(LastErr, true);

          LastErr = afterRun(LastErr);
          if(!gComm)
            break;

          // potrebbero arrivare troppi messaggi al svisor che lo ingolferebbero.
          // Ora il svisor effettua una riletura ogni ciclo senza verificare
          // la necessità di rileggerli.
          // Nuova modifica.
          // Viene attivata solo se la periferica necessita di aggiornamento forzato
          if(forceDirtyOnReceive && gComm->isDirty()) {
            commIdentity *cI = getIdentity();
            cI->sendMessage(*this, cI->get_WM(),
                  MAKEWPARAM(MSG_CHANGED_DATA, makeHiWordTParam1()),
                  makeTParam2(), SEND_TO_ALL, true);
            }

          } while(false);

        if(!getHandle() || MustClose) {
          if(gComm)
            gComm->reqNotify(last);
          inExec = false;
          return;
          }
        switch(LastErr) {
          case gestComm::needMoreCycle:
          case gestComm::noAction:
          case gestComm::noActionButSave:
            break;
          default:
            ++countTot;
            break;
          }
//        if(gComm->needSendResponde())
//          needMore = false;

        // ripristina il valore precedente di abilitazione alla notifica
        if(gComm)
          gComm->reqNotify(last);
        if(needMore) {
          bool wantMore;
          getAppl()->pumpOnlyOneMessage(wantMore);
          }
#define _RESET_
#ifdef _RESET_
        bool manCons = false;
        if(gComm)
          manCons = gComm->manageCounter(countSend, countRec, countTot, consecutiveError);
        if(!manCons) {
          if((countRec + countSend) > totCount)
            ++consecutiveError;
          }
        if(consecutiveError > MAX_ERR) {
          PostMessage(*this, WM_COMMAND, MAKEWPARAM(ID_BUTTON, 0), 0);
          PostMessage(*this, WM_COMMAND, MAKEWPARAM(ID_BUTTON, 0), 0);
          consecutiveError = 0;
          break;
          }
        else
          switch(LastErr) {
            case gestComm::needMoreCycle:
            case gestComm::noAction:
            case gestComm::noActionButSave:
              break;
            default:
              if(gestComm::NoErr <= LastErr)
                consecutiveError /= 2;
              break;
            }
//        else if(gestComm::noAction != LastErr && gestComm::NoErr <= LastErr)
//          consecutiveError /= 2;
#endif
        }
      InvalidateRect(getHandle(), 0, false);
      if(w_active)
        UpdateWindow(*this);
      }
    else if(!toggled)
      init_gComm();
    inExec = false;
}
//----------------------------------------------------------------------------
void gestCommWindow::Paint(HDC hdc)
{
  TCHAR msg[300] = _T("Result = ");
  switch(LastErr) {
    case gestComm::NoErr:
      _tcscat_s(msg, _T("NoErr"));
      break;
    case gestComm::OpenComErr:
      // se si verifica un errore nell'apertura si abortisce,
      // più probabilmente si tratta di un errore di inizializzazione
      _tcscat_s(msg, _T("InitComErr"));
      break;
    case gestComm::OpenFileErr:
      _tcscat_s(msg, _T("OpenFileErr"));
      break;
    case gestComm::WriteFileErr:
      _tcscat_s(msg, _T("WriteFileErr"));
      break;
    case gestComm::ReadFileErr:
      _tcscat_s(msg, _T("ReadFileErr"));
      break;
    case gestComm::SendErr:
      _tcscat_s(msg, _T("SendErr"));
      break;
    case gestComm::RecErr:
      _tcscat_s(msg, _T("RecErr"));
      break;
    }

  if(gComm)
    gComm->manageCounter(countSend, countRec, countTot, consecutiveError);

  int l = _tcslen(msg);
  wsprintf(msg + l, _T("\r\n\tSendErr = %d\r\n\tRecErr = %d\r\n\tTotCount = %d\r\nConsErr= %d"),
                countSend, countRec, countTot, consecutiveError);

  int old = SetBkMode(hdc, OPAQUE);
  PRect r;
  GetClientRect(*this, r);
  r.Inflate(-10, -10);
  DrawText(hdc, msg, -1, r, DT_WORDBREAK | DT_LEFT |DT_EXPANDTABS);
  SetBkMode(hdc, old);

}
//----------------------------------------------------------------------------
void gestCommWindow::toggle(UINT id)
{
  if(ID_BUTTON == id) {
    // per evitare altri click durante l'elaborazione
    EnableWindow(*Reset, false);
    toggled = !toggled;
    if(toggled) {
      if(gComm) {
        // finge una chiusura dell'applicazione per terminare rapidamente
        // la gestione della periferica
        commIdentity *cI = getIdentity();
        ::PostMessage(cI->getHSuperVisor(), cI->get_WM(), MAKEWPARAM(MSG_IS_READY, 0), SEND_INFO_FLAGS);
        Started = false;
        if(idTimer) {
          KillTimer(getHandle(), idTimer);
          idTimer = 0;
          }
        MustClose = 1;
        gComm->MustClose();
        gestComm *t = gComm;
        gComm = 0;
/*
        for(int i = 0; i < 3; ++i) {
          bool more;
          getAppl()->pumpOnlyOneMessage(more);
          if(!more)
            break;
//          getAppl()->pumpMessages();
          }
*/
        delete t;
        MustClose = 0;
        }
      SendMessage(*Reset, WM_SETTEXT, 0, (LPARAM)_T("&Start"));
      }
    else {
      repeatWaitOnInitError = 0;
      if(!init_gComm()) {
        if(gComm) {
          delete gComm;
          gComm = 0;
          }
        }
      SendMessage(*Reset, WM_SETTEXT, 0, (LPARAM)_T("&Stop"));
      if(useTimer() && !idTimer)
        SetTimer(getHandle(), idTimer = 1, getTimerTick(), 0);
      }
    EnableWindow(*Reset, true);
    countTot = 0;
    countRec = 0;
    countSend = 0;
    InvalidateRect(getHandle(), 0, true);
    }
}
//----------------------------------------------------------------------------
void gestCommWindow::CloseApp(int val)
{
    MustClose = val;
    Started = false;
    if(idTimer) {
      KillTimer(*this, idTimer);
      idTimer = 0;
      }

    if(gComm)
      gComm->MustClose();
      // aspettiamo per un po' che tutto torni nella normalità
//    for(int i = 0; i < 5; ++i)
//      getAppl()->pumpMessages();
    PostQuitMessage(0);
}
//----------------------------------------------------------------------------
LRESULT gestCommWindow::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  if(WM_MAIN_PRG == message) {
    if(MSG_MAIN_IS_CLOSING == LOWORD(wParam)) {
      CloseApp(1);
      return true; //result;
      }
    if(!gComm)
      ;
    else if(Started)
      return processSVisorMsg(hwnd, message, wParam, lParam);
    }
  else {
    switch(message) {

      case WM_COMMAND:
        toggle(LOWORD(wParam));
        break;

      case WM_ACTIVATEAPP:
        w_active = toBool(wParam);
        if(w_active)
          UpdateWindow(*this);
        break;

      case WM_TIMER:
        if(1 == wParam)
          processCommEvent();
        break;

      case WM_ERASEBKGND:
        return 1;

      case WM_PAINT:

        if(!GetUpdateRect(hwnd, 0, 0))
          break;
        do {
          PAINTSTRUCT ps;
          HDC hdc = BeginPaint(hwnd, &ps);

#define USE_MEMDC
#ifdef USE_MEMDC

#define DEF_COLOR_BKG GetSysColor(COLOR_BTNFACE)
          PRect r;
          GetClientRect(hwnd, r);
          HBITMAP hBmpTmp = CreateCompatibleBitmap(hdc, r.Width(), r.Height());

          HDC mdc = CreateCompatibleDC(hdc);
          HGDIOBJ oldObj = SelectObject(mdc, hBmpTmp);
#if 1
          HBRUSH br = (HBRUSH) GetStockObject(WHITE_BRUSH);
          FillRect(mdc, r, br);
#else
          HBRUSH br = CreateSolidBrush(DEF_COLOR_BKG);
          FillRect(mdc, r, br);
          DeleteObject(HGDIOBJ(br));
#endif
          Paint(mdc);
          BitBlt(hdc, 0, 0, r.Width(), r.Height(), mdc, 0, 0, SRCCOPY);
          SelectObject(mdc, oldObj);

          DeleteDC(mdc);

          DeleteObject(hBmpTmp);
#else
          Paint(hdc);
#endif
          EndPaint(hwnd, &ps);
          if(firstShow) {
            firstShow = false;
            SetWindowPos(*this, 0, Attr.x, Attr.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
            }
          } while(false);
        return 0;
//        break;

      case ID_MSG_HAS_CHAR:
        if(useDualMode() || !useTimer()) {
          processCommEvent();
          return true;
          }
        break;

      case WM_COPYDATA:
        return processSVisorMsg(hwnd, message, wParam, lParam);

      case WM_DESTROY:
        Destroy(hwnd);
        break;
      }
    }
  return PMainWin::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void gestCommWindow::getWindowClass(WNDCLASS& wcl)
{
  PMainWin::getWindowClass(wcl);
  wcl.hbrBackground  = (HBRUSH) GetStockObject(WHITE_BRUSH);
}
