//-------- password.cpp ------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "macro_utils.h"

#include "password.h"
#include "defin.h"

#include "p_base.h"
#include "pmoddialog.h"
#include "d_psw.h"
#include "POwnBtnImageStd.h"
#include "1.h"
#include "mainClient.h"
//----------------------------------------------------------------------------
#define MAX_LEVEL_PSW 3
//----------------------------------------------------------------------------
int PassWord::level = 0;
int PassWord::levelByPrph;
int PassWord::prevLevelByPrph = -1;
DWORD PassWord::seconds = 0;
bool PassWord::inExec = false;
DWORD PassWord::maxTimePsw = (DWORD)-1;
uint PassWord::prph;
uint PassWord::addr;
uint PassWord::maxLevelByPrph;
bool PassWord::waitForMessageResult;
//----------------------------------------------------------------------------
// id,maxTime,prph,addr,maxLevel
void PassWord::calcPrph()
{
  LPCTSTR p = getString(ID_PASSWORD_TIME_LEASE);
  if(!p) {
    maxTimePsw = MAX_TIME_PSW;
    return;
    }
  maxTimePsw = _ttoi(p) * 1000;
  p = findNextParamTrim(p);
  if(!p)
    return;
  prph = _ttoi(p);
  p = findNextParamTrim(p);
  if(!p)
    return;
  addr = _ttoi(p);
  p = findNextParamTrim(p);
  if(!p)
    return;
  maxLevelByPrph = _ttoi(p);
  if(maxLevelByPrph > MAX_LEVEL_PSW)
    maxLevelByPrph = MAX_LEVEL_PSW;
}
//----------------------------------------------------------------------------
void PassWord::verifyValidityTime(PWin* w, DWORD tickCount)
{
  // se livello zero è inutile controllare il tempo
  if(!level)
    return;
  DWORD sec = tickCount;
  if(sec < seconds)  // reiniziato il ciclo dei secondi
    sec += (DWORD)-1;
  if(sec - seconds > maxTimePsw) // tempo scaduto
    restorePsw(w);
}
//--------------------------------------------------------------
void PassWord::savePswToPrph(PWin* w, int newpsw)
{
  if(!prph)
    return;
  mainClient* mc = getParentWin<mainClient>(w);
  if(!mc) {
    prph = 0;
    return;
    }
  genericPerif* p = mc->getGenPerif(prph);
  if(!p) {
    prph = 0;
    return;
    }
  prfData data;
  data.lAddr = addr;
  data.typeVar = prfData::tWData;
  data.U.w = newpsw;
  p->set(data);
}
//--------------------------------------------------------------
void PassWord::setPsw(PWin* w)
{
  prevLevelByPrph = levelByPrph;
  levelByPrph = level;
  savePswToPrph(w, levelByPrph);
  mainClient* mc = getParentWin<mainClient>(w);
  if(mc) {
    waitForMessageResult = true;
    PostMessage(*mc, WM_POST_RESET_PASSWORD, 0, 0);
    }
}
//--------------------------------------------------------------
void PassWord::restorePsw(PWin* w)
{
  if(waitForMessageResult) {
    PostMessage(*w, WM_POST_RESET_PASSWORD, 0, 0);
    return;
    }
  if(prevLevelByPrph >= 0) {
    levelByPrph = prevLevelByPrph;
    savePswToPrph(w, prevLevelByPrph);
    prevLevelByPrph = -1;
    }
  level = levelByPrph;
}
//--------------------------------------------------------------
void PassWord::restartTime(int lev)
{
  seconds = GetTickCount();
  level = lev;
}
//--------------------------------------------------------------
class onlyConfirmDialog : public svModDialog
{
  private:
    typedef svModDialog baseClass;
  public:
    onlyConfirmDialog(PWin* parent, uint id = IDD_PSW_1000, HINSTANCE hInst = 0) :
      baseClass(parent, id, hInst) {}
    ~onlyConfirmDialog() { destroy(); }
    bool create();
};
//--------------------------------------------------------------
bool onlyConfirmDialog::create()
{
  new POwnBtnImageStd(this, IDOK, new PBitmap(IDB_OK, getHInstance()), POwnBtnImageStd::wLeft, true);
  new POwnBtnImageStd(this, IDCANCEL, new PBitmap(IDB_CANC, getHInstance()), POwnBtnImageStd::wLeft, true);
  if(!baseClass::create())
    return false;

  setWindowTextByLangGlob(*this, ID_CONFIRM_SEND_TITLE, false);
  setWindowTextByLangGlob(GetDlgItem(*this, IDC_STATIC_INFO), ID_CONFIRM_SEND_MSG, false);
  return true;
}
//--------------------------------------------------------------
class showPasswordDialog : public svModDialog
{
  private:
    typedef svModDialog baseClass;
  public:
    showPasswordDialog(PWin* parent, uint level, uint id = IDD_PSW_SHOW, HINSTANCE hInst = 0) :
      baseClass(parent, id, hInst), Level(level) {}
    ~showPasswordDialog() { destroy(); }
    bool create();
  private:
    uint Level;
};
//--------------------------------------------------------------
bool showPasswordDialog::create()
{
  new POwnBtnImageStd(this, IDOK, new PBitmap(IDB_OK, getHInstance()), POwnBtnImageStd::wLeft, true);
  if(!baseClass::create())
    return false;

  smartPointerConstString tmsg = getStringByLangGlob(ID_MSG_RESULT_PSW);
  TCHAR msg[100];
  if(tmsg)
    wsprintf(msg, tmsg, Level);
  else
    wsprintf(msg,_T("Accesso di %d° livello ai dati\r\n(!!!manca id[%d] del messaggio!!!)"),
          Level, ID_MSG_RESULT_PSW);

  setWindowTextByLangGlob(*this, ID_TITLE_RESULT_PSW, false);
  SET_TEXT(IDC_STATIC_INFO, msg);
  return true;
}
//--------------------------------------------------------------
int PassWord::getPsW(int requested, PWin* w, bool retUsed)
{
  if(inExec)
    return 0;
  inExec = true;

  if(ONLY_CONFIRM == requested) {
    int result = IDOK == onlyConfirmDialog(w).modal() ? requested : 0;
    inExec = false;
    return result;
    }
  // se primo ingresso, ricava valori dallo std_msg
  if((DWORD)-1 == maxTimePsw)
    calcPrph();

  if(!checkLevelFromPeriph(w))
    verifyValidityTime(w, GetTickCount());

  verifyPsW(requested, w);
  int ret = level;
  if(requested <= level) { // reimposta il time
//    if(prevLevelByPrph == -1)
      seconds = GetTickCount();
    if(!retUsed)
      ret = requested;
    }
  inExec = false;
  return ret;
}
//--------------------------------------------------------------
bool PassWord::checkLevelFromPeriph(PWin* w)
{
  if(!prph)
    return false;
  mainClient* mc = getParentWin<mainClient>(w);
  if(!mc) {
    prph = 0;
    return false;
    }
  genericPerif* p = mc->getGenPerif(prph);
  if(!p) {
    prph = 0;
    return false;
    }
  prfData data;
  data.lAddr = addr;
  data.typeVar = prfData::tWData;
  p->get(data);
  int psw = min(data.U.w, maxLevelByPrph);
  bool noCheckTime = false;
  if(psw >= level) {
    level = psw;
    noCheckTime = true;
    }
  // se si è azzerato/diminuito il livello, imposta il tempo come scaduto
  if(levelByPrph > psw) {
    seconds = GetTickCount() - maxTimePsw - 1;
    noCheckTime = false;
    }
  levelByPrph = psw;
//  if(prevLevelByPrph < 0)
//    prevLevelByPrph = psw;
  return noCheckTime && prevLevelByPrph == -1;
}

