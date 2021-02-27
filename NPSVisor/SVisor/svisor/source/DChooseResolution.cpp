//------- DChooseResolution.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
//----------------------------------------------------------------------------
#include "DChooseResolution.h"
#include "macro_utils.h"
#include "sizer.h"
#include "POwnBtnImageStd.h"
#include "language.h"
#include "1.h"
//----------------------------------------------------------------------------
bool resolutionByUser()
{
  const cfg& Cfg = config::getAll();
  return toBool(Cfg.ForceResolutionByUser);
}
//----------------------------------------------------------------------------
static sizer::eSizer checkResolution(HWND hwnd, const uint* idc, SIZE& sz)
{
  sizer::eSizer szr[] = {
    sizer::s640x480, sizer::s800x600, sizer::s1024x768, sizer::s1280x1024, sizer::s1440x900,
    sizer::s1600x1200, sizer::s1680x1050, sizer::s1920x1440, sizer::sPersonalized,
    };
  sz.cx = GetDlgItemInt(hwnd, IDC_EDIT_RES_WIDTH, 0, 1);
  sz.cy = GetDlgItemInt(hwnd, IDC_EDIT_RES_HEIGHT, 0, 1);

  sizer::eSizer res = sizer::sOutOfRange;
  for(uint i = 0; i < SIZE_A(szr); ++i)
    if(BST_CHECKED == SendMessage(GetDlgItem(hwnd, idc[i]), BM_GETCHECK, 0, 0)) {
      res = szr[i];
      break;
      }
  return res;
}
//----------------------------------------------------------------------------
static void initResolution(HWND hwnd, int lastRes, SIZE sz)
{
  uint idc[] = {
    IDC_RADIOBUTTON__640x480,
    IDC_RADIOBUTTON__800x600,
    IDC_RADIOBUTTON__1024x768,
    IDC_RADIOBUTTON__1280x1024,
    IDC_RADIOBUTTON__1440x900,
    IDC_RADIOBUTTON__1600x1200,
    IDC_RADIOBUTTON__1680x1050,
    IDC_RADIOBUTTON__1920x1440,
    IDC_RADIOBUTTON__RES_PERS,
    };
  int ix = sizer::sPersonalized == lastRes ? SIZE_A(idc) - 1 : lastRes;
  SendMessage(GetDlgItem(hwnd, idc[ix]), BM_SETCHECK, BST_CHECKED, 0);
  SetDlgItemInt(hwnd, IDC_EDIT_RES_WIDTH, sz.cx, 1);
  SetDlgItemInt(hwnd, IDC_EDIT_RES_HEIGHT, sz.cy, 1);
}
//----------------------------------------------------------------------------
DChooseResolution::DChooseResolution(PWin* parent, bool& changed, uint resId, HINSTANCE hinstance)
:
    baseClass(parent, resId, hinstance), Changed(changed)
{
  new svEdit(this, IDC_EDIT_RES_WIDTH);
  new svEdit(this, IDC_EDIT_RES_HEIGHT);
  new POwnBtnImageStd(this, IDC_BUTTON_F1, new PBitmap(IDB_OK, getHInstance()), POwnBtnImageStd::wLeft, true);
  new POwnBtnImageStd(this, IDC_BUTTON_F2, new PBitmap(IDB_CANC, getHInstance()), POwnBtnImageStd::wLeft, true);
}
//----------------------------------------------------------------------------
DChooseResolution::~DChooseResolution()
{
  destroy();
}
//----------------------------------------------------------------------------
bool DChooseResolution::create()
{
  if(!baseClass::create())
    return false;
  setCaption(getStringOrIdByLangGlob(ID_TEXT_CHOOSE_RES_TITLE));
  SET_TEXT(IDC_RADIOBUTTON__RES_PERS, getStringOrIdByLangGlob(ID_TEXT_CHOOSE_RES_PERSON));
  SET_TEXT(IDC_CHECK_FORCE_RES, getStringOrIdByLangGlob(ID_TEXT_CHOOSE_RES_FORCE));

  SIZE sz;
  uint res = sizer::getCurr(sz);
  initResolution(*this, res, sz);
  const cfg& Cfg = config::getAll();

  SET_CHECK_SET(IDC_CHECK_FORCE_RES, Cfg.ForceResolutionByUser);
  return true;
}
//----------------------------------------------------------------------------
#define EV_BN_CLICKED(id, fz) \
        case id:  \
          fz();   \
          break;
//----------------------------------------------------------------------------
LRESULT DChooseResolution::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        EV_BN_CLICKED(IDC_BUTTON_F1, BNClicked_F1)
        EV_BN_CLICKED(IDC_BUTTON_F2, BNClicked_F2)

        case IDC_EDIT_RES_WIDTH:
          if(EN_KILLFOCUS == HIWORD(wParam))
            checkValidWidth();
          break;
        case IDC_EDIT_RES_HEIGHT:
          if(EN_KILLFOCUS == HIWORD(wParam))
            checkValidHeight();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void DChooseResolution::checkValidWidth()
{
  int cx = GetSystemMetrics(SM_CXSCREEN);
  int v;
  GET_INT(IDC_EDIT_RES_WIDTH, v);
  if(v > cx)
    SET_INT(IDC_EDIT_RES_WIDTH, cx);
  int cy = GetSystemMetrics(SM_CYSCREEN);

}
//----------------------------------------------------------------------------
void DChooseResolution::checkValidHeight()
{
  int cy = GetSystemMetrics(SM_CYSCREEN);
  int v;
  GET_INT(IDC_EDIT_RES_HEIGHT, v);
  if(v > cy)
    SET_INT(IDC_EDIT_RES_HEIGHT, cy);
}
//----------------------------------------------------------------------------
static bool operator !=(const SIZE& sz1, const SIZE& sz2)
{
  return sz1.cx != sz2.cx || sz1.cy != sz2.cy;
}
//----------------------------------------------------------------------------
void DChooseResolution::BNClicked_F1()
{
  uint idc[] = {
    IDC_RADIOBUTTON__640x480,
    IDC_RADIOBUTTON__800x600,
    IDC_RADIOBUTTON__1024x768,
    IDC_RADIOBUTTON__1280x1024,
    IDC_RADIOBUTTON__1440x900,
    IDC_RADIOBUTTON__1600x1200,
    IDC_RADIOBUTTON__1680x1050,
    IDC_RADIOBUTTON__1920x1440,
    IDC_RADIOBUTTON__RES_PERS
    };
  SIZE oldsz;
  sizer::eSizer oldres = sizer::getCurr(oldsz);

  config Cfg;
  cfg& cfg_ = const_cast<cfg&>(Cfg.getAll());
  bool chg_user = IS_CHECKED(IDC_CHECK_FORCE_RES) ^ toBool(cfg_.ForceResolutionByUser);
  SIZE sz;
  sizer::eSizer res = checkResolution(*this, idc, sz);
  Changed = res != oldres || sz != oldsz || chg_user;
  if(Changed) // && (chg_user && IS_CHECKED(IDC_CHECK_FORCE_RES)))
    sizer::setDim(res, &sz);
  cfg_.ForceResolutionByUser = IS_CHECKED(IDC_CHECK_FORCE_RES);
  cfg_.ResolutionByUserType = res;
  cfg_.ResolutionByUserW = (WORD)sz.cx;
  cfg_.ResolutionByUserH = (WORD)sz.cy;
  Cfg.setup(cfg_);

  baseClass::CmOk();
}
//----------------------------------------------------------------------------
void DChooseResolution::BNClicked_F2()
{
  baseClass::CmCancel();
}
//----------------------------------------------------------------------------
