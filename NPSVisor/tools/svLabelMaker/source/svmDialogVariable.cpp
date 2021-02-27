//--------------- svmDialogVariable.cpp ---------------------
//-----------------------------------------------------------
#include "precHeader.h"
#include "macro_utils.h"
#include "svmDialogVariable.h"
#include "p_txt.h"
//-----------------------------------------------------------
static manageVariable gInfoVar[MAX_PAGES];
//-----------------------------------------------------------
manageVariable& getManVariables()
{
  dataProject& dp = getDataProject();
  return gInfoVar[dp.currPage];
}
//-----------------------------------------------------------
class manageCopyVars
{
  public:
    manageCopyVars() : pageSource(0) {}
    manageCopyVars(uint pageSource, PVect<infoVariable> vars) : pageSource(pageSource), Vars(vars) {}
    void pasteTo(PWin* owner, uint page);
    void copyNew(PWin* owner, uint pgSource, PVect<int> vars);
    enum eAction { eActSkip, eActSkipAll, eActCopy, eActCopyAll };
    bool canPaste() const { return Vars.getElem() > 0; }
  private:
    uint pageSource;
    PVect<infoVariable> Vars;
    eAction getAction(PWin* owner, uint id);
};
//-----------------------------------------------------------
static manageCopyVars gManageCopyVars;
//-----------------------------------------------------------
namespace prphDataType {
enum prphType
{
  tNoData,  // nessun tipo definito -  0
  tBitData, // campo di bit         -  1
  tBData,   // 8 bit -> 1 byte      -  2
  tWData,   // 16 bit-> 2 byte      -  3
  tDWData,  // 32 bit-> 4 byte      -  4
  tFRData,  // 32 bit float         -  5
  ti64Data, // 64 bit               -  6
  tRData,   // 64 bit float         -  7
  tDateHour,// 64 bit per data + ora-  8
  tDate,    // 64 bit per data      -  9
  tHour,     // 64 bit per ora      - 10
  // valori signed
  tBsData,   // 8 bit -> 1 byte      -  11
  tWsData,   // 16 bit-> 2 byte      -  12
  tDWsData,  // 32 bit-> 4 byte      -  13

