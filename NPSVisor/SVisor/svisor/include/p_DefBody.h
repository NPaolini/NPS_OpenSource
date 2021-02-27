//--------P_DefBody.h --------------------------------------------------------
#ifndef P_DEFBODY_H_
#define P_DEFBODY_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef P_BODY_H_
  #include "p_Body.h"
#endif
#ifndef MAINCLIENT_H_
  #include "mainClient.h"
#endif
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
class P_DefBody : public P_Body {
  public:
    P_DefBody(PWin* parent, LPCTSTR pageDescr);
    P_DefBody(int idParent, PWin* parent, uint resId, HINSTANCE hInstance = 0);

    virtual ~P_DefBody();

    // cerca nel set di stringhe se è definita una azione.
    // Riferirsi alle specifiche per le azioni implementate.
    virtual P_Body* pushedBtn(int idBtn);

    virtual P_Body* getPreviousPage();

    virtual P_BaseBody::preSend preSendEdit(PVarEdit*, prfData&);
    virtual bool preSendBtn(baseActive*, prfData&);
    virtual void postSendEdit(PVarEdit*);

    LPCTSTR getPageName() const { return pageName; }
    void setPageName(LPCTSTR pageDescr);

    enum {  ID_OPEN_PAGE = 1,
            ID_SEND_ALL,            //  2
            ID_RECEIVE_ALL,         //  3
            ID_EXPORT_DATA,         //  4
            ID_SEND_DATA,           //  5
            ID_OPEN_TREND,          //  6
            ID_OPEN_RECIPE,         //  7
            ID_OPEN_RECIPE_ROW,     //  8
            ID_OPEN_MODALBODY,      //  9
            ID_CHANGE_PSW,          // 10
            ID_OPEN_MODELESSBODY,   // 11
            // nuovi codici
            ID_GOTO_PREVIOUS_PAGE,  // 12
            ID_SHUTDOWN,            // 13
            ID_SHOW_TREEVIEW,       // 14
            ID_PRINT_TREEVIEW,      // 15
            ID_SHOW_ALARM,          // 16
            ID_PRINT_SCREEN,        // 17
            ID_OPEN_MAINT,          // 18

            ID_CUSTOM_ACT,          // 19
            ID_OPEN_RECIPE_LB,      // 20

            ID_TOGGLE_BIT,          // 21
            ID_SPIN_UP_DOWN,        // 22
            ID_SEND_CTRL_ACTION     // 23

           };
    virtual void refreshBody();

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void action_Btn(uint idBtn);
    virtual void openModal(LPCTSTR p);
    virtual void openModeless(LPCTSTR p, bool needOpen);
    virtual void openKeyboard(LPCTSTR p, HWND hwTarget);

    void postActionBtnWithPsw(LPCTSTR p, UINT code);
    void postCTRL_ActionBtnWithPsw(LPCTSTR p, UINT code);

    void getFileStr(LPTSTR path);

    // torna false se va ricaricato, true se i dati non sono più vecchi della pagina
    virtual bool getCacheFile(LPTSTR target, uint sz);

//    void getFileStrCheckRemote(LPTSTR target, LPCTSTR file);

    virtual LPCTSTR findChildPage(int idBtn, int& idPar, int& pswLevel, bool& predefinite);
    void Send(LPCTSTR str);
    void Receive(uint idprf);
    void performExport(LPCTSTR p);
    void SendData(LPCTSTR p, bool toggle);

    bool hasPermission(int pswLevel, bool& rd);

//    void setPageName(LPCTSTR pageDescr);

#if 0
    // l'apertura del modal è stata provocata dal gestData che ha settato un bit
    // per evitare il rientro. E' necessario azzerare quel bit alla chiusura del
    // modale.
    void resetModalBit();
#endif
    // per evitare che non venga impostato nel gestData, viene settato
    // internamente e si avvia il modale solo se non era già impostato.
    // Al termine viene poi resettato
    bool setModalBit(bool set, uint bit);

    void sendSpin(LPCTSTR p);

  private:
    LPTSTR pageName;

    typedef P_Body baseClass;
};
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif

