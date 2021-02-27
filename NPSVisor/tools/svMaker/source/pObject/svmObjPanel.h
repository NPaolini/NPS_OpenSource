//------------------- svmObjPanel.h -------------------------
//-----------------------------------------------------------
#ifndef SVMOBJPANEL_H_
#define SVMOBJPANEL_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmObject.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmObjPanel : public svmObject
{
  private:
    typedef svmObject baseClass;
  public:
    svmObjPanel(PWin* par, int id, int x1 = DATA_NOT_USED, int y1 = DATA_NOT_USED,
            int x2 = DATA_NOT_USED, int y2 = DATA_NOT_USED);
    svmObjPanel(PWin* par, int id, const PRect& r);

    virtual svmObjHistory::typeOfAction dialogProperty(bool onCreate = true);
    virtual smartPointerConstString getTitle() const;

    virtual bool save(P_File& pfCript, P_File& pfClear, uint order);
    virtual bool load(uint id, setOfString& set);
    virtual uint getStyle() const;

    virtual void fillTips(LPTSTR tips, int size);
    virtual Property* allocProperty() { return new Property; }
    bool useFont() const { return false; }
  protected:
    virtual void DrawObject(HDC hdc);
    virtual PRect getRectTitle(HDC hdc, LPCTSTR title) const;

    virtual svmObject* makeClone();
    virtual void Clone(const svmObject& other);
  private:
    void init();
};
//-----------------------------------------------------------
//-----------------------------------------------------------
inline smartPointerConstString svmObjPanel::getTitle() const
{
  return smartPointerConstString(_T("Pannello"), 0);
}
//-----------------------------------------------------------
#endif
