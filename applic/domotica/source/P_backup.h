//-------- P_backup.h --------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef P_backup_H
#define P_backup_H
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef PDIALOG_H_
  #include "pdialog.h"
#endif

#include "resource.h"
#include "p_vect.h"
#include "pbitmap.h"
#include "connClass.h"
#include "P_BaseCommon.h"
//----------------------------------------------------------------------------
class P_backup : public P_BaseCommon
{
  private:
    typedef P_BaseCommon baseClass;
  public:
    P_backup(PConnBase* conn, PWin* parent, uint resId = IDD_DIALOG_BACKUP, HINSTANCE hinstance = 0);
    virtual ~P_backup();
    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void saveBackup();
    void sendRestore();
    void enable();
    void open_File();
  private:
    void BNClickedOk();
    PVect<PBitmap*> Bmp;
};
//----------------------------------------------------------------------------
#endif
