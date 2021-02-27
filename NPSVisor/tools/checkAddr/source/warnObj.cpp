//----------- allPages.cpp ----------------------------------------------------
//----------------------------------------------------------------------------
//#define ISOLATION_AWARE_ENABLED 1
//#include <windows.h>
//#include <commctrl.h>
#include "precHeader.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#include <stdio.h>
//----------------------------------------------------------------------------
#include "warnObj.h"
#include "HeaderMsg.h"
//----------------------------------------------------------------------------
//---------------------------------------------------------------------------
struct infoColumn
{
  LPCTSTR title;
  int sz;
};
//----------------------------------------------------------------------------
static
infoColumn titleList[] = {
  { _T("Obj"), 100 },
  { _T("X"), 60 },
  { _T("Y"), 60 },
  { _T("Prph"), 50 },
  { _T("Addr"), 60 },
  { _T("Type"), 90 },
  { _T("nBits"), 50 },
  { _T("Offs"),  50 },
  { _T("Page"), 220 },
  { _T("Action"), 150 }
};
//----------------------------------------------------------------------------
#define TOT_COLUMN SIZE_A(titleList)
//-----------------------------------------------------------
PD_WarnObj::PD_WarnObj(PWin* parent, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance)
{

}
//----------------------------------------------------------------------------
void PD_WarnObj::fillTitle()
{
  LVCOLUMN lvc;
  lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

  for(uint i = 0; i < TOT_COLUMN; ++i) {
    lvc.iSubItem = i;
    lvc.pszText = (LPTSTR)titleList[i].title;
    lvc.cx = titleList[i].sz;
    if(i != TOT_COLUMN - 1)
      lvc.fmt = LVCFMT_CENTER;
    else
      lvc.fmt = LVCFMT_LEFT;
    if(-1 == ListView_InsertColumn(*getList(), i, &lvc))
      return;
    }
  DWORD dwExStyle = LVS_EX_FLATSB | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;// | LVS_EX_SUBITEMIMAGES;
  ListView_SetExtendedListViewStyle(*getList(), dwExStyle);

#define LV_TXT_COLOR RGB(0x60, 0, 0)
//#define LV_BKG_COLOR RGB(0xA7, 0xD4, 0xF1)
//#define LV_BKG_COLOR RGB(0xf0, 0xff, 0xff)
#define LV_BKG_COLOR RGB(0xff, 0xff, 0xd0)
  ListView_SetBkColor(*getList(), LV_BKG_COLOR);
  ListView_SetTextBkColor(*getList(), LV_BKG_COLOR);

  ListView_SetTextColor(*getList(), LV_TXT_COLOR);


  customFill();
}
//----------------------------------------------------------------------------
extern
LPCTSTR getStrPageType(uint ix);
//----------------------------------------------------------------------------
extern
LPCTSTR getStrObject(uint ix);
//----------------------------------------------------------------------------
extern
LPCTSTR getStrAction(uint ix);
//----------------------------------------------------------------------------
extern
LPCTSTR getStrDataType(uint ix);
extern void getTextAction(const infoPrph* ipp, LPTSTR act);
//----------------------------------------------------------------------------
void PD_WarnObj::customFill()
{
  if(!pMP)
    return;
  managePages& MP = *pMP;

  PVect<LPTSTR> t;
  TCHAR buff[SIZE_A_c(titleList) - 2][64];
  ZeroMemory(buff, sizeof(buff));
  TCHAR page[255];
  TCHAR act[255];
  for(uint i = 0; i < SIZE_A_c(titleList) - 2; ++i)
    t[i] = buff[i];

  t[SIZE_A_c(titleList) - 2] = page;
  t[SIZE_A_c(titleList) - 1] = act;

  ListView_DeleteAllItems(*getList());

  MP.setOrder(managePages::byPrph);

  if(!MP.setFirstPrph())
    return;

  do {
    infoPrph const* ipp;
    if(!MP.getCurr(ipp))
      break;

    if(GET_pV(ipp, Prph) && GET_pV(ipp, dataType))
      continue;

    if(!GET_pV(ipp, Prph)) {
      uint type = GET_pV(ipp, dataType);
      if(!type && GET_pV(ipp, action))
        continue;
      if((9 == type || 10 ==  type) && !GET_pV(ipp, action))
        continue;
      }

    int X = GET_pV(ipp, X);
    int Y = GET_pV(ipp, Y);
    *buff[1] = 0;
    *buff[2] = 0;

    if(COORD_BY_FUNCT == X) {
      uint pgType = GET_pV(ipp, pageType);
      switch(pgType) {
        case infoPrph::eStdMsg:
          switch(Y) {
            case COORD_BY_TIMER_TREND:
              _tcscpy(buff[2], _T("Timer"));
              break;
            case COORD_BY_ENABLE_TREND:
              _tcscpy(buff[2], _T("Enable"));
              break;
            case COORD_BY_PRINT_VARS:
              continue;
            case COORD_BY_PRINT_SCREEN:
            case COORD_BY_SAVE_SCREEN:
            case COORD_BY_FILTER_ALARM:
            case COORD_BY_CURR_DATETIME:
              break;
            default:
              wsprintf(buff[2], _T("%d"), Y);
              break;
            }
          break;
        case infoPrph::eDll:
          switch(Y) {
            case COORD_BY_FUNCT:
              break;
            case COORD_BY_DLL_RELOAD:
              _tcscpy(buff[2], _T("reload"));
              break;
            default:
              continue;
              do {
                LPCTSTR p = MP.getVarNameScript(Y);
                if(p)
                  wsprintf(buff[2], _T("$%s"), p);
                else
                  wsprintf(buff[2], _T("$%d"), Y);
                } while(false);
              break;
            }
          break;
        default:
          switch(Y) {
            case COORD_BY_FUNCT:
              break;
            case COORD_BY_RECIPE_PAGE:
              _tcscpy(buff[2], _T("for Recipe"));
              break;
            case COORD_BY_RECIPE_ROW_PAGE:
              _tcscpy(buff[2], _T("for RecipeRow"));
              break;
            default:
              if(Y < 20)
                wsprintf(buff[2], _T("F%d"), Y + 1);
              else
                wsprintf(buff[2], _T("S%d"), Y + 1 - 20);
              break;
            }
          break;
        }
      }
    else {
      wsprintf(buff[1], _T("%d"), GET_pV(ipp, X));
      wsprintf(buff[2], _T("%d"), GET_pV(ipp, Y));
      }
    wsprintf(buff[3], _T("%d"), GET_pV(ipp, Prph));
    wsprintf(buff[4], _T("%d"), GET_pV(ipp, Addr));

    uint ix = GET_pV(ipp, dataType);
    wsprintf(buff[5], _T("%s"), getStrDataType(ix));

    wsprintf(buff[6], _T("%d"), GET_pV(ipp, nBit));
    wsprintf(buff[7], _T("%d"), GET_pV(ipp, Offs));

    page[0] = 0;
    if(GET_pV(ipp, Page))
      _tcscpy(page, GET_pV(ipp, Page));

    getTextAction(ipp, act);
    ix = GET_pV(ipp, objType);

    wsprintf(buff[0], _T("%s"), getStrObject(ix));
    if(!addItem(t, OFFS_IMG(ix)))
      break;
    } while(MP.setNextPrph());
}
//----------------------------------------------------------------------------
