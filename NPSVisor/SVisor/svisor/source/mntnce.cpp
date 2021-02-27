//--------- mntnce.cpp -------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <process.h>

#include "1.h"
#include "pcrt_lck.h"
#include "mntnce.h"
#include "file_lck.h"
#include "p_date.h"
#include "p_name.h"
#include "p_heap.h"

#include "svDialog.h"
#include "PListbox.h"
#include "P_Body.h"
#include "p_Util.h"

IMaint *allocMaint() { return new gest_maint; }
//#define TRACE
#ifdef TRACE
  #define __MSG__ \
    { TCHAR buff[255]; \
      wsprintf(buff, _T("linea %d"), __LINE__);\
      ::MessageBox(0,_T("Thread"), buff, MB_OK); \
      }
#else
  #define __MSG__ ;
#endif
//---------------------------------------------------------------
//#define TIME_FOR_SAVE (1000L * 20)
#define TIME_FOR_SAVE (1000L * 60)
//#define TIME_FOR_SAVE (1000L * 60 * 5)
//-------------------------------------------------------------------------
#define Q_TO_M(a) (reinterpret_cast<g_mem_maint*>(a))
#define M_TO_Q(a) (reinterpret_cast<CntData>(a))
//-------------------------------------------------------------------------
class setOfMaint : public P_Heap_p<g_mem_maint*>
{
  public:
    setOfMaint() : Curr(0) { }
    ~setOfMaint() { flush(); }
    const g_mem_maint* getMaint() const { return Vect[Curr]; }
    bool setFirst();
    bool setNext();
    void flush();
  private:
//    g_mem_maint* &get() { return P_Heap_p<g_mem_maint*>::Vect[0]; }
    uint Curr;
};
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
class P_ListMaint : public svDialog
{
  public:
    P_ListMaint(DWORD idThreadTo, setOfMaint &l, PWin* parent, int id = IDD_LIST_MAINT,
          HINSTANCE hinstance = 0);
    virtual ~P_ListMaint();
    virtual bool create();

  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    PListBox *LB;
    setOfMaint &L;
    DWORD mainThreadId;
  private:
    typedef svDialog baseClass;
};
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
__int64 g_mem_maint::remain() const
{
  __int64 time = cMK_I64(maint.ft);
  __int64 hours = maint.hours;
  hours *= HOUR_TO_I64;
  hours -= time;
  return hours;
}
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
void setOfMaint::flush()
{
  while(getNumElem())
    delete Pop();
}
//-------------------------------------------------------------------------
bool setOfMaint::setFirst()
{
  Curr = 0;
  return getNumElem() > 0;
}
//-------------------------------------------------------------------------
bool setOfMaint::setNext()
{
  ++Curr;
  return Curr < getNumElem();
}
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
gest_maint::gest_maint() :
    hEvent(0), Dirty(false),
    idThread(0), goingOut(true), mainThreadId(GetCurrentThreadId())
{
  init();
}
//-------------------------------------------------------------------------
gest_maint::~gest_maint()
{
  stop();
}
//-------------------------------------------------------------------------
void gest_maint::init()
{
#if 1
  P_File fl(FILENAME_MAINT);
  if(fl.P_open()) {
    if(fl.get_len() < sizeof(g_maint) * MAX_MAINT) {
      // se il file non esisteva lo crea vuoto e torna senza nessuna azione
      g_maint fgm[MAX_MAINT];
      ZeroMemory(fgm, sizeof(g_maint) * MAX_MAINT);
      fl.P_write(fgm, sizeof(g_maint) * MAX_MAINT);
      }
    }
#else
  P_File_Lock fl(FILENAME_MAINT);
  if(fl.isLocked()) {
    if(fl.P_get_len() < sizeof(g_maint) * MAX_MAINT) {
      // se il file non esisteva lo crea vuoto e torna senza nessuna azione
      g_maint fgm[MAX_MAINT];
      ZeroMemory(fgm, sizeof(g_maint) * MAX_MAINT);
      fl.P_write(fgm, sizeof(g_maint) * MAX_MAINT);
      }
    }
#endif
}
//-------------------------------------------------------------------------
void gest_maint::resetThread()
{
// non serve una sezione critica, la chiamata avviene solo
// nel thread e già in una sezione critica
  criticalLock lck(CS);
  idThread = 0;
  hEvent = 0;
  goingOut = true;
}
//-------------------------------------------------------------------------
bool gest_maint::isRunning()
{
  if(idThread) {
    while(true) {
      do {
        criticalLock lck(CS);
        // se è stato creato l'evento è in run
        if(hEvent)
          return true;
        // altrimenti, se ha raggiunto la fine del thread sta terminando
        else if(goingOut)
          return false;
        } while(false);
      // altrimenti aspetta che si verifichi uno dei due eventi
      Sleep(10);
      }
    }
  return false;
}
//-------------------------------------------------------------------------
#define MINUTE_OF_TOL 1
//#define MINUTE_OF_TOL (TIME_FOR_SAVE / (1000L * 60))
#define TIME_OF_TOLERANCE (MINUTE_OF_TOL * MINUTE_TO_I64)
static void set_tolerance(g_mem_maint &m)
{
  __int64 t = MK_I64(m.maint.ft);
//  t += TIME_OF_TOLERANCE / 2;
  t /= TIME_OF_TOLERANCE;
  if(!t)
    t = TIME_OF_TOLERANCE;
  else
    t *= TIME_OF_TOLERANCE;
  MK_I64(m.maint.ft) = t;
}
//-------------------------------------------------------------------------
bool gest_maint::fillList(setOfMaint &l, bool& fullExpired, bool force)
{
  int expired = 0;
  l.flush(); // per sicurezza, prima la scarica
  do {
#if 1
    P_File fl(FILENAME_MAINT);
    if(!fl.P_open())
      __MSG__
#else
    P_File_Lock fl(FILENAME_MAINT);
    if(!fl.isLocked())
      __MSG__
#endif
    else {
      g_maint fgm[MAX_MAINT];
//      ZeroMemory(fgm, sizeof(fgm));
      // legge tutto il file
      fl.P_read(fgm, sizeof(g_maint) * MAX_MAINT);
      g_mem_maint mOld;
      for(int i = 0; i < MAX_MAINT; ++i) {
        // se è presente nel file dei testi
        if(getString(ID_FIRST_MAINT + i)) {
          // se non gli è stato ancora assegnato un time, non lo carica
          if(!fgm[i].hours)
            continue;
          // crea un temporaneo per il record corrente
          g_mem_maint m(i, fgm[i]);
          // se negativo significa che è già scaduto, ma non resettato
          // e non va caricato nella lista
          if(m < 0 && !force) {
            ++expired;
            continue;
            }
          // setta una tolleranza affinché scadenze
          // vicine vengano processate assieme
          set_tolerance(m);
          if(!l.setFirst()) {
            g_mem_maint *last = new g_mem_maint(i, fgm[i]);
            l.Push(last);
            mOld = m;
            }
          else {
            // se quello corrente ha scadenza superiore a quello già
            // caricato, salta
            if(!force && m > mOld)
              continue;
            // altrimenti alloca memoria
            g_mem_maint *add = new g_mem_maint(i, fgm[i]);
            // se il nuovo ha scadenza inferiore, scarica tutti
            // quelli precedenti, se uguale lo aggiunge
            if(!force && m < mOld) {
              l.flush();
              mOld = m;
              }
            l.Push(add);
            }
          }
        }
      }
    } while(false);
  if(l.setFirst())
    return true;
  fullExpired = toBool(expired);
  return toBool(expired);
}
//-------------------------------------------------------------------------
void gest_maint::start()
{
  stop();
  do {
    criticalLock lck(CS);
    goingOut = false;
    hEvent = 0;
    } while(false);
#if 0
  typedef unsigned ( __stdcall * pfThread )( void * );
  HANDLE hThread = (HANDLE)_beginthreadex(0, 0, (pfThread)ScadManutProc, this, 0, (unsigned*)&idThread);
#else
  HANDLE hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ScadManutProc,
                  this, 0, &idThread);
