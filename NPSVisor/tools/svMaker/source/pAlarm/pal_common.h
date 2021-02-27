//----------- pAl_common.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef pAl_common_H_
#define pAl_common_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "resource.h"
#include "pStatic.h"
#include "pEdit.h"
#include "PButton.h"
#include "PRadioBtn.h"
#include "p_util.h"
#include "p_txt.h"
#include "p_file.h"
#include "p_avl.h"
#include "pOwnBtnImageStd.h"
#include "pModDialog.h"
#include "pCommonFilter.h"
#include "macro_utils.h"
#include "headerMsg.h"
#include "language_util.h"
//----------------------------------------------------------------------------
#define WM_CLIENT_REQ (WM_FW_FIRST_FREE + 10)
  #define CM_SAVE_ALL 1

#define WM_CLIENT_WHEEL (WM_CLIENT_REQ + 1)
#define WM_CUSTOM_VSCROLL (WM_CLIENT_WHEEL + 1)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define FIRST_PERIPH 1
#define MAX_PERIPH 9
//----------------------------------------------------------------------------
void makeSystemSet(setOfString& set, LPCTSTR filename);
void makeSystemFilename(LPTSTR target, LPCTSTR filename);
//----------------------------------------------------------------------------
struct commonInfo
{
  DWORD currRow;
  DWORD currGotoRow;
  bool dirty;
};
//----------------------------------------------------------------------------
#define MAX_TEXT 2000
//----------------------------------------------------------------------------
#define MAX_DIM_ASSOC_FILE 64
//----------------------------------------------------------------------------
#define MAX_GROUP 16
#define MAX_ADDRESSES 4000
//----------------------------------------------------------------------------
typedef P_SmartPointer<LPTSTR> sPointerString;
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
    bool evKeyDown(UINT& key) {
      if(VK_RETURN == key)
        PostMessage(*getParent(), WM_COMMAND, MAKEWPARAM(cmdid, 0), 0);
      return false;
      }

    uint cmdid;
};
//----------------------------------------------------------------------------
struct baseRowAlarm
{
  DWORD grp;
  DWORD filter;
  TCHAR text[MAX_TEXT];
  void clear() { grp = 0; filter = 0; text[0] = 0; }
};
//----------------------------------------------------------------------------
struct baseRowAssoc
{
  DWORD prph;
  DWORD addr;
  DWORD type;
  DWORD nBit;
  DWORD offset;
  DWORD assocType;
  TCHAR assocFile[MAX_DIM_ASSOC_FILE];
  void clear() { prph = 0; addr = 0; type = 0; nBit = 0; offset = 0; assocType = 0; assocFile[0] = 0; }
};
//----------------------------------------------------------------------------
struct baseRowAssocFile
{
  DWORD value;
  TCHAR assocText[MAX_TEXT];
  void clear() { value = 0; assocText[0] = 0; }
};
//----------------------------------------------------------------------------
inline DWORD dwFromChar(TCHAR assocType) { return assocType; }
extern TCHAR charFromDw(DWORD assocType);
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define PAGE_SCROLL_LEN  MAX_GROUP
#define MAX_V_SCROLL (MAX_ADDRESSES - MAX_GROUP)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define IDC_EDIT_FIRST_FIELD 2000
//----------------------------------------------------------------------------
#define DEF_OFFSET_CTRL 100
//----------------------------------------------------------------------------
#define X_INIT 57
#define Y_INIT 85
//----------------------------------------------------------------------------
#define H_EDIT 19
//----------------------------------------------------------------------------
#define W_PRPH        39
#define W_ADDR        39
#define W_TYPE        33
#define W_NBIT        33
#define W_OFFS        54
#define W_BTN_ASSOC   54
#define W_TEXT_ASSOC 206
#define W_RADIO       33

#define W_GROUP       33
#define W_FILTER      33
#define W_ASSOCR      33
#define W_DESCR      273
//----------------------------------------------------------------------------
#define OFFSET_GROUP   0
#define OFFSET_FILTER (OFFSET_GROUP + DEF_OFFSET_CTRL)
#define OFFSET_ASSOC  (OFFSET_FILTER + DEF_OFFSET_CTRL)
#define OFFSET_DESCR  (OFFSET_ASSOC + DEF_OFFSET_CTRL)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define OFFSET_PRPH   0
#define OFFSET_ADDR  (OFFSET_PRPH + DEF_OFFSET_CTRL)
#define OFFSET_TYPE  (OFFSET_ADDR + DEF_OFFSET_CTRL)
#define OFFSET_NBIT  (OFFSET_TYPE + DEF_OFFSET_CTRL)
#define OFFSET_OFFS  (OFFSET_NBIT + DEF_OFFSET_CTRL)
#define OFFSET_BTN_ASSOC (OFFSET_OFFS + DEF_OFFSET_CTRL)
#define OFFSET_TEXT_ASSOC (OFFSET_BTN_ASSOC + DEF_OFFSET_CTRL)
#define OFFSET_RADIO (OFFSET_TEXT_ASSOC + DEF_OFFSET_CTRL)

#define LAST_OFFSET OFFSET_RADIO
//----------------------------------------------------------------------------
class PEditEmpty : public langCheckEdit
{
  private:
    typedef langCheckEdit baseClass;
  public:
    PEditEmpty(PWin * parent, uint id, const PRect& r, int textlen = 255, uint id2 = FORCE_NO_ALL,
              HINSTANCE hinst = 0):
        baseClass(parent, id, id2, r, 0, textlen, hinst), alsoSibling(0) {}

    void invalidateNum() const;
    void setSibling(int offs) { alsoSibling = offs; }
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    int alsoSibling;
};
//----------------------------------------------------------------------------
#define IS_THIS_CHANGED() SendMessage(*this, EM_GETMODIFY, 0, 0)
//----------------------------------------------------------------------------
struct infoPosCtrl
{
  int x;
  int y;
  int w;
  int h;
};
//----------------------------------------------------------------------------
class PRow_Base
{
  public:

    PRow_Base() {}
    virtual ~PRow_Base() {}

    void offset(PWin** ctrlSet, const PVect<infoPosCtrl>& ipc);
    virtual bool isDirty(bool reset = true) = 0;
    virtual void resetDirty(PWin** ctrl, uint nElem);
  protected:
    DWORD getGen(PWin* ctrl);
    void setGen(PWin* ctrl, DWORD val);
    bool isDirty(PWin** ctrl, uint nElem, bool reset);
};
//----------------------------------------------------------------------------
class PRow_Alarm : public PRow_Base
{
  private:
    typedef PRow_Base baseClass;
  public:

    PRow_Alarm(PWin* parent, uint first_id, POINT pt, int whichRow);
    virtual ~PRow_Alarm() {}

    void offset(const PVect<infoPosCtrl>& ipc);

    void getDescr(LPTSTR buff, size_t sz) const;
    void setDescr(LPCTSTR buff) const;

    DWORD getGroup();
    void setGroup(DWORD grp);

    DWORD getFilter();
    void setFilter(DWORD flt);

    void setAssoc(LPCTSTR p) const;

    void getAll(DWORD& group, DWORD& filter);
    void setAll(DWORD group, DWORD filter, LPCTSTR assoc);

    bool isDirty(bool reset = true);
    void resetDirty();
  protected:
    PEditEmpty* Group;
    PEditEmpty* Filter;
    PStatic* Assoc;
    PEditEmpty* Descr;

    uint firstId;
    void invalidateNum(PEditEmpty* ctrl) const;
};
//----------------------------------------------------------------------------
class PRow_Assoc : public PRow_Base
{
  private:
    typedef PRow_Base baseClass;
  public:

    PRow_Assoc(PWin* parent, uint first_id, POINT pt, int whichRow);
    virtual ~PRow_Assoc() {}

    void offset(const PVect<infoPosCtrl>& ipc);

    DWORD getPrph();
    void setPrph(DWORD prph);

    DWORD getAddr();
    void setAddr(DWORD addr);

    DWORD getType();
    void setType(DWORD type);

    DWORD getNBit();
    void setNBit(DWORD nbit);

    DWORD getOffs();
    void setOffs(DWORD offs);

    DWORD getAssoc();
    void setAssoc(DWORD assoc);

    void getFile(LPTSTR file);
    void setFile(LPCTSTR file);

    void getAll(DWORD& prph, DWORD& addr, DWORD& type, DWORD& nbit, DWORD& offs, DWORD& assoc, LPTSTR file);
    void setAll(DWORD prph, DWORD addr, DWORD type, DWORD nbit, DWORD offs, DWORD assoc, LPCTSTR file);

    bool isDirty(bool reset = true);
    void resetDirty();

    bool isSelected();
    void setSelected();
  protected:
    PEditEmpty* Prph;
    PEditEmpty* Addr;
    PEditEmpty* Type;
    PEditEmpty* nBit;
    PEditEmpty* Offs;
    PEditEmpty* Assoc;
    PEditEmpty* TextAssoc;

    PRadioButton* Radio;

    bool dirty;
    uint firstId;
    void invalidateNum(PEditEmpty* ctrl) const;
};
//----------------------------------------------------------------------------
class PRow_Assoc_File : public PRow_Base
{
  private:
    typedef PRow_Base baseClass;
  public:

    PRow_Assoc_File(PWin* parent, uint first_id, POINT pt, int whichRow);
    virtual ~PRow_Assoc_File() {}

    void offset(const PVect<infoPosCtrl>& ipc);

    void getDescr(LPTSTR buff, size_t sz) const;
    void setDescr(LPCTSTR buff) const;

    DWORD getValue();
    void setValue(DWORD val);

    bool isDirty(bool reset = true);
    void resetDirty();
  protected:
    PEditEmpty* Value;
    PEditEmpty* Descr;

