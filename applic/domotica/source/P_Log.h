//-------- P_Log.h ---------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef P_Log_H
#define P_Log_H
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef PDIALOG_H_
  #include "pdialog.h"
#endif

#include "resource.h"
#include "p_vect.h"
#include "pbitmap.h"
#include "paramconn.h"
#include "connClass.h"
#include "P_BaseCommon.h"
//----------------------------------------------------------------------------
class P_Log : public P_BaseCommon
{
  private:
    typedef P_BaseCommon baseClass;
  public:
    P_Log(PConnBase* conn, PWin* parent, uint resId = IDD_DIALOG_LOG, HINSTANCE hinstance = 0);
    virtual ~P_Log();
    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void loadLog();
    void delLog();
    int getCurrSel();
    void clearReceived();
    void readComm(int );
    void performShowChar(HWND ctrl, const BYTE* buff, int avail);
    virtual void readConn(WORD v);

  private:
    void BNClickedOk();
    PVect<PBitmap*> Bmp;
};
//----------------------------------------------------------------------------
#endif
