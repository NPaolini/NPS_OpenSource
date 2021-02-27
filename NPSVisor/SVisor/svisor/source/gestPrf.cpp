//--------- gestprf.cpp ------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>

#include "mainClient.h"
#include "log_stat.h"
#include "gestPrf.h"
#include "d_wait.h"
#include "p_txt.h"
#include "def_dir.h"
#include "1.h"
//---------------------------------------------------------------------
static bool canDisableWindow = true;
//---------------------------------------------------------------------
bool setDisableWindowByTranfer(bool disable)
{
  bool old = canDisableWindow;
  canDisableWindow = disable;
  return old;
}
//---------------------------------------------------------------------
static void showNotReady(PWin *w)
{
  HWND old = ::GetFocus();
#if 1
  msgBoxByLangGlob(w, ID_MSG_PERIF_NOT_READY, ID_MSG_PERIF_NOT_READY_TITLE,  MB_ICONSTOP | MB_SETFOREGROUND | MB_TOPMOST);
#else
  svMessageBox(w, getStringOrId(ID_MSG_PERIF_NOT_READY),
            getStringOrId(ID_MSG_PERIF_NOT_READY_TITLE),
            MB_ICONSTOP | MB_SETFOREGROUND | MB_TOPMOST);
#endif
  if(old && IsWindow(old))
    ::SetFocus(old);
}
//---------------------------------------------------------------------
genericPerif::genericPerif(uint id_msg) : idMsg(id_msg), typeAddress(prfData::tNoData),
        onAutoComm(NOT_DEFINED), readOnly(false)
{
  LPCTSTR p = getString(ID_READ_ONLY_PROGRAM);
  readOnly = toBool(p && _ttoi(p));
}
//---------------------------------------------------------------------
#define DEF_DELAY_RESPONCE 5000
#define MAX_DELAY_RESPONCE 30000
#define MIN_DELAY_RESPONCE 1000
//#define MAX_DELAY_RESPONCE 10000
//---------------------------------------------------------------------
gestPerif::gestPerif(uint id_msg, mainClient* par) : genericPerif(id_msg), hWnd(0),
          Dirty(false), Responce(NO), Data(0), needRefresh(true),
          Par(par), Ready(false), inExec(false),
            maxDelay(DEF_DELAY_RESPONCE),
            requestVarPresent(true),

            idLast(0), hThread(0), HEvSend(CreateEvent(0, FALSE, FALSE, 0)),
            HEvClose(CreateEvent(0, TRUE, FALSE, 0))
{
  LPCTSTR p = getString(ID_MAX_DELAY_RESPONCE);
  if(p) {
    maxDelay = _ttol(p);
    if(maxDelay < MIN_DELAY_RESPONCE)
      maxDelay = MIN_DELAY_RESPONCE;
    else if(maxDelay > MAX_DELAY_RESPONCE)
      maxDelay = MAX_DELAY_RESPONCE;
    }
  DWORD idThread = 0;
  hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)DriverProc, this, 0, &idThread);
}
//---------------------------------------------------------------------
gestPerif::~gestPerif()
{
  if(HEvClose) {
    SetEvent(HEvClose);
    if(hThread) {
      WaitForSingleObject(hThread, INFINITE);
      CloseHandle(hThread);
      }
    if(HEvSend)
      CloseHandle(HEvSend);
    }
/*
  while(Queue) {
    sendQueue* p = Queue;
    Queue = p->next;
    delete p;
    }
*/
  closeApp();
}
//-------------------------------------------------------------
#define ERR_FAILED -1
#define ERR_TIMEOUT -2
static void showMsg(PWin* Par, eResponce responce, int err)
{
  int id = OK_WRITE == responce ? ID_MSG_ERR_WAIT_TX : ID_MSG_ERR_WAIT_RX;
  int id2 = err == ERR_TIMEOUT ? ID_MSG_ERR_WAIT_TIMEOUT : ID_MSG_ERR_WAIT_FAILED;
  TCHAR buff[400];
#if 1
  smartPointerConstString msg1(getStringOrIdByLangGlob(id2));
  smartPointerConstString msg2(getStringOrIdByLangGlob(id));

  smartPointerConstString tit(getStringOrIdByLangGlob(ID_MSG_ERR_TITLE));

  wsprintf(buff, _T("%s %s"), &msg1, &msg2);
  svMessageBox(Par, buff, &tit, MB_ICONSTOP);
#else
  wsprintf(buff, _T("%s %s"), getStringOrId(id2), getStringOrId(id));
  svMessageBox(Par, buff, getStringOrId(ID_MSG_ERR_TITLE),
            MB_ICONSTOP);
#endif
}
//---------------------------------------------------------------------
#define NO_WAIT_ASYNC
//---------------------------------------------------------------------
bool gestPerif::performSendCustom(WORD msg, howSend how, WORD hiP1, DWORD& p2, WORD desiredResponce) const
{
  if(isReadOnly() || !Ready || !IsWindow(hWnd)) {
    if(CHECK_READY & how)
      showNotReady(Par);
    return false;
    }
  bool result = false;
  if(hWnd) {
    if(WAIT_RESULT & how) {
      if(IsWindow(hWnd)) {
        DWORD_PTR sndresult;
        p2 = ::SendMessageTimeout(hWnd, WM_MAIN_PRG, MAKEWPARAM(msg, hiP1), p2,
                    SMTO_NORMAL, maxDelay, &sndresult);
        if(p2) {
          p2 = sndresult;
          result = true;
          }
        else {
          int err = ERR_TIMEOUT;
          if(GetLastError())
            err = ERR_FAILED;
          showMsg(Par, (eResponce)desiredResponce, err);
          }
        }
      }
    else if(WAIT_ASYNC & how) {
      ::PostMessage(hWnd, WM_MAIN_PRG, MAKEWPARAM(msg, hiP1), p2);
#ifdef NO_WAIT_ASYNC
      result = true;
#else
      result = toBool(waitFor(eResponce(desiredResponce), toBool(SHOW_WAIT & how)));
#endif
      }
    else if(NO_CHECK == how || CHECK_READY == how) {
      ::PostMessage(hWnd, WM_MAIN_PRG, MAKEWPARAM(msg, hiP1), p2);
      result = true;
      }
    }
  return result;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool gestPerif::SendCustom(WORD msg, howSend how, WORD hiP1, DWORD& p2, WORD desiredResponce) const
{
#ifdef NO_WAIT_ASYNC
  bool needDisable = toBool((WAIT_RESULT) & how);
#else
  bool needDisable = toBool((WAIT_RESULT | WAIT_ASYNC) & how);
#endif

  if(needDisable)
    Par->disableFunctionKey();
  bool result = performSendCustom(msg, how, hiP1, p2, desiredResponce);
  if(needDisable)
    Par->enableFunctionKey();
  return result;
}
//---------------------------------------------------------------------
#define MAX_COUNT_RESEND 20
//---------------------------------------------------------------------
struct infoSend
{
  union {
    struct {
      WORD id;
      WORD count;
      };
    DWORD dw;
    } U;
  infoSend() {  U.dw = 0; }
  explicit infoSend(WORD id) { U.id = id; U.count = 0; }
  explicit infoSend(DWORD id) {  U.dw = id; }
  operator DWORD&() { return U.dw; }
};
//---------------------------
WORD gestPerif::getNewId()
{
  criticalLock cl(CS);
  if(IdSent.getFree() && Id2Send.getFree()) {
    if(!++idLast)
      idLast = 1;
    return idLast;
    }
  return 0;
}
//---------------------------
bool gestPerif::SendHide(const void *buff, int len)
{
  return Send(buff, len, false);
}
//---------------------------
bool gestPerif::Send(const void *buff, int len, bool showWait)
{
  if(isReadOnly() || !Ready || !IsWindow(hWnd))
    return false;
  WORD id = getNewId();
  if(!id)
    return false;
  if(makeDataToSend(buff, len, id)) {
    criticalLock cl(CS);
    infoSend is(id);
    Id2Send.Push(is);
    SetEvent(HEvSend);
    return true;
    }
  return false;
}
//-----------------------------------------------------
void gestPerif::makefilenamecommand(LPTSTR path, uint id)
{
  _tcscpy_s(path, _MAX_PATH, getLocalBaseDir());
  int l = _tcslen(path);

  MK_NAME_FILE_COMMAND(path + l, getId(), id);
}
//-----------------------------------------------------
void gestPerif::makefilenamecommandQueue(LPTSTR path)
{
  _tcscpy_s(path, _MAX_PATH, getLocalBaseDir());
  int l = _tcslen(path);

  MK_NAME_FILE_COMMAND_QUEUE(path + l, getId());
}
//---------------------------
bool gestPerif::thPerformSend(DWORD id) const
{
  if(isReadOnly() || !Ready || !IsWindow(hWnd))
    return false;

  ::PostMessage(hWnd, WM_MAIN_PRG, MAKEWPARAM(MSG_WRITE_CUSTOM, NO_CHECK), id);
  return true;
}
//---------------------------
// evento
void gestPerif::thEvent()
{
  criticalLock cl(CS);
  if(!Id2Send.getStored())
    return;
  infoSend id = infoSend(Id2Send.Pop());
  if(thPerformSend(id.U.id))
    IdSent.Push(id);
  else
    Id2Send.Push(id);
}
//---------------------------
// timeout
void gestPerif::thTimeout()
{
  queueId IdTmp;
  while(true) {
    DWORD id;
    {
    criticalLock cl(CS);
    id = IdSent.Pop();
    }
    if(!id)
      break;
    infoSend is(id);
    TCHAR command[_MAX_PATH];
    makefilenamecommand(command, is.U.id);
    WIN32_FIND_DATA fd;

    HANDLE hfff = FindFirstFile(command, &fd);
    if(INVALID_HANDLE_VALUE != hfff) {
      FindClose(hfff);
      IdTmp.Push(id);
      }
    }
  criticalLock cl(CS);
  while(IdTmp.getStored()) {
    DWORD id = IdTmp.Pop();
    infoSend is(id);
    if(++is.U.count >= MAX_COUNT_RESEND) {
      thPerformSend(is.U.id);
      is.U.count = 0;
      }
    IdSent.Push(is);
    }
}
//---------------------------
// all'avvio
void gestPerif::thStart()
{
  TCHAR path[_MAX_PATH];
  makefilenamecommandQueue(path);
  idLast = 0;
  if(P_File::P_exist(path)) {
    P_File pf(path, P_READ_ONLY);
    if(pf.P_open()) {
    uint len = (uint)(pf.get_len() / sizeof(idLast));
      criticalLock cl(CS);
      for(uint i = 0; i < len; ++i) {
        WORD t;
        pf.P_read(&t, sizeof(t));
        infoSend is(t);
        if(idLast < is.U.id)
          idLast = is.U.id;
        Id2Send.Push(is);
        }
      pf.P_close();
      }
    DeleteFile(path);
    SetEvent(HEvSend);
    }
}
//---------------------------
static
void thSaveQueue(queueId& q, P_File& pf)
{
  while(q.getStored()) {
    DWORD t = q.Pop();
    if(t) {
      infoSend is(t);
      pf.P_write(&is.U.id, sizeof(is.U.id));
      }
    }
}
//---------------------------
// alla chiusura
void gestPerif::thEnd()
{
  TCHAR path[_MAX_PATH];
  makefilenamecommandQueue(path);
  DeleteFile(path);
  criticalLock cl(CS);
  if(IdSent.getStored() || Id2Send.getStored()) {
    P_File pf(path, P_CREAT);
    if(pf.P_open()) {
      thSaveQueue(IdSent, pf);
      thSaveQueue(Id2Send, pf);
      }
    }
}
//---------------------------

//------------------------------------------------------------------
unsigned FAR PASCAL DriverProc(void* cl)
{
  gestPerif *owner = reinterpret_cast<gestPerif*>(cl);
  owner->thStart();
  Sleep(100);
  HANDLE HandlesToWaitFor[] = { owner->HEvSend, owner->HEvClose };

  const DWORD timeout = 100;
  bool terminate = false;
  while(!terminate) {
    DWORD result = WaitForMultipleObjects(SIZE_A(HandlesToWaitFor), HandlesToWaitFor, FALSE, timeout);
    switch(result) {
      case WAIT_OBJECT_0:
        owner->thEvent();
        break;
      case WAIT_TIMEOUT:
        owner->thTimeout();
        // nel caso ci siano richieste non evase ...
        owner->thEvent();
        break;
      default:
        terminate = true;
        break;
      }
    }
  owner->thEnd();
  return 0;
}
//---------------------------------------------------------------------
bool gestPerif::Receive(int addr_init, int addr_end, WORD flag) const
{
  DWORD lParam = MAKELPARAM(addr_init, addr_end);
  bool result = SendCustom(MSG_READ_CUSTOM, ALL_AND_SHOW, flag, lParam, OK_READ);
  Data = lParam;
  needRefresh = true;
  return result;
}
//---------------------------------------------------------------------
bool gestPerif::ReceiveHide(int addr_init, int addr_end) const
{
  DWORD lParam = MAKELPARAM(addr_init, addr_end);
  bool result = SendCustom(MSG_READ_CUSTOM, NO_CHECK, 0, lParam, NO);
  Data = lParam;
  needRefresh = true;
  return result;
}
//---------------------------------------------------------------------
bool gestPerif::waitFor(eResponce r, bool showWait) const
{
  int ret = 0;
  PAppl *app = getAppl();

  DWORD tick = GetTickCount();
  bool wantMore;
  while(inExec && Par->getHandle()) {
    app->pumpOnlyOneMessage(wantMore);
    if(GetTickCount() - tick > maxDelay)
      break;
    Sleep(10);
    }
  if(inExec) {
    // se chiusura applicazione, non deve far apparire alcun messaggio
    if(!Par->getHandle())
      return true;
    // altrimenti è bloccato da una richiesta pendente non
    // ancora terminata (???)
    return false;
    }
  inExec = true;
  HWND old = ::GetFocus();
  TD_Wait *wait = 0;
  PVect<PWin*> toDisable;
  do {
    if(showWait) {
      smartPointerConstString tit(getStringOrIdByLangGlob(OK_WRITE == r ? ID_MSG_WAIT_TX : ID_MSG_WAIT_RX));
      wait = new TD_Wait(&tit, Par);
      wait->create();
      PWin* pw = getCurrBody();
      int i = 0;
      if(!canDisableWindow)
        ::SetFocus(old);
      while(canDisableWindow && pw) {
        toDisable[i] = pw;
        ++i;
        EnableWindow(*pw, false);
        if(dynamic_cast<mainClient*>(pw))
          break;
        pw = pw->getParent();
        }
      }
    tick = GetTickCount();
    Responce = FAILED + 1;
    do {
      app->pumpOnlyOneMessage(wantMore);
      WORD rsp = Responce;
      if(rsp != FAILED + 1) {
        ret = rsp == FAILED ? ERR_FAILED : 1;
        break;
        }
      if(!Par->getHandle()) // potrebbe essere terminato il programma
        ret = 1;
      if(GetTickCount() - tick > maxDelay)
        ret = ERR_TIMEOUT;
      Sleep(10);
      } while(!ret);
    } while(0);
  int nElem = toDisable.getElem();
  for(int j = nElem - 1; j >= 0; --j)
    EnableWindow(*toDisable[j], true);

  delete wait;
  if(ret < 0 && showWait)
    showMsg(Par, r, ret);
/**/
  if(wait) {
    if(IsWindow(old))
      ::SetFocus(old);
    else
      ::SetFocus(*getCurrBody());
    }
/**/
  inExec = false;
  return ret > 0;
}
//-------------------------------------------------------------
#define SEND_MSG ::PostMessage
//#define SEND_MSG ::SendMessage
void gestPerif::closeApp()
{
  if(hWnd && IsWindow(hWnd)) {
    SEND_MSG(hWnd, WM_MAIN_PRG, MAKEWPARAM(MSG_MAIN_IS_CLOSING, 0), 0);
//    hWnd = 0;
    }
  hWnd = 0;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
WORD gestPerif::LtoP_Init(int addr) const
{
  switch(typeAddress) {
    case prfData::tBData:   // 8 bit
    case prfData::tBsData:   // 8 bit
      return (WORD)addr;

    case prfData::tWData:   // 16 bit
    case prfData::tWsData:   // 16 bit
      return (WORD)(addr << 1);

    case prfData::tFRData:  // 32 bit
    case prfData::tDWData:  // 32 bit
    case prfData::tDWsData:  // 32 bit
      return (WORD)(addr << 2);

    case prfData::ti64Data:
    case prfData::tRData:
    case prfData::tDateHour:
    case prfData::tDate:
    case prfData::tHour:
      return (WORD)(addr << 3);
    }
  return 0;
}
//---------------------------------------------------------------------
WORD gestPerif::LtoP_End(int addr)  const
{
  return (WORD)(LtoP_Init(addr + 1) - 1);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
//---------------------------------------------------------------------
perifsSet::perifsSet() : Head(0), Curr(0) { }
//-----------------------------------------------------------
perifsSet::~perifsSet()
{
  removeAll();
}
//-----------------------------------------------------------
void perifsSet::remove(listPerif* curr)
{
  if(!curr)
    return;
  remove(curr->next);
  if(curr->autodelete)
    delete curr->perif;
  delete curr;

}
//-----------------------------------------------------------
bool perifsSet::setFirst()
{
  Curr = Head;
  return Curr != 0;
}
//-----------------------------------------------------------
bool perifsSet::setNext()
{
  if(!Curr)
    Curr = Head;
  if(!Curr || !Curr->next)
    return false;
  Curr = Curr->next;
  return true;
}
//-----------------------------------------------------------
void perifsSet::add(gestPerif *p, bool autodelete)
{
  listPerif *tmp = new listPerif;
  tmp->next = Head;
  tmp->perif = p;
  tmp->autodelete = autodelete;
  Head = tmp;
}
//-----------------------------------------------------------
gestPerif* perifsSet::get(uint id) const
{
  listPerif *tmp = Head;
  while(tmp) {
    gestPerif *p = tmp->perif;
    if(p->getId() == id)
      return p;
    tmp = tmp->next;
    }
  return 0;
}
//-----------------------------------------------------------
gestPerif* perifsSet::getByMsg(uint msg) const
{
  listPerif *tmp = Head;
  while(tmp) {
    gestPerif *p = tmp->perif;
    if(p->acceptMsgId(msg))
      return p;
    tmp = tmp->next;
    }
  return 0;
}
//-----------------------------------------------------------
void perifsSet::refresh()
{
  listPerif *tmp = Head;
  while(tmp) {
    gestPerif *p = tmp->perif;
    p->refresh();
    tmp = tmp->next;
    }
}
//-----------------------------------------------------------
bool perifsSet::isDirty() const
{
  listPerif *tmp = Head;
  while(tmp) {
    gestPerif *p = tmp->perif;
    if(p->isDirty())
      return true;
    tmp = tmp->next;
    }
  return false;
}
//-----------------------------------------------------------
bool perifsSet::isReady() const
{
  if(!Head)
    return false;
  listPerif *tmp = Head;
  bool ready = true;
  while(tmp) {
    gestPerif *p = tmp->perif;
    ready &= p->isReady();
    tmp = tmp->next;
    }
  return ready;
}
//-----------------------------------------------------------
bool perifsSet::isDirty(uint id) const
{
  const gestPerif *tmp = get(id);
  if(tmp)
    return tmp->isDirty();
  return false;
}
//-----------------------------------------------------------
void perifsSet::commit()
{
  listPerif *tmp = Head;
  while(tmp) {
    gestPerif *p = tmp->perif;
    p->commit();
    p->commitAlarm();
    tmp = tmp->next;
    }
}
//-----------------------------------------------------------
gestPerif* perifsSet::getCurr() const
{
  if(!Curr)
    return 0;
  return Curr->perif;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
void perifsSet::closeApp()
{
  listPerif *tmp = Head;
  while(tmp) {
    gestPerif *p = tmp->perif;
    p->closeApp();
    tmp = tmp->next;
    }
}
//-----------------------------------------------------------
void perifsSet::removeAll()
{
  remove(Head);
  Curr = Head = 0;
}
//-----------------------------------------------------------
gestPerif::statAlarm perifsSet::hasAlarm() const
{
  listPerif *tmp = Head;
  gestPerif::statAlarm ret = gestPerif::sNoAlarm;
  while(tmp) {
    const gestPerif *p = tmp->perif;
    gestPerif::statAlarm t = p->hasAlarm();
    if(t > ret) {
      ret = t;
      // priorità più alta, inutile proseguire
      if(gestPerif::sAlarm == ret)
        break;
      }
    tmp = tmp->next;
    }
  return ret;
}
//-----------------------------------------------------------
void perifsSet::logAlarm(log_status &log, int wich, bool force)
{
  listPerif *tmp = Head;
  while(tmp) {
    gestPerif *p = tmp->perif;
#if 0
    // la periferica deve controllare se è realmente in allarme
    // quindi è inutile preverificarlo
    gestPerif::statAlarm t = p->hasAlarm();
    if(gestPerif::sNoAlarm != t)
#endif
      p->logAlarm(log, wich, force);
    tmp = tmp->next;
    }
}
//-----------------------------------------------------------
void perifsSet::resetAlarm()
{
  listPerif *tmp = Head;
  while(tmp) {
    gestPerif *p = tmp->perif;
    p->sendResetAlarm();
    tmp = tmp->next;
    }
}
//-----------------------------------------------------------
void perifsSet::sendHideShow(bool hide)
{
  listPerif *tmp = Head;
  while(tmp) {
    gestPerif *p = tmp->perif;
    HWND hWnd = p->getHwndDriver();
    if(hWnd && IsWindow(hWnd))
      SEND_MSG(hWnd, WM_MAIN_PRG, MAKEWPARAM(MSG_HIDE_SHOW_DRIVER, hide), 0);
    tmp = tmp->next;
    }
}
//-----------------------------------------------------------
