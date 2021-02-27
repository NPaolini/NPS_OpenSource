//----------- loadPage.cpp --------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <commctrl.h>
#include "1.h"
#include "loadPage.h"
#include "svDialog.h"
#include "p_file.h"
#include "p_BaseBody.h"
#define USE_LVIEW
#ifdef USE_LVIEW
  #include "pCustomListView.h"
#else
  #include "pListbox.h"
#endif
#include "def_dir.h"
#include "p_ManDynaBody.h"
#include "PTreeView.h"
//----------------------------------------------------------------------------
#define ID_VAR_BTN      44

#define ID_INIT_VAR_BTN       5001
//----------------------------------------------------------------------------
#define ADD_INIT_VAR          300
#define ADD_INIT_SECOND       300
#define ADD_INIT_BITFIELD     ADD_INIT_SECOND
//----------------------------------------------------------------------------
#define PRF_MEMORY  1
#define PRF_PLC     2
//----------------------------------------------------------------------------
setOfPages::~setOfPages()
{
  reset();
}
//---------------------------------------------------------------------------
    // se result != failed torna il puntatore all'oggetto (nuovo o esistente)
    // level, in caso di esistenza, viene aggiornato al livello più alto (valore minore)
setOfPages::result setOfPages::addPage(LPCTSTR page, const infoPages* infoParent,
            uint level, uint pswLevel, infoPages** target)
{
  uint nElem = getElem();

  int upp = nElem - 1;
  int mid = 0;
  int low = 0;
  while(low <= upp) {
    mid = (upp + low) / 2;
    infoPages* ip = Pages[mid];
    int result = _tcsicmp(page, ip->getPageCurr());
    if(!result) {
      ip->setLevel(level);
      ip->setPswLevel(pswLevel);
      *target = ip;
      ip->addParent(infoParent);
      return exist;
      }
    if(result < 0)
      upp = mid - 1;
    else
      low = ++mid;
    }
  *target = new infoPages(page, infoParent, level, pswLevel);
  if(!nElem)
    Pages[nElem] = *target;
  else
    Pages.insert(*target, mid);
  return added;
}
//---------------------------------------------------------------------------
bool setOfPages::existPage(LPCTSTR page) const
{
  uint nElem = getElem();
  int upp = nElem - 1;
  int mid = 0;
  int low = 0;
  while(low <= upp) {
    mid = (upp + low) / 2;
    infoPages* ip = Pages[mid];
    int result = _tcsicmp(page, ip->getPageCurr());
    if(!result)
      return true;
    if(result < 0)
      upp = mid - 1;
    else
      low = ++mid;
    }
  return false;
}
//---------------------------------------------------------------------------
LPCTSTR setOfPages::getPage(uint pos) const
{
  uint nElem = getElem();
  if(pos >= nElem)
    return 0;
  return Pages[pos]->getPageCurr();
}
//---------------------------------------------------------------------------
/*
LPCTSTR setOfPages::getPage(LPCTSTR name) const
{
  const infoPages* ip = getPageInfo(name);
  if(ip)
    return ip->getPageCurr();

  return 0;
}
*/
//---------------------------------------------------------------------------
const infoPages* setOfPages::getPageInfo(LPCTSTR name) const
{
  uint nElem = getElem();
  for(uint i = 0; i < nElem; ++i) {
    infoPages* ip = Pages[i];
    if(!_tcsicmp(ip->getPageCurr(), name))
      return ip;
    }
  return 0;
}
//---------------------------------------------------------------------------
const infoPages* setOfPages::getPageInfo(uint pos) const
{
  uint nElem = getElem();
  if(pos >= nElem)
    return 0;
  return Pages[pos];
}
//---------------------------------------------------------------------------
extern void pageStrToSet(setOfString& sos, LPCTSTR filename);
//---------------------------------------------------------------------------
infoPages::infoPages(LPCTSTR PageCurr, const infoPages*  info_parent, uint level, uint pswLevel) :
        PageCurr(str_newdup(PageCurr)), Level(level), pswLevel(pswLevel)
{
  if(info_parent) {
    infoParent[0] = info_parent;
    }
}
//---------------------------------------------------------------------------
infoPages::~infoPages()
{
  delete []PageCurr;
}
//---------------------------------------------------------------------------
void infoPages::setPswLevel(uint level)
{
  if(!level)
    return;
  if(1000 == level && pswLevel)
    return;
  if(pswLevel < level)
    pswLevel = level;
}
//---------------------------------------------------------------------------
void infoPages::addChild(infoPages* child)
{
  uint nElem = PageChild.getElem();

  const PVect<infoPages*>&cpc = PageChild;
  for(uint i = 0; i < nElem; ++i)
    if(cpc[i] == child)
      return;
  PageChild[nElem] = child;
}
//---------------------------------------------------------------------------
void infoPages::addParent(const infoPages* parent)
{
  uint nElem = infoParent.getElem();

  const PVect<const infoPages*>&cip = infoParent;
  for(uint i = 0; i < nElem; ++i)
    if(cip[i] == parent)
      return;

  infoParent[nElem] = parent;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
managePages::managePages(P_BaseBody* owner, LPCTSTR firstPage) :
      firstPage(str_newdup(firstPage)), Root(0), currPage(0), Owner(owner)
{
}
//----------------------------------------------------------------------------
managePages::~managePages()
{
  delete []firstPage;
}
//----------------------------------------------------------------------------
#define MAX_FUNCT_BTN 12

#define ID_INIT_FUNCT_BTN 14
//----------------------------------------------------------------------------
void managePages::addFromFunct(LPCTSTR name, infoPages* iP, uint level, uint pswLevel)
{
  if(!name)
    return;

  infoPages* child = loadPage(name, iP, level + 1, pswLevel);
  if(child)
    iP->addChild(child);
}
//----------------------------------------------------------------------------
LPCTSTR checkAction(uint id, LPCTSTR p, uint& pswLevel)
{
  pswLevel = 0;
  p = findNextParamTrim(p);
  if(!p)
    return 0;
  pswLevel = _ttoi(p);
  switch(id) {
    case 1: // new body standard or by id
      return findNextParam(p, 2);

    case 6: // open trend
      return findNextParam(p, 1);

    case 7:  // open recipe
      do {
        LPCTSTR p2 = findNextParam(p, 2);
        if(p2)
          p = p2;
        else
          p = findNextParam(p, 1);
        } while(false);
      return p;

    case 8:  // open recipe row
      return findNextParam(p, 1);
/*
    case 9: // open modal
      act = infoPrph::eOpenModal;
      return findNextParam(p, 1);

    case 11: // open modeless
      act = infoPrph::eOpenModeless;
      return findNextParam(p, 1);
*/
    case 18: // open maint
      return findNextParam(p, 1);
    }
  return 0;
}
//----------------------------------------------------------------------------
#define ID_SHIFT_F1   90001
#define ID_CTRL_F1    90031
//----------------------------------------------------------------------------
#define ID_SHIFT_F1_ACT  (ID_SHIFT_F1 + MAX_FUNCT_BTN)
#define ID_CTRL_F1_ACT   (ID_CTRL_F1 + MAX_FUNCT_BTN)
//----------------------------------------------------------------------------
void managePages::checkMenu(infoPages* iP, setOfString& sos, uint level)
{
  // verifica tasti funzione
  uint ids[] = { ID_INIT_FUNCT_BTN, ID_SHIFT_F1_ACT, ID_CTRL_F1_ACT };
  for(uint j = 0; j < SIZE_A(ids); ++j) {
    for(uint i = 0; i < MAX_FUNCT_BTN; ++i) {
      LPCTSTR p = sos.getString(ids[j] + i);
      if(!p)
        continue;
      uint id = _ttoi(p);
      uint pswLevel;
      p = checkAction(id, p, pswLevel);
      if(p)
        addFromFunct(p, iP, level, pswLevel);
      }
    }
}
//----------------------------------------------------------------------------
void managePages::checkBtn(infoPages* iP, setOfString& sos, uint level)
{
  LPCTSTR p = sos.getString(ID_VAR_BTN);
  if(!p)
    return;

  int nObj = _ttoi(p);
  for(int i = 0; i < nObj; ++i) {
    manageObjId moi(0, ID_INIT_VAR_BTN);
    uint id = moi.calcAndSetExtendId(i);
    uint idAdd = moi.getFirstExtendId();
    p = sos.getString(idAdd);
    p = findNextParam(p, 5);
    if(!p)
      continue;
    uint typeCommand = _ttoi(p);

    switch(typeCommand) {
      case 0:
        checkActionBtn(iP, sos, level, id);
        break;
      }
    }
}
//----------------------------------------------------------------------------
void managePages::addFromBtn(LPCTSTR name, infoPages* iP, uint level, uint pswLevel)
{
  addFromFunct(name, iP, level, pswLevel);
}
//----------------------------------------------------------------------------
void managePages::checkActionBtn(infoPages* iP, setOfString& sos, uint level, uint idInit)
{
  uint idAdd = manageObjId(idInit, ID_INIT_VAR_BTN).getFirstExtendId();
  LPCTSTR p = sos.getString(idAdd);
  if(!p || _ttoi(p))
    return;
  p = findNextParam(p, 1);
  if(!p)
    return;
  uint id = _ttoi(p);
  if(id >= 1 && id <= 12)
    id += 13;
  else if(id >= 13 && id <= 24)
    id += ID_SHIFT_F1_ACT - 13;
  else if(id >= 25 && id <= 36)
    id += ID_CTRL_F1_ACT - 25;

  p = sos.getString(id);
  if(!p)
    return;
  id = _ttoi(p);

  uint pswLevel;
  p = checkAction(id, p, pswLevel);
  if(p)
    addFromBtn(p, iP, level, pswLevel);
}
//----------------------------------------------------------------------------
static void load_page(setOfString& sos, LPCTSTR filename)
{
  TCHAR t[_MAX_PATH];
  getFileStrCheckRemote(t, filename);
  pageStrToSet(sos, t);
}
//----------------------------------------------------------------------------
// la prima chiamata avviene con infoPages* root = loadPage("page1.txt", 0, 0);
//----------------------------------------------------------------------------
infoPages* managePages::loadPage(LPCTSTR filename, infoPages* iP_Parent, uint level, uint pswLevel)
{
  infoPages* iP = 0;

  setOfPages::result res = soPages.addPage(filename, iP_Parent, level, pswLevel, &iP);
  if(setOfPages::failed == res)
    return 0;

  if(setOfPages::exist == res) {
    if(!iP_Parent)
      return 0;
    return iP;
    }
  if(!iP)
    return 0;

  setOfString sos;
  load_page(sos, filename);

  LPCTSTR p = sos.getString(ID_PAGE_DYNAMIC_LINK_V2);
  while(p) {
    int offsX = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      break;
    int offsY = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      break;

    setOfString tmpSet;
    load_page(tmpSet, p);

    if(!tmpSet.setFirst())
      break;
    POINT pt = { offsX, offsY };
    P_ManDynaBody(Owner, sos, tmpSet, pt).run();

    break;
    }

  checkMenu(iP, sos, level);
  checkBtn(iP, sos, level);

  return iP;
}
//----------------------------------------------------------------------------
void managePages::loadStdMsg(setOfString& sos)
{
  TCHAR t[_MAX_PATH] = STD_MSG_NAME;
  getRelPath(t, dSystem);
  pageStrToSet(sos, t);
}
//----------------------------------------------------------------------------
bool managePages::makeAll()
{
  soPages.reset();
  // va chiamata prima per memorizzare l'id del plc reale
  sosStdMsg.reset();
  loadStdMsg(sosStdMsg);

  Root = loadPage(firstPage, 0, 0, 0);
  currPage = 0;
  sosStdMsg.reset();
  return toBool(Root);
}
//----------------------------------------------------------------------------
bool managePages::setFirstPage()
{
  currPage = 0;
  return toBool(soPages.getElem());
}
//----------------------------------------------------------------------------
bool managePages::setLastPage()
{
  uint nElem = soPages.getElem();
  if(nElem) {
    currPage = nElem - 1;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool managePages::setPrevPage()
{
  if(currPage > 0) {
    --currPage;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool managePages::setNextPage()
{
  uint nElem = soPages.getElem();
  if(currPage < nElem - 1) {
    ++currPage;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool managePages::getCurr(const infoPages* &target)
{
  uint nElem = soPages.getElem();
  if(nElem) {
    target = soPages.getPageInfo(currPage);
    return true;
    }
  return toBool(target);
}
//----------------------------------------------------------------------------
/*
IDD_CHOOSE_PAGE DIALOGEX 12, 17, 141, 233
STYLE DS_ABSALIGN | DS_SETFONT | DS_MODALFRAME | DS_3DLOOK | DS_FIXEDSYS | DS_CENTER | WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU
CAPTION "Scelta Pagina"
FONT 10, "MS Shell Dlg", 0, 0, 0x0
BEGIN
    LISTBOX         IDC_LISTBOX_CHOOSE_PAGE,3,18,134,214,LBS_OWNERDRAWVARIABLE | LBS_HASSTRINGS | WS_VSCROLL | WS_TABSTOP
    PUSHBUTTON      "F1_Ok",IDC_BUTTON_F1,2,2,45,14
    PUSHBUTTON      "F2_Esc",IDC_BUTTON_F2,47,2,45,14
    PUSHBUTTON      "F3_Refresh",IDC_BUTTON_F3,92,2,45,14
END
*/
//----------------------------------------------------------------------------
#define FILENAME_SAVE_PAGE_TREE _T("pageTree.tmp")
//------------------------------------------------------------------
class wrapLPCTSTR
{
  public:
    wrapLPCTSTR() : Obj(0) {}
    wrapLPCTSTR(LPCTSTR obj) : Obj(obj) {}
    operator LPCTSTR() const { return Obj; }
    bool operator <(const wrapLPCTSTR& other) { return _tcscmp(Obj, other.Obj) < 0; }
  private:
    LPCTSTR Obj;
};
//------------------------------------------------------------------
class my_ManageRowOfBuff : public manageRowOfBuff
{
  private:
    typedef manageRowOfBuff baseClass;
  public:
    my_ManageRowOfBuff(LPCTSTR filename, PVect<LPCTSTR>& rows) :
      baseClass(filename), Rows(rows) {}
  protected:
    virtual bool manageLine(LPCTSTR row) { Rows[Rows.getElem()] = str_newdup(row); return true; }
  private:
    PVect<LPCTSTR>& Rows;
};
//----------------------------------------------------------------------------
class svChoosePageGen : public svModDialog
{
  private:
    typedef svModDialog baseClass;
  public:
    svChoosePageGen(setOfString& set, PWin* parent, uint id, HINSTANCE hInst = 0) :
      baseClass(parent, id, hInst), Set(set), useStack(true) {}
    ~svChoosePageGen() { destroy(); }

    bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    virtual void resetContent() = 0;
    virtual void addRow(LPCTSTR row) = 0;
    virtual void getPage(LPTSTR page) = 0;

    virtual void prepare() {}
    virtual void endprepare() {}

    void fillRow(LPTSTR row, const infoPages* ipage, uint level);
    uint unfillRow(LPTSTR row, LPCTSTR source);

    void CmOk() { makeResult(); }
//    void CmCancel() {}
    virtual void refresh();
    void makeResult();

    void addPage(P_File& pf, const infoPages* ipage, LPTSTR tmp, uint level);

    setOfString& Set;
    PVect<wrapLPCTSTR> Loaded;
    bool useStack;
};
//----------------------------------------------------------------------------
#define ID_TITLE_CHOOSE_PAGE 77
#define ID_F1_CHOOSE_PAGE 74
#define ID_F2_CHOOSE_PAGE 75
#define ID_F3_CHOOSE_PAGE 76
//----------------------------------------------------------------------------
bool svChoosePageGen::create()
{
  if(!baseClass::create())
    return false;
  LPCTSTR p = getString(ID_NO_MEM_PATH_PAGE);
  useStack = !p || !_ttoi(p);
  prepare();
  setWindowTextByLangGlob(*this, ID_TITLE_CHOOSE_PAGE, false);
  setWindowTextByLangGlob(GetDlgItem(*this, IDC_BUTTON_F1), ID_F1_CHOOSE_PAGE, false);
  setWindowTextByLangGlob(GetDlgItem(*this, IDC_BUTTON_F2), ID_F2_CHOOSE_PAGE, false);
  setWindowTextByLangGlob(GetDlgItem(*this, IDC_BUTTON_F3), ID_F3_CHOOSE_PAGE, false);

  PVect<LPCTSTR> Rows;
  do {
    my_ManageRowOfBuff(FILENAME_SAVE_PAGE_TREE, Rows).run();
    } while(false);
  uint nElem = Rows.getElem();
  if(!nElem)
    refresh();
  else {
    for(uint i = 0; i < nElem; ++i)
      addRow(Rows[i]);
    flushPV(Rows);
    }
  endprepare();
  return true;
}
//----------------------------------------------------------------------------
LRESULT svChoosePageGen::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_F1:
          makeResult();
          break;
        case IDC_BUTTON_F2:
          baseClass::CmCancel();
          break;
        case IDC_BUTTON_F3:
          refresh();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void svChoosePageGen::fillRow(LPTSTR row, const infoPages* ipage, uint level)
{
  TCHAR page[_MAX_PATH];
  _tcscpy_s(page, ipage->getPageCurr());
  int len = _tcsclen(page);
  for(int i = len - 1; i > 0; --i) {
    if(_T('.') == page[i]) {
      page[i] = 0;
      break;
      }
    }
  wsprintf(row, _T("[%d]\t{psw %d}\t%s  "), level, ipage->getPswLevel(), page);
}
//----------------------------------------------------------------------------
uint svChoosePageGen::unfillRow(LPTSTR row, LPCTSTR source)
{
  TCHAR tmp[_MAX_PATH];
  _tcscpy_s(tmp, source);
  row[0] = 0;
  LPCTSTR p = tmp;
  while(*p && _T(']') != *p)
    ++p;
  if(*p) {
    ++p;
    while(*p && *p <= _T(' '))
      ++p;
    }
  uint psw = 0;
  for(int i = 0; *p && i < _MAX_PATH; ++i, ++p) {
    if(_T('{') == *p) {
      while(*p && !_istdigit(*p))
        ++p;
      psw = _ttoi(p);
      }
    else if(_T('}') == *p) {
      ++p;
      while(*p && _T(' ') >= *p)
        ++p;
      break;
      }
    }
  int i = 0;
  for(; *p && i < _MAX_PATH; ++i, ++p)
    row[i] = *p;
  row[i] = 0;
  trim(row);
  _tcscat_s(row, _MAX_PATH, PAGE_EXT);
  return psw;
}
//----------------------------------------------------------------------------
extern bool isMainPage(LPCTSTR page);
//----------------------------------------------------------------------------
void svChoosePageGen::addPage(P_File& pf, const infoPages* ipage, LPTSTR tmp, uint level)
{
  if(*ipage->getPageCurr()) {
    if(!(useStack && isMainPage(ipage->getPageCurr()))) {
      wrapLPCTSTR wrap(ipage->getPageCurr());
      uint pos;
      if(!Loaded.find(wrap, pos)) {
        fillRow(tmp, ipage, level);
        pf.P_writeString(tmp);
        pf.P_writeString(_T("\r\n"));
        addRow(tmp);
        Loaded.insert(wrap);
        }
      }
    }
  const PVect<infoPages*>& child = ipage->getChild();
  uint nElem = child.getElem();
  for(uint i = 0; i < nElem; ++i)
    if(child[i]->getLevel() > ipage->getLevel())
      addPage(pf, child[i], tmp, level + 1);
}
//----------------------------------------------------------------------------
void svChoosePageGen::refresh()
{
  resetContent();
  Loaded.reset();
  P_BaseBody* owner = getParentWin<P_BaseBody>(this);
  managePages manP(owner, FIRST_PAGE);
  if(manP.makeAll()) {
    const infoPages* root = manP.getRoot();
    if(!root)
      return;
    P_File pf(FILENAME_SAVE_PAGE_TREE, P_CREAT);
    if(!pf.P_open())
      return;
    TCHAR tmp[_MAX_PATH];
    addPage(pf, root, tmp, 0);
    }
}
//----------------------------------------------------------------------------
#define ID_CODE_PAGE_TYPE 999999
//----------------------------------------------------------------------------
void svChoosePageGen::makeResult()
{
  TCHAR page[_MAX_PATH] = { 0 };
  getPage(page);
  if(!*page)
    return;
  uint psw = unfillRow(page, page);

  setOfString sos;
  load_page(sos, page);
  LPCTSTR p = sos.getString(ID_CODE_PAGE_TYPE);
  int code = 0;
  if(p)
    code = _ttoi(p);
  enum pageType {
    Pt_STANDARD,
    Pt_RECIPE,
    Pt_RECIPE_LB,
    Pt_RECIPE_ROW,
    Pt_GRAPH_TREND,
    Pt_MAINT
    };
  TCHAR inside[24] = { 0 };
  TCHAR buff[_MAX_PATH] = { 0 };
  switch(code) {
    case Pt_STANDARD:
    default:
      wsprintf(buff, _T("1,%d,0,"), psw);
      break;
    case Pt_RECIPE:
      wsprintf(buff, _T("7,%d,"), psw);
      break;
    case Pt_RECIPE_LB:
      wsprintf(buff, _T("7,%d,1,"), psw);
      break;
    case Pt_RECIPE_ROW:
      wsprintf(buff, _T("8,%d,"), psw);
      break;
    case Pt_GRAPH_TREND:
      wsprintf(buff, _T("6,%d,"), psw);
      break;
    case Pt_MAINT:
      wsprintf(buff, _T("18,%d,1,"), psw);
      break;
    }
  _tcscat_s(buff, page);
  Set.replaceString(ID_OPEN_PAGE_BY_CHOOSE, str_newdup(buff), true);
  baseClass::CmOk();
}
//-------------------------------------------------------------------
static
LPCTSTR makeColor(COLORREF& target, LPCTSTR p)
{
  if(p)  {
    int r = 255;
    int g = 255;
    int b = 255;
    _stscanf_s(p, _T("%d,%d,%d"), &r, &g, &b);
    target = RGB(r, g, b);
    p = findNextParamTrim(p, 3);
    }
  return p;
}
//----------------------------------------------------------------------------
class loadfontAndColors
{
  public:
    loadfontAndColors() : hF(0) {}
    ~loadfontAndColors() { if(hF) DeleteObject(hF); }

    bool init();
    HFONT getFont() { HFONT t = hF; hF = 0; return t; }
    COLORREF getFgNorm() const { return FgNorm; }
    COLORREF getBgNorm() const { return BgNorm; }
    COLORREF getFgSel() const { return FgSel; }
    COLORREF getBgSel() const { return BgSel; }

  private:
    COLORREF FgNorm;
    COLORREF BgNorm;
    COLORREF FgSel;
    COLORREF BgSel;
    HFONT hF;
};
//----------------------------------------------------------------------------
bool loadfontAndColors::init()
{
  LPCTSTR p = getString(ID_COLOR_FONT_CHOOSE_PAGE);
  if(!p)
    return false;

  p = makeColor(FgNorm, p);
  p = makeColor(BgNorm, p);
  p = makeColor(FgSel, p);
  p = makeColor(BgSel, p);
  if(p) {
    uint h = _ttoi(p);
    p = findNextParamTrim(p);
    if(p)
      hF = D_FONT(h, 0, 0, p);
    }
  return true;
}
//----------------------------------------------------------------------------
#ifdef USE_LVIEW
  #define IDD_CHOOSE_LIST__ IDD_CHOOSE_PAGE_LISTVIEW
#else
  #define IDD_CHOOSE_LIST__ IDD_CHOOSE_PAGE
#endif
//----------------------------------------------------------------------------
class svChoosePageList : public svChoosePageGen
{
  private:
    typedef svChoosePageGen baseClass;
  public:
    svChoosePageList(setOfString& set, PWin* parent, uint id = IDD_CHOOSE_LIST__, HINSTANCE hInst = 0) :
      baseClass(set, parent, id, hInst), hlb(0), hdcLB(0), hfOld(0), awi(0), added(0), im(0) {}
    ~svChoosePageList();
    bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    virtual void resetContent();
    virtual void addRow(LPCTSTR row);
    virtual void getPage(LPTSTR page);
    virtual void refresh();
    virtual void prepare();
    virtual void endprepare();
    HWND hlb;
    HDC hdcLB;
    HFONT hfOld;
    int awi;
    int added;
    PVect<LPCTSTR> ordered;
#ifdef USE_LVIEW
    loadfontAndColors lfc;
    void addRow(HWND hlb, LPCTSTR row);
    HIMAGELIST im;
#endif
};
//----------------------------------------------------------------------------
svChoosePageList::~svChoosePageList()
{
#ifdef USE_LVIEW
  if(im)
    ImageList_Destroy(im);
#endif
  if(hfOld)
    SelectObject(hdcLB, hfOld);
  ReleaseDC(hlb, hdcLB);
  destroy();
}
//----------------------------------------------------------------------------
#ifndef LVS_EX_FLATSB
  #define LVS_EX_FLATSB           0x00000100
#endif
//----------------------------------------------------------------------------
#define RET_COLOR(a) do { if(!lfc) return false; c = lfc->a(); return true; } while(false)
//----------------------------------------------------------------------------
class myListView : public PCustomListView
{
  private:
    typedef PCustomListView baseClass;
  public:
    myListView(PWin * parent, uint resid, loadfontAndColors* lfc, HINSTANCE hinst = 0) :
          baseClass(parent, resid, hinst), lfc(lfc) {  }
    virtual bool ProcessNotify(LPARAM lParam, LRESULT& result);
    bool create();
  protected:
    virtual bool getColorBkg(COLORREF& c) { RET_COLOR(getBgNorm); }
    virtual bool getColorTextBkg(COLORREF& c) { RET_COLOR(getBgNorm); }
    virtual bool getColorText(COLORREF& c) { RET_COLOR(getBgNorm); }
    virtual bool getColorTextBkgSel(COLORREF& c) { RET_COLOR(getBgSel); }
    virtual bool getColorTextSel(COLORREF& c) { RET_COLOR(getFgSel); }
  private:
    loadfontAndColors* lfc;
    void init();
};
//----------------------------------------------------------------------------
bool myListView::create()
{
  if(!baseClass::create())
    return false;
  if(lfc)
    setFont(lfc->getFont(), true);
  return true;
}
//----------------------------------------------------------------------------
bool myListView::ProcessNotify(LPARAM lParam, LRESULT& result)
{
  if(!lfc)
    return baseClass::ProcessNotify(lParam, result);
  LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW) lParam;
  if(!lplvcd)
    return false;
  if(NM_CUSTOMDRAW != ((LPNMHDR)lParam)->code)
    return false;

  result = CDRF_DODEFAULT;
  switch (lplvcd->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
      result = CDRF_NOTIFYITEMDRAW;
      break;
   case CDDS_ITEMPREPAINT:
      if((CDIS_SELECTED | CDIS_FOCUS) & lplvcd->nmcd.uItemState) {
        lplvcd->clrText   = lfc->getFgSel();
        lplvcd->clrTextBk = lfc->getBgSel();
        result = CDRF_NEWFONT;
        }
      break;
   }
  return true;
}
//----------------------------------------------------------------------------
bool svChoosePageList::create()
{
#ifdef USE_LVIEW
  bool success = lfc.init();
  myListView* lb = new myListView(this, IDC_LISTBOX_CHOOSE_PAGE, success ? &lfc : 0);
  if(!baseClass::create())
    return false;
#else
  int tabs[] = { 3, 7, 255 };
  int show[] = { 0, 1, 1 };
  PListBox* lb = new PListBox(this, IDC_LISTBOX_CHOOSE_PAGE);
  lb->SetTabStop(SIZE_A(tabs), tabs, show);
  lb->setAlign(0, PListBox::aCenter);
  if(!baseClass::create())
    return false;
  lb->setIntegralHeight();
  loadfontAndColors lfc;
  if(lfc.init()) {
    lb->SetColor(lfc.getFgNorm(), lfc.getBgNorm());
    lb->SetColorSel(lfc.getFgSel(), lfc.getBgSel());
    lb->setFont(lfc.getFont(), true);
    }
#endif
  return true;
}
//----------------------------------------------------------------------------
//static bool sendedFont;
#define LV_BKG_COLOR RGB(255, 0, 0)
//----------------------------------------------------------------------------
void svChoosePageList::prepare()
{
  if(hlb)
    return;
  hlb = GetDlgItem(*this, IDC_LISTBOX_CHOOSE_PAGE);
#ifdef USE_LVIEW
  PWin * w = PWin::getWindowPtr(hlb);
  PCustomListView *lb = dynamic_cast<PCustomListView*>(w);
  SendMessage (hlb, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

  LVCOLUMN lvc;
  lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
  lvc.fmt = LVCFMT_CENTER;
  PRect r;
  GetClientRect(*this, r);
  r.Inflate(-10, 0);
  lvc.cx = r.Width() / 4;
  lvc.pszText = _T("Level");
  lvc.cchTextMax = _tcslen(lvc.pszText);
  ListView_InsertColumn (hlb, 0, &lvc);

  lvc.pszText = _T("Password");
  lvc.cchTextMax = _tcslen(lvc.pszText);
  ListView_InsertColumn (hlb, 1, &lvc);

  lvc.cx = r.Width() / 2;
  lvc.pszText = _T("Page");
  lvc.cchTextMax = _tcslen(lvc.pszText);
  ListView_InsertColumn (hlb, 2, &lvc);

#else
  hdcLB = GetDC(hlb);
  HFONT hfNew = (HFONT)SendMessage(hlb, WM_GETFONT, 0, 0);
  if(hfNew)
    hfOld = (HFONT)SelectObject(hdcLB, hfNew);
  TEXTMETRIC tm;
  GetTextMetrics(hdcLB, &tm);
  added = tm.tmAveCharWidth * 2;
  PRect r;
  SendMessage(hlb, LB_GETITEMRECT, 0, (LPARAM)(LPRECT)r);
  awi = r.Width() - added;
  SendMessage(hlb, LB_SETHORIZONTALEXTENT, awi, 0);
#endif
}
//----------------------------------------------------------------------------
static int fzCmpStr(const LPCTSTR& v, const LPCTSTR& ins)
{
  LPCTSTR p1 = v;
  while(*p1 && *p1 != _T('}'))
    ++p1;
  LPCTSTR p2 = ins;
  while(*p2 && *p2 != _T('}'))
    ++p2;
  return _tcsicmp(p1, p2);
}
//----------------------------------------------------------------------------
#ifdef USE_LVIEW

static LPCTSTR copyToTab(LPTSTR target, LPCTSTR source)
{
  while(source && *source) {
    if(*source < _T(' ')) {
      *target = 0;
      if(*source)
        return source + 1;
      return 0;
      }
    else
      *target++ = *source;
    ++source;
    }
  *target = 0;
  return 0;
}
//----------------------------------------------------------------------------
static void removeBracket(LPTSTR target)
{
  TCHAR t[_MAX_PATH];
  _tcscpy_s(t, target);
  int i = 0;
  for(; t[i]; ++i)
    if(_istdigit(t[i]))
      break;
  int j = 0;
  for(; _istdigit(t[i]); ++j, ++i)
    target[j] = t[i];
  target[j] = 0;
}
//----------------------------------------------------------------------------
void svChoosePageList::addRow(HWND hlb, LPCTSTR row)
{
  LVITEM lvI;
  lvI.mask = LVIF_TEXT;
  lvI.iItem = 65535;
  lvI.iSubItem = 0;
  TCHAR textItem[_MAX_PATH];
  row = copyToTab(textItem, row);
  removeBracket(textItem);
  lvI.pszText = textItem;

  int curr = SendMessage(hlb, LVM_INSERTITEM, 0, (LPARAM) &lvI);
  lvI.iItem = curr;
  row = copyToTab(textItem, row);
  removeBracket(textItem);
  ListView_SetItemText(hlb, curr, 1, textItem);

  row = copyToTab(textItem, row);
  ListView_SetItemText(hlb, curr, 2, textItem);
}
//----------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------
void svChoosePageList::endprepare()
{
  if(!hlb)
    return;
#ifdef USE_LVIEW
  uint nElem = ordered.getElem();
  for(uint i = 0; i < nElem; ++i)
    addRow(hlb, ordered[i]);
#else
  int oldawi = (int)SendMessage(hlb, LB_GETHORIZONTALEXTENT, 0, 0);
  if(oldawi < awi) {
    int delta =  awi - oldawi + added;
    PRect r;
    GetWindowRect(*this, r);
    SetWindowPos(*this, 0, 0, 0, r.Width() + delta, r.Height(), SWP_NOZORDER | SWP_NOMOVE);
    GetWindowRect(hlb, r);
    SetWindowPos(hlb, 0, 0, 0, r.Width() + delta, r.Height(), SWP_NOZORDER | SWP_NOMOVE);
    SendMessage(hlb, LB_SETHORIZONTALEXTENT, awi, 0);
    }
  uint nElem = ordered.getElem();
  for(uint i = 0; i < nElem; ++i)
    SendMessage(hlb, LB_ADDSTRING, 0, (LPARAM)ordered[i]);
#endif
  flushPAV(ordered);
}
//----------------------------------------------------------------------------
LRESULT svChoosePageList::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
#ifndef USE_LVIEW
      switch(LOWORD(wParam)) {
        case IDC_LISTBOX_CHOOSE_PAGE:
          if(LBN_DBLCLK == HIWORD(wParam))
            makeResult();
          break;
        }
#endif
      break;
#ifdef USE_LVIEW
    case WM_LBUTTONDBLCLK:
      PostMessage(*getParent(), WM_COMMAND, MAKEWPARAM(IDC_BUTTON_F1, 0), 0);
      break;

    case WM_NOTIFY:
      if(LOWORD (wParam) != IDC_LISTBOX_CHOOSE_PAGE)
        break;

      switch (((LPNMHDR)lParam)->code) {
        case NM_CUSTOMDRAW:
          do {
            PCustomListView* lv = dynamic_cast<PCustomListView*>(PWin::getWindowPtr(((LPNMHDR)lParam)->hwndFrom));
            if(lv) {
              LRESULT res;
              if(lv->ProcessNotify(lParam, res)) {
                SetWindowLongPtr(hwnd, DWLP_MSGRESULT, res);
                return TRUE;
                }
              }
            } while(false);
          break;
        }
      break;
#endif
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void svChoosePageList::resetContent()
{
#ifdef USE_LVIEW
  ListView_DeleteAllItems(GetDlgItem(*this, IDC_LISTBOX_CHOOSE_PAGE));
#else
  awi = (int)SendMessage(hlb, LB_GETHORIZONTALEXTENT, 0, 0);
  SendMessage(GetDlgItem(*this, IDC_LISTBOX_CHOOSE_PAGE), LB_RESETCONTENT, 0, 0);
#endif
}
//----------------------------------------------------------------------------
void svChoosePageList::addRow(LPCTSTR row)
{
#ifndef USE_LVIEW
  calcExt(hdcLB, row, awi);
#endif
  ordered.insertEx(str_newdup(row), fzCmpStr);
}
//----------------------------------------------------------------------------
void svChoosePageList::refresh()
{
  prepare();
  baseClass::refresh();
  endprepare();
}
//----------------------------------------------------------------------------
void svChoosePageList::getPage(LPTSTR page)
{
  page[0] = 0;
  HWND hwl = GetDlgItem(*this, IDC_LISTBOX_CHOOSE_PAGE);
#ifdef USE_LVIEW
  int cursel = ListView_GetNextItem(hwl, -1, LVNI_SELECTED);
  if(cursel >= 0) {
    TCHAR lev[10];
    TCHAR psw[10];
    TCHAR pg[_MAX_PATH];
    ListView_GetItemText(hwl, cursel, 0, lev, SIZE_A(lev));
    ListView_GetItemText(hwl, cursel, 1, psw, SIZE_A(psw));
    ListView_GetItemText(hwl, cursel, 2, pg, _MAX_PATH);
    wsprintf(page, _T("[%s] {%s} %s"), lev, psw, pg);
    }
#else
  int cursel = SendMessage(hwl, LB_GETCURSEL, 0, 0);
  if(cursel >= 0)
    SendMessage(hwl, LB_GETTEXT, cursel, (LPARAM)page);
#endif
}
//----------------------------------------------------------------------------
bool choosePageByList(PWin* parent, setOfString& set)
{
  return IDOK == svChoosePageList(set, parent).modal();
}
//----------------------------------------------------------------------------
class myPTreeView : public PTreeView
{
  private:
    typedef PTreeView baseClass;
  public:
    myPTreeView(PWin * parent, uint resid, HINSTANCE hinst = 0) :
      baseClass(parent, resid, hinst) {}

    ~myPTreeView() { destroy(); }
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};
//----------------------------------------------------------------------------
LRESULT myPTreeView::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_LBUTTONDBLCLK:
      PostMessage(*getParent(), WM_COMMAND, MAKEWPARAM(IDC_BUTTON_F1, 0), 0);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
struct TV_Color
{
  COLORREF Fg_N;
  COLORREF Bg_N;
  COLORREF Fg_S;
  COLORREF Bg_S;
  TV_Color() : Fg_N(0), Bg_N(RGB(255, 255, 255)), Fg_S(RGB(255, 255, 127)), Bg_S(RGB(0, 0, 127)) {}
};
//----------------------------------------------------------------------------
class svChoosePageTreeView : public svChoosePageGen
{
  private:
    typedef svChoosePageGen baseClass;
  public:
    svChoosePageTreeView(setOfString& set, PWin* parent, uint id = IDD_CHOOSE_PAGE_TREEVIEW, HINSTANCE hInst = 0) :
      baseClass(set, parent, id, hInst) {}
    ~svChoosePageTreeView();
    bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void resize();
    virtual void resetContent();
    virtual void addRow(LPCTSTR row);
    virtual void getPage(LPTSTR page);
    virtual void refresh();
    virtual void endprepare();

//    PRect minRect;
    PVect<LPCTSTR> Rows;

    void fillTreeView(const PVect<LPCTSTR>& rows);
    void fillTreeView(HTREEITEM root, const PVect<LPCTSTR>& rows, uint& ix, int currLevel);

    TV_Color Colors;
//    COLORREF Fg;
//    COLORREF Bg;
};
//----------------------------------------------------------------------------
svChoosePageTreeView::~svChoosePageTreeView()
{
  destroy();
}
//----------------------------------------------------------------------------
static PRect minRect;
static PRect currRect;
//----------------------------------------------------------------------------
bool svChoosePageTreeView::create()
{
  myPTreeView* tv = new myPTreeView(this, IDC_TREE1);
  if(!baseClass::create())
    return false;
  if(!minRect.Width())
    GetWindowRect(*this, minRect);
  if(currRect.Width())
    SetWindowPos(*this, 0, currRect.left, currRect.top, currRect.Width(), currRect.Height(), SWP_NOZORDER);
  resize();
  loadfontAndColors lfc;
  if(lfc.init()) {
#if 1
    Colors.Fg_N = lfc.getFgNorm();
    Colors.Bg_N = lfc.getBgNorm();
    Colors.Fg_S = lfc.getFgSel();
    Colors.Bg_S = lfc.getBgSel();
#else
    TreeView_SetBkColor(*tv, lfc.getBgNorm());
    TreeView_SetTextColor(*tv, lfc.getFgNorm());
    Fg = lfc.getFgSel();
    Bg = lfc.getBgSel();
#endif
    tv->setFont(lfc.getFont(), true);
    }
  return true;
}
//----------------------------------------------------------------------------
LPCTSTR getLevel(LPCTSTR row, int& level)
{
  level = 0;
  if(!row)
    return 0;
  LPCTSTR p = row;
  while(*p && !_istdigit(*p))
    ++p;
  if(!*p)
    return 0;
  level = _ttoi(p);
  while(*p && _T('{') != *p)
    ++p;

  return p;
}
//----------------------------------------------------------------------------
void svChoosePageTreeView::fillTreeView(const PVect<LPCTSTR>& rows)
{
  if(!rows.getElem())
    return;
  bool hasPage1 = false;
  do {
    TCHAR page[_MAX_PATH] = { 0 };
    unfillRow(page, rows[0]);
    hasPage1 = isMainPage(page);
    } while(false);
  uint ix = 0;
  int level;
  getLevel(rows[0], level);
  fillTreeView(0, rows, ix, level);
  if(hasPage1) {
    HWND hwTv = GetDlgItem(*this, IDC_TREE1);
    HTREEITEM root = TreeView_GetRoot(hwTv);
    TreeView_Expand(hwTv, root, TVE_EXPAND);
    }
}
//----------------------------------------------------------------------------
static void copyNoTab(LPTSTR target, LPCTSTR source)
{
  while(*source) {
    if(*source < _T(' '))
      *target++ = _T(' ');
    else
      *target++ = *source;
    ++source;
    }
  *target = 0;
}
//----------------------------------------------------------------------------
void svChoosePageTreeView::fillTreeView(HTREEITEM root, const PVect<LPCTSTR>& rows, uint& ix, int currLevel)
{
  HWND hwTv = GetDlgItem(*this, IDC_TREE1);
  HTREEITEM node = TVI_FIRST;
  uint nElem = rows.getElem();
  for(; ix < nElem; ++ix) {
    int level;
    LPCTSTR p = getLevel(rows[ix], level);
    if(!p || !*p)
      break;
    if(level < currLevel)
      break;
    if(level > currLevel) {
      fillTreeView(node, rows, ix, currLevel + 1);
      --ix;
      }
    else {
      TCHAR buff[500];
      copyNoTab(buff, p);
      TV_INSERTSTRUCT is;
      is.hParent = root;
      is.hInsertAfter = node;
      is.item.mask = TVIF_HANDLE | TVIF_TEXT;
      is.item.hItem = 0;
      is.item.pszText = buff;

      node = TreeView_InsertItem(hwTv, &is);
      }
    }
}
//----------------------------------------------------------------------------
void svChoosePageTreeView::endprepare()
{
  fillTreeView(Rows);
  flushPAV(Rows);
}
//----------------------------------------------------------------------------
void svChoosePageTreeView::resize()
{
  GetWindowRect(*this, currRect);
  PRect r(currRect);
  HWND htv = GetDlgItem(*this, IDC_TREE1);
  PRect r2;
  GetWindowRect(htv, r2);
  int left = r2.left - r.left;
  int top = r2.top - r.top;
  r.top += top;
  r.right -= left;
  SetWindowPos(htv, 0, 0, 0, r.Width(), r.Height(), SWP_NOMOVE | SWP_NOZORDER);
  uint pix = TreeView_GetIndent(htv);
  TreeView_SetIndent(htv, pix + 1);
  TreeView_SetIndent(htv, pix);
}
//----------------------------------------------------------------------------
static LRESULT ProcessNotify(LPARAM lParam, const TV_Color& colors)
{
  LPNMTVCUSTOMDRAW  lptvcd = (LPNMTVCUSTOMDRAW) lParam;
  if(!lptvcd)
    return -1;
  switch (lptvcd->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
      return CDRF_NOTIFYITEMDRAW;
    case CDDS_ITEMPREPAINT:
      lptvcd->clrText   = colors.Fg_N;
      lptvcd->clrTextBk = colors.Bg_N;
      if((CDIS_SELECTED | CDIS_FOCUS) & lptvcd->nmcd.uItemState) {
        lptvcd->clrText   = colors.Fg_S;
        lptvcd->clrTextBk = colors.Bg_S;
        return CDRF_NEWFONT;
        }
      break;
   }
  return CDRF_DODEFAULT;
}
//----------------------------------------------------------------------------
LRESULT svChoosePageTreeView::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_MOVE:
      GetWindowRect(*this, currRect);
      break;
    case WM_SIZE:
      resize();
      break;
/*
    case WM_NOTIFY:
      if(((NMHDR*)lParam)->hwndFrom == GetDlgItem(*this, IDC_TREE1)) {
        switch(((NMHDR*)lParam)->code) {
          case TVN_KEYDOWN:
            if(VK_RETURN == ((LPNMTVKEYDOWN)lParam)->wVKey)
            makeResult();
            break;
          }
        }
      break;
*/
    case WM_NOTIFY:
      if(LOWORD (wParam) != IDC_TREE1)
        break;

      switch(((LPNMHDR)lParam)->code) {
        case NM_CUSTOMDRAW:
          SetWindowLongPtr(hwnd, DWLP_MSGRESULT, (LONG_PTR)ProcessNotify(lParam, Colors));
          return TRUE;
        }
      break;

    case WM_GETMINMAXINFO:
      do {
        LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam;
        lpmmi->ptMinTrackSize.x = minRect.Width();
        lpmmi->ptMinTrackSize.y = minRect.Height();
        } while(false);
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void svChoosePageTreeView::resetContent()
{
  TreeView_DeleteAllItems(GetDlgItem(*this, IDC_TREE1));
}
//----------------------------------------------------------------------------
void svChoosePageTreeView::addRow(LPCTSTR row)
{
  Rows[Rows.getElem()] = str_newdup(row);
}
//----------------------------------------------------------------------------
void svChoosePageTreeView::refresh()
{
  prepare();
  baseClass::refresh();
  endprepare();
}
//----------------------------------------------------------------------------
void svChoosePageTreeView::getPage(LPTSTR page)
{
  page[0] = _T(']');
  page[1] = 0;
  HWND hwTv = GetDlgItem(*this, IDC_TREE1);
  HTREEITEM sel = TreeView_GetSelection(hwTv);
  if(!sel)
    return;

  TV_ITEM tvi;
  tvi.mask = TVIF_TEXT;
  tvi.hItem = sel;
  tvi.pszText = page + 1;
  tvi.cchTextMax = _MAX_PATH - 1;
  TreeView_GetItem(hwTv, &tvi);
}
//----------------------------------------------------------------------------
bool choosePageByTreeView(PWin* parent, setOfString& set)
{
  return IDOK == svChoosePageTreeView(set, parent).modal();
}
