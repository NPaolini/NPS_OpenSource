//------------- d_alarm.h ----------------------------------------------------
//----------------------------------------------------------------------------
#ifndef D_ALARM_H_
#define D_ALARM_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdlib.h>

#ifndef SVDIALOG_H_
  #include "svdialog.h"
#endif

#ifndef PSTATIC_H_
  #include "pstatic.h"
#endif

#ifndef PLISTBOX_H_
  #include "plistbox.h"
#endif

#ifndef PRADIOBTN_H_
  #include "pradiobtn.h"
#endif

#ifndef PCHECKBOX_H_
  #include "pcheckbox.h"
#endif

#include "svPrinter.h"

#include "setPack.h"
//----------------------------------------------------------------------------
// poiché gli id dei report sono stati spostati nei target finali,
// occorre definire questa funzione necessaria alla gestione generica
extern smartPointerConstString getTitleRepAlarm();
//----------------------------------------------------------------------------
extern svPrinter* getPrinter();
//----------------------------------------------------------------------------
class TD_Alarm : public svDialog {
  public:
    TD_Alarm(PWin* parent, class perifsSet *perif, uint resId = IDD_ALARM, HINSTANCE hinst = 0);
    virtual ~TD_Alarm();
    virtual bool create();
    virtual bool preProcessMsg(MSG& msg);

  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void BNClicked_F1();
    void BNClicked_F2();
    void BNClicked_F3();
    void BNClicked_F4();

    bool BNClicked_RB(int id);
    void setReport();
    void handleLBDblClick(HWND hwnd);


  protected:
    PCheckBox* CB_Rep_Al;
    PRadioButton* RB_TypeRep[3];
    PStatic* Frame1;
    PStatic* HeadLabel;
    PStatic* Exit;
    PStatic* Reset;
    PStatic* Show;
    PListBox* LB_Alarm;
    PListBox* LB_Report;
    HFONT Font;
    int idTimer;
    static TCHAR FilterText[512];
    int showAlarm[7];
    int showReport[7];

    void makeHeader(bool report);
    PVect<setOfBlockString*> InfoAlarm;

    class perifsSet *Perif;
    bool checkBtn;
    int currRep;
    enum eTypeShow { SHOW_ALARM, SHOW_REPORT };
    eTypeShow currShow;

    void fill_LB(int action = 0);
    bool acceptRowAlarm(const fullInfoAlarm& fia);
    bool formatAlarm(const fullInfoAlarm& fia, const SYSTEMTIME &st, int stat, bool atTop = false);
    void fill_Report(LPCTSTR path, bool reset);

    void print_Alarm(bool preview, bool setup);
    TCHAR filenameReport[_MAX_PATH];
    void print_Report(bool preview, bool setup);
    void print_Setup();

    void showInfoAlarm();

    // -1 = no refresh, 0 = richiesta, 1 = refresh continuo
    int waitRefresh;
    void checkWaitRefresh();

    struct statAlarm {
      int id;
      int stat;
      statAlarm(int id = 0, int stat = 0) :
          id(id), stat(stat) {}
      };

    typedef PVect<statAlarm> vStat;
    vStat Status;

    void update_LB();
    int checkExist(const statAlarm& sa);
    bool updateStatus(int& oldSel, int row, int ix);
    void removeItem(int& oldSel, int row, int ix);

    void setInfoTitle();
    void setAdvancedReport();
  private:
    typedef svDialog baseClass;
};
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif

