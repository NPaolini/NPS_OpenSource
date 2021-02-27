//--------------- clipboard_control.h -----------------------------
#ifndef clipboard_control_H_
#define clipboard_control_H_
//-----------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------------
struct infoClipControl
{
  HWND hwnd;
  uint *itemSize;
  uint itemTot;
  LPTSTR buff;
  uint szBuff;

  infoClipControl(HWND hwnd = 0, uint *itemSize = 0, uint itemTot = 0) :
        hwnd(hwnd), itemSize(itemSize), itemTot(itemTot), buff(0), szBuff(0) {}
  ~infoClipControl() { delete []buff; }

  // nessun controllo sulla validità dei dati
  void calc_dim_buff_and_alloc() { szBuff = getMaxSize(); buff = new TCHAR[szBuff + 1]; }

  private:
    uint getMaxSize() { uint t = 0; for(uint i = 0; i < itemTot; ++i) t += itemSize[i] + 1; return t + 1; }
    NO_COPY_COSTR_OPER(infoClipControl)

};
//-------------------------------------------------------------------
enum eActionPaste {
  eap_append,     // aggiunge in coda
  eap_insert,     // aggiunge dal punto corrente
  eap_replase,    // sostituisce dal punto corrente
  eap_replaceAll  // rimpiazza la lista (elimina quelli presenti)
  };
//-------------------------------------------------------------------
class Clipboard_Control
{
  public:
    Clipboard_Control(PWin* owner) : Owner(owner) {}
    virtual ~Clipboard_Control() {}

    virtual void copyFromControl(infoClipControl& iLb);
    virtual void pasteToControl(infoClipControl& iLb);
    PWin* getOwner() { return Owner; }
  protected:
    PWin* Owner;
    virtual void fillRowByClipboard(LPCTSTR clip, infoClipControl& iLb);

    virtual LRESULT getCount(infoClipControl& iLb) = 0;
    virtual void getLine(infoClipControl& iLb, int line) = 0;
    virtual LRESULT getSel(infoClipControl& iLb) = 0;
    virtual void reset(infoClipControl& iLb) = 0;
    virtual void deleteLine(infoClipControl& iLb, int line) = 0;
    virtual void addLine(infoClipControl& iLb, int line) = 0;
    virtual LPTSTR prepareForClipboard(LPTSTR target, infoClipControl& iLb) = 0;
    virtual LPCTSTR makeForControl(LPCTSTR p, infoClipControl& iLb) = 0;

    virtual bool getChoosePaste(uint& choose) = 0;
};
//-------------------------------------------------------------------
class Clipboard_ListBox : public Clipboard_Control
{
  private:
    typedef Clipboard_Control baseClass;
  public:
    Clipboard_ListBox(PWin* owner) : baseClass(owner) {}
  protected:
    virtual LRESULT getCount(infoClipControl& iLb);
    virtual void getLine(infoClipControl& iLb, int line);
    virtual LRESULT getSel(infoClipControl& iLb);
    virtual void reset(infoClipControl& iLb);
    virtual void deleteLine(infoClipControl& iLb, int line);
    virtual void addLine(infoClipControl& iLb, int line);
    virtual LPTSTR prepareForClipboard(LPTSTR target, infoClipControl& iLb);
    virtual LPCTSTR makeForControl(LPCTSTR p, infoClipControl& iLb);
};
//-------------------------------------------------------------------
#include "restorePack.h"
#endif
