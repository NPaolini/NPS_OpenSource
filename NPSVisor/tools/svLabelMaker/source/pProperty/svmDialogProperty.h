//--------------- svmDialogProperty.h -----------------------------
//-----------------------------------------------------------
#ifndef svmDialogProperty_H_
#define svmDialogProperty_H_
//-----------------------------------------------------------
#include "precHeader.h"
#include "svmRectProperty.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmBaseDialogProperty : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    svmBaseDialogProperty(class svmObject* owner, Property* prop, PWin* parent, uint id, HINSTANCE hInst = 0) :
      baseClass(parent, id, hInst), Owner(owner), Prop(prop), tmpProp(0) {  }
    ~svmBaseDialogProperty();

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType) { return 0; }
    virtual void CmOk();
    Property* Prop;
    Property* tmpProp;
    class svmObject* Owner;

    // per creare tmpProp
    virtual Property* allocProperty();
};
//-----------------------------------------------------------
class  svmLineDialogProperty : public svmBaseDialogProperty
{
  private:
    typedef svmBaseDialogProperty baseClass;
  public:
    svmLineDialogProperty(class svmObject* owner, Property* prop, PWin* parent, uint id, HINSTANCE hInst = 0) :
      baseClass(owner, prop, parent, id, hInst), hb_line(0), alsoNoLine(false)  {  }
    ~svmLineDialogProperty();

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);
    virtual void CmOk();

    HBRUSH hb_line;
    COLORREF c_line;
    virtual void checkCbChange(uint idc);
    virtual void checkTickness();
    bool alsoNoLine;
};
//-----------------------------------------------------------
class  svmDrawDialogProperty : public svmLineDialogProperty
{
  private:
    typedef svmLineDialogProperty baseClass;
  public:
    svmDrawDialogProperty(class svmObject* owner, Property* prop, PWin* parent, uint id, HINSTANCE hInst = 0) :
      baseClass(owner, prop, parent, id, hInst), hb_bkg(0)  { alsoNoLine = true; }
    ~svmDrawDialogProperty();

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);
    virtual void CmOk();
    void checkCbChange(uint idc);

    HBRUSH hb_bkg;
    COLORREF c_bkg;
};
//-----------------------------------------------------------
class  svmRectDialogProperty : public svmDrawDialogProperty
{
  private:
    typedef svmDrawDialogProperty baseClass;
  public:
    svmRectDialogProperty(class svmObject* owner, Property* prop, PWin* parent, uint id = IDD_DIALOG_RECT_PROP, HINSTANCE hInst = 0) :
      baseClass(owner, prop, parent, id, hInst)  {  }
    ~svmRectDialogProperty() { destroy(); }

    virtual bool create();
  protected:
    virtual void CmOk();
};
//-----------------------------------------------------------
class  svmTextDialogProperty : public svmBaseDialogProperty
{
  private:
    typedef svmBaseDialogProperty baseClass;
  public:
    svmTextDialogProperty(class svmObject* owner, Property* prop, PWin* parent, uint id, HINSTANCE hInst = 0) :
      baseClass(owner, prop, parent, id, hInst), hb_frg(0), hb_bkg(0)  {  }
    ~svmTextDialogProperty();

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);
    virtual void CmOk();

    HBRUSH hb_frg;
    COLORREF c_frg;
    HBRUSH hb_bkg;
    COLORREF c_bkg;
    virtual void chooseFont();
    virtual void checkAlignType();
    virtual void checkType();
    virtual void chooseVar();

};
//-----------------------------------------------------------
class  svmBitmapDialogProperty : public svmBaseDialogProperty
{
  private:
    typedef svmBaseDialogProperty baseClass;
  public:
    svmBitmapDialogProperty(class svmObject* owner, Property* prop, PWin* parent, uint id, HINSTANCE hInst = 0) :
      baseClass(owner, prop, parent, id, hInst)  {  }
    ~svmBitmapDialogProperty();

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();

    virtual void chooseBmp();
};
//-----------------------------------------------------------
#endif
