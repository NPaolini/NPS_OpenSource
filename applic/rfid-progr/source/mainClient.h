//----------- mainClient.h ---------------------------------------------------
//----------------------------------------------------------------------------
#ifndef MAINCLIENT_H_
#define MAINCLIENT_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "conn_dll.h"
#include "resource.h"

#ifndef PMODDIALOG_H_
#include "pmoddialog.h"
#endif

#ifndef PRECT_H_
#include "prect.h"
#endif
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
    virtual LRESULT windowProc(HWND hwnd,
                            UINT message,
                            WPARAM wParam,
                            LPARAM lParam);
    virtual HBRUSH evCtlColor(HDC dc, HWND hWndChild, UINT ctlType);

  private:
    HFONT Font;
    HBRUSH Brush;
    HWND hwTips;
    PVect<PBitmap*> Bmp;
    int currAddr;
    setting Setting;

    bool dontDelete;

    bool openCom();
    void switchOpenBmp();
    void setupCom();

    void CloseApp(HWND hwnd);

    bool sendBuff(const BYTE* buff, int len, bool useDelay = true);
    bool verifyCom();

    void readComm(int readed);

    void performShowChar(HWND ctrl, const BYTE* buff, int avail);
    void clearReceived();
    void addClient(HANDLE_CONN client);

    class PConnBase* Com;
    void sendCode();
    bool checkSendCode(const BYTE* buff, int avail);

    void checkEnable();
    void sendReset();
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
