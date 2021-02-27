//--------- language_util.h ----------------------------------------------
//-------------------------------------------------------------------
#ifndef language_util_H_
#define language_util_H_
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "pedit.h"
#include "p_Txt.h"
#include "smartPS.h"
//-------------------------------------------------------------------
#define MAX_LANG 10
//-------------------------------------------------------------------
class langEdit : public PEdit
{
  private:
    typedef PEdit baseClass;
  public:
    langEdit(PWin * parent, uint resid, int textlen = 0, HINSTANCE hinst = 0) :
          baseClass(parent, resid, textlen, hinst) {}
    langEdit(PWin * parent, uint resid, const PRect& r, LPCTSTR text = 0, int textlen = 255, HINSTANCE hinst = 0) :
          baseClass(parent, resid, r, text, textlen, hinst) {}
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void runEditMulti(bool byMenu = false);
    virtual void runGlobalPageTExt(bool byMenu = false);
    virtual void openMenuLang();
};
//-------------------------------------------------------------------
// ereditando da langCheckEdit si possono evitare/forzare la gestione della lingua
#define FORCE_NO_LANG ((uint)-1)
#define FORCE_ON_LANG ((uint)-2)
#define FORCE_NO_ALL  ((uint)-3)
#define FORCE_ON_GLOB ((uint)-4)
#define FORCE_OFF     ((uint)-5)
//-------------------------------------------------------------------
class langCheckEdit : public langEdit
{
  private:
    typedef langEdit baseClass;
  public:
    langCheckEdit(PWin * parent, uint resid, uint resid2, int textlen = 0, HINSTANCE hinst = 0) :
          baseClass(parent, resid, textlen, hinst), resId2(resid2) {}

    langCheckEdit(PWin * parent, uint resid, uint resid2, const PRect& r, LPCTSTR text = 0, int textlen = 255, HINSTANCE hinst = 0) :
          baseClass(parent, resid, r, text, textlen, hinst), resId2(resid2) {}
  protected:
    uint resId2;
    virtual void runEditMulti(bool byMenu = false);
    virtual void openMenuLang();
    virtual void runGlobalPageTExt(bool byMenu = false);
    int checkRunEdit();
};
//-------------------------------------------------------------------
class langEditDefCR : public langEdit
{
  private:
    typedef langEdit baseClass;
  public:
    langEditDefCR(PWin * parent, uint resid, int textlen = 0, HINSTANCE hinst = 0) :
          baseClass(parent, resid, textlen, hinst) {}
    langEditDefCR(PWin * parent, uint resid, const PRect& r, LPCTSTR text = 0, int textlen = 255, HINSTANCE hinst = 0) :
          baseClass(parent, resid, r, text, textlen, hinst) {}
  protected:
    int verifyKey()  { return false; }
    virtual bool evChar(UINT& key) { return false; }
    bool evKeyDown(UINT& key) { return false; }
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
LPCTSTR getStringByLangSimple(LPCTSTR source, bool& needDelete);
//----------------------------------------------------------------------------
smartPointerConstString getStringByLangSimple(LPCTSTR source);
//-------------------------------------------------------------------
bool isGlobalPageString(LPCTSTR p);
//-------------------------------------------------------------------
void gRunGlobalPageTExt(PWin* owner);
//-------------------------------------------------------------------
#endif