    uint firstId;
    void invalidateNum(PEditEmpty* ctrl) const;
};
//----------------------------------------------------------------------------
class PageEdit_A : public PEditEmpty
{
  private:
    typedef PEditEmpty baseClass;
  public:
    PageEdit_A(PWin * parent, uint id, const PRect& r, int textlen = 255, uint id2 = FORCE_NO_ALL,
              HINSTANCE hinst = 0):
        PEditEmpty(parent, id, r, textlen, id2, hinst)
    {
      Attr.style |= ES_AUTOHSCROLL | ES_CENTER | ES_MULTILINE | ES_WANTRETURN;
      setFont((HFONT)GetStockObject(DEFAULT_GUI_FONT));
    }
  protected:
    virtual bool evKeyDown(WPARAM& key);
};
//----------------------------------------------------------------------------
#define MY_EN_CHANGE 0x1010
//----------------------------------------------------------------------------
class clickEdit : public PageEdit_A
{
  private:
    typedef PageEdit_A baseClass;
  public:
    clickEdit(PWin * parent, uint id, const PRect& r, int textlen = 255, uint id2 = FORCE_NO_ALL,
              HINSTANCE hinst = 0):
        PageEdit_A(parent, id, r, textlen, id2, hinst) { Attr.style |= ES_NOHIDESEL; }
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
    clickEditType(PWin * parent, uint id, const PRect& r, int textlen = 255, uint id2 = FORCE_NO_ALL,
              HINSTANCE hinst = 0):
        clickEdit(parent, id, r, textlen, id2, hinst) {}
  protected:
    virtual bool performChoose(LPTSTR buff);
};
//----------------------------------------------------------------------------
class clickEditAssoc : public clickEdit
{
  private:
    typedef clickEdit baseClass;
  public:
    clickEditAssoc(PWin * parent, uint id, const PRect& r, int textlen = 255, uint id2 = FORCE_NO_ALL,
              HINSTANCE hinst = 0):
        clickEdit(parent, id, r, textlen, id2, hinst) {}
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
              int textlen = 255, uint id2 = FORCE_NO_ALL, HINSTANCE hinst = 0):
        baseClass(parent, id, r, textlen, id2, hinst) {}
  protected:
    virtual bool evKeyDown(WPARAM& key);
};
//----------------------------------------------------------------------------
class clickEditAssocFirst : public clickEditAssoc
{
  private:
    typedef clickEditAssoc baseClass;
  public:
    clickEditAssocFirst(PWin * parent, uint id, const PRect& r,
              int textlen = 255, uint id2 = FORCE_NO_ALL, HINSTANCE hinst = 0):
        baseClass(parent, id, r, textlen, id2, hinst) {}
  protected:
    virtual bool evKeyDown(WPARAM& key);
};
//----------------------------------------------------------------------------
class clickEditTypeLast : public clickEditType
{
  private:
    typedef clickEditType baseClass;
  public:
    clickEditTypeLast(PWin * parent, uint id, const PRect& r,
              int textlen = 255, uint id2 = FORCE_NO_ALL, HINSTANCE hinst = 0):
        baseClass(parent, id, r, textlen, id2, hinst) {}
  protected:
    virtual bool evKeyDown(WPARAM& key);
};
//----------------------------------------------------------------------------
class clickEditAssocLast : public clickEditAssoc
{
  private:
    typedef clickEditAssoc baseClass;
  public:
    clickEditAssocLast(PWin * parent, uint id, const PRect& r,
              int textlen = 255, uint id2 = FORCE_NO_ALL, HINSTANCE hinst = 0):
        baseClass(parent, id, r, textlen, id2, hinst) {}
  protected:
    virtual bool evKeyDown(WPARAM& key);
};
//----------------------------------------------------------------------------
class firstEdit_A : public PageEdit_A
{
  private:
    typedef PageEdit_A baseClass;
  public:
    firstEdit_A(PWin * parent, uint id, const PRect& r, int textlen = 255, uint id2 = FORCE_NO_ALL,
              HINSTANCE hinst = 0):
        PageEdit_A(parent, id, r, textlen, id2, hinst) {}
  protected:
    virtual bool evKeyDown(WPARAM& key);
};
//----------------------------------------------------------------------------
#define MAX_EDIT_BYTE   4
#define MAX_EDIT_DWORD 12
#define MAX_EDIT_REAL  12
//----------------------------------------------------------------------------
class lastEdit_A : public PageEdit_A
{
  private:
    typedef PageEdit_A baseClass;
  public:
    lastEdit_A(PWin * parent, uint id, const PRect& r, int textlen = 255, uint id2 = FORCE_NO_ALL,
              HINSTANCE hinst = 0):
        PageEdit_A(parent, id, r, textlen, id2, hinst) {}
  protected:
    virtual bool evKeyDown(WPARAM& key);
};
//----------------------------------------------------------------------------
#define IS_CHANGED(hCtrl) SendMessage(hCtrl, EM_GETMODIFY, 0, 0)
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
class ClearChoose : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    ClearChoose(PWin* parent, int& from, int& to);
    ~ClearChoose();
    bool create();
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    void CmOk();
    void checkEnable();

  private:
    int& From;
    int& To;
    class PBitmap* Bmp[2];
};
//----------------------------------------------------------------------------
#include "common_inline.h"
//----------------------------------------------------------------------------
#endif
