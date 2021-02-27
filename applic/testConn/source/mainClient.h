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
    PVect<BYTE> pBuff;
    PVect<PBitmap*> Bmp;
    class P_File* Capture;
    int currAddr;
    setting Setting;
    bool showHex;
    bool ShowMsgBox;

    bool dontDelete;

    bool openCom();
    void switchOpenBmp();
    void setupCom();

    int getDec(uint idCtrl);
    void setDec(uint idCtrl, int val);
    void updateEdit(uint idCtrl);
    void CloseApp(HWND hwnd);

    bool open_File(LPTSTR file);
    bool sendBuff(const BYTE* buff, int len, bool useDelay = true);
    void sendFile();
    void saveFile();
    bool verifyCom();

    enum whichSend { STRING, VAL };
    void sendString(whichSend type);
    void readComm(int readed);

    void clearReceived();
    void setCaptureFile();
    void changeShow();
    void performShowChar(HWND ctrl, const BYTE* buff, int avail);
    void performShowHex(HWND ctrl, const BYTE* buff, int avail, int currLen);

    bool getTelephoneNumber(LPTSTR number);

    void showConn(DWORD conn, LPARAM lParam);
    void addClient(HANDLE_CONN client);

    class PConnBase* Com;
    class customServerLanData* ServerLan;
    friend class customServerLanData;
    class P_Status* InfoConn;
    void openInfoConn();
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
