//----------------svmObjButton.h ----------------------------
//-----------------------------------------------------------
#ifndef SVMOBJBUTTON_H_
#define SVMOBJBUTTON_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmObject.h"
#include "svmPropertyBtn.h"
#include "pOwnBtnImageStd.h"
//-----------------------------------------------------------
class svmObjButton : public svmObject
{
  private:
    typedef svmObject baseClass;
  public:
    svmObjButton(PWin* par, int id, int x1 = DATA_NOT_USED, int y1 = DATA_NOT_USED,
            int x2 = DATA_NOT_USED, int y2 = DATA_NOT_USED);
    svmObjButton(PWin* par, int id, const PRect& r);
    ~svmObjButton();
    virtual svmObjHistory::typeOfAction dialogProperty(bool onCreate = true);

    virtual bool save(P_File& pfCript, P_File& pfClear, uint order);
    virtual bool load(uint id, setOfString& set);
    virtual smartPointerConstString getTitle4Prop() const { return smartPointerConstString(_T("Pulsante"), 0); }

    // N.B. deve essere richiamata subito dopo la creazione dell'oggetto,
    // non effettua controlli sui text precedenti
    void makeDef4Cam(uint prf, uint addr, uint offsBit, LPCTSTR released, LPCTSTR pressed);

    virtual void setOffsetAddr(const infoOffsetReplace& ior);
    Property* allocProperty();
  protected:
    virtual void DrawObject(HDC hdc);
    virtual void DrawRectTitle(HDC hdc, LPCTSTR title, PRect rect);
    virtual PRect getRectTitle(HDC hdc, LPCTSTR title) const;
    void drawFocus(HDC hdc);
    virtual smartPointerConstString getTitle() const;

    virtual svmObject* makeClone();
    virtual void Clone(const svmObject& other);
    void addInfoAd4Base(manageInfoAd4& set);
    void fillTips(LPTSTR tips, int size);
  private:
    void init();
    int offsXText;
    HTHEME hTheme;
    void loadTheme();
    bool isThemed() const { return toBool(hTheme); }

    LPTSTR chgFunctionData(LPCTSTR source, bool forPrph, const infoOffsetReplace& ior);
};
//-----------------------------------------------------------
#endif
