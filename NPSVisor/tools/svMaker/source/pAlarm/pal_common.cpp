//----------- common.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "common.h"
#include "pAl_common.h"
#include "baseDlg.h"
#include "svmDataProject.h"
#include "show_edit.h"
#include <stdio.h>
#include <stdlib.h>
//-----------------------------------------------------------------------------
#define WM_MOUSEWHEEL                   0x020A
//----------------------------------------------------------------------------
LRESULT PEditEmpty::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_KILLFOCUS:
      invalidateNum();
      break;
    case WM_MOUSEWHEEL:
      do {
        PD_Base* pdb = getParentWin<PD_Base>(this);
        if(pdb)
          PostMessage(*pdb, message, wParam, lParam);
        } while(false);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void PEditEmpty::invalidateNum() const
{
  if(IS_THIS_CHANGED()) {
    uint ix = Attr.id  - IDC_EDIT_FIRST_FIELD;
    ix %= DEF_OFFSET_CTRL;
    HWND hw = GetDlgItem(*getParent(), ix + IDC_STATICTEXT_1);
    if(hw)
      InvalidateRect(hw, 0, 0);
    InvalidateRect(*this, 0, 1);
    if(alsoSibling) {
      hw = GetDlgItem(*getParent(), Attr.id + alsoSibling);
      if(hw)
        InvalidateRect(hw, 0, 1);
      }
    }
}
//----------------------------------------------------------------------------
#define MAX_DIST_CTRL 4
//----------------------------------------------------------------------------
static HWND isMyEditCtrl(HWND hwnd)
{
  PWin* w = PWin::getWindowPtr(hwnd);
  return dynamic_cast<PageEdit_A*>(w) ? hwnd : 0;
}
//----------------------------------------------------------------------------
bool PageEdit_A::evKeyDown(WPARAM& key)
{
  switch(key) {
    case VK_NEXT:
      PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_PAGERIGHT, 0);
      return true;
    case VK_PRIOR:
      PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_PAGELEFT, 0);
      return true;
    case VK_UP:
      do {
        uint ix = Attr.id  - 1;
        HWND hw = isMyEditCtrl(GetDlgItem(*getParent(), ix));
        if(hw)
          SetFocus(hw);
        } while(false);
      return true;
    case VK_DOWN:
    case VK_RETURN:
      do {
        uint ix = Attr.id  + 1;
        HWND hw = isMyEditCtrl(GetDlgItem(*getParent(), ix));
        if(hw)
          SetFocus(hw);
        } while(false);
      return true;
    case VK_LEFT:
      do {
        LRESULT sel = SendMessage(*this, EM_GETSEL, 0, 0);
        if(-1 == sel)
          break;
        int init = LOWORD(sel);
        int end  = HIWORD(sel);
        if(!init && !end) {
          uint ix = Attr.id  - DEF_OFFSET_CTRL;
          for(uint i = 0; i < MAX_DIST_CTRL; ++i) {
            HWND hw = isMyEditCtrl(GetDlgItem(*getParent(), ix));
            if(hw) {
              SetFocus(hw);
              return true;
              }
            ix -= DEF_OFFSET_CTRL;
            }
          }
        } while(false);
      break;
    case VK_RIGHT:
      do {
        LRESULT sel = SendMessage(*this, EM_GETSEL, 0, 0);
        if(-1 == sel)
          break;
        int init = LOWORD(sel);
        int end  = HIWORD(sel);
        sel = SendMessage(*this, WM_GETTEXTLENGTH, 0, 0);

        if(init == end && init == sel) {
          uint ix = Attr.id  + DEF_OFFSET_CTRL;
          for(uint i = 0; i < MAX_DIST_CTRL; ++i) {
            HWND hw = isMyEditCtrl(GetDlgItem(*getParent(), ix));
            if(hw) {
              SetFocus(hw);
              return true;
              }
            ix += DEF_OFFSET_CTRL;
            }
          }
        } while(false);
      break;
    }
  return baseClass::evKeyDown(key);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
