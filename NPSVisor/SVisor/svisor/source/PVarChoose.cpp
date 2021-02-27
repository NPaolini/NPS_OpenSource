//------ PVarChoose.cpp ---------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "PVarChoose.h"
#include "mainclient.h"
#include "p_basebody.h"
#include "p_util.h"
#include "PListBox.h"
#include "svDialog.h"
#include "sizer.h"
//----------------------------------------------------------------------------
class PTextFixedPanelChoose : public PTextFixedPanel
{
  private:
    typedef PTextFixedPanel baseClass;
  public:
    PTextFixedPanelChoose(LPCTSTR str, const PRect& rect, HFONT font, COLORREF txt,
                     PPanel::bStyle type = DN, COLORREF bkg = GetSysColor(COLOR_BTNFACE),
                     b3DStyle style3d = PTextPanel::NO3D) :
        baseClass(str, rect, font, txt, type, bkg, style3d)
    {}
    virtual void draw(HDC hdc);
};
//--------------------------------------------------------------------------
#define MAX_ARROW_SIZE 24
//-----------------------------------------------------------
void PTextFixedPanelChoose::draw(HDC hdc)
{
  PRect r(getRect());
  int h = r.Height();
  if(h > MAX_ARROW_SIZE)
    h = MAX_ARROW_SIZE;
  r.right -= h;
  setRect(r);
  baseClass::draw(hdc);
  r.right += h;
  setRect(r);
  r.bottom = r.top + h;
  r.left = r.right - h;
  r.Inflate(-2, -2);
  DrawFrameControl(hdc, r, DFC_SCROLL, DFCS_SCROLLDOWN);
}
//----------------------------------------------------------------------------
class POwnBtnChoose : public POwnBtn
{
  public:
    POwnBtnChoose(PTextFixedPanel* panel, PWin * parent, uint id, int x, int y, int w, int h,
          LPCTSTR text = 0, HINSTANCE hinst = 0) : Panel(panel),
        POwnBtn(parent, id, x, y, w, h, text, hinst)
    {
      PRect r(1, 1, w - 1, h - 1);
      Panel->setRect(r);
      enableCapture(true);
    }

    POwnBtnChoose(PTextFixedPanel* panel, PWin * parent, uint id, const PRect& r,
          LPCTSTR text = 0, HINSTANCE hinst = 0) : Panel(panel),
        POwnBtn(parent, id, r, text, hinst)
    {
      PRect r2(1, 1, r.Width() - 1, r.Height() - 1);
      Panel->setRect(r2);
      enableCapture(true);
    }

    POwnBtnChoose(PTextFixedPanel* panel, PWin * parent, uint resid, HINSTANCE hinst = 0) :
        Panel(panel), POwnBtn(parent, resid, hinst)
    {
      PRect r(Panel->getRect());
      PRect r2(1, 1, r.Width() - 1, r.Height() - 1);
      Panel->setRect(r2);
      enableCapture(true);
    }

    ~POwnBtnChoose() { destroy(); delete Panel; }

  protected:
    virtual void drawSelected(HDC hdc, LPDRAWITEMSTRUCT dis) { drawAll(hdc, dis); }
    virtual void drawUnselected(HDC hdc, LPDRAWITEMSTRUCT dis) { drawAll(hdc, dis); }

    virtual void mouseEnter(const POINT& pt, uint flag);

  private:
    typedef POwnBtn baseClass;

    PTextFixedPanel* Panel;

    void drawAll(HDC hdc, LPDRAWITEMSTRUCT dis);
};
//----------------------------------------------------------------------------
inline
void POwnBtnChoose::mouseEnter(const POINT& pt, uint flag)
{
  baseClass::mouseEnter(pt, flag);
  SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURS_HAND)));
//  SetFocus(*this);
}
//--------------------------------------------------------------------------
void POwnBtnChoose::drawAll(HDC hdc, LPDRAWITEMSTRUCT /*dis*/)
{
  Panel->draw(hdc);
}
//-------------------------------------------------------------------------
PVarChoose::PVarChoose(P_BaseBody* owner, uint id) : baseActive(owner, id),
              currChoice(0), Btn(0), Panel(0), idColor(0), idText(0), customSend(0),
              AlternateTextFile(0), TextVar(0)

