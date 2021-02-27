//-------- P_ModEdit.h -----------------------------------------
//-------------------------------------------------------------------
#ifndef P_ModEdit_H
#define P_ModEdit_H
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
//-------------------------------------------------------------------
#include "pdialog.h"
#include "p_freeList.h"
//-------------------------------------------------------------------
#define SIZE_SINGLE_BUFF 4096
//-------------------------------------------------------------------
#define MAX_LANG 10
//-------------------------------------------------------------------
class freeList
{
  public:
    freeList() : Free(SIZE_SINGLE_BUFF * sizeof(TCHAR)) {}

    LPTSTR get() { return (LPTSTR)Free.getFree(); }
    void release(LPCTSTR buff) { Free.addToFree((LPVOID)buff); }
  private:
    P_freeList Free;
};
//-------------------------------------------------------------------
extern freeList& getSetFree();
extern void releaseVect(PVect<LPCTSTR>& v);
extern void releaseVect(PVect<LPTSTR>& v);
//-------------------------------------------------------------------
struct fillInfo;
//-------------------------------------------------------------------
extern void setLang(HWND hwnd, uint idText);
extern void setLang(HWND hwnd, uint idc, uint idText);
extern LPCTSTR getStringOrDef(uint code, LPCTSTR def);
//-------------------------------------------------------------------
class P_ModEdit : public PDialog
{
  private:
    typedef PDialog baseClass;
  public:
    P_ModEdit(PWin * parent, uint id, HINSTANCE hinst = 0) :
        baseClass(parent, id, hinst), DirtyNameLang(false), font(0) { }
    ~P_ModEdit();

    bool create();

    void changedSel();
    void setTitle(PVect<LPTSTR>& buff);
    void setText(PVect<LPTSTR>& buff);
    void setSelLang(uint ix);

    void getTitle(PVect<LPTSTR>& buff);
    uint getSelLang() const;

    void toggleNameLang();

    bool isDirtyNameLang() const { return DirtyNameLang; }
    void resetDirtyNameLang() { DirtyNameLang = false; }

    void fillField(const fillInfo& fInfo);
  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    bool DirtyNameLang;
    HFONT font;
};
//-------------------------------------------------------------------
#endif
