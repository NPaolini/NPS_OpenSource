//-------- password.cpp ------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "defin.h"
#include "password.h"
#include "p_file.h"
#include "config.h"
//----------------------------------------------------------------------------
//#define SUB_BASE_KEY _T("Software\\NPS_Programs\\NPSVisor")
#define KEY_STRING_1 _T("pLevel1")
#define KEY_STRING_2 _T("pLevel2")
#define KEY_STRING_3 _T("pLevel3")
//----------------------------------------------------------------------------
#define STRINGER(a) #a

#define BASE_KEY HKEY_CURRENT_USER
//#define BASE_KEY HKEY_LOCAL_MACHINE
#define STR_BASE_KEY STRINGER(BASE_KEY)
//----------------------------------------------------------------------------
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
  myManageIni2 ini(INI_FILE_NAME);
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
  myManageIni2 ini(INI_FILE_NAME);
  ini.parse();
  LPCTSTR str = ini.getValue(name, MAIN_PATH);
  if(!str) {
    if(writeIfNotFound)
      return writeKey(name, value, len);
    return false;
    }
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
static pswData psw_Show =
{ // ennepiesse
    0x0a9e, 111,
    {
      0x15, 0x51, 0x16, 0x96, 0xf4, 0xb2, 0xd9, 0x0d, 0xf9, 0xb6, 0x98, 0x4a
    },
    KEY_STRING_1
};
/*
"pLevel1"=hex:9e,0a, 0x15, 0x51, 0x16, 0x96, 0xf4, 0xb2, 0xd9, 0x0d, 0xf9, 0xb6, 0x98, 0x4a
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
static char regStr[] =
"REGEDIT4\r\n\r\n[" STR_BASE_KEY "\\SOFTWARE\\NPS_Programs\\NPSVisor]\r\n";
//----------------------------------------------------------------------------
static
void saveLevelPsw(P_File& pf, LPBYTE psw, uint lev)
{
  if(255 == psw[LEN_KEY])
    return;
  char buff[100];
  wsprintfA(buff, "\"%s\"=hex:%02x", psw_Data[lev].regString, psw[0]);
  pf.P_writeString(buff);
  LPSTR p = buff;
  for(uint i = 1; i < LEN_KEY; ++i) {
    wsprintfA(p, ",%02x", psw[i]);
    p = p + strlen(p);
    }
  strcat(p, "\r\n");
  pf.P_writeString(buff);
}
//----------------------------------------------------------------------------
static
bool savePsw(LPCTSTR file, BYTE psw[MAX_PSW][LEN_KEY + 2])
{
  P_File pf(file, P_CREAT);
  if(!pf.P_open()) {
    MessageBox(0, _T("Impossibile creare il file"), _T("Errore"), MB_OK | MB_ICONSTOP);
    return false;
    }
  pf.P_writeString(regStr);
  for(int i = 0; i < MAX_PSW; ++i)
    saveLevelPsw(pf, psw[i], i);
  return true;
}
//----------------------------------------------------------------------------
void saveAllPsw()
{
  BYTE psw[MAX_PSW][LEN_KEY + 2];
  ZeroMemory(psw, sizeof(psw));
  bool success = true;
  for(int i = 0; i < MAX_PSW; ++i)
    if(!readKey(psw_Data[i].regString, psw[i], LEN_KEY, false)) {
      psw[i][LEN_KEY] = 255;
//      MessageBox(0, _T("Impossibile leggere il registry"), _T("Errore"), MB_OK | MB_ICONSTOP);
//      return;
      }
  if(savePsw(_T("NPS_sVisor_psw.reg"), psw))
    MessageBox(0, _T("Password salvate\r\n(cartella -> System, nome -> NPS_sVisor_psw.reg)"), _T("Successo"), MB_OK | MB_ICONINFORMATION);
}
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
    MessageBox(*this, _T("Password changed"), _T("Result"),
        MB_OK | MB_ICONINFORMATION);
    }
  PModDialog::CmOk();
}
//----------------------------------------------------------------------------
void P_ChgPassword::showMsg(int lev)
{
  TCHAR msg[100];
  wsprintf(msg, _T("Password level %d don't match"), lev);
  MessageBox(*this, msg, _T("Error"), MB_ICONSTOP);
}
//----------------------------------------------------------------------------
bool P_ChgPassword::create()
{
  if(!PModDialog::create())
    return false;
  HWND child = GetDlgItem(*this, IDC_CHECKBOX_L1);
  SendMessage(child, BM_SETCHECK, BST_CHECKED, 0);

  child = GetDlgItem(*this, IDC_CHECKBOX_L2);
  SendMessage(child, BM_SETCHECK, BST_UNCHECKED, 0);
  child = GetDlgItem(*this, IDC_CHECKBOX_L3);
  SendMessage(child, BM_SETCHECK, BST_UNCHECKED, 0);
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
class P_PromptPassword : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    P_PromptPassword(PWin* parent, uint resId = IDD_PROMPT_PSW, HINSTANCE hinst = 0)  :
      PModDialog(parent, resId, hinst) {  }
    virtual void CmOk();
};
//----------------------------------------------------------------------------
class P_ShowPassword : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    P_ShowPassword(PWin* parent, uint resId = IDD_SHOW_PSW, HINSTANCE hinst = 0)  :
      PModDialog(parent, resId, hinst) {  }
    bool create();
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  private:
    int id_timer;
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void showAllPsw(PWin* parent)
{
  if(IDOK == P_PromptPassword(parent).modal())
    P_ShowPassword(parent).modal();
}
//----------------------------------------------------------------------------
void P_PromptPassword::CmOk()
{
  char text[LEN_PSW + 1] = { ' ' };
  GetWindowTextA(GetDlgItem(*this, IDC_EDIT_PSW), text, SIZE_A(text));
  fillBuff((LPBYTE)text);
  BYTE buff[LEN_KEY + 1];
  WORD crc = crypt((LPBYTE)text, buff, psw_Show.Key, LEN_PSW);
  if(crc != psw_Show.Crc || memcmp(buff, psw_Show.criptBase, LEN_PSW)) {
    MessageBox(*this, _T("Password di accesso errata"), _T("Errore"), MB_ICONSTOP | MB_OK);
    CmCancel();
    }
  else
    baseClass::CmOk();
}
//----------------------------------------------------------------------------
bool P_ShowPassword::create()
{
  if(!baseClass::create())
    return false;

  char text[LEN_PSW + 1];
  BYTE buff[LEN_KEY + 1];
  uint idc[] = { IDC_EDIT_PSW_L1, IDC_EDIT_PSW_L2, IDC_EDIT_PSW_L3 };
  for(uint i = 0; i < SIZE_A(psw_Data); ++i) {
    if(!readKey(psw_Data[i].regString, buff, LEN_KEY, false))
      continue;
    WORD crc = crypt(OFFS_PSW(buff), (LPBYTE)text, psw_Data[i].Key, LEN_PSW);
    text[LEN_PSW] = 0;
    trim(text);
    SetWindowTextA(GetDlgItem(*this, idc[i]), text);
    }
  id_timer = SetTimer(*this, 2121, 60 * 1000, 0);
  return true;
}
//----------------------------------------------------------------------------
LRESULT P_ShowPassword::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_DESTROY:
    case WM_TIMER:
      if(id_timer) {
        KillTimer(hwnd, id_timer);
        id_timer = 0;
        }
      if(WM_TIMER == message)
        CmCancel();
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
