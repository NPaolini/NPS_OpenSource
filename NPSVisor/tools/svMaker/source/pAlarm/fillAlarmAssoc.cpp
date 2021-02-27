//----------- fillAlarmAssoc.cpp ---------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
//----------------------------------------------------------------------------
#include "fillAlarmAssoc.h"
#include "pEdit.h"
#include "p_util.h"
#include "p_txt.h"
#include "p_file.h"
#include "pOwnBtnImageStd.h"
//----------------------------------------------------------------------------
template <typename T>
class setField
{
  public:
    setField(T cfg[]) : Cfg(cfg) {}
    virtual ~setField() {}
    virtual void setValue(DWORD ix, DWORD v) {}
    virtual void setValue(DWORD ix, LPCTSTR v) {}
    virtual void getValue(DWORD ix, LPTSTR v, size_t dim) {}
  protected:
    T* Cfg;
};
//----------------------------------------------------------------------------
#define MAKE_BASE_PTR(t, r) \
   new setField<r>(t)
//----------------------------------------------------------------------------
#define MAKE_CLSS(a, r) \
class setField_##a : public setField<r> \
{ \
  private: \
    typedef setField baseClass; \
  public: \
    setField_##a(r cfg[]) : baseClass(cfg) {} \
    virtual void setValue(DWORD ix, DWORD v) { Cfg[ix].a = v; } \
};
//----------------------------------------------------------------------------
#define MAKE_CLSS_STR(a, r) \
class setField_##a : public setField<r> \
{ \
  private: \
    typedef setField baseClass; \
  public: \
    setField_##a(r cfg[]) : baseClass(cfg) {} \
    virtual void setValue(DWORD ix, LPCTSTR v) { _tcscpy_s(Cfg[ix].a, v); } \
    virtual void getValue(DWORD ix, LPTSTR v, size_t dim) { _tcscpy_s(v, dim, Cfg[ix].a); } \
};
//----------------------------------------------------------------------------
#define MAKE_PTR(a, t) \
   new setField_##a(t)
//----------------------------------------------------------------------------
PD_FillBase::PD_FillBase(PWin* parent, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance)
{
  int idBmp[] = { IDB_BITMAP_OK, IDB_BITMAP_CANC };
  int idBtn[] = { IDOK, IDCANCEL };

  HINSTANCE hi = getHInstance();
  POwnBtn::colorRect cr(GetSysColor(COLOR_BTNFACE), RGB(0, 0, 0xff), 1, GetSysColor(COLOR_BTNTEXT));
  for(uint i = 0; i < SIZE_A(Bmp); ++i) {
    Bmp[i] = new PBitmap(idBmp[i], hi);
    POwnBtnImageStd* btn = new POwnBtnImageStd(this, idBtn[i], Bmp[i]);
    btn->setColorRect(cr);
    }
}
//----------------------------------------------------------------------------
PD_FillBase::~PD_FillBase()
{
  destroy();
  for(uint i = 0; i < SIZE_A(Bmp); ++i)
    delete Bmp[i];
}
//----------------------------------------------------------------------------
bool PD_FillBase::create()
{
  if(!baseClass::create())
    return false;
  load();
  return true;
}
//----------------------------------------------------------------------------
void PD_FillBase::CmOk()
{
  if(save())
    baseClass::CmOk();
}
//----------------------------------------------------------------------------
void PD_FillBase::enableDisable(HWND check, uint idCtrl)
{
  bool enable = BST_CHECKED == SendMessage(check, BM_GETCHECK, 0, 0);
  EnableWindow(GetDlgItem(*this, idCtrl), enable);
}
//----------------------------------------------------------------------------
extern int extent(HWND hWnd, LPCTSTR lpString);
//----------------------------------------------------------------------------
void addString(HWND hList, LPCTSTR str)
{
  int wid = extent(hList, str);
  int awi = SendMessage(hList, CB_GETDROPPEDWIDTH, 0, 0);
  if(wid > awi)
    ::SendMessage(hList, CB_SETDROPPEDWIDTH, wid, 0);
  ::SendMessage(hList, CB_ADDSTRING, 0, LPARAM(str));
}
//----------------------------------------------------------------------------
static void fillCB(HWND hcb, LPCTSTR rows[], int select)
{
  SendMessage(hcb, CB_RESETCONTENT, 0, 0);
  int i = 0;
  while(rows[i]) {
    addString(hcb, rows[i]);
    ++i;
    }
  SendMessage(hcb, CB_SETCURSEL, select, 0);
}
//----------------------------------------------------------------------------
int getNumAndSelCB(HWND hcb, int& select)
{
  TCHAR buff[50];
  GetWindowText(hcb, buff, SIZE_A(buff));
  int num = _ttoi(buff);
  if(!num)
    select = -1;
  else
    select = SendMessage(hcb, CB_GETCURSEL, 0, 0);
  return num;
}
//----------------------------------------------------------------------------
static LPCTSTR cbType[] = {
  _T("0 - null"),
  _T("1 - bit"),
  _T("2 - byte"),
  _T("3 - word"),
  _T("4 - dword"),
  _T("5 - float"),
  _T("6 - int64"),
  _T("7 - real"),
  _T("11 - char"),
  _T("12 - short"),
  _T("13 - long"),
  _T("14 - string"),
  0
};
//----------------------------------------------------------------------------
static LPCTSTR cbAssocType[] = {
  _T("- - nessuna"),
  _T("V - valore"),
  _T("N - indice numerico"),
  _T("B - indice a bit"),
  0
};
//----------------------------------------------------------------------------
#define SET_CHECK_IF(a, b, c) setCheckIf(*this, (a), (b), (c))
//----------------------------------------------------------------------------
inline void setCheckIf(HWND hwnd, int idCheck, int idEdit, bool active)
{
  SendMessage(GetDlgItem(hwnd, idCheck), BM_SETCHECK, active ? BST_CHECKED : BST_UNCHECKED, 0);
  EnableWindow(GetDlgItem(hwnd, idEdit), active);
}
//----------------------------------------------------------------------------
void fillCbDataType(HWND hcb, int num)
{
  int sel = 0;
  for(uint i = 0; i < SIZE_A(cbType) - 1; ++i)
    if(_ttoi(cbType[i]) == num) {
      sel = i;
      break;
      }
  fillCB(hcb, cbType, sel);
}
//----------------------------------------------------------------------------
void fillCbDataAssocType(HWND hcb, TCHAR car)
{
  int sel = 0;
  for(uint i = 0; i < SIZE_A(cbAssocType) - 1; ++i)
    if(cbAssocType[i][0] == car) {
      sel = i;
      break;
      }
  fillCB(hcb, cbAssocType, sel);
}
//----------------------------------------------------------------------------
PD_FillAlarm::fillRowAlarm PD_FillAlarm::fillData;
//----------------------------------------------------------------------------
PD_FillAlarm::PD_FillAlarm(baseRowAlarm cfg[], PWin* parent, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance),  Cfg(cfg)
{
  new langEdit(this, IDC_EDIT_FILL_AL_TEXT);
}
//----------------------------------------------------------------------------
#define SET_INT_AND_CHECK(edit, v, box, set) { SET_INT(edit, v); SET_CHECK_IF(box, edit, set); }
//----------------------------------------------------------------------------
#define SET_TEXT_AND_CHECK(edit, v, box, set) { SET_TEXT(edit, v); SET_CHECK_IF(box, edit, set); }
//----------------------------------------------------------------------------
#define SETINT(a, b, i) SET_INT_AND_CHECK(IDC_EDIT_FILL_##a, fillData.bra.b, IDC_CHECKBOX_FILL_##a, toBool(fillData.checked & (1 << i)))
//----------------------------------------------------------------------------
#define SETTXT(a, b, i) SET_TEXT_AND_CHECK(IDC_EDIT_FILL_##a, fillData.bra.b, IDC_CHECKBOX_FILL_##a, toBool(fillData.checked & (1 << i)))
//----------------------------------------------------------------------------
void PD_FillAlarm::load()
{
  if(fillData.addrStart <= 0)
    fillData.addrStart = 1;
  SET_INT(IDC_EDIT_FILL_AL_START, fillData.addrStart);
  SET_INT(IDC_EDIT_FILL_AL_LEN, fillData.lenData);
  SETINT(AL_GRP, grp, 0);
  SETINT(AL_FILTER, filter, 1);
  SETTXT(AL_TEXT, text, 2);
  if(fillData.checked & (1 << 16))
    SET_CHECK(IDC_RADIO_PREFIX);
  else if(fillData.checked & (1 << 17))
    SET_CHECK(IDC_RADIO_SUFFIX);
  else
    SET_CHECK(IDC_RADIO_OVERWRITE);
  enableDisableText();
}
//----------------------------------------------------------------------------
void PD_FillAlarm::enableDisableText()
{
  bool enable = IS_CHECKED(IDC_CHECKBOX_FILL_AL_TEXT);
  ENABLE(IDC_EDIT_FILL_AL_TEXT, enable);
  ENABLE(IDC_RADIO_OVERWRITE, enable);
  ENABLE(IDC_RADIO_PREFIX, enable);
  ENABLE(IDC_RADIO_SUFFIX, enable);
}
//----------------------------------------------------------------------------
#define MAKE_CLSS_AL(v) MAKE_CLSS(v, baseRowAlarm)
//----------------------------------------------------------------------------
MAKE_CLSS_AL(grp)
MAKE_CLSS_AL(filter)
MAKE_CLSS_STR(text, baseRowAlarm)
//----------------------------------------------------------------------------
#define MAKE_BASE_PTR_AL(t) MAKE_BASE_PTR(t, baseRowAlarm)
//----------------------------------------------------------------------------
struct infoAlarmNum
{
  int startNum;
  int nDigit;
  int firstAlarm;
  infoAlarmNum() : startNum(0), nDigit(0), firstAlarm(0) {}
  void parse(LPCTSTR source);
};
//----------------------------------------------------------------------------
#define CHAR_DELIM _T('$')
//----------------------------------------------------------------------------
void infoAlarmNum::parse(LPCTSTR source)
{
  while(*source) {
    if(CHAR_DELIM == *source) {
      ++source;
      break;
      }
    ++startNum;
    ++source;
    }
  if(!source)
    return;
  firstAlarm = _ttoi(source);
  while(*source) {
    if(CHAR_DELIM == *source)
      break;
    if(!_istdigit(*source)) {
      nDigit = 0;
      break;
      }
    ++nDigit;
    ++source;
    }
}
//----------------------------------------------------------------------------
static void transformText(LPTSTR target, LPCTSTR old, LPCTSTR source, uint ix, DWORD whichCopy, const infoAlarmNum& ian)
{
  TCHAR tmp[MAX_TEXT + 2] = { 0 };
  LPTSTR t = tmp;
  switch(whichCopy) {
    case 0:
    case 1:
      break;
    case 2:
      _tcscpy_s(tmp, old);
      t += _tcslen(t);
      break;
    }
  if(ian.nDigit) {
    copyStrZ(t, source, ian.startNum);
    t += _tcslen(t);
    TCHAR format[] = _T("%0xd");
    format[2] = ian.nDigit + _T('0');
    wsprintf(t, format, ian.firstAlarm + ix);
    t += _tcslen(t);
    source += ian.startNum + ian.nDigit + 2;
    _tcscpy_s(t, SIZE_A(tmp) - (t - tmp), source);
    t += _tcslen(t);
    }
  else {
    _tcscpy_s(t, SIZE_A(tmp) - (t - tmp), source);
    t += _tcslen(t);
    }
  switch(whichCopy) {
    case 0:
    case 2:
      break;
    case 1:
      _tcscpy_s(t, SIZE_A(tmp) - (t - tmp), old);
      break;
    }
  _tcscpy_s(target, MAX_TEXT, tmp);
}
//----------------------------------------------------------------------------
bool PD_FillAlarm::save()
{
  GET_INT(IDC_EDIT_FILL_AL_LEN, fillData.lenData);
  if(!fillData.lenData)
    return false;

  GET_INT(IDC_EDIT_FILL_AL_START, fillData.addrStart);
  int start = fillData.addrStart;
  if(start <= 0)
    return false;
  --start;
  if(start + fillData.lenData > MAX_ADDRESSES)
    return false;

  GET_INT(IDC_EDIT_FILL_AL_GRP, fillData.bra.grp);
  GET_INT(IDC_EDIT_FILL_AL_FILTER, fillData.bra.filter);
  GET_TEXT(IDC_EDIT_FILL_AL_TEXT, fillData.bra.text);

  uint idCheck[] = {
        IDC_CHECKBOX_FILL_AL_GRP,
        IDC_CHECKBOX_FILL_AL_FILTER,
        IDC_CHECKBOX_FILL_AL_TEXT,
        };

  fillData.checked = 0;
  for(uint i = 0; i < SIZE_A(idCheck); ++i)
    if(IS_CHECKED(idCheck[i]))
      fillData.checked |= 1 << i;

/**/
  if(!fillData.checked)
    return true;
  if(IS_CHECKED(IDC_RADIO_PREFIX))
    fillData.checked |= 1 << 16;
  else if(IS_CHECKED(IDC_RADIO_SUFFIX))
    fillData.checked |= 1 << 17;



  setField<baseRowAlarm>* clssField[SIZE_A_c(idCheck)];

  if(fillData.checked & (1 << 0))
    clssField[0] = MAKE_PTR(grp, Cfg);
  else
    clssField[0] = MAKE_BASE_PTR_AL(Cfg);

  if(fillData.checked & (1 << 1))
    clssField[1] = MAKE_PTR(filter, Cfg);
  else
    clssField[1] = MAKE_BASE_PTR_AL(Cfg);

  infoAlarmNum iAN;
  if(fillData.checked & (1 << 2)) {
    clssField[2] = MAKE_PTR(text, Cfg);
    iAN.parse(fillData.bra.text);
    }
  else
    clssField[2] = MAKE_BASE_PTR_AL(Cfg);

  int end = start + fillData.lenData;
  TCHAR text[MAX_TEXT + 2];
  TCHAR tmp[MAX_TEXT + 2];
  for(int i = start; i < end; ++i) {
    clssField[0]->setValue(i, fillData.bra.grp);
    clssField[1]->setValue(i, fillData.bra.filter);
    if(fillData.checked & (1 << 2)) {
      tmp[0] = 0;
      clssField[2]->getValue(i, tmp, MAX_TEXT);
      transformText(text, tmp, fillData.bra.text, i - start, fillData.checked >> 16, iAN);
      clssField[2]->setValue(i, text);
      }
    }

  for(uint i = 0; i < SIZE_A(clssField); ++i)
    delete clssField[i];

  return true;
}
//----------------------------------------------------------------------------
LRESULT PD_FillAlarm::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_CHECKBOX_FILL_AL_GRP:
          enableDisable((HWND) lParam, IDC_EDIT_FILL_AL_GRP);
          break;
        case IDC_CHECKBOX_FILL_AL_FILTER:
          enableDisable((HWND) lParam, IDC_EDIT_FILL_AL_FILTER);
          break;
        case IDC_CHECKBOX_FILL_AL_TEXT:
          enableDisableText();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
PD_FillAssoc::fillRowAssoc PD_FillAssoc::fillData;
//----------------------------------------------------------------------------
PD_FillAssoc::PD_FillAssoc(baseRowAssoc cfg[], PWin* parent, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance),  Cfg(cfg)
{
}
//----------------------------------------------------------------------------
void PD_FillAssoc::load()
{
  if(fillData.addrStart <= 0)
    fillData.addrStart = 1;
  SET_INT(IDC_EDIT_FILL_AS_START, fillData.addrStart);
  SET_INT(IDC_EDIT_FILL_AS_LEN, fillData.lenData);
  SETINT(AS_PRPH, prph, 0);
  SETINT(AS_ADDR, addr, 1);

  SET_INT(IDC_EDIT_FILL_AS_ADDR_STEP, fillData.addrStep);
  SET_CHECK_IF(IDC_CHECKBOX_FILL_AS_ADDR, IDC_EDIT_FILL_AS_ADDR_STEP, toBool(fillData.checked & (1 << 1)));

  fillCB(GetDlgItem(*this, IDC_COMBOBOX_FILL_AS_TYPE), cbType, fillData.bra.type);
  SET_CHECK_IF(IDC_CHECKBOX_FILL_AS_TYPE, IDC_COMBOBOX_FILL_AS_TYPE, toBool(fillData.checked & (1 << 2)));

  SETINT(AS_NBIT, nBit, 3);
  SETINT(AS_OFFS, offset, 4);

  TCHAR t = charFromDw(fillData.bra.assocType);
  fillCbDataAssocType(GetDlgItem(*this, IDC_COMBOBOX_FILL_AS_ASSOC_TYPE), t);
  SET_CHECK_IF(IDC_CHECKBOX_FILL_AS_ASSOC_TYPE, IDC_COMBOBOX_FILL_AS_ASSOC_TYPE, toBool(fillData.checked & (1 << 5)));

  SETTXT(AS_ASSOC_FILE, assocFile, 6);

  checkAll();
}
//----------------------------------------------------------------------------
#define MAKE_CLSS_ASS(v) MAKE_CLSS(v, baseRowAssoc)
//----------------------------------------------------------------------------
MAKE_CLSS_ASS(prph)
MAKE_CLSS_ASS(addr)
MAKE_CLSS_ASS(type)
MAKE_CLSS_ASS(nBit)
MAKE_CLSS_ASS(offset)
MAKE_CLSS_ASS(assocType)
MAKE_CLSS_STR(assocFile, baseRowAssoc)
//----------------------------------------------------------------------------
#define MAKE_BASE_PTR_ASSOC(t) MAKE_BASE_PTR(t, baseRowAssoc)
//----------------------------------------------------------------------------
bool PD_FillAssoc::save()
{
  GET_INT(IDC_EDIT_FILL_AS_LEN, fillData.lenData);
  if(!fillData.lenData)
    return false;

  GET_INT(IDC_EDIT_FILL_AS_START, fillData.addrStart);
  if(fillData.addrStart + fillData.lenData > MAX_ADDRESSES)
    return false;
  if(fillData.addrStart <= 0)
    return false;
  GET_INT(IDC_EDIT_FILL_AS_PRPH, fillData.bra.prph);
  GET_INT(IDC_EDIT_FILL_AS_ADDR, fillData.bra.addr);
  int v;
  fillData.bra.type = getNumAndSelCB(GetDlgItem(*this, IDC_COMBOBOX_FILL_AS_TYPE), v);
  GET_INT(IDC_EDIT_FILL_AS_NBIT, fillData.bra.nBit);
  GET_INT(IDC_EDIT_FILL_AS_OFFS, fillData.bra.offset);
  do {
    TCHAR b[64];
    GET_TEXT(IDC_COMBOBOX_FILL_AS_ASSOC_TYPE, b);
    fillData.bra.assocType = dwFromChar(*b);
    } while(false);

  GET_TEXT(IDC_EDIT_FILL_AS_ASSOC_FILE, fillData.bra.assocFile);

  uint idCheck[] = {
        IDC_CHECKBOX_FILL_AS_PRPH,
        IDC_CHECKBOX_FILL_AS_ADDR,
        IDC_CHECKBOX_FILL_AS_TYPE,
        IDC_CHECKBOX_FILL_AS_NBIT,
        IDC_CHECKBOX_FILL_AS_OFFS,
        IDC_CHECKBOX_FILL_AS_ASSOC_TYPE,
        IDC_CHECKBOX_FILL_AS_ASSOC_FILE,
        };

  fillData.checked = 0;

  for(uint i = 0; i < SIZE_A(idCheck); ++i)
    if(IS_CHECKED(idCheck[i]))
      fillData.checked |= 1 << i;

  if(!fillData.checked)
    return true;

  setField<baseRowAssoc>* clssField[SIZE_A_c(idCheck)];

  if(fillData.checked & (1 << 0))
    clssField[0] = MAKE_PTR(prph, Cfg);
  else
    clssField[0] = MAKE_BASE_PTR_ASSOC(Cfg);

  if(fillData.checked & (1 << 1))
    clssField[1] = MAKE_PTR(addr, Cfg);
  else
    clssField[1] = MAKE_BASE_PTR_ASSOC(Cfg);

  if(fillData.checked & (1 << 2))
    clssField[2] = MAKE_PTR(type, Cfg);
  else
    clssField[2] = MAKE_BASE_PTR_ASSOC(Cfg);

  if(fillData.checked & (1 << 3))
    clssField[3] = MAKE_PTR(nBit, Cfg);
  else
    clssField[3] = MAKE_BASE_PTR_ASSOC(Cfg);

  if(fillData.checked & (1 << 4))
    clssField[4] = MAKE_PTR(offset, Cfg);
  else
    clssField[4] = MAKE_BASE_PTR_ASSOC(Cfg);

  if(fillData.checked & (1 << 5))
    clssField[5] = MAKE_PTR(assocType, Cfg);
  else
    clssField[5] = MAKE_BASE_PTR_ASSOC(Cfg);
  if(fillData.checked & (1 << 6))
    clssField[6] = MAKE_PTR(assocFile, Cfg);
  else
    clssField[6] = MAKE_BASE_PTR_ASSOC(Cfg);

  int end = fillData.addrStart + fillData.lenData - 1;

  GET_INT(IDC_EDIT_FILL_AS_ADDR_STEP, fillData.addrStep);

  int addAddr = 0;
  for(int i = fillData.addrStart - 1; i < end; ++i, addAddr += fillData.addrStep) {
    clssField[0]->setValue(i, fillData.bra.prph);
    clssField[1]->setValue(i, fillData.bra.addr + addAddr);
    clssField[2]->setValue(i, fillData.bra.type);
    clssField[3]->setValue(i, fillData.bra.nBit);
    clssField[4]->setValue(i, fillData.bra.offset);
    clssField[5]->setValue(i, fillData.bra.assocType);
    clssField[6]->setValue(i, fillData.bra.assocFile);
    }

  for(uint i = 0; i < SIZE_A(clssField); ++i)
    delete clssField[i];

  return true;
}
//----------------------------------------------------------------------------
LRESULT PD_FillAssoc::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_CHECKBOX_FILL_AS_PRPH:
          enableDisable((HWND) lParam, IDC_EDIT_FILL_AS_PRPH);
          break;
        case IDC_CHECKBOX_FILL_AS_ADDR:
          enableDisable((HWND) lParam, IDC_EDIT_FILL_AS_ADDR);
          enableDisable((HWND) lParam, IDC_EDIT_FILL_AS_ADDR_STEP);
          break;
        case IDC_CHECKBOX_FILL_AS_TYPE:
          enableDisable((HWND) lParam, IDC_COMBOBOX_FILL_AS_TYPE);
          break;
        case IDC_CHECKBOX_FILL_AS_NBIT:
          enableDisable((HWND) lParam, IDC_EDIT_FILL_AS_NBIT);
          break;
        case IDC_CHECKBOX_FILL_AS_OFFS:
          enableDisable((HWND) lParam, IDC_EDIT_FILL_AS_OFFS);
          break;
        case IDC_CHECKBOX_FILL_AS_ASSOC_TYPE:
          enableDisable((HWND) lParam, IDC_COMBOBOX_FILL_AS_ASSOC_TYPE);
          break;
        case IDC_CHECKBOX_FILL_AS_ASSOC_FILE:
          enableDisable((HWND) lParam, IDC_EDIT_FILL_AS_ASSOC_FILE);
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void PD_FillAssoc::checkAll()
{
  UINT idcs[] = {
      IDC_CHECKBOX_FILL_AS_PRPH, IDC_CHECKBOX_FILL_AS_ADDR,
      IDC_CHECKBOX_FILL_AS_TYPE, IDC_CHECKBOX_FILL_AS_NBIT,
      IDC_CHECKBOX_FILL_AS_OFFS, IDC_CHECKBOX_FILL_AS_ASSOC_TYPE,
      IDC_CHECKBOX_FILL_AS_ASSOC_FILE
      };
  for(uint i = 0; i < SIZE_A(idcs); ++i)
    PostMessage(*this, WM_COMMAND, MAKEWPARAM(idcs, 0), 0);
}


