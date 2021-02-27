//------------ defin.h -------------------------
#ifndef DEFIN_H_
#define DEFIN_H_
//-----------------------------------------------
#ifndef PDEF_H_
  #include "pDef.h"
#endif
//-----------------------------------------------
#define SEEK_SET_  FILE_BEGIN
#define SEEK_CUR_  FILE_CURRENT
#define SEEK_END_  FILE_END
//-----------------------------------------------
// typedef per file
typedef __int64 DimF;
typedef unsigned __int64 UDimF;
typedef long lDimF;
typedef unsigned long lUDimF;
//-----------------------------------------------
typedef short int SWORD;
//-----------------------------------------------
typedef char  BsDATA;
typedef short WsDATA;
typedef long  DWsDATA;
//-----------------------------------------------
typedef unsigned char  BDATA;
typedef unsigned short WDATA;
typedef unsigned long  DWDATA;
typedef float          fREALDATA;
typedef double         REALDATA;
//-----------------------------------------------
// dimensione di default dello schermo
#define DEF_X_SCREEN 800.0
#define DEF_Y_SCREEN 600.0
//-----------------------------------------------
#define DEF_FONT(h) D_FONT(h, 0, 0, _T("arial"))
//-----------------------------------------------
#define fITALIC     1
#define fBOLD       2
#define fUNDERLINED 4
#define fLIGHT      8
#define fTHIN      16
//-----------------------------------------------
#define D_FONT(h, l, flag, name)  \
    D_FONT_ORIENT(h, l, 0, flag, name)
//-----------------------------------------------
#define D_FONT_ORIENT(h, l, esc, flag, name)  \
    CreateFont(h, l, esc, esc, flag & fBOLD ? FW_BOLD : flag & fLIGHT ? \
        FW_LIGHT : flag & fTHIN ? FW_THIN : FW_NORMAL, flag & fITALIC,\
        flag & fUNDERLINED ? 1 : 0, 0,  \
        DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,\
        PROOF_QUALITY, VARIABLE_PITCH | FF_SWISS, name)
//-----------------------------------------------
#define Z_END(a) ((a)[SIZE_A(a) - 1] = 0)
//-----------------------------------------------
#endif
