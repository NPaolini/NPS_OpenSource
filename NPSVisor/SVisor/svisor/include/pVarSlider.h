//-------- PVarSlider.h --------------------------------------------------------
#ifndef pVarSlider_H_
#define pVarSlider_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef P_BASEOBJ_H_
  #include "p_baseObj.h"
#endif

#include "pstatic.h"
#include "setPack.h"
//----------------------------------------------------------------------------
class PVarSlider;
class objSliderBase;
//----------------------------------------------------------------------------
struct infoSlider
{
  PVarSlider* Owner;
  uint H_W; // nel caso di fullImage indica quanta parte del bitmap deve comunque essere visualizzata
  bool fullImage; // se false il valore corrente è al centro dell'immagine, se true il valore è al bordo interessato
  bool vert; // ruota l'immagine di 90° in senso antiorario
  bool mirror; // l'immagine viene specchiata
  bool reverse; // il verso del movimento è al contrario del valore

  bool readOnly;
  bool updateAlways;

  infoSlider() : Owner(0), H_W(0), fullImage(false), vert(false), mirror(false),
                 reverse(false), readOnly(false), updateAlways(false) {}
  infoSlider(PVarSlider* Owner, uint H_W, bool fullImage, bool vert, bool mirror, bool reverse,
              bool readOnly, bool updateAlways) :
    Owner(Owner), H_W(H_W), fullImage(fullImage), vert(vert), mirror(mirror),
                 reverse(reverse), readOnly(readOnly), updateAlways(updateAlways) {}
};
//----------------------------------------------------------------------------
class PVarSlider : public baseVar
{
  private:
    typedef baseVar baseClass;
  public:
    PVarSlider(P_BaseBody* owner, uint id);

    virtual ~PVarSlider();

    virtual bool allocObj(LPVOID param = 0);
    virtual bool update(bool force);

    bool create();

    void addReqVar(PVect<P_Bits*>& allBits);
  protected:
    virtual uint getBaseId() const { return ID_INIT_VAR_SLIDER; }

  protected:
    virtual void performDraw(HDC hdc);
    void sendCurrData();
    bool reload(bool force = false);
  private:
    objSliderBase* Slider;
    pVariable vMax;
    pVariable vMin;
    double valueMin;
    double valueMax;
    double valueCurr;
    bool canUpdate;
    PBitmap* Image;
    uint dimImage;
    HRGN hrgn;
    uint getdimImg() { return dimImage; }
    void evPaint(HDC hdc);
    friend class objSliderBase;
//    friend class objSliderVert;
//    friend class objSliderHorz;
};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

