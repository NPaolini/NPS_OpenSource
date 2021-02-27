//-------------------- pProgress.h ---------------------------
//-----------------------------------------------------------
#ifndef PPROGRESS_H_
#define PPROGRESS_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#ifndef PCONTROL_H_
  #include "pControl.h"
#endif
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class IMPORT_EXPORT PProgress : public PControl
{
  public:
    PProgress(PWin * parent, uint id, int x, int y, int w, int h,
              LPCTSTR text = 0, HINSTANCE hinst = 0);
    PProgress(PWin * parent, uint id, const PRect& r,
              LPCTSTR text = 0, HINSTANCE hinst = 0);
    PProgress(PWin * parent, uint resid, HINSTANCE hinst = 0);

    ~PProgress() { destroy(); }

  protected:
    virtual LPCTSTR getClassName() const;
};
//-----------------------------------------------------------
//-----------------------------------------------------------
inline PProgress::PProgress(PWin * parent, uint id, int x, int y, int w, int h,
                    LPCTSTR text, HINSTANCE hinst) :
      PControl(parent, id, x, y, w, h, text, hinst) { }
//-----------------------------------------------------------
inline PProgress::PProgress(PWin * parent, uint id, const PRect& r, LPCTSTR text, HINSTANCE hinst) :
      PControl(parent, id, r, text, hinst)  { }
//-----------------------------------------------------------
inline PProgress::PProgress(PWin * parent, uint resid, HINSTANCE hinst) :
        PControl(parent, resid, hinst)  { }
//-----------------------------------------------------------
inline LPCTSTR PProgress::getClassName() const { return PROGRESS_CLASS; }
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
