//------------------- PTextPanelStatic.h --------------------
//-----------------------------------------------------------
#ifndef PTextPanelStatic_H_
#define PTextPanelStatic_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pStatic.h"
#include "pTextPanel.h"
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class IMPORT_EXPORT PTextPanelStatic : public PStatic
{
  private:
    typedef PStatic baseClass;
  public:
    PTextPanelStatic(PWin * parent, uint id, int x, int y, int w, int h,
            LPCTSTR text = 0, HINSTANCE hinst = 0) : Text(0),
        baseClass(parent, id, x, y, w, h, text, hinst) {}
    PTextPanelStatic(PWin * parent, uint id, const PRect& r,
            LPCTSTR text = 0, HINSTANCE hinst = 0) : Text(0),
        baseClass(parent, id, r, text, hinst) {}
    PTextPanelStatic(PWin * parent, uint resid, HINSTANCE hinst = 0) : Text(0),
        baseClass(parent, resid, hinst) {}

    ~PTextPanelStatic() { destroy(); delete Text; }
    virtual bool create();

    // o si ridefiniscono tutti i metodi o si ritorna l'oggetto e si usa quello per
    // modificare le proprietà, più semplice così
    PTextFixedPanel* getPanelText() { return Text; }
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    PTextFixedPanel* Text;
};
/*
l'oggetto Text viene creato per default in questo modo:
  Text = new PTextFixedPanel(0, rect..., getFont(), RGB(0, 0, 0), PPanel::BORDER_FILL, RGB(0xca,0xca,0xff));
  N.B. il testo è sempre quello presente in PStatic, quello di Text viene ignorato, quindi inutile usare
  l'oggetto per assegnare il testo
*/
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
