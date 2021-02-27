//-------------------- svmPropertyBmp.h --------------------
//-----------------------------------------------------------
#ifndef SVMPROPERTYBMP_H_
#define SVMPROPERTYBMP_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "svmDefObj.h"
//#include "svmObjBmp.h"
//-----------------------------------------------------------
#define useMove(a) LOWORD((a)->type2)
#define useShowHide(a) HIWORD((a)->type2)
//-----------------------------------------------------------
#define setUseMove(a) ((a)->type2 |= 1)
#define setUseShowHide(a) ((a)->type2 |= (1 << 16))
//-----------------------------------------------------------
/*
class bmpMoveInfo
{
  public:
    bmpMoveInfo() : dir(0) {}
    bmpMoveInfo(const bmpMoveInfo& other);
    const bmpMoveInfo& operator =(const bmpMoveInfo& other);

    struct varInfo
    {
      UINT perif;
      UINT addr;
      UINT typeVal;
      int normaliz;

      varInfo() : perif(0), addr(0), typeVal(0), normaliz(0) {}
    };

    enum var {
      bmi_minX,
      bmi_minY,
      bmi_maxX,
      bmi_maxY,
      bmi_currX,
      bmi_currY,
      MAX_MOVE_INFO
      };

//  private:
    PRect rect;
    varInfo vI[MAX_MOVE_INFO];

    int dir;
};
*/
#include "svmObjBmp.h"
//-----------------------------------------------------------
// type1 della classe base viene usato per il flag
//-----------------------------------------------------------
class PropertyBmp : public Property
{
  private:
    typedef Property baseClass;
  public:
    PropertyBmp();
    virtual ~PropertyBmp();

    const PropertyBmp& operator=(const PropertyBmp& other) { clone(other);  return *this; }
    const Property& operator=(const Property& other) { clone(other);  return *this; }

    // copia solo un set minimo di proprietà, usata durante la creazione di
    // nuovi oggetti
    void cloneMinusProperty(const Property& other);
    const PVect<LPCTSTR>& getNames() const { return nameBmp; }
    PVect<LPCTSTR>& getNames() { return nameBmp; }

//    bmpMoveInfo* mInfo;
//    int moveCode[MAX_ELEN_MOVE];

    DWORD timeCycle;
  protected:
    virtual void clone(const Property& other);

  private:
    PropertyBmp(const PropertyBmp& other);
    PVect<LPCTSTR> nameBmp;
};
//-----------------------------------------------------------
//-----------------------------------------------------------
enum bmpStyle {
  bFixed,
  bOnOff,
  bFixedLamp,
  bDualLamp,
  bVarSeq,
  bFixSeq,
  bAnimSeq,
  };
//-----------------------------------------------------------
class svmDialogBmp : public svmBaseDialogProperty
{
  private:
    typedef svmBaseDialogProperty baseClass;
  public:
    svmDialogBmp(svmObject* owner, Property* prop, PWin* parent, uint id = IDD_BMP_PROPERTY, HINSTANCE hInst = 0) :
      baseClass(owner, prop, parent, id, hInst), Bmp(0) {  }
    ~svmDialogBmp()    {   destroy();  delete Bmp; }

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    void fillType();
    void chooseBmp();
    //void movingData();
    void checkEnabled();

    void allocBmp();
    void checkHeight();

    PBitmap* Bmp;
};
//-----------------------------------------------------------
#endif
