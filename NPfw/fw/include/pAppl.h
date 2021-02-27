//-------------------- pAppl.H ---------------------------
#ifndef PAPPL_H_
#define PAPPL_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#ifndef PWIN_H
  #include "pWin.h"
#endif
//-----------------------------------------------------------
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class IMPORT_EXPORT PAppl
{
  public:
    PAppl(HINSTANCE hInstance, int nCmdShow);
    virtual ~PAppl();

    virtual WPARAM run(LPCTSTR title);

    WPARAM pumpMessages();
    WPARAM pumpOnlyOneMessage(bool &wantMore);

    virtual WPARAM end(WPARAM val);

    void setMainWin(PWin* mw) { mainWindow = mw; }

    static HINSTANCE getHinstance() {   return hInstance; }

    PWin *getMainWindow() const;

    bool isEnding();

    void setCmdShow(int nCmdShow) { howShow = nCmdShow; }
  protected:
    // metodo da ridefinire per assegnare PWin personalizzata.
    // Per default usa la PMainWin di base
    virtual PWin *initMainWindow(LPCTSTR title, HINSTANCE hInstance);

  private:
    static HINSTANCE hInstance;

    int howShow;
    DWORD idleCount;
    bool endLoop;
    PWin *mainWindow;
    virtual bool preProcessMessage(MSG& msg);

    NO_COPY_COSTR_OPER(PAppl)
};
//-----------------------------------------------------------
//-----------------------------------------------------------
inline bool PAppl::isEnding()
{
  return endLoop;
}
//-----------------------------------------------------------
inline WPARAM PAppl::end(WPARAM val)
{
  return val;
}
//-----------------------------------------------------------
inline
PWin* PAppl::getMainWindow() const
{
  return mainWindow;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
// torna il puntatore all'applicazione corrente
extern IMPORT_EXPORT PAppl* getAppl();
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------

#endif

