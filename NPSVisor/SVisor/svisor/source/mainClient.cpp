//------- mainClient.cpp -----------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "mainClient.h"
//----------------------------------------------------------------------------
#include "lnk_body.h"
#include "header.h"
#include "footer.h"
#include "svPrinter.h"
#include "config.h"
#include "hdrmsg.h"
#include "p_name.h"
#include "p_date.h"
#include "p_file.h"
#include "password.h"
#include "def_dir.h"

#include "sizer.h"
#include "p_util.h"
#include "pedit.h"

#include "1.h"
#include "PVarCam.h"
#include "TD_Choose_FilterTree.h"
#include "TD_Choose_PrintTree.h"
#include "printScreen.h"

#include "p_MappedFile.h"
#include "newNormal.h"
#include "currAlrm.h"
#include "DChooseResolution.h"
#include "mainApp.h"
//----------------------------------------------------------------------------
#ifndef WM_PLC
  #define WM_PLC 2
#endif
//----------------------------------------------------------------------------
class infoReqRead
{
  public:
    infoReqRead() : pmf(0), id(0), tick(0), idPrph(0) {}
    ~infoReqRead() { delete pmf; }

    bool sendData(HWND driver, uint prph, const PVect<DWORD>& data, bool oneReq);
    bool isEqu(WORD w, DWORD dw);
    bool isPrph(uint prph);

    bool onTimeout(uint prph);

  private:
    p_MappedFile* pmf;
    WORD id;
    DWORD tick;
    uint idPrph;
};
//----------------------------------------------------------
#include "svVersionM.h"
//----------------------------------------------------------
#define M_BASEVER svVerM1
#define m_BASEVER svVerM2
#define APPL_BASE_VER MAKEWORD(m_BASEVER, M_BASEVER)
//----------------------------------------------------------------------------
#define TRACE(a)  MessageBox(0, _T(a), " ", MB_OK)
//----------------------------------------------------------------------------
extern P_Body *allocMainMenu(PWin *parent);
//----------------------------------------------------------------------------
#define SLEEP__(a) SleepEx(a, false)
//----------------------------------------------------------------------------
#define IS_WINNT_BASED isWinNT_Based()
// time per effettuare un refresh del log
#define SECONDS_DELAY_TO_REFRESH 60
//----------------------------------------------------------------------------
#define FNAME_SYSTEM        "_npSV_System"
#define FNAME_REFRESH       "_npSV_Refresh"
#define FNAME_BODY_REFRESH  "_npSV_RefreshBody"
//----------------------------------------------------------------------------
struct svDllMan
{
  SV_SYSTEM system;
  SV_REFRESH refresh;
  SV_REFRESHBODY refreshBody;
  HMODULE hDll;
  setOfString* set;
  LPCTSTR fileName;
  svDllMan() : set(0), system(0), refresh(0), refreshBody(0), hDll(0), fileName(0) {}
  ~svDllMan() { delete set; delete []fileName; }
};
//----------------------------------------------------------------------------
static PVect<svDllMan> vSV_DLL;
//----------------------------------------------------------------------------
void flushDll()
{
  int nElem = vSV_DLL.getElem();
  for(int i = 0; i < nElem; ++i) {
    delete vSV_DLL[i].set;
    vSV_DLL[i].set = 0;
    }
  vSV_DLL.reset();
}
//----------------------------------------------------------------------------
void SV_System(DWORD msg)
{
  int nElem = vSV_DLL.getElem();
  for(int i = 0; i < nElem; ++i)
    (vSV_DLL[i].system)(msg);
}
//----------------------------------------------------------------------------
void SV_Refresh()
{
  int nElem = vSV_DLL.getElem();
  for(int i = 0; i < nElem; ++i)
    (vSV_DLL[i].refresh)();
}
//----------------------------------------------------------------------------
bool SV_RefreshBody(uint idBody, LPCTSTR pageName, bool force)
{
  int nElem = vSV_DLL.getElem();
  bool success = true;
  for(int i = 0; i < nElem; ++i)
    success &= (vSV_DLL[i].refreshBody)(idBody, pageName, force);
  return success;
}
//----------------------------------------------------------------------------
static criticalSect CsDll;
//----------------------------------------------------------------------------
LPCTSTR getLocalString(uint id, HINSTANCE hdll)
{
  criticalLock crtLck(CsDll);
  int nElem = vSV_DLL.getElem();
  for(int i = 0; i < nElem; ++i) {
    if(vSV_DLL[i].hDll == hdll)
      return vSV_DLL[i].set->getString(id);
    }
  return 0;
}
//----------------------------------------------------------------------------
bool reloadDllText(HINSTANCE hDll)
{
  int nElem = vSV_DLL.getElem();
  for(int i = 0; i < nElem; ++i) {
    if(vSV_DLL[i].hDll == hDll) {
      vSV_DLL[i].set->reset();
      if(isCrypted())
        return toBool(load_CriptedTxt(getDecryptCode(), *vSV_DLL[i].set, vSV_DLL[i].fileName));
      else {
        load_Txt(*vSV_DLL[i].set, vSV_DLL[i].fileName);
        return true;
        }
      }
    }
  return 0;
}
//----------------------------------------------------------------------------
// funzioni segnaposto che non fanno nulla
void dummy_SV_System(DWORD) {}
void dummy_SV_Refresh() {}
bool dummy_SV_RefreshBody(uint, LPCTSTR, bool) { return true; }
//----------------------------------------------------------------------------
extern int SV_DLL_DUMMY;
//----------------------------------------------------------------------------
//funzione di attivazione window degli allarmi
void showAlarm(PWin * par, perifsSet *perifs);
//----------------------------------------------------------------------------
static bool isSameOrParent(HWND parent, HWND check);
//----------------------------------------------------------------------------
static eDataFormat dataFormat =  European;
//----------------------------------------------------------------------------
eDataFormat whichData() { return dataFormat; }
//----------------------------------------------------------------------------
eDataFormat loadWhichData()
{
  LPCTSTR p = getString(ID_DATA_TYPE);
  if(p) {
    LPTSTR dummy;
    return(eDataFormat(_tcstol(p, &dummy, 10)));
    }
  return European;
}
//----------------------------------------------------------------------------
void showCursor();
void hideCursor();
//----------------------------------------------------------------------------
#define INIT_COUNT (-10)
//----------------------------------------------------------------------------
void checkTaskBar(bool set = true);
void checkSysKey(bool set);
static int dCount = INIT_COUNT;
//----------------------------------------------------------------------------
#define KEY_BASE_CRYPT 123
#define DIM_FAKE_JMP 8
//----------------------------------------------------------------------------
static BYTE decrypted[DIM_KEY_CRYPT + 2];
static bool Crypted;
bool isCrypted() { return Crypted; }
LPCBYTE getDecryptCode() { return decrypted; }
//----------------------------------------------------------------------------
static FARPROC getProc(HINSTANCE hI, LPCSTR name);
//----------------------------------------------------------------------------
static HMODULE hsvBase;
//----------------------------------------------------------------------------
#define CUR_SV_BASE_VER 0x07000001
//----------------------------------------------------------------------------
#define USE_SHOW_TRACE
#ifdef USE_SHOW_TRACE
  #define SHOW_TRACE(a, b) show_trace(a, b)
  bool show_trace(LPCTSTR msg1, LPCTSTR msg2)
  {
    TCHAR t[4096];
    wsprintf(t, _T("%s - %s"), msg1, msg2);
    MessageBox(0, t, _T("Error"), MB_OK | MB_ICONSTOP);
    return false;
  }
#else
  #define SHOW_TRACE(a, b) false
