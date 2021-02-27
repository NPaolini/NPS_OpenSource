//-------- p_param.h ---------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef P_PARAM_H
#define P_PARAM_H
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
//----------------------------------------------------------------------------
#define MAX_LEN_INIT (255 - sizeof(DWORD) * 2 - sizeof(paramConn) - 1)
struct setting
{
  paramConn par;
  DWORD delay;
  DWORD connType : 2; // zero = seriale, 1 = modem , 2 = lan
  DWORD modemType :1; // zero = pulse, uno = tone
  DWORD lanType  : 1; // zero = client, uno = server
  DWORD proto    : 1; // zero = TCP, uno = UDP
  DWORD on485    : 1;
  TCHAR initModem[MAX_LEN_INIT];
  DWORD packet;

  setting() : delay(0), connType(0), modemType(1), lanType(0), proto(0), packet(0)
  { ZeroMemory(initModem, sizeof(initModem)); }
};
//----------------------------------------------------------------------------
struct value_name
{
  int value;
  LPTSTR name;
};
//----------------------------------------------------------------------------
class P_Param : public PDialog
{
  private:
    typedef PDialog baseClass;
  public:
    P_Param(setting& par, PWin* parent, uint resId = IDD_PARAM, HINSTANCE hinstance = 0);
    virtual ~P_Param();

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  private:
    void BNClickedOk();
    void fillCtrl();
    void checkEnable(int type);
    void checkEnableDelay();
    setting &Par;
    PVect<PBitmap*> Bmp;
};
//----------------------------------------------------------------------------
#endif
