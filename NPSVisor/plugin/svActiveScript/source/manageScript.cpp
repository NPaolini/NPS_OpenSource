//------ manageScript.cpp -----------------------------------
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "p_txt.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <activscp.h>
#include <comutil.h>
//-----------------------------------------------------------
#include "p_vect.h"
#include "p_file.h"
#include "p_util.h"
#include "sv_make_dll.h"
#include "smartPS.h"
#include "pOpensave.h"
#include "p_date.h"
#include "pcrt_lck.h"
//-----------------------------------------------------------
#define ID_SCRIPT_TYPE 20
#define ID_SCRIPT_NAME 21
//-----------------------------------------------------------
#define DEF_ARGC_GET 6
#define DEF_ARGC_SET (DEF_ARGC_GET + 1)
//-----------------------------------------------------------
struct ms_infoData
{
  uint prph;
  uint nbit;
  uint offs;
  int norm;
  ms_infoData(uint prph=0, uint nbit=0, uint offs=0, uint norm = 0):
    prph(prph), nbit(nbit), offs(offs), norm(norm)  {}
};
//-----------------------------------------------------------
#define GET_INT(d) (d.U.li.QuadPart)
#define GET_REAL(d) (d.U.rw)
//-----------------------------------------------------------
struct manageData
{
  manageData() : isReal(false) {}
  bool getData(const ms_infoData& idata, SV_prfData& data);
  bool setData(const ms_infoData& idata, SV_prfData& data, VARIANT *ret, bool alsoRet = false);
  bool isReal;
};
//-------------------------------------------
bool manageData::getData(const ms_infoData& idata, SV_prfData& data)
{
  data.U.li.QuadPart = 0;
  if(SV_prfData::tStrData == data.typeVar)
    data.U.str.len = idata.nbit;

  if(SV_prfData::okData > npSV_Get(idata.prph, &data))
    return false;
  isReal = true;
  switch(data.typeVar) {
    case SV_prfData::tBitData:
    case SV_prfData::tBData:
    case SV_prfData::tWData:
    case SV_prfData::tDWData:
    case SV_prfData::tBsData:
    case SV_prfData::tWsData:
    case SV_prfData::tDWsData:
    case SV_prfData::ti64Data:
      if(idata.nbit && !idata.norm) {
        if(idata.offs)
          data.U.li.QuadPart >>= idata.offs;
        data.U.li.QuadPart &= (1 << idata.nbit) - 1;
        }
    case SV_prfData::tStrData:
      isReal = false;
      break;
    case SV_prfData::tDateHour:
    case SV_prfData::tDate:
    case SV_prfData::tHour:
      GET_REAL(data) = (double)GET_INT(data);
      isReal = true;
      break;
    }
  if(idata.norm) {
    double n = npSV_GetNormaliz(idata.norm);
    if(isReal) {
      if(SV_prfData::tFRData == data.typeVar)
        GET_REAL(data) = data.U.fw;
      GET_REAL(data) *= n;
      }
    else {
      GET_REAL(data) = GET_INT(data) * n;
      isReal = true;
      }
    }
  else if(isReal)
    if(SV_prfData::tFRData == data.typeVar)
      GET_REAL(data) = data.U.fw;
  return true;
}
//-----------------------------------------------------------
static bool setValByType(VARIANT& varg, SV_prfData& data)
{
  bool isReal = false;
  if((VT_BYREF & varg.vt) == VT_BYREF) {
    VARIANT* v = (VARIANT*)varg.byref;
    isReal = setValByType(*v, data);
    VariantClear(&varg);
    }
  else {
    switch(varg.vt) {
      case VT_I1:
        data.U.li.QuadPart = varg.cVal;
        break;
      case VT_UI1:
        data.U.li.QuadPart = varg.bVal;
        break;
      case VT_I2:
        data.U.li.QuadPart = varg.iVal;
        break;
      case VT_I4:
        data.U.li.QuadPart = varg.lVal;
        break;
      case VT_UI2:
        data.U.li.QuadPart = varg.uiVal;
        break;
      case VT_UI4:
        data.U.li.QuadPart = varg.ulVal;
        break;
      case VT_I8:
        data.U.li.QuadPart = varg.llVal;
        break;
      case VT_UI8:
        data.U.li.QuadPart = varg.ullVal;
        break;
      case VT_R4:
        data.U.rw = varg.fltVal;
        isReal = true;
        break;
      case VT_R8:
        data.U.rw = varg.dblVal;
        isReal = true;
        break;
      case VT_BSTR:
        switch(data.typeVar) {
          case SV_prfData::tStrData:
            copyStrZ(data.U.str.buff, varg.bstrVal, min(data.U.str.len, SIZE_A(data.U.str.buff) - 1));
            break;
          case SV_prfData::tFRData:
            data.U.fw = (float)_tstof(varg.bstrVal);
            break;
          case SV_prfData::tRData:
            data.U.rw = _tstof(varg.bstrVal);
            break;
          default:
            data.U.dw = _ttol(varg.bstrVal);
            break;
          }
        break;
      }
    }
  return isReal;
}
//-------------------------------------------
#define MIN_REAL 0.000001
//-------------------------------------------
bool manageData::setData(const ms_infoData& idata, SV_prfData& data, VARIANT *val, bool alsoRet)
{
  if(SV_prfData::tStrData == data.typeVar)
    data.U.str.len = idata.nbit;

  bool local_isReal = setValByType(*val, data);

  SV_prfData dataR = data;
  ms_infoData idt = idata;
  if(idata.nbit && !idata.norm && SV_prfData::tStrData != data.typeVar)
    idt.nbit = 0;
  if(!getData(idt, dataR))
    return false;

  isReal = local_isReal;
  switch(dataR.typeVar) {
    case SV_prfData::tBitData:
    case SV_prfData::tBData:
    case SV_prfData::tWData:
    case SV_prfData::tDWData:
    case SV_prfData::tBsData:
    case SV_prfData::tWsData:
    case SV_prfData::tDWsData:
    case SV_prfData::ti64Data:
      if(isReal) {
        if(idata.norm) {
          double n = npSV_GetNormaliz(idata.norm);
          if(!n)
            n = 1;
          GET_INT(data) = ROUND_REAL_64(GET_REAL(data) / n);
          }
        else
          GET_INT(data) = ROUND_REAL_64(GET_REAL(data));
        isReal = false;
        }
      break;
    case SV_prfData::tDateHour:
    case SV_prfData::tDate:
    case SV_prfData::tHour:
      if(isReal) {
        GET_INT(data) = (LONGLONG)(GET_REAL(data) + .5);
        isReal = false;
        }
      break;
    case SV_prfData::tFRData:
    case SV_prfData::tRData:
      if(!isReal)
        GET_REAL(data) = (double)GET_INT(data);
      break;
    }

  if(!isReal) {
    if(idata.nbit && !idata.norm && SV_prfData::tStrData != data.typeVar) {
      data.U.li.QuadPart &= (1 << idata.nbit) - 1;
      if(idata.offs)
        data.U.li.QuadPart <<= idata.offs;
      __int64 t = dataR.U.li.QuadPart;
      t &= ~(((1 << idata.nbit) - 1) << idata.offs);
      data.U.li.QuadPart |= t;
      }
    }
  else {
    if(idata.norm) {
      double n = npSV_GetNormaliz(idata.norm);
      if(!n)
        n = 1;
      GET_REAL(data) /= n;
      }
    }
  if(isReal) {
    if(fabs(GET_REAL(data) - GET_REAL(dataR)) < MIN_REAL)
      return true;
    }
  else {
    if(SV_prfData::tStrData != data.typeVar)
      if(GET_INT(data) == GET_INT(dataR))
        return true;
    }

  if(SV_prfData::tFRData == data.typeVar)
    data.U.fw = (float)GET_REAL(data);

  if(alsoRet) {
    if(SV_prfData::okData > npSV_GetSet(idata.prph, &data))
      return false;
    }
  else if(SV_prfData::okData > npSV_Set(idata.prph, &data))
    return false;
  return true;
}
//-----------------------------------------------------------
static bool svScriptGet(uint prph, uint addr, uint type, uint nbit, uint offs, uint norm, VARIANT *ret)
{
  if(!ret)
    return false;
  ms_infoData idata(prph, nbit, offs, norm);
  SV_prfData data;
  data.lAddr = addr;
  data.typeVar = SV_prfData::tBitData == type ? SV_prfData::tDWData : type;

  manageData rd;
  if(!rd.getData(idata, data))
    return false;
  if(rd.isReal) {
    ret->vt=VT_R8;
    ret->dblVal=GET_REAL(data);
    }
  else {
    if(SV_prfData::tStrData == type) {
      TCHAR t[1024];
      copyStrZ(t, data.U.str.buff, min(nbit, SIZE_A(t) - 1));
      BSTR bstr = SysAllocString(t);
      ret->vt = VT_BSTR;
      ret->bstrVal = bstr;
      }
    else {
      ret->vt=VT_I4;
      ret->llVal=GET_INT(data);
      }
    }
  return true;
}
//-----------------------------------------------------------
static bool svScriptGetSet(uint prph, uint addr, uint type, uint nbit, uint offs, uint norm, VARIANT *val, VARIANT *ret)
{
  if(!ret)
    return false;
  ms_infoData idata(prph, nbit, offs, norm);
  SV_prfData data;
  data.lAddr = addr;
  data.typeVar = SV_prfData::tBitData == type ? SV_prfData::tDWData : type;

  manageData rd;
  if(!rd.setData(idata, data, val, true))
    return false;
  if(rd.isReal) {
    ret->vt=VT_R8;
    ret->dblVal=GET_REAL(data);
    }
  else {
    if(SV_prfData::tStrData == type) {
      TCHAR t[1024];
      copyStrZ(t, data.U.str.buff, min(nbit, SIZE_A(t) - 1));
      BSTR bstr = SysAllocString(t);
      ret->vt = VT_BSTR;
      ret->bstrVal = bstr;
      }
    else {
      ret->vt=VT_I4;
      ret->llVal=GET_INT(data);
      }
    }
  return true;
}
//-----------------------------------------------------------
static bool svScriptSet(uint prph, uint addr, uint type, uint nbit, uint offs, uint norm, VARIANT *val)
{
  ms_infoData idata(prph, nbit, offs, norm);
  SV_prfData data;
  data.lAddr = addr;
  data.typeVar = SV_prfData::tBitData == type ? SV_prfData::tDWData : type;

  manageData rd;
  if(!rd.setData(idata, data, val))
    return false;
  return true;
}
//-----------------------------------------------------------
static bool svScriptReturnString(LPCTSTR p, VARIANT *ret)
{
  ret->vt = VT_BSTR;
  BSTR bstr = SysAllocString(p);
  ret->bstrVal = bstr;
  return true;
}
//-----------------------------------------------------------
static bool svScriptGetLocalString(uint id, VARIANT *ret)
{
  if(!ret)
    return false;
  return svScriptReturnString(npSV_GetLocalString(id, getHinstance()), ret);
}
//-----------------------------------------------------------
static bool svScriptGetGlobalString(uint id, VARIANT *ret)
{
  if(!ret)
    return false;
  return svScriptReturnString(npSV_GetGlobalString(id), ret);
}
//-----------------------------------------------------------
static bool svScriptGetPageString(uint id, VARIANT *ret)
{
  if(!ret)
    return false;
  return svScriptReturnString(npSV_GetPageString(id), ret);
}
//-----------------------------------------------------------
static HWND getMainWin()
{
  return (HWND)npSV_GetBodyRoutine(eSV_GET_HWND_MAIN, 0, 0);
}
//-----------------------------------------------------------
static void alert(LPCTSTR txt)
{
  MessageBox(getMainWin(), txt, _T("svActiveScript-plugin"), MB_OK | MB_TOPMOST);
}
//-----------------------------------------------------------
static bool openSaveDialog(BSTR filter, BSTR file, bool forSave, VARIANT *ret)
{
  if(!ret)
    return false;
  pvvChar pvv;
  int nElem = splitParam(pvv, LPCTSTR(filter), _T('|'));
  uint dimFilter = 0;
  uint nExt = nElem / 2;
  LPTSTR* ext = new LPTSTR[nExt + 1];
  ext[nExt] = 0;

  uint dimExt = 0;
  for(int i = 0; i < nElem; ++i)
    dimFilter += _tcslen(&pvv[i]) + 1;

  smartPointerString flt(new TCHAR[dimFilter + 2], true);

  LPTSTR pFlt = &flt;
  for(int i = 0; i < nElem; ++i) {
    copyStrZ(pFlt, &pvv[i]);
    pFlt += _tcslen(pFlt) + 1;
    if(i & 1)
      ext[i / 2] = str_newdup(&pvv[i]);
    }
  *pFlt = 0;
  infoOpenSave info((LPCTSTR*)ext, &flt, (infoOpenSave::whichAction)forSave, 0, file);
  HWND hw = getMainWin();
  POpenSave os(hw);
  bool success = os.run(info);

  for(uint i = 0; i < nExt; ++i)
    delete []ext[i];
  delete []ext;
  ret->vt = VT_BSTR;
  if(success) {
    BSTR bstr = SysAllocString(os.getFile());
    ret->bstrVal = bstr;
    return true;
    }
  ret->bstrVal = 0;
  return false;
}
//-----------------------------------------------------------
static bool svGetFile(BSTR title, BSTR path, VARIANT *ret)
{
  if(!ret)
    return false;
  TCHAR t[_MAX_PATH];
  _tcscpy_s(t, path);
  LRESULT res = npSV_GetBodyRoutine(eSV_GET_FILE, (LPDWORD)(LPTSTR)t, (LPDWORD)(LPTSTR)title);
  ret->vt = VT_BSTR;
  if(res) {
    BSTR bstr = SysAllocString(t);
    ret->bstrVal = bstr;
    return true;
    }
  ret->bstrVal = 0;
  return false;
}
//-----------------------------------------------------------
static bool svGetFile2(BSTR title, BSTR path, bool showDate, bool orderByDate, bool reverse, VARIANT *ret)
{
  if(!ret)
    return false;
  loadFileInfo lfi;
  ZeroMemory(&lfi, sizeof(lfi));

  _tcscpy_s(lfi.path, path);
  _tcscpy_s(lfi.title, title);
  if(showDate)
    lfi.flag |= 1;
  if(orderByDate)
    lfi.flag |= 2;
  if(reverse)
    lfi.flag |= 4;

  LRESULT res = npSV_GetBodyRoutine(eSV_GET_FILE2, (LPDWORD)&lfi, 0);
  ret->vt = VT_BSTR;
  if(res) {
    BSTR bstr = SysAllocString(lfi.path);
    ret->bstrVal = bstr;
    return true;
    }
  ret->bstrVal = 0;
  return false;
}
//-----------------------------------------------------------
struct manageDateTime
{
  manageDateTime()  {}
  bool getData(uint prph, SV_prfData& data);
  bool setData(uint prph, SV_prfData& data, VARIANT *ret, uint dateType);
};
//-------------------------------------------
bool manageDateTime::getData(uint prph, SV_prfData& data)
{
  switch(data.typeVar) {
    case SV_prfData::tDateHour:
    case SV_prfData::tDate:
    case SV_prfData::tHour:
      break;
    default:
      return false;
    }

  data.U.li.QuadPart = 0;

  if(SV_prfData::okData > npSV_Get(prph, &data))
    return false;
  return true;
}
//-------------------------------------------
bool manageDateTime::setData(uint prph, SV_prfData& data, VARIANT *ret, uint dateType)
{
  if((VT_BYREF & ret->vt) == VT_BYREF) {
    VARIANT* v = (VARIANT*)ret->byref;
    bool result = setData(prph, data, v, dateType);
    VariantClear(ret);
    return result;
    }
  if(VT_BSTR != ret->vt)
    return false;

  if(!getData(prph, data))
    return false;

  FILETIME ft;
  switch(data.typeVar) {
    case SV_prfData::tDateHour:
    case SV_prfData::tDate:
      unformat_data(ret->bstrVal, ft, (eDataFormat)dateType);
      break;
    case SV_prfData::tHour:
      unformat_time(ret->bstrVal, ft);
      break;
    default:
      return false;
    }

  if(data.U.ft == ft)
    return true;
  data.U.ft = ft;
  if(SV_prfData::okData > npSV_Set(prph, &data))
    return false;
  return true;
}
//-----------------------------------------------------------
static bool svGetTime(uint prph, uint addr, VARIANT *ret)
{
  if(!ret)
    return false;
  SV_prfData data;
  data.lAddr = addr;
  data.typeVar = 10;

  manageDateTime rd;
  if(!rd.getData(prph, data))
    return false;
  TCHAR t[256];
  set_format_time_full(t, SIZE_A(t), data.U.ft);
  BSTR bstr = SysAllocString(t);
  ret->vt = VT_BSTR;
  ret->bstrVal = bstr;
  return true;
}
//-----------------------------------------------------------
static bool svSetTime(uint prph, uint addr, VARIANT *ret)
{
  SV_prfData data;
  data.lAddr = addr;
  data.typeVar = 10;

  manageDateTime rd;
  if(!rd.setData(prph, data, ret, 0))
    return false;
  return true;
}
//----------------------------------------------------------------------------
static void removeSep(LPTSTR p)
{
  while(*p) {
    if(_T('|') == *p) {
      *p = 0;
      break;
      }
    ++p;
    }
}
//-----------------------------------------------------------
static bool svGetDate(uint prph, uint addr, uint type, uint dateType, VARIANT *ret)
{
  if(!ret)
    return false;
  SV_prfData data;
  data.lAddr = addr;
  data.typeVar = type;

  manageDateTime rd;
  if(!rd.getData(prph, data))
    return false;
  TCHAR t[256];
  set_format_data_full(t, SIZE_A(t), data.U.ft, (eDataFormat)dateType, _T("|"));
  if(SV_prfData::tDate == type)
    removeSep(t);
  BSTR bstr = SysAllocString(t);
  ret->vt = VT_BSTR;
  ret->bstrVal = bstr;
  return true;
}
//-----------------------------------------------------------
static bool svSetDate(uint prph, uint addr, uint type, uint dateType, VARIANT *ret)
{
  SV_prfData data;
  data.lAddr = addr;
  data.typeVar = type;

  manageDateTime rd;
  if(!rd.setData(prph, data, ret, dateType))
    return false;
  return true;
}
//-----------------------------------------------------------
static bool svLoadRecipe(BSTR pagefile, BSTR recipename, VARIANT *ret)
{
  LRESULT res = npSV_GetBodyRoutine(eSV_LOAD_RECIPE, (LPDWORD)(LPTSTR)pagefile, (LPDWORD)(LPTSTR)recipename);
  if(ret) {
    ret->llVal = !res;
    ret->vt=VT_I4;
    }
  return toBool(res);
}
//-----------------------------------------------------------
static bool svSendRecipe(BSTR pagefile, BSTR recipename, VARIANT *ret)
{
  LRESULT res = npSV_GetBodyRoutine(eSV_SEND_RECIPE, (LPDWORD)(LPTSTR)pagefile, (LPDWORD)(LPTSTR)recipename);
  if(ret) {
    ret->llVal = res;
    ret->vt=VT_I4;
    }
  return !res;
}
//-----------------------------------------------------------
static bool svGetPassword(int req, VARIANT *ret)
{
  if(!ret)
    return false;
  LRESULT res = npSV_GetBodyRoutine(eSV_GET_PASSWORD, (LPDWORD)req, 0);
  ret->llVal = res;
  ret->vt=VT_I4;
  return true;
}
//-----------------------------------------------------------
static bool svGetBlock(int prph, int addr, int ndw, VARIANT *ret)
{
  if(!ret)
    return false;
  SAFEARRAY *psa = SafeArrayCreateVector(VT_UI4, 0, ndw);

  LPDWORD buff;
  HRESULT hr = SafeArrayAccessData(psa, (void**)&buff);
  if (FAILED(hr)) {
    SafeArrayDestroy(psa);
    return false;
    }
  blockInfoPrph bip = { prph, addr, ndw };
  LRESULT res = npSV_GetBodyRoutine(eSV_GET_BLOCK_DATA_PRPH, (LPDWORD)&bip, buff);
  SafeArrayUnaccessData(psa);
  VariantInit(ret);
  ret->vt = VT_ARRAY | VT_UI4;
  ret->parray = psa;
  return true;
}
//-----------------------------------------------------------
static bool svSetBlock(int prph, int addr, int ndw, VARIANT *ret)
{
  if(!ret)
    return false;
  if((ret->vt & (VT_ARRAY | VT_UI4)) != (VT_ARRAY | VT_UI4))
    return false;

  LPDWORD buff;
  HRESULT hr = SafeArrayAccessData(ret->parray, (void **)&buff);
  if (FAILED(hr))
    return false;

  blockInfoPrph bip = { prph, addr, ndw };
  LRESULT res = npSV_GetBodyRoutine(eSV_SET_BLOCK_DATA_PRPH, (LPDWORD)&bip, buff);
  SafeArrayUnaccessData(ret->parray);
  return true;
}
//-----------------------------------------------------------
static bool svFillBlock(int prph, int addr, int ndw, VARIANT *val)
{
  if(!val)
    return false;

  SV_prfData data;
  data.typeVar = SV_prfData::tDWData;
  bool isReal = setValByType(*val, data);
  DWORD v = isReal ? (DWORD)data.U.rw : data.U.dw;

  blockInfoPrph bip = { prph, addr, ndw };
  LRESULT res = npSV_GetBodyRoutine(eSV_FILL_BLOCK_DATA_PRPH, (LPDWORD)&bip, (LPDWORD)v);
  return true;
}
//-----------------------------------------------------------
//static HANDLE hEvBreak;
static HANDLE hEvClose;
//-----------------------------------------------------------
static bool svCanContinue(VARIANT *ret, HANDLE hEvBreak)
{
  if(!ret)
    return false;
  HANDLE HandlesToWaitFor[] = { hEvBreak, hEvClose };
  int res = 0;
  DWORD result = WaitForMultipleObjects(SIZE_A(HandlesToWaitFor), HandlesToWaitFor, FALSE, INFINITE);
  switch(result) {
    case WAIT_OBJECT_0:
      res = 1;
      break;
    default:
      break;
    }
  ret->llVal = res;
  ret->vt=VT_I4;
  return true;
}
//-----------------------------------------------------------
class manageSingleCS
{
  public:
    manageSingleCS() {  }
    ~manageSingleCS() {  }
    void enter();
    void leave();
  private:
    criticalSect CS;
};
//-----------------------------------------------------------
void manageSingleCS::enter()
{
  EnterCriticalSection(&CS.get());
}
//-----------------------------------------------------------
void manageSingleCS::leave()
{
  LeaveCriticalSection(&CS.get());
}
//-----------------------------------------------------------
#define MAX_CS 10
//-----------------------------------------------------------
//-----------------------------------------------------------
class manageCS
{
  public:
    manageCS() {  }
    ~manageCS() {  }
    void enter(uint ix);
    void leave(uint ix);
  private:
    manageSingleCS CS[MAX_CS];
};
//-----------------------------------------------------------
void manageCS::enter(uint ix)
{
  if(ix >= MAX_CS)
    return;
  CS[ix].enter();
}
//-----------------------------------------------------------
void manageCS::leave(uint ix)
{
  if(ix >= MAX_CS)
    return;
  CS[ix].leave();
}
//-----------------------------------------------------------
static manageCS manCS;
//-----------------------------------------------------------
typedef bool (*svScriptGen)(uint prph, uint addr, uint type, uint nbit, uint offs, uint norm, VARIANT *ret);
//-----------------------------------------------------------
#define SV_SCRIPT_GET 1
#define SV_SCRIPT_SET 2
#define SV_SCRIPT_ALERT 3
#define SV_SCRIPT_OPEN_SAVE 4

