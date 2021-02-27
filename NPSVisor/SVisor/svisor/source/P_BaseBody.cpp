//------ P_BaseBody.cpp ----------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//#define _WIN32_WINNT 0x501
#include <commctrl.h>
//----------------------------------------------------------------------------
#include "sizer.h"
#include "p_basebody.h"
#include "mainclient.h"
#include "hdrmsg.h"
#include "password.h"
#include "prfData.h"
#include "p_util.h"
#include "p_date.h"
#include "p_heap.h"
#include "newnormal.h"
#include "pTraspBitmap.h"
#include "pCommonFilter.h"
#include "PCustomListView.h"
#include "1.h"
#include "pAllObj.h"
#include "config.h"
//----------------------------------------------------------------------------
#include "p_ManDynaBody.h"
//----------------------------------------------------------------------------
#ifndef WM_PLC
  #define WM_PLC 2
#endif
//----------------------------------------------------------------------------
P_BaseBody::P_BaseBody(PWin* parent, uint resId, HINSTANCE hInstance)
:
    P_Base(parent, resId, hInstance), timeoutEdit(0), notShowWhileSimpleSend(0),
    hBmpWork(0), mdcWork(0), oldObjWork(0), isReady(false),  hBmpBkgWork(0),
    mdcBkgWork(0), oldObjBkgWork(0), DirtyBkg(false)
{
  Offset.x = 0;
  Offset.y = 0;
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
P_BaseBody::~P_BaseBody()
{
//  sendListValue(ID_SEND_VALUE_ON_CLOSE);

  flushAll();
  destroy();
}
//---------------------------------------------------------------------
LPCTSTR P_BaseBody::sendValue(LPCTSTR p)
{
  if(!p)
    return 0;
  uint prph = 0;
  uint addr = 0;
  uint type = 0;
  uint nbit = 0;
  uint offs = 0;
  uint norm = 0;
  int n = _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d"), &prph, &addr, &type, &nbit, &offs, &norm);
  if(!prph)
    return 0;
  p = findNextParamTrim(p, n);
  if(!p)
    return 0;
  prfData data;
  genericPerif* perif = Par->getGenPerif(prph);
  if(!perif)
    return findNextParamTrim(p);

  data.lAddr = addr;
  data.typeVar = type;
//  if(prfData::tBitData == type && (offs + nbit > 15))
//    data.typeVar = type = prfData::tDWData;

  if(prfData::tFRData == type || prfData::tFRData == type) {
    REALDATA val = _tstof(p);
    REALDATA nrm = getNorm(norm);
    if(!getDenormalizedData(data, val, nrm))
      return findNextParamTrim(p);
    }
  else {
    DWDATA val = _ttoi(p);
    if(nbit) {
      if(prfData::tBitData == type)
        data.U.dw =  MAKELONG(MAKEWORD(nbit, offs), val);
      else {
        perif->get(data);
        data.U.li.QuadPart |= (val & ((1 << nbit) - 1)) << offs;
        }
      }
    else if(norm) {
      REALDATA nrm = getNorm(norm);
      if(!getDenormalizedData(data, val, nrm))
        return findNextParamTrim(p);
      }
    else
      data.U.li.QuadPart = val;
    }
  gestPerif::howSend old = perif->setAutoShow(gestPerif::CHECK_READY);
  perif->set(data);
  perif->setAutoShow(old);
  return findNextParamTrim(p);
}
//---------------------------------------------------------------------
static void flushCam(PVect<PVarCam*>& cam)
{
  uint nElem = cam.getElem();
  for(uint i = 0; i < nElem; ++i) {
    if(cam[i]->isCamGlobal()) {
      cam[i]->restoreMain();
      cam[i] = 0;
      }
    else
      delete cam[i];
    }
  cam.reset();
}
//---------------------------------------------------------------------
void P_BaseBody::flushAll()
{
  removeAllBmpWork(3);
  flushPV(oTableInfo);
  flushPV(oSpin);
  flushPV(oTrend);
  flushPV(oAlarm);
  flushPV(Scope);
  flushPV(XMeter);
  flushPV(Btn);
  flushPV(Choose);
  flushPV(LBox);
  flushPV(Edi);
  flushPV(Curve);
  flushCam(Cam);
//  flushPV(Cam);
  flushPV(Diam);
  flushPV(Led);
  flushPV(barGraph);
  flushPV(Simple);
  flushPV(SimpleWithHide);
  flushPV(Txt);
  flushPV(BmpForBtn);
  flushPV(Bmp);
  flushPV(Slider);

  int nElem = Font.getElem();
  {
  for(int i = 0; i < nElem; ++i)
    DeleteObject(Font[i]);
  Font.reset();
  }
}
//----------------------------------------------------------
HWND P_BaseBody::setListBoxCallBack(infoCallBackSend* fz, uint ixLB)
{
  uint forWinProc = HIWORD(ixLB);
  ixLB = LOWORD(ixLB);
  if(ixLB >= LBox.getElem())
    return 0;
  return LBox[ixLB]->setListBoxCallBack(fz, forWinProc);
}
//---------------------------------------------------------------------
bool P_BaseBody::allocBmpWork(HDC hdc)
{
  if(hBmpWork)
    return true;

  PRect r;
  GetClientRect(*this, r);
  if(r.Width() && r.Height() && getHandle()) {
    HDC dc = hdc;
    if(!dc)
      dc = GetDC(*this);
    hBmpWork = CreateCompatibleBitmap(dc, r.Width(), r.Height());
    if(hBmpWork) {
      mdcWork = CreateCompatibleDC(dc);
      oldObjWork = SelectObject(mdcWork, hBmpWork);
      }
    if(!hdc)
      ReleaseDC(*this, dc);
    }
  return toBool(hBmpWork);
}
//---------------------------------------------------------------------
bool P_BaseBody::allocBmpBkgWork(HDC hdc)
{
  if(hBmpBkgWork)
    return true;

  PRect r;
  GetClientRect(*this, r);
  if(r.Width() && r.Height() && getHandle()) {
    HDC dc = hdc;
    if(!dc)
      dc = GetDC(*this);
    hBmpBkgWork = CreateCompatibleBitmap(dc, r.Width(), r.Height());
    if(hBmpBkgWork) {
      mdcBkgWork = CreateCompatibleDC(dc);
      oldObjBkgWork = SelectObject(mdcBkgWork, hBmpBkgWork);
      HBRUSH br = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
      FillRect(mdcBkgWork, r, br);

      }
    if(!hdc)
      ReleaseDC(*this, dc);
    }
  return toBool(hBmpBkgWork);
}
//---------------------------------------------------------------------
void P_BaseBody::removeAllBmpWork(uint bits)
{
  if(hBmpWork && (bits & 1)) {
    SelectObject(mdcWork, oldObjWork);
    DeleteDC(mdcWork);
    DeleteObject(hBmpWork);
    hBmpWork = 0;
    }
  if(hBmpBkgWork && (bits & 2)) {
    SelectObject(mdcBkgWork, oldObjBkgWork);
    DeleteDC(mdcBkgWork);
    DeleteObject(hBmpBkgWork);
    hBmpBkgWork = 0;
    }
  uint nElem = Edi.getElem();
  for(uint i = 0; i < nElem; ++i)
    Edi[i]->resetClip();
  nElem = Curve.getElem();
  for(uint i = 0; i < nElem; ++i)
    Curve[i]->resetClip();
}
//---------------------------------------------------------------------
WORD P_BaseBody::getVersionBody()
{
  LPCTSTR p = getPageString(ID_VERSION);
  if(!p)
    return 0;
  int h = 0;
  int l = 0;
  _stscanf_s(p, _T("%d,%d"), &h, &l);
  return MAKEWORD(l, h);
}
//---------------------------------------------------------------------
void P_BaseBody::initTimeEdit(int sec)
{
  uint baseTime = Par->getTimeForTimer();
  if(!baseTime)
    baseTime = 300;
  static uint secEdit = 0;
  if(!secEdit) {
    secEdit = DEFAULT_TIMEOUT_EDIT;
    LPCTSTR p = ::getString(ID_TIMER_4_EDIT);
    if(p) {
      uint t = _ttoi(p);
      if(3 <= t && t < 60)
        secEdit = t;
      }
    }
  if(sec == DEFAULT_TIMEOUT_EDIT)
    sec = secEdit;
  sec *= 1000;
  sec /= baseTime;
  timeoutEdit = sec;
}
//---------------------------------------------------------------------
HBRUSH P_BaseBody::evCtlColor(HDC dc, HWND hWndChild, UINT ctlType)
{
  HBRUSH brush = 0;
  int nEdi = Edi.getElem();
  for(int i = 0; i < nEdi; ++i)
    if(Edi[i]->evCtlColor(brush, dc, hWndChild, ctlType))
      return brush;
  int nCurve = Curve.getElem();
  for(int i = 0; i < nCurve; ++i)
    if(Curve[i]->evCtlColor(brush, dc, hWndChild, ctlType))
      return brush;

  return P_Base::evCtlColor(dc, hWndChild, ctlType);
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void P_BaseBody::makeCam()
{
  LPCTSTR p = getPageString(ID_VAR_CAM);
  if(p) {
    int nCam = _ttoi(p);
    if(nCam) {
      if(nCam > 10)
        nCam = 10;
      Cam.setDim(nCam);
      int numC = 0;
      mainClient* mc = getMain();

      for(int i = 0; i < nCam; ++i) {
        TCHAR pagename[_MAX_PATH] = _T("\0");
        uint id = i + ID_INIT_CAM;
        LPCTSTR p0 = getPageString(id);
        LPCTSTR p = findNextParam(p0, 6);
        if(p)
          _tcscpy_s(pagename, SIZE_A(pagename), p);
        else {
          if(getPageName())
            _tcscpy_s(pagename, getPageName());
//          getFileStr(pagename);
          }
        if(mc->isGlobalCam(pagename)) {
          PVarCam* cam = mc->getCam(pagename);
          if(!cam) {
            cam = new PVarCam(this, id, true);
            if(!cam->allocObj()) {
              delete cam;
              continue;
              }
            mc->addCam(cam, pagename);
            }
          else {
            cam->setOwner(this);
            cam->repos(p0);
            }
          cam->Show();
          Cam[numC++] = cam;
          continue;
          }

        PVarCam* cam = new PVarCam(this, id, false);
        if(cam->allocObj())
          Cam[numC++] = cam;
        else
          delete cam;
        }
      }
    }
}
//---------------------------------------------------------------------
template <typename T, typename B, int id, int idInit>
void makeObjT(P_BaseBody* parent, PVect<B*>& set)
{
  LPCTSTR p = parent->getPageString(id);
  if(p) {
    int nObj = _ttoi(p);
    if(nObj) {
      int numC = set.getElem();
      set.setDim(nObj + numC);
      for(int i = 0; i < nObj; ++i) {
        manageObjId moi(0, idInit);
        uint id = moi.calcBaseExtendId(i);
        T* var = new T(parent, id);
        if(var->allocObj())
          set[numC++] = var;
        else
          delete var;
        }
      }
    }
}
//---------------------------------------------------------------------
template <typename T, int id, int idInit>
void makeObjT(P_BaseBody* parent, PVect<T*>& set)
{
  makeObjT<T, T, id, idInit>(parent, set);
}
//---------------------------------------------------------------------
void P_BaseBody::makeCurve()
{
  makeObjT<PVarCurve, ID_VAR_CURVE, ID_INIT_VAR_CURVE>(this, Curve);
}
//---------------------------------------------------------------------
void P_BaseBody::makeEdi()
{
  makeObjT<PVarEdit, ID_VAR_EDI, ID_INIT_VAR_EDI>(this, Edi);
}
//---------------------------------------------------------------------
void P_BaseBody::makeTrend()
{
  makeObjT<PVarTrend, ID_VAR_TREND, ID_INIT_VAR_TREND>(this, oTrend);
}
//---------------------------------------------------------------------
void P_BaseBody::makeSpin()
{
  makeObjT<PVarSpin, ID_VAR_SPIN_UPDN, ID_INIT_VAR_SPIN_UPDN>(this, oSpin);
}
//---------------------------------------------------------------------
void P_BaseBody::makeTableInfo()
{
  makeObjT<pVarTableInfo, ID_VAR_TABLE_INFO, ID_INIT_VAR_TABLE_INFO>(this, oTableInfo);
}
//----------------------------------------------------------------------------
PVarListBox* P_BaseBody::allocOverrideLBox(P_BaseBody* parent, uint style, uint id, const PRect& rect,
              int textlen, HINSTANCE hinst)
{
  return new PVarListBox(this, style, id, rect, textlen, hinst);
}
//----------------------------------------------------------------------------
PVarListBox* P_BaseBody::allocLBox(LPCTSTR p, int id)
{
  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  int style = 0;
  int idFont = ID_INIT_FONT;

  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d"), &x, &y, &w, &h, &style, &idFont);

  x += Offset.x;
  y += Offset.y;

  PRect rect(0, 0, R__X(w), R__Y(h));
  rect.MoveTo(R__X(x), R__Y(y));

  idFont -= ID_INIT_FONT;

  int lbStyle = 0;
  if(style & 1)
    lbStyle = LBS_OWNERDRAWVARIABLE;
  if(style & 2)
    lbStyle |= LBS_SORT;

  PVarListBox* obj = allocOverrideLBox(this, lbStyle, id, rect);

  if(style & 1) {
    manageObjId moi(id, ID_INIT_VAR_LBOX);
    uint id2 = moi.getFirstExtendId();
    p = getPageString(id2);
    if(p) {
      int Rfg = -1;
      int Gfg = -1;
      int Bfg = -1;
      int Rbk = -1;
      int Gbk = -1;
      int Bbk = -1;

      int RfgS = -1;
      int GfgS = -1;
      int BfgS = -1;
      int RbkS = -1;
      int GbkS = -1;
      int BbkS = -1;
      _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
          &Rfg, &Gfg, &Bfg, &Rbk, &Gbk, &Bbk,
          &RfgS, &GfgS, &BfgS, &RbkS, &GbkS, &BbkS);
      if(Rfg != -1) {
        COLORREF fg = RGB(Rfg, Gfg, Bfg);
        COLORREF bg = RGB(Rbk, Gbk, Bbk);
        obj->SetColor(fg, bg);
        if(RfgS != -1)  {
          COLORREF fg = RGB(RfgS, GfgS, BfgS);
          COLORREF bg = RGB(RbkS, GbkS, BbkS);
          obj->SetColorSel(fg, bg);
          }
        }
      }
    }
  if((uint)idFont < Font.getElem())
    obj->setFont(Font[idFont], false);
  return obj;
}
//---------------------------------------------------------------------
void P_BaseBody::makeLBox()
{
  LPCTSTR p = getPageString(ID_VAR_LBOX);
  if(p) {
    int nObj = _ttoi(p);
    if(nObj) {
      LBox.setDim(nObj);
      int numObj = 0;
      for(int i = 0; i < nObj; ++i) {
        p = getPageString(ID_INIT_VAR_LBOX + i);
        if(p) {
          LBox[numObj] = allocLBox(p, ID_INIT_VAR_LBOX + i);
          LBox[numObj++]->create();
          }
        }
      }
    }
}
//---------------------------------------------------------------------
void P_BaseBody::makeBtn()
{
  LPCTSTR p = getPageString(ID_VAR_BTN);
  if(p) {
    int nBtn = _ttoi(p);
    if(nBtn) {
      Btn.setDim(nBtn);
      int j = 0;
      cntGroup* group = 0; // passare &group nella nuova costruzione
      for(int i = 0; i < nBtn; ++i) {
        manageObjId moi(0, ID_INIT_VAR_BTN);
        uint id = moi.calcBaseExtendId(i);
        PVarBtn* var = new PVarBtn(this, id);
        if(var->allocObj(&group))
          Btn[j++] = var;
        else
          delete var;
        }
      }
    }
}
//---------------------------------------------------------------------
void P_BaseBody::makeBitmap()
{
  makeObjT<PVarBmp, ID_VAR_BMP, ID_INIT_VAR_BMP>(this, Bmp);
}
//---------------------------------------------------------------------
void P_BaseBody::makeLed()
{
  makeObjT<PVarLed, ID_VAR_LED, ID_INIT_VAR_LED>(this, Led);
}
//---------------------------------------------------------------------
template <typename T, typename B, int id, int idInit>
void makeSimpleObjT(P_BaseBody* parent, PVect<B*>& set, PVect<B*>& setWithHide)
{
  LPCTSTR p = parent->getPageString(id);
  if(p) {
    int nObj = _ttoi(p);
    if(nObj) {
      for(int i = 0; i < nObj; ++i) {
        manageObjId moi(0, idInit);
        uint id = moi.calcBaseExtendId(i);
        T* var = new T(parent, id);
        if(var->allocObj()) {
          if(var->canHide())
            setWithHide[setWithHide.getElem()] = var;
          else
            set[set.getElem()] = var;
          }
        else
          delete var;
        }
      }
    }
}
//---------------------------------------------------------------------
void P_BaseBody::makeSimple()
{
  makeSimpleBmp();
  makePanel();
  makeSimpleTxt();
}
//---------------------------------------------------------------------
void P_BaseBody::makePanel()
{
  makeSimpleObjT<PSimplePanel, baseSimple, ID_SIMPLE_PANEL, ID_INIT_SIMPLE_PANEL>(this, Simple, SimpleWithHide);
}
//---------------------------------------------------------------------
void P_BaseBody::makeSimpleTxt()
{
  makeSimpleObjT<PSimpleText, baseSimple, ID_SIMPLE_TXT, ID_INIT_SIMPLE_TXT>(this, Simple, SimpleWithHide);
}
//---------------------------------------------------------------------
void P_BaseBody::makeSimpleBmp()
{
  makeSimpleObjT<PSimpleBmp, baseSimple, ID_BMP, ID_INIT_BMP>(this, Simple, SimpleWithHide);
}
//---------------------------------------------------------------------
void P_BaseBody::makeVarTxt()
{
  makeObjT<PVarText, ID_VAR_TXT, ID_INIT_VAR_TXT>(this, Txt);
}
//---------------------------------------------------------------------
void P_BaseBody::makeDiam()
{
  makeObjT<PVarDiam, ID_VAR_DIAM, ID_INIT_VAR_DIAM>(this, Diam);
}
//----------------------------------------------------------------------------
void P_BaseBody::makeChoose()
{
  makeObjT<PVarChoose, ID_VAR_CHOOSE, ID_INIT_VAR_CHOOSE>(this, Choose);
}
//---------------------------------------------------------------------
void P_BaseBody::makeXMeter()
{
  makeObjT<PVarXMeter, ID_VAR_X_METER, ID_INIT_VAR_X_METER>(this, XMeter);
}
//---------------------------------------------------------------------
void P_BaseBody::makeScope()
{
  makeObjT<PVarScope, ID_VAR_SCOPE, ID_INIT_VAR_SCOPE>(this, Scope);
}
//---------------------------------------------------------------------
void P_BaseBody::makePlotXY()
{
  makeObjT<PVarPlotXY, ID_VAR_PLOT_XY, ID_INIT_VAR_PLOT_XY>(this, PlotXY);
}
//---------------------------------------------------------------------
void P_BaseBody::makeSlider()
{
  makeObjT<PVarSlider, ID_VAR_SLIDER, ID_INIT_VAR_SLIDER>(this, Slider);
}
//---------------------------------------------------------------------
void P_BaseBody::makeAlarm()
{
  makeObjT<pVarListAlarm, ID_VAR_ALARM, ID_INIT_VAR_ALARM>(this, oAlarm);
}
//---------------------------------------------------------------------
void P_BaseBody::makeBar()
{
  makeObjT<PVarGraph, ID_BARGRAPH, ID_INIT_BARGRAPH>(this, barGraph);
}
//---------------------------------------------------------------------
bool P_BaseBody::forwardKeyToBtn(DWORD where, HWND hbtn)
{
  int nElem = Btn.getElem();
  for(int i = 0; i < nElem; ++i)
    if(hbtn == Btn[i]->getHwnd())
      return Btn[i]->setFocus((pAround::around)where);
/**/
  nElem = Edi.getElem();
  for(int i = 0; i < nElem; ++i)
    if(hbtn == Edi[i]->getHandle())
      return Edi[i]->setFocus((pAround::around)where);
/**/
  nElem = Choose.getElem();
  for(int i = 0; i < nElem; ++i)
    if(hbtn == Choose[i]->getHwnd())
      return Choose[i]->setFocus((pAround::around)where);

  return false;
}
//---------------------------------------------------------------------
bool P_BaseBody::preProcessMsg(MSG& msg)
{
  pAround::around where = pAround::maxAround;
  switch(msg.message) {
    case WM_KEYDOWN:
      switch(msg.wParam) {
        case VK_LEFT:
          where = pAround::aLeft;
          break;
        case VK_RIGHT:
          where = pAround::aRight;
          break;
        case VK_UP:
          where = pAround::aTop;
          break;
        case VK_DOWN:
          where = pAround::aBottom;
          break;
        }
      break;
    }
  if(where != pAround::maxAround) {
    if(forwardKeyToBtn(where, GetFocus()))
      return true;
    }
  return P_Base::preProcessMsg(msg);
}
//---------------------------------------------------------------------
#define DEF_BIT_4_VISIB 32
//---------------------------------------------------------------------
static void calcAddrAndOffs(uint &addr, int& offsvar, int offsaddr, int offsbit, uint nbitType)
{
  if(!nbitType)
    nbitType = DEF_BIT_4_VISIB;
  addr += offsaddr;
  addr *= nbitType;
  addr += offsvar + offsbit;
  offsvar = addr % nbitType;
  addr /= nbitType;
}
//---------------------------------------------------------------------
static LPCTSTR addOffsetVisibility(LPCTSTR p, int offs, int offsBit, bool noOffsAddr)
{
  LPCTSTR p2 = findNextParamTrim(p, 2);
  if(!p2)
    return p;

  uint prph = 0;
  uint addr = 0;
  uint nbit = 0;
  int offset = 0;
  _stscanf_s(p2, _T("%d,%d,%d,%d"), &prph, &addr, &nbit, &offset);
  if(!prph)
    return p;
  if(PRF_MEMORY == prph && addr <= 1 || PRPH_4_CONST == prph)
    return p;

  TCHAR buff[5000];

  int len = p2 - p;
  // copia prima parte
  copyStr(buff, p, len);
  buff[len] = 0;

  if(offsBit) {
    if(nbit) {
      calcAddrAndOffs(addr, offset, offs, offsBit, DEF_BIT_4_VISIB);
      if(noOffsAddr)
        addr -= offs;
      wsprintf(buff + len, _T("%d,%d,%d,%d,"), prph, addr, nbit, offset);
      p2 = findNextParamTrim(p2, 4);
      if(p2)
        _tcscat_s(buff, SIZE_A(buff), p2);
      }
    else
      offsBit = 0;
    }
  if(!offsBit) {
    p2 = findNextParamTrim(p2, 2);
    if(!p2)
      return p;
    wsprintf(buff + len, _T("%d,%d,"), prph, addr + offs);
    _tcscat_s(buff, SIZE_A(buff), p2);
    }
  delete []p;
  return str_newdup(buff);
}
//---------------------------------------------------------------------
#define CALC_VIS_OFFS(a, o)  (a), (a)+(o)
//---------------------------------------------------------------------
#define STD_INIT_VIS(a) ((a) + ADD_INIT_VAR + ADD_INIT_SECOND)
#define STD_END_VIS(a)  (STD_INIT_VIS(a) + ADD_INIT_SECOND)

#define STD_VIS(a)  STD_INIT_VIS(a), STD_END_VIS(a)

#define STD_VIS_OFFS(a, i, e)  STD_INIT_VIS((a)+(i)), STD_END_VIS((a)+(e))

//---------------------------------------------------------------------
#define CALC_EXT_INIT(a) (((a) - 1) * 10 + OFFS_INIT_VAR_EXT)
//---------------------------------------------------------------------
#define EXT_INIT_VIS(a) ((a) + ADD_INIT_VAR_EXT + ADD_INIT_SECOND_EXT)
#define EXT_END_VIS(a)  (EXT_INIT_VIS(a) + ADD_INIT_THIRD_EXT)

#define EXT_VIS(a)  EXT_INIT_VIS(CALC_EXT_INIT(a)), EXT_END_VIS(CALC_EXT_INIT(a))

#define EXT_VIS_OFFS(a, i, e)  EXT_INIT_VIS((a)+(i)), EXT_END_VIS((a)+(e))

//---------------------------------------------------------------------
#define OFFS_VIS_LED (ADD_INIT_VAR + ADD_INIT_SECOND + ADD_INIT_SECOND)
//---------------------------------------------------------------------
static LPCTSTR addOffsetIf(LPCTSTR p, int id, int offs, setOfString& tmpset, int offsBit, bool alsoVisib, bool noOffsAddr)
{
  if(!offs && !offsBit)
    return p;

  struct visRange {
    int init;
    int end;
    } VR[] = {
    // prima quelli minori di ID_INIT_VAR_TXT

    // zona visibility per barGrph
    { CALC_VIS_OFFS(ID_INIT_BARGRAPH + GRAPH_VISIBILITY_OFFS, GRAPH_VISIBILITY_OFFS) },
    // zona visibility per pannelli
    { STD_VIS(ID_INIT_SIMPLE_PANEL) },

    { STD_VIS(ID_INIT_SIMPLE_TXT) },
    };

  for(uint i = 0; i < SIZE_A(VR); ++i)
    if(VR[i].init <= id && id < VR[i].end)
      return addOffsetVisibility(p, offs, alsoVisib ? offsBit : 0, noOffsAddr);

  // prima dell'inizio dei codici  di variabili
  if(id < ID_INIT_VAR_TXT)
    return p;


  visRange VR2[] = {
    // zona visibility per bitmap semplici
    { CALC_VIS_OFFS(ID_INIT_BMP + ADD_INIT_SIMPLE_BMP, 100) },

    { STD_VIS(ID_INIT_VAR_TXT) },
    { STD_VIS(ID_INIT_VAR_EDI) },

    { STD_VIS(ID_INIT_VAR_BTN) },
    { STD_VIS(ID_INIT_VAR_BMP) },

    { CALC_VIS_OFFS(ID_INIT_VAR_LED + OFFS_VIS_LED, 100) },

    { STD_VIS(ID_INIT_VAR_DIAM) },
    { STD_VIS(ID_INIT_VAR_LBOX) },
    { STD_VIS(ID_INIT_VAR_CHOOSE) },
    { STD_VIS(ID_INIT_VAR_CURVE) },
    { STD_VIS(ID_INIT_VAR_X_METER) },
    { STD_VIS(ID_INIT_VAR_SCOPE) },
    { STD_VIS(ID_INIT_VAR_ALARM) },
    { STD_VIS(ID_INIT_VAR_TREND) },
    { STD_VIS(ID_INIT_VAR_TABLE_INFO) },


    // zona estesa
    { EXT_VIS(ID_INIT_SIMPLE_PANEL) },

    { EXT_VIS(ID_INIT_SIMPLE_TXT) },
    { EXT_VIS(ID_INIT_VAR_TXT) },
    { EXT_VIS(ID_INIT_VAR_EDI) },

    { EXT_VIS(ID_INIT_VAR_BTN) },
    { EXT_VIS(ID_INIT_VAR_BMP) },

    { EXT_VIS(ID_INIT_VAR_DIAM) },
    { EXT_VIS(ID_INIT_VAR_LBOX) },
    { EXT_VIS(ID_INIT_VAR_CHOOSE) },
    { EXT_VIS(ID_INIT_VAR_CURVE) },
    { EXT_VIS(ID_INIT_VAR_X_METER) },
    { EXT_VIS(ID_INIT_VAR_SCOPE) },
    { EXT_VIS(ID_INIT_VAR_ALARM) },
    { EXT_VIS(ID_INIT_VAR_TREND) },
    { EXT_VIS(ID_INIT_VAR_TABLE_INFO) },

    { CALC_VIS_OFFS(ADD_INIT_GRAPH + 1, GRAPH_VISIBILITY_OFFS_EXT) },
    };

  for(uint i = 0; i < SIZE_A(VR2); ++i)
    if(VR2[i].init <= id && id < VR2[i].end)
      return addOffsetVisibility(p, offs, alsoVisib ? offsBit : 0, noOffsAddr);

#if 1
  manageObjId manO(id);
  if(ID_INIT_OBJ_LAST < manO.getBaseId())
    return p;
  manO.adjustId();
  if((int)manO.getId() == id)
    return p;
  if((int)manO.getSecondExtendId() < id)
    return p;

  uint tId = manO.getBaseId();
  // diametri e listbox usano la zona 300 per altri usi
  if(ID_INIT_VAR_DIAM == tId || ID_INIT_VAR_LBOX == tId)
    return p;

  TCHAR buff[5000];
  if(offsBit) {
    LPCTSTR p2 = tmpset.getString(manO.getSecondExtendId());
    uint nbit = 0;
    int offset = 0;
    _stscanf_s(p2, _T("%d,%d"), &nbit, &offset);
    if(nbit) {
      uint prph = 0;
      uint addr = 0;
      uint type = 0;
      _stscanf_s(p, _T("%d,%d,%d"), &prph, &addr, &type);
      if(PRF_MEMORY == prph && addr <= 1 || PRPH_4_CONST == prph)
        return p;
      calcAddrAndOffs(addr, offset, offs, offsBit, prfData::getNBit((prfData::tData)type));
      wsprintf(buff, _T("%d,%d,"), nbit, offset);
      p2 = findNextParam(p2, 2);
      if(p2)
        _tcscat_s(buff, SIZE_A(buff), p2);
      tmpset.replaceString(manO.getSecondExtendId(), str_newdup(buff));
      if(noOffsAddr)
        addr -= offs;
      wsprintf(buff, _T("%d,%d,%d,"), prph, addr, type);
      LPCTSTR p3 = findNextParamTrim(p, 3);
      if(p3)
        _tcscat_s(buff, SIZE_A(buff), p3);
      }
    else
      offsBit = 0;
    }

  if(!offsBit) {
    if((int)manO.getFirstExtendId() > id)
      return p;
    LPCTSTR p2 = findNextParam(p, 1);
    if(!p2)
      return p;

    int len = p2 - p;
    // copia periferica
    copyStr(buff, p, len);
    buff[len] = 0;
    int prph = _ttoi(buff);
    if(!prph)
      return p;

    int addr = _ttoi(p2);
    if(PRF_MEMORY == prph && addr <= 1 || PRPH_4_CONST == prph)
      return p;

    wsprintf(buff + len, _T("%d,"), addr + offs);
    p2 = findNextParam(p2, 1);
    if(p2)
      _tcscat_s(buff, SIZE_A(buff), p2);
    }
  delete []p;
  return str_newdup(buff);
#else
  int tId = id / 1000;

  // diametri e listbox usano la zona 300 per altri usi
  if(ID_INIT_VAR_DIAM / 1000 == tId || ID_INIT_VAR_LBOX / 1000 == tId)
    return p;

  int sub = id % (tId * 1000);

  // nei led i dati della periferica sono all'offset 600-900 anziché 300-600
  if(ID_INIT_VAR_LED / 1000 == tId)
    sub -= ADD_INIT_VAR;

  // è minore della zona x30x (es. 3002 < 3301)
  if(sub <= ADD_INIT_VAR)
    return p;

  // è maggiore della zona x30x (es. 3602 > 3600)
  if(sub > ADD_INIT_VAR + ADD_INIT_SECOND)
    return p;

  LPCTSTR p2 = findNextParam(p, 1);
  if(!p2)
    return p;

  TCHAR buff[5000];
  int len = p2 - p;
  // copia periferica
  copyStr(buff, p, len);
  buff[len] = 0;
  int prf = _ttoi(buff);
  if(!prf)
    return p;

  int addr = _ttoi(p2);
  if(PRF_MEMORY == prf && addr <= 1)
    return p;

  wsprintf(buff + len, _T("%d,"), addr + offs);
  p2 = findNextParam(p2, 1);
  if(p2)
    _tcscat_s(buff, SIZE_A(buff), p2);

  delete []p;
  return str_newdup(buff);
#endif
}
//---------------------------------------------------------------------
void P_BaseBody::fillPageStr(LPCTSTR filename)
{
  pageStrToSet(sStr, filename);

  LPCTSTR p = getPageString(ID_PAGE_DYNAMIC_LINK);
  while(p) {
    int offs = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      break;
    int offsBit = 0;
    bool alsoVisib = false;
    bool noOffsAddr = false;
    do {
      LPCTSTR pbits = getPageString(ID_PAGE_DYNAMIC_LINK_BITS_OFFS);
      if(pbits) {
        offsBit = _ttoi(pbits);
        pbits = findNextParamTrim(pbits);
        if(pbits)
          alsoVisib = toBool(_ttoi(pbits));
          pbits = findNextParamTrim(pbits);
          if(pbits)
            noOffsAddr = toBool(_ttoi(pbits));
        }
      } while(false);
    TCHAR path[_MAX_PATH];
    getFileStrCheckRemote(path, p);

    setOfString tmpSet;
    pageStrToSet(tmpSet, path);
    if(!tmpSet.setFirst())
      break;

    setOfString newSet;
    pageStrToSet(newSet, filename);
    sStr.reset();
    do {
      LPCTSTR t = tmpSet.getCurrString();
      int id = tmpSet.getCurrId();
      tmpSet.replaceString(id, 0, false, false);
      t = addOffsetIf(t, id, offs, tmpSet, offsBit, alsoVisib, noOffsAddr);
      sStr.replaceString(id, (LPTSTR)t, true);
      } while(tmpSet.setFirst());
    // aggiunge/sovrascrive i dati della pagina base con la nuova
    do {
      LPCTSTR t = newSet.getCurrString();
      int id = newSet.getCurrId();
      newSet.replaceString(id, 0, false, false);
      sStr.replaceString(id, (LPTSTR)t, true);
      } while(newSet.setFirst());

    break;
    }

  p = getPageString(ID_PAGE_DYNAMIC_LINK_V2);
  while(p) {
    int offsX = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      break;
    int offsY = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      break;
    TCHAR path[_MAX_PATH];
    getFileStrCheckRemote(path, p);

    setOfString tmpSet;
    pageStrToSet(tmpSet, path);
    if(!tmpSet.setFirst())
      break;
    POINT pt = { offsX, offsY };
    P_ManDynaBody(this, sStr, tmpSet, pt).run();

    break;
    }
}
//---------------------------------------------------------------------
void P_BaseBody::reloadText()
{
  sStr.reset();
  TCHAR path[_MAX_PATH];
  getFileStr(path);
  fillPageStr(path);
  needRefresh = true;
  InvalidateRect(*this, 0, 0);
}
//---------------------------------------------------------------------
static
void
setResolution(setOfString& set)
{
  LPCTSTR p = set.getString(ID_RESOLUTION);
  if(p) {
    switch(_ttoi(p)) {
      case -1:
        sizer::setDefault(sizer::s640x480);
        break;
      case 0:
        sizer::setDefault(sizer::s800x600);
        break;
      case 1:
        sizer::setDefault(sizer::s1024x768);
        break;
      case 2:
        sizer::setDefault(sizer::s1280x1024);
        break;
      case 3:
        sizer::setDefault(sizer::s1440x900);
        break;
      case 4:
        sizer::setDefault(sizer::s1600x1200);
        break;
      case 5:
        sizer::setDefault(sizer::s1680x1050);
        break;
      case 6:
        sizer::setDefault(sizer::s1920x1440);
        break;
      case 100:
        p = findNextParamTrim(p);
        if(p) {
          uint w = _ttoi(p);
          uint h = w / 2;
          p = findNextParamTrim(p);
          if(p)
            h = _ttoi(p);
          SIZE sz = { w, h };
          sizer::setDefault(sizer::sPersonalized, &sz);
          }
        break;
      }
    }
  else
    sizer::setCurrAsDefault();
}
//---------------------------------------------------------------------
void P_BaseBody::setReady(bool first)
{
#if 0
// per test di salvataggio tree
  do {
    TCHAR path[_MAX_PATH];
    getFileStr(path);
    if(!sStr.getNumElem())
      fillPageStr(path);
    sStr.saveTree(path);
    } while(false);
#else
  if(!sStr.getNumElem()) {
    TCHAR path[_MAX_PATH];
    getFileStr(path);
    fillPageStr(path);
    }
#endif
  LPCTSTR p = getPageString(ID_OFFSET_X);
  if(p)
    Offset.x = _ttoi(p);

  p = getPageString(ID_OFFSET_Y);
  if(p)
    Offset.y = _ttoi(p);

  setResolution(sStr);

#if 1
  notShowWhileSimpleSend = true;
#else
  p = getPageString(ID_SHOW_SIMPLE_SEND);
  if(p)
    notShowWhileSimpleSend = _ttoi(p);
#endif
  p = getPageString(ID_INIT_BMP_4_BTN);
  if(p) {
    uint elem = 0;
    do {
      BmpForBtn[elem++] = new PBitmap(p);
      p = getPageString(ID_INIT_BMP_4_BTN + elem);
      } while(p);
    }

  makeBitmap();
  allocFont();
  makeVarTxt();
  makeSimple();
  makeBar();
  makeLed();
  makeDiam();
  makeCam();

  // questa serve per evitare che il focus vada sul footer
  SetWindowPos(*this, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

  makeCurve();
  makeEdi();
  makeLBox();
  makeChoose();
  makeBtn();
  makeXMeter();
  makeScope();
  makeAlarm();
  makeTrend();
  makeSpin();
  makeTableInfo();
  makePlotXY();
  makeSlider();

  pVActive bAll;
  int nElem = Edi.getElem();
  int totElem = nElem;
  totElem += Btn.getElem();
  totElem += LBox.getElem();
  totElem += Choose.getElem();
  totElem += oAlarm.getElem();

  bAll.setDim(totElem);
  int i;
  for(i = 0; i < nElem; ++i)
    bAll[i] = Edi[i];

  nElem = Btn.getElem();
  for(int j = 0; j < nElem; ++i, ++j)
    bAll[i] = Btn[j];

  nElem = LBox.getElem();
  for(int j = 0; j < nElem; ++i, ++j) {
    LBox[j]->setIntegralHeight();
    bAll[i] = LBox[j];
    }

  nElem = Choose.getElem();
  for(int j = 0; j < nElem; ++i, ++j)
    bAll[i] = Choose[j];

  nElem = oAlarm.getElem();
  for(int j = 0; j < nElem; ++i, ++j)
    bAll[i] = oAlarm[j];
/*
  nElem = oTrend.getElem();
  for(int j = 0; j < nElem; ++i, ++j)
    bAll[i] = oTrend[j];
*/
#if 1
  PVect <wrapObjX> wrapX;
  wrapX.setDim(totElem);
  PVect <wrapObjY> wrapY;
  wrapY.setDim(totElem);
  for(i = 0; i < totElem; ++i) {
    wrapX[i] = wrapObjX(bAll[i]);
    wrapY[i] = wrapObjY(bAll[i]);
    }
  wrapX.sort();
  wrapY.sort();

  for(i = 0; i < totElem; ++i)
    bAll[i]->findAround(wrapX, wrapY);
#else
  for(i = 0; i < totElem; ++i)
    bAll[i]->findAround(bAll);
#endif
  SetActiveWindow(*this);

  if(!first) {
    isReady = true;
    refresh();
    }
  makeListReqRead();

  if(!isReady) {
    needRefresh = true;
    isReady = true;
    }
}
//----------------------------------------------------------------------------
void P_BaseBody::request()
{
  makeListReqRead();
}
//-----------------------------------------------------------------
static void insertOnList(PVect<DWORD>& set, uint val)
{
  uint n = set.getElem();
  int pos = n;
  for(uint i = 0; i < n; ++i) {
    if(val == set[i])
      return;
    if(val < set[i]) {
      pos = i;
      break;
      }
    }
  if(pos == n)
    set[pos] = val;
  else
    set.insert(val, pos);
}
//----------------------------------------------------------------------------
void P_BaseBody::makeListReqRead(uint idPrph, PVect<DWORD>& set, const infoVarReq& ir)
{
  LPCTSTR p = getPageString(ir.idBase);
  if(p) {
    int n = _ttoi(p);
    if(n) {
      for(int i = 0; i < n; ++i) {
        if(ir.idList) {
          p = getPageString(ir.idList + i);
          if(p) {
            int prph = 0;
            int addr = 0;
            _stscanf_s(p, _T("%d,%d"),  &prph, &addr);
            if(prph == idPrph)
              insertOnList(set, addr);
            }
          }
        if(ir.offs) {
          p = getPageString(ir.idList + i + ir.offs);
          if(p) {
            int t1;
            int t2;
            int prph = 0;
            int addr = 0;
            _stscanf_s(p, _T("%d,%d,%d,%d"),  &t1, &t2, &prph, &addr);
            if(prph == idPrph)
              insertOnList(set, addr);
            }
          }
        }
      }
    }
}
//---------------------------------------------------------------------
#define ADD_1(a) ((a) + ADD_INIT_VAR)
#define ADD_2(a) ((a) + ADD_INIT_VAR + ADD_INIT_SECOND)
//---------------------------------------------------------------------
void P_BaseBody::addCamToReq(uint idPrph, PVect<DWORD>& set)
{
  LPCTSTR p = getPageString(ID_VAR_CAM);
  if(p) {
    int n = _ttoi(p);
    if(n) {
      for(int i = 0; i < n; ++i) {
        p = getPageString(ID_INIT_CAM + i);
        p = findNextParam(p, 4);
        if(!p)
          continue;
        int prph = 0;
        int addr = 0;
        _stscanf_s(p, _T("%d,%d"),  &prph, &addr);
        if(prph == idPrph) {
          for(uint j = 0; j < 5; ++j)
            insertOnList(set, addr + j);
          }
        }
      }
    }
}
//---------------------------------------------------------------------
void P_BaseBody::makeListReqRead(uint idPrph, PVect<DWORD>& set)
{
  // quelli che hanno zero come secondo parametro non usano variabili dirette (o usano le edit), solo quelle per la visibilità
  // i bargraph hanno un offset di 200 per la visibilità e non usano variabili dirette
  // i led hanno un offset aggiuntivo perché il primo offset è usato per i bitmap dei numeri
  // le listbox sono solo segnaposto
  // le cam hanno una gestione diversa e devono essere implementate a parte
  // le scope hanno la variabile diretta con lettura a richiesta singola quindi il secondo parametro deve essere
  //   messo a zero, usano poi le edit per le altre variabili
  infoVarReq ivr[] = {
      { ID_BMP, 0, ID_INIT_BMP + ADD_INIT_SIMPLE_BMP + 1 },
      { ID_BARGRAPH, 0, ID_INIT_BARGRAPH + 200 },
      { ID_SIMPLE_PANEL, 0, ADD_2(ID_INIT_SIMPLE_PANEL) },
      { ID_SIMPLE_TXT, 0, ADD_2(ID_INIT_SIMPLE_TXT) },
      { ID_VAR_TXT, ADD_1(ID_INIT_VAR_TXT), ADD_2(ID_INIT_VAR_TXT) },
      { ID_VAR_EDI, ADD_1(ID_INIT_VAR_EDI), ADD_2(ID_INIT_VAR_EDI) },
      { ID_VAR_BTN, ADD_1(ID_INIT_VAR_BTN), ADD_2(ID_INIT_VAR_BTN) },
      { ID_VAR_BMP, ADD_1(ID_INIT_VAR_BMP), ADD_2(ID_INIT_VAR_BMP) },

      { ID_VAR_LED, ADD_2(ID_INIT_VAR_LED), ADD_2(ADD_1(ID_INIT_VAR_LED)) },

      { ID_VAR_DIAM, 0, ADD_2(ID_INIT_VAR_DIAM) },
//    { ID_VAR_LBOX

      { ID_VAR_CHOOSE, ADD_1(ID_INIT_VAR_CHOOSE), ADD_2(ID_INIT_VAR_CHOOSE) },
      { ID_VAR_CURVE, ADD_1(ID_INIT_VAR_CURVE), ADD_2(ID_INIT_VAR_CURVE) },

      { ID_VAR_X_METER, ADD_1(ID_INIT_VAR_X_METER), ADD_2(ID_INIT_VAR_X_METER) },
      { ID_VAR_SCOPE, 0, ADD_2(ID_INIT_VAR_X_METER) },
    };
//    { ID_VAR_CAM

  for(uint i = 0; i < SIZE_A(ivr); ++i)
    makeListReqRead(idPrph, set, ivr[i]);

  addCamToReq(idPrph, set);
}
//---------------------------------------------------------------------
void P_BaseBody::makeListReqRead()
{
// non è stata mai usata, poi nel siemens non ha senso
// sembra che sia stata usata e ora tocca rimetterla per compatibilità
  if(isWin95() || isWin98orLater())
    return;

  PVect<bool> sPrph;
  mainClient* mc = getMain();

  if(!mc->getListPrph(sPrph))
    return;
  uint n = sPrph.getElem();
#if 0
  for(uint i = 0; i < n; ++i) {
    if(sPrph[i]) {
      PVect<DWORD> set;
      makeListReqRead(i + WM_PLC, set);
      mc->addToMappedFile(i + WM_PLC, set, false);
      }
    }
}
#else
/*
  modificare il codice passando il set di periferiche ed un set di oggetti a bit,
  ogni oggetto attiva il bit corrispondente all'indirizzo nel set relativo alla
  periferica (se è attiva).

  Forse è meglio passare solo il set di oggetti a bit, non vale la pena verificare
  per ogni oggetto se la periferica corrispondente è attiva, ma si verifica solo che
  sia nel range di periferiche reali -> range(WM_PLC, WM_PLC + MAX_PERIF - 1)
  poi alla fine si ricostruisce il set di DWORD solo per le periferiche attiva da
  passare al mainClient

  se si usa una cache, è meglio implementarla nel mainClient, salvando il set
  che viene inviato tramite la addToMappedFile(). Il mainClient verifica che il
  file non sia più recente della cache, in caso positivo, richiama questo metodo
  altrimenti usa la cache.

  Per non modificare troppe cose, la cache è meglio implementarla qui.
  All'entrata del metodo si verifica la necessità di ricalcolare e si eseguono
  le procedure relative
  if(needReload()) {
    quello sotto
    }
  else {
    apre il file che ha il seguente formato:
      prima dword -> periferica
      seconda dword -> numero di addr che seguono
      ...
      num + 2 + 1 -> periferica
      ...

      il nome del file è uguale alla pagina con l'aggiunta di .cache come estensione
      e viene creato nella sottocartella 'cache' di 'system'
    }
*/
  TCHAR cacheFile[_MAX_PATH];
  bool isValidCache = getCacheFile(cacheFile, SIZE_A(cacheFile));
  // se ci sono alcuni elementi dell'oggetto scope occorre verificare se
  // legge un solo dato e allora si può mettere in cache o se invece il numero
  // dipende da una variabile e allora si rilegge ogni volta (occorre avvisare
  // che se si cambia il numero di elementi bisogna uscire e rientrare nella
  // pagina, se le variabili sono a richiesta)
  if(Scope.getElem()) {
    uint nElem = Scope.getElem();
    for(uint i = 0; i < nElem; ++i) {
      if(!Scope[i]->canCacheReq(sPrph)) {
        isValidCache = false;
        break;
        }
      }
    }
  while(isValidCache) {
    // la pagina non fa uso di cache, è l'header, non deve nemmeno usare
    // variabili a richiesta visto che è sempre presente
    if(!*cacheFile)
      return;
    P_File pf(cacheFile, P_READ_ONLY);
    if(!pf.P_open())
      break;
    uint dim = (uint)pf.get_len();
    P_SmartPointer<LPBYTE> buff(new BYTE[dim], true);
    pf.P_read(buff, dim);
    LPDWORD pdw = (LPDWORD)(LPBYTE)buff;
    dim >>= 2;
    while(dim > 0) {
      DWORD prph = *pdw++;
      // per un uso futuro ... possiamo aggiungerci qualcos'altro, basta
      // aggiungerci un -1 per separare questi dati dal resto
      if((DWORD)-1 == prph)
        break;
      DWORD nAddr = *pdw++;
      PVect<DWORD> set;
      set.setDim(nAddr);
      for(uint i = 0; i < nAddr; ++i)
        set[i] = *pdw++;
      mc->addToMappedFile(prph + WM_PLC, set, false);
      dim -= nAddr + 2;
      }
    return;
    }
  P_File pf(cacheFile, P_CREAT);
  bool opened = pf.P_open();
  PVect<P_Bits*> allBits;
  allBits.setDim(MAX_PERIF);
  for(uint i = 0; i < MAX_PERIF; ++i)
    allBits[i] = new P_Bits(MAX_DWORD_PERIF);
  makeListReqRead(allBits);

  for(uint i = 0; i < n; ++i) {
    if(sPrph[i]) {
      if(!allBits[i]-> hasSet())
        continue;
      PVect<DWORD> set;
      makeListReqRead(set, *(allBits[i]));
      mc->addToMappedFile(i + WM_PLC, set, false);
      if(opened) {
        DWORD v = i;
        pf.P_write(&v, sizeof(v));
        v = set.getElem();
        pf.P_write(&v, sizeof(v));
        DWORD nElem = v;
        for(uint j = 0; j < nElem; ++j) {
          v = set[j];
          pf.P_write(&v, sizeof(v));
          }
        }
      }
    }
  for(uint i = 0; i < MAX_PERIF; ++i)
    delete allBits[i];
}
//---------------------------------------------------------------------
void P_BaseBody::makeListReqRead(PVect<DWORD>& set, P_Bits& bits)
{
  static const DWORD bitOfDWord = (sizeof(DWORD) * 8);
  DWORD n = bits.totSet();
  set.setDim(n);

  DWORD ndw = bits.getNumDWORD();
  DWORD offs = 0;
  for(uint j = 0; j < ndw && n; ++j, offs += bitOfDWord) {
    if(bits.isDWordSet(j)) {
      for(uint i = 0; i < bitOfDWord && n; ++i) {
        if(bits.isSet(offs + i)) {
          set[set.getElem()] = offs + i;
          --n;
          }
        }
      }
    }
}
//---------------------------------------------------------------------
template <typename T>
void appendObj(PVect<baseObj*>& allObj, const PVect<T*>& obj)
{
  uint nElem = obj.getElem();
  uint curr = allObj.getElem();
  for(uint i = 0; i < nElem; ++i, ++curr)
    allObj[curr] = obj[i];
}
//---------------------------------------------------------------------
void P_BaseBody::makeListReqRead(PVect<P_Bits*>& allBits)
{
  PVect<baseObj*> allObj;
  uint nElem = Txt.getElem();
  nElem += Edi.getElem();
  nElem += Btn.getElem();
  nElem += Diam.getElem();
  nElem += LBox.getElem();
  nElem += Choose.getElem();
  nElem += Curve.getElem();
  nElem += Cam.getElem();
  nElem += XMeter.getElem();
  nElem += SimpleWithHide.getElem();
  nElem += Bmp.getElem();
  nElem += Led.getElem();
  nElem += barGraph.getElem();
  nElem += Scope.getElem();
  nElem += oAlarm.getElem();
  nElem += oTrend.getElem();
  nElem += oSpin.getElem();
  nElem += oTableInfo.getElem();

  allObj.setDim(nElem);
  appendObj(allObj, Txt);
  appendObj(allObj, Edi);
  appendObj(allObj, Btn);
  appendObj(allObj, Diam);
  appendObj(allObj, LBox);
  appendObj(allObj, Choose);
  appendObj(allObj, Curve);
  appendObj(allObj, Cam);
  appendObj(allObj, XMeter);
  appendObj(allObj, SimpleWithHide);
  appendObj(allObj, Bmp);
  appendObj(allObj, Led);
  appendObj(allObj, barGraph);
  appendObj(allObj, Scope);
  appendObj(allObj, oAlarm);
  appendObj(allObj, oTrend);
  appendObj(allObj, oSpin);
  appendObj(allObj, oTableInfo);
  for(uint i = 0; i < nElem; ++i)
    allObj[i]->addReqVar(allBits);
}
#endif
//---------------------------------------------------------------------
void P_BaseBody::requestOneRead(uint prph, uint startAddr, uint num)
{
  if(isWin95() || isWin98orLater())
    return;

  PVect<bool> sPrph;
  mainClient* mc = getMain();

  if(!mc->getListPrph(sPrph))
    return;
  uint n = sPrph.getElem();
  prph -= WM_PLC;
  if(prph >= n || !sPrph[prph])
    return;
  PVect<DWORD> set;
  set.setDim(num);
  for(uint i = 0; i < num; ++i)
    set[i] = startAddr++;
  mc->addToMappedFile(prph + WM_PLC, set, true);
}
//---------------------------------------------------------------------
genericPerif* P_BaseBody::getGenPerif(uint Prph)
{
  mainClient* mc = getMain();
  return mc->getGenPerif(Prph);
}
//---------------------------------------------------------------------
template <typename T>
void refreshObj(T& obj, int force)
{
  uint nObj = obj.getElem();
  for(uint i = 0; i < nObj; ++i)
    obj[i]->update(toBool(force));
}
//---------------------------------------------------------------------
static void refreshCam(PVect<PVarCam*>& cam, int force)
{
  uint nObj = cam.getElem();
  for(uint i = 0; i < nObj; ++i)
    if(!cam[i]->isCamGlobal())
      cam[i]->update(toBool(force));
}
//---------------------------------------------------------------------
#define FORCE_REFRESH_COUNT 10
#define TIME_TO_FORCE_REFRESH (1000 * 1)
#define MIN_REFRESH_COUNT 3
//---------------------------------------------------------------------
void P_BaseBody::refresh()
{
  if(!isReady)
    return;
  static uint forceRefreshCount = 0;
  static uint forceRefresh = 0;
  if(!forceRefreshCount) {
    LPCTSTR p = getString(ID_TIMER_APP);
    if(p) {
      uint tick = _ttoi(p);
      if(!tick)
        tick = 200;
      forceRefreshCount = TIME_TO_FORCE_REFRESH / tick;
      if(forceRefreshCount < MIN_REFRESH_COUNT)
        forceRefreshCount = MIN_REFRESH_COUNT;
      }
    else
      forceRefreshCount = FORCE_REFRESH_COUNT;
    }
  bool trueRefresh = toBool(needRefresh);
  if(++forceRefresh >= forceRefreshCount) {
    trueRefresh = true;
    forceRefresh = 0;
    }
  if(timeoutEdit > 0) {
    --timeoutEdit;
    if(!timeoutEdit)
      needRefresh = true;
    }
  refreshObj(Btn, needRefresh);
  refreshObj(Choose, trueRefresh);
  refreshObj(oSpin, needRefresh);

#if 1
  HWND hwFocus = GetFocus();
  int nEdi = Edi.getElem();
  for(int i = 0; i < nEdi; ++i) {
    if(hwFocus == *Edi[i] && !mustResetEdit())
      continue;
    Edi[i]->update(toBool(trueRefresh));
    }
#else
  refreshObj(Edi, trueRefresh);
#endif

#if 0
  uint nObj = Simple.getElem();
  bool repaint = false;
  for(uint i = 0; i < nObj; ++i)
    repaint |= Simple[i]->update(toBool(needRefresh));
  if(repaint)
 #if 1
    setDirtyBkg();
 #else
    removeAllBmpWork(2);
 #endif
#endif
  refreshObj(SimpleWithHide, needRefresh);
  refreshObj(Txt, trueRefresh);
  refreshObj(Led, needRefresh);
  refreshObj(barGraph, needRefresh);
//  refreshCam(Cam, needRefresh);
  refreshObj(Cam, needRefresh);

  refreshObj(Bmp, needRefresh);
  refreshObj(Diam, needRefresh);
#ifdef ALWAYS_RECALC_GDI_XMETER
  refreshObj(XMeter, needRefresh);
#else
  // l'oggetto memorizza in un buffer la posizione e lo sfondo attuale. Riusa il buffer
  // se non sono cambiati i dati e se non è stato cambiato lo sfondo.
  // è solo una piccola ottimizzazione per evitare di ricalcolare quando viene richiesto
  // il repaint magari per una finestra che si muove (o chiude) sullo schermo
//  refreshObj(XMeter, needRefresh || repaint);
  refreshObj(XMeter, needRefresh);
#endif
  refreshObj(Scope, needRefresh);
  refreshObj(PlotXY, needRefresh);
  refreshObj(oAlarm, needRefresh);
  refreshObj(oTrend, needRefresh);
  refreshObj(oTableInfo, needRefresh);
  refreshObj(Curve, needRefresh);
  refreshObj(Slider, needRefresh);
}
//---------------------------------------------------------------------
HFONT P_BaseBody::getFont(uint id)
{
  uint nElem = Font.getElem();
  if(id >= nElem)
    id = 0;
  return Font[id];
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
template <typename T>
void paintObj(T& obj, HDC hdc, const PRect& rect)
{
  uint nObj = obj.getElem();
  for(uint i = 0; i < nObj; ++i)
    obj[i]->draw(hdc, rect);
}
//----------------------------------------------------------------------------
void P_BaseBody::evBkgPaint(HDC hdc, const PRect& rect)
{
  if(!hBmpBkgWork) {
    allocBmpBkgWork(hdc);
    PRect r;
    GetClientRect(*this, r);
    paintObj(Simple, mdcBkgWork, r);
    DirtyBkg = false;
    }
  else if(isDirtyBkg()) {
    DirtyBkg = false;
    PRect r;
    GetClientRect(*this, r);
    HBRUSH br = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    FillRect(mdcBkgWork, r, br);
    paintObj(Simple, mdcBkgWork, r);
    }
  BitBlt(hdc, rect.left, rect.top, rect.Width(), rect.Height(), mdcBkgWork, rect.left, rect.top, SRCCOPY);
}
//----------------------------------------------------------------------------
void P_BaseBody::evPaintSimple(HDC hdc, const PRect& rect)
{
  paintObj(Simple, hdc, rect);
}
//----------------------------------------------------------------------------
void P_BaseBody::evPaint2(HDC hdc, const PRect& rect)
{
  paintObj(SimpleWithHide, hdc, rect);
  paintObj(oTableInfo, hdc, rect);

  paintObj(Bmp, hdc, rect);
  paintObj(Diam, hdc, rect);
  paintObj(XMeter, hdc, rect);
  paintObj(Scope, hdc, rect);
  paintObj(PlotXY, hdc, rect);
  paintObj(Txt, hdc, rect);
  paintObj(barGraph, hdc, rect);
  paintObj(Led, hdc, rect);
  paintObj(Slider, hdc, rect);


  HWND hw = GetFocus();
  uint nElem = LBox.getElem();
  for(uint i = 0; i < nElem; ++i)
    handleLBFocus(*LBox[i], *LBox[i] == hw, hdc);
  nElem = oAlarm.getElem();
  for(uint i = 0; i < nElem; ++i)
    handleLBFocus(*oAlarm[i], *oAlarm[i] == hw, hdc);
}
//----------------------------------------------------------------------------
void P_BaseBody::evPaint(HDC hdc, const PRect& rect)
{
  evBkgPaint(hdc, rect);
  evPaint2(hdc, rect);
}
//----------------------------------------------------------------------------
void P_BaseBody::actionChoose(uint idBtn)
{
  // se non viene trovato nei pulsanti allocati esce
  uint nElem = Choose.getElem();
  PVarChoose* p_Choose = 0;
  for(uint i = 0; i < nElem; ++i) {
    PVarChoose* pChoose = Choose[i];
    if(pChoose->getId() == idBtn) {
      p_Choose = pChoose;
      break;
      }
    }
  if(!p_Choose)
    return;

  prfData data;
  int idprf = p_Choose->action(data);
  if(!idprf)
    return;

  if(!preSendBtn(p_Choose, data))
    return;

  sendData(data, idprf);
}
//----------------------------------------------------------------------------
#define OFFS_PSW_OBJ 7
//----------------------------------------------------------------------------
bool P_BaseBody::checkObjectPsw(uint id, bool* changed)
{
  id = manageObjId(id).getSecondExtendId();

  LPCTSTR p = getPageString(id);
//  LPCTSTR p = getPageString(id + ADD_INIT_VAR + ADD_INIT_SECOND);
  if(!p)
    return true;
  p = findNextParam(p, OFFS_PSW_OBJ);
  if(!p)
    return true;

  int pswLevel = _ttoi(p);
  bool resetPsw = false;
  if(pswLevel < 0) {
    pswLevel = -pswLevel;
    resetPsw = true;
    }
  else if(pswLevel > 3 && ONLY_CONFIRM != pswLevel)
    pswLevel = 3;
  PassWord psw;

  bool success = false;
  success = pswLevel == psw.getPsW(pswLevel, this);
  if(success && changed)
    *changed = psw.isChanged();
  if(resetPsw)
    psw.restartTime(0);
  return success;
}
//----------------------------------------------------------------------------
void P_BaseBody::actionBtn(uint idBtn, bool toggle)
{
  uint id = manageObjId(idBtn).getBaseId();
  if(id != ID_INIT_VAR_BTN && id != ID_INIT_VAR_CHOOSE) {
    idBtn += OFFS_INIT_VAR_EXT;
    id = manageObjId(idBtn).getBaseId();
    }
  // se è nel range dei pulsanti di scelta elabora ed esce
  if(ID_INIT_VAR_CHOOSE == id) {
    actionChoose(idBtn);
    return;
    }
  // se non è nel range dei pulsanti esce
  if(ID_INIT_VAR_BTN != id)
    return;

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
  bool old;
  if(p_Btn->isSetOnPress())
    old = setDisableWindowByTranfer(false);
  do {
    if(!preActionBtn(p_Btn))
      break;

    prfData data;
    if(!p_Btn->makeToSend(data, toggle))
      break;
    if(!preSendBtn(p_Btn, data))
        break;

    sendData(data, p_Btn->getIdPrph());
    postSendBtn(p_Btn, data);
    } while(false);
  if(p_Btn->isSetOnPress())
    setDisableWindowByTranfer(old);
}
//----------------------------------------------------------------------------
void P_BaseBody::actionSpin(uint idSpin, int up)
{
  uint nElem = oSpin.getElem();
  for(uint i = 0; i < nElem; ++i) {
    if(oSpin[i]->getIdSpin() == idSpin) {
      oSpin[i]->action(up);
      break;
      }
    }
}
//---------------------------------------------------------------------
bool P_BaseBody::sendData(const prfData& data, uint idPrf)
{
  prfData::tResultData result = prfData::invalidPerif;
  if(PRF_MEMORY == idPrf)
    result = Par->getData()->set(data, true);
  else {
    gestPerif* perif = Par->getPerif(idPrf);
    if(perif) {
      gestPerif::howSend old = perif->setAutoShow(showWhileSend() ? gestPerif::ALL_AND_SHOW : gestPerif::CHECK_ALL_ASYNC);
      result = perif->set(data);
      perif->setAutoShow(old);
      }
    }

  if(prfData::failed >= result) {
    ShowErrorData(idPrf, data, result);
    return false;
    }
  return true;
}
//---------------------------------------------------------------------
bool P_BaseBody::getData(prfData& target, PVarEdit* edi, int prph)
{
  return edi->getData(target, prph);
}
//----------------------------------------------------------------------------
void P_BaseBody::removeFromClipped(const PRect& r)
{
  if(hBmpWork) {
    HRGN rgn = CreateRectRgn(r.left, r.top, r.right, r.bottom);
    ExtSelectClipRgn(mdcWork, rgn, RGN_OR);
    DeleteObject(rgn);
    }
}
//----------------------------------------------------------------------------
bool P_BaseBody::addToClipped(const PRect& r)
{
  if(hBmpWork) {
    ExcludeClipRect(mdcWork, r.left, r.top, r.right, r.bottom);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
#define DEF_COLOR_BKG GetSysColor(COLOR_BTNFACE)
//----------------------------------------------------------------------------
LRESULT P_BaseBody::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_DESTROY:
      flushAll();
      break;
    case WM_SIZE:
      removeAllBmpWork(3);
      break;

    case WM_MY_VAR_LVIEW_ALARM:
      InvalidateRect((HWND)lParam, 0, 0);
      break;
    case WM_NOTIFY:
      do {
        PWin* w = PWin::getWindowPtr(((LPNMHDR) lParam)->hwndFrom);
        PCustomListView* lv = dynamic_cast<PCustomListView*>(w);
        if(lv) {
          switch(((LPNMHDR)lParam)->code) {
            case LVN_ENDSCROLL:
              PostMessage(*this, WM_MY_VAR_LVIEW_ALARM, 0, (LPARAM)(((LPNMHDR) lParam)->hwndFrom));
              break;
            case NM_CUSTOMDRAW:
              do {
                LRESULT res;
                if(lv->ProcessNotify(lParam, res))
                  return res;
                } while(false);
              break;
            case NM_KILLFOCUS:
              handleLBFocus(*w, false, 0);
              break;
            case NM_SETFOCUS:
              handleLBFocus(*w, true, 0);
              break;
            }
          }
        } while(false);
      break;

    case WM_COMMAND:
      switch(HIWORD(wParam)) {
        case EN_CHANGE:
          initTimeEdit();
          break;
        case BN_CLICKED:
          actionBtn(LOWORD(wParam), false);
          break;

        case LBN_KILLFOCUS:
          handleLBFocus((HWND)lParam, false, 0);
          break;
        case LBN_SETFOCUS:
          handleLBFocus((HWND)lParam, true, 0);
          break;

        case LBN_SELCHANGE:
        case LBN_DBLCLK:
          handleLBChange(hwnd, message, wParam, lParam);
          break;
        }
      break;
#define USE_MEMDC
#ifdef USE_MEMDC

    case WM_ERASEBKGND:
      if(isReady && wParam) {
        PRect r;
        GetClientRect(hwnd, r);
        if(!hBmpWork) {
          HDC dc = GetDC(*this);
          allocBmpWork(dc);
          ReleaseDC(*this, dc);
          }
        HBRUSH br = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
        FillRect(mdcWork, r, br);
        evBkgPaint(mdcWork, r);
        }
      return 1;
#endif
    case WM_PAINT:
      if(GetUpdateRect(hwnd, 0, 0)) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        if(!hdc) {
          EndPaint(hwnd, &ps);
          break;
          }

#ifdef USE_MEMDC
        if(isReady) {
          if(!hBmpWork)
            allocBmpWork(hdc);

          PRect r(ps.rcPaint);
          if(r.Width() && r.Height()) {
            evPaint(mdcWork, r);
            HRGN hrgn = CreateRectRgn(0, 0, 1, 1);
            int res = GetClipRgn(mdcWork, hrgn);
            if(res > 0)
              SelectClipRgn(hdc, hrgn);
            BitBlt(hdc, r.left, r.top, r.Width(), r.Height(), mdcWork, r.left, r.top, SRCCOPY);
            DeleteObject(hrgn);
            }
          }
#else
        evPaint(hdc, PRect(ps.rcPaint));
#endif
        EndPaint(hwnd, &ps);
        }
      return 0;
    }
  return P_Base::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void P_BaseBody::handleLBChange(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  uint nElem = LBox.getElem();
  for(uint i = 0; i < nElem; ++i) {
    if(LBox[i]->getHandle() == (HWND)lParam) {
      LBox[i]->executeWindowProc(hwnd, message, wParam, lParam);
      break;
      }
    }
}
//----------------------------------------------------------------------------
void P_BaseBody::handleLBFocus(HWND hwndCtrl, bool set, HDC hDc)
{
  if(!IsWindowEnabled(hwndCtrl))
    return;
  PWin* w = PWin::getWindowPtr(hwndCtrl);
  baseActive* ba = 0;
  do {
    PVarListBox* lb = dynamic_cast<PVarListBox*>(w);
    if(lb) {
      ba = dynamic_cast<baseActive*>(lb);
      break;
      }
    PVarListView* lv = dynamic_cast<PVarListView*>(w);
    if(lv) {
      ba = dynamic_cast<baseActive*>(lv);
      break;
      }
    return;
    } while(false);

  if(!ba->isVisible())
   return;
  PRect r;
  GetWindowRect(hwndCtrl, r);
  r.Inflate(R__X(2), R__Y(2));
  MapWindowPoints(HWND_DESKTOP, *this, (LPPOINT)(LPRECT)r, 2);

  HDC hdc = hDc;
  if(!hdc)
    hdc = GetDC(*this);
  HPEN hpen = CreatePen(PS_SOLID, R__X(2), set ? RGB(0xff, 0, 0) : RGB(0xff, 0xff, 0xcf));
  HGDIOBJ old = SelectObject(hdc, hpen);
  POINT pt[] = {
    { r.left, r.top },
    { r.right, r.top },
    { r.right, r.bottom },
    { r.left, r.bottom },
    { r.left, r.top },
    };
  Polyline(hdc, pt, SIZE_A(pt));
  DeleteObject(SelectObject(hdc, old));
  if(!hDc)
    ReleaseDC(*this, hdc);
}
//----------------------------------------------------------------------------
void P_BaseBody::allocFont()
{
  int nFont;
  for(nFont = 0; nFont < MAX_FONT; ++nFont) {
    LPCTSTR p = getPageString(ID_INIT_FONT + nFont);
    if(!p)
      break;
    }
  Font.setDim(nFont);
  for(int i = 0; i < nFont; ++i) {
    LPCTSTR p = getPageString(ID_INIT_FONT + i);
    int h = 16;
    int w = 0;
    int flag = 0;
    _stscanf_s(p, _T("%d,%d,%d"), &h, &w, &flag);
    LPCTSTR t = p + _tcslen(p) - 1;
    while(t != p) {
      if(_T(',') == *t) {
        ++t;
        break;
        }
      --t;
      }
    Font[i] = D_FONT(R__Y(h), R__X(w), flag, t);
    }
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
LPCTSTR insertIdBmp(LPCTSTR p, PVect<uint>& target, int numBmp)
{
  if(!p)
    return 0;
  if(!numBmp)
    return p;
  int elem = 0;
  do {
    int id = _ttoi(p) - ID_INIT_BMP_4_BTN;
    if(id < 0)
      break;
    target[elem++] = id;
    p = findNextParamTrim(p);
    } while(p && (numBmp < 0 || elem < numBmp));
  return p;
}
//----------------------------------------------------------------------------
LPCTSTR insertId(LPCTSTR p, PVect<uint>& target, int num)
{
  if(!p)
    return 0;
  int elem = 0;
  do {
    int id = _ttoi(p);
    if(id < 0)
      break;
    target[elem++] = id;
    p = findNextParamTrim(p);
    } while(p && elem < num);
  return p;
}
//----------------------------------------------------------------------------
LPCTSTR getColor(LPCTSTR p, PVect<COLORREF>& col, int num)
{
  if(!p)
    return 0;
  if(num <= 0)
    num = 1000;
  else
    col.setDim(num);
  for(int j = 0; j < num && p; ++j) {
    int color[3] = { 0, 0, 0};
    for(int i = 0; p && i < SIZE_A(color); ++i) {
      color[i] = _ttoi(p);
      p = findNextParamTrim(p);
      }
    if(color[0] < 0)
      col[j] = -1;
    else
      col[j] = RGB(color[0], color[1], color[2]);
    }
  return p;
}
//----------------------------------------------------------------------------
LPCTSTR getFont(LPCTSTR p, uint& id)
{
  if(!p)
    return 0;
  id = _ttoi(p);
  return findNextParamTrim(p);
}
//----------------------------------------------------------------------------
PVarText* P_BaseBody::getVarFromId(uint id)
{
  int nVar = Txt.getElem();
  for(int i = 0; i < nVar; ++i)
    if(Txt[i]->getId() == id)
      return Txt[i];
  return 0;
}
//---------------------------------------------------------------------
PVarEdit* P_BaseBody::getEditFromId(uint id)
{
  int nEdi = Edi.getElem();
  for(int i = 0; i < nEdi; ++i)
    if(Edi[i]->Attr.id == id)
      return Edi[i];
  return 0;
}
//---------------------------------------------------------------------
PVarListBox* P_BaseBody::getLBoxFromId(uint id)
{
  int nElem = LBox.getElem();
  for(int i = 0; i < nElem; ++i)
    if(LBox[i]->Attr.id == id)
      return LBox[i];
  return 0;
}
//---------------------------------------------------------------------
smartPointerConstString P_BaseBody::getPageStringLang(uint id)
{
  return getStringByLang(id);
}
//---------------------------------------------------------------------
LPCTSTR P_BaseBody::getPageString(uint id)
{
  return sStr.getString(id);
}
//---------------------------------------------------------------------
void P_BaseBody::replacePageString(uint id, LPCTSTR newText)
{
  sStr.replaceString(id, str_newdup(newText), true, true);
}
//---------------------------------------------------------------------
PBitmap* P_BaseBody::getBmp4Btn(uint ix)
{
  if(BmpForBtn.getElem() > ix)
    return BmpForBtn[ix];
  return 0;
}
//---------------------------------------------------------------------
const PVect<PBitmap*>& P_BaseBody::getBmp4Btn()
{
  return BmpForBtn;
}
//----------------------------------------------------------------------------
