//-------- PVarBmp.h ---------------------------------------------------------
#ifndef PVARBMP_H_
#define PVARBMP_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef P_BASEOBJ_H_
  #include "p_baseObj.h"
#endif
//----------------------------------------------------------------------------
#define ADD_ID_4_MOVIM 900
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
class PBitmap;
//----------------------------------------------------------------------------
class PVarBmp : public baseVar
{
  private:
    typedef baseVar baseClass;
  public:
    enum bmpStyle {
        fixed,  // un solo bitmap, show/hide
        on_off, // due bitmap, show1/show2
        fixed_lamp, // quando attivo, lampeggia
        dual_lamp,  // quando attivo alterna i due bitmap

        sequence,   // sequenza di bitmap, la variabile associata indica
                    // quale bitmap visualizzare

        anim,       // animazione fissa, cambia bitmap ad ogni ciclo
        animVar,    // animazione in dipendenza della variabile

        lastCode = animVar,

        // vanno sommati
        negative = 1000, // se attivo inverte il significato del lampeggio
        noTransp = 10000,

        // valido per l'animazione con variabile associata
        hideAnim = 100000, // se il bit è attivo visualizza l'animazione, altrimenti nasconde
        enableMove = 1000000,
        maxMul = enableMove
        };

    PVarBmp(P_BaseBody* owner, uint id);

    ~PVarBmp();

    virtual bool allocObj(LPVOID param = 0);
    virtual bool update(bool force);

    void setId(uint newId) { baseObj::setId(newId); }

    bmpStyle getStyle() const { return (bmpStyle)Style; }

    // usata per l'alternanza, per la sequenza e per l'animazione,
    // val indica il numero di sequenza
    void performAltern(DWDATA val = 0);

    // nel caso di un solo bitmap effettua lo show/hide,
    // con due bitmap si scambia tra il primo ed il secondo
    // in caso di lampeggio inizia/termina
    void Set();
    void Reset();

//    virtual void Draw(HDC hdc);
//    virtual void Paint(HDC hdc);

    struct infoData {
      prfData data;
      uint prph;
      uint norm;
      };
    virtual const PRect& get_Rect() const;

//    virtual void addReqVar(PVect<P_Bits*>& allBits);

  protected:
    void performDraw(HDC hdc);
    virtual uint getBaseId() const { return ID_INIT_VAR_BMP; }

  private:
    DWORD Style;
    PBitmap** aBmp;
    PVect<PBitmap*> localBmp;
    class PSeqTraspBitmap* Bmp;

    enum state { sOff, sShow, sHide };
    state currState;

    void toggleState();

    struct effect {
      DWORD neg     : 1;
      DWORD noTrsp  : 1;
      DWORD hide    : 1;
      DWORD move    : 1;
      effect() : neg(0), noTrsp(0), hide(0), move(0) {}
      } Effect;
/*
    // viene allocata solo se necessario
#define MAX_MOVING_DATA 6
    struct moving_data {
      PRect rect;
      infoData Data[MAX_MOVING_DATA];
      POINT last;
      uint dir;
      } *movingData;

    void updateMoving();
    void allocMovingData(LPCTSTR p);
    void fillData(infoData& data, int code);
*/
    bool needRefresh;

    DWORD timeCycle;
    DWORD lastTime;
};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

