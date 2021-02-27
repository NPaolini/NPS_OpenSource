//------------ adrDlg.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
//----------------------------------------------------------------------------
#include "adrDlg.h"
#include "mainDlg.h"
#include "fillDlg.h"
#include "verifyDlg.h"
//-----------------------------------------------------------------
#define CRYPT_HEAD "CRYPNPS"
#define DIM_HEAD sizeof(CRYPT_HEAD)
//-----------------------------------------------------------------
#define CRYPT_STEP     1
#define DIM_KEY_CRYPT  8
//-----------------------------------------------------------------
DWORD load_CriptedTxt(LPCBYTE key, class setOfString& set, LPCTSTR name);
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define TIME2CHECK ftLastWriteTime
//----------------------------------------------------------------------------
static bool getFileDate(LPCTSTR file, FILETIME& time)
{
  WIN32_FIND_DATA FindFileData;
  HANDLE hf = FindFirstFile(file, &FindFileData);
  if(hf != INVALID_HANDLE_VALUE) {
    time = FindFileData.TIME2CHECK;
    FindClose(hf);
    return true;
    }
  if(_T('#') == *file)
    return false;
  TCHAR t[_MAX_PATH] = _T("#");
  _tcscpy_s(t + 1, SIZE_A(t) - 1, file);
  return getFileDate(t, time);
}
//----------------------------------------------------------------------------
static P_File* getFileLock(LPCTSTR file)
{
  WIN32_FIND_DATA FindFileData;
  HANDLE hf = FindFirstFile(file, &FindFileData);
  P_File* pf = 0;
  if(hf != INVALID_HANDLE_VALUE) {
    pf = new P_File(file);
    FindClose(hf);
    if(pf->P_open())
      return pf;
    delete pf;
    return 0;
    }
  if(_T('#') == *file)
    return 0;
  TCHAR t[_MAX_PATH] = _T("#");
  _tcscpy_s(t + 1, SIZE_A(t) - 1, file);
  return getFileLock(t);
}
//----------------------------------------------------------------------------
class ClearChoose : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    ClearChoose(PWin* parent, int& from, int& to) :
      baseClass(parent, IDD_DIALOG_CLEAR), From(from), To(to)
      {
        int idBmp[] = { IDB_OK, IDB_CANC };
        int idBtn[] = { IDOK, IDCANCEL };

        PVect<PBitmap*> vBmp;
        HINSTANCE hi = getHInstance();
        POwnBtn::colorRect cr(GetSysColor(COLOR_BTNFACE), RGB(0, 0, 0xff), 1, GetSysColor(COLOR_BTNTEXT));
        for(uint i = 0; i < SIZE_A(Bmp); ++i) {
          Bmp[i] = new PBitmap(idBmp[i], hi);
          POwnBtnImageStd* btn = new POwnBtnImageStd(this, idBtn[i], Bmp[i]);
          btn->setColorRect(cr);
          }

      }

    ~ClearChoose()
    {
      destroy();
      for(uint i = 0; i < SIZE_A(Bmp); ++i)
        delete Bmp[i];
    }

    bool create();
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CmOk();
    void checkEnable();

  private:
    int& From;
    int& To;
    class PBitmap* Bmp[2];
};
//----------------------------------------------------------------------------
bool ClearChoose::create()
{
  if(!baseClass::create())
    return false;
  if(-1 == From && -1 == To)
    SET_CHECK(IDC_CHECK_CLEAR_ALL);
  else {
    SET_INT(IDC_EDIT_CLEAR_FROM, From);
    SET_INT(IDC_EDIT_CLEAR_TO, To);
    }
  checkEnable();
  return true;
}
//----------------------------------------------------------------------------
void ClearChoose::CmOk()
{
  if(IS_CHECKED(IDC_CHECK_CLEAR_ALL)) {
    From = -1;
    To = -1;
    }
  else {
    GET_INT(IDC_EDIT_CLEAR_FROM, From);
    GET_INT(IDC_EDIT_CLEAR_TO, To);
    }
  baseClass::CmOk();
}
//----------------------------------------------------------------------------
void ClearChoose::checkEnable()
{
  bool enable = !IS_CHECKED(IDC_CHECK_CLEAR_ALL);
  ENABLE(IDC_EDIT_CLEAR_FROM, enable);
  ENABLE(IDC_EDIT_CLEAR_TO, enable);
}
//----------------------------------------------------------------------------
LRESULT ClearChoose::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_CHECK_CLEAR_ALL:
          checkEnable();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
PD_Adr::PD_Adr(PWin* parent, uint id_adr, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance),  idAdr(id_adr), currPos(0),
    dirty(false), maxShow(getNumOfRow(id_adr)), baseAddr(1), onOpen(false),
    Brush(CreateSolidBrush(bkgColor)), Brush2(CreateSolidBrush(bkgColor2)), Brush3(CreateSolidBrush(bkgColor3)),
    oldStat(1), lastModified(I64_TO_FT(0)), lockFile(0), noConvert(false),
    currPrphCopyFrom(-1), enablePrphCopyFrom(false), idFocus(-1)
{
  Filename[0] = 0;
  Clear();
  int idBmp[] = { IDB_OPEN, IDB_REFRESH, IDB_SAVE, IDB_SAVE, IDB_FILL, IDB_COPY, IDB_CLEAR, IDB_DONE };
  int idBtn[] = { IDC_BUTTON_OPEN, ID_UNLOCK, IDC_BUTTON_SAVE, IDC_BUTTON_SAVE_AS, IDC_BUTTON_FILL, IDC_BUTTON_COPY, IDC_BUTTON_CLEAR, ID_EXIT_APP };

  for(uint i = 0; i < SIZE_A(idBmp); ++i) {
    PBitmap* Bmp = new PBitmap(idBmp[i], getHInstance());
    POwnBtnImageStd* btn = new POwnBtnImageStd(this, idBtn[i], Bmp, POwnBtnImageStd::wLeft, true);
    }
  for(uint i = 0; i < MAX_GROUP; ++i)
    Rows[i] = 0;
}
//----------------------------------------------------------------------------
PD_Adr::~PD_Adr()
{
  delete lockFile;
  saveNumOfRow(maxShow, idAdr);
  for(int i = 0; i < MAX_GROUP; ++i)
    delete Rows[i];
  destroy();
  DeleteObject(HGDIOBJ(Brush));
  DeleteObject(HGDIOBJ(Brush2));
  DeleteObject(HGDIOBJ(Brush3));
}
//----------------------------------------------------------------------------
void PD_Adr::setDirty(bool set)
{
  if(onOpen)
    return;
  if(set) {
    if(!lockFile && Filename[0])
      lockFile = getFileLock(Filename);
    }
  else
    safeDeleteP(lockFile);
  dirty = set;
}
//-----------------------------------------------------------------------------
bool PD_Adr::isHex()
{
  return IS_CHECKED(IDC_CHECK_HEX_ADDR) ^ noConvert;
}
//------------------------------------------------------------------------------
void PD_Adr::changeValue()
{
  DWORD pos = currPos;
  noConvert = true;
  bool oldDirty = isDirty();
  onOpen = true;
  saveCurrData();
  noConvert = false;
  for(int i = 0; i < MAX_GROUP; ++i, ++pos)
    Rows[i]->setAddr(Cfg[pos].addr, baseAddr);
  onOpen = false;
  if(!oldDirty)
    dirty = false;
}
//----------------------------------------------------------------------------
uint PD_Adr::getNByte()
{
  int dummy;
  int type = getNumAndSelCB(GetDlgItem(*this, IDC_COMBOBOX_DEF_DATA_TYPE), dummy);
  return type;
}
//----------------------------------------------------------------------------
void PD_Adr::setFileName()
{
  wsprintf(Filename, _T("prph_%d_data.ad7"), idAdr);
}
//----------------------------------------------------------------------------
void PD_Adr::setPrevFileName()
{
  wsprintf(Filename, _T("prf_%d_data.adr"), idAdr);
}
//----------------------------------------------------------------------------
void PD_Adr::Clear()
{
  setDirty(toBool(Filename[0]));
  Filename[0] = 0;
  for(uint i = 0; i < SIZE_A(Cfg); ++i) {
    Cfg[i].ipAddr = 0;
    Cfg[i].port = 0;
    Cfg[i].db = 0;
    Cfg[i].addr = 0;
    Cfg[i].type = 0;
    Cfg[i].action = 0;
    Cfg[i].vMin = 0;
    Cfg[i].vMax = 0;
    Cfg[i].vMinP = 0;
    Cfg[i].vMaxP = 0;
    Cfg[i].text[0] = 0;
    }
}
//----------------------------------------------------------------------------
void PD_Adr::Clear(int from, int to)
{
  setDirty(toBool(Filename[0]));
  from = min((uint)from, SIZE_A(Cfg) - 1);
  to = min((uint)to, SIZE_A(Cfg) - 1);
  for(int i = from; i <= to; ++i) {
    Cfg[i].ipAddr = 0;
    Cfg[i].port = 0;
    Cfg[i].db = 0;
    Cfg[i].addr = 0;
    Cfg[i].type = 0;
    Cfg[i].action = 0;
    Cfg[i].vMin = 0;
    Cfg[i].vMax = 0;
    Cfg[i].vMinP = 0;
    Cfg[i].vMaxP = 0;
    Cfg[i].text[0] = 0;
    }
}
//----------------------------------------------------------------------------
bool PD_Adr::create()
{
  POINT pt = { X_INIT, Y_INIT };
  Rows[0] = new PRow(this, IDC_EDIT_BYTE1_1, pt, -1);

  pt.y += H_EDIT - 1;
  for(int i = 1; i < MAX_GROUP - 1; ++i) {
    Rows[i] = new PRow(this, IDC_EDIT_BYTE1_1 + i, pt, 0);
    pt.y += H_EDIT - 1;
    }

  Rows[MAX_GROUP - 1] = new PRow(this, IDC_EDIT_BYTE1_1 + MAX_GROUP - 1, pt, 1);

  new PEdit(this, IDC_EDIT_MAX_ROW);
  new PEdit(this, IDC_EDIT_TIMER);
  new PEditCR(this, IDC_EDIT_GOTO_ROW, IDC_BUTTON_GOTO_ROW);
  new PEditCR(this, IDC_EDIT_SEARCH_TEXT, IDC_BUTTON_SEARCH_TEXT);
  if(!baseClass::create())
    return false;

  PRect r;
  GetWindowRect(GetDlgItem(*this, IDC_STATIC_IP), r);
  MapWindowPoints(HWND_DESKTOP, *this, (LPPOINT)(LPRECT)r, 2);

  int dx = r.left - X_INIT - 3;
  GetWindowRect(GetDlgItem(*this, IDC_STATIC_DESCR), r);
  double scaleX = r.left;
  GetWindowRect(GetDlgItem(*this, IDC_EDIT_BYTE1_1 + OFFSET_DESCR), r);
  scaleX /= r.left;
  GetWindowRect(GetDlgItem(*this, IDC_STATICTEXT_1), r);
  double scaleY = r.Height();
  GetWindowRect(GetDlgItem(*this, IDC_EDIT_BYTE1_1), r);
  scaleY /= r.Height();

  for(int i = 0; i < MAX_GROUP; ++i) {
    GetWindowRect(GetDlgItem(*this, IDC_STATICTEXT_1 + i), r);
    int dy = r.top;
    GetWindowRect(GetDlgItem(*this, IDC_EDIT_BYTE1_1 + i), r);
    dy -= r.top;
    Rows[i]->offset(dx, dy, scaleX, scaleY);
    }

  SendMessage(GetDlgItem(*this, IDC_RADIOBUTTON_BYTE), BM_SETCHECK, BST_CHECKED, 0);

  Clear();
  fillCbDataType(GetDlgItem(*this, IDC_COMBOBOX_DEF_DATA_TYPE), 2);
  setFileName();
  if(!loadFile()) {
    setPrevFileName();
    if(!loadFile())
      Filename[0] = 0;
    }
  loadData();
  setScrollRange();

//  SendMessage(GetDlgItem(*this, IDC_CHECKBOX_ZERO), BM_SETCHECK, BST_CHECKED, 0);
  changeBase();

  setTitle();

  SetTimer(*this, 111, 1000, 0);

  HICON icon = LoadIcon(getHInstance(), MAKEINTRESOURCE(IDI_ICON1));
  SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_BIG), LPARAM(icon));
  SendMessage(getHandle(), WM_SETICON, WPARAM(ICON_SMALL), LPARAM(icon));

  SetDlgItemInt(*this, IDC_EDIT_MAX_ROW, maxShow, 0);

  HWND firstEdit = GetDlgItem(*this, IDC_EDIT_BYTE1_1);
  SetFocus(firstEdit);

  if(1 >= idAdr || idAdr > 9) {
    ShowWindow(GetDlgItem(*this, IDC_STATICTEXT_TIMER), SW_HIDE);
    ShowWindow(GetDlgItem(*this, IDC_EDIT_TIMER), SW_HIDE);
    }

  return true;
}
//----------------------------------------------------------------------------
#define MSG_DATA_CHANGED _T("I dati sono stati modificati esternamente al programma,\nvuoi ricaricarli?")
#define MSG_DATA_CHANGED_TIT _T("Attenzione!")
//----------------------------------------------------------------------------
void PD_Adr::checkExternalChange()
{
  if(!*Filename || lockFile)
    return;
  FILETIME ft;
  if(!getFileDate(Filename, ft))
    return;
  if(ft > lastModified) {
    lastModified = ft;
    if(IDYES != MessageBox(*this, MSG_DATA_CHANGED, MSG_DATA_CHANGED_TIT, MB_YESNO | MB_ICONINFORMATION))
      return;
    loadFile();
    loadData();
    }
}
//-----------------------------------------------------------------------------
bool PD_Adr::orderSet(orderedPAddr& set)
{
  saveCurrData();
  for(uint i = 0; i < SIZE_A(Cfg); ++i) {
    if(Cfg[i].type) {
      addrConv* p = new addrConv(Cfg[i].ipAddr, Cfg[i].port, Cfg[i].db, Cfg[i].addr, i, Cfg[i].type, Cfg[i].action);
      set.Add(p);
      }
    }
  return set.setFirst();
}
//----------------------------------------------------------------------------
void PD_Adr::verifyAll()
{
  orderedPAddr set;
  if(!orderSet(set))
    return;
  int dummy;
  int type = getNumAndSelCB(GetDlgItem(*this, IDC_COMBOBOX_DEF_DATA_TYPE), dummy);
  int typeShow = IS_CHECKED(IDC_RADIOBUTTON_WORD) ? 2 : IS_CHECKED(IDC_RADIOBUTTON_DWORD) ? 4 : 1;

  EnableWindow(GetDlgItem(*this, IDC_BUTTON_VERIFY), false);
  bool step1 = IS_CHECKED(IDC_CHECKBOX_STEP1);
  PD_Verify* vrf = new PD_Verify(set, type, typeShow, step1, this);
  vrf->create();
}
//----------------------------------------------------------------------------
void PD_Adr::setScrollRange()
{
  HWND hsb = GetDlgItem(*this, IDC_SCROLLBAR_COUNT);
  if(hsb) {
    SCROLLINFO si;
    memset(&si, 0, sizeof(si));
    si.cbSize = sizeof(si);
    si.fMask = SIF_PAGE | SIF_RANGE;
    si.nPage = PAGE_SCROLL_LEN;
    si.nMin = 0;
    si.nMax = maxShow - 1;
    SendMessage(hsb, SBM_SETSCROLLINFO, 1, (LPARAM)&si);
    }
}
//----------------------------------------------------------------------------
bool PD_Adr::preProcessMsg(MSG& msg)
{
  switch(msg.message) {
    case WM_KEYDOWN:
      switch(msg.wParam) {
        case VK_NEXT:
          PostMessage(*this, WM_CUSTOM_VSCROLL, SB_PAGERIGHT, 0);
          break;
        case VK_PRIOR:
          PostMessage(*this, WM_CUSTOM_VSCROLL, SB_PAGELEFT, 0);
          break;
        }
      break;
    }
  if(!toBool(IsDialogMessage(getHandle(), &msg)))
    return baseClass::preProcessMsg(msg);
  return true;
}
//----------------------------------------------------------------------------
void PD_Adr::getInfo(commonInfo& ci)
{
  saveCurrData();
  ci.base = true;//IS_CHECKED(IDC_CHECKBOX_ZERO);
  if(IS_CHECKED(IDC_RADIOBUTTON_WORD))
    ci.baseAddr = 2;
  else if(IS_CHECKED(IDC_RADIOBUTTON_DWORD))
    ci.baseAddr = 4;
  else
    ci.baseAddr = 1;

  GET_INT(IDC_EDIT_MAX_ROW, ci.totRow);
  ci.currRow = currPos;
  GET_INT(IDC_EDIT_GOTO_ROW, ci.currGotoRow);
  ci.nByteBase = getNByte();
}
//----------------------------------------------------------------------------
void PD_Adr::setInfo(const commonInfo& ci)
{
/*
  bool zero = IS_CHECKED(IDC_CHECKBOX_ZERO);
  if(ci.base != zero) {
    SET_CHECK_SET(IDC_CHECKBOX_ZERO, ci.base);
    changeBase();
    }
*/
  DWORD dwV;
  GET_INT(IDC_EDIT_MAX_ROW, dwV);
  if(ci.totRow != dwV) {
    SET_INT(IDC_EDIT_MAX_ROW, ci.totRow);
    changeNumRow();
    }
  if(currPos != ci.currRow) {
    HWND child = GetDlgItem(*this, IDC_SCROLLBAR_COUNT);
    evVScrollBar(child, SB_THUMBTRACK, ci.currRow);
    }
  SET_INT(IDC_EDIT_GOTO_ROW, ci.currGotoRow);

  if(!IS_CHECKED(IDC_RADIOBUTTON_WORD) && ci.baseAddr == 2) {
    SET_CHECK(IDC_RADIOBUTTON_WORD);
    SET_CHECK_SET(IDC_RADIOBUTTON_DWORD, false);
    SET_CHECK_SET(IDC_RADIOBUTTON_BYTE, false);
    setBaseAddr(IDC_RADIOBUTTON_WORD);
    }
  else if(!IS_CHECKED(IDC_RADIOBUTTON_DWORD) && ci.baseAddr == 4) {
    SET_CHECK(IDC_RADIOBUTTON_DWORD);
    SET_CHECK_SET(IDC_RADIOBUTTON_WORD, false);
    SET_CHECK_SET(IDC_RADIOBUTTON_BYTE, false);
    setBaseAddr(IDC_RADIOBUTTON_DWORD);
    }
  else if(!IS_CHECKED(IDC_RADIOBUTTON_BYTE) && ci.baseAddr == 1) {
    SET_CHECK(IDC_RADIOBUTTON_BYTE);
    SET_CHECK_SET(IDC_RADIOBUTTON_WORD, false);
    SET_CHECK_SET(IDC_RADIOBUTTON_DWORD, false);
    setBaseAddr(IDC_RADIOBUTTON_BYTE);
    }
}
//-----------------------------------------------------------------------------
#define WM_MOUSEWHEEL                   0x020A
#define WHEEL_DELTA                     120
//-----------------------------------------------------------------------------
void PD_Adr::evMouseWheel(short delta, short /*x*/, short /*y*/)
{
  int tD = delta;
  tD *= PAGE_SCROLL_LEN;
  tD /= WHEEL_DELTA;
  uint msg = WM_VSCROLL;
  HWND child = GetDlgItem(*this, IDC_SCROLLBAR_COUNT);

  if(IsWindowEnabled(child)) {
    int curr = SendMessage(child, SBM_GETPOS, 0, 0);
    curr -= tD;
    if(curr < 0)
      curr = 0;
    else if(curr > (int)MAX_V_SCROLL)
      curr = MAX_V_SCROLL;
    SendMessage(*this, msg, MAKEWPARAM(SB_THUMBTRACK, curr),(LPARAM)child);
    }
}
//----------------------------------------------------------------------------
LRESULT PD_Adr::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_ENABLE_VERIFY:
      EnableWindow(GetDlgItem(*this, IDC_BUTTON_VERIFY), true);
      break;

    case WM_CLIENT_WHEEL:
      evMouseWheel(HIWORD(wParam), LOWORD(lParam), HIWORD(lParam));
      return 0;

    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDOK:
        case IDCANCEL:
            return 0;
        case ID_EXIT_APP:
          PostMessage(*getParent(), WM_COMMAND, MAKEWPARAM(ID_EXIT_APP, 0), 0);
          break;
        case IDC_BUTTON_CLEAR:
          do {
            saveCurrData();
            static int from = -1;
            static int to = -1;
            bool ok = IDOK == ClearChoose(this, from, to).modal();
            if(ok) {
              if(-1 == from && -1 == to)
                Clear();
              else
                Clear(from, to);
              loadData();
              }
            } while(false);
          break;
        case ID_UNLOCK:
          if(IDYES == MessageBox(*this, _T("Vuoi annullare le modifiche fatte a questa periferica?"),
                  _T("Attenzione"), MB_YESNO | MB_ICONINFORMATION)) {
            loadFile();
            loadData();
            }
          break;
        case IDC_BUTTON_CLIPBOARD:
          copyFromClipboard();
          break;
        case IDC_BUTTON_SAVE:
          saveData();
          break;
        case IDC_BUTTON_SAVE_AS:
          PostMessage(*getParent(), WM_CLIENT_REQ, MAKEWPARAM(CM_SAVE_ALL, 0), 0);
          break;
        case IDC_BUTTON_OPEN:
          openFile();
          break;
