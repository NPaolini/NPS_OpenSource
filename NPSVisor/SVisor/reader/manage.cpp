//------ manage.cpp ----------------------------------------
//----------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------
#include <stdlib.h>

#include "manage.h"
#include "1.h"
#include "def_dir.h"
#include "p_date.h"
#include "perif.h"
#include "gestdata.h"
#include "svDialog.h"
#include "PListBox.h"
//----------------------------------------------------------
#define NAME_CODE_OPER _T("codeOper")
//----------------------------------------------------------
#ifdef UNICODE
//----------------------------------------------------------
struct dualAltern
{
 codeType code;
 char name[MAX_NAME_OPER];
};
//----------------------------------------------------------
#else
//----------------------------------------------------------
struct dualAltern
{
 codeType code;
 WCHAR name[MAX_NAME_OPER];
};
//----------------------------------------------------------
#endif
//----------------------------------------------------------
static void copyStruct(codeOper::dual& target, const dualAltern& source)
{
  target.code = source.code;
  copyStrZ(target.name, source.name, SIZE_A(target.name) - 1);
}
//----------------------------------------------------------
//----------------------------------------------------------
void codeOper::load()
{
  TCHAR name[_MAX_PATH] = NAME_CODE_OPER;
  makePath(name, dSystem, false);

  P_File f(name, P_READ_ONLY);
  if(f.P_open()) {
    DWORD len = (DWORD)f.get_len();
    // se si includerà un header occorrerà sottrarlo e posizionarsi oltre
    dual tmp;
    f.P_read(&tmp, sizeof(tmp));
    bool needConvert = false;
#ifdef UNICODE
    // fallisce ... allora controlliamo a mano
//    if(!IsTextUnicode(tmp.name, SIZE_A(tmp.name), 0))
//      needConvert = true;
    DWORD lenCheck = len / sizeof(dual);
    lenCheck *= sizeof(dual);
    if(lenCheck != len)
      needConvert = true;
    else {
      lenCheck = len / sizeof(dualAltern);
      lenCheck *= sizeof(dualAltern);
      // ci troviamo nel caso che potrebbero essere entrambe
      if(lenCheck == len) {
        uint len = strlen((LPCSTR)tmp.name);
        if(len > 1)
          needConvert = true;
        }
      }
#else // ormai non facciamo più sVisor non unicode ... inutile verificare questa sotto
    if(IsTextUnicode(tmp.name, SIZE_A(tmp.name), 0))
      needConvert = true;
#endif
    f.P_seek(0);
    if(needConvert) {
      dualAltern dualA;
      len /= sizeof(dualA);
      oper.setDim(len);
      for(uint i = 0; i < len; ++i) {
        f.P_read(&dualA, sizeof(dualA));
        copyStruct(oper[i], dualA);
        }
      }
    else {
      len /= sizeof(dual);
      oper.setDim(len);
      for(uint i = 0; i < len; ++i)
        f.P_read(&(oper[i]), sizeof(dual));
      }
    }
}
//----------------------------------------------------------
LPCTSTR codeOper::getName(const codeType& code)
{
  int elem = oper.getElem();
  for(int i = 0; i < elem; ++i)
    if(code == oper[i].code)
      return oper[i].name;
  return 0;
}
//----------------------------------------------------------
const codeType& codeOper::getCode(LPCTSTR name)
{
  int elem = oper.getElem();
  for(int i = 0; i < elem; ++i)
    if(!_tcsicmp(name, oper[i].name))
      return oper[i].code;
  static const codeType t = 0;
  return t;
}
//----------------------------------------------------------
//----------------------------------------------------------
#define dChecks dExRep
//----------------------------------------------------------
//----------------------------------------------------------
manageGen::manageGen(mainClient* parent) : Parent(parent), Stat(stop)
{
  memset(&check, 0, sizeof(check));
}
//----------------------------------------------------------
void manageGen::close()
{
//  sendStop();
  bool remove = !loadTemp();
  tempToStore(remove);
}
//----------------------------------------------------------
bool manageGen::loadTemp()
{
  TCHAR name[_MAX_PATH];
  makeTempPath(name);

  P_File f(name, P_READ_ONLY);
  if(f.P_open()) {
    f.P_read(&check, sizeof(check));
    return true;
    }
  return false;
}
//----------------------------------------------------------
void manageGen::saveTemp()
{
  TCHAR name[_MAX_PATH];
  makeTempPath(name);

  check.end = getFileTimeCurr();
  P_File f(name, P_CREAT);
  if(f.P_open())
    f.P_write(&check, sizeof(check));
}
//----------------------------------------------------------
void manageGen::removeTemp()
{
  TCHAR name[_MAX_PATH];
  makeTempPath(name);
  DeleteFile(name);
}
//----------------------------------------------------------
void manageGen::tempToStore(bool remove)
{
  TCHAR name[_MAX_PATH];
  makeStorePath(name);

  P_File f(name);
  if(f.P_open()) {
    f.P_seek(0, SEEK_END_);
    check.end = getFileTimeCurr();
    f.P_write(&check, sizeof(check));
    if(remove) {
      makeTempPath(name);
      DeleteFile(name);
      }
    }
}
//----------------------------------------------------------
//----------------------------------------------------------
manageStop::manageStop(mainClient* parent) : manageGen(parent), waitBeforeShow(false),
  listOnShow(0), checkListOnShow(false), Value(0)

