//-------------------- pEdit.h ---------------------------
//-----------------------------------------------------------
#ifndef PEDIT_H_
#define PEDIT_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#ifndef PCONTROL_H_
  #include "pControl.h"
#endif
//-----------------------------------------------------------
#include "setPack.h"
//-----------------------------------------------------------
// la classe è dichiarata qui per comodità e non esporre le classi
// più comuni derivate da questa. L'implementazione si trova comunque nel
// file PCommonFilter.cpp
//-----------------------------------------------------------
class PFilter
{
  public:
    PFilter() {}
    virtual ~PFilter() {}

    // viene chiamata dal controllo di edit dopo la creazione,
    // può servire per adattare la lunghezza del testo da immettere
    // alle caratteristiche del filtro
    virtual void initCtrl(HWND ctrl) {  }
    virtual bool accept(WPARAM& /*key*/, HWND /*ctrl*/) { return true; }

    // torna true se è uno dei codici VK_BACK, VK_RETURN, VK_TAB
    // il parametro [ctrl] non è usato, viene passato per eventuale uso
    // nelle classi derivate
    virtual bool isCommonKey(WPARAM& key, HWND ctrl);

    virtual void clone(const PFilter& other) { if(this != &other) *this = other; }
    virtual PFilter* allocCopy() const = 0;

    virtual void onKillFocus(HWND ctrl) {}
  protected:
    bool getSel(HWND ctrl, int& init, int& end);
    void addString(HWND ctrl, LPCTSTR str, int initPos, int endPos);
};
//-----------------------------------------------------------
class IMPORT_EXPORT PEdit : public PControl
{
  public:
    PEdit(PWin * parent, uint id, int x, int y, int w, int h,
              LPCTSTR text = 0, int textlen = 255, HINSTANCE hinst = 0);
    PEdit(PWin * parent, uint id, const PRect& r,
              LPCTSTR text = 0, int textlen = 255, HINSTANCE hinst = 0);
    PEdit(PWin * parent, uint resid, int textlen = 255, HINSTANCE hinst = 0);

    PEdit(HWND hwnd, HINSTANCE hInst) :  PControl(hwnd, hInst), TextLen(0),
      Filter(0), customPasteAndUndo(true) {}

    virtual ~PEdit();

    virtual bool create();

    void setFilter(PFilter* filter);

    int getTextLen() const { return TextLen; }
    void setTextLen(int newLen)  { TextLen = newLen; }

  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual LPCTSTR getClassName() const;

    virtual int verifyKey();
    // per default evKeyDown trasforma le frecce in basso e in alto
    // in tab e shift + tab, mentre evChar trasforma il return in tab.
    virtual bool evKeyDown(WPARAM& key);
    virtual bool evKeyUp(WPARAM& key);
    virtual bool evChar(WPARAM& key);

    const PFilter* getFilter();

    bool customPasteAndUndo;
  private:
    int TextLen;
    PFilter* Filter;
    typedef PControl baseClass;
};
//-----------------------------------------------------------
//-----------------------------------------------------------
//-----------------------------------------------------------
inline PEdit::PEdit(PWin * parent, uint id, int x, int y, int w, int h,
                    LPCTSTR text, int textlen, HINSTANCE hinst) :
      PControl(parent, id, x, y, w, h, text, hinst), TextLen(textlen),
      Filter(0), customPasteAndUndo(true)
{
  Attr.exStyle |= WS_EX_CLIENTEDGE;
  Attr.style |= ES_LEFT | ES_AUTOHSCROLL;
}
//-----------------------------------------------------------
inline PEdit::PEdit(PWin * parent, uint id, const PRect& r, LPCTSTR text,
                    int textlen, HINSTANCE hinst) :
      PControl(parent, id, r, text, hinst), TextLen(textlen),
      Filter(0), customPasteAndUndo(true)
{
  Attr.exStyle |= WS_EX_CLIENTEDGE;
  Attr.style |= ES_LEFT | ES_AUTOHSCROLL;
}
//-----------------------------------------------------------
inline PEdit::PEdit(PWin * parent, uint resid, int textlen, HINSTANCE hinst) :
        PControl(parent, resid, hinst), TextLen(textlen),
        Filter(0), customPasteAndUndo(true)
{ }
//-----------------------------------------------------------
inline PEdit::~PEdit()
{
  delete Filter;
}
//-----------------------------------------------------------
inline LPCTSTR PEdit::getClassName() const { return _T("edit"); }
//-----------------------------------------------------------
inline const PFilter* PEdit::getFilter()
{
  return Filter;
}
//-----------------------------------------------------------
#include "restorePack.h"
//-----------------------------------------------------------
#endif
