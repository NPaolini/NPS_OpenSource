//--------- set_cfg.h --------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef SET_CFG_H_
#define SET_CFG_H_

#define Perif__(a)      Perif[a]

#define OffsPrf__(a)      dwDisp[a]
#define OffsPrf1_        OffsPrf__(0)
#define OffsPrf2_        OffsPrf__(1)
#define OffsPrf3_        OffsPrf__(2)
#define OffsPrf4_        OffsPrf__(3)

#define __ExtendedPeriphOnMem  wDisp[SIZE_DISP_WORD - 1]

#define LangInUse         bDisp[0]

#define ForceResolutionByUser  bDisp[2]
#define ResolutionByUserType   bDisp[3]
#define ResolutionByUserW      wDisp[0]
#define ResolutionByUserH      wDisp[1]

#endif