  tStrData,  // struttura formata dalla dimensione del buffer
};
};
//-----------------------------------------------------------
infoVariable::infoVariable(uint id, uint prph, uint addr, uint type, uint dec_len_offs, int normid, uint n_bit) :
  id(id), prph(prph), addr(addr), type(type), dec_len_offs(dec_len_offs), nBit(n_bit), norm_id(normid), useId(false)
{
  if(norm_id < 0 || norm_id > 0 && prphDataType::tBitData == type) {
    useId = true;
//    norm_id = -norm_id;
    }
}
//-----------------------------------------------------------
manageVariable::manageVariable() {}
//-----------------------------------------------------------
manageVariable::~manageVariable()
{
  reset();
}
//-----------------------------------------------------------
void manageVariable::reset()
{
  Vars.reset();
  uint nElem = VarsText.getElem();
  for(uint i = 0; i < nElem; ++i)
    safeDeleteP(VarsText[i]);
  VarsText.reset();
}
//-----------------------------------------------------------
static int fillInt(PVect<int>& id, LPCTSTR row, int num = -1)
{
  int ix = 0;
  while(row) {
    id[ix] = _ttoi(row);
    ++ix;
    row = findNextParamTrim(row);
    }
  if(num < 0)
    num = ix;
  for(; ix < num; ++ix)
    id[ix] = 0;
  return num;
}
//-----------------------------------------------------------
infoVariableText* manageVariable::findText(uint id)
{
  const PVect<infoVariableText*>& vt = VarsText;
  uint nElem = vt.getElem();
  for(uint i = 0; i < nElem; ++i)
    if(vt[i]->id == id)
      return VarsText[i];
  return 0;
}
//-----------------------------------------------------------
void manageVariable::load(LPCTSTR filename, const uint* idset)
{
  reset();
  setOfString set(filename);
  if(!set.setFirst())
    return;
  PVect<int> ids[MAX_ID_DATA_VARS];
  int nElem = fillInt(ids[0], set.getString(idset[0]));
  for(uint i = 1; i < MAX_ID_DATA_VARS; ++i)
    fillInt(ids[i], set.getString(idset[i]), nElem);

  for(int i = 0; i < nElem; ++i) {
    if(!ids[0][i])
      continue;
    infoVariable iv(i + 1, ids[0][i], ids[1][i], ids[2][i], ids[3][i], ids[4][i]);
    if(iv.useId) {
      int norm_id = abs(iv.norm_id);
      LPCTSTR p = set.getString(norm_id);
      if(p) {
        // nel tipo bit la prima riga contiene il numero di bit ed il testo per nessun
        // bit attivo, mentre nel tipo intero la prima riga contiene il numero di bit,
        // quindi in entrambi i casi c'è il numero di bit
        // se il norm è positivo, nel tipo bit, i due testi abbinati sono allo stesso codice separati dalla virgola
        iv.nBit = _ttoi(p);
        // per portare il primo id corrispondente a quello che si vedrà nel dialogo (per tipo word)
        int subId = 1;
        if(prphDataType::tBitData == iv.type) {
          LPCTSTR p2 = p;
          if(iv.norm_id < 0) {
            p2 = findNextParamTrim(p);
            if(!p2) {
              p2 = p;
              iv.nBit = 1;
              }
            }
          infoVariableText* ivt = new infoVariableText;
          ivt->id = norm_id;
          ivt->text = str_newdup(p2);
          VarsText[VarsText.getElem()] = ivt;
          // nel tipo bit il primo id corrisponde, quindi non va sottratto il primo valore
          subId = 0;
          }
        for(uint k = norm_id + 1, j = 0; j < 128; ++j, ++k) {
          p = set.getString(k);
          if(!p)
            break;
          infoVariableText* ivt = new infoVariableText;
          ivt->id = k - subId;
          ivt->text = str_newdup(p);
          VarsText[VarsText.getElem()] = ivt;
          }
        }
      }
    Vars[Vars.getElem()] = iv;
    }
}
//-----------------------------------------------------------
bool manageVariable::save(LPCTSTR filename, uint* idset)
{
  do {
    P_File pf(filename);
    pf.appendback();
    } while(false);
  P_File pf(filename, P_CREAT);
  if(!pf.P_open())
    return false;
  TCHAR buff[4096 * 2];
  uint nElem = Vars.getElem();
  uint lastId = 0;
  if(nElem) {
    lastId = Vars[nElem - 1].id;
    PVect<int> ids[MAX_ID_DATA_VARS];
    for(uint i = 0; i < SIZE_A(ids); ++i)
      ids[i].setDim(lastId);
    for(uint i = 0; i < lastId; ++i) {
      ids[0][i] = 0;
      ids[1][i] = 0;
      ids[2][i] = 0;
      ids[3][i] = 0;
      ids[4][i] = 0;
      }
    for(uint i = 0; i < nElem; ++i) {
      const infoVariable& iv = Vars[i];
      int ix = iv.id - 1;
      ids[0][ix] = iv.prph;
      ids[1][ix] = iv.addr;
      ids[2][ix] = iv.type;
      ids[3][ix] = iv.dec_len_offs;
      ids[4][ix] = iv.norm_id;
      int norm_id = abs(iv.norm_id);
      if(iv.useId) {
        if(prphDataType::tBitData == iv.type) {
          if(iv.nBit > 1) {
            infoVariableText* txt = findText(norm_id);
            if(txt) {
              LPTSTR t2 = new TCHAR[_tcslen(txt->text) + 8];
              wsprintf(t2, _T("%d,%s"), iv.nBit, txt->text);
              delete []txt->text;
              txt->text = t2;
              }
            }
          else
            ids[4][ix] = abs(iv.norm_id);
          }
        else {
          int j = 0;
          for(; j < 128; ++j)
            if(!findText(norm_id + j))
              break;

          for(--j; j >= 0; --j) {
            infoVariableText* txt = findText(norm_id + j);
            if(!txt)
              break;
            ++txt->id;
            }
          infoVariableText* txt = new infoVariableText;
          txt->id = norm_id;
          TCHAR t[64];
          wsprintf(t, _T("%d"), iv.nBit);
          txt->text = str_newdup(t);
          VarsText[VarsText.getElem()] = txt;
          }
        }
      }
    for(uint j = 0; j < SIZE_A(ids); ++j) {
      idset[j] = ID_FILE_DATA_INIT_SET_VARS + j;
      wsprintf(buff, _T("%d"), ID_FILE_DATA_INIT_SET_VARS + j);
      LPTSTR p = buff  + _tcslen(buff);
      const PVect<int>& id = ids[j];
      for(uint i = 0; i < lastId; ++i) {
        wsprintf(p, _T(",%d"), id[i]);
        p += _tcslen(p);
        }
      wsprintf(p, _T("\r\n"));
      writeStringChkUnicode(pf, buff);
      }
    }
  nElem = VarsText.getElem();
  for(uint i = 0; i < nElem; ++i) {
    wsprintf(buff, _T("%d,"), VarsText[i]->id);
    writeStringChkUnicode(pf, buff);
    writeStringChkUnicode(pf, VarsText[i]->text);
    writeStringChkUnicode(pf, _T("\r\n"));
    }
  pf.P_close();
  // dovrei mantenere la lista dei testi aggiunti e di quelli cambiati :-(
  // ricarico tutto :-)
  load(filename, idset);
  return true;
}
//-----------------------------------------------------------
extern void addStringToComboBox(HWND hList, LPCTSTR str, uint addSize);
//-----------------------------------------------------------
static void fillPrph(HWND hwcb)
{
  LPCTSTR prph[] = {
    _T("01-Prf.Mem"),
    _T("02-Prf.PLC"),
    _T("03-Prf.Gen"),
    _T("04-Prf.Gen"),
    _T("05-Prf.Gen"),
    _T("06-Prf.Gen"),
    _T("07-Prf.Gen"),
    _T("08-Prf.Gen"),
    _T("09-Prf.Gen"),

    _T("10-Prf.Mem+"),
    _T("11-Prf.Mem+"),
    _T("12-Prf.Mem+"),
    _T("13-Prf.Mem+"),
    _T("14-Prf.Mem+"),
    _T("15-Prf.Mem+"),
    _T("16-Prf.Mem+"),
    _T("17-Prf.Mem+"),
    _T("18-Prf.Mem+"),
    _T("19-Prf.Mem+"),
    _T("20-Prf.Mem+"),
    };
  for(int i = 0; i < SIZE_A(prph); ++i)
    addStringToComboBox(hwcb, prph[i], 0);
}
//-----------------------------------------------------------
static void fillType(HWND hwcb)
{
  LPCTSTR cbType[] = {
    _T(" 1 - bit"),
    _T(" 2 - byte"),
    _T(" 3 - word"),
    _T(" 4 - dword"),
    _T(" 5 - float"),
    _T(" 6 - int64"),
    _T(" 7 - real"),
    _T(" 8 - dateHour"),
    _T(" 9 - Date"),
    _T("10 - Hour"),
    _T("11 - char"),
    _T("12 - short"),
    _T("13 - long"),
    _T("14 - string"),
    };
  for(int i = 0; i < SIZE_A(cbType); ++i)
    addStringToComboBox(hwcb, cbType[i], 0);
}
//-----------------------------------------------------------
bool svmDialogVariable::returnId = true;
//-----------------------------------------------------------
bool svmDialogVariable::create()
{
  ListVar = new PCustomListView(this, IDC_LISTVIEW_VAR);
  ListText = new PCustomListViewFullEdit(this, IDC_LISTVIEW_TEXT);
  ListText->Attr.exStyle |= WS_EX_CLIENTEDGE;
/*
  HFONT hf = D_FONT(16, 0, 0, _T("comic sans ms"));
  ListVar->setFont(hf, true);
  ListText->setFont(hf, false);
*/
  if(!baseClass::create())
    return false;

//  DWORD dwExStyle = LVS_EX_FULLROWSELECT;// | LVS_EX_GRIDLINES;
  DWORD dwExStyle = LVS_EX_FLATSB | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
  ListView_SetExtendedListViewStyle(*ListVar, dwExStyle);
  ListView_SetExtendedListViewStyle(*ListText, dwExStyle);

  fillPrph(GetDlgItem(*this, IDC_COMBO_PRPH));
  fillType(GetDlgItem(*this, IDC_COMBO_TYPE));

  makeHead();
  fillList();
  if(returnId)
    SET_CHECK(IDC_CHECK_RET_ID);
  if(!gManageCopyVars.canPaste())
    ENABLE(IDC_BUTTON_PASTE, false);
  return true;
}
//-----------------------------------------------------------
void svmDialogVariable::makeHead()
{
  struct s_head {
    LPCTSTR title;
    uint dim;
    } head[] = {
    { _T("Num. Var."), 50 },
    { _T("Periferica"), 50 },
    { _T("Indirizzo"), 50 },
    { _T("Tipo dato"), 50 },
    { _T("Dec-Len-Offs"), 50 },
    { _T("N. Bit"), 50 },
    { _T("Norm-Id Testo"), 50 },
    };
  PRect r;
  GetClientRect(*ListVar, r);
  int cx = (r.Width() - 20) / SIZE_A(head);

  LVCOLUMN lvCol;
  lvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
  lvCol.cx = cx;
  lvCol.fmt = LVCFMT_CENTER;
  for(uint i = 0; i < SIZE_A(head); ++i) {
    lvCol.pszText = (LPTSTR)head[i].title;
    ListView_InsertColumn(*ListVar, i, &lvCol);
    }

  GetClientRect(*ListText, r);
  cx = r.Width() - 10 - 50;

  lvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
  lvCol.cx = 50;
  lvCol.fmt = LVCFMT_LEFT;
  lvCol.pszText = _T("Id");
  ListView_InsertColumn(*ListText, 0, &lvCol);
  lvCol.cx = cx;
  lvCol.pszText = _T("Testo");
  ListView_InsertColumn(*ListText, 1, &lvCol);
}
//-----------------------------------------------------------
static void insertItemInt(HWND hlv, int v, uint item, uint sub)
{
  TCHAR t[64];
  wsprintf(t, _T("%d"), v);
  ListView_SetItemText(hlv, item, sub, t);
}
//-----------------------------------------------------------
static void addRow_Var(HWND hlv, const infoVariable& row, uint item = 65535)
{
  TCHAR t[64] = {0};
  LVITEM lvItem;
  lvItem.mask = LVIF_TEXT;
  lvItem.iItem = item; //65535;
  lvItem.iSubItem = 0;
  wsprintf(t, _T("%4d"), row.id);
  lvItem.pszText = t;

  int ix = ListView_InsertItem(hlv, &lvItem);
  insertItemInt(hlv, row.prph, ix, 1);
  insertItemInt(hlv, row.addr, ix, 2);
  insertItemInt(hlv, row.type, ix, 3);
  insertItemInt(hlv, row.dec_len_offs, ix, 4);
  insertItemInt(hlv, row.nBit, ix, 5);
  insertItemInt(hlv, row.norm_id, ix, 6);
}
//-----------------------------------------------------------
void svmDialogVariable::addRowVar(HWND hlv, const infoVariable& row, uint item)
{
  addRow_Var(hlv, row, item);
}
//-----------------------------------------------------------
int svmDialogVariable::addRowText(HWND hlv, const infoVariableText& row, uint item)
{
  TCHAR t[64] = {0};
  LVITEM lvItem;
  lvItem.mask = LVIF_TEXT;
  lvItem.iItem = item; //65535;
  lvItem.iSubItem = 0;
  wsprintf(t, _T("%d"), row.id);
  lvItem.pszText = t;

  int ix = ListView_InsertItem(hlv, &lvItem);
  ListView_SetItemText(hlv, ix, 1, (LPTSTR)row.text);
  return ix;
}
//-----------------------------------------------------------
void svmDialogVariable::fillListVars()
{
  ListView_DeleteAllItems(*ListVar);

  manageVariable& man_vars = getManVariables();
  PVect<infoVariable>& vars = man_vars.getVariables();
  uint nElem = vars.getElem();
  for(uint i = 0; i < nElem; ++i) {
    addRowVar(*ListVar, vars[i]);
    if(Selected == vars[i].id) {
      ListView_SetItemState(*ListVar, i, LVIS_FOCUSED | LVIS_SELECTED, 0x000f);
      ListView_EnsureVisible(*ListVar, i, TRUE);
      }
    }
}
//-----------------------------------------------------------
void svmDialogVariable::fillList()
{
  fillListVars();

  manageVariable& man_vars = getManVariables();
  PVect<infoVariableText*>& text = man_vars.getVariablesText();
  uint nElem = text.getElem();
  for(uint i = 0; i < nElem; ++i)
    addRowText(*ListText, *text[i]);
}
//-----------------------------------------------------------
LRESULT svmDialogVariable::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_COMBO_TYPE:
          if(CBN_SELCHANGE == HIWORD(wParam))
            checkAll();
          break;
        case IDC_RADIO_DIRECT:
        case IDC_RADIO_BY_VAL:
          checkAll();
          break;

        case IDC_BUTTON_ADD:
          addField();
          break;
        case IDC_BUTTON_REM:
          remField();
          break;

        case IDC_BUTTON_PLUS:
          PostMessage(*this, WM_FW_FIRST_FREE, 0, 0);
          break;
        case IDC_BUTTON_MINUS:
          remText();
          break;
        case IDC_BUTTON_COPY:
          Copy();
          break;
        case IDC_BUTTON_PASTE:
          Paste();
          break;
        case IDOK:
          if(ListText->getEditHandle() == HWND(lParam))
            return 1;
          break;
        }
      break;
    case WM_FW_FIRST_FREE:
      addText();
      break;
    case WM_NOTIFY:
      if(LOWORD(wParam) == IDC_LISTVIEW_TEXT) {
        LRESULT res;
        if(ListText->ProcessNotify(lParam, res)) {
          SetWindowLong (*this, DWL_MSGRESULT, res);
          return TRUE;
          }
        }
      else if(LOWORD(wParam) == IDC_LISTVIEW_VAR) {
        LPNMHDR lpnmHdr = (LPNMHDR)lParam;
        switch(lpnmHdr->code) {
//          case LVN_ODSTATECHANGED:
          case LVN_ITEMCHANGED:
            loadField();
            break;
          }
        }
      break;
    case WM_VSCROLL:
      if(lParam) {
        switch(LOWORD(wParam)) {
          case SB_THUMBPOSITION:
          case SB_THUMBTRACK:
            break;
          default:
            InvalidateRect((HWND)lParam, 0 ,0);
            break;
          }
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
static void getCurrSel(HWND hwlv, PVect<int>& res)
{
  int pos = -1;
  do {
    pos = ListView_GetNextItem(hwlv, pos, LVNI_ALL | LVNI_SELECTED);
    if(pos < 0)
      break;
    res[res.getElem()] = pos;
    }  while(true);
}
//-----------------------------------------------------------
static int getItemInt(HWND hlv, uint item, uint sub)
{
  TCHAR t[64] = _T("");
  ListView_GetItemText(hlv, item, sub, t, SIZE_A(t));
  trim(lTrim(t));
  return _ttoi(t);
}
//-----------------------------------------------------------
void svmDialogVariable::Copy()
{
  PVect<int> selected;
  getCurrSel(*ListVar, selected);
#if 0
  uint nElem = selected.getElem();
  if(nElem) {
    for(uint i = 0; i < nElem; ++i) {
      uint id = getItemInt(*ListVar, selected[i], 0);
      selected[i] = id;
      }
    dataProject& dp = getDataProject();
    gManageCopyVars.copyNew(this, dp.currPage, selected);
    }
#else
  dataProject& dp = getDataProject();
  gManageCopyVars.copyNew(this, dp.currPage, selected);
#endif
}
//-----------------------------------------------------------
void svmDialogVariable::Paste()
{
  if(!gManageCopyVars.canPaste())
    return;
  dataProject& dp = getDataProject();
  gManageCopyVars.pasteTo(this, dp.currPage);
//  fillListVars();
}
//-----------------------------------------------------------
bool svmDialogVariable::checkNotExist(const infoVariable& row)
{
  int count = ListView_GetItemCount(*ListVar);
  for(int i = 0; i < count; ++i) {
    uint id = getItemInt(*ListVar, i, 0);
    if(id == row.id) {
      if(IDYES == MessageBox(*this, _T("L'id già esiste, vuoi sovrascriverlo?"), _T("Attenzione"), MB_YESNO | MB_ICONSTOP)) {
        ListView_DeleteItem(*ListVar, i);
        return true;
        }
      return false;
      }
    else if(id > row.id)
      return true;
    }
  return true;
}
//-----------------------------------------------------------
void svmDialogVariable::addField()
{
  infoVariable row;
  fromField(row);
  if(checkNotExist(row))
    addRowVar(*ListVar, row);
}
//-----------------------------------------------------------
void svmDialogVariable::remField()
{
  PVect<int> selected;
  getCurrSel(*ListVar, selected);
  infoVariable row;
  int nElem = selected.getElem();
  for(int i = nElem - 1; i >= 0; --i) {
    int ix = selected[i];
    getRowVar(*ListVar, row, ix);
    toField(row);
    ListView_DeleteItem(*ListVar, ix);
    }
  if(nElem) {
    int count = ListView_GetItemCount(*ListVar);
    int ix = selected[0];
    if(ix >= count)
      ix = count - 1;
    if(ix < 0)
      return;
    ListView_SetItemState(*ListVar, ix, LVIS_FOCUSED | LVIS_SELECTED, 0x000f);
    ListView_EnsureVisible(*ListVar, ix, TRUE);
    }
}
//-----------------------------------------------------------
void svmDialogVariable::loadField()
{
  PVect<int> selected;
  getCurrSel(*ListVar, selected);
  if(selected.getElem()) {
    infoVariable row;
    int ix = selected[0];
    getRowVar(*ListVar, row, ix);
    toField(row);
    }
}
//-----------------------------------------------------------
void svmDialogVariable::addText()
{
  infoVariableText text;
  int ix = addRowText(*ListText, text);
  SetFocus(*ListText);
  ListView_SetItemState(*ListText, -1, 0, LVIS_FOCUSED | LVIS_SELECTED);
  ListView_SetItemState(*ListText, ix, LVIS_SELECTED, LVIS_SELECTED);
  ListView_EnsureVisible(*ListText, ix, false);
  ListText->setiItem(ix);
  ListText->setiSubItem(0);
  ListView_EditLabel(*ListText, ix);
}
//-----------------------------------------------------------
void svmDialogVariable::remText()
{
  PVect<int> sel;
  int ix = -1;
  do {
    ix = ListView_GetNextItem(*ListText, ix, LVNI_SELECTED);
    if(-1 == ix)
      break;
    sel[sel.getElem()] = ix;
    } while(true);
  int nElem = sel.getElem();
  for(int i = nElem - 1; i >= 0; --i)
    ListView_DeleteItem(*ListText, sel[i]);
  int count = ListView_GetItemCount(*ListText);
  if(count <= sel[nElem - 1])
    --count;
  else
    count = sel[nElem - 1];
  ListView_SetItemState(*ListText, count, LVIS_SELECTED, LVIS_SELECTED);
  ListView_EnsureVisible(*ListText, count, false);
}
//-----------------------------------------------------------
static void getRow_Var(HWND hlv, infoVariable& row, int ix)
{
  row.id = getItemInt(hlv, ix, 0);
  row.prph = getItemInt(hlv, ix, 1);
  row.addr = getItemInt(hlv, ix, 2);
  row.type = getItemInt(hlv, ix, 3);
  row.dec_len_offs = getItemInt(hlv, ix, 4);
  row.nBit = getItemInt(hlv, ix, 5);
  row.norm_id = getItemInt(hlv, ix, 6);
  row.useId = row.norm_id < 0 || row.norm_id > 0 && prphDataType::tBitData == row.type;
}
//-----------------------------------------------------------
void svmDialogVariable::getRowVar(HWND hlv, infoVariable& row, int ix)
{
  getRow_Var(hlv, row, ix);
}
//-----------------------------------------------------------
void svmDialogVariable::getRowText(HWND hlv, infoVariableText& row, int ix)
{
  row.id = getItemInt(hlv, ix, 0);
  TCHAR t[4096] = _T("");
  ListView_GetItemText(hlv, ix, 1, t, SIZE_A(t));
  row.text = str_newdup(t);
}
//-----------------------------------------------------------
void svmDialogVariable::checkAll()
{
  ENABLE(IDC_EDIT_LEN, false);
  ENABLE(IDC_EDIT_OFFS, false);
  ENABLE(IDC_EDIT_N_DEC, false);
  ENABLE(IDC_EDIT_NORM, false);
  ENABLE(IDC_EDIT_ID_TXT, false);
  ENABLE(IDC_RADIO_DIRECT, false);
  ENABLE(IDC_RADIO_BY_VAL, false);
  int type = SendMessage(GetDlgItem(*this, IDC_COMBO_TYPE), CB_GETCURSEL, 0, 0) + 1;
  switch(type) {
    case prphDataType::tFRData:
    case prphDataType::tRData:
      ENABLE(IDC_EDIT_N_DEC, true);
      ENABLE(IDC_EDIT_NORM, true);
      break;

    case prphDataType::tStrData:
      ENABLE(IDC_EDIT_LEN, true);
      break;

    case prphDataType::tDateHour:
    case prphDataType::tDate:
    case prphDataType::tHour:
      break;

    default:
      ENABLE(IDC_RADIO_DIRECT, true);
      ENABLE(IDC_RADIO_BY_VAL, true);
      ENABLE(IDC_EDIT_OFFS, true);
      if(IS_CHECKED(IDC_RADIO_BY_VAL)) {
        SET_CHECK(IDC_RADIO_BY_VAL);
        SET_CHECK_SET(IDC_RADIO_DIRECT, false);
        ENABLE(IDC_EDIT_ID_TXT, true);
        }
      else {
        SET_CHECK_SET(IDC_RADIO_BY_VAL, false);
        SET_CHECK(IDC_RADIO_DIRECT);
        ENABLE(IDC_EDIT_NORM, true);
        }
      break;
    }
}
//-----------------------------------------------------------
void svmDialogVariable::toField(const infoVariable& row)
{
  SET_INT(IDC_EDIT_ID, row.id);
  SET_INT(IDC_EDIT_ADDR, row.addr);
  SET_INT(IDC_EDIT_NBIT, row.nBit);
  SendMessage(GetDlgItem(*this, IDC_COMBO_PRPH), CB_SETCURSEL, row.prph - 1, 0);
  SendMessage(GetDlgItem(*this, IDC_COMBO_TYPE), CB_SETCURSEL, row.type - 1, 0);

  switch(row.type) {
    case prphDataType::tFRData:
    case prphDataType::tRData:
      SET_INT(IDC_EDIT_N_DEC, row.dec_len_offs);
      SET_INT(IDC_EDIT_NORM, row.norm_id);
      break;

    case prphDataType::tStrData:
      SET_INT(IDC_EDIT_LEN, row.dec_len_offs);
      break;

    case prphDataType::tDateHour:
    case prphDataType::tDate:
    case prphDataType::tHour:
      break;

    default:
      SET_INT(IDC_EDIT_OFFS, row.dec_len_offs);
      if(row.useId) {
        SET_CHECK(IDC_RADIO_BY_VAL);
        SET_CHECK_SET(IDC_RADIO_DIRECT, false);
        SET_INT(IDC_EDIT_ID_TXT, abs(row.norm_id));
        }
      else {
        SET_CHECK_SET(IDC_RADIO_BY_VAL, false);
        SET_CHECK(IDC_RADIO_DIRECT);
        SET_INT(IDC_EDIT_NORM, row.norm_id);
        }
      break;
    }
  checkAll();
}
//-----------------------------------------------------------
void svmDialogVariable::fromField(infoVariable& row)
{
  GET_INT(IDC_EDIT_ID, row.id);
  GET_INT(IDC_EDIT_ADDR, row.addr);
  GET_INT(IDC_EDIT_NBIT, row.nBit);
  row.prph = SendMessage(GetDlgItem(*this, IDC_COMBO_PRPH), CB_GETCURSEL, 0, 0) + 1;
  row.type = SendMessage(GetDlgItem(*this, IDC_COMBO_TYPE), CB_GETCURSEL, 0, 0) + 1;

  switch(row.type) {
    case prphDataType::tFRData:
    case prphDataType::tRData:
      GET_INT(IDC_EDIT_N_DEC, row.dec_len_offs);
      GET_INT(IDC_EDIT_NORM, row.norm_id);
      break;

    case prphDataType::tStrData:
      GET_INT(IDC_EDIT_LEN, row.dec_len_offs);
      break;

    case prphDataType::tDateHour:
    case prphDataType::tDate:
    case prphDataType::tHour:
      break;
    default:
      if(IS_CHECKED(IDC_RADIO_BY_VAL)) {
        row.useId = true;
        GET_INT(IDC_EDIT_ID_TXT, row.norm_id);
        row.norm_id = -row.norm_id;
        }
      else {
        row.useId = false;
        GET_INT(IDC_EDIT_NORM, row.norm_id);
        }
      break;
    }
}
//-----------------------------------------------------------
void svmDialogVariable::CmOk()
{
  manageVariable& man_vars = getManVariables();
  man_vars.reset();
  if((returnId = IS_CHECKED(IDC_CHECK_RET_ID)) != false) {
    int selected = ListView_GetNextItem(*ListVar, -1, LVNI_ALL | LVNI_SELECTED);
    if(selected >= 0)
      Selected = getItemInt(*ListVar, selected, 0);
    }
  int nElem = ListView_GetItemCount(*ListVar);
  PVect<infoVariable>& vars = man_vars.getVariables();
  for(int i = 0; i < nElem; ++i)
    getRowVar(*ListVar, vars[i], i);

  nElem = ListView_GetItemCount(*ListText);
  PVect<infoVariableText*>& text = man_vars.getVariablesText();
  for(int i = 0; i < nElem; ++i) {
    text[i] = new infoVariableText;
    getRowText(*ListText, *text[i], i);
    }
  dataProject& dp = getDataProject();
  dp.setDirty(true);
  baseClass::CmOk();
}
//-----------------------------------------------------------
void svmDialogVariable::CmCancel()
{
  baseClass::CmCancel();
}
//-----------------------------------------------------------
class svmDialogOffsetVariable : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    svmDialogOffsetVariable(PWin* parent, uint id, int& offset, HINSTANCE hInst = 0) :
      baseClass(parent, id, hInst), Offset(offset) {  }
    ~svmDialogOffsetVariable() { destroy(); }

    virtual bool create();
  protected:
    int& Offset;
    void CmOk() { GET_INT(IDC_EDIT_OFFSET, Offset); baseClass::CmOk(); }
};
//-----------------------------------------------------------
bool svmDialogOffsetVariable::create()
{
  if(!baseClass::create())
    return false;
  SET_INT(IDC_EDIT_OFFSET, Offset);
  return true;
}
//-----------------------------------------------------------
bool getOffsetVar(PWin*par, int& v)
{
  return IDOK == svmDialogOffsetVariable(par, IDD_DIALOG_OFFSET, v).modal();
}
//-----------------------------------------------------------
class PDlg_Action : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    PDlg_Action(PWin* parent, uint& action, uint idVar,  uint id = IDD_DIALOG_ACTION, HINSTANCE hInst = 0) :
      baseClass(parent, id, hInst), Action(action), idVar(idVar) {  }
    ~PDlg_Action() { destroy(); }

    virtual bool create();
  protected:
    uint& Action;
    uint idVar;
    void CmOk();
    void CmCancel() {}
    static POINT pt;
};
//-----------------------------------------------------------
POINT PDlg_Action::pt = { 100, 100 };
//-----------------------------------------------------------
bool PDlg_Action::create()
{
  if(!baseClass::create())
    return false;
  uint idc[] = { IDC_SKIP, IDC_SKIP_ALL, IDC_APPLY, IDC_APPLY_ALL };
  SET_CHECK(idc[Action]);
  TCHAR t[64];
  GET_TEXT(IDC_STATIC_INFO_VARID, t);
  TCHAR t2[512];
  wsprintf(t2, t, idVar);
  SET_TEXT(IDC_STATIC_INFO_VARID, t2);
  SetWindowPos(*this, 0, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
  return true;
}
//-----------------------------------------------------------
void PDlg_Action::CmOk()
{
  uint idc[] = { IDC_SKIP, IDC_SKIP_ALL, IDC_APPLY, IDC_APPLY_ALL };
  for(uint i = 0; i < SIZE_A(idc); ++i)
    if(IS_CHECKED(idc[i])) {
      Action = i;
      break;
      }
  PRect r;
  GetWindowRect(*this, r);
  pt.x = r.left;
  pt.y = r.top;
  baseClass::CmOk();
}
//-----------------------------------------------------------
manageCopyVars::eAction manageCopyVars::getAction(PWin* owner, uint id)
{
  static uint action = eActSkip;
  PDlg_Action(owner, action, id).modal();
  return (eAction)action;
}
//-----------------------------------------------------------
void manageCopyVars::copyNew(PWin* owner, uint pgSource, PVect<int> vars)
{
  Vars.reset();
  uint nElem = vars.getElem();
  Vars.setDim(nElem);
  HWND hwList = GetDlgItem(*owner, IDC_LISTVIEW_VAR);
  for(uint i = 0; i < nElem; ++i)
    getRow_Var(hwList, Vars[i], vars[i]);
  pageSource = pgSource;
}
//-----------------------------------------------------------
int varCmp(const infoVariable& v, const infoVariable& ins)
{
  return (int)v.id - (int)ins.id;
}
//-----------------------------------------------------------
static int existVar(const PVect<infoVariable>& t, const infoVariable& iv)
{
  uint pos;
  if(t.find(iv, pos, varCmp))
    return pos;
  return -1;
}
//-----------------------------------------------------------
static void pasteRow(PVect<infoVariable>& target, infoVariable& source)
{
  int res = existVar(target, source);
  if(res < 0)
    res = target.getElem();
  target[res] = source;
}
//-----------------------------------------------------------
void manageCopyVars::pasteTo(PWin* owner, uint page)
{
  uint nElem = Vars.getElem();
  if(!nElem)
    return;
  if(page == pageSource)
    if(IDYES != MessageBox(*owner, _T("La pagina da cui hai copiato è la stessa\r\nSicuro di voler procedere?"),
          _T("Attenzione"), MB_YESNO | MB_ICONSTOP))
      return;
  PVect<infoVariable> target;

  HWND hwList = GetDlgItem(*owner, IDC_LISTVIEW_VAR);
  int nElemList = ListView_GetItemCount(hwList);
  target.setDim(nElemList);
  for(int i = 0; i < nElemList; ++i)
    getRow_Var(hwList, target[i], i);

  PVect<infoVariable> toPaste;

  uint action = eActSkip;
  uint i = 0;
  for(; i < nElem; ++i) {
    if(existVar(target, Vars[i]) >= 0) {
      action = getAction(owner, Vars[i].id);
      if(eActSkipAll == action) {
        ++i;
        break;
        }
      if(eActCopyAll == action)
        break;
      if(eActSkip == action)
        continue;
      }
    toPaste[toPaste.getElem()] = Vars[i];
    }
  if(i < nElem) {
    if(eActSkipAll == action) {
      for(; i < nElem; ++i) {
        if(existVar(target, Vars[i]))
          continue;
        toPaste[toPaste.getElem()] = Vars[i];
        }
      }
    else if(eActCopyAll == action) {
      for(; i < nElem; ++i)
        toPaste[toPaste.getElem()] = Vars[i];
      }
    }
  nElem = toPaste.getElem();
  for(uint i = 0; i < nElem; ++i)
    pasteRow(target, toPaste[i]);
  ListView_DeleteAllItems(hwList);
  nElem = target.getElem();
  for(uint i = 0; i < nElem; ++i)
    addRow_Var(hwList, target[i]);
}
//-----------------------------------------------------------
