//-------------------- svVisualTextEdit.h -------------------
//-----------------------------------------------------------
#ifndef svVisualTextEdit_H_
#define svVisualTextEdit_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pVisualTextEdit.h"
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class svVisualTextEdit : public pVisualTextEdit
{
  private:
    typedef pVisualTextEdit baseClass;
  public:
    svVisualTextEdit(PEdit* owner) : baseClass(owner) {}
    virtual void setFocus();
    virtual void killFocus();
    virtual void setData(LPCTSTR buff);
};
//-----------------------------------------------------------
class svVisualTextEditSep : public pVisualTextEditSep
{
  private:
    typedef pVisualTextEditSep baseClass;
  public:
    svVisualTextEditSep(PEdit* owner, TCHAR sep, uint step, TCHAR sepDec = 0) : baseClass(owner, sep, step, sepDec) {}
    virtual ~svVisualTextEditSep() {}

    virtual void setFocus();
    virtual void killFocus();
    virtual void setData(LPCTSTR buff);
};
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
