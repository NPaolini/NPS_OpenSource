//----------- svmMoveInfo.cpp ----------------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmMoveInfo.h"
#include "svmObject.h"
#include "svmNormal.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
objMoveInfo::objMoveInfo(const objMoveInfo& other) : sz(other.sz), dir(other.dir), absoluteSize(other.absoluteSize)
{
  for(uint i = 0; i < SIZE_A(vI); ++i)
    vI[i] = other.vI[i];
}
//-----------------------------------------------------------
const objMoveInfo& objMoveInfo::operator =(const objMoveInfo& other)
{
  if(&other != this) {
    for(uint i = 0; i < SIZE_A(vI); ++i)
      vI[i] = other.vI[i];
    dir = other.dir;
    sz = other.sz;
    absoluteSize = other.absoluteSize;
    }
  return *this;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
wrapMoveInfo::wrapMoveInfo(const wrapMoveInfo& other) : Obj(0), enabled(other.enabled)
{
  if(enabled) {
    if(other.Obj)
      Obj = new objMoveInfo(*other.Obj);
    else
      enabled = false;
    }
}
//-----------------------------------------------------------
const wrapMoveInfo& wrapMoveInfo::operator =(const wrapMoveInfo& other)
{
  if(&other != this) {
    delete Obj;
    if(!other.Obj) {
      enabled = false;
      Obj = 0;
      }
    else {
      enabled = other.enabled;
      Obj = new objMoveInfo(*other.Obj);
      }
    }
  return *this;
}
//-----------------------------------------------------------
const wrapMoveInfo& wrapMoveInfo::operator =(const objMoveInfo& other)
{
  enabled = true;
  delete Obj;
  Obj = new objMoveInfo(other);
  return *this;
}
//-----------------------------------------------------------
//#define SAVE_CONST_AS_REAL
//-----------------------------------------------------------
bool wrapMoveInfo::saveVarMoveInfo(P_File& pf, uint baseId, const objMoveInfo::varInfo& vI)
{
  baseId = manageObjId(baseId).getFirstExtendId();
  TCHAR buff[100];
#ifdef SAVE_CONST_AS_REAL
  if(PRPH_4_CONST == vI.perif && 5 == vI.typeVal) {
    float v = *(float*)&vI.addr;
    _stprintf_s(buff, _T("%d,%d,%f,0,0\r\n"), baseId, vI.perif, v);
    }
  else
#endif
    wsprintf(buff, _T("%d,%d,%u,%d,%d\r\n"), baseId, vI.perif, vI.addr, vI.typeVal, vI.normaliz);
  return writeStringChkUnicode(pf, buff) > 0;
}
//-----------------------------------------------------------
int wrapMoveInfo::save(P_File& pf, const POINT& pt)
{
  if(!enabled || !Obj)
    return 0;

  svmObjCount& objCount = getObjCount();
  uint id = objCount.getGlobalIdCount();
  TCHAR buff[1000];
  wsprintf(buff, _T("%d,%d,%d,%d,%d"),
        id, REV__X(pt.x), REV__Y(pt.y), REV__X(pt.x + Obj->sz.cx), REV__Y(pt.y + Obj->sz.cy));
  TCHAR t[50];
  for(int i = 0; i < objMoveInfo::MAX_MOVE_INFO; ++i) {
    int code = objCount.getIdCount(oEDIT);
    manageObjId moi2(0, ID_INIT_VAR_EDI);
    code = moi2.calcBaseExtendId(code);
    wsprintf(t, _T(",%d"), code);
    _tcscat_s(buff, t);
    if(!saveVarMoveInfo(pf, code, Obj->vI[i]))
      return 0;
    }
  wsprintf(t, _T(",%d,%d\r\n"), Obj->dir, Obj->absoluteSize);
  _tcscat_s(buff, t);
  return writeStringChkUnicode(pf, buff) > 0 ? id : 0;
}
//-----------------------------------------------------------
void wrapMoveInfo::loadVarMoveInfo(uint id, setOfString& set, objMoveInfo::varInfo& vI)
{
  int perif = 0;
  int addr = 0;
  int typeVal = 0;
  int normaliz = 0;

  id = manageObjId(id).getFirstExtendId();
  LPCTSTR p = set.getString(id);
  if(p)
    _stscanf_s(p, _T("%d,%u,%d,%d"), &perif, &addr, &typeVal, &normaliz);
#ifdef SAVE_CONST_AS_REAL
  if(PRPH_4_CONST == perif && 5 == typeVal) {
    p = findNextParamTrim(p);
    float v = (float)_tstof(p);
    addr = *(LPDWORD)&v;
    }
#endif
  vI.perif = perif;
  vI.addr = addr;
  vI.typeVal = typeVal;
  vI.normaliz = normaliz;
}
//-----------------------------------------------------------
bool wrapMoveInfo::load(setOfString& set, uint id)
{
  if(!id)
    return false;
  LPCTSTR p = set.getString(id);
  if(!p)
    return false;
  enabled = true;
  delete Obj;
  Obj = new objMoveInfo;
  int x1 = 0;
  int y1 = 0;
  int x2 = 0;
  int y2 = 0;
  _stscanf_s(p, _T("%d,%d,%d,%d"), &x1, &y1, &x2, &y2);
  // uno in meno, verrà prelevato all'inizio del ciclo for
  p = findNextParam(p, 4 - 1);

  Obj->sz.cx = R__X(x2 - x1);
  Obj->sz.cy = R__Y(y2 - y1);
  for(int i = 0; i < objMoveInfo::MAX_MOVE_INFO; ++i) {
    p = findNextParam(p, 1);
    if(!p)
      break;
    loadVarMoveInfo(_ttoi(p), set, Obj->vI[i]);
    }
  p = findNextParam(p, 1);
  if(p) {
    Obj->dir = _ttoi(p);
    p = findNextParam(p, 1);
    if(p)
      Obj->absoluteSize = toBool(_ttoi(p));
    }
  return true;
}
//-----------------------------------------------------------
bool wrapMoveInfo::config(PWin* w, const SIZE& sz)
{
  objMoveInfo* pI = Obj;
  if(!pI) {
    pI = new objMoveInfo;
    pI->sz = sz;
    }
  bool ok = IDOK == svmMovingData(*pI, w, IDD_BMP_MOVIM).modal();
  if(ok) {
    if(!Obj)
      Obj = pI;
    enabled = true;
    }
  else if(!Obj)
    delete pI;
  return ok;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
svmMovingData::svmMovingData(objMoveInfo& mInfo, PWin* parent, uint id, HINSTANCE hInst) :
      baseClass(parent, id, hInst), mInfo(mInfo)
{

}
//-----------------------------------------------------------
svmMovingData::~svmMovingData()
{
  destroy();
}
//-----------------------------------------------------------
bool svmMovingData::create()
{
  if(!baseClass::create())
    return false;
  fill();
  uint idcs[] = {  IDC_COMBOBOX_PERIFS_MIN_X, IDC_COMBOBOX_PERIFS_MIN_Y, IDC_COMBOBOX_PERIFS_MAX_X,
        IDC_COMBOBOX_PERIFS_MAX_Y, IDC_COMBOBOX_PERIFS_CURR_X, IDC_COMBOBOX_PERIFS_CURR_Y };
  for(uint i = 0; i < SIZE_A(idcs); ++i)
    check_const_prph(idcs[i]);
  return true;
}
//-----------------------------------------------------------
LRESULT svmMovingData::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {

        case IDC_BUTTON_NORMALIZ_MIN_X:
        case IDC_BUTTON_NORMALIZ_MIN_Y:
        case IDC_BUTTON_NORMALIZ_MAX_X:
        case IDC_BUTTON_NORMALIZ_MAX_Y:
        case IDC_BUTTON_NORMALIZ_CURR_X:
        case IDC_BUTTON_NORMALIZ_CURR_Y:
          chooseNorm(LOWORD(wParam));
          break;
        case IDC_COMBOBOX_PERIFS_MIN_X:
        case IDC_COMBOBOX_PERIFS_MIN_Y:
        case IDC_COMBOBOX_PERIFS_MAX_X:
        case IDC_COMBOBOX_PERIFS_MAX_Y:
        case IDC_COMBOBOX_PERIFS_CURR_X:
        case IDC_COMBOBOX_PERIFS_CURR_Y:
          switch(HIWORD(wParam)) {
            case CBN_SELCHANGE:
              check_const_prph(LOWORD(wParam));
              break;
            }
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
#define FULL_ROW_ENABLE(a) { IDC_COMBOBOX_PERIFS_##a, IDC_EDIT_ADDR_##a, IDC_COMBOBOX_TYPEVAL_##a, IDC_EDIT_NORM_##a, IDC_STATIC_M_##a, IDC_BUTTON_NORMALIZ_##a }
//-----------------------------------------------------------
void svmMovingData::check_const_prph(uint idcb)
{
  struct lk {
    int idPrf;
    int idAddr;
    int idType;
    int idNorm;
    int idLabel;
    int idBtnNorm;
    };
  const lk LK[] = {
    FULL_ROW_ENABLE(MIN_X),
    FULL_ROW_ENABLE(MIN_Y),
    FULL_ROW_ENABLE(MAX_X),
    FULL_ROW_ENABLE(MAX_Y),
    FULL_ROW_ENABLE(CURR_X),
    FULL_ROW_ENABLE(CURR_Y),
    };
  uint ix;
  for(ix = 0; ix < SIZE_A(LK); ++ ix)
    if(LK[ix].idPrf == idcb)
      break;
  if(ix >= SIZE_A(LK))
    return;

  HWND hwnd = GetDlgItem(*this, LK[ix].idPrf);
  int sel = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
  bool enableConst = PRPH_4_CONST_CB_SEL == sel;
//  ENABLE(LK[ix].idType, !enableConst);
  ENABLE(LK[ix].idNorm, !enableConst);
  ENABLE(LK[ix].idBtnNorm, !enableConst);
  if(enableConst)
    SET_TEXT(LK[ix].idLabel, _T("Value"));
  else
    SET_TEXT(LK[ix].idLabel, _T("Addr"));
}
//----------------------------------------------------------------------------
bool getConstValue(LPCTSTR p, DWORD& value, uint type)
{
  bool isReal = false;
  switch (type) {
    case 5:
    case 7:
      isReal = true;
      break;
    default:
      do {
        LPCTSTR p2 = p;
        while (*p2 && *p2 != _T(',')) {
          if (_T('.') == *p2) {
            isReal = true;
            break;
          }
          ++p2;
        }
      } while(false);
      break;
    }
  if(isReal) {
    float v = (float)_tstof(p);
    value = *(LPDWORD)&v;
    }
  else
    value = _ttoi(p);
  return isReal;
}
//-----------------------------------------------------------
#define FULL_ROW(a) { IDC_COMBOBOX_PERIFS_##a, IDC_EDIT_ADDR_##a, IDC_COMBOBOX_TYPEVAL_##a, IDC_EDIT_NORM_##a }
//-----------------------------------------------------------
void svmMovingData::CmOk()
{
  struct lk {
    int idPrf;
    int idAddr;
    int idType;
    int idNorm;
    };

  const lk LK[] = {
    FULL_ROW(MIN_X),
    FULL_ROW(MIN_Y),
    FULL_ROW(MAX_X),
    FULL_ROW(MAX_Y),
    FULL_ROW(CURR_X),
    FULL_ROW(CURR_Y),
    };

  for(uint i = 0; i < SIZE_A(LK); ++i) {
    HWND hwnd = GetDlgItem(*this, LK[i].idPrf);
    mInfo.vI[i].perif = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
    hwnd = GetDlgItem(*this, LK[i].idType);
    if(PRPH_4_CONST_CB_SEL == mInfo.vI[i].perif) {
      mInfo.vI[i].perif = PRPH_4_CONST;
      TCHAR t[128];
      GET_TEXT(LK[i].idAddr, t);
      zeroTrim(t);
      DWORD v;
      mInfo.vI[i].typeVal = SendMessage(hwnd, CB_GETCURSEL, 0, 0);//4;//prfData::tDWData;
      bool isReal = getConstValue(t, v, mInfo.vI[i].typeVal);
      mInfo.vI[i].addr = v;
      mInfo.vI[i].normaliz = 0;
      }
    else {
      mInfo.vI[i].typeVal = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
      GET_INT(LK[i].idAddr, mInfo.vI[i].addr);
      GET_INT(LK[i].idNorm, mInfo.vI[i].normaliz);
      }
    }

  int w;
  int h;
  GET_INT(IDC_EDIT_W, w);
  GET_INT(IDC_EDIT_H, h);
  mInfo.sz.cx = R__X(w);
  mInfo.sz.cy = R__Y(h);
  mInfo.absoluteSize = !IS_CHECKED(IDC_CHECK_RELATIVE_DIM);
  mInfo.dir = 0;
  if(IS_CHECKED(IDC_CHECKBOX_REVERSE_X))
    mInfo.dir |= 1;
  if(IS_CHECKED(IDC_CHECKBOX_REVERSE_Y))
    mInfo.dir |= 2;

  baseClass::CmOk();
}
//-----------------------------------------------------------
void svmMovingData::fill()
{
  struct lk {
    int idPrf;
    int idAddr;
    int idType;
    int idNorm;
    };

  const lk LK[] = {
    FULL_ROW(MIN_X),
    FULL_ROW(MIN_Y),
    FULL_ROW(MAX_X),
    FULL_ROW(MAX_Y),
    FULL_ROW(CURR_X),
    FULL_ROW(CURR_Y),
    };

  for(uint i = 0; i < SIZE_A(LK); ++i) {
    HWND hwnd = GetDlgItem(*this, LK[i].idPrf);
    int prph = mInfo.vI[i].perif;
    if(PRPH_4_CONST == prph)
      prph = PRPH_4_CONST_CB_SEL;
    fillCBPerifEx(hwnd, prph, true);

    hwnd = GetDlgItem(*this, LK[i].idType);
    fillCBTypeVal(hwnd, mInfo.vI[i].typeVal);
    setConstValue(GetDlgItem(*this, LK[i].idAddr), mInfo.vI[i].addr, mInfo.vI[i].perif, mInfo.vI[i].typeVal);
    SET_INT(LK[i].idNorm, mInfo.vI[i].normaliz);
    }

  SET_INT(IDC_EDIT_W, REV__X(mInfo.sz.cx));
  SET_INT(IDC_EDIT_H, REV__Y(mInfo.sz.cy));

  if(mInfo.dir & 1)
    SET_CHECK(IDC_CHECKBOX_REVERSE_X);
  if(mInfo.dir & 2)
    SET_CHECK(IDC_CHECKBOX_REVERSE_Y);

  if(!mInfo.absoluteSize)
    SET_CHECK(IDC_CHECK_RELATIVE_DIM);

}
//-----------------------------------------------------------
void svmMovingData::chooseNorm(int idBtn)
{
  struct lk {
    int ibtn;
    int ied;
    };
  const lk LK[] = {
    { IDC_BUTTON_NORMALIZ_MIN_X, IDC_EDIT_NORM_MIN_X },
    { IDC_BUTTON_NORMALIZ_MIN_Y, IDC_EDIT_NORM_MIN_Y },
    { IDC_BUTTON_NORMALIZ_MAX_X, IDC_EDIT_NORM_MAX_X },
    { IDC_BUTTON_NORMALIZ_MAX_Y, IDC_EDIT_NORM_MAX_Y },
    { IDC_BUTTON_NORMALIZ_CURR_X, IDC_EDIT_NORM_CURR_X },
    { IDC_BUTTON_NORMALIZ_CURR_Y, IDC_EDIT_NORM_CURR_Y },
    };
  int ix = -1;
  for(uint i = 0; i < SIZE_A(LK); ++i)
    if(LK[i].ibtn == idBtn) {
      ix = i;
      break;
      }
  if(ix < 0)
    return;

  uint result;
  GET_INT(LK[ix].ied, result);
  if(svmSetupNormal(this, result))
    SET_INT(LK[ix].ied, result);
}
