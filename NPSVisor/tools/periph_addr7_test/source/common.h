//----------- common.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef COMMON_H_
#define COMMON_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "resource.h"
#include "pStatic.h"
#include "pEdit.h"
#include "p_util.h"
#include "p_txt.h"
#include "p_file.h"
#include "pOwnBtnImageStd.h"
#include "pModDialog.h"
#include "pCommonFilter.h"
#include "macro_utils.h"
#include "headerMsg.h"
#include "defgPerif.h"
//----------------------------------------------------------------------------
#define WM_CLIENT_REQ (WM_FW_FIRST_FREE + 10)
  #define CM_SAVE_ALL 1
  #define CM_NEW_PATH 2

#define WM_CLIENT_WHEEL (WM_CLIENT_REQ + 1)
#define WM_CUSTOM_VSCROLL (WM_CLIENT_WHEEL + 1)
//----------------------------------------------------------------------------
extern bool setFolderSVisor();
extern bool chooseFolderSVisor();
extern void checkBasePage();
//----------------------------------------------------------------------------
// per evitare il refresh dei campi di edit durante lo scorrimento
// imposta il nuovo e torna il vecchio
extern bool setIgnoreKillFocus(bool set);

// valido anche per ignorare modifiche temporanee
extern bool canIgnoreKillFocus();
//----------------------------------------------------------------------------
struct commonInfo
{
  bool base;
  DWORD baseAddr;
  DWORD totRow;
  DWORD currRow;
  DWORD currGotoRow;
  DWORD nByteBase;
};
//----------------------------------------------------------------------------
class PEditCR : public PEdit
{
  private:
    typedef PEdit baseClass;
  public:
    PEditCR(PWin * parent, uint resid, uint cmdid, int textlen = 255, HINSTANCE hinst = 0) :
      baseClass(parent, resid, 0, hinst), cmdid(cmdid) { }
    ~PEditCR() { destroy(); }
  protected:
    int verifyKey()  { return false; }
    virtual bool evChar(UINT& key) { return false; }
    bool evKeyDown(UINT& key);
    uint cmdid;
};
//----------------------------------------------------------------------------
#define MAX_TEXT 255
//----------------------------------------------------------------------------
#define MAX_GROUP 16
//#define MAX_ADDRESSES (4096 * 2 - 1)
#define MAX_ADDRESSES SIZE_OTHER_DATA
//----------------------------------------------------------------------------
//#define MAX_ADDRESSES_PRPH 4096
#define MAX_ADDRESSES_PRPH MAX_DWORD_PERIF
//----------------------------------------------------------------------------
#define IP_TO_DWORD(b1, b2, b3, b4, dw) \
  ((dw) = ((DWORD)(b1) << 24) | ((DWORD)(b2) << 16) | ((DWORD)(b3) << 8) | (b4))
//----------------------------------------------------------------------------
#define DWORD_TO_IP(b1, b2, b3, b4, dw) \
    { (b1) = (dw) >> 24; \
       (b2) = ((dw) >> 16) & 0xff; \
       (b3) = ((dw) >> 8) & 0xff;  \
       (b4) = (dw) & 0xff; }