{
  if(loadTemp() && CODE_NORMAL_STOP != check.code) {
    // sendStop(); // non dovrebbe essere necessario

#define RESTART_TIME
#ifdef RESTART_TIME
    // reinizializza il time se chiusura programma con macchina in run
    // (o spegnimento forzato prima della scelta ??)
    if(TEMP_CODE == check.code) {
      check.init = getFileTimeCurr();
      saveTemp();
      }
#endif
    Stat = stopped_NeedShow;
    waitBeforeShow = true;
    }
  // se si chiude il programma senza aver mai avviato la macchina
  // al prossimo avvio chiede il motivo, settando già la chiusura
  // normale si evita
  else
    check.code = CODE_NORMAL_STOP;

//#define STORE_START_PRG
#ifdef STORE_START_PRG
  codeType old = check.code;
  check.code = CODE_NORMAL_STOP; // CODE_NORMAL_START;
  check.end = check.init;
  saveTemp();
  tempToStore(false);
  check.code = old;
  saveTemp();
#endif
}
//----------------------------------------------------------
manageStop::~manageStop()
{
  if(run == Stat) {
//    waitBeforeShow = true;
//    Stat = stopped_NeedShow;
//    changed = rInitWaitForShow;
    check.init = getFileTimeCurr();
    check.end = check.init;
    check.code = TEMP_CODE;
    saveTemp();
    }
  close();
}
//----------------------------------------------------------
void manageStop::makeTempPath(LPTSTR path)
{
  makePath(path, NAME_MANAGE_STOP_TEMP, dChecks, false);
}
//----------------------------------------------------------
void manageStop::tempToStore(bool remove)
{
  if(TEMP_CODE == check.code)
    return;

  manageGen::tempToStore(remove);
}
//----------------------------------------------------------
void manageStop::makeStorePath(LPTSTR path)
{
  _tcscpy_s(path, _MAX_PATH, NAME_MANAGE_STOP);
  makeYearAndMonth(path + OFFS_DATE);
  makePath(path, dChecks, false);
}
//----------------------------------------------------------
void manageStop::performSetAlternateChoose(DWORD value)
{
	checkListOnShow = false;
  HWND hw = GetDlgItem(*listOnShow, IDC_LISTBOX_CHOOSE_EXP);
  SendMessage(hw, LB_SETCURSEL, (WPARAM)(value - 1), 0);
//  UpdateWindow(hw);
//  Sleep(200);
  PostMessage(*listOnShow, WM_COMMAND, MAKEWPARAM(IDC_LISTBOX_CHOOSE_EXP, LBN_DBLCLK), (LPARAM)hw);
}
//----------------------------------------------------------
void manageStop::setAlternateChoose(DWORD value)
{
  if(!listOnShow || !listOnShow->getHandle()) {
		checkListOnShow = true;
		Value = value;
    return;
		}
	performSetAlternateChoose(value);
}
//----------------------------------------------------------
class TD_CauseList : public svModDialog
{
  private:
    typedef svModDialog baseClass;
  public:
    TD_CauseList(PWin* parent, codeType& code, bool startHided, uint resId = IDD_CHOOSE_LIST_CAUSE, HINSTANCE hinst = 0);
    virtual ~TD_CauseList() { destroy(); }
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    PListBox* LB;
    static int choose;
    codeType& code;

  public:
    virtual bool create();
  protected:
  	bool startHided;
    void CmOk();
    void CmCancel() {}
};
//----------------------------------------------------------
bool manageStop::showList()
{
  TD_CauseList cl(Parent, check.code, checkListOnShow);
  listOnShow = &cl;
  if(checkListOnShow)
	  cl.smodal(false);
	else
	  cl.modal();
  listOnShow = 0;
  saveTemp();
  return CODE_NORMAL_STOP == check.code;
}
//----------------------------------------------------------
/*
  Al fermo macchina, se la manage è in run, attiva il flag di
  attesa prima di presentare la finestra di scelta, modifica
  lo stato in needShow ed attende l'azzeramento del flag.

  Con flag azzerato, entra nello stato needShow, cambia in
  OnShow e ritorna la conferma per l'apertura della finestra
  che avverrà nel successivo ciclo.

  Entrando in OnShow apre una finestra modale per la scelta
  della causa e non torna finché l'operatore non ne accetta una.
  Se sceglie la causa di fine lavoro salva il temporaneo nel
  definitivo. Torna, infine, il codice di chiusura finestra.

  Al riavvio della macchina, salva semplicemente il temporaneo
  nel definitivo.
*/
//----------------------------------------------------------
int manageStop::notify(gestPerif* plc, bool requestCode)
{
	if(checkListOnShow)
		setAlternateChoose(Value);
  // se è in attesa di mostrare la finestra di scelta torna subito
  if(waitBeforeShow || requestCode)
    return rWaitForShow;
  gestPerif::statMachine how = plc->isRunning();

  int changed = rNotChanged;
  switch(Stat) {
    case run:
      if(gestPerif::sOnStop == how) {
        waitBeforeShow = true;
        Stat = stopped_NeedShow;
        changed = rInitWaitForShow;
        check.init = getFileTimeCurr();
        check.end = check.init;
        check.code = TEMP_CODE;
        saveTemp();
        }
      break;

    case stopped_NeedShow:
      Stat = stopped_OnShow;
      changed = rShowMsg;
      break;

    case stopped_OnShow:
      // per evitare di rientrare all'avvio
      waitBeforeShow = true;

      if(gestPerif::sStopped & how)
        Stat = stop;
      else if(gestPerif::sRunning & how)
        Stat = run;

      changed = rShowMsg;
      // se fine lavoro salva nel definitivo
      if(showList())
        tempToStore(true);
      changed = rHideMsg;
      waitBeforeShow = false;
      break;

    case stop:
      if(gestPerif::sOnRun == how) {
        tempToStore(true);
        Stat = run;
        // nessuna notifica particolare di ritorno
        }
    }
  return changed;
}
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
extern
genericPerif* getAddrCodeOperData(mainClient *parent, uint& addr);
//----------------------------------------------------------
manageOperator::manageOperator(mainClient* parent) :
    manageGen(parent), Request(0), notUseReader(false)

