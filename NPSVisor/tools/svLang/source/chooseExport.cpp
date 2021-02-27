//----------- chooseExport.cpp -----------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <process.h>
#include <Userenv.h>
#include <Sddl.h>
#include <shlobj.h>
//----------------------------------------------------------------------------
#include "chooseExport.h"
#include "p_util.h"
#include "p_date.h"
#include "macro_utils.h"
#include "POwnBtnImageStd.h"
#include "P_ModEdit.h"
//----------------------------------------------------------------------------
class myListView : public PListView
{
  private:
    typedef PListView baseClass;
  public:
    myListView(PWin * parent, uint resid, HINSTANCE hinst = 0) :
        baseClass(parent, resid, hinst) {}
    ~myListView() { destroy(); }
  protected:
    virtual int verifyKey() {
      if(GetKeyState(VK_RETURN)& 0x8000)
        return VK_RETURN;
      return 0;
      }
};
//----------------------------------------------------------------------------
PD_chooseExport::PD_chooseExport(PWin* parent, PVect<LPCTSTR>& filelist, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance), beginDrag(0), onRun(false), filelist(filelist)
{
  lbAllDisp = new myListView(this, IDC_LIST_EXIST);
  lbToExport = new myListView(this, IDC_LIST_TO_EXPORT);

  uint idBtnMove[] = {
       IDC_BUTTON_ADD_ALL,
       IDC_BUTTON_ADD,
       IDC_BUTTON_REM,
       IDC_BUTTON_REM_ALL
     };

  for(uint i = 0; i < SIZE_A(idBtnMove); ++i)
    new POwnBtnImageStd(this, idBtnMove[i], (PBitmap*)0);
  int idBmp[] = { IDB_OK, IDB_DONE };
  int idBtn[] = { IDOK, IDCANCEL };

  for(uint i = 0; i < SIZE_A(idBmp); ++i) {
    PBitmap* Bmp = new PBitmap(idBmp[i], getHInstance());
    new POwnBtnImageStd(this, idBtn[i], Bmp, POwnBtnImageStd::wLeft, true);
    }
}
//----------------------------------------------------------------------------
PD_chooseExport::~PD_chooseExport()
{
  destroy();
}
//----------------------------------------------------------------------------
static void setDefStyle(HWND obj)
{
  SendMessage (obj, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
  DWORD dwStyle = GetWindowLong (obj, GWL_STYLE);
  SetWindowLong (obj, GWL_STYLE, dwStyle | LVS_SHOWSELALWAYS);
}
//----------------------------------------------------------------------------
static void setLvColor(HWND obj, COLORREF fg, COLORREF bg)
{
  ListView_SetBkColor(obj, bg);
  ListView_SetTextBkColor(obj, bg);
  ListView_SetTextColor(obj, fg);
}
//----------------------------------------------------------------------------
bool PD_chooseExport::create()
{
  if(!baseClass::create())
    return false;

  setLang(*this, 60);
  setLang(*this, IDOK, 58);
  setLang(*this, IDCANCEL, 28);

  PRect r;
  GetClientRect(*lbAllDisp, r);
  setDefStyle(*lbAllDisp);
  setDefStyle(*lbToExport);
  setLvColor(*lbAllDisp, RGB(0, 0, 100), RGB(255, 255, 200));
  setLvColor(*lbToExport, RGB(100, 0, 0), RGB(255, 240, 220));

  LVCOLUMN lvc;
  lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
  lvc.fmt = LVCFMT_LEFT;
  lvc.cx = r.Width() - GetSystemMetrics(SM_CXVSCROLL) - GetSystemMetrics(SM_CXBORDER) * 2 - 10;

  lvc.pszText = (LPTSTR)getStringOrDef(61, _T("File esistenti"));
  if(ListView_InsertColumn (*lbAllDisp, 0, &lvc) < 0)
    return false;
  lvc.pszText = (LPTSTR)getStringOrDef(62, _T("File da esportare"));
  if(ListView_InsertColumn (*lbToExport, 0, &lvc) < 0)
    return false;

  fill();
  SetFocus(*lbAllDisp);
  return true;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
LRESULT PD_chooseExport::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {

    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_ADD_ALL:
          addAll(lbToExport, lbAllDisp);
          break;

        case IDC_BUTTON_ADD:
          addLB(lbToExport, lbAllDisp);
          break;

        case IDC_BUTTON_REM:
          addLB(lbAllDisp, lbToExport);
          break;

        case IDC_BUTTON_REM_ALL:
          addAll(lbAllDisp, lbToExport);
          break;
        }
      break;
    case WM_MOUSEMOVE:
      if(beginDrag) {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        HWND hw = ChildWindowFromPoint(*this, pt);
        if(2 == beginDrag) {
          if(*lbAllDisp == hw)
            SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURSOR_PAN)));
          else
            SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURSOR_PAN_MOVE)));
          }
        else if(1 == beginDrag) {
          if(*lbToExport == hw)
            SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURSOR_PAN)));
          else
            SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURSOR_PAN_MOVE)));
          }
        }
      break;
    case WM_LBUTTONUP:
      if(beginDrag) {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        HWND hw = ChildWindowFromPoint(*this, pt);
        if(2 == beginDrag) {
          if(*lbAllDisp == hw)
            addLB(lbAllDisp, lbToExport);
          }
        else if(1 == beginDrag) {
          if(*lbToExport == hw)
            addLB(lbToExport, lbAllDisp);
          }
        beginDrag = 0;
        ReleaseCapture();
        SetCursor(LoadCursor(0, IDC_ARROW));
        }
      break;
    case WM_NOTIFY:
      switch(((LPNMHDR)lParam)->code) {
        case LVN_KEYDOWN:
          if(VK_RETURN != ((LPNMLVKEYDOWN)lParam)->wVKey)
            break;
          // fall through
        case NM_DBLCLK:
          switch(((LPNMHDR)lParam)->idFrom) {
            case IDC_LIST_EXIST:
              addLB(lbToExport, lbAllDisp);
              break;
            case IDC_LIST_TO_EXPORT:
              addLB(lbAllDisp, lbToExport);
              break;
            }
          break;
        case LVN_BEGINDRAG:
          switch(((LPNMHDR)lParam)->idFrom) {
            case IDC_LIST_EXIST:
              beginDrag = 1;
              SetCapture(*this);
              SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURSOR_PAN)));
              break;
            case IDC_LIST_TO_EXPORT:
              beginDrag = 2;
              SetCapture(*this);
              SetCursor(LoadCursor(getHInstance(),  MAKEINTRESOURCE(IDC_CURSOR_PAN)));
              break;
            }
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void PD_chooseExport::fill()
{
  ListView_DeleteAllItems(*lbAllDisp);
  ListView_DeleteAllItems(*lbToExport);
  LVITEM lvI;
  lvI.mask = LVIF_TEXT;
  lvI.iSubItem = 0;
  lvI.iItem = 0;

  uint nElem = filelist.getElem();
  for(uint i = 0; i < nElem; ++i) {
    lvI.pszText = (LPTSTR)filelist[i];
    lvI.iItem = ListView_InsertItem(*lbAllDisp, &lvI);
    if(lvI.iItem < 0) {
      return;
      }
    }
}
//------------------------------------------------------------------------------
void PD_chooseExport::addAll(PListView* target, PListView* source)
{
  int count = ListView_GetItemCount(*source);
  if(0 >= count)
    return;
  TCHAR buff[_MAX_PATH];
  LVITEM lvI;
  lvI.mask = LVIF_TEXT;
  lvI.iItem = 0;
  lvI.cchTextMax = SIZE_A(buff);
  lvI.pszText = buff;
  lvI.iSubItem = 0;

  for(int i = 0; i < count; ++i) {
    lvI.iItem = i;
    if(!ListView_GetItem(*source, &lvI))
      return;
    lvI.iItem = 65535;
    if(ListView_InsertItem(*target, &lvI) < 0)
      return;
    }
  ListView_DeleteAllItems(*source);
}
//----------------------------------------------------------------------------
void PD_chooseExport::addLB(PListView* target, PListView* source)
{
  int sel = ListView_GetNextItem(*source, -1, LVNI_SELECTED);
  if(-1 == sel)
    return;
  PVect<int> selected;
  selected[0] = sel;
  while(true) {
    sel = ListView_GetNextItem(*source, sel, LVNI_SELECTED);
    if(-1 == sel)
      break;
    selected[selected.getElem()] = sel;
    }
  int count = selected.getElem();

  TCHAR buff[_MAX_PATH];
  LVITEM lvI;
  lvI.mask = LVIF_TEXT;
  lvI.iItem = 0;
  lvI.cchTextMax = SIZE_A(buff);
  lvI.pszText = buff;
  lvI.iSubItem = 0;

  for(int i = count - 1; i >= 0; --i) {
    lvI.iItem = selected[i];
    if(!ListView_GetItem(*source, &lvI))
      return;
    ListView_DeleteItem(*source, selected[i]);
    lvI.iItem = 65535;
    if(ListView_InsertItem(*target, &lvI) < 0)
      return;
    }
}
//----------------------------------------------------------------------------
void PD_chooseExport::CmOk()
{
  releaseVect(filelist);

  int count = ListView_GetItemCount(*lbToExport);
  if(0 >= count)
    return;

  TCHAR buff[_MAX_PATH];
  LVITEM lvI;
  lvI.mask = LVIF_TEXT;
  lvI.iItem = 0;
  lvI.cchTextMax = SIZE_A(buff);
  lvI.pszText = buff;
  lvI.iSubItem = 0;

  freeList& SetFree = getSetFree();
  for(int i = 0; i < count; ++i) {
    lvI.iItem = i;
    if(!ListView_GetItem(*lbToExport, &lvI))
      return;
    LPTSTR tmp = SetFree.get();
    _tcscpy_s(tmp, SIZE_SINGLE_BUFF, lvI.pszText);
    filelist[i] = tmp;
    }
  baseClass::CmOk();
}
//----------------------------------------------------------------------------