{}
//----------------------------------------------------------------------------
PVarChoose::~PVarChoose()
{
  delete []AlternateTextFile;
  delete TextVar;
}
//----------------------------------------------------------------------------
void PVarChoose::makeVals(int idText)
{
  for(uint i = 0; i < 500; ++i) {
    LPCTSTR p = getOwner()->getPageString(idText + i);
    if(!p)
      break;
    Vals[i] = _tstof(p);
    p = findNextParamTrim(p);
    // se è presente il valore allora segue il testo e sostituiamo la stringa per
    // rimuovere il valore
    if(p)
      getOwner()->replacePageString(idText + i, p);
    // altrimenti è stato impostato per sbaglio senza premettere valori
    else {
      customSend = false;
      break;
      }
    }
}
//----------------------------------------------------------------------------
bool PVarChoose::allocObj(LPVOID param)
{
  LPCTSTR p = getOwner()->getPageString(getId());
  if(!p)
    return false;

  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  uint idfont = 0;
  int Rfg = 0;
  int Gfg = 0;
  int Bfg = 0;
  int Rbk = 220;
  int Gbk = 220;
  int Bbk = 220;
  int no_up_dn = 0;
  int align = 0;
  customSend = 0;
  int alternateText = 0;

  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &idfont,
                    &Rfg, &Gfg, &Bfg,
                    &Rbk, &Gbk, &Bbk,
                    &no_up_dn, &align, &idColor,
                    &customSend, &alternateText);

  idfont -= ID_INIT_FONT;

  PPanel::bStyle type = no_up_dn == 0 ? PPanel::NO  :
                        no_up_dn == 1 ? PPanel::UP_FILL :
                        no_up_dn == 2 ? PPanel::DN_FILL :
                        no_up_dn == 3 ? PPanel:: FILL   :

                        no_up_dn == 4 ? PPanel::UP :
                        no_up_dn == 5 ? PPanel::DN :
                        no_up_dn == 6 ? PPanel::BORDER_FILL :
                                        PPanel::BORDER;

  x += getOwner()->getOffset().x;
  y += getOwner()->getOffset().y;

  PRect rect(0, 0, R__X(w), R__Y(h));
  rect.MoveTo(R__X(x), R__Y(y));

  setRect(rect);

  HFONT font = getOwner()->getFont(idfont);

  Panel = new PTextFixedPanelChoose(0, rect, font, RGB(Rfg, Gfg, Bfg), type, RGB(Rbk, Gbk, Bbk));
  uint idWin = getId();
  if(idWin > OFFS_INIT_VAR_EXT)
    idWin -= OFFS_INIT_VAR_EXT;

  Btn = new POwnBtnChoose(Panel, getOwner(), idWin, rect);

  enum { aCenter, aLeft, aRight, aTop = 4, aMid = 8, aBottom = 16 };
  align &= 3;
  align = aLeft == (align & aLeft) ? TA_LEFT :
         aRight == (align & aRight) ? TA_RIGHT :
                                     TA_CENTER;

  Panel->setAlign(align);
  Panel->setVAlign(DT_VCENTER);

  POwnBtn::colorRect color(0, RGB(0xff, 0, 0), 1, 0);
