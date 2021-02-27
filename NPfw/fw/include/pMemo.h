//-------------------- pMemo.h ---------------------------
//-----------------------------------------------------------
#ifndef PMEMO_H_
#define PMEMO_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#define SIMPLE_CLASS
#ifdef SIMPLE_CLASS

#ifndef PCONTROL_H_
  #include "pControl.h"
#endif
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class IMPORT_EXPORT PMemo :   public PControl
{
  public:
    PMemo(PWin * parent, uint id, int x, int y, int w, int h,
              LPCTSTR text = 0, HINSTANCE hinst = 0);
    PMemo(PWin * parent, uint id, const PRect& r,
              LPCTSTR text = 0, HINSTANCE hinst = 0);
    PMemo(PWin * parent, uint resid, HINSTANCE hinst = 0);

    ~PMemo() { destroy(); }

  protected:
    virtual LPCTSTR getClassName() const;
};
//-----------------------------------------------------------
//-----------------------------------------------------------
inline PMemo::PMemo(PWin * parent, uint id, int x, int y, int w, int h,
                    LPCTSTR text, HINSTANCE hinst) :
      PControl(parent, id, x, y, w, h, text, hinst)
{
  Attr.exStyle |= WS_EX_CLIENTEDGE;
  Attr.style |= ES_LEFT |
                ES_MULTILINE |
                WS_VSCROLL | ES_WANTRETURN;
}
//-----------------------------------------------------------
inline PMemo::PMemo(PWin * parent, uint id, const PRect& r, LPCTSTR text, HINSTANCE hinst) :
      PControl(parent, id, r, text, hinst)
{
  Attr.exStyle |= WS_EX_CLIENTEDGE;
  Attr.style |= ES_LEFT |
                ES_MULTILINE |
                WS_VSCROLL | ES_WANTRETURN;
}
//-----------------------------------------------------------
inline PMemo::PMemo(PWin * parent, uint resid, HINSTANCE hinst) :
        PControl(parent, resid, hinst)
{ }
//-----------------------------------------------------------
inline LPCTSTR PMemo::getClassName() const { return _T("edit"); }
//-----------------------------------------------------------
//-----------------------------------------------------------

#else

#ifndef PEDIT_H_
  #include "pEdit.h"
#endif

#include "setPack.h"

class IMPORT_EXPORT PMemo :   public PEdit
{
  public:
    PMemo(PWin * parent, uint id, int x, int y, int w, int h,
              LPCTSTR text = 0, HINSTANCE hinst = 0);
    PMemo(PWin * parent, uint resid, HINSTANCE hinst = 0);

  protected:

    virtual bool evKeyDown(UINT& key) { return false; }
    virtual bool evKeyUp(UINT& key) { return false; }
    virtual bool evChar(UINT& key) { return false; }
};
//-----------------------------------------------------------
//-----------------------------------------------------------
inline PMemo::PMemo(PWin * parent, uint id, int x, int y, int w, int h,
                    LPCTSTR text, HINSTANCE hinst) :
      PEdit(parent, id, x, y, w, h, text, 0, hinst)
{
  Attr.exStyle |= WS_EX_CLIENTEDGE | WS_EX_CONTROLPARENT;
  Attr.style &= ~ES_AUTOHSCROLL;
  Attr.style |= ES_LEFT |
                ES_MULTILINE |
                WS_VSCROLL | ES_WANTRETURN;
}
//-----------------------------------------------------------
inline PMemo::PMemo(PWin * parent, uint resid, HINSTANCE hinst) :
        PEdit(parent, resid, 0, hinst)
{ }
//-----------------------------------------------------------
//-----------------------------------------------------------
#endif
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
