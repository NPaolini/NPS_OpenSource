//-------- P_Status.h ---------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef P_Status_H
#define P_Status_H
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "pModDialog.h"

#include "resource.h"
#include "connClass.h"
//----------------------------------------------------------------------------
struct infoStatus
{
  DWORD CTS_ON  : 1;
  DWORD DSR_ON  : 1;
  DWORD RING_ON : 1;
  DWORD RLSD_ON : 1;
  DWORD CTS_W   :1;
  DWORD DSR_W   :1;
  DWORD RLSD_W  :1;
  DWORD XOFF_R  :1;
  DWORD XOFF_T  :1;
  DWORD InQueue;
  DWORD OutQueue;
  void refresh(PConnBase* conn);
  void clear();
  infoStatus() : CTS_ON(0), DSR_ON(0), RING_ON(0), RLSD_ON(0), CTS_W(0), DSR_W(0),
        RLSD_W(0), XOFF_R(0), XOFF_T(0), InQueue(0), OutQueue(0)  {  }
};
//----------------------------------------------------------------------------
class P_Status : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    P_Status(PWin* parent, PConnBase* conn, uint resId = IDD_DIALOG_STAT, HINSTANCE hinstance = 0);
    virtual ~P_Status();

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    HBRUSH evCtlColor(HDC hdc, HWND hWndChild, UINT ctlType);

  private:
    void CmOk();
    void CmCancel() {}
    void sendRTS();
    void sendDTR();

    infoStatus Par;
    PConnBase* Conn;
    HBRUSH hbRed;
    HBRUSH hbGreen;
    uint idTimer;
};
//----------------------------------------------------------------------------
#endif