//  POwnBtn::colorRect color(0, RGB(0, 0, 0xff), 2, 0);
  Btn->setColorRect(color);

  if(!makeStdVars())
    return false;
  idText = BaseVar.getDec();
  BaseVar.setDec(0);

  if(customSend)
    makeVals(idText);

  if(idColor) {
    if(-1 == BaseVar.getNorm()) {
      BaseVar.setNorm(0);
      Colors.makeColors(getOwner(), idColor);
      TypeShow = tsIxColor;
      }
/*
// questa parte di codice non può essere attivata, in svMaker viene impostato fisso -1 come normalizzatore
// in caso di modifica occorre salvare diversamente la lista dei colori, per ora solo i colori, occorrerà
// aggiungere il valore abbinato
    else  {
      if(-2 >= BaseVar.getNorm()) {
        TypeShow = tsValColorAndText;
        if(-2 == BaseVar.getNorm())
          BaseVar.setNorm(0);
        else
          BaseVar.setNorm(-BaseVar.getNorm());
        }
      else
        TypeShow = tsValColor;
      VarColors.makeColors(getOwner(), idColor);
      }
*/
    }
  else if(-1 == BaseVar.getNorm()) {
    BaseVar.setNorm(0);
    TypeShow = tsIxText;
    }
  while(alternateText) {
    p = getOwner()->getPageString(alternateText);
    if(!p)
      break;
    uint prph = _ttoi(p);
    p = findNextParamTrim(p);
    if(!p)
      break;
    if(!prph)
      AlternateTextFile = str_newdup(p);
    else {
      uint addr = _ttoi(p);
      p = findNextParamTrim(p);
      if(!p)
        break;
      uint dim = _ttoi(p);
      TextVar = new pVariable(prph, addr, prfData::tStrData, 0, 0, 0, dim);
      }
    break;
    }
  if(!Btn->create())
    return false;

  return true;
}
//----------------------------------------------------------------------------
#define MIN_REAL 0.000001
#define IS_EQU_REAL(v1, v2) (fabs((v1) - (v2)) < MIN_REAL)
//----------------------------------------------------------------------------
bool PVarChoose::update(bool force)
{
  bool upd = baseVar::update(force);
  if(!isVisible())
    return upd;

  prfData data;
  prfData::tResultData result = BaseVar.getData(getOwner(), data, getOffs());

  if(prfData::failed >= result) {
    getOwner()->ShowErrorData(BaseVar.getPrph(), data, result);
    return upd;
    }
  force |= upd;
  if(!force && prfData::notModified == result)
    return upd;

  COLORREF oldfg = Panel->getTextColor();
  COLORREF oldbg = Panel->getColor();
  TCHAR buff[512] = _T("\0");
  if(customSend) {
    double v = BaseVar.getNormalizedResult(data);
    uint nElem = Vals.getElem();
    bool found = false;
    // prima cerca il valore esatto
    for(uint i = 0; i < nElem; ++i) {
      if(IS_EQU_REAL(v, Vals[i])) {
        found = true;
        if(!getAlternateText(buff, SIZE_A(buff), i)) {
          smartPointerConstString sp = getOwner()->getPageStringLang(idText + i);
          if(sp)
            _tcscpy_s(buff, sp);
          }
        if(tsIxColor == TypeShow) {
          COLORREF fg;
          COLORREF bg;
          if(Colors.getColors(fg, bg, i)) {
            Panel->setTextColor(fg);
            Panel->setBkgColor(bg);
            }
          }
        break;
        }
      }
    if(!found) {
      BaseVar.makeString(buff, SIZE_A(buff), data);
      }
    }
  else {
    int ix = BaseVar.getIxResult(data);
    if(tsIxColor == TypeShow) {
      COLORREF fg;
      COLORREF bg;
      if(Colors.getColors(fg, bg, ix)) {
        Panel->setTextColor(fg);
        Panel->setBkgColor(bg);
        }
      }
    if(!getAlternateText(buff, SIZE_A(buff), ix)) {
      smartPointerConstString sp = getOwner()->getPageStringLang(idText + ix);
      if(sp)
        _tcscpy_s(buff, SIZE_A(buff), sp);
      }
    }
  if(_tcscmp(Panel->getText(), buff)) {
    Panel->setText(buff);
    invalidate();
    }
  else if(oldfg != Panel->getTextColor() || oldbg != Panel->getColor())
    invalidate();
  return true;
}
//----------------------------------------------------------------------------
HWND PVarChoose::getHwnd()
{
  return Btn ? Btn->getHandle() : 0;
}
//----------------------------------------------------------------------------
class PVarChooseDialog : public svModDialog
{
  private:
    typedef svModDialog baseClass;
  public:
    PVarChooseDialog(P_BaseBody* parent, PVarChoose* owner, LPCTSTR curr, PRect rect, uint idFirstText,
            uint idColors, int& pos, HFONT font) :
      baseClass(parent, IDD_P_VAR_CHOOSE), idFirstText(idFirstText), Body(parent), Owner(owner), processed(false),
      Pos(pos), rect(rect), idColors(idColors), Font(font), Curr(curr) {}

