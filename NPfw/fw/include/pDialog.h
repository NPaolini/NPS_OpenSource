//-------------------- PDialog.H ---------------------------
#ifndef PDIALOG_H_
#define PDIALOG_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#ifndef PWIN_H_
  #include "pWin.h"
#endif
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class IMPORT_EXPORT PDialog : public PWin
{
  private:
    typedef PWin baseClass;
  public:
    PDialog(PWin* parent, uint id, HINSTANCE hInst = 0);
    virtual ~PDialog();
    virtual bool create();
    virtual int modal();
    virtual int smodal(bool disableOther = true);
    virtual bool preProcessMsg(MSG& msg);
    bool isCustomEraseBkg() const { return customEraseBkg; }
    void setCustomEraseBkg(bool set) { customEraseBkg = set; }
  protected:
    // se modale acquisisce il focus della window corrente per
    // ripristinarlo alla fine
    HWND oldFocus;

    virtual LPCTSTR getClassName() const { return WC_DIALOG; }

    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    DWORD Result;
    virtual HWND getFirstFocus();

  private:
    void registerChildObjects();
    HWND doCreate();
    HWND doModal();
    bool customEraseBkg;

};
//----------------------------------------------------------------------------
/*
  Se si crea un dialogo modale non è possibile servirsi del metodo preProcessMsg()
  con cui è possibile intercettare tutti i messaggi prima che siano elaborati.
  Questo sotto è un esempio di come creare un dialogo non modale ma che si comporta
  come se lo fosse.

  // disabilita la window che lo richiama
  EnableWindow(*this, false);

  // crea l'oggetto e la window
  myDialogModal dm(this);
  if(dm.create()) {
    // finché non viene chiusa la window
    while(dm.getHandle()) {
      // aspetta l'arrivo di un messaggio
      ::WaitMessage();
      // lo fa elaborare dall'applicazione principale che provvede al dispatch
      getAppl()->pumpMessages();
      }
    }
  // riabilita
  EnableWindow(*this, true);
  // giusto per sicurezza
  SetForegroundWindow(*this);

*/
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif
