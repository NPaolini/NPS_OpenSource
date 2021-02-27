//-------------------- pVisualTextEdit.h ---------------------------
//-----------------------------------------------------------
#ifndef pVisualTextEdit_H_
#define pVisualTextEdit_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "pEdit.h"
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
class pVisualTextEdit
{
  public:
    pVisualTextEdit(PEdit* owner) : Owner(owner), onFocus(false) {}
    virtual ~pVisualTextEdit() {}
    virtual void setFocus();
    virtual void killFocus();
    virtual void getData(LPTSTR buff, size_t dim);
    virtual void setData(LPCTSTR buff);
  protected:
    PEdit* Owner;
    bool onFocus;
};
//-----------------------------------------------------------
class pVisualTextEditSep : public pVisualTextEdit
{
  private:
    typedef pVisualTextEdit baseClass;
  public:
    pVisualTextEditSep(PEdit* owner, TCHAR sep, uint step, TCHAR decSep = 0) :
        baseClass(owner), Sep(sep), Step(step), DecSep(decSep), Sign(true) {}
    virtual ~pVisualTextEditSep() {}

    virtual void setFocus();
    virtual void killFocus();
    virtual void getData(LPTSTR buff, size_t dim);
    virtual void setData(LPCTSTR buff);

    static void removeSep(LPTSTR buff, TCHAR sep);
    static void addSep(LPTSTR target, LPCTSTR source, TCHAR sep, uint step, bool sign = true);
    static void addSep(LPTSTR target, LPCTSTR source, TCHAR sep, TCHAR decSep, uint step, bool sign = true);
    bool checkSign(bool set) { bool old = Sign; Sign = set; return old; }
  protected:
    TCHAR Sep;
    uint Step;
    TCHAR DecSep;
    bool Sign;

};
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
