//------------------ svmCalibrate.h ------------------------
//-----------------------------------------------------------
#ifndef svmCalibrate_H_
#define svmCalibrate_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "p_util.h"
#include "pmoddialog.h"
#include "svmManZoom.h"
#include "resource.h"
//-----------------------------------------------------------
class PCalibrate : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    PCalibrate(PWin* parent, uint id = IDD_CALIBRATE, HINSTANCE hInstance = 0) : baseClass(parent, id, hInstance),
          onEdit(false), onTrack(false)
    {
    }
    virtual ~PCalibrate() { destroy(); }

    virtual bool create();

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CmOk();
    void CmCancel();
    void evHScrollBar(HWND child, int flags, int pos);
    void setAdjuct(infoAdjusct v);
    infoAdjusct oldAdjuct;

    void editPos(uint idc);
    void chgPos(DWORD pos, uint idc);
    bool onEdit;
    bool onTrack;
};
//-----------------------------------------------------------
#endif
