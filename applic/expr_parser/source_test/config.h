//---------------- config.h -----------------------------------------
//-------------------------------------------------------------------
#ifndef config_H
#define config_H
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "resource.h"
#include "PModDialog.h"
#include "exp_BaseContainer.h"
//-------------------------------------------------------------------
#include "setPack.h"
//-------------------------------------------------------------------
class PDialogBaseColor;
//-------------------------------------------------------------------
class PDialogColor : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    PDialogColor(PWin* parent, uint id, infoEdit* iedit, HINSTANCE hInst = 0);
    ~PDialogColor() { destroy();  }
    bool create();

  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    PDialogBaseColor* Pages[MAX_BODY];
    void CmOk();
    infoEdit* iEdit;
    int currPage;
    void chgPage(int page);
    void moveChild(PWin* child);
};
//-------------------------------------------------------------------
class PDialogSearchReplace : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    PDialogSearchReplace(PWin* parent, uint id, exp_BaseContainer** owners, bool read_only, HINSTANCE hInst = 0);
    ~PDialogSearchReplace() { destroy();  }
    bool create();
    void setCurrSel(bool read_only);
    bool preProcessMsg(MSG& msg);
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CmOk() {}
    void CmCancel();

    void getFlag();
    void find(bool next);
    void replace(bool next);
    void replaceAll();

    exp_BaseContainer** Owners;
    static DWORD flag;
    static DWORD target;
    static TCHAR lastWord[512];
    static TCHAR lastReplace[512];
    bool readOnly;
    void checkReadOnly();
};
//-------------------------------------------------------------------
#include "restorePack.h"
//-------------------------------------------------------------------
#endif
