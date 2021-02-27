//------ P_DefBody.cpp -------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include "p_DefBody.h"
#include "pAllObj.h"
#include "id_btn.h"
#include "password.h"
#include "p_util.h"
#include "def_dir.h"
#include "lnk_body.h"
#include "newnormal.h"
#include "pTouchKeyb.h"
//----------------------------------------------------------------------------
#include "1.h"
//----------------------------------------------------------------------------
bool isSpinId(uint id)
{
  return P_DefBody::ID_SPIN_UP_DOWN == id;
}
//----------------------------------------------------------------------------
P_DefBody::P_DefBody(int /*idParent*/, PWin* parent, uint resId, HINSTANCE hInstance) :
    baseClass(0, parent, resId, hInstance), pageName(0)
{}
//----------------------------------------------------------------------------
P_DefBody::P_DefBody(PWin* parent, LPCTSTR pageDescr) :
    baseClass(0, parent, IDD_STANDARD), pageName(str_newdup(pageDescr))
{
}
//----------------------------------------------------------------------------
P_DefBody::~P_DefBody()
{
  destroy();
  delete [] pageName;
}
//----------------------------------------------------------------------------
void P_DefBody::setPageName(LPCTSTR pageDescr)
{
  delete []pageName;
  pageName = str_newdup(pageDescr);
}
//----------------------------------------------------------------------------
LRESULT P_DefBody::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(HIWORD(wParam)) {
        case BN_CLICKED:
          action_Btn(LOWORD(wParam));
          break;
        }
      break;
    case WM_POST_BTN_MODAL_BODY:
      pushedBtn(wParam);
      return 1;

    case WM_POST_NAME_MODAL_BODY:
      if(setModalBit(true, BIT_ON_EXEC)) {
        openModal((LPCTSTR) lParam);
        setModalBit(false, BIT_ON_EXEC);
        }
      return 1;

    case WM_POST_NAME_MODELESS_BODY:
      openModeless((LPCTSTR) lParam, toBool(wParam));
      return 1;

    case WM_POST_OPEN_MODAL_KEYB:
//      if(setModalBit(true, BIT_ON_KEYB)) {
        openKeyboard((LPCTSTR) lParam, HWND(wParam));