LRESULT clickEdit::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_RBUTTONUP:
      wParam |= MK_CONTROL;
    case WM_LBUTTONUP:
      if(MK_CONTROL & wParam) {
        TCHAR buff[64];
        TCHAR old[64];
        GetWindowText(*this, buff, SIZE_A(buff));
        _tcscpy_s(old, buff);
        SendMessage(*this, EM_SETSEL, 0, (LPARAM) -1);
        if(performChoose(buff) && _tcsicmp(buff, old)) {
          SetWindowText(*this, buff);
          SendMessage(*this, EM_SETMODIFY, TRUE, 0);
          PostMessage(*getParent(), WM_COMMAND, MAKEWPARAM(Attr.id, MY_EN_CHANGE), (LPARAM)getHandle());
          invalidateNum();
          }
        SendMessage(*this, EM_SETSEL, -1, (LPARAM) -1);
        if(WM_RBUTTONUP == message)
          return 1;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
bool clickEditType::performChoose(LPTSTR buff)
{
  return IDOK == ShowType(this, buff).modal();
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool clickEditAssoc::performChoose(LPTSTR buff)
{
  return IDOK == ShowAssocType(this, buff).modal();
}
//----------------------------------------------------------------------------
bool firstEdit_A::evKeyDown(WPARAM& key)
{
  switch(key) {
    case VK_UP:
      PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_LINELEFT, 0);
      return true;
    }
  return PageEdit_A::evKeyDown(key);
}
//----------------------------------------------------------------------------
bool lastEdit_A::evKeyDown(WPARAM& key)
{
  switch(key) {
    case VK_DOWN:
    case VK_RETURN:
    PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_LINERIGHT, 0);
    return true;
    }
  return PageEdit_A::evKeyDown(key);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool clickEditTypeFirst::evKeyDown(WPARAM& key)
{
  switch(key) {
    case VK_UP:
      PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_LINELEFT, 0);
      return true;
    }
  return baseClass::evKeyDown(key);
}
//----------------------------------------------------------------------------
bool clickEditAssocFirst::evKeyDown(WPARAM& key)
{
  switch(key) {
    case VK_UP:
      PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_LINELEFT, 0);
      return true;
    }
  return baseClass::evKeyDown(key);
}
//----------------------------------------------------------------------------
bool clickEditTypeLast::evKeyDown(WPARAM& key)
{
  switch(key) {
    case VK_DOWN:
    case VK_RETURN:
      PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_LINERIGHT, 0);
      return true;
    }
  return baseClass::evKeyDown(key);
}
//----------------------------------------------------------------------------
bool clickEditAssocLast::evKeyDown(WPARAM& key)
{
  switch(key) {
    case VK_DOWN:
    case VK_RETURN:
      PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_LINERIGHT, 0);
      return true;
    }
  return baseClass::evKeyDown(key);
}
//----------------------------------------------------------------------------
PEditEmpty* makeFactory(PWin* parent, uint id, const PRect& r, int len, int whichRow, uint id2 = FORCE_NO_ALL)
{
  switch(whichRow) {
    case -1:
      return new firstEdit_A(parent, id, r, len, id2);
    case 1:
      return new lastEdit_A(parent, id, r, len, id2);
    default:
      return new PageEdit_A(parent, id, r, len, id2);
    }
}
//----------------------------------------------------------------------------
PEditEmpty* makeFactoryType(PWin* parent, uint id, const PRect& r, int len, int whichRow, uint id2 = FORCE_NO_ALL)
{
  switch(whichRow) {
    case -1:
      return new clickEditTypeFirst(parent, id, r, len, id2);
    case 1:
      return new clickEditTypeLast(parent, id, r, len, id2);
    default:
      return new clickEditType(parent, id, r, len, id2);
    }
}
//----------------------------------------------------------------------------
PEditEmpty* makeFactoryAssoc(PWin* parent, uint id, const PRect& r, int len, int whichRow, uint id2 = FORCE_NO_ALL)
{
  switch(whichRow) {
    case -1:
      return new clickEditAssocFirst(parent, id, r, len, id2);
    case 1:
      return new clickEditAssocLast(parent, id, r, len, id2);
    default:
      return new clickEditAssoc(parent, id, r, len, id2);
    }
}
//----------------------------------------------------------------------------
bool PRow_Base::isDirty(PWin** ctrl, uint nElem, bool reset)
{
  if(reset) {
    bool dirty = false;
    for(uint i = 0; i < nElem; ++i) {
      if(SendMessage(*ctrl[i], EM_GETMODIFY, 0, 0)) {
        dirty = true;
        SendMessage(*ctrl[i], EM_SETMODIFY, 0, 0);
        }
      }
    return dirty;
    }
  else {
    for(uint i = 0; i < nElem; ++i) {
      if(SendMessage(*ctrl[i], EM_GETMODIFY, 0, 0))
        return true;
      }
    }
  return false;
}
//----------------------------------------------------------------------------
void PRow_Base::resetDirty(PWin** ctrl, uint nElem)
{
  for(uint i = 0; i < nElem; ++i)
    SendMessage(*ctrl[i], EM_SETMODIFY, 0, 0);
}
//----------------------------------------------------------------------------
void PRow_Base::offset(PWin** ctrlSet, const PVect<infoPosCtrl>& ipc)
{
  uint nElem = ipc.getElem();
  for(uint i = 0; i < nElem; ++i) {
    POINT pt = { ipc[i].x, ipc[i].y };
    MapWindowPoints(HWND_DESKTOP, *(ctrlSet[i]->getParent()), &pt, 1);
    SetWindowPos(*ctrlSet[i], 0, pt.x, pt.y, ipc[i].w, ipc[i].h, SWP_NOZORDER);
    }
}
//----------------------------------------------------------------------------
DWORD PRow_Base::getGen(PWin* ctrl)
{
  TCHAR buff[100];
  GetWindowText(*ctrl, buff, SIZE_A(buff));
  return _ttoi(buff);
}
//----------------------------------------------------------------------------
void PRow_Base::setGen(PWin* ctrl, DWORD val)
{
  TCHAR buff[100];
  wsprintf(buff, _T("%d"), val);
  SetWindowText(*ctrl, buff);
}
//----------------------------------------------------------------------------
#define MAX_EDIT_DDWORD  20
//----------------------------------------------------------------------------
PRow_Alarm::PRow_Alarm(PWin* parent, uint first_id, POINT pt, int whichRow) :
    firstId(first_id)
{
  PRect r(0, 0, W_GROUP, H_EDIT);
  r.MoveTo(pt.x, pt.y);
  PRect r2(r);
  r2.Inflate(0, -1);
  Group = makeFactory(parent, firstId + OFFSET_GROUP, r, MAX_EDIT_DDWORD, whichRow);

  r.Offset(r.Width() + 1, 0);
  r.right = r.left + W_FILTER;
  r2 = r;
  r2.Inflate(0, -1);
  Filter = makeFactory(parent, firstId + OFFSET_FILTER, r, MAX_EDIT_DDWORD, whichRow);

  r.Offset(r.Width() + 1, 0);
  r.right = r.left + W_ASSOCR;
  r2 = r;
  r2.Inflate(0, -1);
  Assoc = new PStatic(parent, firstId + OFFSET_ASSOC, r);
  Assoc->setFont(D_FONT(16, 0, 0, _T("arial")), true);

  r.Offset(r.Width() + 1, 0);
  r.right = r.left + W_DESCR;
  r2 = r;
  r2.Inflate(0, -1);
  Descr = makeFactory(parent, firstId + OFFSET_DESCR, r, 0, whichRow, FORCE_OFF);
  Descr->Attr.style &= ~ES_CENTER;
  Descr->Attr.style |= ES_LEFT;

}
//----------------------------------------------------------------------------
bool PRow_Alarm::isDirty(bool reset)
{
  PWin* ctrl[] = {  Group, Filter, Descr  };
  return baseClass::isDirty(ctrl, SIZE_A(ctrl), reset);
}
//----------------------------------------------------------------------------
bool PRow_Assoc::isDirty(bool reset)
{
  PWin* ctrl[] = { Addr, Type, nBit, Offs, Assoc, TextAssoc };
  return dirty | baseClass::isDirty(ctrl, SIZE_A(ctrl), reset);
}
//----------------------------------------------------------------------------
void PRow_Alarm::resetDirty()
{
  PWin* ctrl[] = {  Group, Filter, Descr  };
  baseClass::resetDirty(ctrl, SIZE_A(ctrl));
}
//----------------------------------------------------------------------------
void PRow_Assoc::resetDirty()
{
  PWin* ctrl[] = { Addr, Type, nBit, Offs, Assoc, TextAssoc };
  baseClass::resetDirty(ctrl, SIZE_A(ctrl));
  dirty = false;
}
//----------------------------------------------------------------------------
void PRow_Alarm::offset(const PVect<infoPosCtrl>& ipc)
{
  PWin* ctrl[] = {  Group, Filter, Assoc, Descr  };
  baseClass::offset(ctrl, ipc);
}
//----------------------------------------------------------------------------
TCHAR charFromDw(DWORD assocType)
{
  TCHAR t = (TCHAR)assocType;
  switch(t) {
    case _T('N'):
    case _T('B'):
    case _T('V'):
    case _T('n'):
    case _T('b'):
    case _T('v'):
      return t;
    }
  return 0;
}
//----------------------------------------------------------------------------
PRow_Assoc::PRow_Assoc(PWin* parent, uint first_id, POINT pt, int whichRow) :
    firstId(first_id)
{
  PRect r(0, 0, W_PRPH, H_EDIT);
  r.MoveTo(pt.x, pt.y);

  Prph = makeFactory(parent, firstId, r, MAX_EDIT_DDWORD, whichRow);

  r.Offset(r.Width() + 1, 0);
  r.right = r.left + W_ADDR;
  Addr = makeFactory(parent, firstId + OFFSET_ADDR, r, MAX_EDIT_DDWORD, whichRow);

  r.Offset(r.Width(), 0);
  r.right = r.left + W_TYPE;
  Type = makeFactoryType(parent, firstId + OFFSET_TYPE, r, MAX_EDIT_DDWORD, whichRow);

  r.Offset(r.Width() + 1, 0);
  r.right = r.left + W_NBIT;
  nBit = makeFactory(parent, firstId + OFFSET_NBIT, r, MAX_EDIT_DDWORD, whichRow);

  r.Offset(r.Width() + 1, 0);
  r.right = r.left + W_OFFS;
  Offs = makeFactory(parent, firstId + OFFSET_OFFS, r, MAX_EDIT_DDWORD, whichRow);

  r.Offset(r.Width() + 1, 0);
  r.right = r.left + W_BTN_ASSOC;
  Assoc = makeFactoryAssoc(parent, firstId + OFFSET_BTN_ASSOC, r, MAX_EDIT_DDWORD, whichRow);

  r.Offset(r.Width() + 1, 0);
  r.right = r.left + W_TEXT_ASSOC;
  TextAssoc = makeFactory(parent, firstId + OFFSET_TEXT_ASSOC, r, MAX_DIM_ASSOC_FILE, whichRow);
  TextAssoc->Attr.style &= ~ES_CENTER;
  TextAssoc->Attr.style |= ES_LEFT;

  r.Offset(r.Width() + 1, 0);
  r.right = r.left + W_RADIO;

  Radio = new PRadioButton(parent, firstId + OFFSET_RADIO, r);
}
//----------------------------------------------------------------------------
void PRow_Assoc::offset(const PVect<infoPosCtrl>& ipc)
{
  PWin* ctrl[] = { Prph, Addr, Type, nBit, Offs, Assoc, TextAssoc, Radio };
  baseClass::offset(ctrl, ipc);
}
//----------------------------------------------------------------------------
DWORD PRow_Assoc::getAssoc()
{
  TCHAR buff[4];
  GetWindowText(*Assoc, buff, SIZE_A(buff));
  buff[1] = 0;
  DWORD v = *(LPDWORD)buff;
  return v;
}
//----------------------------------------------------------------------------
void PRow_Assoc::setAssoc(DWORD assoc)
{
  TCHAR buff[4];
  *(LPDWORD)buff = assoc;
  SetWindowText(*Assoc, buff);
}
//----------------------------------------------------------------------------
void PRow_Assoc::getFile(LPTSTR file)
{
  GetWindowText(*TextAssoc, file, MAX_DIM_ASSOC_FILE);
}
//----------------------------------------------------------------------------
void PRow_Assoc::setFile(LPCTSTR file)
{
  SetWindowText(*TextAssoc, file);
}
//----------------------------------------------------------------------------
void PRow_Assoc::setSelected()
{
  SendMessage(*Radio, BM_SETCHECK, BST_CHECKED, 0);
}
//----------------------------------------------------------------------------
bool PRow_Assoc::isSelected()
{
  return toBool(SendMessage(*Radio, BM_GETCHECK, 0, 0));
}
//----------------------------------------------------------------------------
void PRow_Assoc::getAll(DWORD& prph, DWORD& addr, DWORD& type, DWORD& nbit, DWORD& offs, DWORD& assoc, LPTSTR file)
{
  prph = getPrph();
  addr = getAddr();
  type = getType();
  nbit = getNBit();
  offs = getOffs();
  assoc = getAssoc();
  getFile(file);
}
//----------------------------------------------------------------------------
void PRow_Assoc::setAll(DWORD prph, DWORD addr, DWORD type, DWORD nbit, DWORD offs, DWORD assoc, LPCTSTR file)
{
  setPrph(prph);
  setAddr(addr);
  setType(type);
  setNBit(nbit);
  setOffs(offs);
  setAssoc(assoc);
  setFile(file);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
PRow_Assoc_File::PRow_Assoc_File(PWin* parent, uint first_id, POINT pt, int whichRow) :
    firstId(first_id)
{
  PRect r(0, 0, W_GROUP, H_EDIT);
  r.MoveTo(pt.x, pt.y);
  PRect r2(r);
  r2.Inflate(0, -1);
  Value = makeFactory(parent, firstId + OFFSET_GROUP, r, MAX_EDIT_DDWORD, whichRow);

  r.Offset(r.Width() + 1, 0);
  r.right = r.left + W_DESCR;
  r2 = r;
  r2.Inflate(0, -1);
  Descr = makeFactory(parent, firstId + OFFSET_DESCR, r, 0, whichRow, FORCE_OFF);
  Descr->Attr.style &= ~ES_CENTER;
  Descr->Attr.style |= ES_LEFT;
}
//----------------------------------------------------------------------------
bool PRow_Assoc_File::isDirty(bool reset)
{
  PWin* ctrl[] = {  Value, Descr  };
  return baseClass::isDirty(ctrl, SIZE_A(ctrl), reset);
}
//----------------------------------------------------------------------------
void PRow_Assoc_File::resetDirty()
{
  PWin* ctrl[] = {  Value, Descr  };
  baseClass::resetDirty(ctrl, SIZE_A(ctrl));
}
//----------------------------------------------------------------------------
void PRow_Assoc_File::offset(const PVect<infoPosCtrl>& ipc)
{
  PWin* ctrl[] = {  Value, Descr  };
  baseClass::offset(ctrl, ipc);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
ClearChoose::ClearChoose(PWin* parent, int& from, int& to) :
      baseClass(parent, IDD_DIALOG_CLEAR), From(from), To(to)
{
  int idBmp[] = { IDB_BITMAP_OK, IDB_BITMAP_CANC };
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
//----------------------------------------------------------------------------
ClearChoose::~ClearChoose()
{
  destroy();
  for(uint i = 0; i < SIZE_A(Bmp); ++i)
    delete Bmp[i];
}
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
void makeSystemSet(setOfString& set, LPCTSTR filename)
{
  TCHAR path[_MAX_PATH];
  makeSystemFilename(path, filename);
  manageCryptPage mcp;
  bool crypted = mcp.isCrypted(path);
  if(crypted)
    mcp.makePathAndDecrypt(path);
  set.add(path);
  if(crypted)
    mcp.releaseFileDecrypt(path);
}
//----------------------------------------------------------------------------
void makeSystemFilename(LPTSTR target, LPCTSTR filename)
{
  dataProject& dp = getDataProject();
  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, dp.newPath);
  appendPath(path, dp.systemPath);
  appendPath(path, filename);
  _tcscpy_s(target, _MAX_PATH, path);
}
//----------------------------------------------------------------------------