//----------------------------------------------------------------------------
struct baseRow
{
  DWORD ipAddr;
  DWORD port;
  DWORD db;
  DWORD addr;
  DWORD type;
  DWORD action;
  double vMin;
  double vMax;
  double vMinP;
  double vMaxP;
  TCHAR text[MAX_TEXT];
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
struct menuPopup
{
  uint flag;
  uint id;
  LPCTSTR text;
};
//----------------------------------------------------------------------------
#define PAGE_SCROLL_LEN  (MAX_GROUP / 2)
#define MAX_V_SCROLL (maxShow - MAX_GROUP)
//#define MAX_V_SCROLL (MAX_ADDRESSES - MAX_GROUP)
//----------------------------------------------------------------------------
#define MAX_COL_INT 9
#define MAX_COL_REAL 4
//----------------------------------------------------------------------------
#define MAX_COL (MAX_COL_INT + MAX_COL_REAL)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define IDC_EDIT_BYTE1_1 2000
//----------------------------------------------------------------------------
#define DEF_OFFSET_CTRL 100
//----------------------------------------------------------------------------
#define OFFSET_BYTE1  0
#define OFFSET_BYTE2  (OFFSET_BYTE1 + DEF_OFFSET_CTRL)
#define OFFSET_BYTE3  (OFFSET_BYTE2 + DEF_OFFSET_CTRL)
#define OFFSET_BYTE4  (OFFSET_BYTE3 + DEF_OFFSET_CTRL)
#define OFFSET_PORT   (OFFSET_BYTE4 + DEF_OFFSET_CTRL)
#define OFFSET_DB     (OFFSET_PORT + DEF_OFFSET_CTRL)
#define OFFSET_ADDR   (OFFSET_DB + DEF_OFFSET_CTRL)
#define OFFSET_TYPE   (OFFSET_ADDR + DEF_OFFSET_CTRL)
#define OFFSET_ACTION (OFFSET_TYPE + DEF_OFFSET_CTRL)
#define OFFSET_VMIN   (OFFSET_ACTION + DEF_OFFSET_CTRL)
#define OFFSET_VMAX   (OFFSET_VMIN + DEF_OFFSET_CTRL)
#define OFFSET_VMIN_P (OFFSET_VMAX + DEF_OFFSET_CTRL)
#define OFFSET_VMAX_P (OFFSET_VMIN_P + DEF_OFFSET_CTRL)
#define OFFSET_DESCR  (OFFSET_VMAX_P + DEF_OFFSET_CTRL)
//----------------------------------------------------------------------------
extern bool openFileAddr(HWND owner, LPTSTR file, DWORD prph);
extern bool saveAsAddr(HWND owner, LPTSTR file, DWORD prph);
extern void saveNumOfRow(DWORD maxShow, DWORD prph);
extern DWORD getNumOfRow(DWORD prph);
//----------------------------------------------------------------------------
class PEditEmpty : public PEdit
{
  private:
    typedef PEdit baseClass;
  public:
    PEditEmpty(PWin * parent, uint id, const PRect& r,
              int textlen = 255, HINSTANCE hinst = 0):
        baseClass(parent, id, r, 0, textlen, hinst), alsoSibling(0) { }

    void invalidateNum() const;
    void setSibling(int offs) { alsoSibling = offs; }
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    int alsoSibling;
};
//----------------------------------------------------------------------------
#define IS_THIS_CHANGED() SendMessage(*this, EM_GETMODIFY, 0, 0)
//----------------------------------------------------------------------------
class PRow
{
  public:

    PRow(PWin* parent, uint first_id, POINT pt, int whichRow);
    virtual ~PRow() {}

    void offset(int x, int y, double scaleX, double scaleY);

    void getDescr(LPTSTR buff) const;
    void setDescr(LPCTSTR buff) const;
    DWORD getIpAddr();
    void setIpAddr(DWORD addr);

    DWORD getPort();
    void setPort(DWORD db);

    DWORD getDB();
    void setDB(DWORD db);

    DWORD getAddr(DWORD base);
    void setAddr(DWORD addr, DWORD base);

    DWORD getType();
    void setType(DWORD type);
    DWORD getAction();
    void setAction(DWORD action);

    double getMin();
    void setMin(double v);

    double getMax();
    void setMax(double v);

    double getMinP();
    void setMinP(double v);

    double getMaxP();
    void setMaxP(double v);

    void getAll(DWORD& ipAddr, DWORD& port, DWORD& db, DWORD& addr, DWORD& type, DWORD& action,
                double& vmin, double& vmax, double& vminP, double& vmaxP, DWORD base);
    void setAll(DWORD ipAddr, DWORD port, DWORD db, DWORD addr, DWORD type, DWORD action,
                double vmin, double vmax, double vminP, double vmaxP, DWORD base);

    bool hasId(uint id);
    void invalidate();
  protected:
    PEditEmpty* Byte1;
    PEditEmpty* Byte2;
    PEditEmpty* Byte3;
    PEditEmpty* Byte4;
    PEditEmpty* Port;
    PEditEmpty* Db;
    PEditEmpty* Addr;
    PEditEmpty* Type;
    PEditEmpty* Action;
    PEditEmpty* Descr;

    PEditEmpty* Min;
    PEditEmpty* Max;
    PEditEmpty* MinP;
    PEditEmpty* MaxP;