//        case IDC_CHECKBOX_ZERO:
//          changeBase();
//          break;
        case IDC_CHECK_HEX_ADDR:
          changeValue();
          break;
        case IDC_BUTTON_FILL:
          fill();
          break;
        case IDC_BUTTON_COPY:
          copy();
          break;

        case IDC_RADIOBUTTON_BYTE:
        case IDC_RADIOBUTTON_WORD:
        case IDC_RADIOBUTTON_DWORD:
          setBaseAddr(LOWORD(wParam));
          break;
        case IDC_BUTTON_VERIFY:
          verifyAll();
          break;
        case IDC_BUTTON_GOTO_ROW:
          do {
            DWORD pos;
            GET_INT(IDC_EDIT_GOTO_ROW, pos);
            HWND child = GetDlgItem(*this, IDC_SCROLLBAR_COUNT);
            evVScrollBar(child, SB_THUMBTRACK, pos);
            } while(false);
          break;
        case IDC_BUTTON_SEARCH_TEXT:
          searchText();
          break;
        }
      switch(HIWORD(wParam)) {
        case EN_KILLFOCUS:
          switch(LOWORD(wParam)) {
            case IDC_EDIT_MAX_ROW:
              if(SendMessage((HWND)lParam, EM_GETMODIFY, 0, 0))
                changeNumRow();
              break;
            }
          break;
        case LBN_SELCHANGE:
          setDirty(true);
          break;
        case EN_CHANGE:
          switch(LOWORD(wParam)) {
            case IDC_EDIT_MAX_ROW:
            case IDC_EDIT_GOTO_ROW:
            case IDC_EDIT_SEARCH_TEXT:
              break;
            default:
              if(!canIgnoreKillFocus())
                setDirty(true);
            }
          break;
        case EN_SETFOCUS:
          checkRowWithFocus(LOWORD(wParam));
          break;
        }
      break;

    case WM_CUSTOM_VSCROLL:
      lParam = (LPARAM)GetDlgItem(*this, IDC_SCROLLBAR_COUNT);
      // passa sotto
    case WM_VSCROLL:
      evVScrollBar((HWND)lParam, LOWORD(wParam), HIWORD(wParam));
      break;

