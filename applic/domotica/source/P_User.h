//-------- P_User.h ---------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef P_User_H
#define P_User_H
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef PDIALOG_H_
  #include "pdialog.h"
#endif

#include "PCustomListViewFullEdit.h"
#include "resource.h"
#include "p_vect.h"
#include "pbitmap.h"
#include "paramconn.h"
#include "connClass.h"
#include "P_BaseCommon.h"
//----------------------------------------------------------------------------
class P_User : public P_BaseCommon
{
  private:
    typedef P_BaseCommon baseClass;
  public:
    P_User(PConnBase* conn, PWin* parent, uint resId = IDD_DIALOG_USER, HINSTANCE hinstance = 0);
    virtual ~P_User();
    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void parseBuff(LPCSTR buff);
    void parseRow(LPCSTR* v);
    void addEmptyRow(int ix);
  void fillEmpty();
    void loadCurr();
    void appendText(LPSTR buff, size_t dim, uint ix);
  private:
    void BNClickedOk();
    PVect<PBitmap*> Bmp;
    PCustomListViewFullEdit* lv;

};
//----------------------------------------------------------------------------
#endif
