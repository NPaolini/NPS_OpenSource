//------- dChoosePath.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef DCHOOSEPATH_H_
#define DCHOOSEPATH_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef PMODDIALOG_H_
#include "pModDialog.h"
#endif

#ifndef PEDIT_H_
#include "pEdit.h"
#endif

#ifndef PSTATIC_H_
#include "pStatic.h"
#endif

#include "resource.h"
#include "svmMainClient.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class DChoosePath : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    DChoosePath(PWin* parent, dataProject& data, bool first = false, uint resId = IDD_DIALOG_CHOOSE_PATH, HINSTANCE hinstance = 0);
    virtual ~DChoosePath();
  private:
    dataProject& Data;
    dataProject LData;
  public:
    virtual bool create();
    void CmOk();
    void CmCancel();

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  private:
    void checkEnable();
    void checkPartFromStdMsg();
    bool chooseFolder(int idCtrl);
    bool makePath(LPCTSTR path, LPCTSTR sub);
    void makeDirs();
    void manageStdMsg();
    void checkUsedResolution();
    void checkBasePage();
    bool First;
    void checkAll();

    bool notValid;

};
//----------------------------------------------------------------------------
class saveSubPath
{
  public:
    saveSubPath();
    void restore();
  private:
    TCHAR old[_MAX_PATH];
};
//----------------------------------------------------------------------------
class openPageFile : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    openPageFile(PWin* parent, dataProject& data, uint resId = IDD_DIALOG_OPEN_PAGE, HINSTANCE hinstance = 0);
    virtual ~openPageFile();
  private:
    dataProject& Data;
  public:
    virtual bool create();
    void CmOk();
    void CmCancel() { ssPath.restore(); baseClass::CmCancel(); }

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  private:
    void checkEnable();
    void checkType(bool chgFile);

    saveSubPath ssPath;
    void openSource();
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
class getPageFile : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    getPageFile(PWin* parent, LPTSTR target, bool simpleOpen = true, uint resId = IDD_DIALOG_SAVE_PAGE, HINSTANCE hinstance = 0);
    virtual ~getPageFile();
  private:
    LPTSTR Target;
  public:
    virtual bool create();
    void CmOk();
    void CmCancel() { ssPath.restore(); baseClass::CmCancel(); }

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void LB_DblClick();

  private:
    void checkEnable();
    void lbToEdit();
    bool simpleOpen;

    saveSubPath ssPath;
};
//----------------------------------------------------------------------------
class savePageFile : public getPageFile
{
  private:
    typedef getPageFile baseClass;
  public:
    savePageFile(PWin* parent, dataProject& data, bool simpleOpen = false, uint resId = IDD_DIALOG_SAVE_PAGE, HINSTANCE hinstance = 0);
    virtual ~savePageFile();
  private:
    dataProject& Data;
    static TCHAR file[_MAX_PATH];
  public:
    virtual bool create();
    void CmOk();

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  private:
    void enableChange();
    void checkEnableChg();
    void checkType();
};
//----------------------------------------------------------------------------
#endif