    ~PVarChooseDialog() { destroy(); }

    bool create();
  protected:
    bool preProcessMsg(MSG& msg);
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();

  private:
    P_BaseBody* Body;
     PVarChoose* Owner;
    uint idFirstText;
    uint idColors;
    LPCTSTR Curr;
    int& Pos;
    PRect rect;
    HFONT Font;
    bool processed;
};
//----------------------------------------------------------------------------
bool PVarChooseDialog::create()
{
  PListBox* lb = new PListBox(this, IDC_LISTBOX_P_VAR_CHOOSE);
  LPCTSTR p = Body->getPageString(idColors);
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
      lb->SetColor(fg, bg);
      if(RfgS != -1)  {
        COLORREF fg = RGB(RfgS, GfgS, BfgS);
        COLORREF bg = RGB(RbkS, GbkS, BbkS);
        lb->SetColorSel(fg, bg);
        }
      }
    }
  int tabs[] = { 100 };
  int show[] = {  1 };
  lb->SetTabStop(SIZE_A(tabs), tabs, show);
  lb->setFont(Font, false);

  if(!baseClass::create())
    return false;

  uint id = idFirstText;
  int select = (Curr && *Curr) ? -1 : 0;
  TCHAR buff[512];
  while(true) {
    smartPointerConstString p = Body->getPageStringLang(id);
    if(!&p)
      break;
    if(Owner->getAlternateText(buff, SIZE_A(buff), id - idFirstText))
      p = smartPointerConstString(buff, false);
    SendMessage(*lb, LB_ADDSTRING, 0, (LPARAM)&p);

    if(-1 == select) {
      if(!_tcscmp(&p, Curr))
        select = id - idFirstText;
      }
    ++id;
    }
  if(-1 == select)
    select = 0;

  id -= idFirstText;
  if(id) {
    int h = lb->getHItem();
    h *= id;
    h += GetSystemMetrics(SM_CYBORDER) * 4;
    rect.bottom = rect.top + h;
    }

  PRect r2(rect);
  r2.MoveTo(0, 0);
  lb->setWindowPos(0, r2, SWP_NOZORDER);
  lb->setIntegralHeight();
  GetWindowRect(*lb, r2);
  AdjustWindowRect(r2, WS_DLGFRAME, FALSE);
  r2.MoveTo(rect.left, rect.top);
  MapWindowPoints(*getParent(), HWND_DESKTOP, (LPPOINT)(LPRECT)r2, 2);
  int hBottom = GetSystemMetrics(SM_CYCAPTION) * 2;
  if(r2.bottom >= (int)sizer::getHeight() - hBottom)
    r2.MoveTo(r2.left, sizer::getHeight() - r2.Height() - hBottom);
  if(r2.top < hBottom)
    r2.top = hBottom;
  setWindowPos(0, r2, SWP_NOZORDER);

  GetClientRect(*this, r2);
  lb->setWindowPos(0, r2, SWP_NOZORDER);
  lb->setIntegralHeight();
  GetWindowRect(*lb, r2);
  AdjustWindowRect(r2, WS_DLGFRAME, FALSE);
  setWindowPos(0, r2, SWP_NOZORDER | SWP_NOMOVE);

  SendMessage(*lb, LB_SETCURSEL, select, 0);
  return true;
}
//-----------------------------------------------------------
bool PVarChooseDialog::preProcessMsg(MSG& msg)
{
  if(!processed && WM_LBUTTONUP == msg.message) {
    processed = true;
    POINT pt =  { LOWORD(msg.lParam), HIWORD(msg.lParam) };
    if(isInClient(pt))
      PostMessage(*this, WM_COMMAND, MAKEWPARAM(IDOK, 0), 0);
    else
      PostMessage(*this, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0);
    }
  if(!toBool(IsDialogMessage(getHandle(), &msg)))
    return baseClass::preProcessMsg(msg);
  return true;
}
//-----------------------------------------------------------
LRESULT PVarChooseDialog::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(HIWORD(wParam)) {
        case LBN_DBLCLK:
          if(!processed) {
            processed = true;
            PostMessage(*this, WM_COMMAND, MAKEWPARAM(IDOK, 0), 0);
            }
          break;
        case LBN_KILLFOCUS:
          if(!processed) {
            processed = true;
            PostMessage(*this, WM_COMMAND, MAKEWPARAM(IDCANCEL, 0), 0);
            }
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void PVarChooseDialog::CmOk()
{
  HWND hlb = GetDlgItem(*this, IDC_LISTBOX_P_VAR_CHOOSE);
  Pos = SendMessage(hlb, LB_GETCURSEL, 0, 0);
  baseClass::CmOk();
}
//----------------------------------------------------------------------------
int PVarChoose::action(prfData& data)
{
  PRect rect = get_Rect();
  if(rect.right + rect.Width() > (int)sizer::getWidth())
    rect.Offset(-rect.Width(), 0);
  else
    rect.Offset(rect.Width(), 0);

  int pos;
  PVarChooseDialog cd(getOwner(), this, Panel->getText(), rect, idText, getThirdExtendId(), pos, Panel->getFont());
  if(IDOK == cd.smodal(false))
//  if(IDOK == cd.modal())
  {
    if(prfData::tBitData == BaseVar.getType()) {
      pVariable tmpVar = BaseVar;
      tmpVar.setType(prfData::tDWData);
      tmpVar.getData(getOwner(), data, getOffs());
      }
    else
      BaseVar.getData(getOwner(), data, getOffs());

    DWDATA mask = 0;
    if(BaseVar.getnBit() && BaseVar.getnBit() < 32) {
      mask = (1 << BaseVar.getnBit()) - 1;
      mask <<= BaseVar.getOffs();
      mask = ~mask;
      }
    // azzera la parte riguardante la variabile corrente lasciando inalterato
    // lo spazio dei bit non usati
    data.U.dw &= mask;

    double val = pos;
    if(customSend) {
      int pos2 = min(pos, (int)Vals.getElem() - 1);
      val = Vals[pos2];
      pos = ROUND_REAL(val);
      }
    switch(BaseVar.getType()) {
      case prfData::tBitData:
        if(pos)
          data.U.dw |= 1 << (pos - 1 + BaseVar.getOffs());
        data.typeVar = prfData::tDWData;
        break;

      case prfData::tBData:
      case prfData::tBsData:
      case prfData::tWData:
      case prfData::tWsData:
      case prfData::tDWData:
      case prfData::tDWsData:
        data.U.dw |= pos << BaseVar.getOffs();
        break;

      case prfData::tFRData:
        data.U.fw = (fREALDATA)val;
        break;

      default:
        return 0;
      }
    SetFocus(*Btn);
    return BaseVar.getPrph();
    }
  SetFocus(*Btn);
  return 0;
}
//----------------------------------------------------------------------------
bool PVarChoose::getAlternateText(LPTSTR buff, uint sz, uint ix)
{
  if(AlternateTextFile) {
    setOfString set(AlternateTextFile);
    LPCTSTR p = set.getString(ix + 1);
    if(!p)
      return false;
    smartPointerConstString sp = getStringByLangSimple(p);
    _tcscpy_s(buff, sz, &sp);
    return true;
    }
  if(!TextVar)
    return false;
  prfData data;
  prfData::tResultData result = TextVar->getData(getOwner(), data, getOffs());
  if(prfData::failed >= result)
    return false;
  TCHAR t[_MAX_PATH];
  copyStrZ(t, data.U.str.buff, data.U.str.len);
  AlternateTextFile = t;
  bool success = getAlternateText(buff, sz, ix);
  AlternateTextFile = 0;
  return success;
}
//----------------------------------------------------------------------------
