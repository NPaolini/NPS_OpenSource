//-------------------- pStatic.h ---------------------------
//-----------------------------------------------------------
#ifndef PSTATIC_H_
#define PSTATIC_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#ifndef PCONTROL_H_
  #include "pControl.h"
#endif
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class IMPORT_EXPORT PStatic : public PControl
{
  public:
    PStatic(PWin * parent, uint id, int x, int y, int w, int h,
            LPCTSTR text = 0, HINSTANCE hinst = 0);
    PStatic(PWin * parent, uint id, const PRect& r,
            LPCTSTR text = 0, HINSTANCE hinst = 0);
    PStatic(PWin * parent, uint resid, HINSTANCE hinst = 0);

    ~PStatic() { destroy(); }

  protected:
    virtual LPCTSTR getClassName() const;
};
//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
inline PStatic::PStatic(PWin * parent, uint id, int x, int y, int w, int h,
                    LPCTSTR text, HINSTANCE hinst) :
      PControl(parent, id, x, y, w, h, text, hinst)
{
  Attr.style &= ~WS_TABSTOP;
  Attr.style |= SS_CENTER | SS_SUNKEN ;
}
//-----------------------------------------------------------
inline PStatic::PStatic(PWin * parent, uint id, const PRect& r, LPCTSTR text, HINSTANCE hinst) :
      PControl(parent, id, r, text, hinst)
{
  Attr.style &= ~WS_TABSTOP;
  Attr.style |= SS_CENTER | SS_SUNKEN ;
}
//-----------------------------------------------------------
inline PStatic::PStatic(PWin * parent, uint resid, HINSTANCE hinst) :
        PControl(parent, resid, hinst)
{ }
//-----------------------------------------------------------
inline LPCTSTR PStatic::getClassName() const { return _T("static"); }
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif