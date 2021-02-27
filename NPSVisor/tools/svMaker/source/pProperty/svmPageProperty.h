//------- svmPageProperty.h --------------------------------------------------
//----------------------------------------------------------------------------
#ifndef SVMPAGEPROPERTY_H_
#define SVMPAGEPROPERTY_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef PMODDIALOG_H_
#include "pModDialog.h"
#endif

#include "resource.h"
#include "svmMainClient.h"
#ifndef MAX_BTN
  #define MAX_BTN 12
#else
  #if (MAX_BTN != 12)
    #error define errata
  #endif
#endif
#define MAX_LABEL 256
#define MAX_RESULT _MAX_PATH
//----------------------------------------------------------------------------
/*
-> 1 -> apre nuova pagina, segue valore psw, segue id (0 per pagine generiche),
        infine il nome del file descrittore
        se non è una pagina generica NON deve seguire il nome del file.
-> 2 -> spedisce tutti i dati, segue valore psw, segue id della periferica
-> 3 -> rilegge tutti i dati, come [2]
-> 4 -> esegue l'export dei dati, segue valore psw, segue zero per dati correnti,
        diverso da zero per storico
-> 5 -> Spedisce un valore, segue valore psw, segue id di un pulsante. Il pulsante
        può anche essere non visibile usando il tipo comando [5] della specifica dei pulsanti
-> 6 -> Apre una pagina di trend, segue valore psw, infine il nome del file descrittore
-> 7 -> Apre una pagina di ricette da file, segue valore psw, infine il nome del file descrittore
-> 8 -> Apre una pagina di righe di ricette, segue valore psw, infine il nome del file descrittore
-> 9 -> Apre un body modale, segue valore psw, infine il nome del file descrittore

->10 -> cambio psw
->11 -> apre modeless (non usato)

->12 -> pagina precedente
->13 -> fine programma
->14 -> visualizza operatori/cause stop
->15 -> stampa operatori/cause stop
->16 -> pagina allarmi
->17 -> print screen

->18 -> custom action
->19 -> ricetta con listbox
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

*/
enum btn_Action {
//enum btnAction {
  aNoAction,
  aNewPage = 1,
  aSendAll,
  aReceiveAll,
  aExport,
  aSendOne,
  aOpenTrend,
  aOpenfileRecipe,
  aOpenRowRecipe,
  aOpenModBody,

  aChgPsw,
  aOpenModal, // non usato

  aPreviousPage,
  aShutdown,
  aShowTree,
  aPrintTree,
  aShowAlarm,
  aPrintScreen,

  aOpenMaint,

  aCustomAction,
  aOpenfileRecipeDan,

  aSendToggle,

  aSendSpin,

  aCTRL_Action,

  };
//----------------------------------------------------------------------------
extern bool getChooseAction(PWin* parent, LPTSTR buff, size_t lenBuff, uint code, bool& enableBtn);
extern bool isChangePage(LPCTSTR row);
extern bool isSendOneData(LPCTSTR row);
extern bool isSendToggleData(LPCTSTR row);

extern LPCTSTR getTextAction(uint sel);

extern bool isChgPsw(LPCTSTR row);
extern int getChgPswCode();

extern bool isKeyb4TouchScreen(LPCTSTR row);
extern int getKeyb4TouchScreenCode();

extern int  getSendOneDataCode();
extern int  getSendToggleDataCode();
extern void fillCBAction(HWND hwnd, int sel);
extern void fillCBActionOpenPage(HWND hwnd, int sel);
extern void fillMenuOpenPage(menuPopup* menu, int nElem, int sel);
extern uint getNumOpenPage();
extern int getIxActionByOpenPage(int sel);

extern LPCTSTR getPsw(LPCTSTR buff, int& psw, bool prefix);
extern LPTSTR setPsw(LPTSTR buff, int psw, bool prefix);
//----------------------------------------------------------------------------
extern int setSendOneByBtn(LPCTSTR row, int allowedBtn, P_File& pf);
#define setSendToggleByBtn setSendOneByBtn

extern LPTSTR getSendOneByBtn(LPCTSTR row, setOfString& set);
extern LPTSTR getSendToggleByBtn(LPCTSTR row, setOfString& set);
//----------------------------------------------------------------------------
class svmPageProperty : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    svmPageProperty(PWin* parent, setOfString& set, uint resId = IDD_PAGE_PROPERTY, HINSTANCE hinstance = 0);
    virtual ~svmPageProperty();

    virtual bool create();
    void CmOk();

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);

  private:
    HBRUSH bkgEdit;
    setOfString& Set;
    int oldSel[MAX_BTN];

    void checkEnable();
    void setAction(LPCTSTR p, int pos);
    void fillAction();

//    LPCTSTR getPsw(LPCTSTR buff, int& psw);
//    LPTSTR setPsw(LPTSTR buff, int psw);

    void setBtnLabel(int ix, int btn, LPCTSTR row);
    void setNewPage(int ix, int btn, LPCTSTR row);
    void setOpen(int action, int ix, int btn, LPCTSTR row, bool recipeDan);
    void setSendReceive(int action, int ix, int btn, LPCTSTR row);
    void setExport(int ix, int btn, LPCTSTR row);
    void setSendOne(uint code, int ix, int btn, LPCTSTR row, int allowedBtn);
    void setCustom(int ix, int btn, LPCTSTR row);
    void setPrintScreen(int action, int ix, int btn, LPCTSTR row);
    void setOnlyPsw(int action, int ix, int btn, LPCTSTR row);
    void setSpin(int ix, int btn, LPCTSTR row);

    void setCTRL_Action(int ix, int btn, LPCTSTR row);

    void actionChanged(int idCtrl, HWND hwnd);
    void actionChangedByBtn(int idCtrl);

//    bool getPageFile(LPTSTR buff, int code);
//    bool getIdAction(LPTSTR buff, int code);
//    bool getExportAction(LPTSTR buff);

    void fillDataBtn(LPTSTR target, int id);
    void checkModalUse();

    void chgBtnText(int ix);
    void chgPage(bool next);

    struct infoRow
    {
      TCHAR label[MAX_LABEL];
      uint actSel;
      TCHAR result[MAX_RESULT];

      infoRow() : actSel(-1) { label[0] = 0; result[0] = 0; }
      void loadRow(PWin* owner, uint row);
      void setRow(PWin* owner, uint row);
    };

    struct infoPage
    {
      infoRow iRow[MAX_BTN];
      void loadPage(PWin* owner);
      void setPage(PWin* owner);
    };
    infoPage* iPage[3];

    void openChooseVarToSend();

};
//----------------------------------------------------------------------------
#endif

