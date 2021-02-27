//------ d_config.cpp -----------------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "macro_utils.h"
#include "config.h"
#include "d_config.h"
#include "p_param.h"
#include "p_file.h"
#include "def_dir.h"
#include "newNormal.h"
#include "1.h"
#include "set_cfg.h"
//-------------------------------------------------------------------
#define STRINGER(a) #a
#define COM_N(a) { a, _T("COM") _T(STRINGER(a)) }
//-------------------------------------------------------------------
#define MAX_PORT 99
/*
static value_name vPort[] =
{
  COM_N(1),
  COM_N(2),
  COM_N(3),
  COM_N(4),
  COM_N(5),
  COM_N(6),
  COM_N(7),
  COM_N(8),
  COM_N(9),
  COM_N(10),
  COM_N(11),
  COM_N(12),
  COM_N(13),
  COM_N(14),
  COM_N(15),
  COM_N(16),
};
*/
//-------------------------------------------------------------------
#define fileCfg _T("Setup")
//-------------------------------------------------------------------
cfg config::Cfg;
//-------------------------------------------------------------------
bool config::Updated = false;
//-------------------------------------------------------------------
config::config()
{
  if(Updated)
    return;
  ZeroMemory(&Cfg, sizeof(Cfg));
  for(int i = 0; i < MAX_PERIF; ++i) {
    paramCom pC(static_cast<paramCom::ePort>(i));
    Cfg.parPerif[i] = pC;
    }

  Cfg.useMouse = 1; // abilitato per default (se non esiste il file)

  Init();

  reload();

  initNormal();
/*
// viene fatto al primo richiamo nella mainClient.
// se per qualche motivo si sposta il primo richiamo prima che venga
// caricato lo std_msg non funzionarebbe più
  LPCTSTR p = ::getString(ID_LANGUAGE);
  Cfg.LangInUse = 0;
  if(p)
    Cfg.LangInUse = _ttoi(p);
*/
  Updated = true;
}
//-------------------------------------------------------------------
void config::reload()
{
  TCHAR file[_MAX_PATH] = fileCfg;
  _tcscat_s(file, SIZE_A(file), getExt(dSystem));

  getRelPath(file, dSystem);
  P_File f(file);
  if(f.P_open())
    if(f.get_len())
      f.P_read(&Cfg, sizeof(Cfg));
}
//-------------------------------------------------------------------
void config::setup(const cfg& newcfg)
{
  Cfg = newcfg;
  TCHAR file[_MAX_PATH] = fileCfg;
  _tcscat_s(file, SIZE_A(file), getExt(dSystem));
  getRelPath(file, dSystem);
  P_File f(file, P_CREAT);
  if(f.P_open())
    f.P_write(&Cfg, sizeof(Cfg));
  Updated = true;
}
//-------------------------------------------------------------------
// la parte sopra è indipendente dal target eseguibile,

