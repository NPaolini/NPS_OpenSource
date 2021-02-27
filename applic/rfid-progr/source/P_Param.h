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
#define DIM_HEADER 3
#define DIM_CODE  10
#define DIM_VER    3
#define OFFS_CODE (DIM_HEADER)
#define OFFS_VER  (DIM_HEADER + DIM_CODE)

#define DIM_FULL_CODE (DIM_HEADER + DIM_CODE + DIM_VER)

#define MAX_LEN_INIT (255 - sizeof(DWORD) * 2 - sizeof(paramConn) - 1)
struct setting
{
  paramConn par;
  TCHAR Code[DIM_FULL_CODE + 1];

  setting()  { _tcscpy_s(Code, _T("nps0000000000001")); }
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
    setting &Par;
    PVect<PBitmap*> Bmp;

    void codeToCtrl();
    void codeFromCtrl();

};
//----------------------------------------------------------------------------
#endif