{
  if(loadTemp()) {
    // se esisteva il temporaneo chiude il definitivo con il time corrente
    // e l'operatore assegnato
    tempToStore(true);
    check.code = 0;
    }
  uint addr;
  genericPerif* prph = getAddrCodeOperData(parent, addr);
  if(PRF_MEMORY == prph->getId())
    notUseReader = true;
  setOperatorToJob();
}
//----------------------------------------------------------
manageOperator::~manageOperator()
{
  close();
  removeTemp();
  if((PWin*)-1 != Request)
    delete Request;
}
//----------------------------------------------------------
void manageOperator::makeTempPath(LPTSTR path)
{
  makePath(path, NAME_MANAGE_OPER_TEMP, dChecks, false);
}
//----------------------------------------------------------
void manageOperator::makeStorePath(LPTSTR path)
{
  _tcscpy_s(path, _MAX_PATH, NAME_MANAGE_OPER);
  makeYearAndMonth(path + OFFS_DATE);
  makePath(path, dChecks, false);
}
//----------------------------------------------------------
//----------------------------------------------------------
#define ID_CODE 0
//----------------------------------------------------------
bool manageOperator::isValid(const codeType& curr)
{
  // non possono avvenire, viene chiamata solo quando cambia
  // il codice, quindi curr deve essere valido e diverso dal vecchio
//  if(!curr)
//    return false;
//  if(curr == check.code)
//    return true;
  // se riesce ad ottenere il nome il codice è valido
  codeOper co;
  return co.getName(curr) != 0;
}
//----------------------------------------------------------
void manageOperator::showRequest()
{
  Stat = stopped_NeedShow;
  check.init = getFileTimeCurr();
}
//----------------------------------------------------------
static void enableParent(PWin* win, bool enable)
{
  static HWND oldWin;
  if(!enable)
    oldWin = ::GetForegroundWindow();
  else {
    if(IsWindow(oldWin))
      ::EnableWindow(oldWin, true);
    else {
      while(win->getParent())
        win = win->getParent();
      ::EnableWindow(*win, enable);
      }
    }
}
//----------------------------------------------------------
int manageOperator::notify(gestPerif* plc, genericPerif* reader)
{
  prfData data;
  if(notUseReader)
    getAddrCodeOperData(Parent, data.lAddr);
  else
    data.lAddr = ID_CODE;
  data.typeVar = prfData::tDWData;

  // deve essere true se è cambiato il codice o comunque c'è
  // stata una lettura
  bool readed = prfData::isChanged == reader->get(data);
  if(notUseReader && stopped_OnShow == Stat)
    readed = true;

  codeType curr = data.U.dw;

  int changed = rNotChanged;
  gestPerif::statMachine how = plc->isRunning();

  // se il codice non è cambiato
  if(curr && curr == check.code) {
    switch(Stat) {
      case run: // operatore già inserito
        break;

      case stopped_NeedShow:
      case stopped_OnShow:
        // se non c'è stata una rilettura apre la finestra
        // ed esce, altrimente esegue codice comune con onShow
        if(!readed || !curr) {
          changed = rShowMsg;
          if(Stat != stopped_OnShow) {
            Stat = stopped_OnShow;
            showMsg();
            }
          break;
          }
        // fall throught

        // l'operatore non è cambiato, ma ha reinserito la tessera

        // chiude la finestra e riattiva l'operatore
      case stopped_EndShow:
        saveTemp();
        hideMsg();
        changed = rHideMsg;
        Stat = run;
        break;

      // dovrebbe valere solo all'avvio
      case stop:
        Stat = stopped_NeedShow;
        break;
      }
    }
  // è cambiato il codice operatore o non esiste
  else {
    if(!curr && !notUseReader) {
      changed = rShowMsg;
      if(Stat != stopped_OnShow) {
        Stat = stopped_OnShow;
        showMsg();
        }
      return changed;
      }
    // il codice è genericamente valido (il driver trasmette solo
    // codici validi), ma non è abilitato per questa macchina
    if(!isValid(curr))
      return rCodeNotValid;

    switch(Stat) {
      case run:
        tempToStore(false);
        // fall throught

      case stop:
        check.init = getFileTimeCurr();
        // fall throught
      case stopped_OnShow:
      case stopped_NeedShow:
        check.code = curr;
        saveTemp();
        hideMsg();
        Stat = run;
        setOperatorToJob();
        changed = rChanged;
        break;
      }
    }
  if(gestPerif::sOnRun == how) {
    saveTemp();
    tempToStore(false);
    }
  return changed;
}
//----------------------------------------------------------
//#define ID_MSG_NEED_CODE        5990
//----------------------------------------------------------
void manageOperator::showMsg()
{
  if(Request)
    return;
  if(notUseReader) {
    Request = (PWin*)-1;
    return;
    }
  EnableWindow(*Parent, false);
  Request = new svDialog(Parent, IDD_NEED_CODE);
  if(Request->create()) {
    UpdateWindow(*Request);

    ::SetDlgItemText(*Request, IDC_NEED_CODE_MSG, getStringOrIdByLangGlob(ID_MSG_NEED_CODE));
    while(Request->getHandle()) {
      ::WaitMessage();
      getAppl()->pumpMessages();
      }
    }
  EnableWindow(*Parent, true);
  delete Request;
  Request = 0;
}
//----------------------------------------------------------
void manageOperator::hideMsg()
{
  if((PWin*)-1 == Request)
    Request = 0;
  if(Request && Request->getHandle())
    DestroyWindow(*Request);
}
//----------------------------------------------------------
void manageOperator::setOperatorToJob()
{
  if(notUseReader)
    return;
  uint addr;
  genericPerif* prph = getAddrCodeOperData(Parent, addr);
  if(prph) {
    prfData data;
    data.lAddr = addr;
    data.typeVar = prfData::tDWData;
    data.U.dw = check.code;
    prph->set(data);
    }
}
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
int TD_CauseList::choose;
//----------------------------------------------------------
TD_CauseList::TD_CauseList(PWin* parent, codeType& code, bool startHided, uint resId, HINSTANCE hinst) :
    svModDialog(parent, resId, hinst), code(code), startHided(startHided)
{
  LB = new PListBox(this, IDC_LISTBOX_CHOOSE_EXP);
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
bool TD_CauseList::create()
{
  if(!baseClass::create())
    return false;
  if(!startHided)
	  ShowWindow(*this, SW_NORMAL);
  for(int i = ID_CAUSE_TITLE_INIT; i <= ID_CAUSE_TITLE_END; ++i) {
    smartPointerConstString p(getStringByLangGlob(i));
    if(!p)
      break;
    SendMessage(*LB, LB_ADDSTRING, 0, (LPARAM)&p);
    }
  for(int i = FIRST_ID_DEFAULT; i <= LAST_ID_DEFAULT; ++i) {
    smartPointerConstString p(getStringByLangGlob(i));
    if(!p)
      break;
    SendMessage(*LB, LB_ADDSTRING, 0, (LPARAM)&p);
    }

  if(TEMP_CODE != code) {
    int count = SendMessage(*LB, LB_GETCOUNT, 0, 0);
    if((int)code > count - NUM_DEF_CAUSE) {
      choose = code - FIRST_CODE_DEFAULT;
      choose += count - NUM_DEF_CAUSE;
      }
    else
      choose = code - 1;
    }

  SendMessage(*LB, LB_SETCURSEL, choose, 0);

  return true;
}
//--------------------------------------------------------------------------
LRESULT TD_CauseList::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_F1:
          CmOk();
          break;
        case IDC_LISTBOX_CHOOSE_EXP:
          switch(HIWORD(wParam)) {
            case LBN_DBLCLK:
              CmOk();
              break;
            }
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//--------------------------------------------------------------------------
void TD_CauseList::CmOk()
{
  choose = SendMessage(*LB, LB_GETCURSEL, 0, 0);
  if(choose < 0)
    return;
  TCHAR buff[500];
  SendMessage(*LB, LB_GETTEXT, choose, (LPARAM)buff);

  int count = SendMessage(*LB, LB_GETCOUNT, 0, 0);
  if(choose >= count - NUM_DEF_CAUSE) {
    code = choose - (count - NUM_DEF_CAUSE);
    code += FIRST_CODE_DEFAULT;
    }
  else
    code = choose + 1;
  baseClass::CmOk();
}