// questa parte sotto è dipendente dal supervisore a causa della
// TD_Config e della Init()
//-------------------------------------------------------------------
bool config::setup(P_Body* w)
{
  if(TD_Config(&Cfg, w).modal() == IDOK) {
    TCHAR file[_MAX_PATH] = fileCfg;
    _tcscat_s(file, SIZE_A(file), getExt(dSystem));
    getRelPath(file, dSystem);
    P_File f(file,P_CREAT);
    if(f.P_open())
      f.P_write(&Cfg, sizeof(Cfg));
    Updated = true;
    return true;
    }
  return false;
}
//-------------------------------------------------------------------
void config::Init()
{
//  Cfg.__numBlk = MAX_N_BLK;  // per default attiva MAX_N_BLK
}
//---------------------------------------------------------
//---------------------------------------------------------
static void setName(setOfString &StrsSet, HWND HWindow);
//---------------------------------------------------------
struct id_ctrl2 {
  uint idCtrl[2];
};
//---------------------------------------------------------
struct id_ctrl5 {
  uint idCtrl[5];
};
//---------------------------------------------------------
#define D_LINK_5(a) \
  { {IDC_CHECKBOX_PERIF##a, IDC_COMBOBOX_PRG_PERIF##a, \
    IDC_COMBOBOX_COM_PERIF##a, IDC_BUTTON_PARA_PERIF##a, IDC_EDIT_OFFSET_PRF##a } }

static id_ctrl5 IdCtrlLink5[] = {
#if 1
  D_LINK_5(1),
  D_LINK_5(2),
  D_LINK_5(3),
  D_LINK_5(4),
  D_LINK_5(5),
  D_LINK_5(6),
  D_LINK_5(7),
  D_LINK_5(8),
#else
  { {IDC_CHECKBOX_PERIF1, IDC_COMBOBOX_PRG_PERIF1,
    IDC_COMBOBOX_COM_PERIF1, IDC_BUTTON_PARA_PERIF1, IDC_EDIT_OFFSET_PRF1 } },

  { {IDC_CHECKBOX_PERIF2, IDC_COMBOBOX_PRG_PERIF2,
    IDC_COMBOBOX_COM_PERIF2, IDC_BUTTON_PARA_PERIF2, IDC_EDIT_OFFSET_PRF2 } },

  { {IDC_CHECKBOX_PERIF3, IDC_COMBOBOX_PRG_PERIF3,
    IDC_COMBOBOX_COM_PERIF3, IDC_BUTTON_PARA_PERIF3, IDC_EDIT_OFFSET_PRF3 } },

  { {IDC_CHECKBOX_PERIF4, IDC_COMBOBOX_PRG_PERIF4,
    IDC_COMBOBOX_COM_PERIF4, IDC_BUTTON_PARA_PERIF4, IDC_EDIT_OFFSET_PRF4 } },
#endif
};
//---------------------------------------------------------
/**/
static id_ctrl2 IdCtrlLink2[] = {
  { {IDC_STATICTEXT_LANG, IDC_COMBOBOX_LANG } },
  };
/**/
//---------------------------------------------------------
static uint IdCtrlUnary[] = {
  IDC_CHECKBOX_USE_MOUSE,
  IDC_CHECKBOXSHUTDOWN,
  IDC_CHECKBOX_NO_POWEROFF,
  IDC_CHECKBOX_NUMLOCK,
  IDC_CHECKBOX_USE_READER,
  IDC_GROUPBOX_CFG1,
  IDC_GROUPBOX_CFG2,
  IDC_GROUPBOX_CFG3,
  IDC_CHECKBOX_HIDE_TASKBAR,
//  IDC_CHECKBOX_NO_ALT_KEY,
//  IDC_CHECKBOX_NO_CTRL,
//  IDC_CHECKBOX_NO_WINKEY,

  IDC_BUTTON_CFG_NORM,
  IDC_BUTTON_TURN,
};

static uint IdCtrlUnary2[] = {
  IDC_CHECKBOX_NO_ALT_KEY,
  IDC_CHECKBOX_NO_CTRL,
  IDC_CHECKBOX_NO_WINKEY,
};
/*
specifico per win9x, è controllato solo il primo
2196,Disattiva SYSKEY

su winNT si possono scegliere, ma non si disabilita il CTRL+ALT+CANC
,2196,Disattiva ALT
,2197,Disattiva CTRL
,2198,Disattiva WIN_KEY
*/
//---------------------------------------------------------
// file contenente i testi per il dialogo
#define FILE_NAME_CFG_TXT _T("config") PAGE_EXT
#define FILE_NAME_CFG_TXT_9X _T("config_9x") PAGE_EXT
#define FILE_NAME_CFG_TXT_NT _T("config_NT") PAGE_EXT
#define IS_WINNT_BASED isWinNT_Based()
//---------------------------------------------------------
TD_Config::TD_Config(cfg *cfg, P_Body* parent, int resId, HINSTANCE hinst)
:
    baseClass(parent, resId, hinst), Cfg(cfg)
{
  TCHAR path[_MAX_PATH];
  checkSystemPathTxt(path, FILE_NAME_CFG_TXT);
  pageStrToSet(StrsSet, path);

  do {
    if(IS_WINNT_BASED)
      checkSystemPathTxt(path, FILE_NAME_CFG_TXT_NT);
    else
      checkSystemPathTxt(path, FILE_NAME_CFG_TXT_9X);
    pageStrToSet(StrsSet, path);
    } while(false);
  hideShowDriver = new PCheckBox(this, IDC_CHECK_HIDE_SHOW_DRIVER);

  ShutDown = new PCheckBox(this, IDC_CHECKBOXSHUTDOWN);
  DisablePoweroff = new PCheckBox(this, IDC_CHECKBOX_NO_POWEROFF);
  UseMouse = new PCheckBox(this, IDC_CHECKBOX_USE_MOUSE);
  numLock = new PCheckBox(this, IDC_CHECKBOX_NUMLOCK);

  taskBar = new PCheckBox(this, IDC_CHECKBOX_HIDE_TASKBAR);
  AltKey = new PCheckBox(this, IDC_CHECKBOX_NO_ALT_KEY);
  CtrlKey = new PCheckBox(this, IDC_CHECKBOX_NO_CTRL);
  WinKey = new PCheckBox(this, IDC_CHECKBOX_NO_WINKEY);

  disableDLL = new PCheckBox(this, IDC_CHECKBOX_NO_DLL);
  useCard = new PCheckBox(this, IDC_CHECKBOX_USE_READER);

  for(int i = 0; i < CFG_PERIF; ++i) {
    CB_Perif[i] = new PCheckBox(this, IdCtrlLink5[i].idCtrl[0]);
    Prg_Perif[i] = new PComboBox(this, IdCtrlLink5[i].idCtrl[1]);
    CBX_Perif[i] = new PComboBox(this, IdCtrlLink5[i].idCtrl[2]);
    OffsPrf[i] = new svEdit(this, IdCtrlLink5[i].idCtrl[4], 6);
    }
  new svEdit(this, IDC_EDIT_NUM_EXTENDED_PRF_ONMEM);
}
//---------------------------------------------------------
TD_Config::~TD_Config()
{
  destroy();
}
//---------------------------------------------------------
//---------------------------------------------------------
static void fillPort(PComboBox *bx)
{
  SendMessage(*bx, CB_RESETCONTENT, 0, 0);
  TCHAR t[128];
  for(int i = 0; i < MAX_PORT; ++i) {
    wsprintf(t, _T("Com%d"), i + 1);
    SendMessage(*bx, CB_ADDSTRING, 0, (LPARAM)t);
    }
}
//---------------------------------------------------------
#define PRF_ON_MEM_NAME _T(" prph_OnMem")
LPCTSTR getPrphOnMemName() { return PRF_ON_MEM_NAME; }
//---------------------------------------------------------
static void fillPrg(PComboBox *bx, LPCTSTR path)
{
  SendMessage(*bx, CB_RESETCONTENT, 0, 0);
  SendMessage(*bx, CB_ADDSTRING, 0, (LPARAM)_T(" "));
  SendMessage(*bx, CB_ADDSTRING, 0, (LPARAM)PRF_ON_MEM_NAME);
  if(!path || !*path)
    return;
  WIN32_FIND_DATA ffd;
  HANDLE fff = FindFirstFile(path, &ffd);
  if(INVALID_HANDLE_VALUE != fff) {
    do {
      SendMessage(*bx, CB_ADDSTRING, 0, (LPARAM)ffd.cFileName);
      } while(FindNextFile(fff, &ffd));
    FindClose(fff);
    }
}
//---------------------------------------------------------
static void fillOffs(PEdit *offs, DWORD value)
{
  TCHAR buff[50];
  wsprintf(buff, _T("%d"), value);
  SetWindowText(*offs, buff);
}
//---------------------------------------------------------
#define PATH_PLC  _T("prph_*.exe")
#define PATH_PRF   PATH_PLC
//#define PATH_PRF   _T("PRF_*.EXE")
//---------------------------------------------------------
static LPCTSTR pathP[CFG_PERIF] = {
  PATH_PLC,
  PATH_PRF,
  PATH_PRF,
  PATH_PRF,
  PATH_PRF,
  PATH_PRF,
  PATH_PRF,
  PATH_PRF,
  };
//---------------------------------------------------------
void findAndSetText(HWND cbx, LPCTSTR txt)
{
  int count = SendMessage(cbx, CB_GETCOUNT, 0, 0);
  TCHAR t[500];
  for(int i = 0; i < count; ++i) {
    SendMessage(cbx, CB_GETLBTEXT, i, (LPARAM)t);
    if(!_tcscmp(t, txt)) {
      SendMessage(cbx, CB_SETCURSEL, i, 0);
      break;
      }
    }
}
//---------------------------------------------------------
#define CHECK(a) SendMessage(*a, BM_SETCHECK, BST_CHECKED, 0)
#define UNCHECK(a) SendMessage(*a, BM_SETCHECK, BST_UNCHECKED, 0)
//---------------------------------------------------------
bool TD_Config::create()
{
  if(!baseClass::create())
    return false;

  if(Cfg->noShowPeriph)
    CHECK(hideShowDriver);

  if(Cfg->disableDLL)
    CHECK(disableDLL);

  if(Cfg->useMouse)
    CHECK(UseMouse);
  if(Cfg->shutDown)
    CHECK(ShutDown);
  if(Cfg->disablePoweroff)
    CHECK(DisablePoweroff);

  if(Cfg->numLock)
    CHECK(numLock);

  if(Cfg->hideTaskBar)
    CHECK(taskBar);
  if(Cfg->disableAltKey)
    CHECK(AltKey);
  if(Cfg->disableCtrlKey)
    CHECK(CtrlKey);
  if(Cfg->disableWinKey)
    CHECK(WinKey);
  if(Cfg->useCardOper)
    CHECK(useCard);

  for(int i = 0; i < CFG_PERIF; ++i) {
    if(Cfg->Perif[i])
      CHECK(CB_Perif[i]);
    fillPort(CBX_Perif[i]);
    fillPrg(Prg_Perif[i], pathP[i]);
    fillOffs(OffsPrf[i], Cfg->OffsPrf__(i));

    TCHAR path[_MAX_PATH];
    wsprintf(path, _T("Com%d"), Cfg->parPerif[i].getPort() + 1);
    findAndSetText(*CBX_Perif[i], path);
    copyStrZ(path, Cfg->pathPerif[i]);
    findAndSetText(*Prg_Perif[i], path);

    }
  setName(StrsSet, getHandle());
  fillLanguage();
  SET_INT(IDC_EDIT_NUM_EXTENDED_PRF_ONMEM, Cfg->__ExtendedPeriphOnMem);
  return true;
}
//---------------------------------------------------------
//#define IS_CHECKED(a) (BST_CHECKED == SendMessage(*a, BM_GETCHECK, 0, 0))
void TD_Config::BNClickedOK()
{
  // INSERT>> Your code here.
  Cfg->disableDLL = IS_CHECKED_THIS(disableDLL);
  Cfg->useMouse = IS_CHECKED_THIS(UseMouse);
  Cfg->shutDown = IS_CHECKED_THIS(ShutDown);
  Cfg->disablePoweroff = IS_CHECKED_THIS(DisablePoweroff);
  Cfg->numLock = IS_CHECKED_THIS(numLock);

  Cfg->noShowPeriph = IS_CHECKED_THIS(hideShowDriver);

  Cfg->hideTaskBar = IS_CHECKED_THIS(taskBar);
  Cfg->disableAltKey = IS_CHECKED_THIS(AltKey);
  Cfg->disableCtrlKey = IS_CHECKED_THIS(CtrlKey);
  Cfg->disableWinKey = IS_CHECKED_THIS(WinKey);
  Cfg->useCardOper = IS_CHECKED_THIS(useCard);


  TCHAR t[20];
  for(int i = 0; i < CFG_PERIF; ++i) {
    Cfg->Perif[i] = IS_CHECKED_THIS(CB_Perif[i]);
    GetWindowText(*CBX_Perif[i], t, SIZE_A(t));
    if(*t) {
      int port = _ttoi(t + 3);
      if(port)
        Cfg->parPerif[i].setPort(port - 1);
      }
    TCHAR path[_MAX_PATH];
    GetWindowText(*Prg_Perif[i], path, SIZE_A(path));
    if(!*path)
      Cfg->Perif[i] = 0;
    else
      copyStrZ(Cfg->pathPerif[i], path);

    GetWindowText(*OffsPrf[i], t, SIZE_A(t));
    if(*t) {
      int offs = _ttoi(t);
      Cfg->OffsPrf__(i) = offs;
      }
    }
  HWND hwCb = GetDlgItem(*this, IDC_COMBOBOX_LANG);
  int sel = SendMessage(hwCb, CB_GETCURSEL, 0, 0);
  if(sel < 0)
    sel = 0;
  Cfg->LangInUse = (BYTE)sel;
  GET_INT(IDC_EDIT_NUM_EXTENDED_PRF_ONMEM, Cfg->__ExtendedPeriphOnMem);

  EndDialog(*this, IDOK);
}
//---------------------------------------------------------
static void makeTitle(setOfString &StrsSet, LPTSTR buff, int id)
{
  LPCTSTR p = StrsSet.getString(IDC_GROUPBOX_CFG2);
  LPCTSTR p2 = StrsSet.getString(IdCtrlLink5[id].idCtrl[0]);
  wsprintf(buff, _T("%s [%s]"), p, p2);
}
//---------------------------------------------------------
void TD_Config::BNClickedParam(int prf)
{
  P_Param p(Cfg->parPerif[prf], this);
  TCHAR t[255];
  makeTitle(StrsSet, t, prf);
  p.setCaption(t);
  p.modal();
}
//---------------------------------------------------------
void TD_Config::BNClickedParam_Perif1()
{
  BNClickedParam(0);
}
//---------------------------------------------------------
void TD_Config::BNClickedParam_Perif2()
{
  BNClickedParam(1);
}
//---------------------------------------------------------
void TD_Config::BNClickedParam_Perif3()
{
  BNClickedParam(2);
}
//---------------------------------------------------------
void TD_Config::BNClickedParam_Perif4()
{
  BNClickedParam(3);
}
//---------------------------------------------------------
void TD_Config::BNClickedParam_Perif5()
{
  BNClickedParam(4);
}
//---------------------------------------------------------
void TD_Config::BNClickedParam_Perif6()
{
  BNClickedParam(5);
}
//---------------------------------------------------------
void TD_Config::BNClickedParam_Perif7()
{
  BNClickedParam(6);
}
//---------------------------------------------------------
void TD_Config::BNClickedParam_Perif8()
{
  BNClickedParam(7);
}
//---------------------------------------------------------
//---------------------------------------------------------
void TD_Config::BNClickedUseMouse()
{
  extern void showCursor();
  extern void hideCursor();
  if(IS_CHECKED_THIS(UseMouse))
    showCursor();
  else
    hideCursor();
}
//---------------------------------------------------------
void TD_Config::BNClickedNormaliz()
{
  setupNormal(this);
}
//---------------------------------------------------------
void TD_Config::BNClickedTurni()
{
  extern void setTurnOver(PWin *);
  setTurnOver(this);
}
//---------------------------------------------------------
static void setName5(setOfString &StrsSet, HWND HWindow)
{
  for(int i = 0; i < SIZE_A(IdCtrlLink5); ++i) {
    LPCTSTR p = StrsSet.getString(IdCtrlLink5[i].idCtrl[0]);
    if(p) {
      HWND h = ::GetDlgItem(HWindow, IdCtrlLink5[i].idCtrl[0]);
      if(h)
        ::SetWindowText(h, p);
      p = StrsSet.getString(IdCtrlLink5[i].idCtrl[3]);
      if(p) {
        HWND h = ::GetDlgItem(HWindow, IdCtrlLink5[i].idCtrl[3]);
        if(h)
          ::SetWindowText(h, p);
        }
      }
    else {
      for(int j = 0; j < 5; ++j) {
        HWND h = ::GetDlgItem(HWindow, IdCtrlLink5[i].idCtrl[j]);
        if(h)
          ::ShowWindow(h, SW_HIDE);
        }
      }
    }
}
//---------------------------------------------------------
static void setName2(setOfString &StrsSet, HWND HWindow)
{
  for(int i = 0; i < SIZE_A(IdCtrlLink2); ++i) {
    LPCTSTR p = StrsSet.getString(IdCtrlLink2[i].idCtrl[0]);
    if(p) {
      HWND h = ::GetDlgItem(HWindow, IdCtrlLink2[i].idCtrl[0]);
      if(h)
        ::SetWindowText(h, p);
      }
    else {
      for(int j = 0; j < 2; ++j) {
        HWND h = ::GetDlgItem(HWindow, IdCtrlLink2[i].idCtrl[j]);
        if(h)
          ::ShowWindow(h, SW_HIDE);
        }
      }
    }
}
//---------------------------------------------------------
static void setName1(setOfString &StrsSet, HWND HWindow)
{
  for(int i = 0; i < SIZE_A(IdCtrlUnary); ++i) {
    HWND h = ::GetDlgItem(HWindow, IdCtrlUnary[i]);
    if(h) {
      LPCTSTR p = StrsSet.getString(IdCtrlUnary[i]);
      if(p)
        ::SetWindowText(h, p);
      else
        ::ShowWindow(h, SW_HIDE);
      }
    }
}
//---------------------------------------------------------
static void setNameKeyCtrl(setOfString &StrsSet, HWND HWindow)
{
  uint tot = IS_WINNT_BASED ? 3 : 1;
  bool missed = false;
  for(uint i = 0; i < tot; ++i) {
    LPCTSTR p = StrsSet.getString(IdCtrlUnary2[i]);
    if(!p) {
      missed = true;
      break;
      }
    }
  if(!missed) {
    for(uint i = 0; i < tot; ++i) {
      HWND h = ::GetDlgItem(HWindow, IdCtrlUnary2[i]);
      if(h) {
        LPCTSTR p = StrsSet.getString(IdCtrlUnary2[i]);
        if(p)
          ::SetWindowText(h, p);
        }
      }
    }
  for(int i = tot; i < SIZE_A(IdCtrlUnary2); ++i) {
    HWND h = ::GetDlgItem(HWindow, IdCtrlUnary2[i]);
    if(h)
      ::ShowWindow(h, SW_HIDE);
    }
  HWND h = ::GetDlgItem(HWindow, IDC_CHECK_HIDE_SHOW_DRIVER);
  if(h) {
    LPCTSTR p = StrsSet.getString(IDC_CHECK_HIDE_SHOW_DRIVER);
    if(p)
      ::SetWindowText(h, p);
    }
}
//---------------------------------------------------------
void setName(setOfString &StrsSet, HWND HWindow)
{
  setName1(StrsSet, HWindow);
  setName2(StrsSet, HWindow);
  setName5(StrsSet, HWindow);
  setNameKeyCtrl(StrsSet, HWindow);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define EV_BN_CLICKED(id, fz) \
        case id:  \
          fz();   \
          break;
//----------------------------------------------------------------------------
#define EV_CBN_SELCHANGE(id, fz) \
        case id:  \
          fz();   \
          break;
//----------------------------------------------------------------------------
LRESULT TD_Config::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        EV_BN_CLICKED(IDC_BUTTON_PARA_PERIF1, BNClickedParam_Perif1)
        EV_BN_CLICKED(IDC_BUTTON_PARA_PERIF2, BNClickedParam_Perif2)
        EV_BN_CLICKED(IDC_BUTTON_PARA_PERIF3, BNClickedParam_Perif3)
        EV_BN_CLICKED(IDC_BUTTON_PARA_PERIF4, BNClickedParam_Perif4)
        EV_BN_CLICKED(IDC_BUTTON_PARA_PERIF5, BNClickedParam_Perif5)
        EV_BN_CLICKED(IDC_BUTTON_PARA_PERIF6, BNClickedParam_Perif6)
        EV_BN_CLICKED(IDC_BUTTON_PARA_PERIF7, BNClickedParam_Perif7)
        EV_BN_CLICKED(IDC_BUTTON_PARA_PERIF8, BNClickedParam_Perif8)
        EV_BN_CLICKED(IDC_BUTTON_CFG_NORM, BNClickedNormaliz)
        EV_BN_CLICKED(IDC_BUTTON_TURN, BNClickedTurni)

        EV_BN_CLICKED(IDC_CHECKBOX_USE_MOUSE, BNClickedUseMouse)

        EV_BN_CLICKED(IDOK, BNClickedOK)

        case IDCANCEL:
          EndDialog(*this, IDCANCEL);
          break;
        }
      break;

    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//---------------------------------------------------------
static void addToCb(HWND hwCb, LPCTSTR path)
{
  SendMessage(hwCb, CB_ADDSTRING, 0, (LPARAM)path);
}
//---------------------------------------------------------
#define PATH_LANG _T("Language")
#define MSG_NO_CHANGE_LANG _T("~No Change~")
//---------------------------------------------------------
void TD_Config::fillLanguage()
{
  HWND hwCb = GetDlgItem(*this, IDC_COMBOBOX_LANG);
  LPCTSTR p = getString(ID_LANGUAGE);
  int currL = p ? _ttoi(p) : 0;
  if(!currL) {
    EnableWindow(hwCb, false);
    return;
    }
  p = findNextParam(p, 1);
  if(!p)
    return;

  pvvChar target;
  uint nElem = splitParam(target, p);
  addToCb(hwCb, _T("- no Language -"));
  for(uint i = 0; i < MAX_LANG && i < nElem; ++i) {
    if(*&target[i])
      addToCb(hwCb, &target[i]);
    else
      addToCb(hwCb, _T("-------"));
    }
  SendMessage(hwCb, CB_SETCURSEL, Cfg->LangInUse, 0);
}
//---------------------------------------------------------
static void addToLb(HWND hwCb, LPCTSTR path)
{
  SendMessage(hwCb, LB_ADDSTRING, 0, (LPARAM)path);
}
//---------------------------------------------------------
class D_SetLang : public svModDialog
{
  private:
    typedef svModDialog baseClass;
  public:
    D_SetLang(PWin* parent, uint id = IDD_EXPORT, HINSTANCE hInst = 0) : baseClass(parent, id, hInst) {}
    ~D_SetLang() { destroy(); }

    bool create();
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CmOk();
};
//---------------------------------------------------------
bool setLang(PWin* parent)
{
  LPCTSTR p = getString(ID_LANGUAGE);
  p = findNextParam(p, 1);
  if(!p)
    return false;
  return IDOK == D_SetLang(parent).modal();
}
//---------------------------------------------------------
bool D_SetLang::create()
{
  if(!baseClass::create())
    return false;
  setWindowTextByLangGlob(GetDlgItem(*this, IDC_BUTTON_F1), ID_DIALOG_OK);
  setWindowTextByLangGlob(GetDlgItem(*this, IDC_BUTTON_F2), ID_DIALOG_CANC);

  LPCTSTR p = getString(ID_LANGUAGE);
  p = findNextParam(p, 1);
  if(!p)
    return false;

  pvvChar target;
  uint nElem = splitParam(target, p);
  HWND hwCb = GetDlgItem(*this, IDC_LISTBOX_CHOOSE_EXP);
  for(uint i = 0; i < MAX_LANG && i < nElem; ++i) {
    if(*&target[i])
      addToLb(hwCb, &target[i]);
    else
      break;
    }
  const cfg& Cfg = config::getAll();
  SendMessage(hwCb, LB_SETCURSEL, Cfg.LangInUse - 1, 0);
  return true;
}
//----------------------------------------------------------------------------
LRESULT D_SetLang::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_F1:
        case IDOK:
          CmOk();
          break;
        case IDC_BUTTON_F2:
        case IDCANCEL:
          CmCancel();
          break;
        }
      switch(HIWORD(wParam)) {
        case LBN_DBLCLK:
          CmOk();
          break;
        }
      break;

    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//---------------------------------------------------------
void D_SetLang::CmOk()
{
  HWND hwCb = GetDlgItem(*this, IDC_LISTBOX_CHOOSE_EXP);
  int langInUse = SendMessage(hwCb, LB_GETCURSEL, 0, 0);
  if(langInUse < 0)
    return;
  cfg _cfg = config().getAll();
  _cfg.LangInUse = langInUse + 1;
  config().setup(_cfg);
  baseClass::CmOk();
}
