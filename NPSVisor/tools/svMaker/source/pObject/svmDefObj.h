//---------------- svmDefObj.h ------------------------------
//-----------------------------------------------------------
#ifndef SVMDEFOBJ_H_
#define SVMDEFOBJ_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
enum objType {
  oUNDEF = -1,
  oBUTTON,
  oEDIT,
  oPANEL,
  oTEXT,
  oBARGRAPH,
  oLED,
  oLISTBOX,
  oDIAM,
  oCHOOSE,
  oSIMPLE_TEXT,
  oBITMAP,
  oCURVE,
  oXMETER,
  oCAM,
  oXSCOPE,
  oLB_ALARM,
  oTREND,
  oSPIN,
  oTBL_INFO,
  oPLOT_XY,
  oSLIDER,
  // deve essere l'ultimo
  oBKG_BITMAP,

  oMAX_OBJ_TYPE
};
//-----------------------------------------------------------
extern LPCTSTR get_objName(int obj);
//extern LPCTSTR get_objName(objType obj);
//-----------------------------------------------------------
// copiati da p_BaseBody.h
//-----------------------------------------------------------
#define ID_TITLE 1
#define ID_INIT_MENU (ID_TITLE + 1)
//#define ID_INIT_MENU 11
#define ID_INIT_ACTION_MENU (ID_INIT_MENU + 12)

#define ID_DEF_SEND_EDIT (ID_INIT_ACTION_MENU + 12)


#define ID_SHOW_SIMPLE_SEND 27

#define ID_LINK_PAGE 28
#define ID_LINK_PAGE_DYN 29

#define ID_OFFSET_X 30
#define ID_OFFSET_Y 31

#define ID_VERSION  32

#define ID_MODAL_RECT   33
#define ID_MODELESS_FIX 34

#define ID_BMP          35
#define ID_BARGRAPH     36
#define ID_SIMPLE_PANEL 40
#define ID_SIMPLE_TXT   41
#define ID_VAR_TXT      42
#define ID_VAR_EDI      43
#define ID_VAR_BTN      44
#define ID_VAR_BMP      45
#define ID_VAR_LED      46
#define ID_VAR_DIAM     47
#define ID_VAR_LBOX     48
#define ID_VAR_CHOOSE   49
#define ID_VAR_CURVE    50
#define ID_VAR_XMETER   51
#define ID_VAR_CAM      52
#define ID_VAR_SCOPE    53
#define ID_VAR_ALARM    54
#define ID_VAR_TREND    55
#define ID_VAR_SPIN     56
#define ID_VAR_TABLE_INFO 57
#define ID_VAR_PLOT_XY  58
#define ID_VAR_SLIDER   59

#define ID_INIT_BMP_4_BTN  151

//#define ID_INIT_BMP_4_VAR  251
#define ID_INIT_BMP_4_VAR  ID_INIT_BMP_4_BTN

//#define ID_MIRROR           97
#define ID_NO_CAPTION_MODAL 98
#define ID_RESOLUTION 99
#define ID_INIT_FONT 100
#define MAX_FONT 100

#define ID_INIT_VAR_CAM       480

#define ID_INIT_BMP           501
#define ID_INIT_BARGRAPH      601
#define ID_INIT_SIMPLE_PANEL 1001
#define ID_INIT_SIMPLE_TXT   2001
#define ID_INIT_VAR_TXT      3001
#define ID_INIT_VAR_EDI      4001
#define ID_INIT_VAR_BTN      5001
#define ID_INIT_VAR_BMP      6001
#define ID_INIT_VAR_LED      7001
#define ID_INIT_VAR_DIAM     8001
#define ID_INIT_VAR_LBOX     9001
#define ID_INIT_VAR_CHOOSE  10001
#define ID_INIT_VAR_CURVE   11001
#define ID_INIT_VAR_XMETER  12001
#define ID_INIT_VAR_SCOPE   13001
#define ID_INIT_VAR_ALARM   14001
#define ID_INIT_VAR_TREND   15001
#define ID_INIT_VAR_SPIN    16001
#define ID_INIT_VAR_TABLE_INFO  17001
#define ID_INIT_VAR_PLOT_XY 18001
#define ID_INIT_VAR_SLIDER  19001

#define ADD_INIT_VAR          300
#define ADD_INIT_SECOND       300
#define ADD_INIT_BITFIELD     ADD_INIT_SECOND

#define ADD_INIT_SIMPLE_BMP 200000
//----------------------------------------------------------------------------
#define OFFS_INIT_SIMPLE_BMP (ADD_INIT_SIMPLE_BMP - 50000)

//----------------------------------------------------------------------------
// usato per i testi semplici
#define ADD_MIDDLE_VAR        400
//----------------------------------------------------------------------------
#define OFFS_INIT_VAR_EXT     250000
#define ADD_INIT_VAR_EXT      2500
#define ADD_INIT_SECOND_EXT   2500
#define ADD_INIT_THIRD_EXT    ADD_INIT_SECOND_EXT
//----------------------------------------------------------------------------
#define OFFSET_BTN_OPEN_MODELES  (ADD_INIT_SIMPLE_BMP + 1000)
#define MAX_NORMAL_OPEN_MODELESS (ID_INIT_VAR_BTN + 99)
//----------------------------------------------------------------------------
// range di utilizzo normale dei barGraph
#define GRAPH_VISIBILITY_OFFS 200

// nel OFFSET_BTN_OPEN_MODELES viene aggiunto anche l'id, quindi il valore reale
// diventa OFFSET_BTN_OPEN_MODELES + ID_INIT_VAR_BTN.
// qui lo usiamo come base e quindi è effettivo
#define ADD_INIT_GRAPH       (ADD_INIT_SIMPLE_BMP + 1000)
#define GRAPH_VISIBILITY_OFFS_EXT 2000
//----------------------------------------------------------------------------

#define PRF_MEMORY  1
#define PRF_PLC     2

enum fBmp {
  bUSE_COORD,
  bTILED,
  bCENTERED,
  bSCALED,
  bFILLED,
  };
//-----------------------------------------------------------
#endif
