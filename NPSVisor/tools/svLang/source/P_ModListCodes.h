//-------- P_ModListCodes.h -----------------------------------------
//-------------------------------------------------------------------
#ifndef P_ModListCodes_H
#define P_ModListCodes_H
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
//-------------------------------------------------------------------
#include "fullView.h"
#include "pListview.h"
//-------------------------------------------------------------------
struct fillInfo;
//-------------------------------------------------------------------
class P_ModListCodes : public fullView
{
  private:
    typedef fullView baseClass;
  public:
    P_ModListCodes(PWin* parent, LPCTSTR title = 0, HINSTANCE hInst = 0) :
        baseClass(parent, title, hInst), Dirty(false), currSel(-1) { }

    P_ModListCodes(PWin * parent, uint id, int x = 0, int y = 0, int w = 0, int h = 0, LPCTSTR title = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, x, y, w, h, title, hinst), Dirty(false), currSel(-1) { }

    P_ModListCodes(PWin * parent, uint id, const PRect& r, LPCTSTR title = 0, HINSTANCE hinst = 0) :
        baseClass(parent, id, r, title, hinst), Dirty(false), currSel(-1) { }

      ~P_ModListCodes();
    bool create();

    virtual void refresh();

    bool isDirty() const { return Dirty; }
    void resetDirty() { Dirty = false; }
    void setTitle(PVect<LPTSTR>& allTitle);
    void setText(PVect<LPTSTR>& allText);

    void getText(PVect<LPTSTR>& allText);

    HWND getHWLv() { return *List; }

    void setNext();
    void setPrev();

    void fillField(const fillInfo& fInfo);
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HWND getHWClient() { return *List; }

    PListView* List;
    bool Dirty;

    int currSel;
    void chgSel(bool force = false);
    void performFillText(PVect<LPTSTR>& allText, const fillInfo& fInfo);
};
//-------------------------------------------------------------------
#endif