#define SV_SCRIPT_GET_TIME 5
#define SV_SCRIPT_GET_DATE 6
#define SV_SCRIPT_SET_TIME 7
#define SV_SCRIPT_SET_DATE 8

#define SV_SCRIPT_LOAD_RECIPE 9
#define SV_SCRIPT_SEND_RECIPE 10

#define SV_SCRIPT_CAN_CONTINUE 11

#define SV_SCRIPT_GET_FILE  12

#define SV_SCRIPT_GET_PASSWORD 13

#define SV_SCRIPT_GET_LOCALSTRING 14
#define SV_SCRIPT_GET_GLOBALSTRING 15
#define SV_SCRIPT_GET_PAGESTRING 16

#define SV_SCRIPT_GETSET 17

#define SV_SCRIPT_GET_BLOCK  18
#define SV_SCRIPT_SET_BLOCK  19
#define SV_SCRIPT_FILL_BLOCK 20

#define SV_SCRIPT_SLEEP 21

#define SV_SCRIPT_ENTER_CS 22
#define SV_SCRIPT_LEAVE_CS 23
//-----------------------------------------------------------
class manageScript;
//-----------------------------------------------------------
struct infoErr
{
  LPCTSTR title;
  LPCTSTR msg;
  infoErr() : title(0), msg(0) {}
  infoErr(LPCTSTR tit, LPCTSTR ms) : title(str_newdup(tit)), msg(str_newdup(ms)) {}
  ~infoErr() { delete []title; delete []msg; }
  infoErr(infoErr& other) : title(other.title), msg(other.msg) {  other.title = 0; other.msg = 0; }
  infoErr& operator=(infoErr& other) { title = other.title; other.title = 0; msg = other.msg; other.msg = 0; }
};
//-----------------------------------------------------------
struct ScriptInterface : public IDispatch
{
  ScriptInterface(manageScript* owner) :  onExec(false), Owner(owner) { }
  long  WINAPI QueryInterface( REFIID riid,void ** object) {

    *object = IsEqualIID(riid, IID_IDispatch) ? this : 0;
    return *object ? NOERROR : E_NOINTERFACE;
    }
  DWORD WINAPI AddRef ()  { return 0; }
  DWORD WINAPI Release()  { return 0; }
  long  WINAPI GetTypeInfoCount( UINT *p) { *p = 0; return NO_ERROR; }
  long  WINAPI GetTypeInfo( UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo)
  {
    *ppTInfo = 0;
    return TYPE_E_ELEMENTNOTFOUND;
  }
  // This is where we register procs (or vars)
  long  WINAPI GetIDsOfNames( REFIID riid, WCHAR** name, UINT cnt ,LCID lcid, DISPID *id);