//    case WM_ENTERIDLE:
    case WM_TIMER:
      checkBtn();
      break;

    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLORSTATIC:
      do {
        HBRUSH result = evCtlColor((HDC)wParam, (HWND)lParam, message);
        if(result)
          return (LRESULT)result;
        } while(false);
      break;

    case WM_DESTROY:
      KillTimer(hwnd, 111);
      break;

    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
static bool myIcmp(LPCTSTR str1, LPCTSTR str2, uint len)
{
  for(uint i = 0; i < len; ++i, ++str1, ++str2)
    if(_totlower(*str1) != *str2)
      return false;
  return true;
}
//----------------------------------------------------------------------------
static bool foundText(LPCTSTR text, LPCTSTR toFind, uint len)
{
  if(!*text)
    return false;
  int l = _tcslen(text) - len + 1;
  for(int i = 0; i < l; ++i)
    if(myIcmp(text + i, toFind, len))
      return true;
  return false;
}
//----------------------------------------------------------------------------
int gSearchText(HWND edit, const baseRow* cfg, int currPos)
{
  static int last = -1;
  TCHAR t[512];
  GetWindowText(edit, t, SIZE_A(t));
  trim(lTrim(t));
  if(!*t)
    return -1;
  if(currPos == last)
    ++currPos;
  last = currPos;
  uint len = 0;
  for(; t[len]; ++len)
    t[len] = _totlower(t[len]);

  for(uint i = 0; i < MAX_ADDRESSES; ++i) {
    int pos = (currPos + i) % MAX_ADDRESSES;
    if(foundText(cfg[pos].text, t, len)) {
      last = pos;
      return pos;
      }
    }
  return -1;
}
//----------------------------------------------------------------------------
void PD_Adr::searchText()
{
  int pos = gSearchText(GetDlgItem(*this, IDC_EDIT_SEARCH_TEXT), Cfg, currPos);
  if(pos >= 0)
    evVScrollBar(GetDlgItem(*this, IDC_SCROLLBAR_COUNT), SB_THUMBTRACK, pos);
}
//----------------------------------------------------------------------------
void PD_Adr::checkRowWithFocus(uint id)
{
  int idF = -1;
  for(uint i = 0; i < SIZE_A(Rows); ++i) {
    if(Rows[i]->hasId(id)) {
      idF = i;
      break;
      }
    }
  if(idF >= 0 && idFocus != idF) {
    if(idFocus >= 0) {
      Rows[idFocus]->invalidate();
      InvalidateRect(GetDlgItem(*this, IDC_STATICTEXT_1 + idFocus), 0 ,1);
      }
    idFocus = idF;
    Rows[idFocus]->invalidate();
    InvalidateRect(GetDlgItem(*this, IDC_STATICTEXT_1 + idFocus), 0 ,1);
    }
}
//------------------------------------------------------------------
#define U_(a) ((unsigned)(a))
//------------------------------------------------------------------
static
LPTSTR get_line(LPTSTR buff, int len)
{
  if(len < 0)
    return 0;
  int i;
  LPTSTR p = buff;
  for(i = 0; i < len; ++i, ++p)
    if(U_(*p) == _T('\r') || U_(*p) == _T('\n'))
      break;
  if(i == len)
    return 0;
  TCHAR c = *p;
  *p++ = 0;
  if(++i == len)
    return 0;

  for(; i < len; ++i, ++p)
    if(U_(*p) != _T('\r') && U_(*p) != _T('\n'))
      break;

  return(i < len ? p : 0);
}
//----------------------------------------------------------------------------
void PD_Adr::fillRowByClipboard(LPTSTR lptstr)
{
  int offs = 0;
  int dim = _tcslen(lptstr);
  for(int pos = currPos; lptstr && pos < MAX_ADDRESSES; ++pos) {
    LPTSTR next = get_line(lptstr, dim - offs);
    _tcscpy_s(Cfg[pos].text, lptstr);
    int len = next ? next - lptstr : dim - offs;
    offs += len;
    lptstr = next;
    }
  loadData();
  setDirty(true);
}
//----------------------------------------------------------------------------
void PD_Adr::copyFromClipboard()
{
#ifdef UNICODE
  DWORD cf_text = CF_UNICODETEXT;
#else
  DWORD cf_text = CF_TEXT;
#endif
  if(!IsClipboardFormatAvailable(cf_text))
    return;
  if(!OpenClipboard(*this))
    return;
  HGLOBAL hglb = GetClipboardData(cf_text);
  while(hglb) {
    LPTSTR lptstr = (LPTSTR)GlobalLock(hglb);
    if(!lptstr)
      break;
    fillRowByClipboard(lptstr);
    GlobalUnlock(hglb);
    break;
    }
  CloseClipboard();
}
//----------------------------------------------------------------------------
void PD_Adr::changeNumRow()
{
  maxShow = GetDlgItemInt(*this, IDC_EDIT_MAX_ROW, 0, 0);
  if(maxShow > MAX_ADDRESSES) {
    maxShow = MAX_ADDRESSES;
    SetDlgItemInt(*this, IDC_EDIT_MAX_ROW, maxShow, 0);
    }
  setScrollRange();
  loadData();
}
//----------------------------------------------------------------------------
void PD_Adr::changeBase()
{
//  int zeroBased = IS_CHECKED(IDC_CHECKBOX_ZERO) ? 0 : 1;
  int pos = currPos;// + zeroBased;
  for(int i = 0; i < MAX_GROUP; ++i, ++pos) {
    TCHAR buff[10];
    wsprintf(buff, _T("%d"), pos);
    SetDlgItemText(*this, IDC_STATICTEXT_1 + i, buff);
    }
}
//----------------------------------------------------------------------------
void PD_Adr::fill()
{
  saveCurrData();
  baseRow* tmpCfg = new baseRow[MAX_ADDRESSES];
  for(int i = 0; i < MAX_ADDRESSES; ++i) {
    tmpCfg[i] = Cfg[i];
    tmpCfg[i].addr /= baseAddr;
    }
  if(IDOK == PD_Fill(tmpCfg, MAX_ADDRESSES, this).modal())  {
    for(int i = 0; i < MAX_ADDRESSES; ++i) {
      tmpCfg[i].addr *= baseAddr;
      Cfg[i] = tmpCfg[i];
      }
    loadData();
    setDirty(true);
    }
  delete []tmpCfg;
}
//----------------------------------------------------------------------------
void PD_Adr::copy()
{
  saveCurrData();
  static infoCopy iCopy;

  PD_Address* pda = dynamic_cast<PD_Address*>(getParent());
  if(!pda)
    return;
  pda->fillBit(iCopy.sourcePrph);
  iCopy.currPrph = currPrphCopyFrom;
  if(enablePrphCopyFrom)
    iCopy.flag |= infoCopy::ePrph;
  else
    iCopy.flag &= ~infoCopy::ePrph;
  if(IDOK == PD_Copy(iCopy, this).modal())  {
    currPrphCopyFrom = iCopy.currPrph;
    enablePrphCopyFrom = toBool(iCopy.flag & infoCopy::ePrph);
    baseRow* source = getRows();
    if(enablePrphCopyFrom) {
      source = pda->getRows(currPrphCopyFrom);
      if(!source)
        return;
      }
    uint nblock = max(iCopy.numBlock, 1);
    infoCopy icTmp = iCopy;
    // la copia/spostamento potrebbe accavallarsi se si usa la stessa periferica, allora
    // per non complicare il codice, si esegue una copia di tutte le righe e si usa la copia come sorgente.
    // Nel caso di spostamento, si esegue prima la cancellazione delle righe e poi la copia dal sorgente
    // Se invece lo spostamento avviene tra due periferiche diverse, la cancellazione del sorgente avviene
    // durante il ciclo

    baseRow* CfgTmp = 0;
    if(!enablePrphCopyFrom) {
      CfgTmp = new baseRow[MAX_ADDRESSES];
      for(uint i = 0; i < MAX_ADDRESSES; ++i)
        CfgTmp[i] = Cfg[i];
      source = CfgTmp;
      if(iCopy.move) {
        icTmp.move = false;
        infoCopy icTmp2 = iCopy;
        int len = icTmp2.numData;
        if(icTmp2.sourceAddr < icTmp2.targetAddr)
          len = min(len, MAX_ADDRESSES - (int)(icTmp2.targetAddr + len));
        else
          len = min(len, MAX_ADDRESSES - (int)(icTmp2.sourceAddr + len));
        if(len) {
          for(uint i = icTmp2.sourceAddr, k = 0; k < (uint)len; ++i, ++k)
            ZeroMemory(Cfg + i, sizeof(Cfg[i]));
          }
        }
      }
    bool offsProgr = toBool(iCopy.flag & infoCopy::eOffsetProgr);
    int addrOffs = iCopy.addrOffs * baseAddr;
    icTmp.addrOffs = 0;
    for(uint n = 0; n < nblock; ++n) {
      int len = icTmp.numData;
      if(icTmp.sourceAddr < icTmp.targetAddr)
        len = min(len, MAX_ADDRESSES - (int)(icTmp.targetAddr + len));
      else
        len = min(len, MAX_ADDRESSES - (int)(icTmp.sourceAddr + len));
      if(len <= 0)
        break;
      if(n || !(iCopy.flag & infoCopy::eNoFirstOffset)) {
        icTmp.dbOffs += iCopy.dbOffs;
        if(offsProgr)
          icTmp.addrOffs += addrOffs;
        else
          icTmp.addrOffs = addrOffs;
        }
      for(uint i = icTmp.sourceAddr, j = icTmp.targetAddr, k = 0; k < (uint)len; ++i, ++j, ++k) {
        if(!iCopy.checkNoAction || source[i].action) {
          Cfg[j] = source[i];
          if(n || !(iCopy.flag & infoCopy::eNoFirstOffset)) {
            if(toBool(icTmp.flag & infoCopy::eDb))
              Cfg[j].db += icTmp.dbOffs;
            if(toBool(icTmp.flag & infoCopy::eAddr))
              Cfg[j].addr += icTmp.addrOffs;
            }
          }
        if(icTmp.move)
          ZeroMemory(source + i, sizeof(source[i]));
        }
      icTmp.targetAddr += iCopy.offsBlock;
      }
    delete []CfgTmp;
    if(enablePrphCopyFrom && iCopy.move)
      pda->setDirty(currPrphCopyFrom);
    loadData();
    setDirty(true);
    }
}
//------------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool PD_Adr::saveOnExit()
{
  if(dirty) {
    TCHAR buff[64];
    wsprintf(buff, _T("I dati sono stati modificati [Prph %d]"), getId());
    int choose = MessageBox(*this, _T("Vuoi effettuare il salvataggio?"),
                                   buff, MB_YESNOCANCEL | MB_ICONINFORMATION);
    switch(choose) {
      case IDCANCEL:
        return false;
      case IDYES:
        dirty = !toBool(*Filename);
        if(!saveData())
          return false;
        break;
      }
    }
  return true;
}
//------------------------------------------------------------------------------
void PD_Adr::evVScrollBar(HWND child, int flags, int pos)
{
  int curr = SendMessage(child, SBM_GETPOS, 0, 0);
  switch(flags) {
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
      break;
    case SB_ENDSCROLL:
      pos = curr;
      break;
    case SB_LEFT:
      pos = 0;
      break;
    case SB_RIGHT:
      pos = MAX_V_SCROLL;
      break;
    case SB_LINELEFT:
      pos = curr - 1;
      break;
    case SB_LINERIGHT:
      pos = curr + 1;
      break;
    case SB_PAGELEFT:
      pos = curr - PAGE_SCROLL_LEN;
      break;
    case SB_PAGERIGHT:
      pos = curr + PAGE_SCROLL_LEN;
      break;
    }
  if(pos < 0)
    pos = 0;
  else if(pos > (int)MAX_V_SCROLL)
    pos = MAX_V_SCROLL;
  SendMessage(child, SBM_SETPOS, pos, true);

  saveCurrData();
  currPos = pos;
  loadData();
}
//------------------------------------------------------------------------------
bool PD_Adr::saveData()
{
  saveCurrData();
  setFileName();
  setDirty(false);
  P_File file(Filename, P_CREAT);
  if(file.P_open()) {
    int len = _tcslen(Filename);
    bool newVer = !_tcsicmp(Filename + len - 4, _T(".ad7"));
    if(newVer) {
      do {
        int dummy;
        int type = getNumAndSelCB(GetDlgItem(*this, IDC_COMBOBOX_DEF_DATA_TYPE), dummy);
        TCHAR t[50];
        wsprintf(t, _T("%d,%d\r\n"), ID_DEF_DATATYPE, type);
        file.P_writeString(t);
        wsprintf(t, _T("%d,%d\r\n"), ID_DEF_VER, ADR_VER);
        file.P_writeString(t);

        GET_INT(IDC_EDIT_TIMER, dummy);
        if(!dummy || (unsigned)dummy > 10000) {
          dummy = 200;
          SET_INT(IDC_EDIT_TIMER, dummy);
          }
        wsprintf(t, _T("%d,%d\r\n"), ID_TIMER_TICK, dummy);
        file.P_writeString(t);
        type = IS_CHECKED(IDC_RADIOBUTTON_WORD) ? 1 : IS_CHECKED(IDC_RADIOBUTTON_DWORD) ? 2 : 0;
        wsprintf(t, _T("%d,%d\r\n"), ID_TYPEVAR_SHOW, type);
        file.P_writeString(t);
        } while(false);

      for(int i = 0; i < MAX_ADDRESSES; ++i) {
        if(Cfg[i].db || Cfg[i].text[0] || Cfg[i].addr || Cfg[i].type) {
          TCHAR buff[500];
          if(Cfg[i].type == 0)
            Cfg[i].type = 3;
          TCHAR vMin[20];
          TCHAR vMax[20];
          TCHAR vMinP[20];
          TCHAR vMaxP[20];
          _stprintf_s (vMin, _T("%f"), Cfg[i].vMin);
          zeroTrim(vMin);

          _stprintf_s (vMax, _T("%f"), Cfg[i].vMax);
          zeroTrim(vMax);

          _stprintf_s (vMinP, _T("%f"), Cfg[i].vMinP);
          zeroTrim(vMinP);

          _stprintf_s (vMaxP, _T("%f"), Cfg[i].vMaxP);
          zeroTrim(vMaxP);

          _stprintf_s (buff, _T("%d,%d,%d,%d,%d,%u,%d,%s,%s,%s,%s,%s\r\n"), i + 1,
              Cfg[i].db, Cfg[i].addr, Cfg[i].type, Cfg[i].action, Cfg[i].ipAddr,
              Cfg[i].port, vMin, vMax, vMinP, vMaxP,
              Cfg[i].text);
          file.P_writeString(buff);
          }
        }
      }
    else {
      do {
        int dummy;
        int type = getNumAndSelCB(GetDlgItem(*this, IDC_COMBOBOX_DEF_DATA_TYPE), dummy);
        TCHAR t[50];
        wsprintf(t, _T("%d,%d\r\n"), ID_DEF_DATATYPE, type);
        file.P_writeString(t);
        } while(false);

      for(int i = 0; i < MAX_ADDRESSES; ++i) {
        if(Cfg[i].db || Cfg[i].text[0] || Cfg[i].addr || Cfg[i].type) {
          TCHAR buff[500];
          if(Cfg[i].type == 0)
            Cfg[i].type = 3;
          TCHAR vMin[20];
          TCHAR vMax[20];
          TCHAR vNorm[20] = _T("0.0");
          _stprintf_s (vMin, _T("%f"), Cfg[i].vMin);
          zeroTrim(vMin);

          _stprintf_s (vMax, _T("%f"), Cfg[i].vMax);
          zeroTrim(vMax);

          if(toBool(Cfg[i].vMax - Cfg[i].vMin) && toBool(Cfg[i].vMaxP - Cfg[i].vMinP)) {
            double vL = Cfg[i].vMax - Cfg[i].vMin;
            double vP = Cfg[i].vMaxP - Cfg[i].vMinP;

            _stprintf_s (vNorm, _T("%f"), vP / vL);
            zeroTrim(vNorm);
            }
          _stprintf_s (buff, _T("%d,%d,%d,%d,%d,%u,%s,%s,%s,%s\r\n"), i + 1,
              Cfg[i].db, Cfg[i].addr, Cfg[i].type, Cfg[i].action, Cfg[i].ipAddr,
              vMin, vMax, vNorm,
              Cfg[i].text);
          file.P_writeString(buff);
          }
        }
      }
    file.P_close();
    if(isCrypted()) {
      BYTE key[DIM_KEY_CRYPT + 2];
      wsprintfA((LPSTR)key, "p%s_%02d", "PRPH", idAdr);
      infoFileCrCopy ifcc;
      ifcc.key =       key;
      ifcc.dimKey =    DIM_KEY_CRYPT;
      ifcc.step =      CRYPT_STEP;
      ifcc.header =    (LPCBYTE)CRYPT_HEAD;
      ifcc.lenHeader = DIM_HEAD;

      TCHAR target[_MAX_PATH] = _T("#");
      _tcscpy(target + 1, Filename);
      if(cryptFileCopy(target, Filename, ifcc)) {
        DeleteFile(Filename);
        }
      }
    else {
      TCHAR path[_MAX_PATH] = _T("#");
      _tcscpy(path + 1, Filename);
      DeleteFile(path);
      }
    getFileDate(Filename, lastModified);
    loadData();
    return true;
    }
  return false;
}
//------------------------------------------------------------------------------
void PD_Adr::saveCurrData()
{
  int pos = currPos;
  for(int i = 0; i < MAX_GROUP; ++i, ++pos) {
    Cfg[pos].ipAddr = Rows[i]->getIpAddr();
    Cfg[pos].port = Rows[i]->getPort();
    Cfg[pos].db = Rows[i]->getDB();
    Cfg[pos].addr = Rows[i]->getAddr(baseAddr);
    Cfg[pos].type = Rows[i]->getType();
    Cfg[pos].action = Rows[i]->getAction();
    Cfg[pos].vMin = Rows[i]->getMin();
    Cfg[pos].vMax = Rows[i]->getMax();
    Cfg[pos].vMinP = Rows[i]->getMinP();
    Cfg[pos].vMaxP = Rows[i]->getMaxP();
    Rows[i]->getDescr(Cfg[pos].text);
    }
}
//------------------------------------------------------------------------------
void PD_Adr::loadData()
{
  int pos = currPos;

  bool oldDirty = dirty;
  onOpen = true;
  bool old = setIgnoreKillFocus(true);

  int zeroBased = 0; //IS_CHECKED(IDC_CHECKBOX_ZERO) ? 0 : 1;
  for(int i = 0; i < MAX_GROUP && pos < MAX_ADDRESSES; ++i, ++pos) {
    TCHAR buff[10];
    wsprintf(buff, _T("%d"), pos + zeroBased);
    SetDlgItemText(*this, IDC_STATICTEXT_1 + i, buff);

    Rows[i]->setAll(Cfg[pos].ipAddr, Cfg[pos].port, Cfg[pos].db, Cfg[pos].addr, Cfg[pos].type,
        Cfg[pos].action, Cfg[pos].vMin, Cfg[pos].vMax, Cfg[pos].vMinP, Cfg[pos].vMaxP, baseAddr);
    Rows[i]->setDescr(Cfg[pos].text);
    InvalidateRect(GetDlgItem(*this, IDC_STATICTEXT_1 + i), 0, 0);
    }
  onOpen = false;
  dirty = oldDirty;
  setIgnoreKillFocus(old);
}
//----------------------------------------------------------------------------
bool PD_Adr::isRowEmpty(uint pos)
{
  pos += IDC_EDIT_BYTE1_1;
  for(int i = 0; i < MAX_COL_INT; ++i, pos += DEF_OFFSET_CTRL)
    if(GetDlgItemInt(*this, pos, 0, true))
      return false;

  TCHAR buff[500];
  for(int i = MAX_COL_INT; i < MAX_COL; ++i, pos += DEF_OFFSET_CTRL) {
    GetWindowText(GetDlgItem(*this, pos), buff, SIZE_A(buff));
    if(_tstof(buff))
      return false;
    }
  return true;
}
//----------------------------------------------------------------------------
#define CHECK_ZERO_CTRL(id, base) (DEF_OFFSET_CTRL + IDC_EDIT_BYTE1_1 + base <= (id) || \
                                  (id) < IDC_EDIT_BYTE1_1 + base)
