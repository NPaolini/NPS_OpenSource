//----------- PDiagOffset.cpp ----------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "PDiagOffset.h"
//----------------------------------------------------------------------------
PDiagOffset::PDiagOffset(PWin* parent, infoOffsetReplace& ior, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance),  i_o_r(ior)
{
}
//----------------------------------------------------------------------------
PDiagOffset::~PDiagOffset()
{
}
//----------------------------------------------------------------------------
bool PDiagOffset::create()
{
  if(!baseClass::create())
    return false;
  SET_INT(IDC_EDIT_OFFSET, i_o_r.offs);
  SET_INT(IDC_EDIT_OFFSET_BIT, i_o_r.bitOffs);
  if(isSetBitf(svmObject::eoAlsoVisibility, i_o_r.flag))
    SET_CHECK(IDC_CHECK_VISIB);
  if(isSetBitf(svmObject::eoAlsoOther, i_o_r.flag))
    SET_CHECK(IDC_CHECK_OTHER);
  if(isSetBitf(svmObject::eoExcludePrph1, i_o_r.flag))
    SET_CHECK(IDC_CHECK_NO_PRPH1);
  if(isSetBitf(svmObject::eoProgrBitOffs, i_o_r.flag))
    SET_CHECK(IDC_CHECK_OFFS_PROGR);
  if(isSetBitf(svmObject::eoProgrAddr, i_o_r.flag))
    SET_CHECK(IDC_CHECK_OFFS_PROGR_ADDR);

  SET_INT(IDC_EDIT_PRPH_FROM, i_o_r.prphFrom);
  SET_INT(IDC_EDIT_PRPH_TO, i_o_r.prphTo);
  if(isSetBitf(svmObject::eoUseAddr, i_o_r.flag))
    SET_CHECK(IDC_CHECK_USE_ADDR);
  if(isSetBitf(svmObject::eoUsePrph, i_o_r.flag))
    SET_CHECK(IDC_CHECK_USE_PRPH);
  if(isSetBitf(svmObject::eoAlsoPrphVisibility, i_o_r.flag))
    SET_CHECK(IDC_CHECK_VISIB_PRPH);

  SET_INT(IDC_EDIT_SIMPLE_TXT, i_o_r.offsTxt);
  if(isSetBitf(svmObject::eoUseSimpleText, i_o_r.flag))
    SET_CHECK(IDC_CHECK_SIMPLE_TXT);

  SET_INT(IDC_EDIT_PRPH_FILTER, i_o_r.prphFilterOnAddr);
  if(isSetBitf(svmObject::eoOnlyOnePrphOnUseAddr, i_o_r.flag))
    SET_CHECK(IDC_CHECK_PRPH_FILTER);

  SET_INT(IDC_EDIT_PRPH_FROM2, i_o_r.prphFrom2);
  SET_INT(IDC_EDIT_PRPH_TO2, i_o_r.prphTo2);
  SET_INT(IDC_EDIT_ADDR_FROM, i_o_r.addrFrom);
  SET_INT(IDC_EDIT_ADDR_TO, i_o_r.addrTo);
  if(isSetBitf(svmObject::eoSubstVar, i_o_r.flag))
    SET_CHECK(IDC_CHECK_SUBST_VAR);

  enableAddr();
  enablePrph();
  enableSimpleText();
  enableVar();

  return true;
}
//----------------------------------------------------------------------------
LRESULT PDiagOffset::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {
        case IDC_CHECK_USE_ADDR:
        case IDC_CHECK_PRPH_FILTER:
          enableAddr();
          break;
        case IDC_CHECK_USE_PRPH:
          enablePrph();
          break;
        case IDC_CHECK_SIMPLE_TXT:
          enableSimpleText();
          break;
        case IDC_CHECK_SUBST_VAR:
          enableVar();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
static void enableGen(HWND parent, uint* idc, uint nElem, bool enable)
{
  for(uint i = 0; i < nElem; ++i)
    EnableWindow(GetDlgItem(parent, idc[i]), enable);
}
//----------------------------------------------------------------------------
void PDiagOffset::enableAddr()
{
  uint idc[] = { IDC_EDIT_OFFSET, IDC_CHECK_VISIB, IDC_CHECK_OTHER, IDC_CHECK_NO_PRPH1, IDC_CHECK_PRPH_FILTER,
                 IDC_EDIT_PRPH_FILTER, IDC_EDIT_OFFSET_BIT, IDC_CHECK_OFFS_PROGR, IDC_CHECK_OFFS_PROGR_ADDR };
  bool enable = IS_CHECKED(IDC_CHECK_USE_ADDR);
  enableGen(*this, idc, SIZE_A(idc), enable);
  if(enable)
    EnableWindow(GetDlgItem(*this, IDC_EDIT_PRPH_FILTER), IS_CHECKED(IDC_CHECK_PRPH_FILTER));
}
//----------------------------------------------------------------------------
void PDiagOffset::enablePrph()
{
  uint idc[] = { IDC_EDIT_PRPH_FROM, IDC_EDIT_PRPH_TO, IDC_CHECK_VISIB_PRPH };
  enableGen(*this, idc, SIZE_A(idc), IS_CHECKED(IDC_CHECK_USE_PRPH));
}
//----------------------------------------------------------------------------
void PDiagOffset::enableSimpleText()
{
  uint idc[] = { IDC_EDIT_SIMPLE_TXT };
  enableGen(*this, idc, SIZE_A(idc), IS_CHECKED(IDC_CHECK_SIMPLE_TXT));
}
//----------------------------------------------------------------------------
void PDiagOffset::enableVar()
{
  uint idc[] = { IDC_EDIT_PRPH_FROM2, IDC_EDIT_PRPH_TO2, IDC_EDIT_ADDR_FROM, IDC_EDIT_ADDR_TO };
  enableGen(*this, idc, SIZE_A(idc), IS_CHECKED(IDC_CHECK_SUBST_VAR));
}
//----------------------------------------------------------------------------
void PDiagOffset::CmOk()
{
  GET_INT(IDC_EDIT_OFFSET, i_o_r.offs);
  GET_INT(IDC_EDIT_OFFSET_BIT, i_o_r.bitOffs);
  i_o_r.flag = 0;
  if(IS_CHECKED(IDC_CHECK_VISIB))
    setBitf(i_o_r.flag, svmObject::eoAlsoVisibility);

  if(IS_CHECKED(IDC_CHECK_OTHER))
    setBitf(i_o_r.flag, svmObject::eoAlsoOther);

  if(IS_CHECKED(IDC_CHECK_NO_PRPH1))
    setBitf(i_o_r.flag, svmObject::eoExcludePrph1);

  if(IS_CHECKED(IDC_CHECK_OFFS_PROGR))
    setBitf(i_o_r.flag, svmObject::eoProgrBitOffs);
  if(IS_CHECKED(IDC_CHECK_OFFS_PROGR_ADDR))
    setBitf(i_o_r.flag, svmObject::eoProgrAddr);

  GET_INT(IDC_EDIT_PRPH_FROM, i_o_r.prphFrom);
  GET_INT(IDC_EDIT_PRPH_TO, i_o_r.prphTo);

  bool usePrph = false;
  if(IS_CHECKED(IDC_CHECK_USE_ADDR))
    setBitf(i_o_r.flag, svmObject::eoUseAddr);
  else if(IS_CHECKED(IDC_CHECK_USE_PRPH)) {
    setBitf(i_o_r.flag, svmObject::eoUsePrph);
    usePrph = true;
    }
  if(IS_CHECKED(IDC_CHECK_VISIB_PRPH))
    setBitf(i_o_r.flag, svmObject::eoAlsoPrphVisibility);

  if(usePrph && (i_o_r.prphFrom < 1 || i_o_r.prphFrom > 20)) {
    MessageBox(*this, _T("Valore errato!"), _T("'Periferica Da' fuori range"), MB_OK | MB_ICONSTOP);
    return;
    }
  if(usePrph && (i_o_r.prphTo < 1 || i_o_r.prphTo > 20)) {
    MessageBox(*this, _T("Valore errato!"), _T("'Periferica A' fuori range"), MB_OK | MB_ICONSTOP);
    return;
    }
  if(usePrph && (i_o_r.prphTo < 1 || i_o_r.prphTo > 20 || i_o_r.prphFrom == i_o_r.prphTo)) {
    MessageBox(*this, _T("Valore errato!"), _T("Le Periferiche non possono coincidere"), MB_OK | MB_ICONSTOP);
    return;
    }

  GET_INT(IDC_EDIT_SIMPLE_TXT, i_o_r.offsTxt);
  if(IS_CHECKED(IDC_CHECK_SIMPLE_TXT))
    setBitf(i_o_r.flag, svmObject::eoUseSimpleText);

  GET_INT(IDC_EDIT_PRPH_FILTER, i_o_r.prphFilterOnAddr);
  if(IS_CHECKED(IDC_CHECK_PRPH_FILTER))
    setBitf(i_o_r.flag, svmObject::eoOnlyOnePrphOnUseAddr);

  GET_INT(IDC_EDIT_PRPH_FROM2, i_o_r.prphFrom2);
  GET_INT(IDC_EDIT_PRPH_TO2, i_o_r.prphTo2);
  GET_INT(IDC_EDIT_ADDR_FROM, i_o_r.addrFrom);
  GET_INT(IDC_EDIT_ADDR_TO, i_o_r.addrTo);
    if(IS_CHECKED(IDC_CHECK_SUBST_VAR))
      setBitf(i_o_r.flag, svmObject::eoSubstVar);

  baseClass::CmOk();
}
//----------------------------------------------------------------------------
