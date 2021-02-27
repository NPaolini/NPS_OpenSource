//------ PVarEdit.cpp --------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "1.h"
#include "pVarEdit.h"
#include "p_basebody.h"
#include "p_util.h"
#include "pCommonFilter.h"
//----------------------------------------------------------------------------
int PVarEdit::useSeparator = -1;
//----------------------------------------------------------------------------
PVarEdit::PVarEdit(P_BaseBody* owner, uint id) :  baseActive(owner, id),
    svEdit(owner, id, 0, 0, 0, 0), Fg(0), Bkg(0), Brush(0), vTextEdit(0), clipped(false)
{
  if(Attr.id > OFFS_INIT_VAR_EXT)
    Attr.id -= OFFS_INIT_VAR_EXT;
  if(useSeparator < 0) {
    LPCTSTR p = getString(USE_NUMBER_SEPARATOR);
    useSeparator = p && _ttoi(p);
    }
}
//----------------------------------------------------------------------------
PVarEdit::~PVarEdit()
{
  destroy();
  if(Brush)
    DeleteObject(Brush);
  delete vTextEdit;
}
//----------------------------------------------------------------------------
bool PVarEdit::allocObj(LPVOID param)
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
  int len = 0;
  int align = 0;
  int style = 0;

  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &idfont,
                    &Rfg, &Gfg, &Bfg,
                    &Rbk, &Gbk, &Bbk,
                    &len, &align, &style);

  idfont -= ID_INIT_FONT;

  x += getOwner()->getOffset().x;
  y += getOwner()->getOffset().y;

  PRect rect(0, 0, R__X(w), R__Y(h));
  rect.MoveTo(R__X(x), R__Y(y));

  setRect(rect);

  Attr.x = rect.left;
  Attr.y = rect.top;
  Attr.w = rect.Width();
  Attr.h = rect.Height();
  setTextLen(len ? len : 255);

  Fg = RGB(Rfg, Gfg, Bfg);
  Bkg = RGB(Rbk, Gbk, Bbk);

  Brush = CreateSolidBrush(Bkg);

  HFONT font = getOwner()->getFont(idfont);
  setFont(font);

  if(makeStdVars()) {
    uint base = BaseVar.getBaseNum();
    switch(BaseVar.getType()) {
      case prfData::tBData:
      case prfData::tWData:
      case prfData::tBitData:
      case prfData::tDWData:

      case prfData::tBsData:
      case prfData::tWsData:
      case prfData::tDWsData:
        // se usa nBit forziamo il tipo a bit
        if(BaseVar.getnBit())
          BaseVar.setType(prfData::tBitData);
        if(base) {
          if(ebBinary == base)
            setFilter(new PBinaryFilter);
          else if(ebHex == base)
            setFilter(new PHexFilter);
          break;
          }
        // else fall through
      case prfData::tFRData:
      case prfData::ti64Data: // 64 bit
      case prfData::tRData:   // 64 bit float
        if(useSeparator)
          setFilter(new PNumbFilterComma);
        else
          setFilter(new PNumbFilter);
        break;

      case prfData::tDate:    // 64 bit per data
        do {
          PDateFilter* dFlt = new PDateFilter;
          if(Other == whichData())
            dFlt->setStandard(false);
          setFilter(dFlt);
          } while(false);
        break;

      case prfData::tHour:    // 64 bit per ora
        setFilter(new PTimeFilter);
        break;

      case prfData::tDateHour:// 64 bit per data + ora
        do {
          PDateAndTimeFilter* dFlt = new PDateAndTimeFilter;
          if(Other == whichData())
            dFlt->setStandard(false);
          setFilter(dFlt);
          } while(false);
        break;
      default:
        setCustomPaste(false);
        break;
      }
    }
  // per compatibilità, se i valori di align e style sono entrambi zero, il testo è centrato, top e stile normale
  enum eAlign { aCenter, aLeft, aRight, aTop = 4, aMid = 8, aBottom = 16 };
  uint alignV = (align & aMid) ? 1 : (align & aBottom) ? 2 : 0;
  Attr.style &= ~ES_LEFT;
  if(align & aLeft)
    Attr.style |= ES_LEFT | ES_MULTILINE;
  else if(align & aRight)
    Attr.style |= ES_RIGHT | ES_MULTILINE;
  else
    Attr.style |= ES_CENTER | ES_MULTILINE;
  uint border = 2;
  switch(style) {
    case 0: // stile normale precedente
      Attr.exStyle = WS_EX_WINDOWEDGE | WS_EX_STATICEDGE;
      break;
    case 1: // no bordo
      Attr.exStyle = 0;
      break;
    case 2: // bordo nero
      Attr.style |= WS_BORDER;
      border += 2;
      Attr.exStyle = 0;
      break;
    case 3: // stile normale attuale
//      border += 2;
//      Attr.style |= WS_BORDER;
      Attr.exStyle = //WS_EX_WINDOWEDGE |
          WS_EX_CLIENTEDGE |
          0;
      break;
    case 4: // stile rilievo
//      Attr.style |= WS_BORDER;
      Attr.exStyle = WS_EX_DLGMODALFRAME; // | WS_EX_WINDOWEDGE;
      break;
    }
  TCHAR point[8];
  getLocaleThousandSep(point, SIZE_A(point));

  int type = BaseVar.getType();
  if(type !=  prfData::tStrData && BaseVar.getDec())
    type = prfData::tFRData;

  switch(type) {
    case prfData::tBData:
    case prfData::tWData:
    case prfData::tBitData:
    case prfData::tDWData:

    case prfData::tBsData:
    case prfData::tWsData:
    case prfData::tDWsData:
      switch(BaseVar.getBaseNum()) {
        case ebTen:
        default:
          if(useSeparator)
            vTextEdit = new svVisualTextEditSep(this, point[0], 3);
          else
            vTextEdit = new svVisualTextEdit(this);
          break;
        case ebHex:
          vTextEdit = new svVisualTextEditSep(this, _T('-'), 2);
          break;
        case ebBinary:
          vTextEdit = new svVisualTextEditSep(this, _T('.'), 4);
          break;
        }
      break;
    case prfData::tFRData:
    case prfData::tRData:   // 64 bit float
      if(useSeparator) {
        TCHAR comma[8];
        getLocaleDecimalPoint(comma, SIZE_A(comma));
        vTextEdit = new svVisualTextEditSep(this, point[0], 3, comma[0]);
        }
      else
        vTextEdit = new svVisualTextEdit(this);
      break;
    default:
      vTextEdit = new svVisualTextEdit(this);
      break;
    }
  bool success = create();
  if(success && alignV) {
    SIZE sz = getSizeFont();
    PRect r;
//    SendMessage(*this, EM_SETRECT, 0, (LPARAM)(LPRECT)r);
//    if(!r.Width())
      GetClientRect(*this, r);
    int top;
    PRect r2;
    GetWindowRect(*this, r2);
//    SIZE offs = { (r2.Width() - r.Width()) / 2, (r2.Height() - r.Height()) / 2 };
    SIZE offs = { border, border };
    if(1 == alignV) {
      top = (r.Height() - sz.cy / SIMUL_DEC_FONT) / 2 - offs.cy;
      r2 = PRect(r.left + offs.cx, top, r.right - offs.cx, r.bottom - top);
      }
    else {
      top = (r.Height() - sz.cy / SIMUL_DEC_FONT) - offs.cy;
      r2 = PRect(r.left + offs.cx, top, r.right - offs.cx, r.bottom);
      }
    SendMessage(*this, EM_SETRECT, 0, (LPARAM)(LPRECT)r2);
    }
  return success;
}
//----------------------------------------------------------------------------
bool PVarEdit::evCtlColor(HBRUSH& result, HDC hdc, HWND hwnd, uint /*message*/)
{
  if(hwnd == getHandle()) {
    SetTextColor(hdc, Fg);
    SetBkColor(hdc, Bkg);
    result = Brush;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
void PVarEdit::setVisibility(bool set)
{
  if(set) {
    SendMessage(*this, EM_SETMODIFY, TRUE, 0);
    Owner->addToClipped(getRect());
    }
  else
    Owner->removeFromClipped(getRect());
  baseActive::setVisibility(set);
}
//----------------------------------------------------------------------------
LRESULT PVarEdit::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_SETFOCUS:
      vTextEdit->setFocus();
      break;

    case WM_KILLFOCUS:
      vTextEdit->killFocus();
      break;
    }
  return svEdit::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
HWND PVarEdit::evSetAroundFocus(UINT key)
{
  HWND hwnd = (HWND)-1;
  switch(key) {
    case VK_UP:
      hwnd = Around.getHwnd(pAround::aTop);
      break;

    case VK_DOWN:
      hwnd = Around.getHwnd(pAround::aBottom);
      break;

    case VK_LEFT:
      do {
        LRESULT sel = SendMessage(*this, EM_GETSEL, 0, 0);
        if(-1 == sel)
          break;
        int init = LOWORD(sel);
        int end  = HIWORD(sel);
        if(!init && !end)
          hwnd = Around.getHwnd(pAround::aLeft);
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

        if(init == end && init == sel)
          hwnd = Around.getHwnd(pAround::aRight);
        } while(false);
      break;
    }
  if(!hwnd)
    hwnd = (HWND)-1;
  else if(hwnd == (HWND)-1)
    hwnd = 0;
  return hwnd;
}
//----------------------------------------------------------------------------
bool PVarEdit::setFocus(pAround::around where)
{
  uint key[] = { VK_LEFT, VK_UP, VK_RIGHT, VK_DOWN };
  HWND result = evSetAroundFocus(key[where]);
  if(result) {
    sendCurrData();
    if(result != (HWND)-1) {
      do {
        getAppl()->pumpMessages();
        if(!(GetAsyncKeyState(VK_RETURN)& 0x8000) && !(GetAsyncKeyState(VK_TAB)& 0x8000))
          break;
        } while(true);
      do {
        getAppl()->pumpMessages();
        if(!(GetAsyncKeyState(VK_RETURN)& 0x8000) && !(GetAsyncKeyState(VK_TAB)& 0x8000))
          break;
        } while(true);
      SetFocus(result);
      }
    else
      SendMessage(*this, EM_SETSEL, 0, (LPARAM)-1);
    }

  return toBool(result);
}
//----------------------------------------------------------------------------
bool PVarEdit::evChar(WPARAM& key)
{
  if(VK_TAB == key) {
    if(GetKeyState(VK_SHIFT) & 0x8000)
      setFocus(pAround::aTop);
    else
      setFocus(pAround::aBottom);
    return true;
    }
  else if(VK_ESCAPE == key)
    getOwner()->resetTimeEdit();

  return svEdit::evChar(key);
}
//----------------------------------------------------------------------------
bool PVarEdit::evKeyDown(WPARAM& key)
{
  if((GetAsyncKeyState(VK_RETURN)& 0x8001) || (GetAsyncKeyState(VK_TAB)& 0x8001))
    return false;
  return svEdit::evKeyDown(key);
}
//----------------------------------------------------------------------------
void PVarEdit::sendCurrData()
{
  if(SendMessage(*this, EM_GETMODIFY, 0, 0)) {
    if(BaseVar.getPrph()) {
      prfData data;
      if(!getData(data, 0))
        return;
      if(prfData::tBitData == data.typeVar)
        data.U.dw = MAKELONG(MAKEWORD(BaseVar.getnBit(), BaseVar.getOffs()), data.U.w);
      if(!verifyRange(BaseVar.getNorm(), data))
        return;

      P_BaseBody::preSend resSend = getOwner()->preSendEdit(this, data);
      switch(resSend) {
        case P_BaseBody::YES_SEND:
          SendMessage(*this, EM_SETMODIFY, 0, 0);
          getOwner()->sendData(data, BaseVar.getPrph());
          getOwner()->postSendEdit(this);
          break;
        case P_BaseBody::WAIT_SEND:
          SendMessage(*this, EM_SETMODIFY, 0, 0);
          break;
        }
      }
      getOwner()->resetTimeEdit();
    }
}
//----------------------------------------------------------------------------
bool PVarEdit::getData(prfData& target, int prph)
{
  if(prph > 0 && prph != BaseVar.getPrph())
    return false;

  TCHAR buff[1024];
  vTextEdit->getData(buff, SIZE_A(buff));

  switch(BaseVar.getType()) {
    case prfData::tFRData:
    case prfData::tRData:
      if(useSeparator) {
        TCHAR decpoint[8];
        getLocaleDecimalPoint(decpoint, SIZE_A(decpoint));
        if(_T('.') != decpoint[0]) {
          LPTSTR p = buff;
          while(*p) {
            if(decpoint[0] == *p)
              *p = _T('.');
            ++p;
            }
          }
        }
      break;
    }
  bool success = BaseVar.getDeNormalized(getOwner(), target, buff);
  target.lAddr += getOffs();
  return success;
}
//----------------------------------------------------------------------------
bool PVarEdit::update(bool force)
{
  bool upd = baseActive::update(force);
  if(!isVisible())
    return upd;
  if(!clipped)
    clipped = Owner->addToClipped(getRect());
  force |= upd;

  prfData data;
  prfData::tResultData result = BaseVar.getData(getOwner(), data, getOffs());

  if(prfData::failed >= result) {
    getOwner()->ShowErrorData(BaseVar.getPrph(), data, result);
    return upd;
    }

  if(SendMessage(*this, EM_GETMODIFY, 0, 0))
    force = true;

  TCHAR t[1024] = { 0 };
  SendMessage(*this, WM_GETTEXT, SIZE_A(t) - 1, (LPARAM)t);
  if(!force && prfData::notModified == result) {
    if(_tcscmp(WAIT_UPDATE_CHAR_EDIT, t))
      return upd;
    }
  TCHAR buff[512] = { 0 };
  BaseVar.makeString(buff, SIZE_A(buff), data);
  if(useSeparator)
    replaceWithComma(buff);
  if(_tcscmp(t, buff))
    vTextEdit->setData(buff);
  return true;
}