    DWORD realAddr;
//    DWORD baseAddr;

    bool isHex();
    uint firstId;
    void invalidateNum(PEditEmpty* ctrl) const;
    DWORD getGen(PEditEmpty* ctrl);
    void setGen(PEditEmpty* ctrl, DWORD val);
    DWORD getGenHex(PEditEmpty* ctrl);
    void setGenHex(PEditEmpty* ctrl, DWORD val);
};
//----------------------------------------------------------------------------
class PageEdit : public PEditEmpty
{
  private:
    typedef PEditEmpty baseClass;
  public:
    PageEdit(PWin * parent, uint id, const PRect& r,
              int textlen = 255, HINSTANCE hinst = 0):
        baseClass(parent, id, r, textlen, hinst)
    {
      Attr.style |= ES_AUTOHSCROLL | ES_CENTER | ES_WANTRETURN;
      setFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
    }
  protected:
    virtual bool evKeyDown(UINT& key);
};
//----------------------------------------------------------------------------
class clickEdit : public PageEdit
{
  private:
    typedef PageEdit baseClass;
  public:
    clickEdit(PWin * parent, uint id, const PRect& r,
              int textlen = 255, HINSTANCE hinst = 0):
        baseClass(parent, id, r, textlen, hinst) { /*Attr.style |= ES_NOHIDESEL;*/ }
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual bool performChoose(LPTSTR buff) = 0;
};
//----------------------------------------------------------------------------
class clickEditType : public clickEdit
{
  private:
    typedef clickEdit baseClass;
  public:
    clickEditType(PWin * parent, uint id, const PRect& r,
              int textlen = 255, HINSTANCE hinst = 0):
        baseClass(parent, id, r, textlen, hinst) {}
  protected:
    virtual bool performChoose(LPTSTR buff);
};
//----------------------------------------------------------------------------
class clickEditAction : public clickEdit
{
  private:
    typedef clickEdit baseClass;
  public:
    clickEditAction(PWin * parent, uint id, const PRect& r,
              int textlen = 255, HINSTANCE hinst = 0):
        baseClass(parent, id, r, textlen, hinst) {}
  protected:
    virtual bool performChoose(LPTSTR buff);
};
//----------------------------------------------------------------------------
class clickEditTypeFirst : public clickEditType
{
  private:
    typedef clickEditType baseClass;
  public:
    clickEditTypeFirst(PWin * parent, uint id, const PRect& r,
              int textlen = 255, HINSTANCE hinst = 0):
        baseClass(parent, id, r, textlen, hinst) {}
  protected:
    virtual bool evKeyDown(UINT& key);
};
//----------------------------------------------------------------------------
class clickEditActionFirst : public clickEditAction
{
  private:
    typedef clickEditAction baseClass;
  public:
    clickEditActionFirst(PWin * parent, uint id, const PRect& r,
              int textlen = 255, HINSTANCE hinst = 0):
        baseClass(parent, id, r, textlen, hinst) {}
  protected:
    virtual bool evKeyDown(UINT& key);
};
//----------------------------------------------------------------------------
class clickEditTypeLast : public clickEditType
{
  private:
    typedef clickEditType baseClass;
  public:
    clickEditTypeLast(PWin * parent, uint id, const PRect& r,
              int textlen = 255, HINSTANCE hinst = 0):
        baseClass(parent, id, r, textlen, hinst) {}
  protected:
    virtual bool evKeyDown(UINT& key);
};
//----------------------------------------------------------------------------
class clickEditActionLast : public clickEditAction
{
  private:
    typedef clickEditAction baseClass;
  public:
    clickEditActionLast(PWin * parent, uint id, const PRect& r,
              int textlen = 255, HINSTANCE hinst = 0):
        baseClass(parent, id, r, textlen, hinst) {}
  protected:
    virtual bool evKeyDown(UINT& key);
};
//----------------------------------------------------------------------------
class firstEdit : public PageEdit
{
  private:
    typedef PageEdit baseClass;
  public:
    firstEdit(PWin * parent, uint id, const PRect& r,
              int textlen = 255, HINSTANCE hinst = 0):
        baseClass(parent, id, r, textlen, hinst) {}
  protected:
    virtual bool evKeyDown(UINT& key);
};
//----------------------------------------------------------------------------
#define MAX_EDIT_BYTE   4
#define MAX_EDIT_DWORD 12
#define MAX_EDIT_REAL  40
//----------------------------------------------------------------------------
class lastEdit : public PageEdit
{
  private:
    typedef PageEdit baseClass;
  public:
    lastEdit(PWin * parent, uint id, const PRect& r,
              int textlen = 255, HINSTANCE hinst = 0):
        baseClass(parent, id, r, textlen, hinst) {}
  protected:
    virtual bool evKeyDown(UINT& key);
};
//----------------------------------------------------------------------------
#define X_INIT 57
#define Y_INIT 85
//----------------------------------------------------------------------------
#define H_EDIT 19
//----------------------------------------------------------------------------
#define W_BYTE    27
#define W_PORT    51
#define W_DB      42
#define W_ADDR    39
#define W_TYPE    24
#define W_ACTION  24
#define W_MIN     54
#define W_MAX     54
#define W_NORM    54
#define W_DESCR   196
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define DEF_OFFSET_CTRL_TEST DEF_OFFSET_CTRL
//----------------------------------------------------------------------------
#define W_IP_PORT       104
#define W_DB_ADDR       56
#define W_TYPE_ACTION   50
#define W_RANGE         100
#define W_RANGEP        100
#define W_VALUE         (296 - 186)
#define W_DESCR_TEST    (296 - W_VALUE)
//#define W_VALUE         80
//#define W_DESCR_TEST    210

#define OFFSET_DB_ADDR      (DEF_OFFSET_CTRL_TEST)
#define OFFSET_TYPE_ACTION  (OFFSET_DB_ADDR + DEF_OFFSET_CTRL_TEST)
#define OFFSET_VRANGE       (OFFSET_TYPE_ACTION + DEF_OFFSET_CTRL_TEST)
#define OFFSET_VRANGEP      (OFFSET_VRANGE + DEF_OFFSET_CTRL_TEST)
#define OFFSET_VALUE        (OFFSET_VRANGEP + DEF_OFFSET_CTRL_TEST)

#define OFFSET_DESCR_TEST   (OFFSET_VALUE + DEF_OFFSET_CTRL_TEST)
//----------------------------------------------------------------------------
inline
void PRow::getDescr(LPTSTR buff) const
{
  GetWindowText(*Descr, buff, MAX_TEXT - 1);
}
//----------------------------------------------------------------------------
#define IS_CHANGED(hCtrl) SendMessage(hCtrl, EM_GETMODIFY, 0, 0)
//----------------------------------------------------------------------------
inline
void PRow::invalidateNum(PEditEmpty* ctrl) const
{
  ctrl->invalidateNum();
}
//----------------------------------------------------------------------------
inline
void PRow::setDescr(LPCTSTR buff) const
{
  SetWindowText(*Descr, buff);
  invalidateNum(Descr);
}
//----------------------------------------------------------------------------
inline
DWORD PRow::getGen(PEditEmpty* ctrl)
{
  TCHAR buff[50];
  GetWindowText(*ctrl, buff, SIZE_A(buff));
  return _tcstoul(buff, 0, 10);
}
//----------------------------------------------------------------------------
inline
DWORD PRow::getGenHex(PEditEmpty* ctrl)
{
  TCHAR buff[50];
  GetWindowText(*ctrl, buff, SIZE_A(buff));
  return _tcstoul(buff, 0, 16);
}
//----------------------------------------------------------------------------
inline
void PRow::setGen(PEditEmpty* ctrl, DWORD val)
{
  TCHAR buff[50];
  wsprintf(buff, _T("%d"), val);
  SetWindowText(*ctrl, buff);
  invalidateNum(ctrl);
}
//----------------------------------------------------------------------------
inline
void PRow::setGenHex(PEditEmpty* ctrl, DWORD val)
{
  TCHAR buff[50];
  wsprintf(buff, _T("%X"), val);
  SetWindowText(*ctrl, buff);
  invalidateNum(ctrl);
}
//----------------------------------------------------------------------------
inline
DWORD PRow::getPort()
{
  return getGen(Port);
}
//----------------------------------------------------------------------------
inline
void PRow::setPort(DWORD val)
{
  setGen(Port, val);
}
//----------------------------------------------------------------------------
inline
DWORD PRow::getDB()
{
  return getGen(Db);
}
//----------------------------------------------------------------------------
inline
void PRow::setDB(DWORD val)
{
  setGen(Db, val);
}
//----------------------------------------------------------------------------
/*
inline
DWORD PRow::getAddr(DWORD baseAddr)
{
  if(IS_CHANGED(*Addr))
    return getGen(Addr) * baseAddr;
  return realAddr;
}
//----------------------------------------------------------------------------
inline
void PRow::setAddr(DWORD vAddr, DWORD base)
{
  realAddr = vAddr;
  setGen(Addr, vAddr / base);
}
*/
//----------------------------------------------------------------------------
inline
DWORD PRow::getType()
{
  return getGen(Type);
}
//----------------------------------------------------------------------------
inline
void PRow::setType(DWORD vType)
{
  setGen(Type, vType);
}
//----------------------------------------------------------------------------
inline
DWORD PRow::getAction()
{
  return getGen(Action);
}
//----------------------------------------------------------------------------
inline
void PRow::setAction(DWORD vAct)
{
  setGen(Action, vAct);
}
//----------------------------------------------------------------------------
inline
double PRow::getMin()
{
  TCHAR buff[50];
  GetWindowText(*Min, buff, SIZE_A(buff));
  return _tstof(buff);
}
//----------------------------------------------------------------------------
inline
double PRow::getMax()
{
  TCHAR buff[50];
  GetWindowText(*Max, buff, SIZE_A(buff));
  return _tstof(buff);
}
//----------------------------------------------------------------------------
inline
double PRow::getMinP()
{
  TCHAR buff[50];
  GetWindowText(*MinP, buff, SIZE_A(buff));
  return _tstof(buff);
}
//----------------------------------------------------------------------------
inline
double PRow::getMaxP()
{
  TCHAR buff[50];
  GetWindowText(*MaxP, buff, SIZE_A(buff));
  return _tstof(buff);
}
//----------------------------------------------------------------------------
inline
void PRow::getAll(DWORD& ipAddr, DWORD& port, DWORD& db, DWORD& addr, DWORD& type, DWORD& action,
        double& vmin, double& vmax, double& vminP, double& vmaxP, DWORD base)
{
  ipAddr = getIpAddr();
  port = getPort();
  db = getDB();
  addr = getAddr(base);
  type = getType();
  action = getAction();
  vmin = getMin();
  vmax = getMax();
  vminP = getMinP();
  vmaxP = getMaxP();
}
//----------------------------------------------------------------------------
inline
void PRow::setAll(DWORD ipAddr, DWORD port, DWORD db, DWORD addr, DWORD type, DWORD action,
            double vmin, double vmax, double vminP, double vmaxP, DWORD base)
{
  setIpAddr(ipAddr);
  setPort(port);
  setDB(db);
  setAddr(addr, base);
  setType(type);
  setAction(action);
  setMin(vmin);
  setMax(vmax);
  setMinP(vminP);
  setMaxP(vmaxP);
}
//----------------------------------------------------------------------------
struct rangeLP
{
  double minL;
  double maxL;
  double minP;
  double maxP;
};
//----------------------------------------------------------------------------
 // da prfData
    enum tData {  tNoData,  // nessun tipo definito -  0
                  tBitData, // campo di bit         -  1
                  tBData,   // 8 bit -> 1 byte      -  2
                  tWData,   // 16 bit-> 2 byte      -  3
                  tDWData,  // 32 bit-> 4 byte      -  4
                  tFRData,  // 32 bit float         -  5
                  ti64Data, // 64 bit               -  6
                  tRData,   // 64 bit float         -  7
                  tDateHour,// 64 bit per data + ora-  8
                  tDate,    // 64 bit per data      -  9
                  tHour,     // 64 bit per ora      - 10
                  // valori signed
                  tBsData,   // 8 bit -> 1 byte      -  11
                  tWsData,   // 16 bit-> 2 byte      -  12
                  tDWsData,  // 32 bit-> 4 byte      -  13

