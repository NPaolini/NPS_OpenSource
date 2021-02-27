//----------- fillAlarmAssoc.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef fillAlarmAssoc_H_
#define fillAlarmAssoc_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdlib.h>
#include "resource.h"

#ifndef PDIALOG_H_
#include "pDialog.h"
#endif
//----------------------------------------------------------------------------
#include "alarmEditorDlg.h"
#include "pAl_common.h"
//----------------------------------------------------------------------------
extern void fillCbDataType(HWND hcb, int sel);
extern int getNumAndSelCB(HWND hcb, int& select);
//----------------------------------------------------------------------------
class PD_FillBase : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    PD_FillBase(PWin* parent, uint resId, HINSTANCE hinstance = 0);
    virtual ~PD_FillBase();

    virtual bool create();

  protected:
    void CmOk();
    virtual bool save() = 0;
    virtual void load() = 0;
    void enableDisable(HWND check, uint idCtrl);
  private:
    class PBitmap* Bmp[2];
};
//----------------------------------------------------------------------------
class PD_FillAlarm : public PD_FillBase
{
  private:
    typedef PD_FillBase baseClass;
  public:
    PD_FillAlarm(baseRowAlarm cfg[], PWin* parent, uint resId = IDD_DIALOG_FILL_ALARM, HINSTANCE hinstance = 0);
    virtual ~PD_FillAlarm() {}

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    bool save();
    void load();
  private:
    baseRowAlarm* Cfg;

    struct fillRowAlarm
    {
      baseRowAlarm bra;
      DWORD addrStart;
      DWORD lenData;
      DWORD checked;
      fillRowAlarm() : addrStart(0), lenData(0), checked(0) {}
    };

    void enableDisableText();
    static fillRowAlarm fillData;

};
//----------------------------------------------------------------------------
class PD_FillAssoc : public PD_FillBase
{
  private:
    typedef PD_FillBase baseClass;
  public:
    PD_FillAssoc(baseRowAssoc cfg[], PWin* parent, uint resId = IDD_DIALOG_FILL_ASSOC, HINSTANCE hinstance = 0);
    virtual ~PD_FillAssoc() {}

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    bool save();
    void load();
  private:
    baseRowAssoc* Cfg;

    struct fillRowAssoc
    {
      baseRowAssoc bra;
      DWORD addrStart;
      DWORD lenData;
      DWORD checked;
      DWORD addrStep;
      fillRowAssoc() : addrStart(0), lenData(0), checked(0), addrStep(0) {}
    };

    static fillRowAssoc fillData;
  void checkAll();
};
//----------------------------------------------------------------------------
class PD_FillAssocFile : public PD_FillBase
{
  private:
    typedef PD_FillBase baseClass;
  public:
    PD_FillAssocFile(baseRowAssocFile cfg[], PWin* parent, uint resId = IDD_DIALOG_FILL_ASSOC_FILE, HINSTANCE hinstance = 0);
    virtual ~PD_FillAssocFile() {}

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    bool save();
    void load();
  private:
    baseRowAssocFile* Cfg;

    struct fillRowAssocFile
    {
      baseRowAssocFile bra;
      DWORD addrStart;
      DWORD lenData;
      DWORD checked;
      DWORD valueStep;
      fillRowAssocFile() : addrStart(0), lenData(0), checked(0), valueStep(0) {}
    };

    static fillRowAssocFile fillData;

};
//----------------------------------------------------------------------------
#endif
