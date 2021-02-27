//------------------- svmObjAlarm.h -------------------------
//-----------------------------------------------------------
#ifndef svmObjAlarm_H_
#define svmObjAlarm_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmObject.h"
#include "svmPropertyAlarm.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmObjAlarm : public svmObject
{
  private:
    typedef svmObject baseClass;
  public:
    svmObjAlarm(PWin* par, int id, int x1 = DATA_NOT_USED, int y1 = DATA_NOT_USED,
            int x2 = DATA_NOT_USED, int y2 = DATA_NOT_USED);
    svmObjAlarm(PWin* par, int id, const PRect& r);

    virtual svmObjHistory::typeOfAction dialogProperty(bool onCreate = true);
    virtual smartPointerConstString getTitle() const;

    virtual bool save(P_File& pfCript, P_File& pfClear, uint order);
    virtual bool load(uint id, setOfString& set);
    virtual smartPointerConstString getTitle4Prop() const { return smartPointerConstString(_T("Listbox-Alarm"), 0); }

    Property* allocProperty();

  protected:
    virtual void DrawObject(HDC hdc);
    virtual void DrawTitle(HDC hdc);
    virtual PRect getRectTitle(HDC hdc, LPCTSTR title) const;

    virtual svmObject* makeClone();
    virtual void Clone(const svmObject& other);
    virtual void addInfoAd4Other(manageInfoAd4& set);
  private:
    void init();
};
//-----------------------------------------------------------
//-----------------------------------------------------------
#endif