                  tStrData,  // struttura formata dalla dimensione del buffer
                             // ed il puntatore (32 + 32 bit)

                  tNegative = 1 << 30,
//                  tArray = 1 << 31,
               };
//----------------------------------------------------------------------------
class PRowTest
{
  public:

    PRowTest(PWin* parent, uint first_id, POINT pt, int whichRow);
    virtual ~PRowTest() {}

    void offset(int x, int y, double scaleX, double scaleY);

    void getDescr(LPTSTR buff) const;
    void setDescr(LPCTSTR buff) const;

    DWORD getIpAddr_Port(DWORD& port);
    void setIpAddr_Port(DWORD addr, DWORD port);

    DWORD getDB_Addr(DWORD& addr);
    void setDB_Addr(DWORD db, DWORD addr);

    DWORD getType_Action(DWORD& action);
    void setType_Action(DWORD type, DWORD action);

    rangeLP getRange();
    void setRange(const rangeLP& r);

    DWORD getValue();
    void setValue(DWORD value);

//    void getAll(DWORD& ipAddr, DWORD& port, DWORD& db, DWORD& addr, DWORD& type, DWORD& action,
//                rangeLP& vR, DWORD& value);
    void setAll(DWORD ipAddr, DWORD port, DWORD db, DWORD addr, DWORD type, DWORD action,
                const rangeLP& vR, DWORD value);

