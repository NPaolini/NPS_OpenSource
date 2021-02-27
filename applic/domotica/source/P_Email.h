//-------- P_Email.h ---------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef P_Email_H
#define P_Email_H
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
class P_Email : public P_BaseCommon
{
  private:
    typedef P_BaseCommon baseClass;
  public:
    P_Email(PConnBase* conn, PWin* parent, uint resId = IDD_DIALOG_EMAIL, HINSTANCE hinstance = 0);
    virtual ~P_Email();
    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void parseBuff(LPCSTR buff);
    void loadCurr();
    void refreshBase64(UINT idc1, UINT idc2);
    void parsePair(LPCSTR l, LPCSTR v);
    void setText_and_64(LPCSTR v, UINT idc1, UINT idc2);
    void appendText(LPSTR buff, size_t dim, LPCSTR l, UINT idc, LPCSTR prefx);
  private:
    void BNClickedOk();
    PVect<PBitmap*> Bmp;
};
//----------------------------------------------------------------------------
#endif
