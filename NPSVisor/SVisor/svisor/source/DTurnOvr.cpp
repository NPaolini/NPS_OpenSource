//------- dturnovr.cpp -------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "dturnovr.h"
#include "p_file.h"
#include "p_Util.h"
#include "prfData.h"
//----------------------------------------------------------------------------
#define FILE_TURN _T("turn.dat")
//----------------------------------------------------------------------------
void setTurnOver(PWin *w)
{
  turnOver().setup(w);
}
//----------------------------------------------------------------------------
bool turnOver::updated = false;
lb_data turnOver::tData[MAX_TURN];
int turnOver::NumTurn;
//----------------------------------------------------------------------------
turnOver::turnOver()
{
  if(updated)
    return;
  ZeroMemory(&tData, sizeof(tData));
  NumTurn = 0;
  P_File f(FILE_TURN, P_READ_ONLY);
  if(f.P_open()) {
    f.P_read(&NumTurn, sizeof(NumTurn));
    for(int i = 0; i < NumTurn; ++i)
      f.P_read(&tData[i], sizeof(tData[i]));
    }
  updated = true;
}
//----------------------------------------------------------------------------
void turnOver::setup(PWin *w)
{
  lb_data t[MAX_TURN];
  memcpy(&t, &tData, sizeof(t));
  int n = NumTurn;
  if(TD_TurnOver(n, t, w).modal() == IDOK) {
    NumTurn = n;
    memcpy(&tData, &t, sizeof(t));
    P_File f(FILE_TURN, P_CREAT);
    if(f.P_open()) {
      f.P_write(&NumTurn, sizeof(NumTurn));
      for(int i = 0; i < NumTurn; ++i)
        f.P_write(&tData[i], sizeof(tData[i]));
      }
    }
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
TD_TurnOver::TD_TurnOver(int& n_turn, lb_data *t, PWin* parent, uint resId, HINSTANCE hinst)
:
    baseClass(parent, resId, hinst), Turn(t), nTurn(n_turn)
{
  CBX_type = new PComboBox(this, IDC_COMBOBOX_TURN);
  AOre = new svEdit(this, IDC_EDIT_A_ORE, DIM_ORA + 1);
  DaOre = new svEdit(this, IDC_EDIT_DA_ORE, DIM_ORA + 1);
  IdTurn = new svEdit(this, IDC_EDIT_ID_TURN, DIM_ID + 1);

  LB_Turn = new PListBox(this, IDC_LISTBOX_TURN, 12);

//  AOre->SetValidator(new TPXPictureValidator("[##:##]",true));
//  DaOre->SetValidator(new TPXPictureValidator("[##:##]",true));

  int tabs[] = { -(DIM_ID + 2), DIM_ORA, DIM_ORA, DIM_INFO };

  LB_Turn->SetTabStop(SIZE_A(tabs), tabs, 0);
  LB_Turn->setAlign(1, PListBox::aCenter);
  LB_Turn->setAlign(2, PListBox::aCenter);
}
//----------------------------------------------------------------------------
TD_TurnOver::~TD_TurnOver()
{
  destroy();
}
//----------------------------------------------------------------------------
#define SET_(d, a, v) d.a[SIZE_A(d.a)-1] = v
//----------------------------------------------------------------------------
#define TAB_(d, a) SET_(d, a, TAB)
//----------------------------------------------------------------------------
#define MAX_COUNT 9
//----------------------------------------------------------------------------
void TD_TurnOver::BNClickedAdd()
{
  if(SendMessage(*LB_Turn, LB_GETCOUNT, 0, 0) >= MAX_TURN) {
    svMessageBox(this, _T("Max da 1 a 9 turni"), _T("Errore, troppe immissioni"), MB_ICONSTOP | MB_OK);
    return;
    }

  lb_data dt;

  fillStr(dt.id, _T(' '), SIZE_A(dt.id));
  fillStr(dt.da, _T(' '), SIZE_A(dt.da));
  fillStr(dt.a, _T(' '), SIZE_A(dt.a));
  fillStr(dt.info, _T(' '), SIZE_A(dt.info));

  TCHAR t[DIM_INFO + 6];
  SetFocus(*IdTurn);

  GetWindowText(*IdTurn, t, SIZE_A(t));
  if(!*t)
    return;
  _tcsncpy_s(dt.id, SIZE_A(dt.id), t, _tcslen(t));

  GetWindowText(*DaOre, t, SIZE_A(t));
  if(!*t)
    return;
  _tcsncpy_s(dt.da, SIZE_A(dt.da), t, _tcslen(t));

  GetWindowText(*AOre, t, SIZE_A(t));
  if(!*t)
    return;
  _tcsncpy_s(dt.a, SIZE_A(dt.a), t, _tcslen(t));

  GetWindowText(*CBX_type, t, SIZE_A(t));
  if(!*t)
    return;
  _tcsncpy_s(dt.info, SIZE_A(dt.info), t, _tcslen(t));

  TAB_(dt, id);
  TAB_(dt, da);
  TAB_(dt, a);
  TAB_(dt, info);
  SendMessage(*LB_Turn, LB_ADDSTRING, 0, (LPARAM)&dt);
}
//----------------------------------------------------------------------------
void TD_TurnOver::BNClickedDel()
{
  int ix = SendMessage(*LB_Turn, LB_GETCURSEL, 0, 0);
  if(ix >= 0)
    SendMessage(*LB_Turn, LB_DELETESTRING, ix, 0);
}
//----------------------------------------------------------------------------
#define ZERO_(d, a) SET_(d, a, 0)
//----------------------------------------------------------------------------
#define MIN_TXT_CMP 6
//----------------------------------------------------------------------------
void TD_TurnOver::BNClickedMod()
{
  int ix = SendMessage(*LB_Turn, LB_GETCURSEL, 0, 0);
  if(ix >= 0) {
    lb_data dt;
    SendMessage(*LB_Turn, LB_GETTEXT, ix, (LPARAM)&dt);
    ZERO_(dt, id);
    ZERO_(dt, da);
    ZERO_(dt, a);
    ZERO_(dt, info);
    SetWindowText(*IdTurn, dt.id);
    SetWindowText(*DaOre, dt.da);
    SetWindowText(*AOre, dt.a);
    TCHAR old = dt.info[MIN_TXT_CMP];
    dt.info[MIN_TXT_CMP] = 0;
    SetWindowText(*CBX_type, dt.info);
    dt.info[MIN_TXT_CMP] = old;

    SendMessage(*LB_Turn, LB_DELETESTRING, ix, 0);
    }
}
//----------------------------------------------------------------------------
void TD_TurnOver::LBNDblclk()
{
  BNClickedMod();
}
//----------------------------------------------------------------------------
#define TAB_A_(a) TAB_(Turn[i], a)
//----------------------------------------------------------------------------
#define ZERO_A_(a) ZERO_(Turn[i], a)
//----------------------------------------------------------------------------
bool TD_TurnOver::create()
{
  if(!baseClass::create())
    return false;

  SendMessage(*CBX_type, CB_ADDSTRING, 0, (LPARAM)_T("Lavorativo"));
  SendMessage(*CBX_type, CB_ADDSTRING, 0, (LPARAM)_T("Riposo"));

  for(int i = 0; i < nTurn; ++i) {
    TAB_A_(id);
    TAB_A_(da);
    TAB_A_(a);
    TAB_A_(info);
    SendMessage(*LB_Turn, LB_ADDSTRING, 0, (LPARAM)&Turn[i]);
    }
  return true;
}
//----------------------------------------------------------------------------
void TD_TurnOver::BNClickedOK()
{
  nTurn = SendMessage(*LB_Turn, LB_GETCOUNT, 0, 0);
  if(nTurn < 0)
    nTurn = 0;
  for(int i = 0; i < nTurn; ++i) {
    SendMessage(*LB_Turn, LB_GETTEXT, i, (LPARAM)&Turn[i]);
    ZERO_A_(id);
    ZERO_A_(da);
    ZERO_A_(a);
    ZERO_A_(info);
    }
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
LRESULT TD_TurnOver::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_BUTTON_ADD_TURN:
          BNClickedAdd();
          break;
        case IDC_BUTTON_DEL_TURN:
          BNClickedDel();
          break;
        case IDC_BUTTON_MOD_TURN:
          BNClickedMod();
          break;
        case IDC_LISTBOX_TURN:
          if(LBN_DBLCLK == HIWORD(wParam))
            LBNDblclk();
          break;
        case IDOK:
          BNClickedOK();
          EndDialog(hwnd, IDOK);
          break;
        case IDCANCEL:
          EndDialog(hwnd, IDCANCEL);
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}

