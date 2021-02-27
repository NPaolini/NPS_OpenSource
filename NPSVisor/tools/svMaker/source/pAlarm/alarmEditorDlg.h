//----------- alarmEditorDlg.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef alarmEditorDlg_H_
#define alarmEditorDlg_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <commctrl.h>
#include <stdlib.h>
#include "resource.h"

#include "pModDialog.h"

#include "p_Vect.h"
#include "alarmDlg.h"
#include "assocDlg.h"
//----------------------------------------------------------------------------
class PD_alarmEditorDlg : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    PD_alarmEditorDlg(PWin* parent, uint resId = IDD_ALARM_EDITOR, HINSTANCE hinstance = 0);
    virtual ~PD_alarmEditorDlg();

    virtual bool create();

    PD_Alarm* getAlarm(PD_Assoc* client);
    PD_Assoc* getAssoc(PD_Alarm* client);

    // verifica non l'oggetto passato, ma quello collegato
    bool isDirty(PD_Alarm* client);
    bool isDirty(PD_Assoc* client);

    bool isDirty(PD_Base* client);

    bool isDirty();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  private:
    PVect<PD_Alarm*> clientAlarm;
    PVect<PD_Assoc*> clientAssoc;

    int currPage;

    HIMAGELIST ImageList;
    void chgPage(int page);
    void chgAssocPage(int page);
    void chgAlarmPage(int page);

    enum which { jobbingAlarm, jobbingAssoc };
    which onJob;
    void toggleVis();
    bool saveOnExit();

    void remTab();
    void addTab();

    void moveChild(PWin* child);

    void makeClient();
    void performAddTab(uint cid, uint pos);

    bool saveDirtyPage(bool req);

};
//----------------------------------------------------------------------------
#endif
