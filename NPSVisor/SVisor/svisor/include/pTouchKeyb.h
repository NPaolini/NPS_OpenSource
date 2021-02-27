//--------pTouchKeyb.h ------------------------------------------------------
#ifndef pTouchKeyb_H_
#define pTouchKeyb_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "p_ModalBody.h"
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
class pTouchKeyb : public P_ModalBody
{
  public:
    pTouchKeyb(PWin* parent, LPCTSTR pageDescr, HWND hwTarget, bool& setup);
    pTouchKeyb(int idParent, PWin* parent, uint resId, HWND hwTarget, bool& setup, HINSTANCE hInstance = 0);

    virtual ~pTouchKeyb();

    bool create();

    P_Body* pushedBtn(int idBtn);

    virtual void refresh();
    virtual bool preProcessMsg(MSG& msg);
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual LPCTSTR findChildPage(int idBtn, int& idPar, int& pswLevel, bool& predefinite);

    void action_Btn(uint idBtn);
  private:
    HWND hwTarget;
    typedef P_ModalBody baseClass;

    void init();

    void sendChar(PVarBtn* pBtn, uint addr);
    void postSetup();
    void toggleStatus(PVarBtn* pBtn, uint addr);
    void performSendChar(uint addr);
    void saveData();

    #define MAX_TOGGLE_KEY 3

    struct stat_toggle {
      bool stat;
      uint key;
      };
    stat_toggle StatusModifier[MAX_TOGGLE_KEY];

    uint oldSel;
    bool& Setup;
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

