//------------ footer.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef FOOTER_H_
#define FOOTER_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef PBUTTON_H_
  #include "pbutton.h"
#endif

#ifndef MAINCLIENT_H_
  #include "mainclient.h"
#endif

#include "id_btn.h"

#ifndef P_BASE_H_
  #include "p_base.h"
#endif
//----------------------------------------------------------------------------
#include "setPack.h"

#define SECOND_OF_TIME_STATUS 20
//----------------------------------------------------------------------------
#define TIME_STATUS (SECOND_OF_TIME_STATUS * (1000/TIME_SLICE))
//----------------------------------------------------------------------------
class Footer : public P_Base {
  public:
    Footer(class mainClient* parent, uint resId = IDD_FOOTER, HINSTANCE hinstance = 0);
    virtual ~Footer();
    void refresh();
    int Height();

    int trueHeight();

    void setInfoBtn(setOfString& set, long idInit);

    void setMsgStatus(LPCTSTR status, int wait = TIME_STATUS);
    void setVersion(DWORD applVersion, DWORD bodyVersion);
    void enableBtn(int idBtn, bool enable = true);
    bool isEnabledBtn(int idBtn);

    virtual bool create();
    bool idle(DWORD count);

    void setNewText(int idBtn, LPCTSTR newText);

    void setVisual(forceVis type);
  protected:
    virtual LRESULT windowProc(HWND hwnd,
                            UINT message,
                            WPARAM wParam,
                            LPARAM lParam);

  private:
    enum tRefresh { rStatus = 1, rInfo, rAll };

    PButton *Btn[MAX_BTN];
    class PTextFixedPanel *Txt[MAX_BTN];
    class PTextFixedPanel *PStatus;
    class PTextFixedPanel *PVersion;
    HFONT Font;
    HFONT FontStat;
    int showAlert;
    int H_Btn;
    int H_Txt;
    int H_Stat;
    void makeRefresh(tRefresh type);
    void BnClicked(int btn);
    void evPaint(HDC hdc);

    void makeAll();

    DWORD ForceVis;
};
//----------------------------------------------------------------------------
extern  Footer *allocFooter(class mainClient *parent);
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif

