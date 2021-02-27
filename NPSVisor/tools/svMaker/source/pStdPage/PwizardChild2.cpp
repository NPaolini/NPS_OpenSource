//----------- pWizardChild2.cpp ----------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <process.h>
#include <lm.h>
//#include <Lmwksta.h>
//----------------------------------------------------------------------------
#include "wizardDlg.h"
#include "pWizardChild2.h"
#include "common.h"
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define MB_BTN (MB_OK | MB_ICONINFORMATION)
//----------------------------------------------------------------------------
static id_value pageDataCommon[] =
{
  {  1, _T("Trend")  },
  {  2, _T("Menù Preced.")  },
  {  3, _T("Imposta tempo")  },
  {  4, _T("Sovrapp.\nSeparati")  },
  {  5, _T("Imposta\nestremi")  },
  {  9, _T("Scelta dati visual.")  },
  {  10, _T("Refresh grafico")  },
  {  11, _T("Stampa\nGrafico")  },
  {  12, _T("Export")  },
  {  13, _T("Allarmi")  },
  {  32, _T("4,0")  },
  {  501, _T("0,0,0,0,1,image\\bkg.bmp")  },
  {  35, _T("1")  },
  {  100, _T("16,0,0,arial")  },
  {  101, _T("16,0,0,times new roman")  },
  {  102, _T("22,0,2,times new roman")  },
  {  1001, _T("9,70,774,220,3,255,255,250")  },
  {  40, _T("1")  },
  {  2001, _T("62,5,78,20,100,0,0,0,222,222,222,2,0,800001")  },
  {  2002, _T("141,5,78,20,100,0,0,0,222,222,222,2,0,800002")  },
  {  2003, _T("452,5,174,20,100,0,0,0,222,222,222,2,0,800003")  },
  {  2004, _T("627,5,69,20,100,0,0,0,222,222,222,2,0,800004")  },
  {  2005, _T("698,5,69,20,100,0,0,0,222,222,222,2,0,800005")  },
  {  2006, _T("232,5,210,20,100,0,0,0,222,222,222,2,0,800006")  },
  {  2007, _T("232,26,210,24,102,0,0,0,207,239,255,2,0,800007")  },
  {  2008, _T("9,26,52,20,100,0,0,0,222,222,222,2,0,800008")  },
  {  2009, _T("9,47,52,20,100,0,0,0,222,222,222,2,0,800009")  },
  {  41, _T("9")  },

  {  4001, _T("62,26,78,20,100,127,0,0,255,255,222,11,0")  },
  {  4301, _T("0,22,9,0,0")  },

  {  4002, _T("141,26,78,20,100,127,0,0,255,255,222,11,0")  },
  {  4003, _T("62,47,78,20,100,127,0,0,255,255,222,9,0")  },

  {  800003, _T("Descrizione")  },
  {  800004, _T("Minimo")  },
  {  800005, _T("Massimo")  },
  {  800006, _T("File")  },
  {  800007, _T("??")  },
  {  9001, _T("452,26,330,40,1")  },
  {  9301, _T("0,0,0,220,220,220,0,0,0,220,220,220")  },
  {  48, _T("1")  },
};
//----------------------------------------------------------------------------
static LPCTSTR pageDataCommonCust[] =
{

  {   _T("1000001,255,150,20,2,5\r\n")  },
  {   _T("1000002,0,255,0,2\r\n")  },
  {   _T("1000003,200,200,255,2\r\n")  },
  {   _T("1000004,255,255,0,2\r\n")  },
  {   _T("1000005,255,0,255,2\r\n")  },
  {   _T("1000006,0,255,255,2\r\n")  },
  {   _T("1000007,0,0,0,2\r\n")  },
  {   _T("1000008,255,255,255,2\r\n")  },
  {   _T("1000009,200,150,80,2\r\n")  },
  {   _T("1000010,100,190,128,2\r\n")  },

  {   _T("1000020,Scelta Grafici\r\n")  },
  {   _T("1000021,Ok\r\n")  },
  {   _T("1000022,Annulla\r\n")  },
  {   _T("1000023,Aggiungi\r\n")  },
  {   _T("1000024,Togli\r\n")  },
  {   _T("1000025,Sposta Su\r\n")  },
  {   _T("1000026,Sposta Giu\r\n")  },

  {   _T("1000065,2007\r\n")  },
  {   _T("1000066,16,2,arial\r\n")  },
  {   _T("1000070,Stampa Grafici\r\n")  },
  {   _T("1000071,Num. di pagine\r\n")  },
  {   _T("1000072,range.set\r\n")  },
  {   _T("1000073,exrep\\\\mh*.trd\r\n")  },
  {   _T("1000076,Scelta File Trend\r\n")  },
  {   _T("1000077,1,1\r\n")  },
  {   _T("1000080,Impostazione Timer Salvataggio\r\n")  },
  {   _T("1000081,mSec\r\n")  },
};
//----------------------------------------------------------------------------
static id_value pageDataStd[] =
{
  {  8, _T("Scelta file")  },
  {  4302, _T("0,24,9,0,0")  },
  {  4303, _T("0,26,10,0,0")  },

  {  4004, _T("141,47,78,20,100,127,0,0,255,255,222,9,0")  },
  {  4304, _T("0,28,10,0,0")  },
  {  43, _T("4")  },
  {  800001, _T("Da")  },
  {  800002, _T("A")  },
  {  800008, _T("Data")  },
  {  800009, _T("Ora")  },
};
//----------------------------------------------------------------------------
static id_value pageDataFix[] =
{
  {  6, _T("Indietro")  },
  {  7, _T("Avanti")  },
  {  800001, _T("Inizio")  },
  {  800002, _T("Ore")  },
  {  800008, _T("Date")  },
  {  800009, _T("Time")  },
//  {  4302, _T("0,24,10,0,0")  },
  {  4303, _T("0,26,10,0,0")  },

  {  43, _T("3")  },
};
//----------------------------------------------------------------------------
static id_value pageDataFixTime[] =
{
  {  4302, _T("0,24,10,0,0")  },
  {  4302, _T("0,24,4,0,0")  },
  {  800002, _T("Ore")  },
  {  800002, _T("Minuti")  },
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
pWizardChild2::pWizardChild2(PWin* parent, uint resId, HINSTANCE hinstance) :
    baseClass(parent, resId, hinstance)
{
}
//----------------------------------------------------------------------------
pWizardChild2::~pWizardChild2()
{
  destroy();
}
//----------------------------------------------------------------------------
bool pWizardChild2::create()
{
  if(!baseClass::create())
    return false;

  SET_CHECK(IDC_RADIO_WZ_FIX_FT);
  SET_CHECK(IDC_RADIO_WZ_WIN_VAR);
  checkEnable();
  return true;
}
//----------------------------------------------------------------------------
LRESULT pWizardChild2::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_COMMAND:
      switch(LOWORD(wParam)) {

        case IDC_RADIO_WZ_WIN_VAR:
        case IDC_RADIO_WZ_WIN_FIX:
          checkEnable();
          break;
        }
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
//----------------------------------------------------------------------------
void pWizardChild2::checkEnable()
{
  uint ids[] = { IDC_RADIO_WZ_FIX_HOUR, IDC_RADIO_WZ_FIX_MINUTE, IDC_RADIO_WZ_FIX_FT };
  bool enable = IS_CHECKED(IDC_RADIO_WZ_WIN_FIX);
  for(uint i = 0; i < SIZE_A(ids); ++i)
    ENABLE(ids[i], enable);
}
//----------------------------------------------------------------------------
#define NAME_WIZ2 _T("Pagina Trend")
//----------------------------------------------------------------------------
void pWizardChild2::getTitle(LPTSTR append, size_t lenBuff)
{
  _tcscpy_s(append, lenBuff, NAME_WIZ2);
}
//----------------------------------------------------------------------------
bool pWizardChild2::save(setOfString& Set)
{
  Set.reset();
  P_File* pf = makeFileTmpCustom(true);
  if(!pf)
    return false;

  P_File* pfClear = makeFileTmpCustom(false);
  if(!pfClear) {
    delete pf;
    return false;
    }

  for(uint i = 0; i < SIZE_A(pageDataCommon); ++i) {
    id_value& pd = pageDataCommon[i];
    Set.replaceString(pd.id, str_newdup(pd.value), true);
    }
  for(uint i = 0; i < SIZE_A(pageDataCommonCust); ++i) {
    if(!writeStringForceUnicode(*pf, pageDataCommonCust[i])) {
      delete pf;
      delete pfClear;
      return false;
      }
    }
  int type = -1;
  int fix = 1;
  int typeFix = 2;
  if(IS_CHECKED(IDC_RADIO_WZ_WIN_VAR)) {
    fix = 0;
    for(uint i = 0; i < SIZE_A(pageDataStd); ++i) {
      id_value& pd = pageDataStd[i];
      Set.replaceString(pd.id, str_newdup(pd.value), true);
      }
    id_value& pd = pageDataFixTime[0];
    Set.replaceString(pd.id, str_newdup(pd.value), true);
    }
  else {
    if(IS_CHECKED(IDC_RADIO_WZ_FIX_HOUR))
      typeFix = 0;
    else if(IS_CHECKED(IDC_RADIO_WZ_FIX_MINUTE))
      typeFix = 1;
    for(uint i = 0; i < SIZE_A(pageDataFix); ++i) {
      id_value& pd = pageDataFix[i];
      Set.replaceString(pd.id, str_newdup(pd.value), true);
      }
    int ix = 2 == typeFix ? 0 : 1;
    id_value& pd = pageDataFixTime[ix];
    Set.replaceString(pd.id, str_newdup(pd.value), true);
    ix = 1 == typeFix ? 3 : 2;
    pd = pageDataFixTime[ix];
    Set.replaceString(pd.id, str_newdup(pd.value), true);
    }
  TCHAR buff[40];
  wsprintf(buff, _T("%d,%d,%d,%d\r\n"), 1001003, type, fix, typeFix);
  writeStringForceUnicode(*pf, buff);

  dataProject& dp = getDataProject();
  dp.setPageType(ptcGraphTrend);

  delete pf;
  delete pfClear;

  return Set.setFirst();
}
//----------------------------------------------------------------------------