  // And this is where they are called from script
  long  WINAPI Invoke( DISPID id, REFIID riid, LCID lcid, WORD flags, DISPPARAMS *arg, VARIANT *ret, EXCEPINFO *excp, UINT *err);
  bool onExec;
  manageScript* Owner;
  long retResult(long ret) { onExec = false; return ret; }
  void setErrorMsg(infoErr* iErr);
};
//-----------------------------------------------------------
struct ScriptHost : public IActiveScriptSite
{
  ScriptHost(manageScript* owner) : Interface(owner) { }
    ScriptInterface Interface;
    long  WINAPI QueryInterface( REFIID riid,void ** object)
    {
      *object = (IsEqualIID(riid, IID_IActiveScriptSite)) ? this : 0;
      return *object ? NOERROR : E_NOINTERFACE;
      }
    DWORD WINAPI AddRef ()                                                                { return 0; }
    DWORD WINAPI Release()                                                                { return 0; }
    long  WINAPI GetLCID( DWORD *lcid )           {  *lcid = LOCALE_USER_DEFAULT;           return 0; }
    long  WINAPI GetDocVersionString( BSTR* ver ) {  *ver  = 0;                             return 0; }
    long  WINAPI OnScriptTerminate(const VARIANT *,const EXCEPINFO *)                     { return 0; }
    long  WINAPI OnStateChange( SCRIPTSTATE state)                                        { return 0; }
    long  WINAPI OnEnterScript()                                                          { return 0; }
    long  WINAPI OnLeaveScript()                                                          { return 0; }
    long  WINAPI GetItemInfo(const WCHAR *name,DWORD req, IUnknown ** obj, ITypeInfo ** type)
    {
      if(req & SCRIPTINFO_IUNKNOWN)
        *obj = &Interface;
      if(req & SCRIPTINFO_ITYPEINFO)
        *type = 0;
      return 0;
    }
    long  WINAPI OnScriptError( IActiveScriptError *err )
    {
      EXCEPINFO e;
      ZeroMemory(&e, sizeof(e));
      err->GetExceptionInfo(&e);
      _bstr_t wrapSource;
      BSTR bstrSource = 0;
      if(S_OK == err->GetSourceLineText(&bstrSource)) {
        wrapSource.Attach(bstrSource);
        DWORD dwSourceContext = 0;
        ULONG ulLineNumber = 0;
        LONG pichCharPosition = 0;
        err->GetSourcePosition(&dwSourceContext, &ulLineNumber, &pichCharPosition);
        wchar_t buff[4096];
        wsprintfW(buff, L"%s\r\npos[%d] at line [%d]\r\nsource row:\r\n%s", e.bstrDescription, pichCharPosition, ulLineNumber, bstrSource);
        infoErr* iErr = new infoErr(e.bstrSource, buff);
        Interface.setErrorMsg(iErr);
//        MessageBoxW(getMainWin(), buff, e.bstrSource, MB_OK | MB_TOPMOST | MB_ICONSTOP);

        }
      else {
        infoErr* iErr = new infoErr(e.bstrSource, e.bstrDescription);
        Interface.setErrorMsg(iErr);
//        MessageBoxW(getMainWin(), e.bstrDescription, e.bstrSource, MB_OK | MB_TOPMOST | MB_ICONSTOP);
        }
      SysFreeString(e.bstrSource);
      SysFreeString(e.bstrDescription);
      SysFreeString(e.bstrHelpFile);
      return 0;
    }
};
//-----------------------------------------------------------
class manageScript
{
  public:
    manageScript(uint offs);
    ~manageScript();