//----------------------------------------------------------------------------
PD_FillAssocFile::fillRowAssocFile PD_FillAssocFile::fillData;
//----------------------------------------------------------------------------
PD_FillAssocFile::PD_FillAssocFile(baseRowAssocFile cfg[], PWin* parent, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance),  Cfg(cfg)
{
  new langEdit(this, IDC_EDIT_FILL_AL_TEXT);
}
//----------------------------------------------------------------------------
void PD_FillAssocFile::load()
{
  if(fillData.addrStart <= 0)
    fillData.addrStart = 1;
  SET_INT(IDC_EDIT_FILL_AL_START, fillData.addrStart);
  SET_INT(IDC_EDIT_FILL_AL_LEN, fillData.lenData);
  SET_INT(IDC_EDIT_FILL_AS_VAL_STEP, fillData.valueStep);
  SETINT(AL_AS_VAL, value, 0);
  SETTXT(AL_TEXT, assocText, 1);
}
//----------------------------------------------------------------------------
#define MAKE_CLSS_AS_FILE(v) MAKE_CLSS(v, baseRowAssocFile)
//----------------------------------------------------------------------------
MAKE_CLSS_AS_FILE(value)
MAKE_CLSS_STR(assocText, baseRowAssocFile)
//----------------------------------------------------------------------------
#define MAKE_BASE_PTR_AS_FILE(t) MAKE_BASE_PTR(t, baseRowAssocFile)
//----------------------------------------------------------------------------
bool PD_FillAssocFile::save()
{
  GET_INT(IDC_EDIT_FILL_AL_LEN, fillData.lenData);
  if(!fillData.lenData)
    return false;

  GET_INT(IDC_EDIT_FILL_AL_START, fillData.addrStart);
  int start = fillData.addrStart;
  if(start <= 0)
    return false;
  --start;
  if(start + fillData.lenData > MAX_ADDRESSES)
    return false;

  GET_INT(IDC_EDIT_FILL_AL_AS_VAL, fillData.bra.value);
  GET_TEXT(IDC_EDIT_FILL_AL_TEXT, fillData.bra.assocText);

  uint idCheck[] = {
        IDC_CHECKBOX_FILL_AL_AS_VAL,
        IDC_CHECKBOX_FILL_AL_TEXT,
        };

  fillData.checked = 0;
  for(uint i = 0; i < SIZE_A(idCheck); ++i)
    if(IS_CHECKED(idCheck[i]))
      fillData.checked |= 1 << i;

/**/
  if(!fillData.checked)
    return true;

  setField<baseRowAssocFile>* clssField[SIZE_A_c(idCheck)];

  if(fillData.checked & (1 << 0))
    clssField[0] = MAKE_PTR(value, Cfg);
  else
    clssField[0] = MAKE_BASE_PTR_AS_FILE(Cfg);

  if(fillData.checked & (1 << 1))
    clssField[1] = MAKE_PTR(assocText, Cfg);
  else
    clssField[1] = MAKE_BASE_PTR_AS_FILE(Cfg);

  int end = start + fillData.lenData;

  GET_INT(IDC_EDIT_FILL_AS_VAL_STEP, fillData.valueStep);

  int addValue = 0;
  for(int i = start; i < end; ++i, addValue += fillData.valueStep) {
    clssField[0]->setValue(i, fillData.bra.value + addValue);
    clssField[1]->setValue(i, fillData.bra.assocText);
    }

  for(uint i = 0; i < SIZE_A(clssField); ++i)
    delete clssField[i];

  return true;
}
//----------------------------------------------------------------------------
LRESULT PD_FillAssocFile::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_CHECKBOX_FILL_AL_AS_VAL:
          enableDisable((HWND) lParam, IDC_EDIT_FILL_AL_AS_VAL);
          enableDisable((HWND) lParam, IDC_EDIT_FILL_AS_VAL_STEP);
          break;
        case IDC_CHECKBOX_FILL_AL_TEXT:
          enableDisable((HWND) lParam, IDC_EDIT_FILL_AL_TEXT);
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
