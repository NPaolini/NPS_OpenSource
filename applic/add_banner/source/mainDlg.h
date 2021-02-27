//----------- mainDlg.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef MAINDLG_H_
#define MAINDLG_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <commctrl.h>
#include <stdlib.h>
#include "resource.h"
#include "pModDialog.h"
//----------------------------------------------------------------------------
class manageKey
{
public:
  static int verifyKey();
  static bool evKeyDown(UINT& key);
  static bool evKeyUp(UINT& key);
  static bool evChar(UINT& key);
};
//----------------------------------------------------------------------------
class PDShowVars : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    PDShowVars(PWin* parent, uint resId, HINSTANCE hinstance = 0);
    virtual ~PDShowVars();

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    void searchFile();
    void searchFolder(uint idc);
    void run();
    void loadIni();
    void saveIni();
    void enableDimPhoto();
    void enableDimPhotoByList();
    void enableDimLogo();
    void fillListImage();
    void calcDim(uint idc_from);
    void showDimLogo();
    void calcDimLogo();
    void forceRefreshList();
};
//----------------------------------------------------------------------------
#endif
