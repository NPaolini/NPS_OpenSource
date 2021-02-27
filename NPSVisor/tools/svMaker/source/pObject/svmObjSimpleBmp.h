//-------------------- svmObjSimpleBmp.h --------------------
//-----------------------------------------------------------
#ifndef SVMOBJSIMPLEBMP_H_
#define SVMOBJSIMPLEBMP_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmObject.h"
#include "svmPropertySimpleBmp.h"
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmObjSimpleBmp : public svmObject
{
  private:
    typedef svmObject baseClass;
  public:
    svmObjSimpleBmp(PWin* par, int id, int x1 = DATA_NOT_USED, int y1 = DATA_NOT_USED,
            int x2 = DATA_NOT_USED, int y2 = DATA_NOT_USED);
    svmObjSimpleBmp(PWin* par, int id, const PRect& r);

    virtual svmObjHistory::typeOfAction dialogProperty(bool onCreate = true);
    virtual int getIdType() const;
    virtual smartPointerConstString getTitle() const;

    virtual bool save(P_File& pfCript, P_File& pfClear, uint order);
    virtual bool load(uint id, setOfString& set);
    virtual smartPointerConstString getTitle4Prop() const { return smartPointerConstString(_T("Immagine di sfondo"), 0); }
    virtual void fillTips(LPTSTR tips, int size);

    Property* allocProperty();
    bool useFont() const { return false; }
  protected:
    virtual void DrawObject(HDC hdc);
    virtual PRect getRectTitle(HDC hdc, LPCTSTR title) const;

    virtual svmObject* makeClone();
    virtual void Clone(const svmObject& other);

    virtual void DrawTitle(HDC hdc);
  private:
    void init();
};
//-----------------------------------------------------------
#endif
