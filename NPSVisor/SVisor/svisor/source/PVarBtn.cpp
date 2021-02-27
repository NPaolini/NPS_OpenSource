//------ PVarBtn.cpp ---------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "pVarBtn.h"
#include "p_basebody.h"
#include "pedit.h"
#include "p_util.h"
#include "mainClient.h"
#include "id_Btn.h"
#include "1.h"
#include "POwnBtnSelect.h"
#include "PVarBtnStd.h"
//----------------------------------------------------------------------------
static int insertBmp(const PVect<uint>& idBmp, PVect<PBitmap*>& target, const PVect<PBitmap*>& source)
{
  int numBmp = idBmp.getElem();
  target.setDim(max(4, numBmp));
  int numExistBmp = source.getElem();
  for(int i = 0; i < numBmp; ++i) {
    int id = idBmp[i];
    if(id < 0 || id >= numExistBmp) {
      numBmp = i;
      break;
      }

    target[i] = source[id];
    }
  switch(numBmp) {
    case 1:
      target[1] = source[idBmp[0]];
      ++numBmp;
      // fall through
    case 2:
      target[2] = target[0];
      ++numBmp;
      // fall through
    case 3:
      target[3] = target[1];
      ++numBmp;
      break;
    }

  return numBmp;
}
//----------------------------------------------------------------------------
class POwnBtnSwitchFocus : public POwnBtnSwitch
{
  private:
    typedef POwnBtnSwitch baseClass;
  public:
    POwnBtnSwitchFocus(PWin * parent, uint id, const PRect& r,
          const PVect<PBitmap*>& image,
          bool sizeByImage = true, LPCTSTR text = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, r, image, sizeByImage, text, hinst)
        {
          enableCapture(true);
        }

    virtual ~POwnBtnSwitchFocus() { destroy(); }
    bool isPressed() const { return ManagedSwitch ? ManagedSwitch->isPressed() : false; }

  protected:
    virtual void mouseEnter(const POINT& pt, uint flag);
    virtual void mouseMove(const POINT& pt, uint flag);
};
//----------------------------------------------------------------------------
inline
void POwnBtnSwitchFocus::mouseEnter(const POINT& pt, uint flag)
{
  baseClass::mouseEnter(pt, flag);
  SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURS_HAND)));
}
//----------------------------------------------------------------------------
inline
void POwnBtnSwitchFocus::mouseMove(const POINT& pt, uint flag)
{
  baseClass::mouseMove(pt, flag);
  SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURS_HAND)));
}
//----------------------------------------------------------------------------
class POwnBtnSelectFocus : public POwnBtnSelect
{
  private:
    typedef POwnBtnSelect baseClass;
  public:
    POwnBtnSelectFocus(cntGroup* group, PWin * parent, uint id, const PRect& r,
          const PVect<PBitmap*>& image,
          bool sizeByImage = true, LPCTSTR text = 0, HINSTANCE hinst = 0) :
        baseClass(group, parent, id, r, image, sizeByImage, text, hinst)
        {
          enableCapture(true);
        }

    virtual ~POwnBtnSelectFocus() { destroy(); }
    bool isPressed() const { return ManagedSwitch ? ManagedSwitch->isPressed() : false; }
  protected:
    virtual void mouseEnter(const POINT& pt, uint flag);
    virtual void mouseMove(const POINT& pt, uint flag);
    virtual void unselectOther();
};
//----------------------------------------------------------------------------
inline
void POwnBtnSelectFocus::mouseMove(const POINT& pt, uint flag)
{
  baseClass::mouseMove(pt, flag);
  SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURS_HAND)));
}
//----------------------------------------------------------------------------
inline
void POwnBtnSelectFocus::mouseEnter(const POINT& pt, uint flag)
{
  baseClass::mouseEnter(pt, flag);
  SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURS_HAND)));
}
//----------------------------------------------------------------------------
void POwnBtnSelectFocus::unselectOther()
{
  PVect<uint> idsBtn;
  int nElem = Group->Group.getElem();
  const PVect<POwnBtn*>& cGrp = Group->Group;
  for(int i = 0; i < nElem; ++i) {
    if(cGrp[i] == this)
      continue;
    manageBtnSwitch::state stat = ((POwnBtnSelect*)cGrp[i])->getState();
    if(stat != manageBtnSwitch::OFF) {
      idsBtn[idsBtn.getElem()] = cGrp[i]->Attr.id;
      }
    }
  baseClass::unselectOther();
  nElem = idsBtn.getElem();
  if(nElem) {
    P_BaseBody *bb;
    PWin* par = getParent();
    do {
      bb = dynamic_cast<P_BaseBody*>(par);
      if(bb)
        break;
      par = par->getParent();
      } while(par);
    if(bb) {
      for(int i = 0; i < nElem; ++i)
        bb->simulActionBtn(idsBtn[i]);
      bb->simulActionBtn(Attr.id);
      }
    }
}
//----------------------------------------------------------------------------
class POwnBtnImageFocus : public POwnBtnImage
{
  private:
    typedef POwnBtnImage baseClass;
  public:
    POwnBtnImageFocus(PWin * parent, uint id, const PRect& r,
          const PVect<PBitmap*>& image,
          bool sizeByImage = true, LPCTSTR text = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, r, image, sizeByImage, text, hinst)
        {
          enableCapture(true);
        }

