//------------ baseAdvBtnDlg.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef baseAdvBtnDlg_H_
#define baseAdvBtnDlg_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdlib.h>
#include "resource.h"

#ifndef PDIALOG_H_
#include "pDialog.h"
#endif
//----------------------------------------------------------------------------
#include "svmPropertyBtn.h"
//----------------------------------------------------------------------------
class baseAdvBtnDlg : public PDialog
{
  private:
    typedef PDialog baseClass;
  public:
    baseAdvBtnDlg(PWin* parent, PropertyBtn& prop, uint resId, HINSTANCE hinstance = 0);
    virtual ~baseAdvBtnDlg();

    virtual void saveData() = 0;

    bool create();
    virtual void changedPosBmp(uint selected) {}
  protected:
    PropertyBtn& Prop;
    int lastSel4Mod;
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    virtual uint getCheck() = 0;
    virtual const fullPrph& getDataPrf() = 0;
    virtual void enablePrph(bool enable);
    virtual void enableOther(uint ix);
    virtual void setPrph(const fullPrph& DataPrf);
    virtual void setCheck(uint ix);
    virtual uint retrieveCheck();
    virtual void retrievePrph(fullPrph& DataPrf);

    virtual bool needVal();

    virtual void move(bool up);
    virtual void add();
    virtual int rem();
    virtual void mod();
    // torna la dimensione del buffer, se si passa 0 come target esegue solo il calcolo
    virtual int makeRow(LPTSTR target, uint sz) = 0;
    virtual int unmakedRow(LPTSTR target) = 0;

    virtual void enableMod(bool set);
};
//----------------------------------------------------------------------------
enum eBtnPageType { eptText, eptColor, eptBmp, eptMax };
//----------------------------------------------------------------------------
class advancedBtnDlg : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    advancedBtnDlg(PWin* parent, PropertyBtn& prop, uint resId, HINSTANCE hinstance = 0);
    virtual ~advancedBtnDlg();

    bool create();
    void setOnMod(bool set);
    bool inOnMod() const { return onMod; }
  protected:
    PropertyBtn& Prop;
    baseAdvBtnDlg* advBtn[eptMax];
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CmOk();
    void CmCancel();
    void enableStyle();

    int currPage;
    void chgPage(int page);
    bool onMod;
};
//----------------------------------------------------------------------------
#endif
