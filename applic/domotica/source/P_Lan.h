//-------- p_lan.h ---------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef P_LAN_H
#define P_LAN_H
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
class P_Lan : public P_BaseCommon
{
  private:
    typedef P_BaseCommon baseClass;
  public:
    P_Lan(PConnBase* conn, PWin* parent, uint resId = IDD_DIALOG_LAN, HINSTANCE hinstance = 0);
    virtual ~P_Lan();
    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void refreshBase64();
    void parseIP(LPCSTR buff);
    void parsePing(LPCSTR buff);
	void parseWiFi(LPCSTR buff);
	void parseResponce(LPSTR buff, int dim);
  private:
    void BNClickedOk();
    PVect<PBitmap*> Bmp;
};
//----------------------------------------------------------------------------
#endif