    virtual ~POwnBtnImageFocus() { destroy();  }
  protected:
    virtual void mouseEnter(const POINT& pt, uint flag);
    virtual void mouseMove(const POINT& pt, uint flag);
};
//----------------------------------------------------------------------------
inline
void POwnBtnImageFocus::mouseMove(const POINT& pt, uint flag)
{
  baseClass::mouseMove(pt, flag);
  SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURS_HAND)));
}
//----------------------------------------------------------------------------
inline
void POwnBtnImageFocus::mouseEnter(const POINT& pt, uint flag)
{
  baseClass::mouseEnter(pt, flag);
  SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURS_HAND)));
}
//----------------------------------------------------------------------------
class POwnBtnPress : public POwnBtnSwitch
{
  public:
    POwnBtnPress(PWin * parent, uint id, int x, int y, int w, int h,
          const PVect<PBitmap*>& image, bool sizeByImage = true, LPCTSTR text = 0,
          HINSTANCE hinst = 0) :
        POwnBtnSwitch(parent, id, x, y, w, h, image, sizeByImage, text, hinst)
        {
          enableCapture(true);
        }

    POwnBtnPress(PWin * parent, uint id, const PRect& r,
          const PVect<PBitmap*>& image,
          bool sizeByImage = true, LPCTSTR text = 0, HINSTANCE hinst = 0) :
        POwnBtnSwitch(parent, id, r, image, sizeByImage, text, hinst)
        {
          enableCapture(true);
        }

    POwnBtnPress(PWin * parent, uint resid,
          const PVect<PBitmap*>& image,
          bool sizeByImage = true, HINSTANCE hinst = 0) :
        POwnBtnSwitch(parent, resid, image, sizeByImage, hinst)
        {
          enableCapture(true);
        }

    ~POwnBtnPress() { destroy(); }

    bool preProcessMsg(MSG& msg);

    bool isPressed() const { return ManagedSwitch ? ManagedSwitch->isPressed() : false; }
    void setPressed(bool set)  { if(ManagedSwitch) ManagedSwitch->setPressed(set); }
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void mouseEnter(const POINT& pt, uint flag);
    virtual void mouseMove(const POINT& pt, uint flag);
    bool setPushed(uint msg, uint key, bool previous);

  private:
    typedef POwnBtnSwitch baseClass;

