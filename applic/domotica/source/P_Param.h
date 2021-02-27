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
#include "connClass.h"
#include "p_matchStream.h"
#include "progressBar.h"
//----------------------------------------------------------------------------
#define DIM_HEADER 3
#define DIM_CODE  10
#define DIM_VER    3
#define OFFS_CODE (DIM_HEADER)
#define OFFS_VER  (DIM_HEADER + DIM_CODE)
#define MY_CODE_RFID _T("nps0000000000001")

#define DIM_FULL_CODE (DIM_HEADER + DIM_CODE + DIM_VER)

#define MAX_LEN_INIT (255 - sizeof(DWORD) * 2 - sizeof(paramConn) - 1)
//----------------------------------------------------------------------------
#define DIM_WAIT_ACK 1024
//----------------------------------------------------------------------------
struct setting
{
  paramConn par;
  TCHAR Code[DIM_FULL_CODE + 1];
  DWORD delay;
  DWORD packet;
  bool useWiFi;
  setting() : delay(0), packet(0), useWiFi(false) { _tcscpy_s(Code, MY_CODE_RFID); }
};
//----------------------------------------------------------------------------
void fillSetting(setting& s);
//----------------------------------------------------------------------------
void saveSetting(const setting& s);
//----------------------------------------------------------------------------
struct value_name
{
  int value;
  LPTSTR name;
};
//----------------------------------------------------------------------------
class connSend
{
  public:
    connSend(PConnBase* conn, progressBar* pbar = 0);
    ~connSend() {}
    bool send(LPVOID buff, int len);

    // la dimensione è fissa, DIM_WAIT_ACK, non usa la dimensione né il ritardo impostato nel setup
    bool sendWaitAck(LPVOID buff, int len);
  private:
    DWORD delay;
    DWORD packet;
    PConnBase* Conn;
    progressBar* pBar;
};
//----------------------------------------------------------------------------
#define MAX_DIM_CHECK 24
class consumer : public p_matchStream<char, MAX_DIM_CHECK>
{
private:
  typedef p_matchStream<char, MAX_DIM_CHECK> baseClass;
public:
  consumer(LPCSTR buff, uint len) : Buff(buff), Len(len), curr(0) { }
protected:
  virtual bool has_data() { return curr < Len; }
  virtual void read_one_data(char& buff) { buff = Buff[curr]; ++curr; }
private:
  LPCSTR Buff;
  uint Len;
  uint curr;
};
//----------------------------------------------------------------------------
#define ACK_ "ACK"
//----------------------------------------------------------------------------
class consumerConn : public p_matchStream<BYTE, MAX_DIM_CHECK>
{
private:
  typedef p_matchStream<BYTE, MAX_DIM_CHECK> baseClass;
public:
	consumerConn(PConnBase* conn) : Conn(conn) { }
protected:
  virtual bool has_data();
  virtual void read_one_data(BYTE& buff);
private:
  PConnBase* Conn;
};
//----------------------------------------------------------------------------
int checkPos(LPCSTR buff, LPCSTR m, int dim);
bool connFindMatch(PConnBase* conn, LPCSTR m, int dim);
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
bool readUntil(PConnBase* conn, char c);
DWORD storeUntil(PConnBase* conn, char c, LPSTR buff, DWORD len);
//----------------------------------------------------------------------------
#endif