//        setModalBit(false, BIT_ON_KEYB);
//        }
      return 1;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void P_DefBody::refreshBody()
{
  if(isBodyReady()) {
    P_Body* bd = dynamic_cast<P_Body*>(getParent());
    if(!bd)
      if(!SV_RefreshBody(Attr.id, getPageName(), toBool(needRefresh)))
        return;
    }
  baseClass::refreshBody();
}
//----------------------------------------------------------------------------
bool P_DefBody::setModalBit(bool set, uint bit)
{
  genericPerif* prf = Par->getGenPerif(PRF_MEMORY);
  prfData data;
  data.lAddr = TEST_BIT_MODAL;
  data.typeVar = prfData::tDWData;
  prf->get(data);
  bit = 1 << bit;
  bool changed = toBool(data.U.dw & bit) ^ set;
  if(set)
    data.U.dw |= bit;
  else
    data.U.dw &= ~bit;
  prf->set(data);
  return changed;
}
//----------------------------------------------------------------------------
void P_DefBody::action_Btn(uint idBtn)
{
  // se è nel range dei pulsanti
  uint id = manageObjId(idBtn).getBaseId();
  if(id != ID_INIT_VAR_BTN) {
    idBtn += OFFS_INIT_VAR_EXT;
    id = manageObjId(idBtn).getBaseId();
    }
  if(id == ID_INIT_VAR_BTN) {
//  if(idBtn >= ID_INIT_VAR_BTN && (ID_INIT_VAR_BTN + ADD_INIT_VAR) > idBtn) {

    // se non viene trovato nei pulsanti allocati esce
    uint nElem = Btn.getElem();
    PVarBtn* p_Btn = 0;
    for(uint i = 0; i < nElem; ++i) {
      PVarBtn* pBtn = Btn[i];
      if(pBtn->getId() == idBtn) {
        p_Btn = pBtn;
        break;
        }
      }
    if(!p_Btn)
      return;

    int idprf = p_Btn->getIdPrph();
    if(!idprf) {
      int addr = p_Btn->getAddr();
      if(addr >= 120000)
        return;

      if(110000 <= addr) {
        PWin* w = getParent();
        while(!dynamic_cast<mainClient*>(w)) {
          w = w->getParent();
          if(!w)
            break;
          }
        if(w)
          PostMessage(*w, WM_POST_OPEN_NEW_PAGE_BODY, addr  - MAX_BTN, 0);
        }
      else if(100000 <= addr)
        PostMessage(*this, WM_POST_BTN_MODAL_BODY, addr  - MAX_BTN, 0);

    // aggiunta per pulsanti estesi
      else if(ID_SHIFT_F1 <= addr && addr <= ID_SHIFT_F12 || ID_CTRL_F1 <= addr && addr <= ID_CTRL_F12)
        PostMessage(*Par, WM_MYHOTKEY, addr, 0);

      else if(ID_F1 <= addr && addr <= ID_F12)
        PostMessage(*Par, WM_MYHOTKEY, addr, 0);
      }
    }
}
//----------------------------------------------------------------------------
inline
static void resetPsw()
{
  PassWord().restartTime(0);
}
//----------------------------------------------------------------------------
P_Body* P_DefBody::getPreviousPage()
{
  P_Body* body = getBody(getParent(), 0, pageName);
  resetPsw();
  return body;
}
//----------------------------------------------------------------------------
P_Body* P_DefBody::pushedBtn(int idBtn)
{

  LPCTSTR childPage;
  int idChild;
  int pswLevel;
  bool predefinite;
  switch(idBtn) {
    case ID_F1:
      if(usePredefBtn(ID_F1)) {
        P_Body* body = getBody(getParent(), 0, pageName);
        if(!body) {
          // se si imposta l'uso predefinito di F1 occorre assicurarsi che si possa
          // uscire dal programma, il codice è stato rimosso dalla pagina principale
          LPCTSTR p = getString(ID_END_PSW);
          postActionBtnWithPsw(p, MSG_CLOSE_PROG);
          break;
          }
        return body;
        }
    // fall through
    default:
      idChild = 0;
      predefinite = false;
      childPage = findChildPage(idBtn, idChild, pswLevel, predefinite);
      // se ha caricato almeno uno dei due dati
      if(childPage || idChild) {
        bool rd;
        if(!hasPermission(pswLevel, rd))
          return 0;
        int id = Attr.id;
        // se è una pagina generica annulla l'id
        if(IDD_STANDARD == id)
          id = 0;
        // se non ha caricato un id Parent e non ha settato il nome della pagina
        // significa che si tratta di una pagina non generica, allora occorre
        // caricare l'id proprio, per sapere dove ritornare e per poter eseguire
        // la routine specifica di caricamento della pagina
//        if(!idChild && !pageName)
//          idChild = id;
        P_Body* bd = getBody(getParent(), childPage, pageName, idChild, id);
        if(bd) {
          bd->setReadOnly(rd);
          }
        return bd;
        }
      break;
    }
  return 0;
}
//----------------------------------------------------------------------------
//void P_DefBody::
void getFileStrCheckRemote(LPTSTR target, LPCTSTR file)
{
  if(!file)
    return;
  _tcscpy_s(target, _MAX_PATH, file);
  bool local = false;
  LPCTSTR p = ::getString(ID_USE_LOCAL_PATH_FOR_PAGE_TEXT);
  if(p)
    local = toBool(_ttoi(p));
  if(local)
    getRelPath(target, dSystem);
  else
    getPath(target, dSystem);
}
//----------------------------------------------------------------------------
static void createDirectoryRecurseByFull(LPCTSTR path)
{
  TCHAR t[_MAX_PATH];
  _tcscpy_s(t, path);
  dirName(t);
  createDirectoryRecurse(t);
}
//----------------------------------------------------------------------------
bool P_DefBody::getCacheFile(LPTSTR target, uint sz)
{
  TCHAR t[_MAX_PATH];
  t[0] = 0;
  getPath(t, dSystem);
  appendPath(t, _T("cache"));
  if(pageName && *pageName)
    appendPath(t, pageName);
  else {
    TCHAR tmp[64];
    switch(Attr.id) {
      case IDD_MAIN_MENU:
        _tcscpy_s(tmp, FIRST_PAGE);
        break;
      case ID_OPEN_TREND:
        _tcscpy_s(tmp, _T("Trend")PAGE_EXT);
        break;
      case ID_OPEN_RECIPE:
        _tcscpy_s(tmp, _T("Recipe")PAGE_EXT);
        break;
      case ID_OPEN_RECIPE_ROW:
        _tcscpy_s(tmp, _T("Recipe_Row")PAGE_EXT);
        break;
      default:
        wsprintf(tmp, _T("pag_%d")PAGE_EXT, Attr.id);
        break;
      }
    appendPath(t, tmp);
    }
  createDirectoryRecurseByFull(t);
  _tcscat_s(t, _T(".cache"));
  _tcscpy_s(target, sz, t);
  P_File c(target, P_READ_ONLY);
  // se non riesce ad aprirlo si può supporre che non esista ancora
  if(!c.P_open())
    return false;
  FILETIME ftc;
  c.getData(ftc);
  getFileStr(t);

  P_File pg(t, P_READ_ONLY);
  if(!pg.P_open())
    return false;
  FILETIME ftpg;
  pg.getData(ftpg);
  return ftc >= ftpg;
}
//----------------------------------------------------------------------------
void P_DefBody::getFileStr(LPTSTR path)
{
  if(pageName)
    getFileStrCheckRemote(path, pageName);
}
//----------------------------------------------------------------------------
void P_DefBody::postCTRL_ActionBtnWithPsw(LPCTSTR p, UINT code)
{
  bool hasPsw = false;
  if(p) {
    int pswLevel = _ttoi(p);
    hasPsw = toBool(pswLevel);
    bool dummy;
    if(!hasPermission(pswLevel, dummy))
      return;
    resetPsw();
    }
  PostMessage(*getMain(), WM_MY_HOTKEY, code, 0);
}
//----------------------------------------------------------------------------
void P_DefBody::postActionBtnWithPsw(LPCTSTR p, UINT code)
{
  bool hasPsw = false;
  if(p) {
    int pswLevel = _ttoi(p);
    hasPsw = toBool(pswLevel);
    bool dummy;
    if(!hasPermission(pswLevel, dummy))
      return;
    resetPsw();
    }
  PostMessage(*getMain(), WM_CUSTOM, MAKEWPARAM(code, hasPsw), 0);
}
//----------------------------------------------------------------------------
LPCTSTR P_DefBody::findChildPage(int idBtn, int& idChild, int& pswLevel, bool& predefinite)
{
  int id = idBtn + MAX_BTN;
  LPCTSTR p = getPageString(id);

  if(!p)
    return 0;
  const int id_childs[] = { IDD_LGRAPH2, IDD_RECIPE, IDD_RECIPE_ROW, IDD_MANUTENZ };

  id = _ttoi(p);
  p = findNextParam(p, 1);
  switch(id) {
    case ID_OPEN_PAGE:
      pswLevel = _ttoi(p);
      p = findNextParam(p, 1);
      idChild = _ttoi(p);
      return findNextParam(p, 1);

    case ID_SEND_DATA:
      SendData(p, false);
      break;
    case ID_TOGGLE_BIT:
      SendData(p, true);
      break;
    case ID_SEND_ALL:
      Send(p);
      break;

    case ID_RECEIVE_ALL:
      p = findNextParam(p, 1);
      Receive(_ttoi(p));
      break;

    case ID_EXPORT_DATA:
      performExport(p);
      Par->setEvent(log_status::refreshStat);
      break;

    case ID_OPEN_TREND:
    case ID_OPEN_RECIPE:
    case ID_OPEN_RECIPE_ROW:
    case ID_OPEN_MAINT:
      pswLevel = _ttoi(p);
      if(ID_OPEN_MAINT == id)
        idChild = IDD_MANUTENZ;
      else
        idChild = id_childs[id - ID_OPEN_TREND];
      predefinite = true;
      do {
        LPCTSTR p2 = findNextParam(p, 2);
        if(p2) {
          p2 = findNextParam(p, 1);
          if(_istdigit((unsigned)*p2)) {
            idChild += _ttoi(p2);
            p = p2;
            }
          }
        } while(false);
      return findNextParam(p, 1);

    case ID_OPEN_MODALBODY:
      openModal(p);
      break;

    case ID_CHANGE_PSW:
      do {
        int level = 1;
        if(p)
          level = _ttoi(p);
        PassWord().changePsw(level, this);
        } while(false);
      break;

    case ID_GOTO_PREVIOUS_PAGE:
      postActionBtnWithPsw(p, MSG_PREV_PAGE);
      break;
    case ID_SHUTDOWN:
      postActionBtnWithPsw(p, MSG_CLOSE_PROG);
      break;
    case ID_SHOW_TREEVIEW:
      postActionBtnWithPsw(p, MSG_SHOW_TREEVIEW);
      break;
    case ID_PRINT_TREEVIEW:
      postActionBtnWithPsw(p, MSG_PRINT_TREEVIEW);
      break;
    case ID_SHOW_ALARM:
      postActionBtnWithPsw(p, MSG_SHOW_ALARM);
      break;

    case ID_PRINT_SCREEN:
      do {
        int preview = p ? _ttoi(p) : 1;
        p = findNextParam(p, 1);
        int setup = p ? _ttoi(p) : 1;
        int fullscreen = p ? _ttoi(p) : 1;
        PostMessage(*getMain(), WM_CUSTOM, MAKEWPARAM(MSG_PRINT_SCREEN, fullscreen), MAKELPARAM(preview, setup));
        } while(false);
      break;
    case ID_SPIN_UP_DOWN:
      sendSpin(p);
      break;
    case ID_SEND_CTRL_ACTION:
      do {
        LPCTSTR p2 = findNextParamTrim(p);
        int action = _ttoi(p2);
        postCTRL_ActionBtnWithPsw(p, ID_FZ_SETUP + action);
        } while(false);
      break;
    }
  return 0;
}
//----------------------------------------------------------------------------
P_BaseBody::preSend P_DefBody::preSendEdit(PVarEdit* ctrl, prfData& data)
{
  // se è sola lettura non invia mai
//  if(isReadOnly())
//    return P_BaseBody::NO_SEND;
  if(!checkObjectPsw(ctrl->getId()))
    return P_BaseBody::NO_SEND;

  LPCTSTR p = getPageString(ID_DEF_SEND_EDIT);
  bool autoSend = true;
  if(p)
    autoSend = toBool(_ttoi(p));
  // se periferica in memoria spedisce sempre
  if(1 == ctrl->getIdPrph())
    autoSend = true;

  // il controllo del range è già stato fatto nell'oggetto
  return autoSend ? P_BaseBody::YES_SEND : P_BaseBody::WAIT_SEND;
}
//----------------------------------------------------------------------------
void P_DefBody::postSendEdit(PVarEdit* pe)
{
  baseClass::postSendEdit(pe);
  pe->set_Text(WAIT_UPDATE_CHAR_EDIT);
  SendMessage(*pe, EM_SETMODIFY, TRUE, 0);
}
//----------------------------------------------------------------------------
bool P_DefBody::preSendBtn(baseActive* ctrl, prfData& data)
{
  // se è sola lettura non invia mai
//  if(isReadOnly())
//    return false;
  bool changed;
  if(!checkObjectPsw(ctrl->getId(), &changed)) {
    ctrl->update(true);
    return false;
    }
  if(changed) {
    PVarBtn* p_Btn = dynamic_cast<PVarBtn*>(ctrl);
    if(p_Btn && p_Btn->isSetOnPress()) {
      ctrl->update(true);
      return false;
      }
    }
  return baseClass::preSendBtn(ctrl, data);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define MAX_MULTI_DATA 100
//----------------------------------------------------------------------------
void P_DefBody::Send(LPCTSTR p)
{
  // se è sola lettura non invia mai
/*
  if(isReadOnly()) {
    setDirty();
    HWND focus = ::GetFocus();
    HWND other = GetNextWindow(focus, GW_HWNDNEXT);
    if(!other)
      other = GetNextWindow(focus, GW_HWNDPREV);
    SetFocus(other);
    SetFocus(focus);
    return;
    }
*/
  int pswLevel = _ttoi(p);
  bool resetPsw = false;
  if(pswLevel < 0) {
    pswLevel = -pswLevel;
    resetPsw = true;
    }
  else if(pswLevel > 3 && ONLY_CONFIRM != pswLevel)
    pswLevel = 3;
  PassWord psw;

  if(pswLevel == psw.getPsW(pswLevel, this)) {
    p = findNextParam(p, 1);
    uint idprf = _ttoi(p);

    gestPerif* prf = Par->getPerif(idprf);

    if(!prf)
      return;

    int nElem = Edi.getElem();

    prfData data[MAX_MULTI_DATA];

    int loaded = 0;

    for(int i = 0; i < nElem && loaded < MAX_MULTI_DATA; ++i) {

      if(!getData(data[loaded], Edi[i], idprf))
        continue;
      if(verifyRange(Edi[i]->getIdNorm(), data[loaded]))
        ++loaded;
      Edi[i]->set_Text(_T("---"));
      }
    if(loaded) {
      gestPerif::howSend old = prf->setAutoShow(showWhileSend() ? gestPerif::ALL_AND_SHOW : gestPerif::CHECK_ALL_ASYNC);
      prf->multiSet(data, loaded);
      prf->setAutoShow(old);
      }
    if(resetPsw)
      psw.restartTime(0);
    }
  setDirty();
}
//----------------------------------------------------------------------------
void P_DefBody::SendData(LPCTSTR p, bool toggle)
{
  // se è sola lettura non invia mai
//  if(isReadOnly())
//    return;

  int pswLevel = _ttoi(p);
  bool resetPsw = false;
  if(pswLevel < 0) {
    pswLevel = -pswLevel;
    resetPsw = true;
    }
  else if(pswLevel > 3 && ONLY_CONFIRM != pswLevel)
    pswLevel = 3;
  PassWord psw;

  if(pswLevel == psw.getPsW(pswLevel, this)) {
    p = findNextParam(p, 1);
    uint idBtn = _ttoi(p);

    actionBtn(idBtn, toggle);

    if(resetPsw)
      psw.restartTime(0);
    }
  setDirty();
}
//----------------------------------------------------------------------------
void P_DefBody::sendSpin(LPCTSTR p)
{
  // se è sola lettura non invia mai
//  if(isReadOnly())
//    return;

  int pswLevel = _ttoi(p);
  bool resetPsw = false;
  if(pswLevel < 0) {
    pswLevel = -pswLevel;
    resetPsw = true;
    }
  else if(pswLevel > 3 && ONLY_CONFIRM != pswLevel)
    pswLevel = 3;
  PassWord psw;

  if(pswLevel == psw.getPsW(pswLevel, this)) {
    p = findNextParamTrim(p);
    int up = _ttoi(p);
    p = findNextParamTrim(p);
    uint idSpin = _ttoi(p);

    actionSpin(idSpin, up);

    if(resetPsw)
      psw.restartTime(0);
    }
  setDirty();
}
//----------------------------------------------------------------------------
void P_DefBody::Receive(uint idprf)
{
  gestPerif* prf = Par->getPerif(idprf);

  if(!prf)
    return;

  int nElem = Edi.getElem();

  if(!nElem)
    return;

  prfData data;
  if(!getData(data, Edi[0], idprf))
    return;

  DWDATA init = data.lAddr;
  DWDATA end = init;

  for(int i = 1; i < nElem; ++i) {

    if(!getData(data, Edi[i], idprf))
      continue;
    if(data.lAddr < init)
      init = data.lAddr;
    else if(data.lAddr > end)
      end = data.lAddr;
    }
  prf->Receive(prf->LtoP_Init(init), prf->LtoP_End(end));
  setDirty();
}
//----------------------------------------------------------------------------
void P_DefBody::performExport(LPCTSTR p)
{
  int pswLevel = _ttoi(p);
  bool resetPsw = false;
  if(pswLevel < 0) {
    pswLevel = -pswLevel;
    resetPsw = true;
    }
  else if(pswLevel > 3)
    pswLevel = 3;
  PassWord psw;

  if(pswLevel == psw.getPsW(pswLevel, this)) {
    p = findNextParam(p, 1);
    Par->setEvent(log_status::refreshStat);
    if(p)
      exportData(this, toBool(_ttoi(p)));
    if(resetPsw)
      psw.restartTime(0);
    }
}
//----------------------------------------------------------------------------
bool P_DefBody::hasPermission(int pswLevel, bool& rd)
{
  rd = false;
  if(pswLevel < 0) {
    pswLevel = -pswLevel;
    rd = true;
    }
  else if(pswLevel > 3)
    pswLevel = 3;
  PassWord psw;

  if(pswLevel == psw.getPsW(pswLevel, this)) {
    rd = false;
    return true;
    }
  return rd;
}
//----------------------------------------------------------------------------
#include "p_ModalBody.h"
//#include "P_ModalRecipe.h"
//----------------------------------------------------------------------------
void P_DefBody::openModal(LPCTSTR p)
{
  int pswLevel = _ttoi(p);
  PassWord psw;

  if(pswLevel == psw.getPsW(pswLevel, this)) {
    p = findNextParam(p, 1);
    Par->enableDialogFunctionKey();
    P_Body* mb = 0;

    int isRecipe = false;
    if(_T('@') == *p) {
      ++p;
      isRecipe = 1;
      }

    if(isRecipe) {
      mb = new P_ModalRecipe(0, Par, 0);
      mb->setPageName(p);
      }
    else

      mb = new P_ModalBody(Par, p);

    HWND oldFocus = GetFocus();
    PWin* pw = this;
    PVect<PWin*> toDisable;
    int i = 0;
    do {
      toDisable[i] = pw;
      ++i;
      EnableWindow(*pw, false);
      if(dynamic_cast<mainClient*>(pw))
        break;
      pw = pw->getParent();
      } while(pw);
    mainClient* par = Par;
    P_Body* mod = 0;
    if(mb->create()) {
      mb->sendListValue(true);
      Modal = mb;
      mod = Modal;
      // finché non viene chiusa la window
      while(mb->getHandle()) {
        // aspetta l'arrivo di un messaggio
        ::WaitMessage();
        // lo fa elaborare dall'applicazione principale che provvede al dispatch
        getAppl()->pumpMessages();
        }
      // l'invio della lista in chiusura viene fatto direttamente nell'oggetto, qui non è possibile
      }
    // se è stata chiusa dall'esterno, probabile che non esista nemmeno più questa win quindi non si possono usare variabili dell'oggetto
    bool isRight = Modal && mod == Modal;
    if(isRight) {
      Modal = 0;
      SetForegroundWindow(*this);
      // riabilita
      for(int j = i - 1; j >= 0; --j)
        EnableWindow(*toDisable[j], true);
      setDirty();
      }
    SetFocus(oldFocus);
    par->disableDialogFunctionKey();
    delete mb;
    }
}
//----------------------------------------------------------------------------
void P_DefBody::openKeyboard(LPCTSTR p, HWND hwTarget)
{
    Par->enableDialogFunctionKey();
    bool setup = false;
    pTouchKeyb mb(Par, p, hwTarget, setup);

    HWND oldFocus = GetFocus();
    PWin* pw = PWin::getWindowPtr(hwTarget);
    if(!pw)
      pw = this;
    PVect<PWin*> toDisable;
    int i = 0;
    do {
      toDisable[i] = pw;
      ++i;
      EnableWindow(*pw, false);
      if(dynamic_cast<mainClient*>(pw))
        break;
      pw = pw->getParent();
      } while(pw);

    if(mb.create()) {
      // finché non viene chiusa la window
      while(mb.getHandle()) {
        // aspetta l'arrivo di un messaggio
        ::WaitMessage();
        // lo fa elaborare dall'applicazione principale che provvede al dispatch
        getAppl()->pumpMessages();
        }
      }
    // riabilita
    for(int j = i - 1; j >= 0; --j)
      EnableWindow(*toDisable[j], true);
    SetForegroundWindow(*this);
    SetFocus(oldFocus);
//    setDirty();
    Par->disableDialogFunctionKey();
    if(setup)
      PostMessage(*Par, WM_MY_HOTKEY, ID_FZ_SETUP, 0);
}
//----------------------------------------------------------------------------
extern P_Body *getRecipe(int idPar, PWin *parent);
//----------------------------------------------------------------------------
void P_DefBody::openModeless(LPCTSTR p, bool needOpen)
{
#if 0 // non verrà mai eseguita, il messaggio viene inviato al body e comunque ora si aprono anche ricorsivamente
  if(needOpen && (dynamic_cast<P_ModalBody*>(this) || dynamic_cast<P_ModelessBody*>(this)))
    return;
#endif
  int pswLevel = _ttoi(p);
  PassWord psw;


  if(!needOpen || pswLevel == psw.getPsW(pswLevel, this)) {
    p = findNextParam(p, 1);

    int isRecipe = false;
    if(_T('@') == *p) {
      ++p;
      isRecipe = 1;
      }
//    else if(_T('§') == *p) {
//      ++p;
//      isRecipe = 2;
//      }

    int nElem = modeless.getElem();
    int found = -1;
    for(int i = 0; i < nElem; ++i) {
      P_Body* db = modeless[i];
      LPCTSTR pgName = db->getPageName();
      if(!_tcsicmp(p, pgName)) {
        found = i;
        break;
        }
      }

    if(!needOpen ^ (found >= 0))
      return;

    if(needOpen) {
      P_Body* mb = 0;

      if(isRecipe) {
        mb = new P_ModelessRecipe(0, this, 0);
        mb->setPageName(p);
        }
      else

        mb = new P_ModelessBody(this, p);
      modeless[nElem] = mb;
      mb->create();
      mb->sendListValue(true);
      }
    else {
      P_Body* mb = modeless.remove(found);
      if(mb) {
        mb->sendListValue(false);
        mb->refresh();
        }
      delete mb;
      }
    }
}