#endif
  if(hThread)
    CloseHandle(hThread);
}
//----------------------------------------------------------------------------
void gest_maint::stop()
{
  // se c'è un thread avviato
  while(true) {
    if(!idThread)
      break;
    HANDLE hEv;
    bool going;

    do {
      criticalLock lck(CS);
      hEv = hEvent;
      going = goingOut;
      } while(false);

    if(hEv)
      // segnala al thread di chiudere
      SetEvent(hEv);
    else if(going)
      break;
    Sleep(10);
    }
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static void resetIf(uint cod, g_maint &gm, setOfMaint &Manut)
{
  if(Manut.setFirst()) {
    do {
      const g_mem_maint *g = Manut.getMaint();
      if(cod == g->id) {
        MK_I64(gm.ft) = gm.hours * HOUR_TO_I64 + 2 * SECOND_TO_I64;
        return;
        }
      } while(Manut.setNext());
    }
}

//----------------------------------------------------------------------------
//DWORD FAR PASCAL gest_maint::ScadManutProc(LPSTR cl)
//unsigned __stdcall gest_maint::ScadManutProc(void *cl)
DWORD WINAPI ScadManutProc(LPVOID cl)
{
  setOfMaint Manut;

  gest_maint *pGest = reinterpret_cast<gest_maint*>(cl);

  bool allFinished = false;
  // se non ci sono manutenzioni da attuare
  if(!pGest->fillList(Manut, allFinished)) {
    pGest->resetThread();
    return true;
    }

  HANDLE hEv = CreateEvent( 0, false, false, 0);
    // se non riesce a creare l'evento
  if(!hEv) {
    pGest->resetThread();
    return true;
    }
  do {
    criticalLock lck(pGest->CS);
    pGest->hEvent = hEv;
    } while(false);

  uint nElem = Manut.getNumElem();
  const g_mem_maint *gm = nElem ? Manut.getMaint() : 0;
   // remain contiene il tempo che manca alla scadenza del gruppo più prossimo
  __int64 remain = gm ? gm->remain() : TIME_FOR_SAVE * 1000;

  if(remain < TIME_FOR_SAVE * 1000)
    remain = TIME_FOR_SAVE * 1000;

  FILETIME init = getFileTimeCurr();
  DWORD result = WAIT_TIMEOUT;

  while(remain >= 0) {
    __int64 rem = remain / 1000;
    if(rem > 0x7fffffff)
      rem = 0x7fffffff;
    DWORD time = min((DWORD)rem, TIME_FOR_SAVE);

    // attesa di segnalazione da parte della classe o timeout
    if(remain)
      result = WaitForSingleObject(hEv, time);

    // qualunque sia l'evento, salva i dati correnti
#if 1
    P_File fl(FILENAME_MAINT);
    if(fl.P_open()) {
#else
    P_File_Lock fl(FILENAME_MAINT);
    if(fl.isLocked()) {
#endif
      g_maint fgm[MAX_MAINT];
       // legge tutto il file
      fl.P_read(fgm, sizeof(g_maint) * MAX_MAINT);
      FILETIME ft = getFileTimeCurr();
      // calcola il tempo trascorso dall'inizio della procedura o
      // dall'elaborazione precedente
      __int64 intermed = MK_I64(ft) - MK_I64(init);
      init = ft;
      // aggiorna il tempo rimasto ed il tempo di lavoro
      remain -= intermed;
      if(remain <= 0)
      // per evitare che rientri immediatamente se intermed è rimasto a zero
        --remain; // rende sicuramente negativo
      for(int i = 0; i < MAX_MAINT; ++i)
        if(fgm[i].hours) {
          if(remain < 0 && !allFinished)
            resetIf(i, fgm[i], Manut);
          else
            MK_I64(fgm[i].ft) += intermed;
          }
       // si riposiziona e scrive il file
      fl.P_seek(0);
      fl.P_write(fgm, sizeof(g_maint) * MAX_MAINT);
      do {
        criticalLock lck(pGest->CS);
        pGest->Dirty = true;
        } while(false);
      }
    else
      __MSG__

    if(WAIT_TIMEOUT != result)
      break;
    }

  pGest->resetThread();

  if(WAIT_TIMEOUT == result) {

    // avvia un altro thread
    pGest->start();
    static bool onExec;
    if(!onExec) {
      onExec = true;
      if(allFinished)
        allFinished = !pGest->fillList(Manut, allFinished, true);
      if(!allFinished)  {
        PWin* w = getCurrBody();
        P_ListMaint(pGest->getMainThreadId(), Manut, w).modal();
        }
      onExec = false;
      }
    }
  // la chiusura dell'handle per l'evento l'ho dovuta spostare
  // a dopo l'avvio di un nuovo thread altrimenti veniva riassegnato
  // lo stesso handle e non funzionava più la SetEvent()
  CloseHandle(hEv);
  return true;
}
//-------------------------------------------------------------------------
void gest_maint::reset()
{
  bool running = isRunning();
  stop();
  do {
#if 1
    P_File fl(FILENAME_MAINT);
    if(fl.P_open()) {
#else
    P_File_Lock fl(FILENAME_MAINT);
    if(fl.isLocked()) {
#endif
      g_maint fgm[MAX_MAINT];
      fl.P_read(fgm, sizeof(g_maint) * MAX_MAINT);
      for(int i = 0; i < MAX_MAINT; ++i) {
        // crea un temporaneo per il record corrente
        g_mem_maint m(i, fgm[i]);
        if(m < 0)
          MK_I64(fgm[i].ft) = 0;
        }
      fl.P_seek(0);
      fl.P_write(fgm, sizeof(g_maint) * MAX_MAINT);
      }
    else
      __MSG__
    } while(false);
  if(running)
    start();
}
//-------------------------------------------------------------------------
void gest_maint::setGen(uint pos, uint vHours, __int64 vLeft, tset type)
{
  if(pos >= MAX_MAINT)
    return;
  bool running = isRunning();
  stop();
  do {
#if 1
    P_File fl(FILENAME_MAINT);
    if(fl.P_open()) {
#else
    P_File_Lock fl(FILENAME_MAINT);
    if(fl.isLocked()) {
#endif
      g_maint fgm[MAX_MAINT];
      fl.P_read(fgm, sizeof(g_maint) * MAX_MAINT);
      switch(type) {
        case _hours:
          fgm[pos].hours = vHours;
          break;
        case _left:
          MK_I64(fgm[pos].ft) = vLeft;
          break;
        case _all:
          fgm[pos].hours = vHours;
          MK_I64(fgm[pos].ft) = vLeft;
          break;
        }
      fl.P_seek(0);
      fl.P_write(fgm, sizeof(g_maint) * MAX_MAINT);
      }
    else
      __MSG__
    } while(false);
  if(running)
    start();
}
//-------------------------------------------------------------------------
void gest_maint::setGen(g_maint* all, tset type)
{
  bool running = isRunning();
  stop();
  do {
#if 1
    P_File fl(FILENAME_MAINT);
    if(fl.P_open()) {
#else
    P_File_Lock fl(FILENAME_MAINT);
    if(fl.isLocked()) {
#endif
      g_maint fgm[MAX_MAINT];
      fl.P_read(fgm, sizeof(g_maint) * MAX_MAINT);
      int i;
      switch(type) {
        case _hours:
          for(i = 0; i < MAX_MAINT; ++i)
            fgm[i].hours = all[i].hours;
          break;
        case _left:
          for(i = 0; i < MAX_MAINT; ++i)
            fgm[i].ft = all[i].ft;
          break;
        case _all:
          for(i = 0; i < MAX_MAINT; ++i)
            fgm[i] = all[i];
          break;
        }
      fl.P_seek(0);
      fl.P_write(fgm, sizeof(g_maint) * MAX_MAINT);
      }
    else
      __MSG__
    } while(false);
  if(running)
    start();
}
//----------------------------------------------------------------------------
bool gest_maint::getAll(g_maint* buff)
{
#if 1
  P_File fl(FILENAME_MAINT, P_READ_ONLY);
  if(fl.P_open()) {
#else
  P_File_Lock fl(FILENAME_MAINT);
  if(fl.isLocked()) {
#endif
    fl.P_read(buff, sizeof(g_maint) * MAX_MAINT);
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
extern uint calcLenMaintName();
//----------------------------------------------------------------------------
P_ListMaint::P_ListMaint(DWORD idThreadTo, setOfMaint &l, PWin* parent, int id,
      HINSTANCE hinstance) : baseClass(parent, id, hinstance), L(l)
      , mainThreadId(idThreadTo)
      //, old(0)
{
  LB = new PListBox(this, IDC_LB_MAINT);
  int tab[1] = { calcLenMaintName() };
  LB->SetTabStop(SIZE_A(tab), tab, 0);
}
//----------------------------------------------------------------------------
P_ListMaint::~P_ListMaint()
{
  destroy();
  if(oldFocus)
    SetFocus(oldFocus);
  // evita che venga assegnato erroneamente nel thread corrente
  // dal distruttore di PDialog
  oldFocus = 0;
  AttachThreadInput(GetCurrentThreadId(), mainThreadId, false);
}
//----------------------------------------------------------------------------
/*
void DisplayErrorString(DWORD dwErr)
{
  TCHAR buff[255];
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL,
        dwErr, LANG_SYSTEM_DEFAULT, buff, SIZE_A(buff), 0);
  MessageBox(0, buff, _T("Descrizione Errore"), MB_OK);
}
*/
bool P_ListMaint::create()
{
  if(!baseClass::create())
    return false;
  // per chiudersi automaticamente dopo 5 minuti
  SetTimer(*this, 100, 1000 * 60 * 5, 0);

#if 1
  setWindowTextByLangGlob(*this, ID_TITLE_LIST_MANUT);
#else
  ::SetWindowText(*this, getStringOrId(ID_TITLE_LIST_MANUT));
#endif
  if(L.setFirst()) {
    TCHAR buff[200];
//    fillStr(buff, _T(' '), SIZE_A(buff));
    do {
      const g_mem_maint *m = L.getMaint();
#if 1
      bool needDelete;
      LPCTSTR p = getStringByLangGlob(ID_FIRST_MAINT + m->id, needDelete);
      smartPointerConstString sp(p, needDelete);
#else
      LPCTSTR p = getString(ID_FIRST_MAINT + m->id);
#endif
      if(p) {
        wsprintf(buff, _T("%02d - %s"), m->id + 1, p);
        ::SendMessage(*LB, LB_ADDSTRING, 0, (WPARAM) buff);
        }
      } while(L.setNext());
    }
  SendMessage(*LB, LB_SETCURSEL, 0, 0);

  int width = LB->getLenTab(0) + GetSystemMetrics(SM_CXVSCROLL) + 4;
  PRect r;
  GetClientRect(*LB, r);
  r.right = r.left + width;

  SetWindowPos(*LB, 0, 0, 0, r.Width(), r.Height(), SWP_NOZORDER);
  LONG_PTR style = GetWindowLongPtr(*this, GWL_STYLE);
  AdjustWindowRect(r, style, FALSE);

  SetWindowPos(*this, 0, 0, 0, r.Width(), r.Height(), SWP_NOZORDER | SWP_NOMOVE);

  return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
LRESULT P_ListMaint::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_INITDIALOG:
      AttachThreadInput(GetCurrentThreadId(), mainThreadId, true);
      break;

    case WM_TIMER:
      if(100 == wParam)
        PostMessage(hwnd, WM_COMMAND, IDOK, 0);
      break;

    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_F1:
        case IDOK:
        case IDCANCEL:
          KillTimer(hwnd, 100);
          EndDialog(hwnd, IDOK);
          break;
        }
      break;
    }
  return baseClass::windowProc( hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------

