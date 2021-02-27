//------------ advBtnDlgImpl.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef advBtnDlgImpl_H_
#define advBtnDlgImpl_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdlib.h>
#include "resource.h"

#include "pDialog.h"
#include "pListBox.h"
//----------------------------------------------------------------------------
#include "baseAdvBtnDlg.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
struct colorSet
{
  COLORREF fg;
  COLORREF bg;
  colorSet() : fg(RGB(0, 0, 0)), bg(RGB(255, 255, 255)) {}
};
//----------------------------------------------------------------------------
class lbColor : public PListBox
{
  private:
    typedef PListBox baseClass;
  public:
    lbColor(PWin* parent, uint id) : baseClass(parent, id) {}
  protected:
    HPEN setColor(HDC hDC, DRAWITEMSTRUCT FAR& drawInfo);
    void paintCustom(HDC hDC, PRect& r, int ix, DRAWITEMSTRUCT FAR& drawInfo);
};
//----------------------------------------------------------------------------
class advBtnDlgText : public baseAdvBtnDlg
{
  private:
    typedef baseAdvBtnDlg baseClass;
  public:
    advBtnDlgText(PWin* parent, PropertyBtn& prop, uint resId, HINSTANCE hinstance = 0) :
      baseClass(parent, prop, resId, hinstance) {}
    virtual ~advBtnDlgText() { destroy(); }

    bool create();
    virtual void saveData();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual int makeRow(LPTSTR target, uint sz);
    virtual int unmakedRow(LPTSTR target);
    virtual uint getCheck() { return Prop.Flags.textByVar; }
    virtual const fullPrph& getDataPrf() { return Prop.DataPrf[1]; }
};
//----------------------------------------------------------------------------
class advBtnDlgBmp : public baseAdvBtnDlg
{
  private:
    typedef baseAdvBtnDlg baseClass;
  public:
    advBtnDlgBmp(PWin* parent, PropertyBtn& prop, uint resId, HINSTANCE hinstance = 0) :
      baseClass(parent, prop, resId, hinstance) { nameMod[0] = 0; }
    virtual ~advBtnDlgBmp() { destroy(); }

    bool create();
    virtual void saveData();
    virtual void changedPosBmp(uint selected);
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual int makeRow(LPTSTR target, uint sz);
    virtual int unmakedRow(LPTSTR target);
    virtual uint getCheck() { return Prop.Flags.bitmapByVar; }
    virtual const fullPrph& getDataPrf() { return Prop.DataPrf[2]; }
    TCHAR nameMod[_MAX_PATH];
};
//----------------------------------------------------------------------------
class advBtnDlgColor : public baseAdvBtnDlg
{
  private:
    typedef baseAdvBtnDlg baseClass;
  public:
    advBtnDlgColor(PWin* parent, PropertyBtn& prop, uint resId, HINSTANCE hinstance = 0) :
      baseClass(parent, prop, resId, hinstance), bgSample(0) {}
    virtual ~advBtnDlgColor() { destroy(); if(bgSample) DeleteObject(bgSample); }

    bool create();
    virtual void saveData();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);
    void setColor(uint ix);
    virtual int makeRow(LPTSTR target, uint sz);
    virtual int unmakedRow(LPTSTR target);
    virtual uint getCheck() { return Prop.Flags.colorByVar; }
    virtual const fullPrph& getDataPrf() { return Prop.DataPrf[0]; }
    HBRUSH bgSample;
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#endif
