//----------- show_edit.h ----------------------------------------------------
//----------------------------------------------------------------------------
#ifndef show_edit_H_
#define show_edit_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "pAl_common.h"
#include "plistbox.h"
//----------------------------------------------------------------------------
#define bkg_Color1 0xC1E1D8
#define bkg_Color2 0xD8E1C1
//----------------------------------------------------------------------------
class genShow : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    genShow(PWin* parent, LPTSTR buff, uint idd, COLORREF color);
    ~genShow();

  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);
    LPTSTR Buff;
    COLORREF Color;
    HBRUSH Brush;
};
//----------------------------------------------------------------------------
class ShowType : public genShow
{
  private:
    typedef genShow baseClass;
  public:
    ShowType(PWin* parent, LPTSTR buff) :
      baseClass(parent, buff, IDD_DIALOG_SHOW_TYPE, bkg_Color1) {}

    ~ShowType()
    {
      destroy();
    }

    bool create();
  protected:
    void CmOk();
};
//----------------------------------------------------------------------------
class ShowAssocType : public genShow
{
  private:
    typedef genShow baseClass;
  public:
    ShowAssocType(PWin* parent, LPTSTR buff) :
      baseClass(parent, buff, IDD_DIALOG_SHOW_ASSOC_TYPE, bkg_Color2) {}

    ~ShowAssocType()
    {
      destroy();
    }

    bool create();
  protected:
    void CmOk();
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#endif
