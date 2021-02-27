//-------- svEdit.h --------------------------------------------------------
#ifndef svEdit_H_
#define svEdit_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "pedit.h"
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
class svEdit : public PEdit
{
  private:
    typedef PEdit baseClass;
  public:
    svEdit(PWin * parent, uint id, int x, int y, int w, int h,
              LPCTSTR text = 0, int textlen = 255, HINSTANCE hinst = 0);
    svEdit(PWin * parent, uint id, const PRect& r,
              LPCTSTR text = 0, int textlen = 255, HINSTANCE hinst = 0);
    svEdit(PWin * parent, uint resid, int textlen = 255, HINSTANCE hinst = 0);

    virtual ~svEdit() { destroy(); }

    void makeSingleLine();
    void copyFilter(HWND from);

  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
};
//-----------------------------------------------------------
inline svEdit::svEdit(PWin * parent, uint id, int x, int y, int w, int h,
                    LPCTSTR text, int textlen, HINSTANCE hinst) :
      baseClass(parent, id, x, y, w, h, text, textlen, hinst)
{ }
//-----------------------------------------------------------
inline svEdit::svEdit(PWin * parent, uint id, const PRect& r, LPCTSTR text,
                    int textlen, HINSTANCE hinst) :
      baseClass(parent, id, r, text, textlen, hinst)
{ }
//-----------------------------------------------------------
inline svEdit::svEdit(PWin * parent, uint resid, int textlen, HINSTANCE hinst) :
        baseClass(parent, resid, textlen, hinst)
{ }
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

