//------------- P_Base.h -----------------------------------------------------
//----------------------------------------------------------------------------
#ifndef P_BASE_H_
#define P_BASE_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "resource.h"
//----------------------------------------------------------------------------
#ifndef PMODDIALOG_H_
  #include "PModDialog.h"
#endif
//----------------------------------------------------------------------------
// gestione periferiche
#ifndef GESTPRF_H_
  #include "gestPrf.h"
#endif
//----------------------------------------------------------------------------
// gestione statistiche
#ifndef LOG_STAT_H_
  #include "log_stat.h"
#endif
//----------------------------------------------------------------------------
// gestione nomi
#ifndef P_NAME_H_
  #include "p_name.h"
#endif
//----------------------------------------------------------------------------
#ifndef SIZE_A
  #define SIZE_A(a) (sizeof(a) / sizeof(a[0]))
#endif
#include "language.h"
#include "p_manage_ini.h"
#include "setPack.h"
//----------------------------------------------------------------------------
#define SVISOR_INI_FILE _T("npsVisor.ini")
#define MAIN_PATH _T("\\Main")
//----------------------------------------------------------------------------
class myManageIni : public manageIni
{
  private:
    typedef manageIni baseClass;
  public:
    myManageIni(LPCTSTR filename);
};
//----------------------------------------------------------------------------
// copia in dest escludendo gli spazi in testa e in coda,
// termina la stringa destinazione,
// ritorna il numero di caratteri copiati
int strcpyWithoutSpace(LPTSTR dest, LPCTSTR source);
//----------------------------------------------------------------------------
// apre una finestrina di risultato dell'operazione,
// implementata a parte, il prototipo è stato inserito qui
// per comodità
void showFileResult(bool success, PWin *parent);
// torna true per conferma sovrascrittura
bool showMsgFileExist(PWin *parent);
//----------------------------------------------------------------------------
// timer di base
#define TIME_SLICE 500
//----------------------------------------------------------------------------
// indica quali oggetti devono essere aggiornati dagli allarmi e dagli eventi di run/stop

#define rOPER       (1 << 0)
#define rTURN       (1 << 1)
#define rGLOB_STAT  (1 << 2)
#define rORDER      (1 << 3)
#define rALL_ALARM  (1 << 4)

#define rALL    (rORDER | rOPER | rTURN | rGLOB_STAT | rALL_ALARM)
//----------------------------------------------------------------------------
class mainClient;
//----------------------------------------------------------------------------
class P_Base : public PWin
//class P_Base : public PDialog
{
  private:
    typedef PWin baseClass;
  public:
    P_Base(PWin* parent, uint resId, HINSTANCE hInstance = 0);
    virtual ~P_Base();

    // richiamato dalla finestra principale in risposta a WM_TIMER
    virtual void refresh() = 0;

    void setDirty() { needRefresh = true; }

    virtual bool create();

    enum whichDirty { dBody, dData };
    virtual void postDirty(int which);
//    virtual bool preProcessMsg(MSG& msg) { return toBool(IsDialogMessage(getHandle(), &msg)); }
    virtual bool preProcessMsg(MSG& msg);

    class mainClient* getPar() { return Par; }

    enum forceVis {
        fvNoForce = 0,
        fvHideHeader = 1,   // 0001
        fvShowHeader = 3,   // 0011
        fvHideFooter = 4,   // 0100
        fvShowFooter = 12,  // 1100

        };

  protected:
    int needRefresh;
    class mainClient *Par;

    bool isBody;

    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);
    LPCTSTR getClassName() const;
    void getWindowClass(WNDCLASS& wcl);

};
//----------------------------------------------------------------------------
inline HBRUSH P_Base::evCtlColor(HDC , HWND , UINT ) { return 0; }
//inline HBRUSH P_Base::evCtlColor(HDC dc, HWND hWndChild, UINT ctlType) { return 0; }
//--------------------------------------------------------------------
//--------------------------------------------------------------------
#include "restorePack.h"
#endif