    bool hasId(uint id);
    void invalidate();
  protected:
    PStatic* IP_Port;
    PEditEmpty* Value;
    PStatic* Db_Addr;
    PStatic* Type_Action;
    PStatic* Descr;

    PStatic* Range;
    PStatic* RangeP;

    PWin* Owner;
    bool isHex();
    uint getLenBin();
    uint firstId;
    void invalidateNum(PWin* ctrl) const;
};
//----------------------------------------------------------------------------
inline
void PRowTest::setAll(DWORD ipAddr, DWORD port, DWORD db, DWORD addr, DWORD type, DWORD action,
            const rangeLP& vR, DWORD value)
{
  setIpAddr_Port(ipAddr, port);
  setDB_Addr(db, addr);
  setType_Action(type, action);
  setRange(vR);
  setValue(value);
}
//----------------------------------------------------------------------------
inline
void PRowTest::getDescr(LPTSTR buff) const
{
  GetWindowText(*Descr, buff, MAX_TEXT - 1);
}
//----------------------------------------------------------------------------
inline
void PRowTest::setDescr(LPCTSTR buff) const
{
  SetWindowText(*Descr, buff);
  invalidateNum(Descr);
}
//----------------------------------------------------------------------------
inline
void PRowTest::invalidateNum(PWin* ctrl) const
{
  PEditEmpty* pee = dynamic_cast<PEditEmpty*>(ctrl);
  if(pee)
    pee->invalidateNum();
  else
    InvalidateRect(*ctrl, 0, 1);
}
//----------------------------------------------------------------------------
//-----------------------------------------------------------
class PBinGroupFilter : public PBinaryFilter
{
  private:
    typedef PBinaryFilter baseClass;
  public:
    bool accept(UINT& key, HWND ctrl);
};
//----------------------------------------------------------------------------
#define YELLOW_COLOR  RGB(0xff,0xff,0xcf)
#define bkgColor YELLOW_COLOR
//----------------------------------------------------------------------------
#define CYAN_COLOR  RGB(0xdf,0xff,0xff)
#define bkgColor2 CYAN_COLOR
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define ACTIVE_ROW_COLOR  RGB(0xff,0xff,0xff)
#define ACTIVE_ROW_COLOR_TXT  RGB(0x7f,0x0,0x0)
#define bkgColor3 ACTIVE_ROW_COLOR
//------------------------------------------------------------------------------
#define ID_DEF_DATATYPE 1000000
#define ID_DEF_VER      1000001
#define ID_TIMER_TICK   1000002
#define ID_TYPEVAR_SHOW 1000003
//----------------------------------------------------------------------------
#define ADR_VER 7
#define STR_VER _T("7")
//----------------------------------------------------------------------------
int getNumField(LPCTSTR p);
int getNumField2(LPCTSTR p);
//----------------------------------------------------------------------------
// numero di campi penultima modifica
#define DEF_NUM_FIELD 6
//----------------------------------------------------------------------------
void addField(setOfString& Set, uint id, LPCTSTR p, int nField);
bool convertToLatest(setOfString& Set, LPTSTR Filename);
//----------------------------------------------------------------------------
#endif