    void verifyFocus();
};
//----------------------------------------------------------------------------
inline
void POwnBtnPress::mouseEnter(const POINT& pt, uint flag)
{
  baseClass::mouseEnter(pt, flag);
  SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURS_HAND)));
}
//----------------------------------------------------------------------------
inline
void POwnBtnPress::mouseMove(const POINT& pt, uint flag)
{
  baseClass::mouseMove(pt, flag);
  SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURS_HAND)));
}
//--------------------------------------------------------------------------
#define PREVIOUS_DOWN(a) toBool(((DWORD)(a)) & ( 1 << 30))
bool POwnBtnPress::preProcessMsg(MSG& msg)
{
  if(setPushed(msg.message, msg.wParam, PREVIOUS_DOWN(msg.lParam)))
    return true;
  return baseClass::preProcessMsg(msg);
}
//-----------------------------------------------------------
//#define SEND_MSG SendMessage
#define SEND_MSG PostMessage
//-----------------------------------------------------------
bool POwnBtnPress::setPushed(uint msg, uint key, bool previous)
{
  if(WM_KEYDOWN == msg) {
    if(VK_RETURN == key || VK_SPACE == key) {
      if((!isPressed() || manageBtnSwitch::OFF == getState()) && !previous) {
        setPressed(true);
        verifyFocus();
        setState(manageBtnSwitch::ON);
        SEND_MSG(*getParent(), WM_COMMAND, MAKEWPARAM(Attr.id, BN_CLICKED), (LPARAM)getHandle());
        }
      return true;
      }
    }
  else if(WM_KEYUP == msg) {
    if(VK_RETURN == key || VK_SPACE == key) {
      if(isPressed() || manageBtnSwitch::ON == getState()) {
        setPressed(false);
        verifyFocus();
        setState(manageBtnSwitch::OFF);
        SEND_MSG(*getParent(), WM_COMMAND, MAKEWPARAM(Attr.id, BN_CLICKED), (LPARAM)getHandle());
        }
      return true;
      }
    }
  return false;
}
//-----------------------------------------------------------
inline
void POwnBtnPress::verifyFocus()
{
  if(GetFocus() != getHandle())
    SetFocus(*this);
}
//-----------------------------------------------------------
LRESULT POwnBtnPress::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_LBUTTONUP:
      if(setPushed(WM_KEYUP, VK_SPACE, 0))
        return 0;
      break;
    case WM_LBUTTONDOWN:
      if(setPushed(WM_KEYDOWN, VK_SPACE, 0))
        return 0;
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//-----------------------------------------------------------
#define GET_VALUE_THEME(a) ((a) & 1)
#define GET_VALUE_POS(a) (((a) >> 1) & 0xf)
#define GET_VALUE_NO_BORDER(a) (((a) >> 16) & 1)
//-----------------------------------------------------------
#define CLR_VALUE_THEME(a) ((a) &= ~1)
#define CLR_VALUE_POS(a) ((a) &= ~(0xf << 1))
#define CLR_VALUE_NO_BORDER(a) ((a) &= ~(1 << 16))
//-------------------------------------------------------------------------
#define SET_VALUE_THEME(a, v) do { CLR_VALUE_THEME(a); (a) |= GET_VALUE_THEME(v); } while(false)
#define SET_VALUE_POS(a, v)  do { CLR_VALUE_POS(a); (a) |= GET_VALUE_POS(v); } while(false)
#define SET_VALUE_NO_BORDER(a, v)  do { CLR_VALUE_NO_BORDER(a); (a) |= GET_VALUE_NO_BORDER(v); } while(false)
//-------------------------------------------------------------------------
PVarBtn::PVarBtn(P_BaseBody* owner, uint id) : baseActive(owner, id), openedModeless(-1), Transparent(false),
              CurrText(0), CurrBmp(0), Btn(0)//, count4Changed(0)
{
  Flags.flag = 0;
  Flags.theme = 1;
  Flags.pos = bpTop;
}
//-------------------------------------------------------------------------
PVarBtn::~PVarBtn()
{
  flushPV(localBmp);
}
//-------------------------------------------------------------------------
void PVarBtn::makeValuePos(LPCTSTR p, PVect<valPos>& val)
{
  uint id = _ttoi(p);
  if(!id)
    return;
  p = getOwner()->getPageString(id);
  while(p) {
    val[val.getElem()].Val = _tstof(p);
    p = findNextParamTrim(p);
    }
}
//-------------------------------------------------------------------------
void PVarBtn::appendIdText(uint idInit, PVect<uint>& text)
{
  if(!idInit)
    return;
  for(;;++idInit) {
    if(!getOwner()->getPageString(idInit))
      break;
    text[text.getElem()] = idInit;
    }
}
//-------------------------------------------------------------------------
void PVarBtn::addReqVar(PVect<P_Bits*>& allBits)
{
  baseClass::addReqVar(allBits);
  addReqVar2(allBits, v_Text);
  addReqVar2(allBits, v_Color);
  addReqVar2(allBits, v_Bitmap);
}
//-------------------------------------------------------------------------
bool PVarBtn::allocObj(LPVOID param)
{
  LPCTSTR p = getOwner()->getPageString(getId());
  if(!p)
    return false;

  cntGroup** pgroup = (cntGroup**)param;
  cntGroup* & group = *pgroup;
  int x = 0;
  int y = 0;
  int w = 800;
  int h = 20;
  int typeShow = 0;
  int typeCommand = 0;
  int numBmp = 0;

  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &typeShow, &typeCommand, &numBmp);

  x += getOwner()->getOffset().x;
  y += getOwner()->getOffset().y;


  PRect rect(0, 0, R__X(w), R__Y(h));
  rect.MoveTo(R__X(x), R__Y(y));

  setRect(rect);

  p = findNextParamTrim(p, 7);
  if(!p && typeCommand != bHided)
    return false;

  // è inutile provare a fargli caricare colori e testi se è nascosto
  if(typeCommand == bHided)
    typeShow = sOnlyBmp;

  p = insertIdBmp(p, idBmp, numBmp);

  Style.type = (fBtn)typeCommand;
  Style.show = (fShowBtn)typeShow;

  PVect<COLORREF> idColorFg;
  PVect<COLORREF> idColorBkg;
  uint idFont = 0;

  if(bRadio != typeCommand)
    group = 0;

  if(bInitRadio == typeCommand || !group && bRadio == typeCommand)
    group = new cntGroup;
  else if(bHided == typeCommand)
    typeShow = sOnlyBmp;

  switch(typeShow) {
    case sOnlyBmp:
      // i dati raccolti finora sono sufficienti
      break;

    case sStdAndBmp:
      // oltre a quello sotto occorre anche il colore di bkg per il pulsante
      // N.B. se cambia l'ordine occorre modificare la sequenza
      p = getColor(p, idColorBkg);
      // fall through

    case sBmpAndText:
      // occorre raccogliere colore, font per il testo ed i vari testi
      p = getColor(p, idColorFg);
      p = getFont(p, idFont);
      insertId(p, idText, 2);
      break;

    case sNewStdAndBmp:
      p = getColor(p, idColorBkg);
      p = getColor(p, idColorFg);
      p = getFont(p, idFont);
      p = insertId(p, idText, 2);
      // fin qui è tutto uguale, da qui partono gli id per il nuovo tipo
      while(p) {
        // il primo è per i colori associati a variabile
        uint id = _ttoi(p);
        VarColors.makeFullColorsOnRow(getOwner(), id);
        // il secondo contiene l'id per la periferica di gestione colori
        p = findNextParamTrim(p);
        if(!p)
          break;
        id = _ttoi(p);
        makeOtherVar(v_Color, id);
        // il terzo contiene gli id per i testi aggiuntivi
        p = findNextParamTrim(p);
        if(!p)
          break;
        id = _ttoi(p);
        appendIdText(id, idText);
        // il quarto contiene l'id per la periferica di gestione testi
        p = findNextParamTrim(p);
        if(!p)
          break;
        id = _ttoi(p);
        makeOtherVar(v_Text, id);
        // il quinto contiene l'id per la periferica di gestione bitmap
        p = findNextParamTrim(p);
        if(!p)
          break;
        id = _ttoi(p);
        makeOtherVar(v_Bitmap, id);
        // il sesto contiene i flag per:
        //   1) utilizzo del tema
        //   2) per il posizionamento del testo-bitmap
        //   3) flag per abbinamento on-off o da variabile
        //   4) flag per dimensione fissa o scalata del bmp
        //   5) flag per nessun bordo
        p = findNextParamTrim(p);
        if(!p)
          break;
        Flags.flag = _ttoi(p);
        // il settimo contiene l'id per i valori da variabile testi
        p = findNextParamTrim(p);
        if(!p)
          break;
        makeValuePos(p, valText);
        // l'ottavo contiene l'id per i valori da variabile bmp
        p = findNextParamTrim(p);
        if(!p)
          break;
        makeValuePos(p, valBmp);
        break;
        };

      break;
    }
  idFont -= ID_INIT_FONT;

  return makeVarBtn(idColorFg, idColorBkg, idBmp, idFont,  group);
}
//-------------------------------------------------------------------------
extern bool isSpinId(uint id);
//-------------------------------------------------------------------------
void PVarBtn::checkIfSpin()
{
/*
  Non si può usare un bPress per lo spin, non viene comunque incrementato mentre è premuto ed
  esegue un incremento poi anche al rilascio. La lascio commentata, magari in futuro farò un
  pulsante ad hoc per lo spin.
*/
/*
  if(bStd != Style.type)
    return;
  uint id2 = getFirstExtendId();
  LPCTSTR p = getOwner()->getPageString(id2);
  if(!p || _ttoi(p))
    return;
  p = findNextParamTrim(p);
  if(!p)
    return;
  p = getOwner()->getPageString(_ttoi(p));
  if(!p)
    return;
  if(isSpinId(_ttoi(p)))
    Style.type = bPress;
*/
}
//-------------------------------------------------------------------------
bool PVarBtn::makeVarBtn(const vColor& fg, const vColor& bkg,
                         const PVect<uint>& id_bmp, uint idFont, cntGroup* cnt)
{
  int nElemFg = fg.getElem();
  int nElemBg = bkg.getElem();
  int nElem = max(nElemFg, nElemBg);

  idColor.setDim(nElem);
  for(int i = 0; i < nElem; ++i) {
    idColor[i].txt = 0;
    idColor[i].bkg = -1;
    }
  for(int i = 0; i < nElemFg; ++i)
    idColor[i].txt = fg[i];

  if(nElemBg && bkg[0] == -1) {
    Transparent = true;
    for(int i = 0; i < nElemBg; ++i)
      idColor[i].bkg = -1;
    }
  else {
    for(int i = 0; i < nElemBg; ++i)
      idColor[i].bkg = bkg[i];
    }


//  PVect<PBitmap*> bmp;
  const PVect<PBitmap*>& source = getOwner()->getBmp4Btn();

  switch(Style.show) {
    case sStdAndBmp:
    case sNewStdAndBmp:
      // carica un solo bitmap
      if(id_bmp.getElem()) {
        uint ix = id_bmp[0];
        Bmp[0] = source[ix];
        }
      break;
    default:
      // carica tutti i bitmap richiesti
      insertBmp(id_bmp, Bmp, source);
      break;
    }

  smartPointerConstString title;
  if(idText.getElem())
    title = getOwner()->getPageStringLang(idText[0]);

  uint idWin = getId();
  if(idWin > OFFS_INIT_VAR_EXT)
    idWin -= OFFS_INIT_VAR_EXT;

  checkIfSpin();

  if(sNewStdAndBmp == Style.show) {
    PBitmap* bmp = Bmp.getElem() ? Bmp[0] : 0;
    switch(Style.type) {
      case bStd:
        Btn = new POwnBtnStdFocus(getOwner(), idWin, get_Rect(), bmp, Flags.pos, title);
        break;

      case bCheck:
      case bActionModeless:
      case bActionOpenPage:
        Btn = new POwnBtnStdSwitchFocus(getOwner(), idWin, get_Rect(),  bmp, Flags.pos, title);
        Flags.flat = 0;
        break;

      case bInitRadio:
      case bRadio:
        Btn = new POwnBtnStdSelectFocus(cnt, getOwner(), idWin, get_Rect(),  bmp, Flags.pos, title);
        Flags.flat = 0;
        break;

      case bPress:
        Btn = new POwnBtnStdPress(getOwner(), idWin, get_Rect(),  bmp, Flags.pos, title);
        break;
      }
    if(Btn) {
      ((POwnBtnImageStd*)Btn)->disableTheme(!Flags.theme);
      if(!Flags.styleBmpDim && !Flags.fixedBmpDim)
        Flags.styleBmpDim = POwnBtnImageStd::eisScaled;
      ((POwnBtnImageStd*)Btn)->setImgStyle((POwnBtnImageStd::imgStyle)Flags.styleBmpDim);
      ((POwnBtnImageStd*)Btn)->setFlat(Flags.flat);
      // se non è flat, per default disabilita la cattura, occorre rimetterla
      Btn->enableCapture();
      Btn->setDrawingEdge(!Flags.noBorder);
//      if(Flags.noBorder && (COLORREF)-1 == bkg[0])
//        Flags.colorByVar = false;
      }
    }
  else {
    switch(Style.type) {
      case bStd:
        Btn = new POwnBtnImageFocus(getOwner(), idWin, get_Rect(), Bmp, false, title);
        break;

      case bCheck:
      case bActionModeless:
      case bActionOpenPage:
        Btn = new POwnBtnSwitchFocus(getOwner(), idWin, get_Rect(), Bmp, false, title);
        break;

      case bInitRadio:
      case bRadio:
        Btn = new POwnBtnSelectFocus(cnt, getOwner(), idWin, get_Rect(), Bmp, false, title);
        break;

      case bPress:
        Btn = new POwnBtnPress(getOwner(), idWin, get_Rect(), Bmp, false, title);
        break;
      }
    }

  makeStdVars(true);
  DWORD addr = BaseVar.getAddr();

  // pronto per la gestione della tastiera estesa
  // nel svMaker gli indirizzi per l'abbinamento con i tasti funzione partono da uno fino a trentasei
  bool success = BaseVar.getPrph() || (!BaseVar.getPrph() && (addr >= 1 && addr <= 36 || addr >= 100000));
  if(success && !BaseVar.getPrph() && addr <= 36) {
    TCHAR buff[1024] = _T("");
    LPCTSTR p = ::getString(ID_USE_PREFIX_BTN);
    uint whichText = 0;
    if(p)
      whichText = _ttoi(p);
    p = getOwner()->getPageString(ID_CODE_BTN_FZ_BEHA);
    if(p) {
      uint modf = _ttoi(p);
      switch(modf) {
        case 0:
        default:
          break;
        case 1:
          whichText = 0;
          break;
        case 2:
          whichText = 1;
          break;
        case 3:
          whichText = 3;
          break;
        case 4:
          whichText &= ~2;
          break;
        }
      }
    if(3 == (whichText & 3)) {
      if(addr <= 20)
        wsprintf(buff, _T("F%d"), addr);
      else
        wsprintf(buff, _T("S%d"), addr - 20);
      }
    if(addr > 24)
      BaseVar.setAddr(addr - 25 + ID_S5);
    else if(addr > 12)
      BaseVar.setAddr(addr - 13 + ID_F13);
    else
      BaseVar.setAddr(addr + 1);

    if(whichText & 1) {
      addr = BaseVar.getAddr();
      idText.reset();
      smartPointerConstString text = getOwner()->getPageStringLang(addr);
      if(text) {
        _tcscat_s(buff, SIZE_A(buff), text);
        set_Text(buff);
        }
      else {
        set_Text(_T(""));
        if(whichText & 4 && Btn)
          Btn->Attr.style |= WS_DISABLED;
        }
      }
    }

  if(Btn) {
    HFONT font = getOwner()->getFont(idFont);
    Btn->setFont(font);

    if(!Btn->create()) {
      delete Btn;
      Btn = 0;
      }
    else {
      switch(Style.show) {
        case sStdAndBmp:
        case sNewStdAndBmp:
          // carica un solo bitmap
          if(id_bmp.getElem()) {
            if(Bmp[0]->hasGifControl()) {
              uint nElem = localBmp.getElem();
              localBmp[nElem] = new PBitmap(*Bmp[0]);
              Bmp[0] = localBmp[nElem];
              }
            Bmp[0]->addGifControl(getHwnd());
            if(sNewStdAndBmp == Style.show)
              Btn->replaceBmp(Bmp[0]);
            }
          break;
        }
      setCurrColor(0);
      setCurrBmp(0);
      setCurrText(0);
      if(!id_bmp.getElem() && !Btn->getCaption())
        Btn->setDrawingEdge(false);
      }
    }

  return success && (toBool(Btn) || bHided == Style.type);
}
//----------------------------------------------------------------------------
void PVarBtn::setCurrAll(uint pos)
{
  setCurrColor(pos);
  setCurrText(pos);
  setCurrBmp(pos);
}
//----------------------------------------------------------------------------
void PVarBtn::setCurrText(uint pos)
{
  if(idText.getElem() > pos) {
    smartPointerConstString text = getOwner()->getPageStringLang(idText[pos]);
    if(text) {
      set_Text(text);
      CurrText = pos;
      }
    }
}
//----------------------------------------------------------------------------
void PVarBtn::setCurrColor(uint pos)
{
  if(Btn && idColor.getElem() > pos) {
    POwnBtn::colorRect color = Btn->getColorRect();
    const Color& sColor = idColor[pos];
    color.bkg = sColor.bkg;
    color.txt = sColor.txt;
    Btn->setColorRect(color);
    }
}
//----------------------------------------------------------------------------
void PVarBtn::setCurrBmp(uint pos)
{
  if(pos == CurrBmp)
    return;
  if(Btn && idBmp.getElem() > pos) {
    PBitmap* bmp = getOwner()->getBmp4Btn(idBmp[pos]);
    if(bmp) {
      Btn->replaceBmp(bmp);
      CurrBmp = pos;
      }
    }
}
//----------------------------------------------------------------------------
bool PVarBtn::isSelected()
{
  // se è un pulsante normale ogni click corrisponde alla selezione
  // e quindi è come se fosse sempre selezionato
  if(!Btn || bStd == Style.type)
    return true;
  // se è un pulsante on/off o di selezione dipende dallo stato

  {
  POwnBtnSwitch* p = dynamic_cast<POwnBtnSwitch*>(Btn);
  if(p)
    return manageBtnSwitch::ON == p->getState();
  }
  {
  POwnBtnStdSwitchFocus* p = dynamic_cast<POwnBtnStdSwitchFocus*>(Btn);
  if(p)
    return manageBtnSwitch::ON == p->getState();
  }
  return false;
}
//----------------------------------------------------------------------------
void PVarBtn::setSelOnlyStyle(bool set)
{
  if(!Btn || bStd == Style.type)
    return;
  switch(Style.show) {
    case sBmpAndText:
    case sStdAndBmp:
      setCurrAll(set);
      break;
    case sNewStdAndBmp:
      if(!Flags.colorByVar)
        setCurrColor(set);
      if(!Flags.textByVar)
        setCurrText(set);
      if(!Flags.bitmapByVar)
        setCurrBmp(set);
      break;
    }
}
//----------------------------------------------------------------------------
void PVarBtn::setSel(bool set)
{
  if(!Btn || bStd == Style.type)
    return;

  POwnBtnSwitch* p = dynamic_cast<POwnBtnSwitch*>(Btn);
  if(p)
    p->setState(set ? manageBtnSwitch::ON : manageBtnSwitch::OFF);
  else {
    POwnBtnStdSwitchFocus* p = dynamic_cast<POwnBtnStdSwitchFocus*>(Btn);
    if(p)
      p->setState(set ? manageBtnSwitch::ON : manageBtnSwitch::OFF);
    }
  setSelOnlyStyle(set);
}
//----------------------------------------------------------------------------
int PVarBtn::posFromData(pVariable& var, PVect<valPos>& val, DWORD flag, bool force)
{
  if(!flag)
    return -1;
  prfData data;
  prfData::tResultData result = var.getData(getOwner(), data, getOffs());
  if(prfData::failed >= result)
    return -1;
  if(prfData::notModified != result || force) {
    switch(flag) {
      case 1:
        return var.getIxResult(data);
      default:
        do {
          findIxByVal<valPos, double>  fibv(val, var.getNormalizedResult(data));
          if(2 == flag)
            return fibv.getPos();
          else if(3 == flag)
            return fibv.getExact();
          } while(false);
        break;
      }
    }
  return -1;
}
//----------------------------------------------------------------------------
#define MAX_NORMAL_OPEN_MODELESS (ID_INIT_VAR_BTN + 99)
#define OFFSET_BTN_OPEN_MODELES  (ADD_INIT_SIMPLE_BMP + 1000)
//----------------------------------------------------------------------------
bool PVarBtn::update(bool force)
{
  bool upd = baseActive::update(force);
  if(!isVisible() && !(bActionOpenPage == Style.type || bActionModeless == Style.type) || bHided == Style.type)
    return upd;
  int pos = posFromData(v_Text, valText, Flags.textByVar, force);
  if(pos >= 0) {
    setCurrText(pos);
    if(Flags.flat)
      invalidate();
    }

  pos = posFromData(v_Bitmap, valBmp, Flags.bitmapByVar, force);
  if(pos >= 0) {
    setCurrBmp(pos);
    invalidate();
    }
  if(Flags.colorByVar) {
    prfData data;
    prfData::tResultData result = v_Color.getData(getOwner(), data, getOffs());
    if(prfData::notModified != result || force) {
      COLORREF fg;
      COLORREF bg;
      bool success = false;
      switch(Flags.colorByVar) {
        case 1:
          success = VarColors.getColors(fg, bg, v_Color.getNormalizedResult(data));
          break;
        case 2:
          success = VarColors.getColors(fg, bg, v_Color.getIxResult(data));
          break;
        case 3:
          success = VarColors.getExactColors(fg, bg, v_Color.getNormalizedResult(data));
          break;
        }
      if(success) {
        POwnBtn::colorRect color = Btn->getColorRect();
        if(Transparent)
          color.bkg = (COLORREF)-1;
        else
          color.bkg = bg;
        color.txt = fg;
        Btn->setColorRect(color);
        if(!force)
          invalidate();
        }
      }
    }
  if(!BaseVar.getPrph() || prfData::tStrData == BaseVar.getType())
    return upd;

  prfData data;
  prfData::tResultData result = BaseVar.getData(getOwner(), data, getOffs());

  if(prfData::failed >= result) {
    getOwner()->ShowErrorData(BaseVar.getPrph(), data, result);
    return upd;
    }

  force |= upd;

  force |= prfData::notModified != result;

  bool select;

  DWORD mask = (DWORD)BaseVar.getMaskA();
  // se non c'è la maschera la si crea dal pacchetto di bit
  if(!mask) {
    if(BaseVar.getnBit())
      mask = (1 << BaseVar.getnBit()) - 1;
    else
      mask = (DWORD)-1;
    }
    // se campo di bit, basta che sia attivo almeno un bit
    // della maschera
  if(prfData::tBitData == BaseVar.getType())
    select = toBool(data.U.dw & mask);
    // altrimenti il valore deve coincidere con la maschera
  else
    select = ((data.U.dw >> BaseVar.getOffs()) & mask) == mask;

  bool neg = BaseVar.getNegA();

  select = select ^ neg;

  if(bActionOpenPage == Style.type) {
    // simula l'azione di apertura pagina, in idOpen deve esserci il
    // codice simile a quello impostato dal comando
    if(!select)
      return true;
    SEND_MSG(*getOwner(), WM_COMMAND, MAKEWPARAM(Btn->Attr.id, BN_CLICKED), (LPARAM)Btn->getHandle());
    setSel(false);
    int idOpen = getThirdExtendId();
    mainClient* mc = getParentWin<mainClient>(getOwner());
    if(mc)
      PostMessage(*mc, WM_POST_OPEN_NEW_PAGE_BODY, idOpen  - MAX_BTN, 0);
    return true;
    }


  if(bActionModeless == Style.type && openedModeless != (int)select) {
    // facciamo gestire tutto dall'oggetto dedicato
    int idOpen = getThirdExtendId();
    LPCTSTR p = getOwner()->getPageString(idOpen);
    // proviamo ad aprire una modeless anche non dal body principale
    if(p) {
      P_BaseBody* bd = dynamic_cast<P_BaseBody*>(getOwner());
      while(bd) {
        P_BaseBody* bd2 = dynamic_cast<P_BaseBody*>(bd->getParent());
        if(!bd2) {
          openedModeless = select;
          PostMessage(*bd, WM_POST_NAME_MODELESS_BODY, (WPARAM)select, (LPARAM)p);
          }
        bd = bd2;
        }
      }
    }
  if(isSelected() != select && !isPressed()) {
    setSel(select);
    force = true;
    }
  return force;
}
//----------------------------------------------------------------------------
bool PVarBtn::isPressed()
{
  // i pulsanti setOnPress hanno la loro gestione
  if(isSetOnPress())
    return false;
  {
  POwnBtnStdSwitchFocus* p = dynamic_cast<POwnBtnStdSwitchFocus*>(Btn);
  if(p)
    return p->isPressed();
  }
  POwnBtnSwitchFocus* p = dynamic_cast<POwnBtnSwitchFocus*>(Btn);
  if(p)
    return p->isPressed();

  return false;
}
//----------------------------------------------------------------------------
bool PVarBtn::needReleasePress()
{
  if(!isSetOnPress())
    return false;
  POwnBtnPress* btnP = dynamic_cast<POwnBtnPress*>(Btn);
  if(!btnP)
    return false;
  if(!btnP->isPressed())
    return false;
  btnP->switchState();
  return true;
}
//----------------------------------------------------------------------------
bool PVarBtn::makeToSend(prfData& data, bool toggle)
{
  if(!BaseVar.getPrph())
    return false;
  if(prfData::tBitData == BaseVar.getType()) {
    BaseVar.fillAttr(data, getOffs());
    if(toggle) {
      pVariable t = BaseVar;
      t.setType(prfData::tDWData);
      prfData::tResultData result = t.getData(getOwner(), data, getOffs());
      if(prfData::failed >= result) {
        getOwner()->ShowErrorData(BaseVar.getPrph(), data, result);
        return false;
        }
      }
    }
  else {
    prfData::tResultData result = BaseVar.getData(getOwner(), data, getOffs());

    if(prfData::failed >= result) {
      getOwner()->ShowErrorData(BaseVar.getPrph(), data, result);
      return false;
      }
    }

  bool selected = isSelected();

  setSel(selected);

  bool setNegative = !(BaseVar.getNegA() ^ selected);

  if(prfData::tBitData == BaseVar.getType()) {
    DWORD mask = (DWORD)BaseVar.getMaskA();
    if(!mask) {
      if(BaseVar.getnBit())
        mask = (1 << BaseVar.getnBit()) - 1;
      else
        mask = (DWORD)-1;
      }
    if(!toggle) {
      if(setNegative)
        data.typeVar |= prfData::tNegative;
      data.U.dw = MAKELONG(MAKEWORD(BaseVar.getnBit(), BaseVar.getOffs()), mask);
      }
    else {
      if(setNegative)
        data.U.dw = ~data.U.dw;
      mask <<= BaseVar.getOffs();
      DWDATA t = data.U.dw & ~mask;
      data.U.dw = ~data.U.dw;
      data.U.dw &= mask;
      data.U.dw |= t;
      }
    }
  else {
    __int64 val = BaseVar.getMaskA();
    switch(BaseVar.getType()) {
      case prfData::tFRData:
        data.U.fw = (float)*(double*)&val;
        break;
      case prfData::tRData:
        data.U.rw = *(double*)&val;
        break;
      default:
        do {
          if(val) {
            val <<= BaseVar.getOffs();
            if(BaseVar.getnBit()) {
              __int64 t = data.U.li.QuadPart;
              unsigned __int64 mask = (1 << BaseVar.getnBit()) - 1;
              uint step = (BaseVar.getOffs() / BaseVar.getnBit()) * BaseVar.getnBit();
              mask <<= step;
              t &= ~mask;
              val |= t;
              }
            data.U.li.QuadPart = val;
            }
          else if(BaseVar.getnBit() ||  BaseVar.getOffs()) {
            __int64 t = data.U.li.QuadPart;
            val = (unsigned __int64)-1;
            if(BaseVar.getnBit())
              val = (1 << BaseVar.getnBit()) - 1;
            val <<= BaseVar.getOffs();
            if(selected)
              val |= t;
            else
              val = t & ~val;

            data.U.li.QuadPart = val;
            }
          else
            data.U.li.QuadPart = val;
          } while(false);
        break;
      }
    }
  return true;
}
