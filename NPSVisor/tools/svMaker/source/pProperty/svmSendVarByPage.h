//-------------------- svmSendVarByPage.h --------------------
//-----------------------------------------------------------
#ifndef svmSendVarByPage_H_
#define svmSendVarByPage_H_
//-----------------------------------------------------------
#include "precHeader.h"
#include "PModDialog.h"
#include "P_txt.h"
#include "resource.h"
//-----------------------------------------------------------
#define MAX_DIM_TXT_WRAP  255
struct wrapItem
{
  wrapItem() { V[0] = 0; }
  TCHAR V[MAX_DIM_TXT_WRAP];
};
//-----------------------------------------------------------
class svmSendVarByPage : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    svmSendVarByPage(setOfString& set, PWin* parent, uint id = IDD_SEND_OPEN_CLOSE, HINSTANCE hInst = 0);
    ~svmSendVarByPage();

    virtual bool create();
  protected:
    setOfString& Set;
    class PListBox* LB[2];

    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    void fill();
    enum eLb { esOpen, esClose };
    void add(eLb which);
    void rem(eLb which);
    void move(eLb which, bool up);

    bool fillRowByEdit(LPTSTR target);
    LPCTSTR fillRowByStr(LPTSTR target, LPCTSTR source);
    void unfillRow(PVect<wrapItem>& target, LPCTSTR p);
    void appendVars(LPTSTR target, LPCTSTR source);
    void replaceVars(eLb which);
    void fillLb(LPCTSTR p, HWND hwlb);

};
//-----------------------------------------------------------
#endif
