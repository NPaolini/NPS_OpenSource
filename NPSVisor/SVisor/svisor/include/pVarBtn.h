//-------- PVarBtn.h ---------------------------------------------------------
#ifndef PVARBTN_H_
#define PVARBTN_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef P_BASEOBJ_H_
  #include "p_baseObj.h"
#endif

//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
enum fBtn {
  bStd,   // pulsante standard: al rilascio si risolleva

  bCheck, // simile ad un checkBox: un click lo preme,
          // un successivo lo rilascia

  bInitRadio, // simile ad un radioButton: fa parte di un gruppo e
              // l'attivazione di uno rilascia tutti gli altri

  bRadio, // intermedio/termina il gruppo

  bPress, // premuto attiva il bit, al rilascio lo disattiva

  bHided, //

  bActionModeless, // apre/chiude una finestra non modale

  bActionOpenPage, // apre una pagina da bit e resetta il bit
  };
//----------------------------------------------------------------------------
enum fShowBtn {
  sOnlyBmp,
  sBmpAndText,
  sStdAndBmp,
  sNewStdAndBmp,
  };
//----------------------------------------------------------------------------
typedef PVect<COLORREF> vColor;
//----------------------------------------------------------------------------
class PVarBtn : public baseActive
{
  private:
    typedef baseActive baseClass;
  public:
    struct style {
      fBtn type;
      fShowBtn show;
      style() : type(bStd), show(sOnlyBmp) {}
      };

    PVarBtn(P_BaseBody* owner, uint id);
    virtual ~PVarBtn();

    virtual bool allocObj(LPVOID param = 0);
    virtual bool update(bool force);

    virtual bool makeToSend(prfData& data, bool toggle);

//  protected:
    virtual HWND getHwnd() { return Btn ? Btn->getHandle() : 0; }

    bool setFocus(pAround::around where) { Around.setFocus(where); return true; }

    bool isSetOnPress() const { return bPress == Style.type; }
    bool needReleasePress();

    virtual void addReqVar(PVect<P_Bits*>& allBits);
  protected:
    virtual uint getBaseId() const { return ID_INIT_VAR_BTN; }
    bool isPressed();
  private:
    POwnBtn* Btn;

   struct Color {
      COLORREF bkg;
      COLORREF txt;
      };
    // se pulsante standard si memorizzano qui gli eventuali altri
    // bitmap da scambiare e i corrispondenti testi, oltre a
    // quelli di default
    PVect<uint> idText;
    PVect<uint> idBmp;
    PVect<Color> idColor;

    PVect<PBitmap*> Bmp;
    PVect<PBitmap*> localBmp;

    uint CurrText;
    uint CurrBmp;
    style Style;
//    uint count4Changed;

    bool Transparent;

    virtual void set_Text(LPCTSTR txt) { Btn->setCaption(txt); }

    void setCurrText(uint pos);
    void setCurrBmp(uint pos);
    void setCurrColor(uint pos);
    uint getCurrText() const { return CurrText; }
    uint getCurrBmp() const { return CurrBmp; }
    const style& getStyle() { return Style; }

    void setCurrAll(uint pos);

    bool isSelected();
    void setSel(bool set);
    void setSelOnlyStyle(bool set);
    bool isValid() { return toBool(Btn || bHided == Style.type); }
/*
    void resetChanged();
    void setChanged();
    bool isChanged();
*/
    bool makeVarBtn(const vColor& fg, const vColor& bkg,
                    const PVect<uint>& id_bmp, uint idFont, cntGroup* cnt);

    friend class pTouchKeyb;

    // variabili controllo nuovi stili
    pVariable v_Text;
    pVariable v_Color;
    pVariable v_Bitmap;
    union {
      struct {
        DWORD theme : 1;
        DWORD flat : 1;
        DWORD fixedBmpDim : 1; // non più usato, lasciato per compatibilità
        DWORD pos   : 4;
        // i valori seguenti possono essere: 0 = non usa var, 1 = usa indice, 2 = usa valore, 3 = valore esatto
        DWORD colorByVar : 3;
        DWORD textByVar : 3;
        DWORD bitmapByVar : 3;
        DWORD noBorder : 1;
        DWORD styleBmpDim : 2; // 0 -> fisso, 1 -> scalato, 2 -> riempie tutto il pulsante (valido solo per testo/bmp centrato)
        };
      DWORD flag;
      } Flags;
    enum bmpPos { bpLeft, bpTop, bpRight, bpBottom  };
    baseVarColor VarColors;
    struct valPos { double Val; };
    PVect<valPos> valText;
    PVect<valPos> valBmp;

    void makeValuePos(LPCTSTR p, PVect<valPos>& val);
    int posFromData(pVariable& var, PVect<valPos>& val, DWORD flag, bool force);
    void appendIdText(uint idInit, PVect<uint>& text);
    void checkIfSpin();

    int openedModeless;
};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