//----------------------------------------------------------------------------
bool PD_Adr::hasValidZeroValue(HWND hWndChild)
{
  long id = GetWindowLong(hWndChild, GWL_ID);
  if(CHECK_ZERO_CTRL(id, OFFSET_ADDR) && CHECK_ZERO_CTRL(id, OFFSET_VMIN_P) && CHECK_ZERO_CTRL(id, OFFSET_VMIN))
    return false;
  id += DEF_OFFSET_CTRL;
  TCHAR buff[500];
  hWndChild = GetDlgItem(*this, id);
  GetWindowText(hWndChild, buff, SIZE_A(buff));
  return toBool(_tstof(buff));
}
//----------------------------------------------------------------------------
HBRUSH PD_Adr::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  LONG id = GetWindowLong(hWndChild, GWL_ID);
  if(WM_CTLCOLORSTATIC == ctlType) {
    for(int i = 0; i < MAX_GROUP; ++i) {
      if(IDC_STATICTEXT_1 + i == id) {
        if(idFocus == i) {
          SetTextColor(hdc, ACTIVE_ROW_COLOR_TXT);
          SetBkColor(hdc, bkgColor3);
          return (Brush3);
          }
        if(isRowEmpty(i)) {
          SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
          return GetSysColorBrush(COLOR_BTNFACE);
          }
        SetTextColor(hdc, RGB(0, 0, 128));
        SetBkColor(hdc, bkgColor2);
        return (Brush2);
        }
      }
    return 0;
    }
  if(idFocus >= 0) {
    if(Rows[idFocus]->hasId(id)) {
      SetTextColor(hdc, ACTIVE_ROW_COLOR_TXT);
      SetBkColor(hdc, bkgColor3);
      return (Brush3);
      }
    }
  if(WM_CTLCOLOREDIT == ctlType) {
    SetTextColor(hdc, RGB(0, 0, 0));
    if(IDC_EDIT_MAX_ROW == id || IDC_EDIT_GOTO_ROW == id || IDC_EDIT_SEARCH_TEXT == id) {
//    if(GetDlgItem(*this, IDC_EDIT_MAX_ROW) == hWndChild) {
      SetBkColor(hdc, bkgColor2);
      return (Brush2);
      }
    TCHAR buff[500];
    GetWindowText(hWndChild, buff, SIZE_A(buff));
    if(_tstof(buff) || (*buff && !_istdigit((unsigned)*buff)) || hasValidZeroValue(hWndChild)) {
      SetTextColor(hdc, RGB(0, 0, 128));
      SetBkColor(hdc, bkgColor);
      return (Brush);
      }

    SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
    return GetSysColorBrush(COLOR_BTNFACE);
    }
  return 0;
}
//----------------------------------------------------------------------------
void PD_Adr::setBaseAddr(uint /*idCtrl*/)
{
  saveCurrData();
  if(IS_CHECKED(IDC_RADIOBUTTON_WORD))
    baseAddr = 2;
  else if(IS_CHECKED(IDC_RADIOBUTTON_DWORD))
    baseAddr = 4;
  else
    baseAddr = 1;
  loadData();
}
//----------------------------------------------------------------------------
void PD_Adr::openFile()
{
  PostMessage(*getParent(), WM_CLIENT_REQ, MAKEWPARAM(CM_NEW_PATH, 0), 0);
}
//----------------------------------------------------------------------------
void PD_Adr::setTitle()
{
}
//----------------------------------------------------------------------------
bool PD_Adr::isCrypted()
{
  return true;
//  return IS_CHECKED(IDC_CHECK_CRYPTED);
}
//----------------------------------------------------------------------------
void PD_Adr::setCrypted(bool set)
{
//  SET_CHECK_SET(IDC_CHECK_CRYPTED, set);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define MSG_CONVERT_COMMON _T("Vuoi convertire il file all'ultima versione?\r\n") \
                           _T("Se accetti il vecchio file sarà salvato con l'estensione .abk")
//----------------------------------------------------------------------------
#define MSG_CONVERT_1 _T("L'Id per il tipo di dato non è stato trovato\r\n")
//----------------------------------------------------------------------------
#define MSG_CONVERT_4 _T("Il valore di versione non è stato trovato\r\n")
//----------------------------------------------------------------------------
#define TITLE_CONVERT _T("File .ad7 non valido")
//----------------------------------------------------------------------------
#define MSG_C_1 MSG_CONVERT_1 MSG_CONVERT_COMMON
#define MSG_C_4 MSG_CONVERT_4 MSG_CONVERT_COMMON
//----------------------------------------------------------------------------
bool PD_Adr::loadFile()
{
#if 1
  TCHAR fileAddr[_MAX_PATH] = _T("#");
  _tcscpy_s(fileAddr + 1, SIZE_A(fileAddr) - 1, Filename);

  BYTE key[DIM_KEY_CRYPT + 2];
  wsprintfA((LPSTR)key, "p%s_%02d", "PRPH", idAdr);
  getFileDate(Filename, lastModified);

  setOfString Set;
  DWORD result = load_CriptedTxt(key, Set, fileAddr);
  setCrypted((LONG)result >= 0);
#else
  setOfString Set(Filename);
#endif
  if(!Set.setFirst())
    return false;
  do {
    bool updated = false;
    LPCTSTR p = Set.getString(ID_DEF_DATATYPE);
    if(!p) {
      if(IDYES != MessageBox(*this, MSG_C_1,  TITLE_CONVERT, MB_YESNO | MB_ICONSTOP))
        return false;
      if(!convertToLatest(Set, Filename))
        return true;
      p = Set.getString(ID_DEF_DATATYPE);
      updated = true;
      }
    int type = _ttoi(p);
    Set.replaceString(ID_DEF_DATATYPE, 0);
    fillCbDataType(GetDlgItem(*this, IDC_COMBOBOX_DEF_DATA_TYPE), type);
    p = Set.getString(ID_DEF_VER);
    if(!p && !updated) {
      if(IDYES != MessageBox(*this, MSG_C_4, TITLE_CONVERT, MB_YESNO | MB_ICONSTOP))
        return true;
      if(!convertToLatest(Set, Filename))
        return true;
      setDirty(true);
      }
    p = Set.getString(ID_TIMER_TICK);
    if(!p)
      p = _T("200");
    SET_INT(IDC_EDIT_TIMER, _ttoi(p));

    p = Set.getString(ID_TYPEVAR_SHOW);
    if(p) {
      SET_CHECK_SET(IDC_RADIOBUTTON_BYTE, false);
      SET_CHECK_SET(IDC_RADIOBUTTON_WORD, false);
      SET_CHECK_SET(IDC_RADIOBUTTON_DWORD, false);
      switch(_ttoi(p)) {
        case 0:
          SET_CHECK(IDC_RADIOBUTTON_BYTE);
          setBaseAddr(IDC_RADIOBUTTON_BYTE);
          break;
        case 1:
          SET_CHECK(IDC_RADIOBUTTON_WORD);
          setBaseAddr(IDC_RADIOBUTTON_WORD);
          break;
        case 2:
          SET_CHECK(IDC_RADIOBUTTON_DWORD);
          setBaseAddr(IDC_RADIOBUTTON_DWORD);
          break;
        }
      }
    } while(false);

  P_File::chgExt(Filename, _T(".ad7"));
  if(Set.setFirst()) {
    TCHAR old = Filename[0];
    Filename[0] = 0;
    Clear();
    Filename[0] = old;
    do {
      long id = Set.getCurrId();
      if(id > 0 && id <= MAX_ADDRESSES) {
        LPCTSTR p = Set.getCurrString();
        Cfg[id - 1].db = _ttol(p);

        p = findNextParam(p, 1);
        if(p) {
          Cfg[id - 1].addr = _ttol(p);
          p = findNextParam(p, 1);
          }

        if(p) {
          Cfg[id - 1].type = _ttol(p);
          p = findNextParam(p, 1);
          }

        if(p) {
          Cfg[id - 1].action = _ttol(p);
          p = findNextParam(p, 1);
          }
        if(p) {
          _stscanf(p, _T("%lu"), &Cfg[id - 1].ipAddr);
//          Cfg[id - 1].ipAddr = (DWORD)atol(p);
          p = findNextParam(p, 1);
          }
        if(p) {
          Cfg[id - 1].port = _ttol(p);
          p = findNextParam(p, 1);
          }
        if(p) {
          Cfg[id - 1].vMin = _tstof(p);
          p = findNextParam(p, 1);
          }
        if(p) {
          Cfg[id - 1].vMax = _tstof(p);
          p = findNextParam(p, 1);
          }
        if(p) {
          Cfg[id - 1].vMinP = _tstof(p);
          p = findNextParam(p, 1);
          }
        if(p) {
          Cfg[id - 1].vMaxP = _tstof(p);
          p = findNextParam(p, 1);
          }

        if(p) {
          int len = _tcslen(p);
          if(len >= MAX_TEXT)
            len = MAX_TEXT - 1;
          copyStr(Cfg[id - 1].text, p, len);
          Cfg[id - 1].text[len] = 0;
          }
        }
      } while(Set.setNext());
    setDirty(false);
    }
  setTitle();
  return true;
}
//------------------------------------------------------------------------------
void PD_Adr::checkBtn()
{
  enum statBtn {
    sAllEnabled = 1,
    sAllDisabled = 2,
    sSaveDisabled = 3,
    };
  int newStat = 0;
  if(dirty)
    newStat = sAllEnabled;
  else {
    PD_Address* pda = dynamic_cast<PD_Address*>(getParent());
    if(pda && pda->isDirty())
      newStat = sSaveDisabled;
    else
      newStat = sAllDisabled;
    }

  if(newStat != oldStat) {
    HWND hSave = GetDlgItem(*this, IDC_BUTTON_SAVE);
    HWND hSaveAs = GetDlgItem(*this, IDC_BUTTON_SAVE_AS);
    HWND hUnlock = GetDlgItem(*this, ID_UNLOCK);
    switch(newStat) {
      case sAllEnabled:
        EnableWindow(hSave, true);
        EnableWindow(hUnlock, true);
        if(sAllDisabled == oldStat)
          EnableWindow(hSaveAs, true);
        break;

      case sAllDisabled:
        EnableWindow(hSaveAs, false);
        if(sAllEnabled == oldStat) {
          EnableWindow(hSave, false);
          EnableWindow(hUnlock, false);
          }
        break;

      case sSaveDisabled:
        if(sAllEnabled == oldStat) {
          EnableWindow(hSave, false);
          EnableWindow(hUnlock, false);
          }
        else if(sAllDisabled == oldStat)
          EnableWindow(hSaveAs, true);
        break;
      }
    }
  oldStat = newStat;
}

//------------------------------------------------------------------------------
DWORD load_CriptedTxt(LPCBYTE key, setOfString& set, LPCTSTR name)
{
  infoFileCr result;
  result.header = (LPCBYTE)CRYPT_HEAD;
  result.lenHeader = DIM_HEAD;

  if(decryptFile(name, key, DIM_KEY_CRYPT, CRYPT_STEP, result)) {
#ifdef UNICODE
//  #error modificare ed adattare
#endif
  if (IsTextUnicode(result.buff, result.dim, 0))
    set.add(result.dim, (LPWSTR)result.buff, true);
  else
    set.add(result.dim, (LPSTR)result.buff, true);
  }
  else
    set.add(name + 1);
  return result.crc;
}
//-----------------------------------------------------------------------------
