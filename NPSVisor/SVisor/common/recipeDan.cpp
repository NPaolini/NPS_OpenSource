//---------- recipeDan.cpp ------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <stdio.h>
//----------------------------------------------------------------------------
#include "recipeDan.h"
#include "1.h"
#include "id_btn.h"
#include "mainclient.h"
#include "gestdata.h"
#include "lnk_body.h"
#include "def_dir.h"

#include "password.h"
#include "perif.h"
#include "pVarListBox.h"

#include "p_util.h"

#include "DSerchFl.h"
#include "sizer.h"
//----------------------------------------------------------------------------
//#define NAME_STR _T("PagRicetta.txt")
//----------------------------------------------------------------------------
#define NAME_STR getPageName()
//----------------------------------------------------------------------------
P_Body *getRecipeDan(int idPar, PWin *parent)
{
  P_Body *bd = new TD_RecipeDan(idPar, parent);
  return bd;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class rcpListBoxDan : public PVarListBox
{
  public:
    rcpListBoxDan(int vkBtn, P_BaseBody* parent, uint style, uint id, const PRect& rect, int len = 255,
              HINSTANCE hinst = 0);
    ~rcpListBoxDan() {  }
    void dblClick() {  click(VkBtn); }
    bool preProcessMsg(MSG& msg);
    bool hasFont() const { return toBool(getFont()); }
  protected:
    // deve tornare l'HPEN restituito dalla prima SelectObject(hdc, hPen)
    // per essere reimpostato al termine. hPen verrà distrutto dalla classe base.
//    virtual HPEN setColor(HDC hDC, DRAWITEMSTRUCT FAR& drawInfo);
//    void paintCustom(HDC hdc, PRect& r, int ix, DRAWITEMSTRUCT FAR& drawInfo);
  private:
    int VkBtn;
    typedef PVarListBox baseClass;
};
//------------------------------------------------------------------
rcpListBoxDan::rcpListBoxDan(int vkBtn, P_BaseBody* parent, uint style, uint id, const PRect& rect,
        int len, HINSTANCE hinst):
       PVarListBox(parent, style, id, rect, len, hinst), baseActive(parent, id), VkBtn(vkBtn)
{

}
//----------------------------------------------------------------------------
bool rcpListBoxDan::preProcessMsg(MSG& msg)
{
  if(WM_KEYDOWN == msg.message)
    if(VK_RETURN == msg.wParam)
      click(VkBtn);
  return PListBox::preProcessMsg(msg);
}
//------------------------------------------------------------------
#define MAX_LEN_NAME 16
//#define LEN_DATA_LB 12
//#define NUM_DATA_LB 8
//----------------------------------------------------------------------------
// pulsante associato alla scelta nella listbox
#define ID_FILL_LB_BTN VK_F8
//----------------------------------------------------------------------------
#define LEN_TAB_4_CENTER 10000
//----------------------------------------------------------------------------
PVarListBox* TD_RecipeDan::allocOverrideLBox(P_BaseBody* parent, uint style, uint id, const PRect& rect,
              int textlen, HINSTANCE hinst)
{
  rcpListBoxDan* lb = new rcpListBoxDan(ID_FILL_LB_BTN, parent, style, id, rect, textlen, hinst);

  LPCTSTR p = getPageString(ID_INIT_RECIPE_LB + 3);
  if(!p)
    return lb;

  pVectInt tabs;
  fillSet(tabs, p);
  int nElem = tabs.getElem();

  pVectInt align;
  align.setDim(nElem);

  for(int i = 0; i < nElem; ++i) {
    if(tabs[i] > LEN_TAB_4_CENTER) {
      tabs[i] -= LEN_TAB_4_CENTER;
      align[i] = 1;
      lenRow += tabs[i];
      vLenColumn[i] = tabs[i];
      }
    else {
      if(tabs[i] < 0) {
        lenRow += -tabs[i];
        vLenColumn[i] = -tabs[i];
        }
      else {
        lenRow += tabs[i];
        vLenColumn[i] = tabs[i];
        }
      align[i] = 0;
      }
    }
  lenRow += MAX_LEN_NAME;
  int* tb = new int[nElem + 1];
  tb[0] = MAX_LEN_NAME;
  for(int i = 0; i < nElem; ++i)
    tb[i + 1] = tabs[i];
  lb->SetTabStop(nElem + 1, tb, 0);
  delete []tb;
  for(int i = 0; i < nElem; ++i)
    if(align[i])
      lb->setAlign(i + 1, PListBox::aCenter);


  if(!lb->hasFont()) {

    p = getPageString(ID_INIT_RECIPE_LB);
    if(p) {
      int height = _ttoi(p);
      if(height) {
        p = findNextParam(p, 1);
        int style = _ttoi(p);
        p = findNextParam(p, 1);
        HFONT font = D_FONT(R__Y(height), 0, style, p);
        if(font)
          lb->setFont(font, true);
        }
      }
    }

  return lb;
}
//----------------------------------------------------------------------------
static int IdParent;
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
TD_RecipeDan::TD_RecipeDan(int idPar, PWin* parent, int resId, HINSTANCE hinst)
:
    baseClass(0, parent, resId, hinst), lastChoose(false), lenRow(0)

{
  if(idPar)
    IdParent = idPar;
}
//----------------------------------------------------------------------------
TD_RecipeDan::~TD_RecipeDan()
{
  destroy();
}
//----------------------------------------------------------------------------
void TD_RecipeDan::calcLenAndOffset()
{
  LPCTSTR p = getPageString(ID_INIT_DATA_RECIPE + 3);
  if(!p)
    return;

  pVectInt vAllType;
  do {
    pVectInt vType;
    fillSet(vType, p);

    p = getPageString(ID_INIT_DATA_RECIPE + 10);

    fillSet(vAllType, p);
    int nT = vType.getElem();
    for(int i = vAllType.getElem(); i < nT; ++i)
      vAllType[i] = vType[i];
    } while(false);

  int nElem = vAllType.getElem();
  if(!nElem)
    return;

  p = getPageString(ID_INIT_DATA_RECIPE + 4);
  if(!p)
    return;
  pVectInt vLen;
  fillSet(vLen, p);
  int nElemLen = vLen.getElem();

  for(int i = 0; i < nElemLen; ++i)
    if(!vLen[i])
      vLen[i] = sizeof(DWDATA);

  for(; nElemLen < nElem; ++nElemLen)
    vLen[nElemLen] = sizeof(DWDATA);

  p = getPageString(ID_INIT_RECIPE_LB + 1);
  if(!p)
    return;

  pVectInt vField;
  fillSet(vField, p);
  int nField = vField.getElem();

  p = getPageString(ID_INIT_RECIPE_LB + 2);
  vFilter.reset();
  if(p)
    fillSet(vFilter, p);

  p = getPageString(ID_INIT_RECIPE_LB + 4);
  vDec.reset();
  if(p)
    fillSet(vDec, p);

  vOffset.setDim(nField);
  vLenField.setDim(nField);
  vTypes.setDim(nField);
  vDec.setDim(nField);

  int nDec = vDec.getElem();

  for(int i = nDec; i < nField; ++i)
    vDec[i] = 0;

  for(int i = 0; i < nField; ++i) {
    int f = vField[i];
    vLenField[i] = vLen[f];
    vTypes[i] = vAllType[f];
    int offs = 0;
    for(int j = 0; j < f; ++j)
      offs += vLen[j];
    vOffset[i] = offs;
    }
}
//----------------------------------------------------------------------------
LRESULT TD_RecipeDan::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(HIWORD(wParam)) {
        case LBN_DBLCLK:
          handleLBDblClick((HWND)lParam);
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void TD_RecipeDan::handleLBDblClick(HWND hwnd)
{
  PWin* w = getWindowPtr(hwnd);
  rcpListBoxDan* lb = dynamic_cast<rcpListBoxDan*>(w);
  if(lb)
    lb->dblClick();
}
//----------------------------------------------------------------------------
static
bool hasFzText(setOfString& set, uint id)
{
  LPCTSTR p = set.getString(id);
  return p && *p;
}
//----------------------------------------------------------------------------
#define CHECK_FZ_ENABLED(btn) \
  if(!hasFzText(sStr, btn)) \
    return 0;
//----------------------------------------------------------------------------
P_Body* TD_RecipeDan::pushedBtn(int idBtn)
{
  switch(idBtn) {

    case ID_F3:
      CHECK_FZ_ENABLED(ID_F3)
      loadAll(false);
      return 0;

    case ID_F4:
      CHECK_FZ_ENABLED(ID_F4)
      loadAll(true);
      return 0;

    case ID_F5:
      CHECK_FZ_ENABLED(ID_F5)
      save();
      loadAll(false);
      return 0;

    case ID_F8:
      CHECK_FZ_ENABLED(ID_F8)
      loadFromLb();
      return 0;

    case ID_F11:
      CHECK_FZ_ENABLED(ID_F11)
      erase();
      loadAll(false);
      return 0;

    default:
      break;
    }
  return baseClass::pushedBtn(idBtn);
}
//----------------------------------------------------------------------------
void TD_RecipeDan::setReady(bool first)
{
  baseClass::setReady(true);
  calcLenAndOffset();
  loadAll(false);
  refresh();
}
//-----------------------------------------------------------------
//bool TD_RecipeDan::copyMemory()
//{
//  ImplJobData* job = Par->getIData();
//  return job->loadActMemRecipe();
//}
//----------------------------------------------------------------------------
static
void copy_Str(LPTSTR buff, LPCTSTR name, int maxLen)
{
  for(int i = 0; i < maxLen; ++i) {
    if(!name[i])
      break;
    buff[i] = name[i];
    }
}
//----------------------------------------------------------------------------
LPTSTR addColumn(LPTSTR pb, LPBYTE record, int offs, int type, int lenField, int lenColumn, int dec)
{
  LPTSTR tmp = new TCHAR[lenColumn * 2];
  tmp[0] = 0;
  switch(type) {
    case prfData::tBData:
    case prfData::tBsData:
      wsprintf(tmp, _T("%d"), *(record + offs));
      break;
    case prfData::tWData:
    case prfData::tWsData:
      wsprintf(tmp, _T("%d"), *(WDATA*)(record + offs));
      break;

    case prfData::tDWData:
    case prfData::tDWsData:
      wsprintf(tmp, _T("%d"), *(DWDATA*)(record + offs));
      break;
    case prfData::tFRData:
      _stprintf_s(tmp, lenColumn * 2, _T("%*f"), dec, *(fREALDATA*)(record + offs));
      break;
    case prfData::tRData:
      _stprintf_s(tmp, lenColumn * 2, _T("%*f"), dec, *(REALDATA*)(record + offs));
      break;
/*
    // da verificarne l'utilizzo
    case prfData::tRData:   // 64 bit float
    case prfData::ti64Data: // 64 bit
    case prfData::tDateHour:// 64 bit per data + ora
    case prfData::tDate:    // 64 bit per data
    case prfData::tHour:    // 64 bit per ora
*/
    case prfData::tStrData:
      do {
        int len = min(lenField, lenColumn);
        for(int i = 0; i < len; ++i)
          tmp[i] = record[offs + i];
        tmp[len] = 0;
        } while(false);
      break;

    default:
      break;
    }
  copy_Str(pb, tmp, lenColumn);
  pb += lenColumn;
  *pb++ = _T('\t');
  delete []tmp;
  return pb;
}
//----------------------------------------------------------------------------
void TD_RecipeDan::addToLb(LPCTSTR name, LPBYTE record)
{
  int nColumn = vOffset.getElem();
  if(nColumn <= 0)
    return;

  LPTSTR buff = new TCHAR[lenRow + nColumn + 4];
  fillStr(buff, _T(' '), lenRow + nColumn + 4);

  TCHAR t[MAX_LEN_NAME + 2];
  _tcscpy_s(t, SIZE_A(t), name);
  int len = _tcslen(t);
  for(int i = len - 1; i > 0; --i)
    if(_T('.') == t[i]) {
      t[i] = 0;
      break;
      }

  LPTSTR pb = buff;
  copy_Str(pb, t, MAX_LEN_NAME);

  pb += MAX_LEN_NAME;
  *pb++ = _T('\t');

  for(int i = 0; i < nColumn; ++i)
    pb = addColumn(pb, record, vOffset[i], vTypes[i], vLenField[i], vLenColumn[i], vDec[i]);

  *pb = 0;
  int pos = SendMessage(*LBox[0], LB_ADDSTRING, 0, (LPARAM)buff);
  SendMessage(*LBox[0], LB_SETCURSEL, pos, 0);
  delete []buff;
}
//----------------------------------------------------------------------------
#define FILE_TO_FIND _T("*")
//----------------------------------------------------------------------------
void TD_RecipeDan::loadAll(bool filter)
{
  if(!LBox.getElem())
    return;

  ::SendMessage(*LBox[0], LB_RESETCONTENT, 0 ,0);

  WIN32_FIND_DATA fd;
  TCHAR file[_MAX_PATH];
  makePathRecipeNew(sStr, file, FILE_TO_FIND, false, getRecipeExt());
//  makePath(file, FILE_TO_FIND, dRicette, false, getRecipeExt());

  HANDLE hf = FindFirstFile(file, &fd);
  if(INVALID_HANDLE_VALUE != hf) {
    do {
      if(FILE_ATTRIBUTE_DIRECTORY != fd.dwFileAttributes)
        addRow(fd.cFileName, !filter);
      } while(FindNextFile(hf, &fd));
    FindClose(hf);
    }
  int count = ::SendMessage(*LBox[0], LB_GETCOUNT, 0, 0);
  ::SendMessage(*LBox[0], LB_SETCURSEL, count -1, 0);
  lastChoose = filter;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool TD_RecipeDan::isValid(LPBYTE fBuff)
{
  int nElem = vFilter.getElem();
  if(!nElem)
    return true;
  genericPerif* prfMem = getMemoryPrph();
  if(!prfMem)
    return false;

  for(int i = 0; i < nElem; ++i) {
    if(vFilter[i]) {
      int len = vLenField[i];
      LPBYTE p = fBuff + vOffset[i];
      if(prfData::tStrData == vTypes[i]) {
        prfData data(len);
        data.lAddr = vFilter[i];
        prfMem->get(data);
        LPBYTE t = data.U.str.buff;
        for(int j = 0; j < len; ++j) {
          if(!t[j])
            break;
          if(t[j] != p[j])
            return false;
          }
        }
      else {
        prfData data;
        data.lAddr = vFilter[i];
        data.typeVar = vTypes[i];
        prfMem->get(data);
        if(data.U.dw == 0)
          continue;
        LPBYTE t = (LPBYTE)&(data.U.dw);
        if(memcmp(p, t, len))
          return false;
        }
      }
    }
  return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void TD_RecipeDan::addRow(LPCTSTR filename, bool force)
{
  TCHAR file[_MAX_PATH];
  makePathRecipeNew(sStr, file, filename, false, getRecipeExt());
//  makePath(file, filename, dRicette, false, getRecipeExt());

  P_File pf(file, P_READ_ONLY);
  if(pf.P_open()) {

    int len = (int)pf.get_len();
    LPBYTE fBuff = new BYTE[len + 1];
    fBuff[len] = 0;
    if(len == pf.P_read(fBuff, len))
      if(force || isValid(fBuff))
        addToLb(filename, fBuff);
    delete []fBuff;
    }
}
//--------------------------------------------------------------------
//----------------------------------------------------------------------------
#define MAX_LEN_LB 1000
void TD_RecipeDan::loadFromLb()
{
  int sel = SendMessage(*LBox[0], LB_GETCURSEL, 0, 0);
  if(sel < 0)
    return;
  TCHAR path[MAX_LEN_LB];
  SendMessage(*LBox[0], LB_GETTEXT, sel, (LPARAM)path);
  path[MAX_LEN_NAME] = 0;
  trim(path);

  loadRecipe(path);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool TD_RecipeDan::create()
{
  if(!baseClass::create())
    return false;
  // se occorre un'altra pagina di ricetta bisogna modificare
  // l'id ed aggiungerlo, insieme alla funzione di caricamento, in lnk_body
  // sono disponibili fino a IDD_RECIPE + 10
  Attr.id = IDD_RECIPE + 1;
  return true;
}
//----------------------------------------------------------------------------
/*
// Rourine utilizzata solo per programma Danieli
#define ID_INIT_DATA_STAND  1000021
#define DATA_MEM_FUNZ       1000020
#define ID_DATA_ERASE       1000030

void TD_RecipeDan::setDefaultValue()
{
  LPCTSTR p = getPageString(ID_INIT_DATA_STAND);
  if(!p)
    return;

  int allPrf[] = { 0, 0, WM_PLC, WM_PLC+1, WM_PLC+2, WM_PLC+3 };
  ImplJobData* job = Par->getIData();

  int memStart,nDati;
  sscanf(p, "%d,%d", &memStart, &nDati);

  int AddrE, Len;
  prfData dataE;
  int ii = 0;
  for(;; ++ii) {
    p = getPageString(ID_DATA_ERASE + ii);
    if(!p)
      break;
    sscanf(p, "%d,%d", &AddrE, &Len);
    dataE.typeVar = 4;
    for(int k = 0; k < Len; ++k) {
      dataE.lAddr = AddrE + k;
      dataE.U.dw = 0;
      job->set(dataE);
      }
    }

  int i = 0;
  int pos = 0;
  int Prf,Addr,Type,Num;
  for(;; ++i) {
    p = getPageString(ID_INIT_DATA_STAND + i + 1);
    if(!p)
      break;
    sscanf(p, "%d,%d,%d,%d", &Prf, &Addr, &Type, &Num);
    for(int z = 0; z < Num; ++z, ++pos) {
      if(pos > nDati)
        break;
      prfData data;
      prfData dataSet;
      data.lAddr = Addr;
      data.typeVar = Type;
      dataSet.lAddr = memStart + pos;
      dataSet.typeVar = Type;
      if(Prf == 1)
        job->get(data);
      else {
        gestPerif* prf = Par->getPerif(allPrf[Prf]);
        if(prf)
          prf->get(data);
        else
          continue;
        }

      data.lAddr = memStart + pos;
      job->set(data);
      }
    }

  if(Send())
    MessageBox(*Par, getPageString(ID_MSG_SEND_OK), getPageString(ID_TITLE_SEND_OK), MB_OK);
  else
    return;


  p = getPageString(DATA_MEM_FUNZ);
  if(!p)
    return;;
  int PAddr = atoi(p);
  if(!PAddr)
    return;
  prfData data;
  data.lAddr = PAddr;
  data.typeVar = 3;
  data.U.dw = 1;
  job->set(data);
  return;
}
*/
