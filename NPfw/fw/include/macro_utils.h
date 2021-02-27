#ifndef MACRO_UTILS_H_
#define MACRO_UTILS_H_
//----------------------------------------------------------------------------
#define IS_CHECKED_HW(hw,idc) \
    (BST_CHECKED == SendMessage(::GetDlgItem(hw, idc), BM_GETCHECK, 0, 0))
//----------------------------------------------------------------------------
#define IS_CHECKED(idc) \
    (BST_CHECKED == SendMessage(::GetDlgItem(*this, idc), BM_GETCHECK, 0, 0))
//----------------------------------------------------------------------------
#define IS_CHECKED_THIS(ctrl) \
    (BST_CHECKED == SendMessage(*ctrl, BM_GETCHECK, 0, 0))
//----------------------------------------------------------------------------
#define IS_PUSHED(idc) \
    (BST_PUSHED == SendMessage(::GetDlgItem(*this, idc), BM_GETSTATE, 0, 0))
//----------------------------------------------------------------------------
#define IS_PUSHED_THIS(ctrl) \
    (BST_PUSHED == SendMessage(*ctrl, BM_GETSTATE, 0, 0))
//----------------------------------------------------------------------------
#define IS_PUSHED_SWITCH(btn)  (POwnBtnSwitch::ON == btn->getState())
//----------------------------------------------------------------------------
#define SET_CHECK_THIS(ctrl) \
    SendMessage(*ctrl, BM_SETCHECK, BST_CHECKED, 0)
//----------------------------------------------------------------------------
#define SET_CHECK(idc) \
    SendMessage(::GetDlgItem(*this, idc), BM_SETCHECK, BST_CHECKED, 0)
//----------------------------------------------------------------------------
#define SET_CHECK_SET(idc, set) \
    SendMessage(::GetDlgItem(*this, idc), BM_SETCHECK, (set) ? BST_CHECKED : BST_UNCHECKED, 0)
//----------------------------------------------------------------------------
#define SET_CHECK_UNCHECK(idcheck, iduncheck) \
  {  SendMessage(::GetDlgItem(*this, idcheck), BM_SETCHECK, BST_CHECKED, 0); \
     SendMessage(::GetDlgItem(*this, iduncheck), BM_SETCHECK, BST_UNCHECKED, 0); }
//----------------------------------------------------------------------------
#define ENABLE(idc, set) \
    EnableWindow(::GetDlgItem(*this, idc), set)
//----------------------------------------------------------------------------
#define ENABLE_HW(hw, idc, set) \
    EnableWindow(::GetDlgItem(hw, idc), set)
//----------------------------------------------------------------------------
#define SET_TEXT(idc, txt) \
    SetDlgItemText(*this, idc, txt)
//----------------------------------------------------------------------------
#define SET_INT(idc, val) \
    SetDlgItemInt(*this, idc, (val), 1)
//----------------------------------------------------------------------------
#define GET_P_TEXT(idc, txt, len) \
    GetDlgItemText(*this, idc, txt, len)
//----------------------------------------------------------------------------
#define GET_TEXT(idc, txt) \
    GET_P_TEXT(idc, txt, SIZE_A(txt))
//----------------------------------------------------------------------------
#define GET_INT(idc, val) \
    val = GetDlgItemInt(*this, idc, 0, 1)
//----------------------------------------------------------------------------
#define GET_TEXTLENGHT(idc) \
    GetWindowTextLength(::GetDlgItem(*this, idc))
//----------------------------------------------------------------------------
#endif