//----------------------------------------------------------------------------
//#define SUB_BASE_KEY _T("Software\\NPS_Programs\\NPSVisor")
#define KEY_STRING_1 _T("pLevel1")
#define KEY_STRING_2 _T("pLevel2")
#define KEY_STRING_3 _T("pLevel3")
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
  return (getDec(buff[0]) << 4) + getDec(buff[1]);
}
//----------------------------------------------------------------------------
bool writeKey(LPCTSTR name, LPCBYTE value, int len)
{
  myManageIni ini(SVISOR_INI_FILE);
  ini.parse();
  TCHAR t[256];
  for (int i = 0, j = 0; i < len; ++i, j += 2) {
    wsprintf(t + j, _T("%02X"), value[i]);
  }
  ini_param param = { name, t };
  ini.addParam(param, MAIN_PATH);
  return ini.save();
}
//----------------------------------------------------------------------------
bool readKey(LPCTSTR name, LPBYTE value, int len, bool writeIfNotFound)
{
  myManageIni ini(SVISOR_INI_FILE);
  ini.parse();
  LPCTSTR str = ini.getValue(name, MAIN_PATH);
  if(!str && writeIfNotFound)
    return writeKey(name, value, len);
  int l = _tcslen(str);
  l = min(l, len * 2);
  int j = 0;
  for(int i = 0; i < l; i += 2, ++j) {
    TCHAR t[3] = {};
    t[0] = str[i];
    t[1] = str[i + 1];
    value[j] = (BYTE) hexToInt(t);
    }
  for(; j < len; ++j)
    value[j] = 0;
  return true;
}
//----------------------------------------------------------------------------
#define MAX_PSW 3
//----------------------------------------------------------------------------
#define LEN_PSW 12
//----------------------------------------------------------------------------
struct pswData
{
  WORD Crc;
  WORD Key;
  BYTE criptBase[LEN_PSW];
  TCHAR regString[SIZE_A(KEY_STRING_1) + 1];
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static pswData psw_Data[MAX_PSW] =
{
  { // 123456
    0x086c, 111,
    {
      0x41, 0x0d, 0x4b, 0xc7, 0xb1, 0xed, 0x9c, 0x5e, 0xaa, 0xf3, 0x98, 0x4a
    },
    KEY_STRING_1
  },
  { // 987654
    0x0783, 132,
    {
      0x55, 0xe8, 0x90, 0x7a, 0x25, 0x11, 0x61, 0xa4, 0xb4, 0x19, 0x4d, 0xe0
    },
    KEY_STRING_2
  },
  { // npesse
    0x08f2, 123,
    {
      0xa3, 0x1c, 0xba, 0xcf, 0x7d, 0x83, 0xe6, 0xab, 0x54, 0x26, 0x32, 0x1f
    },
    KEY_STRING_3
  }
};
/*
"pLevel1"=hex:6c,08, 0x41, 0x0d, 0x4b, 0xc7, 0xb1, 0xed, 0x9c, 0x5e, 0xaa, 0xf3, 0x98, 0x4a
"pLevel2"=hex:83,07, 0x55, 0xe8, 0x90, 0x7a, 0x25, 0x11, 0x61, 0xa4, 0xb4, 0x19, 0x4d, 0xe0
"pLevel3"=hex:f2,08, 0xa3, 0x1c, 0xba, 0xcf, 0x7d, 0x83, 0xe6, 0xab, 0x54, 0x26, 0x32, 0x1f
*/
//----------------------------------------------------------------------------
#define LEN_KEY (LEN_PSW + sizeof(WORD))
#define COPY_CRC(buff, crc) *(WORD*)buff = crc
#define COPY_PSW(buff, psw) \
  memcpy(buff + sizeof(WORD), psw, LEN_PSW)
//----------------------------------------------------------------------------
#define GET_CRC(buff) *(WORD*)buff
#define GET_PSW(psw, buff) \
  memcpy(psw, buff + sizeof(WORD), LEN_PSW)

#define OFFS_PSW(buff) (buff + sizeof(WORD))
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static void fillBuff(LPBYTE buff)
{
  int l = strlen((char*)buff);
  for(int i = 0; i < l && i < LEN_PSW; ++i)
    if(buff[i] >= 'A' && buff[i] <= 'Z')
      buff[i] += (char)('a' - 'A');
  for(int i = l; i < LEN_PSW; ++i)
    buff[i] = ' ';
}
//----------------------------------------------------------------------------
#define C_(a) ((char)(a))
void PassWord::verifyPsW(int requested, PWin* w)
{
  if(requested <= level)
    return;

  char buff[20];
  TD_Psw(buff, w).modal();
  fillBuff((LPBYTE)buff);

  char result[20];
  BYTE oldVal[LEN_KEY + 2];
  int ok = 0;
  for(int i = 0; i < MAX_PSW && !ok; ++i) {
    COPY_CRC(oldVal, psw_Data[i].Crc);
    COPY_PSW(oldVal, psw_Data[i].criptBase);
    if(!readKey(psw_Data[i].regString, oldVal, LEN_KEY, true))
      break;

    WORD crc = crypt((LPBYTE)buff, (LPBYTE)result, psw_Data[i].Key, LEN_PSW);

    if(crc == GET_CRC(oldVal) && !memcmp(result, OFFS_PSW(oldVal), LEN_PSW))
      ok = i + 1;
    }

  if(ok) {
    level = ok;
    if(level >= requested)
      setPsw(w);
    }
  Changed = IDOK == showPasswordDialog(w, level).modal();
  waitForMessageResult = false;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class P_ChgPassword : public PModDialog
{
  public:
    P_ChgPassword(PWin* parent, int level, uint resId = IDD_CHG_PSW, HINSTANCE hinst = 0)  :
      PModDialog(parent, resId, hinst), Level(level) {  }
    virtual bool create();
    virtual void CmOk();
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  private:
    void showMsg(int lev);
    void setEnableEdit(int ix);
    int Level;
};
//----------------------------------------------------------------------------
bool PassWord::changePsw(int reqLev, PWin* w)
{
  if(ONLY_CONFIRM == reqLev)
    return false;
  int result = getPsW(reqLev, w, true);
  restartTime(0);

  if(result < reqLev || !result || result > MAX_LEVEL_PSW)
    return false;

  return IDOK == P_ChgPassword(w, result).modal();
}
//----------------------------------------------------------------------------
void P_ChgPassword::CmOk()
{
  struct infoPsw {
    BYTE buff[LEN_KEY + 2];
    uint idcCheck;
    uint idcEd1;
    uint idcEd2;
    bool enable;
    infoPsw() { ZeroMemory(this, sizeof(*this)); }
    };

  BYTE buffT[LEN_KEY + 2];
  infoPsw iP[MAX_PSW];
  iP[0].idcCheck = IDC_CHECKBOX_L1;
  iP[0].idcEd1 = IDC_EDIT_PSW_L1;
  iP[0].idcEd2 = IDC_EDIT_PSW_L1_C;

  iP[1].idcCheck = IDC_CHECKBOX_L2;
  iP[1].idcEd1 = IDC_EDIT_PSW_L2;
  iP[1].idcEd2 = IDC_EDIT_PSW_L2_C;

  iP[2].idcCheck = IDC_CHECKBOX_L3;
  iP[2].idcEd1 = IDC_EDIT_PSW_L3;
  iP[2].idcEd2 = IDC_EDIT_PSW_L3_C;

  bool enable = false;
  for(uint i = 0; i < SIZE_A(iP); ++i) {
    HWND child = GetDlgItem(*this, iP[i].idcCheck);
    iP[i].enable = BST_CHECKED == SendMessage(child, BM_GETCHECK, 0, 0);
    if(iP[i].enable) {
      GetDlgItemTextA(*this, iP[i].idcEd1, (char*)iP[i].buff, SIZE_A(iP[i].buff));
      fillBuff(iP[i].buff);

      GetDlgItemTextA(*this, iP[i].idcEd2, (char*)buffT, SIZE_A(buffT));
      fillBuff(buffT);
      if(memcmp(iP[i].buff, buffT, LEN_PSW)) {
        showMsg(1 + i);
        return;
        }
      enable = true;
      }
    }
  if(enable) {
    for(uint i = 0; i < SIZE_A(iP); ++i) {
      if(iP[i].enable) {
        WORD crc = crypt(iP[i].buff, OFFS_PSW(buffT), psw_Data[i].Key, LEN_PSW);
        COPY_CRC(buffT, crc);

        if(!writeKey(psw_Data[i].regString, buffT, LEN_KEY))
          return;
        }
      }
    svMessageBox(this, _T("Password changed"), _T("Result"),
        MB_OK | MB_ICONINFORMATION);
    }
  PModDialog::CmOk();
}
//----------------------------------------------------------------------------
void P_ChgPassword::showMsg(int lev)
{
  TCHAR msg[100];
  wsprintf(msg, _T("Password level %d don't match"), lev);
  svMessageBox(this, msg, _T("Error"), MB_ICONSTOP);
}
//----------------------------------------------------------------------------
bool P_ChgPassword::create()
{
  uint ids[] =  {
    IDC_EDIT_PSW_L1, IDC_EDIT_PSW_L1_C,
    IDC_EDIT_PSW_L2, IDC_EDIT_PSW_L2_C,
    IDC_EDIT_PSW_L3, IDC_EDIT_PSW_L3_C
    };
  for(uint i = 0; i < SIZE_A(ids); ++i)
    new svEdit(this, ids[i], MAX_LEN_PSW);

  if(!PModDialog::create())
    return false;

  LPCTSTR p = getString(ID_PSW_CHAR);
  if(p && *p)
//    p = _T("@");
    for(uint i = 0; i < SIZE_A(ids); ++i)
      SendMessage(GetDlgItem(*this, ids[i]), EM_SETPASSWORDCHAR, (UINT)*p, 0);

  HWND child = GetDlgItem(*this, IDC_CHECKBOX_L1);
  SendMessage(child, BM_SETCHECK, BST_CHECKED, 0);

  child = GetDlgItem(*this, IDC_CHECKBOX_L2);
  SendMessage(child, BM_SETCHECK, BST_UNCHECKED, 0);
  if(Level < 2)
    EnableWindow(child, false);
  child = GetDlgItem(*this, IDC_CHECKBOX_L3);
  SendMessage(child, BM_SETCHECK, BST_UNCHECKED, 0);
  if(Level < 3)
    EnableWindow(child, false);
  setEnableEdit(1);
  setEnableEdit(2);

  return true;
}
//----------------------------------------------------------------------------
LRESULT P_ChgPassword::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_CHECKBOX_L1:
          setEnableEdit(0);
          break;
        case IDC_CHECKBOX_L2:
          setEnableEdit(1);
          break;
        case IDC_CHECKBOX_L3:
          setEnableEdit(2);
          break;
        }
      break;
    }
  return PModDialog::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void P_ChgPassword::setEnableEdit(int ix)
{
  struct ids {
    int cb;
    int ed1;
    int ed2;
    };
  ids Ids[] = {
    { IDC_CHECKBOX_L1, IDC_EDIT_PSW_L1, IDC_EDIT_PSW_L1_C },
    { IDC_CHECKBOX_L2, IDC_EDIT_PSW_L2, IDC_EDIT_PSW_L2_C },
    { IDC_CHECKBOX_L3, IDC_EDIT_PSW_L3, IDC_EDIT_PSW_L3_C },
    };

  HWND child = GetDlgItem(*this, Ids[ix].cb);
  bool enable = BST_CHECKED == SendMessage(child, BM_GETCHECK, 0, 0);
  EnableWindow(GetDlgItem(*this, Ids[ix].ed1), enable);
  EnableWindow(GetDlgItem(*this, Ids[ix].ed2), enable);
}
//----------------------------------------------------------------------------
