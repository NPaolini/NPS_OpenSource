//---------- recipe_row.cpp --------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdio.h>
#include <stdlib.h>
//----------------------------------------------------------------------------
#include "recipe_row.h"
#include "1.h"
#include "id_btn.h"
#include "mainclient.h"
#include "gestdata.h"
#include "lnk_body.h"
#include "pBitmap.h"
#include "sizer.h"
#include "def_dir.h"

#include "pvaredit.h"
#include "PSimpleText.h"
#include "password.h"
#include "perif.h"

#include "p_util.h"

#include "id_msg_common.h"
//----------------------------------------------------------------------------
#define USE_PLC_WORD_4_CURR
//----------------------------------------------------------------------------
P_Body *getRecipeRow(int idPar, PWin *parent)
{
  P_Body *bd = new TD_RecipeRow(idPar, parent);
  return bd;
}
//----------------------------------------------------------------------------
#define _M(a) (idInitMenu + (a))
//----------------------------------------------------------------------------
static bool getInfoEdit(const setOfString& set, int& firstId, int& nRow, int& nCol);
//----------------------------------------------------------------------------
static int IdParent;
//----------------------------------------------------------------------------
//#define NAME_STR _T("recipe.txt")
//----------------------------------------------------------------------------
#define NAME_STR getPageName()
//----------------------------------------------------------------------------
#define NAME_BMP _T("image\\indicator.bmp")
//----------------------------------------------------------------------------
TD_RecipeRow::TD_RecipeRow(int idPar, PWin* parent, int resId, HINSTANCE hinst)
:
    baseClass(idPar, parent, resId, hinst), currRow(2), Indicator(0),
    nRow(0), nCol(0), firstId(0), idInitMem(0), useFirstForText(false),
    firstIdNum(0)
{
  if(idPar)
    IdParent = idPar;
}
//----------------------------------------------------------------------------
TD_RecipeRow::~TD_RecipeRow()
{
  destroy();
  flushPV(Numb);
  delete Indicator;
}
//----------------------------------------------------------------------------
bool TD_RecipeRow::create()
{
  if(!baseClass::create())
    return false;
  for(int i = 0; i < MAX_ROW_RECIPE; ++i)
    fillStr(buffText[i], 0, SIZE_A(buffText[i]));
  Attr.id = IDD_RECIPE_ROW;
  LPCTSTR p = getPageString(ID_INIT_RECIPE_ROW_IN_MEM);
  if(p)
    // il primo valore è riservato per la riga attiva
    idInitMem = _ttoi(p) + 1;
  p = getPageString(ID_USE_FIRST_EDIT_FOR_TEXT);
  if(p)
    useFirstForText = toBool(_ttoi(p));
  return true;
}
//----------------------------------------------------------------------------
void TD_RecipeRow::moveUp()
{
  int lastRow = currRow;
  if(currRow > 0)
    --currRow;
  else
    currRow = MAX_ROW_RECIPE - 1;
  updateMove(lastRow);
}
//----------------------------------------------------------------------------
void TD_RecipeRow::moveDown()
{
  int lastRow = currRow;
  if(++currRow >= MAX_ROW_RECIPE)
    currRow = 0;

  updateMove(lastRow);
}
//----------------------------------------------------------------------------
void TD_RecipeRow::invalidateRow(int row)
{
  if(row >= nRow)
    return;
  for(int i = 0; i < nCol; ++i, row += nRow)
    InvalidateRect(*Edi[row], 0, 1);
}
//----------------------------------------------------------------------------
inline void invalidateBmp(PWin* owner, PTraspBitmap* bmp)
{
  PRect r = bmp->getRect();
  r.Inflate(2, 2);
  InvalidateRect(*owner, r, 0);
}
//----------------------------------------------------------------------------
void TD_RecipeRow::updateMove(int previousRow)
{
  if(!Indicator)
    return;
  if(Edi.getElem()) {
    int delta = Edi[0]->getOffs();
    // se la nuova posizione è all'interno del range
    if(currRow >= delta && (currRow - delta) < nRow) {
      if(Indicator) {
//        invalidateBmp(this, Indicator);
        POINT pt = { PtInd.x, PtInd.y + Delta * (currRow - delta) };
        Indicator->moveTo(pt);
//        invalidateBmp(this, Indicator);
        }
      invalidateRow(previousRow - delta);
      invalidateRow(currRow - delta);
      }
    else {
      setOffsEdi(false);
      refreshNum();
      }
    }
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
P_Body* TD_RecipeRow::pushedBtn(int idBtn)
{
  switch(idBtn) {
    case ID_F3:
      CHECK_FZ_ENABLED(ID_F3)
      load();
      return 0;

    case ID_F5:
      CHECK_FZ_ENABLED(ID_F5)
      do {
        int pswLevel = 3;
        LPCTSTR p = getPageString(ID_PSW_LEVEL_SAVE_RR);
        if(p)
          pswLevel = _ttoi(p);
        PassWord psw;
        if(pswLevel == psw.getPsW(pswLevel, this)) {
          save();
          psw.restartTime(0);
          }
        } while(false);
      load();
      return 0;

    case ID_F7:
      CHECK_FZ_ENABLED(ID_F7)
      do {
        int pswLevel = 2;
        LPCTSTR p = getPageString(ID_PSW_LEVEL_SEND_RR);
        if(p)
          pswLevel = _ttoi(p);
        PassWord psw;
        if(pswLevel == psw.getPsW(pswLevel, this)) {
          // non dovrebbe poter inviare una ricetta diversa da quelle
          // memorizzate, per cui esegue prima un reload dei dati
          save(false);
          Send();
          psw.restartTime(0);
          }
        } while(false);
      return 0;

    case ID_F9:
      CHECK_FZ_ENABLED(ID_F9)
      moveUp();
      return 0;

    case ID_F10:
      CHECK_FZ_ENABLED(ID_F10)
      moveDown();
      return 0;

    default:
      break;
    }
  return baseClass::pushedBtn(idBtn);
}
//----------------------------------------------------------------------------
void TD_RecipeRow::setOffsEdi(bool invalidateAll)
{
  if(!Indicator)
    return;
  preOffsEdi();

  int deltaAddr = Edi[0]->getOffs();
  if(!currRow && deltaAddr == MAX_ROW_RECIPE - nRow) {
    deltaAddr = 0;
//    invalidateAll = true;
    }
  else if(MAX_ROW_RECIPE - 1 == currRow && !deltaAddr) {
    deltaAddr = MAX_ROW_RECIPE - nRow;
//    invalidateAll = true;
    }
  else {
    if(currRow - (nRow + deltaAddr) >= 1)
      deltaAddr = currRow - nRow + 1;
    else if(currRow >= deltaAddr + nRow)
      ++deltaAddr;
    else
      --deltaAddr;
    }
  int numEdi = nCol * nRow;
  for(int i = 0; i < numEdi; ++i) {
    Edi[i]->setOffsAddr(deltaAddr);
    if(!invalidateAll)
      InvalidateRect(*Edi[i], 0, true);
    }
  if(!currRow || MAX_ROW_RECIPE - 1 == currRow) {
    int previous = !currRow ? nRow -1 : MAX_ROW_RECIPE - nRow;
    updateMove(previous);
    }
  postOffsEdi();
  if(invalidateAll)
    InvalidateRect(*this, 0, true);
  setDirty();
}
//----------------------------------------------------------------------------
genericPerif* TD_RecipeRow::getPrfMem()
{
  if(Edi.getElem()) {
    uint idprf = Edi[0]->getIdPrph();
    if(idprf)
      return Par->getGenPerif(idprf);
    if(Edi.getElem() > (uint)nRow)
      idprf = Edi[nRow]->getIdPrph();
    return Par->getGenPerif(idprf);
    }
  return 0;
}
//----------------------------------------------------------------------------
int TD_RecipeRow::getActiveRow(int defVal)
{
  gestPerif *plc = Par->getPerif(WM_REAL_PLC);
  if(!plc)
    return defVal;
  prfData data;
#ifdef USE_PLC_WORD_4_CURR
  prfData::tData type = prfData::tWData;
#else
  prfData::tData type = plc->getDefaultType();
#endif
  data.typeVar = type;

  LPCTSTR p = getPageString(ID_WORD_PLC);

  // il primo indirizzo per il numero della ricetta (uno based)
  data.lAddr = _ttoi(p);
  int result = defVal;

  // se l'indirizzo è negativo, nel plc non viene memorizzato
  if((int)data.lAddr > 0) {
    plc->get(data);

    switch(type) {
      case prfData::tBData:
      case prfData::tWData:
      case prfData::tDWData:
        result = data.U.dw - 1;
        break;

      case prfData::tFRData:
        result = (int)(data.U.fw - 1 + 0.5);
        break;
      }
    }
  if(result < 0 || result >= MAX_ROW_RECIPE)
    result = defVal;
  return result;
}
//----------------------------------------------------------------------------
void TD_RecipeRow::setReady(bool first)
{
  baseClass::setReady(true);

  getInfoEdit(sStr, firstId, nRow, nCol);

  gestPerif *plc = Par->getPerif(WM_REAL_PLC);
  if(!plc)
    return;
  genericPerif *prf = getPrfMem();
  if(prf) {
    prfData data;
    data.typeVar = prfData::tDWData;
    data.lAddr = idInitMem - 1;
    prf->get(data);
    currRow = data.U.dw;
    if((unsigned)currRow >= MAX_ROW_RECIPE) {
      currRow = 0;
      data.U.dw = 0;
      prf->set(data);
      }
    }

  currRow = getActiveRow(currRow);

  if(Edi.getElem() > 1) {
    Edi[0]->getColors(cActive.fg, cActive.bkg);

    cActive.hBrush = CreateSolidBrush(cActive.bkg);

    Edi[1]->getColors(cUnactive.fg, cUnactive.bkg);

    cUnactive.hBrush = CreateSolidBrush(cUnactive.bkg);
    }
  LPCTSTR indName = getPageString(ID_INDICATOR_BMP);
  if(!indName)
    indName = NAME_BMP;
  PBitmap* bmp = new PBitmap(indName);

  SIZE sz = bmp->getSize();
  bool success = false;
  InvalidateRect(*this, 0, true);
  while(true) {
    if(!sz.cx || !sz.cy)
      break;

    LPCTSTR p = getPageString(ID_FIRST_NUM);
    if(!p)
      break;
    int id = _ttoi(p);
    PRect r;
    PRect r2;
    uint nElem = Simple.getElem();
    uint found = 0;
    for(uint i = 0; i < nElem && found < 2; ++i) {
      if(Simple[i]->getId() == id) {
        firstIdNum = i;
        r = Simple[i]->get_Rect();
        ++found;
        }
      else if(Simple[i]->getId() == id + 1) {
        r2 = Simple[i]->get_Rect();
        ++found;
        }
      }
    if(found < 2)
      break;

    Delta = r2.top - r.top;

    double scale = r.Height();
    scale /= sz.cy;

    PtInd.x = (LONG)(r.left - sz.cx * scale - R__X(4));
    PtInd.y = r.top;

    Indicator = new PTraspBitmap(this, bmp, PtInd, true);
    Indicator->setScale(scale);
//    Indicator->setCanSaveBkg(false);

    int curr = currRow;
    if(curr >= nRow) {
      curr = nRow - 1;
      setOffsEdi(true);
      }
    POINT pt = { PtInd.x, PtInd.y + Delta * curr };
    Indicator->moveToSimple(pt);
//    UpdateWindow(*this);
//    Indicator->Draw();
    Simple.remove(firstIdNum, nRow, Numb);
    InvalidateRect(*this, 0, 0);
    success = true;
    break;
    }
  if(!success)
    delete bmp;

  load();
  refresh();
}
//----------------------------------------------------------------------------
HBRUSH TD_RecipeRow::evCtlColor(HDC dc, HWND hWndChild, UINT ctlType)
{
  if(WM_CTLCOLOREDIT != ctlType)
    return baseClass::evCtlColor(dc, hWndChild, ctlType);
  int ctlId = GetDlgCtrlID(hWndChild);
  if(ctlId - firstId >= nCol * nRow)
    return baseClass::evCtlColor(dc, hWndChild, ctlType);
  int delta = Edi[0]->getOffs();

  int id = firstId + currRow - delta;
  for(int i = 0; i < nCol; ++i)
    if(ctlId == id + i * nRow) {
      SetTextColor(dc, cActive.fg);
      SetBkColor(dc, cActive.bkg);
      return cActive.hBrush;
      }
  SetTextColor(dc, cUnactive.fg);
  SetBkColor(dc, cUnactive.bkg);
  return cUnactive.hBrush;

}
//----------------------------------------------------------------------------
void TD_RecipeRow::refreshNum()
{
  if(firstIdNum < 0)
    return;
  for(int i = 0; i < nRow; ++i)
    Numb[i]->invalidate();
}
//----------------------------------------------------------------------------
void TD_RecipeRow::drawNum(int delta, HDC hdc, const PRect& rect)
{
  if(firstIdNum < 0)
    return;
  for(int i = 0; i < nRow; ++i) {
    PSimpleText* st = (PSimpleText*)Numb[i];
    TCHAR buff[20];
    wsprintf(buff, _T("%d"), delta + i + 1);
    st->set_Text(buff);
    st->draw(hdc, rect);
    }
}
//----------------------------------------------------------------------------
void TD_RecipeRow::evPaint(HDC hdc, const PRect& rect)
{
  baseClass::evPaint(hdc, rect);

  if(Edi.getElem()) {
    int delta = Edi[0]->getOffs();
    drawNum(delta, hdc, rect);
    }

  if(Indicator)
    Indicator->Draw(hdc);
}
//-----------------------------------------------------------------
#define MAX_MULTI_DATA 30
//-----------------------------------------------------------------
//#define ID_BIT_SEND 1000011
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool TD_RecipeRow::Send()
{
  gestPerif *plc = Par->getPerif(WM_REAL_PLC);
  if(!plc)
    return false;

//  ImplJobData* job = Par->getIData();

  prfData data[MAX_MULTI_DATA];
#ifdef USE_PLC_WORD_4_CURR
  prfData::tData type = prfData::tWData;
#else
  prfData::tData type = plc->getDefaultType();
#endif
  data[0].typeVar = type;

  LPCTSTR p = getPageString(ID_BIT_SEND);
  int Prf = 0;
  int Addr = 0;
  int Type = 1;
  int nBit = 0;
  int Offs = 0;

  if(p)
    _stscanf_s(p, _T("%d,%d,%d,%d,%d"), &Prf, &Addr, &Type, &nBit, &Offs);

  prfData checkBit;
  checkBit.lAddr = Addr;
  checkBit.typeVar = Type;

  if(prfData::tBitData == Type)
    checkBit.U.dw = MAKELONG(nBit, Offs);

  // non ha più senso verificare se è in memoria, esistono altre periferiche memoria
  // diverse dalla prima
//  if(1 != Prf)
    {
    genericPerif* prf = Par->getGenPerif(Prf);
    if(prf)
      prf->get(checkBit);
    else
      // non deve visualizzare il messaggio se non trova la periferica
      // potrebbe non essere stato inserito il codice
      checkBit.U.dw = 1;
    }

  genericPerif *prf = getPrfMem();

  if(prfData::tBitData != Type && nBit)
    checkBit.U.dw &= ((1 << nBit) - 1) << Offs;

  if(!checkBit.U.dw) {
//      svMessageBox(Par, "Linea non in Emergenza", "Emergenza", MB_ICONINFORMATION);
    msgBoxByLangGlob(Par, ID_MSG_ON_EMERG, ID_TITLE_ON_EMERG, MB_ICONINFORMATION | MB_OK);
    return false;
  }


  p = getPageString(ID_WORD_PLC);

  prfData dataJob;
  dataJob.typeVar = prfData::tDWData;

  int first = 1;
  // il primo indirizzo per il numero della ricetta (uno based)
  data[0].lAddr = _ttoi(p);
  if((int)data[0].lAddr > 0) {
    switch(type) {
      case prfData::tBData:
      case prfData::tWData:
      case prfData::tDWData:
        data[0].U.dw = currRow + 1;
        break;

      case prfData::tFRData:
        data[0].U.fw = (fREALDATA)(currRow + 1);
        break;

      default:
        return false;
      }
    }
  else
    first = 0;
//  if(prfData::okData > plc->set(data))
//    return;

#ifdef USE_PLC_WORD_4_CURR
  type = plc->getDefaultType();
#endif
  int i;
  int j = 0;
  for(i = 0; i < nCol; ++i) {

    p = findNextParam(p, 1);
    if(!p)
      break;
    int addr = _ttoi(p);
    if(addr < 0)
      continue;
    data[j + first].typeVar = type;
    data[j + first].lAddr = addr;

    dataJob.lAddr = i * MAX_ROW_RECIPE + idInitMem + currRow;// - 1;
    prf->get(dataJob);
//    job->get(dataJob);
    switch(type) {
      case prfData::tBData:
      case prfData::tWData:
      case prfData::tDWData:
        data[j + first].U.dw = (DWDATA)dataJob.U.fw;
        break;

      case prfData::tFRData:
        data[j + first].U.fw = dataJob.U.fw;
        break;

      default:
        return false;
      }
//    if(!verifyRange(Edi[i]->getIdNorm(), data[loaded])) {
//      showMsgNotInRange();
//      break;
//      }

    ++j;
    }

  if(i == nCol) {
    gestPerif::howSend old = plc->setAutoShow(gestPerif::ALL_AND_SHOW);
    if(prfData::okData <= plc->multiSet(data, j + first)) {
      dataJob.lAddr = idInitMem - 1; //CURR_ROW_IN_MEM;
      dataJob.U.dw = currRow;
      prf->set(dataJob);
      }
    plc->setAutoShow(old);
    }
  p = getPageString(ID_BIT_OKSEND);
  Prf = 0;
  if(p) {
    _stscanf_s(p, _T("%d,%d,%d,%d,%d"), &Prf, &Addr, &Type, &nBit, &Offs);

    prf = Par->getGenPerif(Prf);
    if(prf) {

      prfData okBit;
      okBit.lAddr = Addr;
      okBit.typeVar = Type;
      if(prfData::tBitData == type)
        okBit.U.dw = MAKELONG(MAKEWORD(nBit, Offs), (1 << nBit) - 1);
      else {
        plc->get(okBit);
        okBit.U.dw |= ((1 << nBit) - 1) << Offs;
        }
      prf->set(okBit);
      }
    }
  setNameActive(true);
  return true;
}
//----------------------------------------------------------------------------
#define DEF_EXT_ROW _T(".row")
//----------------------------------------------------------------------------
bool TD_RecipeRow::save(bool showResult)
{
  LPCTSTR p = getPageString(ID_RECIPE_ROW_FILE);
  if(!p)
    return false;

  DWDATA nData = MAX_ROW_RECIPE * nCol + 1;
  DWDATA size = nData * sizeof(DWDATA);

  genericPerif *prf = getPrfMem();
  if(!prf)
    return false;
  TCHAR file[_MAX_PATH];
  makePath(file, p, dRicette, false, DEF_EXT_ROW);
  P_File f(file, P_CREAT);
  bool success = false;
  if(f.P_open()) {
    DWDATA* pbuff = new DWDATA[nData];
    prfData data;
    data.typeVar = prfData::tDWData;
    data.lAddr = idInitMem - 1;
    for(DWDATA i = 0; i < nData; ++i) {
      if(!i)
        pbuff[i] = getActiveRow(currRow);
      else {
        prf->get(data);
        pbuff[i] = data.U.dw;
        }
      ++data.lAddr;
      }
    success = size == f.P_write(pbuff, size);
    delete []pbuff;
    }
  if(success)
    success = customSave(f);
  if(showResult)
    showFileResult(success, this);
  return success;
}
//----------------------------------------------------------------------------
bool TD_RecipeRow::load()
{
  LPCTSTR p = getPageString(ID_RECIPE_ROW_FILE);
  if(!p)
    return false;

  genericPerif *prf = getPrfMem();
  if(!prf)
    return false;

  DWDATA nData = MAX_ROW_RECIPE * nCol + 1;
  DWDATA size = nData * sizeof(DWDATA);
  TCHAR file[_MAX_PATH];
  makePath(file, p, dRicette, false, DEF_EXT_ROW);
  P_File f(file, P_READ_ONLY);
  bool success = false;
  if(f.P_open()) {
    DWDATA* pbuff = new DWDATA[nData];
#if 1 // correzione
    DWORD sizeFile = (DWORD)f.get_len();
    if(MAX_ROW_RECIPE * MAX_BUFF_TYPE + MAX_ROW_RECIPE * nCol * sizeof(DWDATA) == sizeFile) {
      --nData;
      size -= sizeof(DWDATA);
      }
#endif
    success = size == f.P_read(pbuff, size);
    if(success) {
      prfData data;
      data.typeVar = prfData::tDWData;
      data.lAddr = idInitMem - 1;
      gestPerif::howSend old = gestPerif::NOT_DEFINED;
      gestPerif* gp = dynamic_cast<gestPerif*>(prf);
      if(gp)
        old = gp->setAutoShow(gestPerif::CHECK_READY);

      prf->dontSaveNow();
      for(DWDATA i = 0; i < nData; ++i) {
        data.U.dw = pbuff[i];
        if(prfData::okData > prf->set(data)) {
          success = false;
          break;
          }
        ++data.lAddr;
        }
      if(gp)
        gp->setAutoShow(old);
      prf->saveNow();
      }
    delete []pbuff;
    }
  if(success)
    success = customLoad(f);
  return success;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool getInfoEdit(const setOfString& set, int& firstId, int& nRow, int& nCol)
{
  LPCTSTR p = set.getString(ID_FIRST_EDIT);
  if(!p)
    return false;
  firstId = _ttoi(p);

  p = set.getString(ID_NUM_ROW);
  if(!p)
    return false;
  nRow = _ttoi(p);

  p = set.getString(ID_NUM_COL);
  if(!p)
    return false;
  nCol = _ttoi(p);
  return true;
}
//----------------------------------------------------------------------------
void TD_RecipeRow::getFileStr(LPTSTR path)
{
  getFileStrCheckRemote(path, NAME_STR);
//  _tcscpy(path, NAME_STR);
//  getPath(path, dSystem);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void TD_RecipeRow::setNameActive(bool useCurrRow)
{
  LPCTSTR p = getPageString(ID_SAVE_NAME_RECIPE);
  if(p) {
    int prph = 0;
    int addr = 0;
    _stscanf_s(p, _T("%d,%d"), &prph, &addr);
    genericPerif* prf = Par->getGenPerif(prph);
    if(prf) {
      int currActive = useCurrRow ? currRow : getActiveRow(currRow);
      prfData data;
      data.lAddr = addr;
#ifdef UNICODE
      BYTE t[MAX_BUFF_TYPE + 1];
      copyStrZ(t, buffText[currActive], MAX_BUFF_TYPE);
      data.setStr(MAX_BUFF_TYPE, t);
#else
      data.setStr(MAX_BUFF_TYPE, (LPBYTE)buffText[currActive]);
#endif
      prf->set(data);
      }
    }
}
//----------------------------------------------------------------------------
bool TD_RecipeRow::customSave(P_File& f)
{
  if(!useFirstForText)
    return true;
  preOffsEdi();
  char tmp[MAX_BUFF_TYPE];
  for(int i = 0; i < MAX_ROW_RECIPE; ++i) {
    copyStr(tmp, buffText[i], MAX_BUFF_TYPE);
    f.P_write(tmp, MAX_BUFF_TYPE);
    }
  setNameActive();
  return true;
}
//----------------------------------------------------------------------------
void TD_RecipeRow::updateText()
{
  if(!useFirstForText)
    return;
  int j = Edi[0]->getOffs();
  for(int i = 0; i < nRow; ++i, ++j)
    SetDlgItemText(*this, firstId + i, buffText[j]);
}
//----------------------------------------------------------------------------
bool TD_RecipeRow::customLoad(P_File& f)
{
  if(!useFirstForText)
    return true;
  char tmp[MAX_BUFF_TYPE];
  for(int i = 0; i < MAX_ROW_RECIPE; ++i) {
    f.P_read(tmp, MAX_BUFF_TYPE);
    copyStr(buffText[i], tmp, MAX_BUFF_TYPE);
    }
  updateText();
  setNameActive();
  return true;
}
//----------------------------------------------------------------------------
void TD_RecipeRow::preOffsEdi()
{
  if(!useFirstForText)
    return;
  int j = Edi[0]->getOffs();
  for(int i = 0; i < nRow; ++i, ++j)
    GetDlgItemText(*this, firstId + i, buffText[j], MAX_BUFF_TYPE + 1);
}
//----------------------------------------------------------------------------
void TD_RecipeRow::postOffsEdi()
{
  updateText();
}
//----------------------------------------------------------------------------
#define SIZE_DATA_JOB (sizeof(((job_file*)0x1000)->data[0]))
//#define SIZE_DATA_JOB sizeof(DWDATA)
//----------------------------------------------------------------------------
bool getRecipeTextType(LPTSTR target, uint row, LPCTSTR pageFile)
{
  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, SIZE_A(path), pageFile);

  bool local = false;
  LPCTSTR p = ::getString(ID_USE_LOCAL_PATH_FOR_PAGE_TEXT);
  if(p)
    local = toBool(_ttoi(p));
  if(local)
    getRelPath(path, dSystem);
  else
    getPath(path, dSystem);

  setOfString sos;
  pageStrToSet(sos, path);

  int firstId;
  int nRow;
  int nCol;

  if(!getInfoEdit(sos, firstId, nRow, nCol))
    return false;

  p = sos.getString(ID_RECIPE_ROW_FILE);
  if(!p)
    return false;

  DWDATA size = MAX_ROW_RECIPE * SIZE_DATA_JOB * nCol;
  TCHAR file[_MAX_PATH];
  makePath(file, p, dRicette, false, DEF_EXT_ROW);

  P_File f(file, P_READ_ONLY);
  if(f.P_open()) {
    f.P_seek(size + row * MAX_BUFF_TYPE);
    char tmp[MAX_BUFF_TYPE];
    if(MAX_BUFF_TYPE == f.P_read(tmp, MAX_BUFF_TYPE)) {
      copyStr(target, tmp, MAX_BUFF_TYPE);
      target[MAX_BUFF_TYPE] = 0;
      return true;
      }
    }
  return false;
}
//----------------------------------------------------------------------------
void TD_RecipeRow::ShowErrorData(uint idprf, const prfData& data, prfData::tResultData result)
{
  if(useFirstForText && !idprf)
    return;
  baseClass::ShowErrorData(idprf, data, result);
}
//----------------------------------------------------------------------------
