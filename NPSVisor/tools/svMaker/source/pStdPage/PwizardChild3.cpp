//----------- pWizardChild3.cpp ----------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <process.h>
#include <lm.h>
//----------------------------------------------------------------------------
#include "wizardDlg.h"
#include "pWizardChild3.h"
#include "common.h"
//----------------------------------------------------------------------------
#define MAX_DIM_FILENAME 30
#define MAX_DIM_LABEL    40
//----------------------------------------------------------------------------
#define MIN_COLUMN   3
#define MIN_ROW      3
#define MAX_COLUMN  20
#define MAX_ROW     15
//----------------------------------------------------------------------------
struct firstFieldAsText
{
  bool use;
  uint prph;
  uint addr;

  firstFieldAsText() : use(true), prph(0), addr(0) {}
};
//----------------------------------------------------------------------------
struct dataPrph
{
  uint prph;
  uint addr;
  uint dataType;
  uint nbit;
  uint offs;
  dataPrph() : prph(0), addr(0), dataType(0), nbit(0), offs(0) {}
};
//----------------------------------------------------------------------------
struct labelAddr
{
  int addr;
  TCHAR label[MAX_DIM_LABEL];
  labelAddr() : addr(0) { label[0] = 0; }
};
//----------------------------------------------------------------------------
class recipeRowInfo
{
  public:
    recipeRowInfo();

    int save(setOfString& Set);

  private:
    int performSave(setOfString& Set, P_File* pf, P_File* pfClear);

    uint idPrphMem4Edit;
    uint initAddrMem4Edit;

    uint nRow;
    uint nCol;
    firstFieldAsText ffT;
    TCHAR filename[MAX_DIM_FILENAME + 1];
    // il primo per l'abilitazione, il secondo per l'attivazione
    dataPrph dP[2];

    uint pswSend;
    uint pswSave;

