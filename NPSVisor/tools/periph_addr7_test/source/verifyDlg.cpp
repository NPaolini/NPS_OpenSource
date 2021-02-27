//----------- verifyDlg.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
//----------------------------------------------------------------------------
#include "verifyDlg.h"
#include "pEdit.h"
#include "p_util.h"
#include "p_txt.h"
#include "p_file.h"
#include "POwnBtnImageStd.h"
#include "PListBox.h"
#include "mainDlg.h"
//----------------------------------------------------------------------------
class prfData
{
  public:
    enum tData {  tNoData,  // nessun tipo definito -  0
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
                             // ed il puntatore (32 + 32 bit)

                  tNegative = 1 << 30,
//                  tArray = 1 << 31,
               };

    enum tResultData {  invalidType,
                        invalidAddress,
                        invalidPerif,

                        // per verificare velocemente se si è in errore

                        // da failed (compreso) andando indietro ci sono gli errori
                        failed = invalidPerif,

                        // da okData (compreso) in poi sono tutti validi,
                        okData,

                        notModified = okData,
                        isChanged
                     };

#if 0
      // buffer per appoggio dato da caricare

      // se campo di bit l'indirizzo è comunque relativo
      // all'indirizzamento, il numero di bit e l'offset richiesti vanno
      // inseriti in u.dw come MAKELONG(nBit, offset). L'offset è
      // inteso come numero di bit da shiftare a dx prima di prendere i bit
      // che servono, es. se i bit sono tre e l'offset 2
      // (per 16 bit f edc ba9 876 543 210 -> - -fe dcb a98 765 432 -> xx

      // in caso di invio bit occorre usare MAKELONG(MAKEWORD(nBit, offset), val)
      // se il valore da inviare supera la capacità di 16 bit probabilmente avrebbe
      // più senso usarlo come word a parte
    union uData {
        BDATA b;
        WDATA w;
        DWDATA dw;

        BsDATA sb;
        WsDATA sw;
        DWsDATA sdw;

        fREALDATA fw;
        REALDATA rw;
        LARGE_INTEGER li;
        FILETIME ft; // usato sia per data/ora/data_ora
        struct strData {
          DWDATA len; // len non contempla il terminatore stringa che viene
          LPBYTE buff;// comunque allocato, in pratica len(buff) = len + 1
          } str;
        } U;

      // indirizzo logico di lettura, nel buffer
    uint lAddr;
      // indirizzo fisico di lettura, dipendente dalla periferica
    uint pAddr;

      // tipo di dato da leggere
    // tData
    DWDATA typeVar;

    prfData() : lAddr(0), pAddr(0), typeVar(tNoData) { U.li.QuadPart = 0; }
    prfData(uData u, uint p_addr, tData type, uint l_addr);

    prfData(uint lenBuff, LPBYTE Buff = 0);
    prfData(LPBYTE buff);

    void setStr(uint len, LPBYTE newBuff = 0);

    ~prfData();

    prfData(const prfData& other);
    const prfData& operator =(const prfData& other);
#endif
    // nel tipo bit torna zero. Nel caso (bit) la dimensione del tipo
    // viene impostata dalla periferica e quindi non si può stabilire a priori.
    // Nel caso (string) torna uno.
    static int getNBit(uint type);
    static int getNByte(uint type);

      // se lo stesso errore si è già verificato torna false e non visualizza nulla
//    bool showMessageError(class PWin* win, uint prf, tResultData result) const;
  private:
/*
    //usati come campi di bit
    static struct tagErrBuff {
      WORD Type;
      WORD Addr;
      WORD Perif;
      tagErrBuff() : Type(0), Addr(0), Perif(0) { }

      // la periferica viene considerata uno based.
      // Secondo le specifiche, la periferica uno identifica la memoria,
      // ma viene comunque considerata
      bool hasErr(uint perif, tResultData type);
      void setErr(uint perif, tResultData type);
      } errBuff;
*/
};
//----------------------------------------------------------------------
/**/
int prfData::getNBit(uint type)
{
  switch(type) {
    case tBData:
    case tBsData:
    case tStrData:
      return 8;
    case tWData:
    case tWsData:
      return 16;
    case tDWData:
    case tDWsData:
    case tFRData:
      return 32;
    case ti64Data:
    case tRData:
    case tDateHour:
    case tDate:
    case tHour:
      return 64;
    case tBitData:
    case tNoData:
      return 0;
    }
  return 0;
}
//----------------------------------------------------------------------
int prfData::getNByte(uint type)
{
  switch(type) {
    case tBData:
    case tBsData:
    case tStrData:
      return 1;
    case tWData:
    case tWsData:
      return 2;
    case tDWData:
    case tDWsData:
    case tFRData:
      return 4;
    case ti64Data:
    case tRData:
    case tDateHour:
    case tDate:
    case tHour:
      return 8;
    case tBitData:
    case tNoData:
      return 0;
    }
  return 0;
}
//----------------------------------------------------------------------------
int orderedPAddr::Cmp(const TreeData toCheck, const TreeData current) const
{
  addrConv* check = reinterpret_cast<addrConv*>(toCheck);
  addrConv* curr = reinterpret_cast<addrConv*>(current);
  if(*curr > *check)
    return -1;
  if(*check > *curr)
    return 1;
  return 0;
}

