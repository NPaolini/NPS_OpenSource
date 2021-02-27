//------- svmPagePropAction.h ------------------------------------------------
//----------------------------------------------------------------------------
#ifndef SVMPAGEPROPACTION_H_
#define SVMPAGEPROPACTION_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef PMODDIALOG_H_
#include "pModDialog.h"
#endif

#include "resource.h"
#include "svmMainClient.h"
#ifndef MAX_BTN
  #define MAX_BTN 12
#else
  #if (MAX_BTN != 12)
    #error define errata
  #endif
#endif
//----------------------------------------------------------------------------
struct svmPageBase
{
  TCHAR name[_MAX_PATH];
  int id;
  int pswLevel;
  svmPageBase() : id(0), pswLevel(0) { fillStr(name, 0, SIZE_A(name)); }
};
//----------------------------------------------------------------------------
class svmPagePropActionGetPrintScreen : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    svmPagePropActionGetPrintScreen(PWin* parent, svmPageBase& data, uint resId = IDD_ACTION_PRINT_SCREEN, LPCTSTR title = 0, HINSTANCE hinstance = 0);
    virtual ~svmPagePropActionGetPrintScreen();
  protected:
    svmPageBase& Data;
  public:
    virtual bool create();
    void CmOk();

};
//----------------------------------------------------------------------------
class svmPagePropAction : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    svmPagePropAction(PWin* parent, svmPageBase& data, uint resId = IDD_ACTION_ONLY_PSW, LPCTSTR title = 0, HINSTANCE hinstance = 0);
    virtual ~svmPagePropAction();
  protected:
    svmPageBase& Data;
  public:
    virtual bool create();
    void CmOk();

    void addEditToCB(HWND hcbx, LPCTSTR row);
  protected:
//    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  private:
    void fillPsw();

};
//----------------------------------------------------------------------------
class svmPagePropActionOpenPage : public svmPagePropAction
{
  private:
    typedef svmPagePropAction baseClass;
  public:
    svmPagePropActionOpenPage(PWin* parent, svmPageBase& data, uint resId = IDD_ACTION_OPEN_PAGE, HINSTANCE hinstance = 0) :
      baseClass(parent, data, resId, 0, hinstance) {}
    virtual ~svmPagePropActionOpenPage() { destroy(); }
  public:
    virtual bool create();
    void CmOk();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  private:
    void checkEnable();
    void addNameToCB(HWND hcbx);
};
//----------------------------------------------------------------------------
class svmPagePropActionOpenName : public svmPagePropAction
{
  private:
    typedef svmPagePropAction baseClass;
  public:
    svmPagePropActionOpenName(PWin* parent, svmPageBase& data, LPCTSTR title = 0, uint resId = IDD_ACTION_OPEN_NAME, HINSTANCE hinstance = 0) :
      baseClass(parent, data, resId, title, hinstance) {}
    virtual ~svmPagePropActionOpenName() { destroy(); }
  public:
    virtual bool create();
    void CmOk();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  private:
};
//----------------------------------------------------------------------------
class svmPagePropActionGetPerif : public svmPagePropAction
{
  private:
    typedef svmPagePropAction baseClass;
  public:
    svmPagePropActionGetPerif(PWin* parent, svmPageBase& data, LPCTSTR title = 0, uint resId = IDD_ACTION_OPEN_PERIF, HINSTANCE hinstance = 0) :
      baseClass(parent, data, resId, title, hinstance) {}
    virtual ~svmPagePropActionGetPerif() { destroy(); }
  public:
    virtual bool create();
    void CmOk();
  protected:
};
//----------------------------------------------------------------------------
class svmPagePropActionGetCTRL_Action : public svmPagePropAction
{
  private:
    typedef svmPagePropAction baseClass;
  public:
    svmPagePropActionGetCTRL_Action(PWin* parent, svmPageBase& data, LPCTSTR title = 0, uint resId = IDD_ACTION_OPEN_PERIF, HINSTANCE hinstance = 0) :
      baseClass(parent, data, resId, title, hinstance) {}
    virtual ~svmPagePropActionGetCTRL_Action() { destroy(); }
  public:
    virtual bool create();
    void CmOk();
  protected:
};

//----------------------------------------------------------------------------
struct perifForBtnAction
{
  union {
    struct single {
      DWORD prf  :5;
      DWORD type :5;
      DWORD neg  :1;
      DWORD nbit :6;
      DWORD offs :6;
      } s;
    DWORD allBit;
    } U;

  DWORD addr;
  __int64 mask;
  perifForBtnAction() : addr(0), mask(0) { U.allBit = 0; }
//  perifForBtnAction() : prf(0), type(0), neg(0), nbit(0), offs(0), addr(0), mask(0) {}
  static void toString(LPTSTR target, const perifForBtnAction& data);
  static void fromString(perifForBtnAction& data, LPCTSTR source);
};
//----------------------------------------------------------------------------
class svmPagePropActionGetIdBtn : public svmPagePropAction
{
  private:
    typedef svmPagePropAction baseClass;
  public:
    svmPagePropActionGetIdBtn(PWin* parent, svmPageBase& data, LPCTSTR title = 0, uint resId = IDD_ACTION_OPEN_ID, HINSTANCE hinstance = 0) :
      baseClass(parent, data, resId, title, hinstance) {}
    virtual ~svmPagePropActionGetIdBtn() { destroy(); }
  public:
    virtual bool create();
    void CmOk();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  private:
    void chgNameMask();
};
//----------------------------------------------------------------------------
class svmPagePropActionGetExport : public svmPagePropAction
{
  private:
    typedef svmPagePropAction baseClass;
  public:
    svmPagePropActionGetExport(PWin* parent, svmPageBase& data, LPCTSTR title = 0, uint resId = IDD_ACTION_EXPORT, HINSTANCE hinstance = 0) :
      baseClass(parent, data, resId, title, hinstance) {}
    virtual ~svmPagePropActionGetExport() { destroy(); }
  public:
    virtual bool create();
    void CmOk();
};
//----------------------------------------------------------------------------
class svmPagePropActionSpin : public svmPagePropAction
{
  private:
    typedef svmPagePropAction baseClass;
  public:
    svmPagePropActionSpin(PWin* parent, svmPageBase& data, LPCTSTR title = 0, uint resId = IDD_ACTION_SPIN, HINSTANCE hinstance = 0) :
      baseClass(parent, data, resId, title, hinstance) {}
    virtual ~svmPagePropActionSpin() { destroy(); }
  public:
    virtual bool create();
    void CmOk();
};
//----------------------------------------------------------------------------
#endif