    bool run();
    bool init();
    void setError() { initialized = -1; }
    void setError(infoErr* ie) { if(iErr) delete iErr; iErr = ie; }
    HANDLE getHBreak() { return hEvBreak; }
  private:
    int initialized;
    HANDLE hThread;
    HANDLE hEvBreak;
    uint Offs;
    friend unsigned FAR PASCAL ScriptProc(void*);
    infoErr* iErr;
    void showInfoErr();
};
//-----------------------------------------------------------
static PVect<manageScript*> mScript;
//-----------------------------------------------------------
#define MAX_SCRIPT 10
#define OFFS_SCRIPT 10
//-----------------------------------------------------------
bool runScript()
{
  uint nElem = mScript.getElem();
  if(!nElem)
    return false;
  // lo facciamo permissivo, fallisce se falliscono tutti
  bool success = false;
  for(uint i = 0; i < nElem; ++i)
    success |= mScript[i]->run();
  return success;
}
//-----------------------------------------------------------
void endScript()
{
  uint nElem = mScript.getElem();
  for(uint i = 0; i < nElem; ++i)
    delete mScript[i];
  mScript.reset();
  CloseHandle(hEvClose);
  hEvClose = 0;
}
//-----------------------------------------------------------
bool initScript()
{
  endScript();
  if(!hEvClose)
    hEvClose = CreateEvent(0, TRUE, FALSE, 0);
  for(uint i = 0, j = 0; i < MAX_SCRIPT; ++i) {
    LPCTSTR p = npSV_GetLocalString(ID_SCRIPT_TYPE + i * OFFS_SCRIPT, getHinstance());
    if(p) {
      mScript[j] = new manageScript(i * OFFS_SCRIPT);
      if(!mScript[j]->init()) {
        endScript();
        return false;
        }
      ++j;
      }
    }
  return true;
}
//------------------------------------------------------------------
manageScript::manageScript(uint offs) : initialized(0), Offs(offs), hThread(0),
    hEvBreak(CreateEvent(0, FALSE, FALSE, 0)), iErr(0)
{
}
//------------------------------------------------------------------
manageScript::~manageScript()
{
  SetEvent(hEvClose);
  WaitForSingleObject(hThread, INFINITE);
  CloseHandle(hEvBreak);
  hEvBreak = 0;
  delete iErr;
}
//------------------------------------------------------------------
void manageScript::showInfoErr()
{
  if(iErr) {
    infoErr wrap = *iErr;
    delete iErr;
    iErr = 0;
    MessageBox(getMainWin(), wrap.msg, wrap.title, MB_OK | MB_TOPMOST);
    }
}
//------------------------------------------------------------------
bool manageScript::run()
{
  if(!init()) {
    showInfoErr();
    return false;
    }
  SetEvent(hEvBreak);
  return true;
}
//---------------------------------------------------------------------
bool manageScript::init()
{
  if(initialized)
    return initialized > 0;

  DWORD idThread = 0;
  hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ScriptProc, this, 0, &idThread);
  if(!hThread)
    return false;
  for(uint i = 0; i < 100; ++i) {
    if(initialized > 0)
      return true;
    if(initialized < 0)
      break;
    Sleep(100);
    }
  showInfoErr();
  return false;
}
//------------------------------------------------------------------
#define MAX_SOURCE 4
//------------------------------------------------------------------
struct sourceFile
{
  LPTSTR source;
  uint dim;
  sourceFile() : source(0), dim(0) {}
  bool load(LPCTSTR filename);
};
//------------------------------------------------------------------
bool sourceFile::load(LPCTSTR file)
{
  do {
    source = openFile(file, dim);
    if(!source)
      break;
    if(!dim)
      break;
    return true;
    } while(false);
  return false;
}
//------------------------------------------------------------------
static LPTSTR unionSource(sourceFile* set, uint nElem)
{
  uint tot = 0;
  for(uint i = 0; i < nElem; ++i)
    tot += set[i].dim;
  if(!tot)
    return 0;
  ++tot;
  LPTSTR t = new TCHAR[tot + 1];
  t[0] = 0;
  for(uint i = 0; i < nElem; ++i) {
    if(set[i].dim) {
      _tcscat_s(t, tot, set[i].source);
      delete []set[i].source;
      set[i].dim = 0;
      }
    }
  set[0].dim = tot;
  return t;
}
//------------------------------------------------------------------
unsigned FAR PASCAL ScriptProc(void* cl)
{
  CoInitialize(0);
  manageScript *owner = reinterpret_cast<manageScript*>(cl);

  IActiveScript* script = 0;
  IActiveScriptParse* parse = 0;
  ScriptHost host(owner);

  bool success = false;
  LPTSTR source = 0;
  do {
    LPCTSTR lang = npSV_GetLocalString(ID_SCRIPT_TYPE + owner->Offs, getHinstance());
    if(!lang)
      break;
    sourceFile SourceFile[MAX_SOURCE];
    // carica prima i tre facenti da include e poi il primo che è il file principale
    uint seq[] = { 1, 2, 3, 0 };
    for(uint i = 0; i < MAX_SOURCE; ++i) {
      LPCTSTR file = npSV_GetLocalString(ID_SCRIPT_NAME + owner->Offs + seq[i], getHinstance());
      if(file)
        SourceFile[i].load(file);
      }
    source = unionSource(SourceFile, MAX_SOURCE);
    if(!SourceFile[0].dim)
      break;
    if(!source)
      break;
    GUID guid;
    CLSIDFromProgID(lang, &guid);

    HRESULT hr = CoCreateInstance(guid, 0, CLSCTX_ALL,IID_IActiveScript,(void **)&script);
    if(!script)
      break;
    hr = script->QueryInterface(IID_IActiveScriptParse, (void **)&parse);
    if(S_OK != hr)
      break;

    script->SetScriptSite(&host);
    script->AddNamedItem(L"sVisor", SCRIPTITEM_ISVISIBLE | SCRIPTITEM_NOCODE | SCRIPTITEM_ISPERSISTENT);
    hr = parse->InitNew();
    if(S_OK != hr)
      break;
    hr = parse->ParseScriptText(source, 0, 0, 0, 0, 0, SCRIPTTEXT_ISPERSISTENT, 0, 0);
    if(S_OK != hr)
      break;
    // tutto ok, imposta il successo
    success = true;
    } while(false);
  delete []source;
  if(success) {
    owner->initialized = 1;
    script->SetScriptState(SCRIPTSTATE_CONNECTED);
    owner->initialized = -2;
    }
  else
    owner->initialized = -1;
  if(script) {
    script->Close();
    script->Release();
    }
  CoUninitialize();
  return 0;
}
//-----------------------------------------------------------
void ScriptInterface::setErrorMsg(infoErr* iErr) { Owner->setError(iErr); }