#endif
//----------------------------------------------------------------------------
bool checkCode();
//----------------------------------------------------------------------------
static bool performCheckCode(HMODULE hMod)
{
  Crypted = false;
  bool success = false;
  while(!success) {
    int offs = (GetTickCount() & 0xfc) | 8;
    typedef void (*SV_Fake)(bool&);
    BYTE cryptCode[DIM_KEY_CRYPT + 2 + DIM_FAKE_JMP];
    DWORD k = GetTickCount();
    *(LPDWORD)cryptCode = k;


    typedef void (*SV_Base)(LPBYTE buff);
    SV_Base bs = (SV_Base) getProc(hMod, "_SV_Ver");
    if(!bs) {
      SHOW_TRACE(_T("getProc(_SV_Ver)"), _T("failed"));
      break;
      }
    bs(cryptCode);
    *(LPDWORD)cryptCode ^= k;
    if(CUR_SV_BASE_VER > *(LPDWORD)cryptCode) {
      SHOW_TRACE(_T("cryptCode Ver"), _T("failed"));
      break;
      }
    DWORD ver = *(LPDWORD)cryptCode;

    bs = (SV_Base) getProc(hMod, "_SV_Base");
    if(!bs) {
      SHOW_TRACE(_T("getProc(_SV_Base)"), _T("failed"));
      break;
      }

    LPDWORD pdw = (LPDWORD)cryptCode;
    k = GetTickCount();
    pdw[0] = k;
    pdw[2] = offs ^ ver;
    pdw[3] = k;
    bs(cryptCode);
    pdw[0] ^= k;
    pdw[1] ^= ~k;

#if 0
// per creare la chiave vuota
    for(uint i = 0; i < DIM_KEY_CRYPT; ++i)
      cryptCode[i] = 0x20;
#endif
    crypt(cryptCode, decrypted, KEY_BASE_CRYPT, DIM_KEY_CRYPT);
    for(uint i = 0; i < DIM_KEY_CRYPT; ++i)
      if(decrypted[i] != 0x20) {
        Crypted = true;
        break;
        }
    success = true;
    break;
    }
  return success;
}
//----------------------------------------------------------------------------
bool checkCode()
{
  if(hsvBase)
    return true;
  bool success = false;
  while(!success) {
    hsvBase = LoadLibrary(NAME_SVBASE);
    if(!hsvBase) {
      SHOW_TRACE(_T("loadlibrary"), _T("failed"));
      break;
      }
    success = performCheckCode(hsvBase);
    break;
    }
  return success;
}
//---------------------------------------------------------------------
void releaseSvBase()
{
  if(!hsvBase)
    return;
  FreeLibrary(hsvBase);
  hsvBase = 0;
}
//---------------------------------------------------------------------
unsigned FAR PASCAL ReaderDllProc(void*)
{
  HMODULE hsv = LoadLibrary(NAME_SVBASE);
  if(hsv)
    FreeLibrary(hsv);
  return 0;
}
//---------------------------------------------------------------------
static uint idTimerCheck = 124578;
//---------------------------------------------------------------------
static void checkKey(HWND hwnd)
{
  bool success = false;
  DWORD idThread;
  HANDLE hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ReaderDllProc, 0, 0, &idThread);
  if(hThread)
    CloseHandle(hThread);
  uint tm = rand() % 30 + 10;
  tm *= 60 * 1000;
  KillTimer(hwnd, idTimerCheck);
  SetTimer(hwnd, idTimerCheck, tm, 0);
}
//---------------------------------------------------------------------
void addSymbFile(LPTSTR filename)
{
  TCHAR disk[_MAX_DRIVE];
  TCHAR dir[_MAX_DIR];
  TCHAR file[_MAX_FNAME] = _T("#");
  TCHAR ext[_MAX_FNAME];
  _tsplitpath_s(filename, disk, SIZE_A(disk), dir, SIZE_A(dir), file + 1, SIZE_A(file) - 1, ext, SIZE_A(ext));
  _tmakepath_s(filename, _MAX_PATH, disk, dir, file, ext);
}
//---------------------------------------------------------------------
void pageStrToSet(setOfString& sos, LPCTSTR filename)
{
  TCHAR file2[_MAX_PATH];
  _tcscpy_s(file2, SIZE_A(file2), filename);
  addSymbFile(file2);
  if(isCrypted()) {
    infoFileCr result;
    result.header = (LPCBYTE)CRYPT_HEAD;
    result.lenHeader = DIM_HEAD;

    if(decryptFile(file2, getDecryptCode(), DIM_KEY_CRYPT, CRYPT_STEP, result)) {
      uint dim = result.dim;
      LPTSTR res = autoConvert(result.buff, dim);
      sos.add(dim, res, true);
      }
    }
  else
    sos.add(file2);
  sos.add(filename);
}
//----------------------------------------------------------------------------
static svPrinter* gPrinter;
//----------------------------------------------------------------------------
svPrinter* getPrinter()
{
  if(!gPrinter) {
    mainClient* mc = getMain();
    gPrinter = new svPrinter(mc);
    }
  return gPrinter;
}
//----------------------------------------------------------------------------
static FARPROC getProc(HINSTANCE hI, LPCSTR name)
{
  // prima prova con l'underscore
  FARPROC f = GetProcAddress(hI, name);
  if(!f)
    // prova senza underscore
    f = GetProcAddress(hI, name + 1);
  return f;
}
//----------------------------------------------------------------------------
#define U_(a) ((unsigned)(a))
static
LPTSTR get_next(LPTSTR buff)
{
  LPCTSTR next = findNextParam(buff, 1);
  int offs = _tcslen(buff);
  if(next) {
    offs = next - buff - 1;
    buff[offs] = 0;
    }
  while(buff[offs] <= _T(' ')) {
    buff[offs] = 0;
    --offs;
    }
  return (LPTSTR)next;
}
//----------------------------------------------------------------------------
static void resolveDll(LPTSTR p)
{
  int ix = 0;
  do {
    LPTSTR next = get_next(p);

    HMODULE h = LoadLibrary(p);
    if(h) {
      vSV_DLL[ix].hDll = h;

      FARPROC f = getProc(h, FNAME_SYSTEM);
      if(f)
        vSV_DLL[ix].system = (SV_SYSTEM)f;
      else
        vSV_DLL[ix].system = dummy_SV_System;

      f = getProc(h, FNAME_REFRESH);
      if(f)
        vSV_DLL[ix].refresh = (SV_REFRESH)f;
      else
        vSV_DLL[ix].refresh = dummy_SV_Refresh;

      f = getProc(h, FNAME_BODY_REFRESH);
      if(f)
        vSV_DLL[ix].refreshBody = (SV_REFRESHBODY)f;
      else
        vSV_DLL[ix].refreshBody = dummy_SV_RefreshBody;
      TCHAR file[_MAX_PATH] = _T("#");
      _tcscpy_s(file + 1, SIZE_A(file) - 1, p);
      P_File::chgExt(file, PAGE_EXT);
      vSV_DLL[ix].fileName = str_newdup(file);
      vSV_DLL[ix].set = new setOfString;
      if(isCrypted())
        load_CriptedTxt(getDecryptCode(), *vSV_DLL[ix].set, file);
      else
        load_Txt(*vSV_DLL[ix].set, file);
      ++ix;
      }
    else {
      TCHAR t[_MAX_PATH * 2];
      wsprintf(t,_T("File -[ %s ]- mancante!!"), p);
      MessageBox(0, t, _T("Errore"), MB_OK | MB_ICONSTOP | MB_SYSTEMMODAL);
      PostQuitMessage(0);
      return;
      }
    p = next;
    } while(p);
}
//----------------------------------------------------------------------------
#include "set_cfg.h"
//----------------------------------------------------------------------------
enum msg4Dll { eINIT_DLL = 1, eEND_DLL };
//----------------------------------------------------------------------------
static mainClient* gMainClient;
mainClient* getMain() { return gMainClient; }
//----------------------------------------------------------------------------
mainClient::mainClient(PWin* parent, uint resId,  HINSTANCE hInstance)
  : baseClass(parent, resId),
    enableForwardFunctionKey(false), idTimerResetPsw(0), idTimer(0),
    JobData(0), mSecTimer(TIME_SLICE), Bd(0), enableHookFunctionKey(1),
    oldShow(0), timeBox(0)
{
  gMainClient = this;
  checkIfUseFilterCode();

  dataFormat = loadWhichData();
  config Confg;
  const cfg& Cfg = config::getAll();
  if(Cfg.ForceResolutionByUser) {
    SIZE sz = { Cfg.ResolutionByUserW, Cfg.ResolutionByUserH };
    sizer::setDim((sizer::eSizer)Cfg.ResolutionByUserType, &sz);
    }
  else {
    LPCTSTR p = ::getString(ID_RESOLUTION_BASE);
    if(p) {
      sizer::eSizer defRes = sizer::sOutOfRange;
      switch(_ttoi(p)) {
        case 0:
          break;
        case 1:
          defRes = sizer::s640x480;
          break;
        case 2:
          defRes = sizer::s800x600;
          break;
        case 3:
          defRes = sizer::s1024x768;
          break;
        case 4:
          defRes = sizer::s1280x1024;
          break;
        case 5:
          defRes = sizer::s1440x900;
          break;
        case 6:
          defRes = sizer::s1600x1200;
          break;
        case 7:
          defRes = sizer::s1680x1050;
          break;
        case 8:
          defRes = sizer::s1920x1440;
          break;
        case 100:
          p = findNextParamTrim(p, 3);
          if(p) {
            uint w = _ttoi(p);
            uint h = w / 2;
            p = findNextParamTrim(p);
            if(p)
              h = _ttoi(p);
            SIZE sz = { w, h };
            sizer::setDim(sizer::sPersonalized, &sz);
            }
          break;
        }
      sizer::setDim(defRes);
      }
    }
  Ft = allocFooter(this);

  Hd = allocHeader(this);

  RBody = PRect(0, 0, 0, 0);
  JobData = new gestJobData(this);
  useReqPrph.setDim(MAX_PERIF);
  for(uint i = 0; i < MAX_PERIF; ++i)
    useReqPrph[i] = false;
}
//----------------------------------------------------------------------------
#define SEND_MSG ::PostMessage
//#define SEND_MSG ::SendMessage
//----------------------------------------------------------
mainClient::~mainClient()
{
  delete JobData;
  checkTaskBar(false);
  checkSysKey(false);
  delete gPrinter;

#if 0
  SV_System(eEND_DLL);

  int nElem = vSV_DLL.getElem();
  for(int i = 0; i < nElem; ++i)
    FreeLibrary(vSV_DLL[i].hDll);
  --SV_DLL_DUMMY;
#endif
  destroy();
//  --SV_DLL_DUMMY;
//  flushPV(GlobalCam);
  flushPV(setMappedReq);
}
//----------------------------------------------------------
static TCHAR addedNameToTitle[_MAX_PATH];
//----------------------------------------------------------
void setCurrCaption(const PWin* par)
{
  smartPointerConstString sPS = getStringOrIdByLangGlob(ID_MAIN_TITLE);
  for(;;) {
    const PWin* parent = par->getParent();
    if(!parent)
      break;
    par = parent;
    }
  TCHAR buff[500];

  int appl = getVersionApp();
  wsprintf(buff, _T("%s - Ver. %d.%d.%d.%d"), &sPS,
        HIBYTE(APPL_BASE_VER),
        LOBYTE(APPL_BASE_VER),
        HIBYTE(appl),
        LOBYTE(appl)
        );
  if(*addedNameToTitle) {
    _tcscat_s(buff, SIZE_A(buff), _T(" - "));
    _tcscat_s(buff, SIZE_A(buff), addedNameToTitle);
    }
  SetWindowText(*par, buff);
}
//----------------------------------------------------------
HWND mainClient::setListBoxCallBack(infoCallBackSend* fz, uint ixLB)
{
  return Bd->setListBoxCallBack(fz, ixLB);
}
//----------------------------------------------------------
void mainClient::addInfoTitle(LPCTSTR name)
{
  _tcscpy_s(addedNameToTitle, SIZE_A(addedNameToTitle), name);
   setCurrCaption(this);
}
//----------------------------------------------------------
void mainClient::reloadText()
{
  resetTxt();
#ifdef ALARM_CRIPTED
  if(isCrypted()) {
    load_CriptedTxt(getDecryptCode());
    load_CriptedAlarm(getDecryptCode());
    }
  else {
    load_Txt();
    load_Alarm();
    }
#else
  if(isCrypted())
    load_CriptedTxt(getDecryptCode());
  else
    load_Txt();
  load_Global_Txt();
  load_Alarm();
#endif
  Hd->reloadText();
  setCurrCaption(this);
}
//----------------------------------------------------------
gestPerif::statMachine mainClient::isRunning()
{
  if(Perif.setFirst()) {
    do {
      const gestPerif *p = Perif.getCurr();
      gestPerif::statMachine st = p->isRunning();
      if(gestPerif::sNoCheck != st)
        return st;
      } while(Perif.setNext());
    }
  return gestPerif::sNoCheck;
}
//----------------------------------------------------------
void checkSysKey(bool set)
{
  static bool disabled = false;

  const cfg& Cfg = config::getAll();

  set &= Cfg.disableAltKey;

  if(set ^ !disabled)
    return;
  if(set)
    disabled = disableSysKey(true);
  else
    disabled = !disableSysKey(false);
}
//----------------------------------------------------------
void checkTaskBar(bool set)
{
  const cfg& Cfg = config::getAll();
  bool xor = !isVisibleTaskBar();
  xor ^= Cfg.hideTaskBar;
  if(set) {
    if(xor)
      P_ShowHideTaskBar(Cfg.hideTaskBar);
    }
  else {
    if(!xor)
      P_ShowHideTaskBar(false);
    }
}
//----------------------------------------------------------------------------
extern bool choosePageByList(PWin* parent, setOfString& set);
extern bool choosePageByTreeView(PWin* parent, setOfString& set);
extern bool setLang(PWin* parent);
//----------------------------------------------------------
void mainClient::pushBtn(int btn)
{
  static bool inExec;
  if(inExec)
    return;

  if(!Bd)
    return;

  inExec = true;
  switch(btn) { // per messaggi gestiti a livello generale

    case ID_FZ_PAGE_BY_CHOOSE:
    case ID_FZ_PAGE_BY_CHOOSE_TV:
      do {
        long dummy;
        setOfString& set = Bd->getInfo(dummy);
        bool success;
        if(ID_FZ_PAGE_BY_CHOOSE_TV == btn)
          success = choosePageByTreeView(this, set);
        else
          success = choosePageByList(this, set);
        if(success) {
          P_Body *newBody = Bd->pushedBtn(ID_OPEN_PAGE_BY_CHOOSE - MAX_BTN);
          if(newBody)
            initializeNewBody(newBody);
          }
        } while(false);
      break;
    case ID_FZ_LANGUAGE_CHG:
      do {
        const cfg& Cfg = config::getAll();
        int oldLang = Cfg.LangInUse;
        if(oldLang && setLang(Bd)) {
          if(oldLang != Cfg.LangInUse) {
            resetCacheLang(false);
            reloadTextAlarm();
            Hd->remakeByLang();
            reloadBody();
            }
          }
        } while(0);
      break;

    case ID_FZ_SETUP:
      do {
        PassWord psw;
        if(3 == psw.getPsW(3,this)) {
          const cfg& Cfg = config::getAll();
          int oldLang = Cfg.LangInUse;
          bool oldHide = Cfg.noShowPeriph;
          if(config().setup(Bd)) {
//            LPCTSTR p = getString(ID_NO_MEM_PATH_PAGE);
            if(oldLang != Cfg.LangInUse) {
              resetCacheLang(false);
              reloadTextAlarm();
              Hd->remakeByLang();
              }
            if(oldHide != Cfg.noShowPeriph) {
              perifsSet* pset = getPerifs();
              pset->sendHideShow(Cfg.noShowPeriph);
              }
            // forza un refresh dei dati, potrebbero essere cambiati
            // i normalizzatori
            JobData->setDirty();

            reloadBody();

            checkTaskBar();
            checkSysKey(true);
            }
          psw.restartTime(0);
          }
        } while(0);
      break;
    case ID_FZ_RESOLUTION_CHG:
      do {
        bool changed = false;
        if(IDOK == DChooseResolution(this, changed).modal()) {
          if(changed) {
            reloadBody();
            calcCurrRes();
            EvSize(oldShow, true);

            checkTaskBar();
            checkSysKey(true);
            }
          }
        } while(false);
      break;
    // la gestione degli allarmi viene delegata ai singoli controlli
    // se non presente flag di attivazione
    case ID_F12:
      if(Bd->usePredefBtn(ID_F12)) {
        showAlarm(this, &Perif);
        break;
        }
      // fall through
    default:
      do {
        P_Body *newBody = Bd->pushedBtn(btn);
        if(newBody)
          initializeNewBody(newBody);
        } while(0);
      break;
    }
  inExec = false;
}
//----------------------------------------------------------
void mainClient::calcCurrRes()
{
  LONG_PTR v = GetWindowLongPtr(*getParent(), GWL_STYLE);
  PRect t;
  GetWindowRect(*Bd, t);
  PRect r(t);
  GetWindowRect(*Ft, t);
  r |= t;
  GetWindowRect(*Hd, t);
  r |= t;
  AdjustWindowRect(r, v, false);
  getParent()->setWindowPos(0, r, SWP_NOMOVE);
}
//----------------------------------------------------------
void mainClient::reloadBody()
{
  uint id = Bd->Attr.id;
  LPTSTR page = 0;
  if(IDD_STANDARD == id) {
    id = 0;
    }
  P_DefBody* dBd = dynamic_cast<P_DefBody*>(Bd);
  if(dBd)
    page = str_newdup(dBd->getPageName());

  LPCTSTR p = getString(ID_NO_MEM_PATH_PAGE);
  bool useStack = !p || !_ttoi(p);

  if(!useStack) {
    // forza la diversità tra i dati vecchi e nuovi per ricaricare la stessa pagina
    P_Body* bd = getBody(this, IDD_MAIN_MENU == id ? 0 : page, 0, id, id - 1);
    initializeNewBody(bd);
    }

  // il body empty non fa altro che uscire e fa quindi ricaricare
  // il body precedente
  else {
    Bd->destroyModal();
    hideCam();
    SV_RefreshBody(ID_BODY_FOR_EXIT, dBd->getPageName(), true);
    P_Body* oldBd = Bd;
    Bd = getBody(this, 0, page, IDD_EMPTY, id);
    Bd->create();
    delete oldBd;
  }
  delete []page;
}
//----------------------------------------------------------
void mainClient::initializeNewBody(P_Body *newBody)
{
  // in win9x se si hanno il footer e l'header nascosti la distruzione del body
  // fa si che non restino window attive e va in tilt. Allora si posticipa la
  // distruzione del vecchio dopo la creazione del nuovo.

  // non è più necessario, la cache è locale alla pagina
//  resetCacheLang(true);

  P_DefBody* dBd = dynamic_cast<P_DefBody*>(Bd);
  SV_RefreshBody(ID_BODY_FOR_EXIT, dBd->getPageName(), true);

  uint nP = useReqPrph.getElem();
  for(uint j = 0; j < nP; ++j) {
    if(useReqPrph[j])
      sendClearListReq(j + WM_PLC);
    }
  Bd->destroyModal();
  Bd->releasePress();
  hideCam();
  Bd->sendListValue(false);
#if 1
  P_Body* oldBd = Bd;
#else
  P_Body* oldBd = 0;
  if(isWinNT_Based())
    delete Bd;
  else
    oldBd = Bd;
#endif
  Bd = newBody;
  long id;
  setOfString& set = Bd->getInfo(id);
  Bd->create();

  DWORD typeShow = 0;
  LPCTSTR p = set.getString(ID_CODE_PAGE_TYPE);
  p = findNextParam(p, 1);
  if(p)
    typeShow = _ttoi(p);

  EvSize(typeShow, oldShow != typeShow);
  oldShow = typeShow;

  Bd->setWindowPos(0, RBody, SWP_NOZORDER);
  Ft->setInfoBtn(set, id);
  smartPointerConstString tit = Bd->getTitle();
  Hd->setTitle(tit);

  // informa il body che è pronto per funzionare
  Bd->setReady(false);
  Ft->setVersion(MAKELONG(APPL_BASE_VER, getVersionApp()), Bd->getVersionBody());
  Bd->sendListValue(true);

  SetFocus(*Bd);
/*
  int nElem = GlobalCam.getElem();
  for(int i = 0; i < nElem; ++i) {
    PVarCam* cam = GlobalCam[i]->getCam();
    cam->setOwner(Bd);
    }
*/
  delete oldBd;
}
//----------------------------------------------------------
void mainClient::gotoPreviousPage()
{
  P_Body *newBody = Bd->getPreviousPage();
  if(newBody)
    initializeNewBody(newBody);
  else
    shutDown();
}
//----------------------------------------------------------
void mainClient::shutDown(bool passedPsw)
{
  static bool inExec = false;
  if(inExec)
    return;
  inExec = true;
  if(!passedPsw) {
    LPCTSTR p = getString(ID_END_PSW);
    int level;
    if(p && (level = _ttoi(p))) {
      PassWord psw;
      if(level != psw.getPsW(level, this))
        return;
       psw.restartTime(0);
      }
    }
  if(msgBoxByLangGlob(this, ID_END_MSG, ID_END_TITLE, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES) {
    CloseApp();
    ::PostQuitMessage(EXIT_SUCCESS);
    }
  else
    inExec = false;
}
//----------------------------------------------------------
void showCursor()
{
  ClipCursor(0);
  ShowCursor(true);
}
//----------------------------------------------------------
void hideCursor()
{
  PRect r(30, 50, 31, 51);
  ClipCursor(r);
  SetCursorPos(0,0);
  ShowCursor(false);
}
//----------------------------------------------------------
#define _REG(a) const UINT WM_##a = RegisterWindowMessage(_T("WM_") _T(#a))

  _REG(MAIN_PRG);
  _REG(CUSTOM);

  _REG(PRF_1);
//----------------------------------------------------------
static bool setNumLock(bool set, bool& old)
{
  old = set;
  if(IS_WINNT_BASED) {
    // assume che il valore true per il tipo bool valga uno
    bool needChange = set ^ (GetKeyState(VK_NUMLOCK) & 1);
    if(needChange) {
      old = !set;
      BYTE scan = static_cast<BYTE>(MapVirtualKey(VK_NUMLOCK, 0));
      keybd_event(static_cast<BYTE>(VK_NUMLOCK), scan, 0, 0);
      keybd_event(static_cast<BYTE>(VK_NUMLOCK), scan, KEYEVENTF_KEYUP, 0);
      }
    }
  else {
    BYTE pbKeyState[256];
    GetKeyboardState(reinterpret_cast<LPBYTE>(&pbKeyState));
    old = pbKeyState[VK_NUMLOCK] & 1;
    if(set)
      pbKeyState[VK_NUMLOCK] |= 1;
    else
      pbKeyState[VK_NUMLOCK] &= ~1;
    SetKeyboardState(reinterpret_cast<LPBYTE>(&pbKeyState));
    }
  return true;
}
//----------------------------------------------------------
static bool oldNumLock;
//----------------------------------------------------------
#ifndef SPI_SETFOREGROUNDLOCKTIMEOUT

#define SPI_GETACTIVEWINDOWTRACKING         0x1000
#define SPI_SETACTIVEWINDOWTRACKING         0x1001
#define SPI_GETMENUANIMATION                0x1002
#define SPI_SETMENUANIMATION                0x1003
#define SPI_GETCOMBOBOXANIMATION            0x1004
#define SPI_SETCOMBOBOXANIMATION            0x1005
#define SPI_GETLISTBOXSMOOTHSCROLLING       0x1006
#define SPI_SETLISTBOXSMOOTHSCROLLING       0x1007
#define SPI_GETGRADIENTCAPTIONS             0x1008
#define SPI_SETGRADIENTCAPTIONS             0x1009
#define SPI_GETMENUUNDERLINES               0x100A
#define SPI_SETMENUUNDERLINES               0x100B
#define SPI_GETACTIVEWNDTRKZORDER           0x100C
#define SPI_SETACTIVEWNDTRKZORDER           0x100D
#define SPI_GETHOTTRACKING                  0x100E
#define SPI_SETHOTTRACKING                  0x100F
#define SPI_GETFOREGROUNDLOCKTIMEOUT        0x2000
#define SPI_SETFOREGROUNDLOCKTIMEOUT        0x2001
#define SPI_GETACTIVEWNDTRKTIMEOUT          0x2002
#define SPI_SETACTIVEWNDTRKTIMEOUT          0x2003
#define SPI_GETFOREGROUNDFLASHCOUNT         0x2004
#define SPI_SETFOREGROUNDFLASHCOUNT         0x2005

#endif
//----------------------------------------------------------
extern bool isOnSplash();
//----------------------------------------------------------
void mainClient::showPart()
{
  if(!isOnSplash()) {
    LPCTSTR p = getString(ID_HIDE_PART);
    int hidePart = 0;
    if(p)
      hidePart = _ttoi(p);
    if(!(hidePart & 2))
      ShowWindow(*Ft, SW_SHOWNORMAL);

    if(!(hidePart & 1))
      ShowWindow(*Hd, SW_SHOWNORMAL);
    ShowWindow(*Bd, SW_SHOWNORMAL);
    SetForegroundWindow(*getParent());
    }
}
//----------------------------------------------------------
#define ID_TIMER 1
//----------------------------------------------------------
bool mainClient::create()
{
  if(!baseClass::create())
    return false;

  cfg& cfg_ = const_cast<cfg&>(config::getAll());
  if(!cfg_.useMouse)
    hideCursor();

  LPCTSTR p = ::getString(ID_LANGUAGE);
  int currL = p ? _ttoi(p) : 0;
  if(!cfg_.LangInUse || !currL)
    cfg_.LangInUse = currL;

  Bd = allocMainMenu(this);
  Bd->create();
  long id;
  setOfString& set = Bd->getInfo(id);

  p = ::getString(ID_HIDE_PART);
  if(p) {
    oldShow = _ttoi(p);
    if(oldShow == (DWORD)-1)
      oldShow = 0;
    }
  p = set.getString(ID_CODE_PAGE_TYPE);
  p = findNextParam(p, 1);
  DWORD v = oldShow;
  if(p) {
    v = _ttoi(p);
    if(v == (DWORD)-1)
      v = 0;
    }

  if(oldShow != v) {
    EvSize(v, true);
    oldShow = v;
    }
  else
    Bd->setWindowPos(0, RBody, SWP_NOZORDER);

  // occorre richiamare prima il footer, vedere sopra
  Ft->setInfoBtn(set, id);
  smartPointerConstString tit = Bd->getTitle();
  Hd->setTitle(tit);
  P_BaseBody *pBb = dynamic_cast<P_BaseBody*>(Hd);
  if(pBb)
    pBb->setReady(true);
  Bd->setReady(true);
  Ft->setVersion(MAKELONG(APPL_BASE_VER, getVersionApp()), Bd->getVersionBody());
  setCurrCaption(this);

  SetFocus(*Bd);
  SetTimer(*this, idTimer = ID_TIMER, mSecTimer, 0);
  SetTimer(*this, idTimerCheck, 5 * 60 * 1000, 0);

  setHotKey();
  setNumLock(cfg_.numLock, oldNumLock);

  checkTaskBar();
  checkSysKey(true);
#if 0
  extern bool canLoadDll();
  if(canLoadDll() /* !cfg_.disableDLL */ ) {
    p = getString(ID_DLL_NAME);
    if(p) {
      LPTSTR buff = str_newdup(p);
      resolveDll(buff);
      delete []buff;
      ++SV_DLL_DUMMY;
      SV_System(eINIT_DLL);
      }
    }
#endif
  check_old_rep();
  return true;
}
//---------------------------------------------------------------
void mainClient::setTimeForTimer(uint msec)
{
  if(!msec)
    return;
  if(idTimer) {
    KillTimer(*this, idTimer);
    idTimer = 0;
    }
  SetTimer(*this, idTimer = ID_TIMER, msec, 0);
  mSecTimer = msec;
}
//---------------------------------------------------------------
void mainClient::setStatus(LPCTSTR msgStatus)
{
  Ft->setMsgStatus(msgStatus);
}
//---------------------------------------------------------------
bool mainClient::isEnabledBtn(int idBtn)
{
  return Ft->isEnabledBtn(idBtn);
}
//---------------------------------------------------------------
void mainClient::EvSize(DWORD typeShow, bool force)
{
  static SIZE oldSize = { 0, 0 };
  static int hCaption = -1;
  if(!getHandle())
    return;
  if(hCaption < 0) {
    hCaption = GetSystemMetrics(SM_CYCAPTION);
    LPCTSTR p = getString(ID_HIDE_BAR);
    if(p && 1 == _ttoi(p))
      hCaption = 0;
    }

  if(force || uint(oldSize.cx) != sizer::getWidth() ||
         uint(oldSize.cy) != sizer::getHeight()) {
    oldSize.cx = sizer::getWidth();
    oldSize.cy = sizer::getHeight();
    PRect rect(0, 0, oldSize.cx, oldSize.cy - hCaption);
    PRect rect2 = rect;
    if(force) {
      if(!(typeShow & 1))
        rect2.bottom = Hd->trueHeight();
      else
        rect2.bottom = 0;
      }
    else
      rect2.bottom = Hd->Height();

    DWORD flag = SWP_NOZORDER;
    if(rect2.Height())
      flag |= SWP_SHOWWINDOW;
    else
      flag |= SWP_HIDEWINDOW;
    Hd->setWindowPos(0, rect2, flag);

    timeBox = rect2.bottom ? 0 : Hd->getTimeBox();
    if(timeBox) {
      LPCTSTR p = getString(ID_HIDE_TIME);
      if(p && _ttoi(p))
        timeBox = 0;
      }

    if(force) {
      if(!(typeShow & 2))
        rect.top = rect.bottom - Ft->trueHeight();
      else
        rect.top = rect.bottom;
      }
    else
      rect.top = rect.bottom - Ft->Height();

    flag = SWP_NOZORDER;
    if(rect.Height())
      flag |= SWP_SHOWWINDOW;
    else
      flag |= SWP_HIDEWINDOW;
    Ft->setWindowPos(0, rect, flag);


    RBody = PRect(rect2.left, rect2.bottom, rect2.right, rect.top);
    if(Bd)
      Bd->setWindowPos(0, RBody, SWP_NOZORDER | SWP_SHOWWINDOW);
    }
}
//----------------------------------------------------------------------------
static HHOOK HookHandle;
static HWND HWTarget;
//----------------------------------------------------------------------------
void waitWhileKillFocus()
{
#define CHECK_TWO_FOCUS
#ifdef CHECK_TWO_FOCUS
  HWND last;
  for(int j = 0; j < 2; ++j) {
    HWND hFocus = GetForegroundWindow();
    for(int i = 0; i < 5; ++i) {
      SleepEx(20, 0);
      last = GetForegroundWindow();
      if(last != hFocus)
        break;
      }
    }
 #else
   // se non sappiamo quante window diventano attive prima del menù di start
   // purtroppo il tempo dipende dalla velocità del pc e della scheda grafica
   // nel mio ha avuto successo fino a 20.
   SleepEx(200, 0);
 #endif
 }
//----------------------------------------------------------------------------
static int checkExtendKey(int key, int ctrl, int shift)
{
  if(ctrl && shift)
    return 0;

  if(ctrl > 0)
    return key - VK_F1 + ID_CTRL_F1;
  if(shift > 0)
    return key - VK_F1 + ID_SHIFT_F1;
  if(!ctrl && !shift)
    return key - VK_F1 + ID_F1;
  return 0;
}
//----------------------------------------------------------------------------
static bool needKeyBlock(int code, WPARAM wParam, LPARAM lParam)
{
  const cfg& Cfg = config::getAll();
  if(IS_WINNT_BASED) {
    if(HC_ACTION > code)
      return false;
    switch(wParam) {
      case WM_SYSKEYDOWN:
      case WM_KEYDOWN:
        break;
      default:
        return false;
      }
    KBDLLHOOKSTRUCT *pkbhs = (KBDLLHOOKSTRUCT*)lParam;
    if(Cfg.disableCtrlKey && (GetAsyncKeyState (VK_CONTROL) >> ((sizeof(SHORT) * 8) - 1)))
      return true;

    if(Cfg.disableAltKey && (pkbhs->flags & LLKHF_ALTDOWN))
      return true;
    }
  else {
    switch(wParam) {
      case VK_CONTROL:
      case VK_MENU:
        break;

      default:
        return false;
      }

    if(VK_CONTROL == wParam) {
      if(Cfg.disableCtrlKey)
        return true;
      }
    else
      if(Cfg.disableAltKey)
        return true;
    }
  return false;
}
//----------------------------------------------------------------------------
#define MIN_TIME_TICK 500
extern bool isActiveApp();
//----------------------------------------------------------------------------
LRESULT CALLBACK KeyboardProcEx(int code, WPARAM wParam, LPARAM lParam)
{
  if(code < 0)
    return CallNextHookEx(HookHandle, code, wParam, lParam);

  if(IS_WINNT_BASED) {
    if((HIWORD(lParam) & KF_UP)) {
      switch(wParam) {
        case VK_LWIN:
        case VK_RWIN:
          do {
            const cfg& Cfg = config::getAll();
            if(Cfg.disableWinKey) {
              click(VK_MENU);
              waitWhileKillFocus();
              SetForegroundWindow(HWTarget);
              }
            } while(false);
          break;
        }
      }
    }

  bool keyDown = true;
  WPARAM key = wParam;
  if(IS_WINNT_BASED) {
    KBDLLHOOKSTRUCT *pkbhs = (KBDLLHOOKSTRUCT*)lParam;
    switch(wParam) {
      case WM_KEYUP:
      case WM_SYSKEYUP:
        keyDown = false;
      }
    key = pkbhs->vkCode;
    }
  else
    keyDown = !(HIWORD(lParam) & KF_UP);

  if(keyDown) {
    const cfg& Cfg = config::getAll();
    bool CtrlPressed = toBool(GetAsyncKeyState(VK_CONTROL) & (1 << 15));
    bool ShiftPressed = toBool(GetAsyncKeyState(VK_SHIFT) & (1 << 15));
    static bool oldCTRL;
    static DWORD lastTick;
    DWORD tick = GetTickCount();
    if(!Cfg.disableCtrlKey)
      oldCTRL = CtrlPressed;
    else {
      DWORD tick = GetTickCount();
      if(oldCTRL && !CtrlPressed) {
        if(tick - lastTick > MIN_TIME_TICK)
          oldCTRL = false;
        }
      else if(CtrlPressed) {
        oldCTRL = true;
        lastTick = tick;
        }
      }
    int msg = 0;
    bool needBlock = false;
    switch(key) {
      case 0x31:
      case VK_NUMPAD1:
        if(oldCTRL && !ShiftPressed)
          msg = ID_FZ_SETUP;
        break;

      case 0x32:
      case VK_NUMPAD2:
        if(oldCTRL && !ShiftPressed)
          msg = ID_FZ_PAGE_BY_CHOOSE;
        break;
      case 0x33:
      case VK_NUMPAD3:
        if(oldCTRL && !ShiftPressed)
          msg = ID_FZ_PAGE_BY_CHOOSE_TV;
        break;
      case 0x34:
      case VK_NUMPAD4:
        if(oldCTRL && !ShiftPressed)
          msg = ID_FZ_LANGUAGE_CHG;
        break;
      case 0x35:
      case VK_NUMPAD5:
        if(oldCTRL && !ShiftPressed)
          msg = ID_FZ_RESOLUTION_CHG;
        break;


      case VK_F1:
      case VK_F2:
      case VK_F3:
      case VK_F4:
      case VK_F5:
      case VK_F6:
      case VK_F7:
      case VK_F8:
      case VK_F9:
      case VK_F10:
      case VK_F11:
      case VK_F12:
        msg = checkExtendKey(key, oldCTRL, ShiftPressed);
        break;
      default:
        needBlock = needKeyBlock(code, wParam, lParam);
        break;
      }
    if(msg > 0) {
      oldCTRL = 0;
      if(isActiveApp()) {
        ::PostMessage(HWTarget, WM_MY_HOTKEY, msg, 0);
//      if(VK_F10 == wParam)
        return 1;
        }
      }
    if(needBlock && IS_WINNT_BASED) {
      up_click(key);
      return 1;
      }
    return CallNextHookEx(HookHandle, code, wParam, lParam);
    }
  return CallNextHookEx(HookHandle, code, wParam, lParam);
}
//----------------------------------------------------------------------------
#if 0
LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
  if(code < 0)
    return CallNextHookEx(HookHandle, code, wParam, lParam);
  if(IS_WINNT_BASED) {
    if((HIWORD(lParam) & KF_UP)) {
      switch(wParam) {
        case VK_LWIN:
        case VK_RWIN:
          do {
            const cfg& Cfg = config::getAll();
            if(Cfg.disableWinKey) {
              click(VK_MENU);
              waitWhileKillFocus();
              SetForegroundWindow(HWTarget);
              }
            } while(false);
          break;
        }
      }
    }
  bool keyDown = true;
  WPARAM key = wParam;
  if(IS_WINNT_BASED) {
    KBDLLHOOKSTRUCT *pkbhs = (KBDLLHOOKSTRUCT*)lParam;
    switch(wParam) {
      case WM_KEYUP:
      case WM_SYSKEYUP:
        keyDown = false;
      }
    key = pkbhs->vkCode;
    }
  else
    keyDown = !(HIWORD(lParam) & KF_UP);

  if(keyDown) {
    const cfg& Cfg = config::getAll();
    int CtrlPressed = (GetAsyncKeyState(VK_CONTROL) & (1 << 15)) ? -1 : 0;
    int ShiftPressed = (GetAsyncKeyState(VK_SHIFT) & (1 << 15)) ? -1 : 0;

    static bool oldCTRL;
    static DWORD lastTick;
    DWORD tick = GetTickCount();
    if(oldCTRL && !CtrlPressed) {
      if(tick - lastTick > MIN_TIME_TICK)
        oldCTRL = false;
      }
    else if(CtrlPressed) {
      oldCTRL |= CtrlPressed;
      lastTick = tick;
      }
    int msg = 0;
    bool needBlock = false;
    switch(key) {
      case 0x31:
      case VK_NUMPAD1:
        if(oldCTRL && !ShiftPressed)
          msg = ID_FZ_SETUP;
        break;
      case 0x32:
      case VK_NUMPAD2:
        if(oldCTRL && !ShiftPressed)
          msg = ID_FZ_PAGE_BY_CHOOSE;
        break;
      case 0x33:
      case VK_NUMPAD3:
        if(oldCTRL && !ShiftPressed)
          msg = ID_FZ_PAGE_BY_CHOOSE_TV;
        break;
      case 0x34:
      case VK_NUMPAD4:
        if(oldCTRL && !ShiftPressed)
          msg = ID_FZ_LANGUAGE_CHG;
        break;
      case 0x35:
      case VK_NUMPAD5:
        if(oldCTRL && !ShiftPressed)
          msg = ID_FZ_RESOLUTION_CHG;
        break;

      case VK_F1:
/**/
        if(oldCTRL && !ShiftPressed) {
          msg = ID_FZ_SETUP;
          break;
          }
/**/
        // fall through
      case VK_F2:
      case VK_F3:
      case VK_F4:
      case VK_F5:
      case VK_F6:
      case VK_F7:
      case VK_F8:
      case VK_F9:
      case VK_F10:
      case VK_F11:
      case VK_F12:
        msg = checkExtendKey(key, oldCTRL, ShiftPressed);
        break;

      default:
        needBlock = needKeyBlock(code, wParam, lParam);
        break;
      }

    if(msg > 0) {
      oldCTRL = 0;
      ::PostMessage(HWTarget, WM_MY_HOTKEY, msg, 0);
#if 1
      // non fa processare il messaggio

      // purtroppo causa un crash (almeno nel'ambiente del BC).
      // Occorre allora non posizionare
      // sul pulsante F10 un evento che apra una MessageBox(). Il tasto
      // F10 equivale al tasto ALT e fa attivare il menù di sistema, per cui
      // nella MessageBox() si attiva il menù (lo stesso nelle finestre
      // con il flag del sysmenù attivo).
      // Non ho verificato se tornando zero succeda lo stesso.
      // Ho provato anche a far processare alla CallNextHookEx() un wParam
      // diverso, (cambiato in VK_F9), ma sembra che non cambi nulla,
      // forse avrei dovuto cambiare anche nella tabella buffer dei caratteri

      // Sembra che ora non dia problemi, solo che non permette
      // che il messaggio arrivi alle window che lo elaborerebbero
      // direttamente (es. Preview di stampa).
      // Meglio intercettare ed eliminare solo quello che da più fastidio.
//      if(VK_F10 == key)
        return 1;
#endif
      }
    if(IS_WINNT_BASED && needBlock) {
      up_click(key);
      return 1;
      }
    return CallNextHookEx(HookHandle, code, wParam, lParam);
    }
  return CallNextHookEx(HookHandle, code, wParam, lParam);
}
#endif
//----------------------------------------------------------------------------
void mainClient::setHotKey()
{
  HWTarget = getHandle();
  LPCTSTR p = getString(ID_USE_EXTENTED_KEYB);
  int hook = IS_WINNT_BASED ? WH_KEYBOARD_LL : WH_KEYBOARD;
#if 1
//  if(p && _ttoi(p))
    HookHandle = SetWindowsHookEx(hook, KeyboardProcEx, GetModuleHandle(0), 0);
//  else
//    HookHandle = SetWindowsHookEx(hook, KeyboardProc, GetModuleHandle(0), 0);
#else
  if(p && _ttoi(p))
    HookHandle = SetWindowsHookEx(hook, KeyboardProcEx, 0, ::GetCurrentThreadId());
  else
    HookHandle = SetWindowsHookEx(hook, KeyboardProc, 0, ::GetCurrentThreadId());
#endif
}
//----------------------------------------------------------
void mainClient::unsetHotKey()
{
  UnhookWindowsHookEx(HookHandle);
}
//----------------------------------------------------------
bool infoReqRead::sendData(HWND driver, uint prph, const PVect<DWORD>& data, bool oneReq)
{
  id = rand();
  tick = GetTickCount();
  idPrph = prph;

  TCHAR buff[_MAX_PATH];
  MAKE_MAPPED_FILENAME(buff, id, tick);
  pmf = new p_MappedFile(buff, mP_MEM_ONLY);
  uint len = data.getElem();
  if(!pmf->P_open((len + 1) * sizeof(DWORD))) {
    delete pmf;
    pmf = 0;
    return false;
    }
  tWrapMappedFile<DWORD> twF(*pmf);
  twF[0] = (DWORD)len;
  for(uint i = 0; i < len; ++i)
    twF[i + 1] = data[i];
  uint msg = oneReq ? MSG_ENABLE_ONE_READ : MSG_ENABLE_READ;
  PostMessage(driver, WM_MAIN_PRG, MAKEWPARAM(msg, id), (LPARAM)tick);
  return true;
}
//----------------------------------------------------------
#define MAX_TIMEOUT_REQ_RESP 20000
//----------------------------------------------------------
bool infoReqRead::onTimeout(uint prph)
{
  if(idPrph != prph)
    return false;

  DWORD t = GetTickCount();
  DWORD t2 = t - tick;
  if(t2 < MAX_TIMEOUT_REQ_RESP)
    return false;

  if(t < tick)
    t2 += (DWORD)-1;
  if(t2 < MAX_TIMEOUT_REQ_RESP)
    return false;

  return true;
}
//----------------------------------------------------------
bool infoReqRead::isEqu(WORD w, DWORD dw)
{
  return w == id && dw == tick;
}
//----------------------------------------------------------
bool infoReqRead::isPrph(uint prph)
{
  return prph == idPrph;
}
//----------------------------------------------------------
bool mainClient::getListPrph(PVect<bool>& set)
{
  uint nP = useReqPrph.getElem();
  bool hasReq = false;
  for(uint i = 0; i < nP; ++i) {
    set[i] = useReqPrph[i];
    hasReq |= set[i];
    }

  uint t = set.getElem();
  for(uint i = nP; i < t; ++i)
    set[i] = false;
  return hasReq;
}
//----------------------------------------------------------
void mainClient::checkMappedFile()
{
  uint nP = useReqPrph.getElem();
  for(uint j = 0; j < nP; ++j) {
    if(useReqPrph[j]) {
      bool timeout = false;
      uint nElem = setMappedReq.getElem();
      for(uint i = 0; i < nElem; ++i) {
        if(setMappedReq[i]->onTimeout(j + WM_PLC)) {
          timeout = true;
          useReqPrph[j] = false;
          break;
          }
        }
      if(timeout) {
        for(int i = nElem - 1; i >= 0; --i) {
          if(setMappedReq[i]->isPrph(j + WM_PLC))
            delete setMappedReq.remove(i);
          }
        }
      }
    }
}
//----------------------------------------------------------
void mainClient::sendClearListReq(uint idPrph)
{
  gestPerif* prph = getPerif(idPrph);
  if(!prph)
    return;
  HWND hwd = prph->getHwndDriver();
  if(!hwd)
    return;
  PostMessage(hwd, WM_MAIN_PRG, MAKEWPARAM(MSG_ENABLE_READ, 0), (LPARAM)0);
}
//----------------------------------------------------------
void mainClient::addToMappedFile(uint idPrph, const PVect<DWORD>& data, bool oneReq)
{
  gestPerif* prph = getPerif(idPrph);
  if(!prph)
    return;
  if(!prph->hasRequestVar())
    return;
  HWND hwd = prph->getHwndDriver();
  if(!hwd)
    return;
  infoReqRead* irr = new infoReqRead;
  if(irr->sendData(hwd, idPrph, data, oneReq)) {
    uint nElem = setMappedReq.getElem();
    setMappedReq[nElem] = irr;
    }
}
//----------------------------------------------------------
void mainClient::removeMappedFile(WORD wP, DWORD lP)
{
  uint nElem = setMappedReq.getElem();
  for(uint i = 0; i < nElem; ++i)
    if(setMappedReq[i]->isEqu(wP, lP)) {
      delete setMappedReq.remove(i);
      break;
      }
}
//----------------------------------------------------------------------------
bool mainClient::preProcessMsg(MSG& msg)
{
  return toBool(IsDialogMessage(getHandle(), &msg));
}
//----------------------------------------------------------
LRESULT mainClient::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  gestPerif *p = Perif.getByMsg(message);
  if(p && (hwnd == *this || hwnd == *getParent()))
    if(p->acceptMsg(message, wParam, lParam))
      return lParam;

  if(WM_PRF_1 == message) {
    switch(LOWORD(wParam)) {
      case MSG_CHANGED_DATA:
        JobData->load();
        break;
      case MSG_SAVE_JOB:
        JobData->save();
        break;
      }
    }
  else if(WM_MAIN_PRG == message) {
    switch(LOWORD(wParam)) {
      case MSG_ENABLE_READ:
      case MSG_ENABLE_ONE_READ:
        removeMappedFile(HIWORD(wParam), lParam);
        break;
      }
    }
  else if(WM_CUSTOM == message) {
    switch(LOWORD(wParam)) {
      case MSG_PREV_PAGE:
        gotoPreviousPage();
        break;
      case MSG_CLOSE_PROG:
        shutDown(toBool(HIWORD(wParam)));
        break;
      case MSG_SHOW_ALARM:
        showAlarm(this, &Perif);
        break;

      case MSG_SHOW_TREEVIEW:
        do {
          static vCodeType code;
          static vRangeTime time;
          TD_Choose_FilterTree(Bd, code, time).modal();
          } while(false);
        break;

      case MSG_PRINT_TREEVIEW:
        do {
          static vCodeType code;
          static vRangeTime time;
          TD_Choose_PrintTree(Bd, code, time).modal();
          } while(false);
        break;

      case MSG_PRINT_SCREEN:
        do {
          bool preview = toBool(LOWORD(lParam));
          bool setup   = toBool(HIWORD(lParam));
          printScreen(getCurrBody(), preview, setup, toBool(HIWORD(wParam)));
          } while(false);
        break;

      case MSG_SHOW_KEYB:
        Bd->postOpenKeyboard((PEdit*)lParam);
        break;
      }
    }
  else switch(message) {
    case WM_DESTROY:
      CloseApp(hwnd);
      break;

    case WM_TIMER:
      if(EvTimer(wParam))
        return 0;
      break;

    case WM_ERASEBKGND:
      return 1;

    case WM_SIZE:
      EvSize(0, false);
      break;

    case WM_POST_OPEN_NEW_PAGE_BODY:
      pushBtn(wParam);
      break;

    case WM_MY_HOTKEY:
      if(enableHookFunctionKey <= 0)
        break;
      switch(wParam) {
        case ID_FZ_LANGUAGE_CHG:
        case ID_FZ_PAGE_BY_CHOOSE:
        case ID_FZ_PAGE_BY_CHOOSE_TV:
        case ID_FZ_SETUP:
        case ID_FZ_RESOLUTION_CHG:
          enableForwardFunctionKey = 0;
          pushBtn(wParam);
          return true;
        default:
          if(wParam >= ID_F1 && wParam <= ID_F12) {
            if(enableForwardFunctionKey > 0) {
              HWND hw = GetForegroundWindow();
              if(isSameOrParent(*getParent(), hw))
                ::PostMessage(hw, WM_COMMAND, wParam - ID_F1 + IDC_BUTTON_F1, 0);
              }
            else {
              enableForwardFunctionKey = 0;
              pushBtn(wParam);
              }
            return true;
            }
          if(wParam >= ID_SHIFT_F1 && wParam <= ID_SHIFT_F12 || wParam >= ID_CTRL_F1 && wParam <= ID_CTRL_F12) {
            if(!enableForwardFunctionKey)
              if(Bd->isEnabledBtn(wParam))
                pushBtn(wParam);
            return true;
            }
          break;
        }
      break;
    case WM_SHOW_HIDE_PRPH:
      show_hide_periph(toBool(wParam));
      break;
    case WM_POST_RESET_PASSWORD:
      do {
        DWORD maxTimePsw = MAX_TIME_PSW;
        LPCTSTR p = getString(ID_PASSWORD_TIME_LEASE);
        if(p) {
          maxTimePsw = _ttoi(p) * 1000;
          if(!maxTimePsw)
            maxTimePsw = 500;
          }
        SetTimer(*this, idTimerResetPsw = ID_TIMER + 1, maxTimePsw, 0);
        } while(false);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------
void mainClient::checkRunOrStop(bool force, int which)
{
  gestPerif::statMachine stat = isRunning();
  val_gen val;
  val.id = which;
  if(gestPerif::sOnRun == stat || (force && gestPerif::sDejaRun == stat))
      logStat.setEvent(log_status::onRun, &val);
  else if(gestPerif::sOnStop == stat || (force && gestPerif::sDejaStop == stat))
    logStat.setEvent(log_status::onPause, &val);
}
//----------------------------------------------------------
//----------------------------------------------------------
static bool isSameOrParent(HWND parent, HWND check)
{
  do {
    if(check == parent)
      return true;
    check = ::GetParent(check);
    } while(check);
  return false;
}
//----------------------------------------------------------
// funzionano entrambe
//#define GET_FOCUS GetFocus
#define GET_FOCUS GetForegroundWindow
//----------------------------------------------------------
#define MAX_WAIT_PERIF N_TICK_DELAY
//----------------------------------------------------------
#define N_TICK_DELAY (SECONDS_DELAY_TO_REFRESH * 1000 / TIME_SLICE)
//----------------------------------------------------------
static
bool needRefreshByNorm();
//----------------------------------------------------------
bool mainClient::EvTimer(uint timerId)
{
  if(timerId == idTimerResetPsw) {
    KillTimer(*this, idTimerResetPsw);
    idTimerResetPsw = 0;
    PassWord psw;
    psw.restorePsw(this);
    return true;
    }
  if(timerId == idTimerCheck) {
    checkKey(*this);
    return true;
    }

  if(timerId != idTimer)
    return false;

  // dCount viene inizializzato a INIT_COUNT, per cui se negativo è sicuramente
  // la prima volta che si entra nella routine. A questo punto gli oggetti
  // necessari all'inizializzazione dovrebbero essere tutti pronti
  // lo modifica subito per evitare un rientro indesiderato
  if(INIT_COUNT == dCount) {
    ++dCount;
    JobData->init();
    // forza l'invio dello stato degli allarmi
    Perif.logAlarm(logStat, rALL, true);
    JobData->logAlarm(logStat, rALL, true);
    JobData->notify();
    JobData->commit();

    extern bool canLoadDll();
    if(canLoadDll() /* !cfg_.disableDLL */ ) {
      LPCTSTR p = getString(ID_DLL_NAME);
      if(p) {
        LPTSTR buff = str_newdup(p);
        resolveDll(buff);
        delete []buff;
        ++SV_DLL_DUMMY;
        SV_System(eINIT_DLL);
        }
      }
    }
  ++dCount;

  if(!idTimer)
    return true;

  extern void closeHWSplashScreen();
  static bool activedWin = false;
  if(!activedWin) {
    if(Perif.isReady()) {
      activedWin = true;
      closeHWSplashScreen();
      ShowWindow(*getParent(), SW_SHOWNORMAL);
      }
  // inizializza a true la lista delle periferiche che potrebbero funzionare a richiesta
    if(!Bd->isReadOnly())
      for(uint i = 0; i < MAX_PERIF; ++i) {
        gestPerif* p = Perif.get(i + WM_PLC);
        if(p && p->hasRequestVar())
          useReqPrph[i] = true;
        }
    Bd->request();
    }
   // refresh dei dati delle periferiche
  Perif.refresh();

  // verifica lo stato di run o stop
  checkRunOrStop();

  // aggiorna i dati
  JobData->notify();

  int nElem = GlobalCam.getElem();
  for(int i = 0; i < nElem; ++i) {
    PVarCam* cam = GlobalCam[i]->getCam();
    PWin* w1 = cam->getParent();
    PWin* w2 = this;
    if(w1 == w2)
      cam->update(true);
    }

  // esegue il log degli allarmi
  Perif.logAlarm(logStat, rALL);
  JobData->logAlarm(logStat, rALL);

  // refresh dei log dopo SECONDS_DELAY_TO_REFRESH
  if(dCount > N_TICK_DELAY) {
    dCount = 0;
    JobData->notifySaveLog();
    logStat.setEvent(log_status::refreshStat);
    checkMappedFile();
    }

  // richiama i gestori delle view per l'aggiornamento a video
  if(activedWin && Hd->getHandle()) {
    bool need_refresh = needRefreshByNorm();
    if(need_refresh)
      Hd->setDirty();
    Hd->refresh();
    if(need_refresh)
      Bd->setDirty();
    Bd->refresh();
    Ft->refresh();
/*
  // non serve più, il refresh viene inviato dal body sia alla modal che alle modeless
    if(enableForwardFunctionKey > 0) {
      HWND hw = GetForegroundWindow();
      if(isSameOrParent(*getParent(), hw)) {
        PWin* w = getWindowPtr(hw);
        P_Body* mb = dynamic_cast<P_Body*>(w);
        if(mb) {
          if(need_refresh)
            mb->setDirty();
          mb->refresh();
          }
        }
      }
*/
    }

    // stabilizza lo stato corrente delle periferiche e dei dati generali
  Perif.commit();
  JobData->commit();

  return true;
}
//----------------------------------------------------------
void mainClient::enableBtn(int idBtn, bool enable)
{
  Ft->enableBtn(idBtn, enable);
}
//-----------------------------------------------------------------
void mainClient::setNewText(int idBtn, LPCTSTR newText)
{
  Ft->setNewText(idBtn, newText);
}
//----------------------------------------------------------------------------
//#define EXIT_FLAG (EWX_LOGOFF | EWX_SHUTDOWN | EWX_POWEROFF | EWX_FORCE)
//----------------------------------------------------------------------------
#define EXIT_FLAG (EWX_LOGOFF | EWX_SHUTDOWN | EWX_FORCE)
//----------------------------------------------------------------------------
#define EXIT_REASON (SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_MINOR_OTHER | SHTDN_REASON_FLAG_PLANNED)
//-----------------------------------------------------------------
void mainClient::CloseApp(HWND hwnd)
{
  if(idTimerCheck) {
    KillTimer(hwnd, idTimerCheck);
    idTimerCheck = 0;
    }
  if(idTimer) {
#if 1
    SV_System(eEND_DLL);

    int nElem = vSV_DLL.getElem();
    for(int i = 0; i < nElem; ++i)
      FreeLibrary(vSV_DLL[i].hDll);
    --SV_DLL_DUMMY;
#endif
    KillTimer(hwnd, idTimer);
    idTimer = 0;
    unsetHotKey();

    JobData->end();

    Perif.closeApp();

    delete Bd;
    flushPV(GlobalCam);

    config Cfg;
    const cfg& _cfg = Cfg.getAll();
    if(!_cfg.useMouse)
      showCursor();
    bool dummy;
    setNumLock(oldNumLock, dummy);
    if(_cfg.shutDown) {
      if(IS_WINNT_BASED) {

        HANDLE HProc = GetCurrentProcess();
        HANDLE token;
        if(OpenProcessToken( HProc, TOKEN_WRITE, &token)) {
          LUID luid;
          LookupPrivilegeValue( 0, SE_SHUTDOWN_NAME, &luid);

          TOKEN_PRIVILEGES newPriv;
          newPriv.PrivilegeCount = 1;
          newPriv.Privileges[0].Luid = luid;
          newPriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
          DWORD retLen;
          AdjustTokenPrivileges( token, false, &newPriv, 0, 0, &retLen);
          }
        }
      else {
        HWND hWnd = FindWindowEx(0, 0, _T("Shell_TrayWnd"), 0);
        DWORD processId = 0;
        DWORD threadId = GetWindowThreadProcessId(hWnd, &processId);
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, false, processId);
        TerminateProcess(hProcess, 0);
        Sleep(300);
        }
      DWORD flag = EXIT_FLAG;
      if(!_cfg.disablePoweroff)
        flag |= EWX_POWEROFF;
      ExitWindowsEx(flag, EXIT_REASON);
      }
    }
}
//-----------------------------------------------------------------
void mainClient::RepAllAlarm(bool set)
{
  val_gen val;
  val.id = set;
  logStat.setEvent(log_status::setRepAll, &val);
  if(set) {
    Perif.logAlarm(logStat, rALL_ALARM, true);
    JobData->logAlarm(logStat, rALL_ALARM, true);
    }
}
//-----------------------------------------------------------------
void mainClient::show_hide_periph(bool show)
{
  uint how = show ? SW_SHOWNORMAL : SW_HIDE;
  if(Perif.setFirst()) {
    do {
      gestPerif *p = Perif.getCurr();
      HWND hw = p->getHwndDriver();
      if(hw)
        ShowWindow(hw, how);
      } while(Perif.setNext());
    }
}
//-----------------------------------------------------------------
PVarCam* mainClient::getCam(LPCTSTR name)
{
  int nElem = GlobalCam.getElem();
  for(int i = 0; i < nElem; ++i)
    if(!_tcsicmp(name, GlobalCam[i]->getCamName()))
      return GlobalCam[i]->getCam();
  return 0;
}
//-----------------------------------------------------------------
void mainClient::addCam(PVarCam* cam, LPCTSTR name)
{
  int nElem = GlobalCam.getElem();
  GlobalCam[nElem] = new PVarGlobalCam(cam, name);
}
//-----------------------------------------------------------------
void mainClient::hideCam()
{
  int nElem = GlobalCam.getElem();
  for(int i = 0; i < nElem; ++i) {
    PVarCam* cam = GlobalCam[i]->getCam();
    cam->Hide();
    }
}
//-----------------------------------------------------------------
void mainClient::showCam(PVarCam* cam)
{
  cam->Show();
}
//-----------------------------------------------------------------
bool mainClient::isGlobalCam(LPCTSTR name)
{
  if(!name)
    return false;
  LPCTSTR p = getString(ID_CAM_NAME);
  if(p) {
    LPTSTR buff = str_newdup(p);
    LPTSTR t = buff;
    bool found = false;
    do {
      LPTSTR next = get_next(t);
      if(!_tcsicmp(name, t)) {
        found = true;
        break;
        }
      t = next;
      } while(t);
    delete []buff;
    return found;
    }
  return false;
}
//-----------------------------------------------------------------
PVarGlobalCam::PVarGlobalCam(PVarCam* cam, LPCTSTR name) : Cam(cam), Name(str_newdup(name))
{ }
//-----------------------------------------------------------------
PVarGlobalCam::~PVarGlobalCam()
{
//  delete Cam;
  delete []Name;
}
//----------------------------------------------------------------------------
fREALDATA getNormByVar(uint prph, uint addr, uint typeVar)
{
  mainClient* mc = getMain();
  if(!mc)
    return 1.0f;
  genericPerif* Prph = mc->getGenPerif(prph);
  if(!Prph)
    return 1.0f;
  prfData data;
  data.lAddr = addr;
  data.typeVar = (prfData::tData)typeVar;
  data.U.dw = 0;
  Prph->get(data);
  if(!data.U.dw)
    return 1.0f;

  switch(typeVar) {
    case prfData::tFRData:
      return data.U.fw;

    case prfData::tRData:
      return (fREALDATA)data.U.rw;

    default:
      return (fREALDATA)data.U.dw;
    }
}
//----------------------------------------------------------------------------
bool isSetBitNorm(uint prph, uint addr, uint bit)
{
  mainClient* mc = getMain();
  if(!mc)
    return false;
  genericPerif* Prph = mc->getGenPerif(prph);
  if(!Prph)
    return false;
  prfData data;
  data.lAddr = addr;
  data.typeVar = prfData::tDWData;
  data.U.dw = 0;
  Prph->get(data);
  if(!(data.U.dw & (1 << bit)))
    return false;

  return true;
}
//----------------------------------------------------------------------------
bool isChangedData(uint prph, uint addr, uint nValue)
{
  mainClient* mc = getMain();
  if(!mc)
    return false;
  genericPerif* Prph = mc->getGenPerif(prph);
  if(!Prph)
    return false;
  prfData data;
  data.lAddr = addr;
  data.typeVar = prfData::tDWData;
  if(!nValue)
    nValue = 1;
  for(uint i = 0; i < nValue; ++i, ++data.lAddr) {
    data.U.dw = 0;
    if(prfData::isChanged == Prph->get(data))
      return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool needRefreshByNorm()
{
  bool need_refresh = false;
  const PVect<infoVarNorm>& ivn = getInfoVarNorm();
  uint nElem = ivn.getElem();
  for(uint i = 0; i < nElem; ++i) {
    if(isChangedData(ivn[i].prph, ivn[i].addr, 1))
      return true;
    }
  return false;
}
//----------------------------------------------------------------------------
