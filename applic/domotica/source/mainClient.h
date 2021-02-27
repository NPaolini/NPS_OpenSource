//----------- mainClient.h ---------------------------------------------------
//----------------------------------------------------------------------------
#ifndef MAINCLIENT_H_
#define MAINCLIENT_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "conn_dll.h"
#include "resource.h"
#include "pmoddialog.h"
#include "prect.h"
#include "p_vect.h"
#include "pbitmap.h"
#include "p_param.h"
//----------------------------------------------------------------------------
typedef UINT uint;
//----------------------------------------------------------------------------
class mainClient : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    mainClient(PWin* parent, uint resId = IDD_CLIENT, HINSTANCE hInstance = 0);
    virtual ~mainClient();

    virtual bool create();
    void CloseApp();
    virtual bool preProcessMsg(MSG& msg);

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void switchOpenBmp();
    bool openConn();
    void checkEnable();
  private:
    HWND hwTips;
    PVect<PBitmap*> Bmp;
    setting Setting;
    void setupCom();

    void CloseApp(HWND hwnd);

    class PConnBase* Conn;
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
inline void mainClient::CloseApp()
{
  if(getHandle())
    CloseApp(getHandle());
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// se occorresse derivare dalla finestra di dialogo principale
// alloca la window client principale
PWin* allocMainClient(PWin* parent, HINSTANCE hInstance);
//----------------------------------------------------------------------------
// per modificare lo stile di default o altro.
// Viene richiamata dopo aver settato lo stile di default
void customizeMain(PWin *);
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#endif