static manageSingleCS ManSingleCS;
static uint totId;
//-----------------------------------------------------------
long WINAPI ScriptInterface::GetIDsOfNames(REFIID riid, WCHAR** name, UINT cnt, LCID lcid, DISPID *id)
{
  ManSingleCS.enter();
  totId += cnt;
  for(uint i = 0; i < cnt; ++i) {
    if(!_tcscmp(name[i], _T("svScriptGet")))
      id[i] = SV_SCRIPT_GET;
    else if(!_tcscmp(name[i], _T("svScriptSet")))
      id[i] = SV_SCRIPT_SET;
    else if(!_tcscmp(name[i], _T("svCanContinue")))
      id[i] = SV_SCRIPT_CAN_CONTINUE;
    else if(!_tcscmp(name[i], _T("svGetTime")))
      id[i] = SV_SCRIPT_GET_TIME;
    else if(!_tcscmp(name[i], _T("svGetDate")))
      id[i] = SV_SCRIPT_GET_DATE;
    else if(!_tcscmp(name[i], _T("svSetTime")))
      id[i] = SV_SCRIPT_SET_TIME;
    else if(!_tcscmp(name[i], _T("svSetDate")))
      id[i] = SV_SCRIPT_SET_DATE;
    else if(!_tcscmp(name[i], _T("alert")))
      id[i] = SV_SCRIPT_ALERT;
    else if(!_tcscmp(name[i], _T("openSaveDialog")))
      id[i] = SV_SCRIPT_OPEN_SAVE;
    else if(!_tcscmp(name[i], _T("svGetFile")))
      id[i] = SV_SCRIPT_GET_FILE;
    else if(!_tcscmp(name[i], _T("svLoadRecipe")))
      id[i] = SV_SCRIPT_LOAD_RECIPE;
    else if(!_tcscmp(name[i], _T("svSendRecipe")))
      id[i] = SV_SCRIPT_SEND_RECIPE;
    else if(!_tcscmp(name[i], _T("svGetPassword")))
      id[i] = SV_SCRIPT_GET_PASSWORD;

    else if(!_tcscmp(name[i], _T("svGetLocalString")))
      id[i] = SV_SCRIPT_GET_LOCALSTRING;
    else if(!_tcscmp(name[i], _T("svGetGlobalString")))
      id[i] = SV_SCRIPT_GET_GLOBALSTRING;
    else if(!_tcscmp(name[i], _T("svGetPageString")))
      id[i] = SV_SCRIPT_GET_PAGESTRING;
    else if(!_tcscmp(name[i], _T("svScriptGetSet")))
      id[i] = SV_SCRIPT_GETSET;

    else if(!_tcscmp(name[i], _T("svScriptGetBlock")))
      id[i] = SV_SCRIPT_GET_BLOCK;
    else if(!_tcscmp(name[i], _T("svScriptSetBlock")))
      id[i] = SV_SCRIPT_SET_BLOCK;
    else if(!_tcscmp(name[i], _T("svScriptFillBlock")))
      id[i] = SV_SCRIPT_FILL_BLOCK;
    else if(!_tcscmp(name[i], _T("svScriptSleep")))
      id[i] = SV_SCRIPT_SLEEP;
    else if(!_tcscmp(name[i], _T("svEnterCriticalSection")))
      id[i] = SV_SCRIPT_ENTER_CS;
    else if(!_tcscmp(name[i], _T("svLeaveCriticalSection")))
      id[i] = SV_SCRIPT_LEAVE_CS;
    else {
      ManSingleCS.leave();
      return E_FAIL;
      }
    }
  return S_OK;
}
//-----------------------------------------------------------
static uint getValByType(VARIANT& varg)
{
  if((VT_BYREF & varg.vt) == VT_BYREF) {
    VARIANT* v = (VARIANT*)varg.byref;
    uint ret = getValByType(*v);
    VariantClear(&varg);
    return ret;
    }
  switch(varg.vt) {
    case VT_I1:
      return varg.cVal;
    case VT_UI1:
      return varg.bVal;
    case VT_I2:
      return varg.iVal;
    case VT_I4:
      return varg.lVal;
    case VT_UI2:
      return varg.uiVal;
    case VT_UI4:
      return varg.ulVal;
    case VT_BOOL:
      return varg.boolVal;
    }
  return 0;
}
//-----------------------------------------------------------
static BSTR getBSTRByType(VARIANT& varg)
{
  if((VT_BYREF & varg.vt) == VT_BYREF) {
    VARIANT* v = (VARIANT*)varg.byref;
    BSTR ret = getBSTRByType(*v);
    VariantClear(&varg);
    return ret;
    }
  switch(varg.vt) {
    case VT_BSTR:
      return varg.bstrVal;
    }
  return 0;
}
//-----------------------------------------------------------
#define G_TVAL(ix) getValByType(arg->rgvarg[ix])
#define G_BSTR(ix) getBSTRByType(arg->rgvarg[ix])
//-----------------------------------------------------------
long WINAPI ScriptInterface::Invoke(DISPID id, REFIID riid, LCID lcid, WORD flags, DISPPARAMS *arg, VARIANT *ret, EXCEPINFO *excp, UINT *err)
{
  if(!(flags & DISPATCH_METHOD)) // (DISPATCH_PROPERTYGET | DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF)
    return DISP_E_TYPEMISMATCH;

  if(onExec)
    return E_FAIL;
  onExec = true;
  if(totId)
    --totId;
  if(!totId)
    ManSingleCS.leave();
  switch(id) {
    case SV_SCRIPT_GET:
      if(DEF_ARGC_GET > arg->cArgs)
        return retResult(E_FAIL);
      if(!svScriptGet(G_TVAL(5),G_TVAL(4),G_TVAL(3),G_TVAL(2),G_TVAL(1),G_TVAL(0), ret)) {
        Owner->setError();
        return retResult(E_FAIL);
        }
      break;
    case SV_SCRIPT_SET:
      if(DEF_ARGC_SET > arg->cArgs)
        return retResult(E_FAIL);
      if(!svScriptSet(G_TVAL(6),G_TVAL(5),G_TVAL(4),G_TVAL(3),G_TVAL(2),G_TVAL(1), &arg->rgvarg[0])) {
        Owner->setError();
        return retResult(E_FAIL);
        }
      break;
    case SV_SCRIPT_ALERT:
      if(!arg->cArgs)
        return retResult(E_FAIL);
      alert(arg->rgvarg[0].bstrVal);
      break;
    case SV_SCRIPT_OPEN_SAVE:
      if(3 > arg->cArgs)
        return retResult(E_FAIL);
      if(!openSaveDialog(G_BSTR(2),G_BSTR(1),toBool(G_TVAL(0)), ret)) {
//        mScript->setError();
//        onExec = false;
//        return E_FAIL;
        }
      break;

    case SV_SCRIPT_GET_TIME:
      if(2 > arg->cArgs)
        return retResult(E_FAIL);
      if(!svGetTime(G_TVAL(1),G_TVAL(0), ret)) {
        Owner->setError();
        return retResult(E_FAIL);
        }
      break;
    case SV_SCRIPT_SET_TIME:
      if(3 > arg->cArgs)
        return retResult(E_FAIL);
      if(!svSetTime(G_TVAL(2),G_TVAL(1), &arg->rgvarg[0])) {
        Owner->setError();
        return retResult(E_FAIL);
        }
      break;

    case SV_SCRIPT_GET_DATE:
      if(4 > arg->cArgs)
        return retResult(E_FAIL);
      if(!svGetDate(G_TVAL(3),G_TVAL(2),G_TVAL(1),G_TVAL(0), ret)) {
        Owner->setError();
        return retResult(E_FAIL);
        }
      break;
    case SV_SCRIPT_SET_DATE:
      if(5 > arg->cArgs)
        return retResult(E_FAIL);
      if(!svSetDate(G_TVAL(4),G_TVAL(3),G_TVAL(2),G_TVAL(1), &arg->rgvarg[0])) {
        Owner->setError();
        return retResult(E_FAIL);
        }
      break;

    case SV_SCRIPT_LOAD_RECIPE:
      if(2 > arg->cArgs)
        return retResult(E_FAIL);
      svLoadRecipe(G_BSTR(1),G_BSTR(0), ret);
      break;
    case SV_SCRIPT_SEND_RECIPE:
      if(2 > arg->cArgs)
        return retResult(E_FAIL);
      svSendRecipe(G_BSTR(1),G_BSTR(0), ret);
      break;
    case SV_SCRIPT_CAN_CONTINUE:
      if(!svCanContinue(ret, Owner->getHBreak())) {
        Owner->setError();
        return retResult(E_FAIL);
        }
      break;
    case SV_SCRIPT_GET_FILE:
      if(2 != arg->cArgs && 5 > arg->cArgs)
        return retResult(E_FAIL);
      if(2 == arg->cArgs)
        svGetFile(G_BSTR(1), G_BSTR(0), ret);
      else
        svGetFile2(G_BSTR(4), G_BSTR(3), toBool(G_TVAL(2)), toBool(G_TVAL(1)), toBool(G_TVAL(0)), ret);
      break;
    case SV_SCRIPT_GET_PASSWORD:
      if(!svGetPassword(G_TVAL(0), ret))
        return retResult(E_FAIL);
      break;
    case SV_SCRIPT_GET_LOCALSTRING:
      if(!svScriptGetLocalString(G_TVAL(0), ret))
        return retResult(E_FAIL);
      break;
    case SV_SCRIPT_GET_GLOBALSTRING:
      if(!svScriptGetGlobalString(G_TVAL(0), ret))
        return retResult(E_FAIL);
      break;
    case SV_SCRIPT_GET_PAGESTRING:
      if(!svScriptGetPageString(G_TVAL(0), ret))
        return retResult(E_FAIL);
      break;
    case SV_SCRIPT_GETSET:
      if(DEF_ARGC_SET > arg->cArgs)
        return retResult(E_FAIL);
      if(!svScriptGetSet(G_TVAL(6),G_TVAL(5),G_TVAL(4),G_TVAL(3),G_TVAL(2),G_TVAL(1), &arg->rgvarg[0], ret)) {
        Owner->setError();
        return retResult(E_FAIL);
        }
      break;

    case SV_SCRIPT_GET_BLOCK:
      if(!svGetBlock(G_TVAL(2),G_TVAL(1),G_TVAL(0), ret)) {
        Owner->setError();
        return retResult(E_FAIL);
        }
      break;
    case SV_SCRIPT_SET_BLOCK:
      if(!svSetBlock(G_TVAL(3),G_TVAL(2),G_TVAL(1), &arg->rgvarg[0])) {
        Owner->setError();
        return retResult(E_FAIL);
        }
      break;
    case SV_SCRIPT_FILL_BLOCK:
      if(!svFillBlock(G_TVAL(3),G_TVAL(2),G_TVAL(1), &arg->rgvarg[0])) {
        Owner->setError();
        return retResult(E_FAIL);
        }
      break;
    case SV_SCRIPT_SLEEP:
      Sleep(G_TVAL(0));
      break;
    case SV_SCRIPT_ENTER_CS:
      manCS.enter(G_TVAL(0));
      break;
    case SV_SCRIPT_LEAVE_CS:
      manCS.leave(G_TVAL(0));
      break;

    default:
      return retResult(E_FAIL);
    }
  return retResult(S_OK);
}