    uint addr4SendActive;
    PVect<labelAddr> vLabelAddr;
    friend class wzChild1;
    friend class wzChild2;
};
//----------------------------------------------------------------------------
class wzChild : public PDialog
{
  private:
    typedef PDialog baseClass;
  public:
    wzChild(recipeRowInfo& rri, PWin* parent, uint resId = IDD_WZ_RECIPE_ROW_1, HINSTANCE hinstance = 0) :
      baseClass(parent, resId, hinstance), RRI(rri)
      {}
    virtual bool saveData() = 0;
    virtual void setActive(bool set) { ShowWindow(*this, set ? SW_SHOWNORMAL : SW_HIDE); }
  protected:
    recipeRowInfo& RRI;
};
//----------------------------------------------------------------------------
class wzChild1 : public wzChild
{
  private:
    typedef wzChild baseClass;
  public:
    wzChild1(recipeRowInfo& rri, PWin* parent, uint resId = IDD_WZ_RECIPE_ROW_1, HINSTANCE hinstance = 0) :
      baseClass(rri, parent, resId, hinstance)
      {}
    virtual bool create();
    virtual bool saveData();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  private:
    void setNCol();
    void fillRange(uint idc, int minV, int maxV);
    void checkFirstEdit();
};
//----------------------------------------------------------------------------
class wzChild2 : public wzChild
{
  private:
    typedef wzChild baseClass;
  public:
    wzChild2(recipeRowInfo& rri, PWin* parent, uint resId = IDD_WZ_RECIPE_ROW_2, HINSTANCE hinstance = 0) :
      baseClass(rri, parent, resId, hinstance), nCol(0)
      {}
    virtual bool create();
    virtual bool saveData();
    virtual void setActive(bool set);
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  private:
    uint nCol;
    void showGroupWindow(uint ix, bool show);
};
//----------------------------------------------------------------------------
#define MB_BTN (MB_OK | MB_ICONINFORMATION)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
pWizardChild3::pWizardChild3(PWin* parent, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance)
{
  RRI = new recipeRowInfo;
  child[0] = new wzChild1(*RRI, this);
  child[1] = new wzChild2(*RRI, this);
}
//----------------------------------------------------------------------------
pWizardChild3::~pWizardChild3()
{
  destroy();
  delete RRI;
}
//----------------------------------------------------------------------------
bool pWizardChild3::create()
{
  if(!baseClass::create())
    return false;
  for(uint i = 0; i < SIZE_A(child); ++i)
    SetWindowPos(*child[i], 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
  ShowWindow(*child[0], SW_SHOWNORMAL);
  return true;
}
//----------------------------------------------------------------------------
LRESULT pWizardChild3::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
enum eErrRep { eErrNoErr, eErrNoOpenFile, eErrNoCol, eErrNoPrph4Edit, };
//----------------------------------------------------------------------------
struct errReport
{
  eErrRep err;
  LPCTSTR txt;
} gErrReport[] = {
  { eErrNoOpenFile, _T("Impossibile creare il file per il salvataggio"), },
  { eErrNoCol, _T("Numero di colonne"), },
  { eErrNoPrph4Edit, _T("Periferica per campi di Edit"), },
};
//----------------------------------------------------------------------------
bool pWizardChild3::save(setOfString& Set)
{
  for(uint i = 0; i < SIZE_A(child); ++i)
    if(!child[i]->saveData())
      return false;
  int result = RRI->save(Set);
  if(result) {
    LPCTSTR msg = 0;
    for(uint i = 0; i < SIZE_A(gErrReport); ++i) {
      if(gErrReport[i].err == result) {
        msg = gErrReport[i].txt;
        break;
        }
      }
    if(!msg)
      msg = _T("Sconosciuto!");
    MessageBox(*this, msg, _T("Errore!"), MB_OK | MB_ICONSTOP);
    return false;
    }
  findIndicator();
  return true;
}
//----------------------------------------------------------------------------
LPCTSTR filterIndicatorExt[] = { _T(".bmp"), 0 };
//----------------------------------------------------------------------------
LPCTSTR filterOpenIndicator =
  _T("File immagine per indicatore (bmp)\0")
  _T("*.bmp\0");
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool openFileImageIndicator(HWND owner, LPTSTR file)
{
  DWORD lastIx = 0;
  infoOpenSave Info(filterIndicatorExt, filterOpenIndicator, infoOpenSave::OPEN_F, lastIx);
  myOpenSave open(owner, SVMAKER_INI_FILE, OLD_IMAGE_PATH);

  if(open.run(Info)) {
    _tcscpy_s(file, _MAX_PATH, open.getFile());
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
#define INDICATOR_NAME _T("indicator.bmp")
//----------------------------------------------------------------------------
void pWizardChild3::findIndicator()
{
  dataProject& dp = getDataProject();
  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, dp.newPath);
  appendPath(path, dp.imagePath);
  appendPath(path, INDICATOR_NAME);
  if(!P_File::P_exist(path)) {
    if(IDYES != MessageBox(*this, _T("Vuoi copiarla nella cartella delle immagini?"),
              _T("L'immagine per l'indicatore non esiste"), MB_YESNO | MB_ICONINFORMATION))
      return;
    TCHAR source[_MAX_PATH];
    if(openFileImageIndicator(*this, source))
      CopyFile(source, path, FALSE);
    }
}
//----------------------------------------------------------------------------
void pWizardChild3::setStep(int ix)
{
  baseClass::setStep(ix);
  switch(ix) {
    case 1:
      child[1]->setActive(false);
      child[0]->setActive(true);
      break;
    case 2:
      child[0]->setActive(false);
      child[1]->setActive(true);
      break;
    default:
      child[0]->setActive(false);
      child[1]->setActive(false);
      break;
    }
}
//----------------------------------------------------------------------------
#define NAME_WIZ3 _T("Ricette su Righe")
//----------------------------------------------------------------------------
void pWizardChild3::getTitle(LPTSTR append, size_t lenBuff)
{
  switch(getStep()) {
    case 1:
      _tcscpy_s(append, lenBuff, NAME_WIZ3 _T(" [pag. 1]"));
      break;
    case 2:
      _tcscpy_s(append, lenBuff, NAME_WIZ3 _T(" [pag. 2]"));
      break;
    default:
      baseClass::getTitle(append, lenBuff);
      break;
    }
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool wzChild1::create()
{
  if(!baseClass::create())
    return false;

  SET_TEXT(IDC_EDIT_RROW_FILENAME, _T("Recipe"));

  fillRange(IDC_COMBO_RROW_NROW, MIN_ROW, MAX_ROW);
  fillRange(IDC_COMBO_RROW_NCOL, MIN_COLUMN, MAX_COLUMN);

  SET_CHECK(IDC_CHECK_RROW_USE_FIRST_TXT);

  fillCBPerif(GetDlgItem(*this, IDC_COMBO_RROW_EDIT_MEM), 0);

  fillCBPswLevel(GetDlgItem(*this, IDC_COMBO_RROW_PSW_SEND), 0);
  fillCBPswLevel(GetDlgItem(*this, IDC_COMBO_RROW_PSW_SAVE), 0);

  fillCBPerif(GetDlgItem(*this, IDC_COMBO_RROW_PRPH_SAVE_ACTIVE), 0);

  fillCBPerif(GetDlgItem(*this, IDC_COMBO_RROW_PRPH_ACTIVE), 0);
  fillCBPerif(GetDlgItem(*this, IDC_COMBO_RROW_PRPH_ENABLE), 0);

  fillCBTypeVal(GetDlgItem(*this, IDC_COMBO_RROW_TYPE_ACTIVE), 0);
  fillCBTypeVal(GetDlgItem(*this, IDC_COMBO_RROW_TYPE_ENABLE), 0);
  return true;
}
//----------------------------------------------------------------------------
LRESULT wzChild1::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(HIWORD(wParam)) {
        case CBN_SELCHANGE:
          if(IDC_COMBO_RROW_NCOL == LOWORD(wParam))
            setNCol();
          break;
        }
      switch(LOWORD(wParam)) {
        case IDC_CHECK_RROW_USE_FIRST_TXT:
          checkFirstEdit();
          break;

        }
      break;
    }

  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void wzChild1::checkFirstEdit()
{
  bool enable = IS_CHECKED(IDC_CHECK_RROW_USE_FIRST_TXT);
  EnableWindow(GetDlgItem(*this, IDC_COMBO_RROW_PRPH_SAVE_ACTIVE), enable);
  EnableWindow(GetDlgItem(*this, IDC_EDIT_RROW_ADDR_SAVE_ACTIVE), enable);
  RRI.ffT.use = enable;
}
//----------------------------------------------------------------------------
void wzChild1::fillRange(uint idc, int minV, int maxV)
{
  HWND hcb = GetDlgItem(*this, idc);
  TCHAR buff[20];
  for(int i = minV; i <= maxV; ++i) {
    wsprintf(buff, _T("%d"), i);
    SendMessage(hcb, CB_ADDSTRING, 0, (LPARAM)buff);
    }
  SendMessage(hcb, CB_SETCURSEL, 0, 0);
}
//----------------------------------------------------------------------------
bool wzChild1::saveData()
{
  GetDlgItemText(*this, IDC_EDIT_RROW_FILENAME, RRI.filename, MAX_DIM_FILENAME);

  RRI.initAddrMem4Edit = GetDlgItemInt(*this, IDC_EDIT_RROW_ADDR_ADDR_INIT, 0, 0);

  RRI.idPrphMem4Edit = SendMessage(GetDlgItem(*this, IDC_COMBO_RROW_EDIT_MEM), CB_GETCURSEL, 0 ,0);
  RRI.nRow = GetDlgItemInt(*this, IDC_COMBO_RROW_NROW, 0, 0);
  RRI.nCol = GetDlgItemInt(*this, IDC_COMBO_RROW_NCOL, 0, 0);

  RRI.ffT.use = IS_CHECKED(IDC_CHECK_RROW_USE_FIRST_TXT);
  RRI.ffT.prph = SendMessage(GetDlgItem(*this, IDC_COMBO_RROW_PRPH_SAVE_ACTIVE), CB_GETCURSEL, 0 ,0);
  RRI.ffT.addr = GetDlgItemInt(*this, IDC_EDIT_RROW_ADDR_SAVE_ACTIVE, 0 ,0);

  RRI.dP[0].prph = SendMessage(GetDlgItem(*this, IDC_COMBO_RROW_PRPH_ENABLE), CB_GETCURSEL, 0 ,0);
  RRI.dP[0].addr = GetDlgItemInt(*this, IDC_EDIT_RROW_ADDR_ENABLE, 0 ,0);
  RRI.dP[0].dataType = SendMessage(GetDlgItem(*this, IDC_COMBO_RROW_TYPE_ENABLE), CB_GETCURSEL, 0 ,0);
  RRI.dP[0].nbit = GetDlgItemInt(*this, IDC_EDIT_RROW_NBIT_ENABLE, 0 ,0);
  RRI.dP[0].offs = GetDlgItemInt(*this, IDC_EDIT_RROW_OFFS_ENABLE, 0 ,0);

  RRI.dP[1].prph = SendMessage(GetDlgItem(*this, IDC_COMBO_RROW_PRPH_ACTIVE), CB_GETCURSEL, 0 ,0);
  RRI.dP[1].addr = GetDlgItemInt(*this, IDC_EDIT_RROW_ADDR_ACTIVE, 0 ,0);
  RRI.dP[1].dataType = SendMessage(GetDlgItem(*this, IDC_COMBO_RROW_TYPE_ACTIVE), CB_GETCURSEL, 0 ,0);
  RRI.dP[1].nbit = GetDlgItemInt(*this, IDC_EDIT_RROW_NBIT_ACTIVE, 0 ,0);
  RRI.dP[1].offs = GetDlgItemInt(*this, IDC_EDIT_RROW_OFFS_ACTIVE, 0 ,0);

  RRI.pswSend = SendMessage(GetDlgItem(*this, IDC_COMBO_RROW_PSW_SEND), CB_GETCURSEL,0 ,0);
  if(RRI.pswSend > 3)
    RRI.pswSend = 1000;
  RRI.pswSave = SendMessage(GetDlgItem(*this, IDC_COMBO_RROW_PSW_SAVE), CB_GETCURSEL,0 ,0);
  if(RRI.pswSave > 3)
    RRI.pswSave = 1000;

  RRI.addr4SendActive = GetDlgItemInt(*this, IDC_EDIT_RROW_ADDR_PLC_NUM_ACTIVE, 0 ,0);

  return true;
}
//----------------------------------------------------------------------------
void wzChild1::setNCol()
{
  RRI.nCol = GetDlgItemInt(*this, IDC_COMBO_RROW_NCOL, 0, 0);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool wzChild2::create()
{
  if(!baseClass::create())
    return false;
  return true;
}
//----------------------------------------------------------------------------
LRESULT wzChild2::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
bool wzChild2::saveData()
{
  uint nc = min(MAX_COLUMN, RRI.nCol);
  PVect<labelAddr>& vla = RRI.vLabelAddr;
  vla.setDim(nc);
  for(uint i = 0; i < nc; ++i) {
    vla[i].addr = GetDlgItemInt(*this, IDC_EDIT_ADDR_RROW_1 + i, 0, 0);
    GetDlgItemText(*this, IDC_EDIT_LAB_RROW_1 + i, vla[i].label, MAX_DIM_LABEL);
    }
  return true;
}
//----------------------------------------------------------------------------
void wzChild2::showGroupWindow(uint ix, bool show)
{
  UINT s = show ? SW_SHOWNORMAL : SW_HIDE;
  ShowWindow(GetDlgItem(*this, IDC_EDIT_LAB_RROW_1 + ix), s);
  ShowWindow(GetDlgItem(*this, IDC_EDIT_ADDR_RROW_1 + ix), s);
  ShowWindow(GetDlgItem(*this, IDC_STATIC_IX_RROW_1 + ix), s);
}
//----------------------------------------------------------------------------
void wzChild2::setActive(bool set)
{
  if(set) {
    if(nCol != RRI.nCol) {
      uint nc = min(MAX_COLUMN, RRI.nCol);
      nCol = RRI.nCol;
      uint i = 0;
      for(; i < nc; ++i)
        showGroupWindow(i, true);
      for(; i < MAX_COLUMN; ++i)
        showGroupWindow(i, false);
      UINT s = nc > 10 ? SW_SHOWNORMAL : SW_HIDE;
      ShowWindow(GetDlgItem(*this, IDC_STATIC_LABEL_2), s);
      ShowWindow(GetDlgItem(*this, IDC_STATIC_LABEL_3), s);
      }
    EnableWindow(GetDlgItem(*this, IDC_EDIT_ADDR_RROW_1), !RRI.ffT.use);
    }
  baseClass::setActive(set);
}
//----------------------------------------------------------------------------
recipeRowInfo::recipeRowInfo() :
    idPrphMem4Edit(1), nRow(MIN_ROW), nCol(MIN_COLUMN), pswSend(0), pswSave(0), addr4SendActive(0),
    initAddrMem4Edit(0)
{
  _tcscpy_s(filename, _T("Recipe"));
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
int recipeRowInfo::save(setOfString& Set)
{
  if(!nCol)
    return eErrNoCol;
  if(!idPrphMem4Edit)
    return eErrNoPrph4Edit;

  P_File* pf = makeFileTmpCustom(true);
  if(!pf)
    return false;

  P_File* pfClear = makeFileTmpCustom(false);
  if(!pfClear) {
    delete pf;
    return false;
    }
  int result = performSave(Set, pf, pfClear);
  delete pf;
  delete pfClear;
  return result;
}
//----------------------------------------------------------------------------
static LPCTSTR defCustomSave[] = {
  _T("1000001,2001\r\n"),
  _T("1000004,4001\r\n"),
  };

struct menuLabel
{
  int id;
  LPCTSTR label;
};

static menuLabel ML[] = {
  { 1, _T("Pagina Ricette") },
  { 2, _T("Pagina Precedente") },
  { 6, _T("Rileggi") },
  { 8, _T("Attiva") },
  {10, _T("Riga\nSU") },
  {11, _T("Riga\nGIU'") },
  {13, _T("Allarme") },
  { 32, _T("4,0") },
  { 100, _T("16,0,0,arial") },
  {101, _T("16,0,2,arial") }

  };

//----------------------------------------------------------------------------
int recipeRowInfo::performSave(setOfString& Set, P_File* pf, P_File* pfClear)
{
  Set.reset();
  TCHAR buff[5000];

  int y = 41;

  int idLabel = 2001;
  int idText = 800001;

  // numeri laterali
  for(uint i = 0; i < nRow; ++i, y += 20, ++idText, ++idLabel) {
    wsprintf(buff, _T("50,%d,29,19,101,0,0,0,192,192,192,1,0,%d"), y, idText);
    Set.replaceString(idLabel, str_newdup(buff), true);
    wsprintf(buff, _T("%d"), i + 1);
    Set.replaceString(idText, str_newdup(buff), true);
    }

  for(uint i = 0; i < SIZE_A(ML); ++i)
    Set.replaceString(ML[i].id, str_newdup(ML[i].label), true);

  int x = 80;
  y = 4;
  int w = 700 / nCol - 1;
  int h = 34;

  // label sopra
  for(uint i = 0; i < nCol; ++i, x += w + 1, ++idText, ++idLabel) {
    wsprintf(buff, _T("%d,4,%d,34,100,0,0,0,255,255,0,1,0,%d"), x, w, idText);
    Set.replaceString(idLabel, str_newdup(buff), true);
    Set.replaceString(idText, str_newdup(vLabelAddr[i].label), true);
    }
  wsprintf(buff, _T("%d"), idLabel - 2001);
  Set.replaceString(41, str_newdup(buff), true);

  int idEdit = 4001;
  x = 80;
  y = 41;

  int addr = initAddrMem4Edit + 1;
  for(uint j = 0; j < nCol; ++j, x += w + 1, addr += 40) {
    int y2 = y;
    int addr2 = addr;
    for(uint i = 0; i < nRow; ++i, y2 += 20, ++idEdit, ++addr2) {
      wsprintf(buff, _T("%d,%d,%d,19,101,0,0,128,255,255,255,20,0"), x, y2, w);
      Set.replaceString(idEdit, str_newdup(buff), true);
      if(j || !ffT.use) {
        wsprintf(buff, _T("%d,%d,5,0,0"), idPrphMem4Edit, addr2);
        Set.replaceString(idEdit + 300, str_newdup(buff), true);
        }
      }
    }
  wsprintf(buff, _T("%d"), idEdit - 4001);
  Set.replaceString(43, str_newdup(buff), true);


  for(uint i = 0; i < SIZE_A(defCustomSave); ++i)
    writeStringForceUnicode(*pf, defCustomSave[i]);

  wsprintf(buff, _T("1000002,%d\r\n1000003,%d\r\n1000006,%d\r\n1000008,%s\r\n"), nRow, nCol, initAddrMem4Edit, filename);
  writeStringForceUnicode(*pf, buff);


  if(ffT.use) {
    wsprintf(buff, _T("1000007,1\r\n1000010,%d,%d\r\n"), ffT.prph, ffT.addr);
    writeStringForceUnicode(*pf, buff);
    }

  wsprintf(buff, _T("1000005,%d"), addr4SendActive ? addr4SendActive : -1);
  if(ffT.use)
    _tcscat_s(buff, _T(",-1"));
  else {
    TCHAR t[20];
    wsprintf(t, _T(",%d"), vLabelAddr[0].addr);
    _tcscat_s(buff, t);
    }
  uint nElem = vLabelAddr.getElem();
  for(uint i = 1; i < nElem; ++i) {
    TCHAR t[20];
    wsprintf(t, _T(",%d"), vLabelAddr[i].addr);
    _tcscat_s(buff, t);
    }
  _tcscat_s(buff, _T("\r\n"));
  writeStringForceUnicode(*pf, buff);

  wsprintf(buff, _T("1000011,%d,%d,%d,%d,%d\r\n")
                 _T("1000012,%d,%d,%d,%d,%d\r\n")
                 _T("1000013,%d\r\n")
                 _T("1000014,%d\r\n"),
                 dP[0].prph, dP[0].addr, dP[0].dataType, dP[0].nbit, dP[0].offs,
                 dP[1].prph, dP[1].addr, dP[1].dataType, dP[1].nbit, dP[1].offs,
                 pswSend, pswSave);

  writeStringForceUnicode(*pf, buff);

  dataProject& dp = getDataProject();
  dp.setPageType(ptcRecipeRow);

  return Set.setFirst() ? eErrNoErr : SIZE_A(gErrReport) + 1;
}