//----------------------------------------------------------------------------
PD_Verify::PD_Verify(orderedPAddr& set, int type_sz, int type_show, bool step1, PWin* parent, uint resId, HINSTANCE hinstance) :
    baseClass(0, resId, hinstance),  Set(set), TypeSz(type_sz), typeShow(type_show), Step1(step1), Par(parent)
{
  int idBmp[] = { IDB_DONE };
  int idBtn[] = { IDCANCEL };

  HINSTANCE hi = getHInstance();
  for(uint i = 0; i < SIZE_A(idBtn); ++i) {
    PBitmap* b = new PBitmap(idBmp[i], hi);
    POwnBtnImageStd* btn = new POwnBtnImageStd(this, idBtn[i], b, POwnBtnImageStd::wLeft, true);
    POwnBtn::colorRect cr(GetSysColor(COLOR_BTNFACE), RGB(0, 0, 0xff), 1, GetSysColor(COLOR_BTNTEXT));
    btn->setColorRect(cr);

    }
}
//----------------------------------------------------------------------------
PD_Verify::~PD_Verify()
{
  destroy();
}
//----------------------------------------------------------------------------
/*
static void fillCB(HWND hcb, LPCTSTR rows[])
{
  SendMessage(hcb, CB_RESETCONTENT, 0, 0);
  int i = 0;
  while(rows[i]) {
    addString(hcb, rows[i]);
//    SendMessage(hcb, CB_ADDSTRING, 0, (LPARAM)rows[i]);
    ++i;
    }
  SendMessage(hcb, CB_SETCURSEL, select, 0);
}
*/
//----------------------------------------------------------------------------
#define DIM_IP (4 * 4)
#define DIM_PORT 5
#define DIM_DB 5
#define DIM_ADDR 6
#define DIM_TYPE 12
#define DIM_ACTION 2
#define DIM_DESCR 40
//----------------------------------------------------------------------------
#define MAX_LEN_LB (DIM_IP + DIM_PORT + DIM_DB + DIM_ADDR * 2 + DIM_TYPE + DIM_ACTION + DIM_DESCR + 10)
//----------------------------------------------------------------------------
bool PD_Verify::create()
{
  PListBox* lb = new PListBox(this, IDC_LISTBOX_VERIFY);
  PListBox* lb2 = new PListBox(this, IDC_LISTBOX_VERIFY2);
  int tabs[] = {
    DIM_ADDR,
    DIM_IP,
    DIM_PORT,
    DIM_DB,
    DIM_ADDR,
    DIM_TYPE,
    DIM_ACTION,
//    DIM_DESCR
    };

  lb->SetTabStop(SIZE_A(tabs), tabs, 0);
  lb2->SetTabStop(SIZE_A(tabs), tabs, 0);
  for(uint i = 0; i < SIZE_A(tabs); ++i) {
    lb->setAlign(i, PListBox::aCenter);
    lb2->setAlign(i, PListBox::aCenter);
    }
  lb2->SetColor(RGB(0,0,127), RGB(255,255,200));
  lb2->SetColorSel(RGB(127,0,0), RGB(255,255,100));

  if(!baseClass::create())
    return false;

  do {
    PRect r;
    GetClientRect(*this, r);
    PRect r2;
    GetWindowRect(GetDlgItem(*this, IDCANCEL), r2);
    MapWindowPoints(0, *this, (LPPOINT)(LPRECT)r2, 2);
    int diff = r.bottom - r2.bottom - 6;
    GetWindowRect(*this, r);
    fullRect = r;
    r.bottom -= diff;
    SetWindowPos(*this, 0, r.left, r.top, r.Width(), r.Height(), SWP_NOMOVE | SWP_NOZORDER);
    } while(false);

  load();
  lb->setIntegralHeight();
  lb2->setIntegralHeight();
  return true;
}
/*
#define DWORD_TO_IP(b1, b2, b3, b4, dw) \
    { (b1) = (dw) >> 24; \
       (b2) = ((dw) >> 16) & 0xff; \
       (b3) = ((dw) >> 8) & 0xff;  \
       (b4) = (dw) & 0xff; }

*/
//----------------------------------------------------------------------------
void ipToText(LPTSTR target, uint ip)
{
  int b1;
  int b2;
  int b3;
  int b4;
  DWORD_TO_IP(b1, b2, b3, b4, ip);
  wsprintf(target, _T("%d.%d.%d.%d"), b1, b2, b3, b4);
}
//----------------------------------------------------------------------------
void typeToText(LPTSTR target, uint id)
{
  static LPCTSTR t[] = {
  _T("0 - null"),
  _T("1 - bit"),
  _T("2 - byte"),
  _T("3 - word"),
  _T("4 - dword"),
  _T("5 - float"),
  _T("6 - int64"),
  _T("7 - real"),
  _T("8 - DateHour"),
  _T("9 - Date"),
  _T("10 - Hour"),
  _T("11 - char"),
  _T("12 - short"),
  _T("13 - long"),
  _T("14 - string"),
  };

  if(id >= SIZE_A(t))
    id = 0;
  _tcscpy(target, t[id]);
}
//----------------------------------------------------------------------------
int PD_Verify::addRow(HWND  hwnd, addrConv* ac)
{
  TCHAR buff[MAX_LEN_LB + 1];
  fillStr(buff, _T(' '), MAX_LEN_LB);

  TCHAR tmp[100];
  LPTSTR p = buff;

  if(ac)
    wsprintf(tmp, _T("%d"), ac->lAddr);
  else
    _tcscpy(tmp, _T("--"));
  copyStr(p, tmp, _tcslen(tmp));
  p += DIM_ADDR;
  *p++ = _T('\t');

  if(ac)
    ipToText(tmp, ac->ipAddr);
  else
    _tcscpy(tmp, _T("--"));
  copyStr(p, tmp, _tcslen(tmp));
  p += DIM_IP;
  *p++ = _T('\t');

  if(ac)
    wsprintf(tmp, _T("%d"), ac->port);
  else
    _tcscpy(tmp, _T("--"));
  copyStr(p, tmp, _tcslen(tmp));
  p += DIM_PORT;
  *p++ = _T('\t');

  if(ac)
    wsprintf(tmp, _T("%d"), ac->db);
  else
    _tcscpy(tmp, _T("--"));
  copyStr(p, tmp, _tcslen(tmp));
  p += DIM_DB;
  *p++ = _T('\t');

  if(ac)
    wsprintf(tmp, _T("%d"), ac->pAddr / typeShow);
  else
    _tcscpy(tmp, _T("--"));
  copyStr(p, tmp, _tcslen(tmp));
  p += DIM_ADDR;
  *p++ = _T('\t');

  if(ac)
    typeToText(tmp, ac->type);
  else
    _tcscpy(tmp, _T("--"));
  copyStr(p, tmp, _tcslen(tmp));
  p += DIM_TYPE;
  *p++ = _T('\t');

  int res = 0;
  if(ac) {
    wsprintf(tmp, _T("%d"), ac->action);
    if(!ac->action)
      res = 1;
    }
  else
    _tcscpy(tmp, _T("--"));
  copyStr(p, tmp, _tcslen(tmp));
  p += DIM_ACTION;
  *p++ = _T('\t');

/*
  wsprintf(tmp, _T("%d"), ac->lAddr);
  copyStr(p, ac->descr, _tcslen(ac->descr));
  p += DIM_DESCR;
  *p++ = _T('\t');
*/
  *p = 0;
  ::SendMessage(hwnd, LB_ADDSTRING, 0, (LPARAM)buff);
  if(ac) {
    uint nElem = Db.getElem();
    for(uint i = 0; i < nElem; ++i) {
      if(ac->db == Db[i])
        return res;
      }
    Db[nElem] = ac->db;
    }
  return res;

}
//-----------------------------------------------------------------------------
bool isSimilar(uint type1, uint type2)
{
  // se uno è stringa e l'altro è bit è ok
  if(prfData::tStrData == type1 && prfData::tBitData == type2)
    return true;
  if(prfData::tStrData == type2 && prfData::tBitData == type1)
    return true;
  // impostiamo che che non sono uguali se hanno tipi diversi, se si impostano i min-max poi ci sono problemi
  if(type1 != type2)
    return false;
/*
  // se non hanno la stessa dimensione non è ok
  if(prfData::getNByte(type1) != prfData::getNByte(type2))
    return false;
*/
  // altrimenti sono compatibili
  return true;
}
//----------------------------------------------------------------------------
void PD_Verify::load()
{
  int nRow = 0;
  int nRowWithoutAction = 0;
  int nBreak = 0;
  bool ended = !Set.setFirst();

  addrConv* old;
  HWND hwnd = GetDlgItem(*this, IDC_LISTBOX_VERIFY);
  HWND hwnd2 = GetDlgItem(*this, IDC_LISTBOX_VERIFY2);
  DWORD nConfl = 0;
  DWORD lastLAddr = 0;
  while(!ended) {
    addrConv* p = reinterpret_cast<addrConv*>(Set.getCurr());
    if(lastLAddr < p->lAddr)
      lastLAddr = p->lAddr;
    int typeSz = prfData::getNByte(p->type);
    if(prfData::tStrData == p->type || prfData::tBitData == p->type)
      typeSz = getTypeSize();

    old = p;
    nRowWithoutAction += addRow(hwnd, p);
    ++nRow;
    if(!Set.setNext())
      break;

    int step = Step1 ? 1 : typeSz;
    do {
      ended = false;
      p = reinterpret_cast<addrConv*>(Set.getCurr());
      if(lastLAddr < p->lAddr)
        lastLAddr = p->lAddr;
      // se il successivo non è dello stesso tipo
#if 1
      if(!isSimilar(p->type, old->type))
        break;

#else
      if(p->type != old->type)
        break;
#endif
      // se non usa lo stesso ipAddr
      if(p->ipAddr != old->ipAddr)
        break;
      // se non usa la stessa porta
      if(p->port != old->port)
        break;
      // se non usa lo stesso db
      if(p->db != old->db)
        break;
      // se l'indirizzo non è immediatamente successivo
      if(p->pAddr > old->pAddr + step)
        break;

      // @@ Si è verificato un errore nell'immissione in un file .adr
      //    in cui venivano ripetuti alcuni indirizzi. Per evitare di
      //    andare oltre il buffer reale si verifica che l'indirizzo sia
      //    effettivamente successivo, altrimenti lo si ignora.

      // se l'indirizzo è immediatamente successivo.
      if(p->pAddr != old->pAddr + step) {
        nRowWithoutAction += addRow(hwnd2, p);
        ++nConfl;
        break;
        }
      nRowWithoutAction += addRow(hwnd, p);
      step += Step1 ? 1 : typeSz;

      ++nRow;
      ended = true;

      } while(Set.setNext());
    if(!ended) {
      ++nBreak;
      nRowWithoutAction += addRow(hwnd, 0);
      }
    }
  TCHAR buff[500];
  if(nConfl) {
    SetWindowPos(*this, 0, 0, 0, fullRect.Width(), fullRect.Height(), SWP_NOMOVE | SWP_NOZORDER);
    wsprintf(buff, _T("N° righe = %d, N° blocchi = %d, N° conflitti = %d, N° senza azione = %d, N° DB = %d, Ultimo lAddr = %d"), nRow, nBreak + 1, nConfl, nRowWithoutAction, Db.getElem(), lastLAddr);
    }
  else {
    wsprintf(buff, _T("N° righe = %d, N° blocchi = %d, N° senza azione = %d, N° DB = %d, Ultimo lAddr = %d"), nRow, nBreak + 1, nRowWithoutAction, Db.getElem(), lastLAddr);
    EnableWindow(hwnd2, false);
    }
  SetDlgItemText(*this, IDC_STATIC_INFO_VERIFY, buff);
}
//----------------------------------------------------------------------------
LRESULT PD_Verify::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_DESTROY:
      PostMessage(*Par, WM_ENABLE_VERIFY, 0, 0);
      break;

    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
