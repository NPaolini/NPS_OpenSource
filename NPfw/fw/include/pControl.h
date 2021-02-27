//-------------------- pControl.h ---------------------------
#ifndef PCONTROL_H_
#define PCONTROL_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#ifndef PWIN_H_
  #include "pWin.h"
#endif
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
// utility per gestione tastiera
//-----------------------------------------------------------
// simula la pressione e rilascio del tasto [virtual_key]
void IMPORT_EXPORT click(WPARAM virtual_key);
//-----------------------------------------------------------
// simula la pressione del tasto [virtual_key]
void IMPORT_EXPORT down_click(WPARAM virtual_key);
//-----------------------------------------------------------
// simula il rilascio del tasto [virtual_key]
void IMPORT_EXPORT up_click(WPARAM virtual_key);
//-----------------------------------------------------------
// simula la pressione e rilascio del tasto [virtual_key] premuto
// insieme ad un tasto modificatore (shift, alt, ctrl)
// repeat indica quante volte deve essere ripetuta la simulazione della
// pressione del virtual_key mentre continua ad essere premuto lo special_key
void IMPORT_EXPORT special_click(WPARAM virtual_key, WPARAM special_key, int repeat = 0);
//-----------------------------------------------------------
// simula la pressione e rilascio del tasto [virtual_key] premuto
// insieme al tasto shift
void IMPORT_EXPORT shift_click(WPARAM virtual_key);
//-----------------------------------------------------------
//-----------------------------------------------------------
class IMPORT_EXPORT PControl : public PWin
{
  public:
    PControl(PWin * parent, uint id, int x, int y, int w, int h, LPCTSTR title = 0, HINSTANCE hinst = 0);

    PControl(PWin * parent, uint id, const PRect& r, LPCTSTR title = 0, HINSTANCE hinst = 0);

    PControl(PWin * parent, uint resid, HINSTANCE hinst = 0);
    PControl(HWND hwnd, HINSTANCE hInst) :  PWin(hwnd, hInst), hFont(0), deleteFont(false) {}

    ~PControl ();

    virtual bool create();
    virtual void setFont(HFONT font, bool autoDelete = false);

    #define SIMUL_DEC_FONT 100
    // N.B. le dimensioni riportate sono moltiplicate per SIMUL_DEC_FONT per
    // la simulazione di decimali. La simulazione viene usata per aumentare
    // la precisione del calcolo della larghezza dei caratteri
    SIZE getSizeFont();

    void setText(LPCTSTR text) { setCaption(text); }
    void getText(LPTSTR text, int dim);


  protected:

    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual LPCTSTR getClassName() const
      {
        // genera un errore a runtime se si cerca di utilizzarla direttamente
        return 0;
      }
    // viene richiamata nella gestione del messaggio WM_GETDLGCODE,
    // N.B. la verifica dei tasti di default è stata rimossa, ora è implementata
    // solo nelle classi che la utilizzano (PEdit, POwnBtn)
    // ---- per default verifica i tasti VK_RETURN, VK_DOWN e VK_UP
    // ---- es. if(GetKeyState(VK_UP)& 0x8000) key = VK_UP;
    // ---- se accetta il tasto lo deve tornare (viene simulato il rilascio
    // ---- con up_click() per evitare un rientro) e ne verrà richiesta la
    // ---- gestione con DLGC_WANTALLKEYS
    virtual int verifyKey() { return 0; }

    // deve tornare true se il messaggio viene gestito
    virtual bool evKeyDown(WPARAM& key) { return false; }
    virtual bool evKeyUp(WPARAM& key)   { return false; }
    virtual bool evChar(WPARAM& key)    { return false; }
    HFONT getFont() const { return hFont; }
  private:
    HFONT hFont;
    bool deleteFont;
    typedef PWin baseClass;
};
//-----------------------------------------------------------
inline
PControl::PControl(PWin * parent, uint id, int x, int y, int w, int h, LPCTSTR title, HINSTANCE hinst) :
      baseClass(parent, id, x, y, w, h, title, hinst), hFont(0), deleteFont(false)
{
  Attr.style |= WS_TABSTOP | WS_CHILD;
}
//-----------------------------------------------------------
inline
PControl::PControl(PWin * parent, uint id, const PRect& r, LPCTSTR title, HINSTANCE hinst):
    baseClass(parent, id, r, title, hinst), hFont(0), deleteFont(false)
{
  Attr.style |= WS_TABSTOP | WS_CHILD;
}
//-----------------------------------------------------------
inline
PControl::PControl(PWin * parent, uint resid, HINSTANCE hinst) :
      baseClass(parent, 0, hinst), hFont(0), deleteFont(false)
{
  Attr.id = resid;
  setFlag(pfFromResource);
}
//-----------------------------------------------------------
inline
void PControl::getText(LPTSTR text, int dim)
{
  SendMessage(*this, WM_GETTEXT, dim, (LPARAM)text);
}
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
