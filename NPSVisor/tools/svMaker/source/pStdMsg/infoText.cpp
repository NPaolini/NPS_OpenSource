//--------- infoText.cpp --------------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "infoText.h"
//-------------------------------------------------------------------
#include "dManageStdMsg.h"
//-------------------------------------------------------------------
//-------------------------------------------------------------------
infoText::infoText() : Id(0), Label(0), Text(0) {}
//-------------------------------------------------------------------
infoText::infoText(int id, LPCTSTR label, LPCTSTR text) :
    Id(id), Label(str_newdup(label)), Text(str_newdup(text)) {}
//-------------------------------------------------------------------
infoText::infoText(const infoText& other) : Label(0), Text(0) { clone(other); }
//-------------------------------------------------------------------
infoText::infoText(const infoLabel& info, const setOfString& set) :
    Id(info.Id), Label(str_newdup(info.Label)), Text(0)
{
  LPCTSTR p = set.getString(info.Id);
  if(p) {
    uint len = _tcslen(p);
    LPTSTR tmp = new TCHAR[len + 50];
    translateFromCRNL(tmp, p);
    Text = str_newdup(tmp);
    delete []tmp;
    }
  else
    Text = str_newdup(_T(""));
}
//-------------------------------------------------------------------
infoText::~infoText()
{
  delete []Label;
  delete []Text;
}
//-------------------------------------------------------------------
const infoText& infoText::operator=(const infoText& other)
{
  clone(other);
  return *this;
}
//-------------------------------------------------------------------
void infoText::clone(const infoText& other)
{
  if(this != &other) {
    Id = other.Id;
    delete []Label;
    Label = str_newdup(other.getLabel());
    delete []Text;
    Text = str_newdup(other.getText());
    }
}
//-------------------------------------------------------------------
//-------------------------------------------------------------------
static infoLabel iLabelL[] = {
  { 1, _T("Titolo Applicazione") },

  { 10, _T("Titolo risultato Finestra Password") },
  { 11, _T("Messaggio risultato Finestra Password") },
  { 12, _T("Titolo Finestra Password") },

//  { 14, _T("Nome del file di testo per header") },

  { 15, _T("Messaggio attesa trasmissione") },
  { 16, _T("Messaggio attesa ricezione") },
  { 17, _T("Titolo Finestra errore comunicazione") },
  { 18, _T("Messaggio errore trasmissione") },
  { 19, _T("Messaggio errore ricezione") },
  { 20, _T("Timeout errore trasmissione") },
  { 21, _T("Errore trasmissione") },

  { 26, _T("Titolo per file di trasmissione bloccato") },
  { 27, _T("Messaggio per file di trasmissione bloccato") },

  { 28, _T("Titolo richiesta di invio con psw livello 1000") },
  { 29, _T("Messaggio richiesta di invio con psw livello 1000") },

//  { 33, _T("Serie di nomi per oggetti Cam globali") },

  { 35, _T("Titolo periferica non pronta") },
  { 36, _T("Messaggio periferica non pronta") },

  { 40, _T("Messaggio per macchina in emergenza") },
  { 41, _T("Titolo per macchina in emergenza") },

  { 42, _T("Titolo richiesta Fine Programma") },
  { 43, _T("Messaggio richiesta Fine Programma") },
//  { 44, _T("Livello password Fine Programma (0, 1, 2, 3, 1000)") },

  { 50, _T("Messaggio per sovrascrittura file") },
  { 51, _T("Titolo per sovrascrittura file") },
  { 52, _T("Titolo per mancato caricamento periferiche") },
  { 53, _T("Richiesta per continuare comunque") },
  { 54, _T("Messaggio per successo salvataggio file") },
  { 55, _T("Messaggio per fallimento salvataggio file") },

  { 56, _T("Testo per listbox scelta 'export normale'") },
  { 57, _T("Testo per listbox scelta 'export binario'") },

//  { 58, _T("Immagine per toolBar Preview di stampa") },
  { 62, _T("Set Tempo Campionamento") },
  { 63, _T("mSec") },

  { 64, _T("Titolo (Print Trend)") },
  { 65, _T("Label (Pag.)") },

  { 70, _T("Titolo Finestra scelta valori") },
  { 71, _T("Label Da") },
  { 72, _T("Label A") },
  { 73, _T("Label valore max") },
  { 74, _T("Pulsante F1 per Ok") },
  { 75, _T("Pulsante F2 per Annulla") },

  { 76, _T("Pulsante F3 per Refresh") },
  { 77, _T("Titolo Finestra scelta Pagine") },

  { 80, _T("Titolo Data") },
  { 85, _T("Data Dal") },
  { 86, _T("Data Al") },

  { 87, _T("Titolo Time") },
  { 88, _T("Time Da") },
  { 89, _T("Time A") },

  { 100, _T("Nome mese (tre car.) Gennaio") },
  { 101, _T("Nome mese Febbraio") },
  { 102, _T("Nome mese Marzo") },
  { 103, _T("Nome mese Aprile") },
  { 104, _T("Nome mese Maggio") },
  { 105, _T("Nome mese Giugno") },
  { 106, _T("Nome mese Luglio") },
  { 107, _T("Nome mese Agosto") },
  { 108, _T("Nome mese Settembre") },
  { 109, _T("Nome mese Ottobre") },
  { 110, _T("Nome mese Novembre") },
  { 111, _T("Nome mese Dicembre") },

  { 119, _T("Titolo finestra degli allarmi - storico") },
  { 120, _T("Testo per indicatore allarme") },

  { 121, _T("Titolo finestra degli allarmi - correnti") },

  { 122, _T("Codice tipo eventi") },
  { 123, _T("Codice tipo allarme 1") },
  { 124, _T("Codice tipo allarme 2") },
  { 125, _T("Codice tipo allarme 3") },
  { 126, _T("Codice tipo allarme 4") },
  { 127, _T("Codice tipo allarme 5") },
  { 128, _T("Codice tipo allarme 6") },
  { 129, _T("Codice tipo allarme 7") },

  { 140, _T("Pulsante Esci Allarmi") },
  { 141, _T("Pulsante Reset Allarmi") },
  { 142, _T("Pulsante No Allarmi") },
  { 143, _T("Pulsante Solo Allarmi") },
  { 144, _T("Pulsante Full Allarmi") },
  { 145, _T("Pulsante Show Allarmi") },
  { 146, _T("Pulsante Show Report") },
  { 147, _T("CheckBox Advanced Report") },
  { 148, _T("Etichetta per filtro testuale") },

  { 151, _T("Testo per gestione refresh finestra allarmi") },
  { 152, _T("Titolo per gestione refresh finestra allarmi") },

  { 165, _T("Messaggio informazione errore dato") },
  { 166, _T("Tipo non valido") },
  { 167, _T("Indirizzo non valido") },
  { 168, _T("Periferica non valida") },
  { 170, _T("Errore sconosciuto") },

  { 171, _T("Nessun tipo") },
  { 172, _T("Campo di Bit") },
  { 173, _T("Byte") },
  { 174, _T("Word") },
  { 175, _T("DWord") },
  { 176, _T("Float") },
  { 177, _T("Intero a 64 bit") },
  { 178, _T("Double") },
  { 179, _T("Data e Ora") },
  { 180, _T("Data") },
  { 181, _T("Ora") },
  { 182, _T("Char") },
  { 183, _T("Short") },
  { 184, _T("Long") },
  { 185, _T("String") },

  { 190, _T("Nessuna Periferica") },
  { 191, _T("Memoria Interna") },
  { 192, _T("Periferica 2") },
  { 193, _T("Periferica 3") },
  { 194, _T("Periferica 4") },
  { 195, _T("Periferica 5") },
  { 196, _T("Periferica 6") },
  { 197, _T("Periferica 7") },
  { 198, _T("Periferica 8") },
  { 199, _T("Periferica 9") },

  { 209, _T("Testo per storico") },

  { 210, _T("Header Export Allarmi - Marcia") },
  { 211, _T("Header Export Allarmi - Arresto") },
  { 212, _T("Header Export Allarmi - Tipo1") },
  { 213, _T("Header Export Allarmi - Tipo2") },
  { 214, _T("Header Export Allarmi - Tipo3") },
  { 215, _T("Header Export Allarmi - Tipo4") },
  { 216, _T("Header Export Allarmi - Tipo5") },
  { 217, _T("Header Export Allarmi - Tipo6") },

  { 220, _T("Header Export Report - Evento") },
  { 221, _T("Header Export Report - Id Allarme") },
  { 222, _T("Header Export Report - Allarme") },
  { 223, _T("Header Export Report - Data") },
  { 224, _T("Header Export Report - Ora") },
  { 225, _T("Header Export Report - Inizio") },
  { 226, _T("Header Export Report - Fine") },
  { 227, _T("Header Export Report - Ack") },

  { 230, _T("Titolo Messaggio Export") },
  { 231, _T("Messaggio errore Export") },
  { 232, _T("Messaggio successo Export") },


  { 305, _T("sulla stampante %s") },
  { 306, _T("Attendere, preparazione pagine") },
  { 307, _T("Sto stampando la pagina n.%d di %d") },

  { 2058, _T("Titolo Oper. Info Tree") },
  { 2059, _T("Titolo Cause Info Tree") },
  { 2060, _T("Info Tree nome e time") },
  { 2061, _T("Info Tree Totali") },
  { 2062, _T("Info Tree titolo time") },
  { 2063, _T("Info Tree time") },

  { 5510, _T("Titolo Export Globale") },
  { 5511, _T("Titolo Export Report Allarmi") },
  { 5512, _T("Titolo Export Trend") },

  { 5990, _T("Titolo richiesta card") },

  { 6411, _T("Allarme es. alarm(addr,bit)") },
  { 6412, _T("Stato allarme (un solo carattere)") },
  { 6413, _T("Data evento") },
  { 6414, _T("Time evento") },
  { 6415, _T("Sezione (un solo carattere)") },
  { 6416, _T("Tipo (un solo carattere)") },
  { 6417, _T("descrizione Tipo") },
  { 6418, _T("Periferica (un solo carattere)") },
  { 6419, _T("Descrizione allarme") },

  { 6430, _T("Titolo per segnalazione dimensione file") },
  { 6431, _T("Messaggio per segnalazione dimensione file") },

  { 7000, _T("Dialogo opzioni scelta file") },
  { 7001, _T("Visualizza data") },
  { 7002, _T("Ordina per data") },
  { 7003, _T("Ordine discendente") },
  { 7004, _T("Usa anche storico") },

  { 7100, _T("Titolo 'Scelta configurazione'") },
  { 7101, _T("Personalizzata") },
  { 7102, _T("Memorizza risoluzione utente") },

  { 8000, _T("Titolo Filtro Report") },
  { 8001, _T("Gruppo Date Filtro Report") },
  { 8002, _T("Gruppo Allarmi Filtro Report") },
  { 8003, _T("Id Allarmi - Filtro Report") },
  { 8004, _T("Id Tipi - Filtro Report") },
  { 8005, _T("Id Sezioni - Filtro Report") },
  { 8006, _T("Id Periferiche - Filtro Report") },
  { 8007, _T("Ordinamento Ascendente - Filtro Report") },
  { 8008, _T("Totale Record - Filtro Report") },

  { 8011, _T("Limita record titolo - Filtro Report") },
  { 8012, _T("Limita rec. totali - Filtro Report") },
  { 8013, _T("Limita rec. inizio - Filtro Report") },
  { 8014, _T("Limita rec. numero - Filtro Report") },

  { 8020, _T("Titolo Scelta Tipo e sezione - Filtro Report") },
  { 8021, _T("Disponibili Scelta Tipo e sezione - Filtro Report") },
  { 8022, _T("Attivati Scelta Tipo e sezione - Filtro Report") },
  { 8023, _T("Descrizione Scelta Tipo e sezione - Filtro Report") },
  { 8024, _T("Codice Scelta Tipo e sezione - Filtro Report") },

  { 8031, _T("Head Allarmi - Periferica") },
  { 8032, _T("Head Allarmi - Tipo") },
  { 8033, _T("Head Allarmi - Codice(word,bit)") },
  { 8034, _T("Head Allarmi - Descr. tipo") },
  { 8035, _T("Head Allarmi - Data") },
  { 8036, _T("Head Allarmi - Ora") },
  { 8037, _T("Head Allarmi - Descr. allarme") },

  { 8041, _T("Head Report - Evento") },
  { 8042, _T("Head Report - Codice") },
  { 8043, _T("Head Report - Tipo") },
  { 8044, _T("Head Report - Descr. tipo") },
  { 8045, _T("Head Report - Data") },
  { 8046, _T("Head Report - Ora") },
  { 8047, _T("Head Report - Descr. allarme") },


};
//-------------------------------------------------------------------
#define FIXED_SYSTEM_FOLDER
//-------------------------------------------------------------------
static infoLabel iLabel[] = {
  { 14, _T("Nome del file di testo per header") },
  { 33, _T("Serie di nomi per oggetti Cam globali") },
  { 44, _T("Livello password Fine Programma (0, 1, 2, 3, 1000)") },
  { 58, _T("Immagine per toolBar Preview di stampa") },

  { 240, _T("Path Allarmi Globali") },
  { 241, _T("Path Ricette") },
  { 242, _T("Path Ordini") },
  { 243, _T("Path Operatori") },
  { 244, _T("Path Turni") },
#ifndef FIXED_SYSTEM_FOLDER
  { 245, _T("Path System") },
#endif
  { 246, _T("Path Storico") },
  { 247, _T("Path Bobine") },
  { 248, _T("Path Allarmi Report") },
  { 249, _T("Path Report") },
  { 250, _T("Path Export Report") },

  { 570, _T("Prph-mem10, serie di dword sempre in locale") },
  { 571, _T("Prph-mem11, serie di dword sempre in locale") },
  { 572, _T("Prph-mem12, serie di dword sempre in locale") },
  { 573, _T("Prph-mem13, serie di dword sempre in locale") },
  { 574, _T("Prph-mem14, serie di dword sempre in locale") },
  { 575, _T("Prph-mem15, serie di dword sempre in locale") },
  { 576, _T("Prph-mem16, serie di dword sempre in locale") },
  { 577, _T("Prph-mem17, serie di dword sempre in locale") },
  { 578, _T("Prph-mem18, serie di dword sempre in locale") },
  { 579, _T("Prph-mem19, serie di dword sempre in locale") },
  { 580, _T("Prph-mem20, serie di dword sempre in locale") },


};
//-------------------------------------------------------------------
void fillInfoText(setOfPInfoText& target, const setOfString& set)
{
  flushPAV(target);
  size_t sz = SIZE_A(iLabel);

  target.setDim(SIZE_A(iLabel) + SIZE_A(iLabelL));
  const infoLabel* il = iLabel;

  uint i = 0;
  for(; i < sz; ++i)
    target[i] = new infoText(il[i], set);

  sz = SIZE_A(iLabelL);
  il = iLabelL;

  for(uint j = 0; j < sz; ++i, ++j)
    target[i] = new infoText(il[j], set);
}
//-------------------------------------------------------------------
//----------------------------------------------------------------------------
#define PAGE_SCROLL_LEN  MAX_GROUP
#define MAX_V_SCROLL (maxShow - MAX_GROUP)
//#define MAX_V_SCROLL (MAX_ADDRESSES - MAX_GROUP)
//----------------------------------------------------------------------------
#define WM_CUSTOM_VSCROLL (WM_APP + 101)
//----------------------------------------------------------------------------
//#define IS_CHECKED(id) (BST_CHECKED == SendMessage(GetDlgItem(*this, (id)), BM_GETCHECK, 0, 0))
//----------------------------------------------------------------------------
#define MAX_TEXT 4096
//#define MAX_TEXT 200
#define MAX_LABEL 80
//----------------------------------------------------------------------------
#define OFFSET_LABEL   (IDC_STD_LABEL_1 - IDC_STATICTEXT_1)
#define OFFSET_DESCR  (IDC_EDIT_DESCR_1 - IDC_STATICTEXT_1)
bool PageEdit::evKeyDown(WPARAM& key)
{
  switch(key) {
    case VK_NEXT:
      PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_PAGERIGHT, 0);
      break;
    case VK_PRIOR:
      PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_PAGELEFT, 0);
      break;
    }
  return baseClass::evKeyDown(key);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool firstEdit::evKeyDown(WPARAM& key)
{
  switch(key) {
    case VK_UP:
      PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_LINELEFT, 0);
      return true;
    }
  return baseClass::evKeyDown(key);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool lastEdit::evKeyDown(WPARAM& key)
{
  switch(key) {
    case VK_DOWN:
    case VK_RETURN:
    PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_LINERIGHT, 0);
    return true;
    }
  return baseClass::evKeyDown(key);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool PageEditChk::evKeyDown(WPARAM& key)
{
  switch(key) {
    case VK_NEXT:
      PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_PAGERIGHT, 0);
      break;
    case VK_PRIOR:
      PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_PAGELEFT, 0);
      break;
    }
  return baseClass::evKeyDown(key);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool firstEditChk::evKeyDown(WPARAM& key)
{
  switch(key) {
    case VK_UP:
      PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_LINELEFT, 0);
      return true;
    }
  return baseClass::evKeyDown(key);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
bool lastEditChk::evKeyDown(WPARAM& key)
{
  switch(key) {
    case VK_DOWN:
    case VK_RETURN:
    PostMessage(*getParent(), WM_CUSTOM_VSCROLL, SB_LINERIGHT, 0);
    return true;
    }
  return baseClass::evKeyDown(key);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class PRow
{
  public:

    PRow(PWin* parent, uint first_id);
    virtual ~PRow() {}

    virtual void createDescr();

    void getLabel(LPTSTR buff) const;
    void setLabel(LPCTSTR buff) const;

    void getDescr(LPTSTR buff) const;
    void setDescr(LPCTSTR buff) const;
    DWORD getID();
    void setID(DWORD db);

  protected:
    PStatic* ID;
    PStatic* Label;
    PEdit* Descr;
    uint firstId;

};
//----------------------------------------------------------------------------
class PFirstRow : public PRow
{
  public:

    PFirstRow(PWin* parent, uint first_id) : PRow(parent, first_id) {}
    virtual void createDescr();
};
//----------------------------------------------------------------------------
class PLastRow : public PRow
{
  public:

    PLastRow(PWin* parent, uint first_id) : PRow(parent, first_id) {}
    virtual void createDescr();
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
PRow::PRow(PWin* parent, uint first_id) :
    firstId(first_id)
{
  ID = new PStatic(parent, firstId);
  Label = new PStatic(parent, firstId + OFFSET_LABEL);
//  Descr = new PageEditChk(parent, firstId + OFFSET_DESCR, firstId, MAX_TEXT - 1);
}
//----------------------------------------------------------------------------
void PRow::createDescr()
{
  PWin* parent = ID->getParent();
  Descr = new PageEditChk(parent, firstId + OFFSET_DESCR, firstId, MAX_TEXT - 1);
}
//----------------------------------------------------------------------------
void PFirstRow::createDescr()
{
  PWin* parent = ID->getParent();
  Descr = new firstEditChk(parent, firstId + OFFSET_DESCR, firstId, MAX_TEXT - 1);
}
//----------------------------------------------------------------------------
void PLastRow::createDescr()
{
  PWin* parent = ID->getParent();
  Descr = new lastEditChk(parent, firstId + OFFSET_DESCR, firstId, MAX_TEXT - 1);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
inline
void PRow::getLabel(LPTSTR buff) const
{
  GetWindowText(*Label, buff, MAX_LABEL - 1);
}
//----------------------------------------------------------------------------
inline
void PRow::setLabel(LPCTSTR buff) const
{
  SetWindowText(*Label, buff);
}
//----------------------------------------------------------------------------
inline
void PRow::getDescr(LPTSTR buff) const
{
  GetWindowText(*Descr, buff, MAX_TEXT - 1);
}
//----------------------------------------------------------------------------
inline
void PRow::setDescr(LPCTSTR buff) const
{
  SetWindowText(*Descr, buff);
}
//----------------------------------------------------------------------------
inline
DWORD PRow::getID()
{
  TCHAR buff[50];
  GetWindowText(*ID, buff, SIZE_A(buff));
  return _ttol(buff);
}
//----------------------------------------------------------------------------
inline
void PRow::setID(DWORD val)
{
  TCHAR buff[50];
  wsprintf(buff, _T("%d"), val);
  SetWindowText(*ID, buff);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define MAX_GROUP 16
//----------------------------------------------------------------------------
struct baseRow
{
  DWORD id;
  TCHAR label[MAX_LABEL];
  TCHAR text[MAX_TEXT];
};
//-------------------------------------------------------------------
//#define maxShow SIZE_A(iLabel)
#define maxShow (SIZE_A(iLabelL) + SIZE_A(iLabel))
//-------------------------------------------------------------------
class dManageStdText : public basePage
{
  private:
    typedef basePage baseClass;
  public:
    dManageStdText(const setOfString& set, PWin* parent, uint resId, HINSTANCE hInst = 0) :
        baseClass(set, parent, resId, hInst), currPos(0)
        {}
    ~dManageStdText();

    virtual bool create();
    virtual bool save(P_File& pfCrypt, P_File& pfClear);
    uint needSaveBeforeClose();
  protected:
    bool preProcessMsg(MSG& msg);
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);
    void evMouseWheel(short delta, short x, short y);

  private:
    int prfNum;
    DWORD WM_PRF;
    int dataSize;

    PVect<baseRow> Saved;
    class PRow* Rows[MAX_GROUP];
    int currPos;

    void saveCurrData();
    bool saveData();
    void loadData();

    void evVScrollBar(HWND child, int flags, int pos);
};
//-------------------------------------------------------------------
basePage* allocStdText( const setOfString& Set, PWin* parent, HINSTANCE hInst)
{
  return new dManageStdText(Set, parent, IDD_STD_MSG_TEXT, hInst);
}
//-------------------------------------------------------------------
//-------------------------------------------------------------------
dManageStdText::~dManageStdText()
{
  for(int i = 0; i < MAX_GROUP; ++i)
    delete Rows[i];
  destroy();
}
//-------------------------------------------------------------------
bool dManageStdText::create()
{
  Rows[0] = new PFirstRow(this, IDC_STATICTEXT_1);
  int i;
  for(i = 1; i < MAX_GROUP - 1; ++i)
    Rows[i] = new PRow(this, IDC_STATICTEXT_1 + i);

  Rows[i] = new PLastRow(this, IDC_STATICTEXT_1 + i);

  for(int i = 0; i < MAX_GROUP; ++i)
    Rows[i]->createDescr();

  if(!baseClass::create())
    return false;

  setOfPInfoText target;
  fillInfoText(target, Set);
  int nElem = target.getElem();

  for(int i = 0; i < nElem; ++i) {
    Saved[i].id = target[i]->getId();
    _tcscpy_s(Saved[i].label, target[i]->getLabel());
    _tcscpy_s(Saved[i].text, target[i]->getText());
    }
  flushPV(target);
  loadData();
  Dirty = 0;
  HWND hsb = GetDlgItem(*this, IDC_SCROLLBAR_COUNT);
  if(hsb) {
    SCROLLINFO si;
    memset(&si, 0, sizeof(si));
    si.cbSize = sizeof(si);
    si.fMask = SIF_PAGE | SIF_RANGE;
    si.nPage = PAGE_SCROLL_LEN;
    si.nMin = 0;
    si.nMax = maxShow - 1;
    SendMessage(hsb, SBM_SETSCROLLINFO, 1, (LPARAM)&si);
    }

  return true;
}
//-------------------------------------------------------------------
//----------------------------------------------------------------------------
bool dManageStdText::preProcessMsg(MSG& msg)
{
  switch(msg.message) {
    case WM_KEYDOWN:
      switch(msg.wParam) {
        case VK_NEXT:
          PostMessage(*this, WM_CUSTOM_VSCROLL, SB_PAGERIGHT, 0);
          break;
        case VK_PRIOR:
          PostMessage(*this, WM_CUSTOM_VSCROLL, SB_PAGELEFT, 0);
          break;
        case VK_HOME:
          PostMessage(*this, WM_CUSTOM_VSCROLL, SB_LEFT, 0);
          break;
        case VK_END:
          PostMessage(*this, WM_CUSTOM_VSCROLL, SB_RIGHT, 0);
          break;
        }
      break;
    }
  if(!toBool(IsDialogMessage(getHandle(), &msg)))
    return baseClass::preProcessMsg(msg);
  return true;
}
//----------------------------------------------------------------------------
#define WM_MOUSEWHEEL                   0x020A
#define WHEEL_DELTA                     120
//-----------------------------------------------------------------------------
void dManageStdText::evMouseWheel(short delta, short /*x*/, short /*y*/)
{
  int tD = delta;
  tD *= PAGE_SCROLL_LEN;
  tD /= WHEEL_DELTA;
  uint msg = WM_VSCROLL;
  HWND child = GetDlgItem(*this, IDC_SCROLLBAR_COUNT);

  if(IsWindowEnabled(child)) {
    int curr = SendMessage(child, SBM_GETPOS, 0, 0);
    curr -= tD;
    if(curr < 0)
      curr = 0;
    else if(curr > (int)MAX_V_SCROLL)
      curr = MAX_V_SCROLL;
    SendMessage(*this, msg, MAKEWPARAM(SB_THUMBTRACK, curr),(LPARAM)child);
    }
}
//----------------------------------------------------------------------------
LRESULT dManageStdText::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_CUSTOM_VSCROLL:
      lParam = (LPARAM)GetDlgItem(*this, IDC_SCROLLBAR_COUNT);
      // passa sotto
    case WM_VSCROLL:
      evVScrollBar((HWND)lParam, LOWORD(wParam), HIWORD(wParam));
      break;
    case WM_MOUSEWHEEL:
      evMouseWheel(HIWORD(wParam), LOWORD(lParam), HIWORD(lParam));
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//------------------------------------------------------------------------------
void dManageStdText::evVScrollBar(HWND child, int flags, int pos)
{
  int curr = SendMessage(child, SBM_GETPOS, 0, 0);
  switch(flags) {
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
      break;
    case SB_ENDSCROLL:
      pos = curr;
      break;
    case SB_LEFT:
      pos = 0;
      break;
    case SB_RIGHT:
      pos = MAX_V_SCROLL;
      break;
    case SB_LINELEFT:
      pos = curr - 1;
      break;
    case SB_LINERIGHT:
      pos = curr + 1;
      break;
    case SB_PAGELEFT:
      pos = curr - PAGE_SCROLL_LEN;
      break;
    case SB_PAGERIGHT:
      pos = curr + PAGE_SCROLL_LEN;
      break;
    }
  if(pos < 0)
    pos = 0;
  else if((uint)pos > MAX_V_SCROLL)
    pos = MAX_V_SCROLL;
  SendMessage(child, SBM_SETPOS, pos, true);

  saveCurrData();
  currPos = pos;
  loadData();
}
//------------------------------------------------------------------------------
void dManageStdText::saveCurrData()
{
  int pos = currPos;
  smartPointerString buff(new TCHAR[MAX_TEXT + 1], true);
  for(int i = 0; i < MAX_GROUP; ++i, ++pos) {
    Saved[pos].id = Rows[i]->getID();
    Rows[i]->getLabel(Saved[pos].label);
    Rows[i]->getDescr(buff);
    Dirty |= _tcscmp(buff, Saved[pos].text);
    _tcscpy_s(Saved[pos].text, buff);
    }
}
//------------------------------------------------------------------------------
void dManageStdText::loadData()
{
  int pos = currPos;

  for(int i = 0; i < MAX_GROUP; ++i, ++pos) {

    Rows[i]->setID(Saved[pos].id);
    Rows[i]->setLabel(Saved[pos].label);
    Rows[i]->setDescr(Saved[pos].text);
    }
}
//----------------------------------------------------------------------------
HBRUSH dManageStdText::evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType)
{
  if(WM_CTLCOLORSTATIC == ctlType) {
    int id = ::GetDlgCtrlID(hWndChild);
    for(int i = 0; i < MAX_GROUP; ++i) {
      if(id == IDC_STATICTEXT_1 + i) {
        SetBkColor(hdc, bkgColor2);
        return (Brush2);
        }
      if(id == IDC_STD_LABEL_1 + i) {
        SetBkColor(hdc, bkgColor3);
        return (Brush3);
        }
      }
    return 0;
    }
  if(WM_CTLCOLOREDIT == ctlType) {
    SetTextColor(hdc, RGB(0, 0, 0));
    SetBkColor(hdc, bkgColor);
    return (Brush);
    }
  return 0;
}
//-------------------------------------------------------------------
uint dManageStdText::needSaveBeforeClose()
{
  saveCurrData();
  if(Dirty)
    return reqSave();
  return 0;
}
//-------------------------------------------------------------------
bool dManageStdText::save(P_File& pfCrypt, P_File& pfClear)
{
  saveCurrData();
  P_File* pf[] = { &pfCrypt, &pfClear };
  int nEl[] = { SIZE_A(iLabel), Saved.getElem() };
  int i = 0;
#if 1
  for(; i < nEl[0]; ++i) {
    if(*Saved[i].text) {
      TCHAR buff[20];
      wsprintf(buff, _T("%d,"), Saved[i].id);
      if(!writeStringChkUnicode(pfCrypt, buff))
        return false;
      if(!writeStringChkUnicode(pfCrypt, Saved[i].text))
        return false;
      if(!writeStringChkUnicode(pfCrypt, _T("\r\n")))
        return false;
      }
    }
  for(; i < nEl[1]; ++i) {
    if(*Saved[i].text) {
      int j;
      if(isGlobalPageString(Saved[i].text))
        j = 0;
      else
        j = 1;
      TCHAR buff[20];
      wsprintf(buff, _T("%d,"), Saved[i].id);
      if(!writeStringChkUnicode(*pf[j], buff))
        return false;
      if(!writeStringChkUnicode(*pf[j], Saved[i].text))
        return false;
      if(!writeStringChkUnicode(*pf[j], _T("\r\n")))
        return false;
      }
    }
#else
  for(uint j = 0; j < SIZE_A(nEl); ++j) {
    for(; i < nEl[j]; ++i) {
      if(*Saved[i].text) {
        TCHAR buff[20];
        wsprintf(buff, _T("%d,"), Saved[i].id);
        if(!writeStringChkUnicode(*pf[j], buff))
          return false;
        if(!writeStringChkUnicode(*pf[j], Saved[i].text))
          return false;
        if(!writeStringChkUnicode(*pf[j], _T("\r\n")))
          return false;
        }
      }
    }
#endif
#ifdef FIXED_SYSTEM_FOLDER
// visto che tutti i programmi esterni fanno riferimento a questa
// directory, non va più modificata
  return toBool(writeStringChkUnicode(pfCrypt, _T("245,system\r\n")));
#else
  return true;
#endif
}
//----------------------------------------------------------------------------
