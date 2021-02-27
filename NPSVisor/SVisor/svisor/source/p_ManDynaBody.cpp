//--------p_ManDynaBody.cpp --------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "p_ManDynaBody.h"
#include "p_DefBody.h"
#include "1.h"
#include "id_btn.h"
#include <stdio.h>
//----------------------------------------------------------------------------
enum {
  ixID_MSG,
  ixFONT,
  ixID_FUNCT,
  ixOPEN_BODY,
  ixBMP4BTN,

  ixBARGRAPH,
  ixSIMPLE_PANEL,
  ixSIMPLE_TXT,
  ixVAR_BTN,
  ixVAR_TXT,
  ixBMP,
  ixVAR_BMP,
  ixVAR_LED,
  ixVAR_DIAM,
  ixVAR_LBOX,
  ixVAR_CHOOSE,
  ixVAR_CURVE,
  ixVAR_EDI,
  ixVAR_CAM,
  ixVAR_X_METER,
  ixVAR_SLIDER,
  ixVAR_ALARM,

  };
//----------------------------------------------------------------------------
#define FIRST_ID_MSG        900001
#define FIRST_ID_FUNCT      100001
#define FIRST_ID_OPEN_BODY  110000
#define MAX_ID_CODE_FUNCT   120000
//#define FIRST_ID_MSG 800001
//----------------------------------------------------------------------------
P_ManDynaBody::P_ManDynaBody(P_BaseBody* owner, setOfString& base, setOfString& to_add, POINT offset) :
        Base(base), toAdd(to_add), Offset(offset), Owner(owner)
{
  ZeroMemory(idObjCurr, sizeof(idObjCurr));

  initObjDef(ixOPEN_BODY, FIRST_ID_OPEN_BODY);
  initObjDef(ixID_MSG, FIRST_ID_MSG);
  initObjDef(ixID_FUNCT, FIRST_ID_FUNCT);
}
//----------------------------------------------------------------------------
//P_ManDynaBody::~P_ManDynaBody() {}
//----------------------------------------------------------------------------
void P_ManDynaBody::initObjDef(uint ix, int first)
{
  idObjCurr[ix] = first;
  for(;;) {
    int id = idObjCurr[ix];
    LPCTSTR p = Base.getString(id);
    if(!p)
      break;
    ++idObjCurr[ix];
    }
  ++idObjCurr[ix];
}
//----------------------------------------------------------------------------
#if 1
//----------------------------------------------------------------------------
#define RUN_(a, r) r();
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
#define RUN_(a, r) \
  p = Base.getString(ID_##a);\
  if(p) { \
    nElem = _ttoi(p); \
    idObjCurr[ix##a] = nElem; \
    } \
  r();
//----------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------
#define DUAL_(a) { ix##a, ID_##a }
//----------------------------------------------------------------------------
void P_ManDynaBody::run()
{
  addBmp4Btn();
  addFont();

  struct {
    int ix;
    int id;
    } dualCount[] = {
    DUAL_(BARGRAPH),
    DUAL_(SIMPLE_PANEL),
    DUAL_(SIMPLE_TXT),
    DUAL_(VAR_BTN),
    DUAL_(VAR_TXT),
    DUAL_(BMP),
    DUAL_(VAR_BMP),
    DUAL_(VAR_LED),
    DUAL_(VAR_DIAM),
    DUAL_(VAR_LBOX),
    DUAL_(VAR_CHOOSE),
    DUAL_(VAR_CURVE),
    DUAL_(VAR_EDI),
    DUAL_(VAR_CAM),
    DUAL_(VAR_X_METER),
    DUAL_(VAR_SLIDER),
    DUAL_(VAR_ALARM),
  };

  for(int i = 0; i < SIZE_A(dualCount); ++i) {
    LPCTSTR p = Base.getString(dualCount[i].id);
    if(p)
      idObjCurr[dualCount[i].ix] = _ttoi(p);
    }

//  int nElem;
  RUN_(BMP, addBitmap)

  RUN_(VAR_EDI, addEdi)

  RUN_(VAR_BTN, addBtn)
  RUN_(BARGRAPH, addBar)
  RUN_(SIMPLE_PANEL, addPanel)
  RUN_(SIMPLE_TXT, addLabel)
  RUN_(VAR_TXT, addVarTxt)
  RUN_(VAR_BMP, addVarBmp)
  RUN_(VAR_LED, addLed)
  RUN_(VAR_DIAM, addDiam)
  RUN_(VAR_LBOX, addLBox)
  RUN_(VAR_CHOOSE, addChoose)
  RUN_(VAR_CURVE, addCurve)
  RUN_(VAR_CAM, addCam)
  RUN_(VAR_X_METER, addXMeter)
  RUN_(VAR_SLIDER, addSlider)
  RUN_(VAR_ALARM, addAlarm)

  addMenu();
  addCustom();
}
//----------------------------------------------------------------------------
void P_ManDynaBody::add_Menu(setOfString& Base, setOfString& toAdd, int baseId)
{
  int baseAction = baseId + MAX_BTN;
  for(int i = 0; i < MAX_BTN; ++i, ++baseId, ++baseAction) {
    if(!Base.getString(baseId)) {
      LPCTSTR p = toAdd.getString(baseId);
      if(p) {
        toAdd.replaceString(baseId, 0, false, false);
        Base.replaceString(baseId, (LPTSTR)p, true, true);
        }
      }
    if(!Base.getString(baseAction)) {
      LPCTSTR p = toAdd.getString(baseAction);
      if(p) {
        int act = 0;
        int act2 = 0;
        int obj = 0;
        _stscanf_s(p, _T("%d,%d,%d"), &act, &act2, &obj);
        LPCTSTR p2 = findNextParamTrim(p, 3);
        if(obj && act != P_DefBody::ID_SEND_CTRL_ACTION) { // da verificare se gli unici oggetti (nascosti) siano btn
          int ix = addIdObject(obj, ixVAR_BTN);

          manageObjId moi2(0, ID_INIT_VAR_BTN);
          uint id2 = moi2.calcAndSetExtendId(ix);
          TCHAR buff[1024];
          wsprintf(buff, _T("%d,%d,%d"), act, act2, id2);
          if(p2)
            _tcscat_s(buff, p2);
          delete []p;
          p = str_newdup(buff);
          }
        toAdd.replaceString(baseAction, 0, false, false);
        Base.replaceString(baseAction, (LPTSTR)p, true, true);
        }
      }
    }
}
//----------------------------------------------------------------------------
void P_ManDynaBody::addMenu()
{
  add_Menu(Base, toAdd, ID_INIT_MENU);
  LPCTSTR p = getString(ID_USE_EXTENTED_KEYB);
  if(p && _ttoi(p)) {
    add_Menu(Base, toAdd, ID_F13);
    add_Menu(Base, toAdd, ID_S5);
  }
}
//----------------------------------------------------------------------------
#define ID_INIT_CUSTOM 1000000
//----------------------------------------------------------------------------
void P_ManDynaBody::addCustom()
{
  while(toAdd.setLast()) {
    int id = toAdd.getCurrId();
    if(id < ID_INIT_CUSTOM)
      break;
    LPCTSTR p = toAdd.getCurrString();
    toAdd.replaceString(id, 0, false, false);
    if(!Base.getString(id))
      Base.replaceString(id, (LPTSTR)p, true, true);
    else
      delete []p;
    }
}
//----------------------------------------------------------------------------
void P_ManDynaBody::addFont()
{
  int ix = ID_INIT_FONT;
  while(true) {
    if(!Base.getString(ix))
      break;
    if(++ix >= ID_INIT_FONT + MAX_FONT)
      break;
    }
  idObjCurr[ixFONT] = ix;
  int ix2 = ID_INIT_FONT;
  while(true) {
    LPCTSTR p = toAdd.getString(ix2);
    if(!p)
      break;
    toAdd.replaceString(ix2, 0, false, false);
    Base.replaceString(ix, (LPTSTR)p, true, true);
    if(++ix >= ID_INIT_FONT + MAX_FONT)
      break;
    ++ix2;
    }
}
//----------------------------------------------------------------------------
void P_ManDynaBody::addBmp4Btn()
{
  int ix = ID_INIT_BMP_4_BTN;
  while(true) {
    if(!Base.getString(ix))
      break;
    // dovrebbe essere il prossimo id riservato
    if(++ix >= ID_INIT_CAM)
      break;
    }
  idObjCurr[ixBMP4BTN] = ix;
  int ix2 = ID_INIT_BMP_4_BTN;
  while(true) {
    LPCTSTR p = toAdd.getString(ix2);
    if(!p)
      break;
    toAdd.replaceString(ix2, 0, false, false);
    Base.replaceString(ix, (LPTSTR)p, true, true);
    if(++ix >= ID_INIT_CAM)
      break;
    ++ix2;
    }
}
//----------------------------------------------------------------------------
LPCTSTR P_ManDynaBody::addOffset(LPCTSTR p)
{
  int x = 0;
  int y = 0;
  _stscanf_s(p, _T("%d,%d"), &x, &y);
  TCHAR buff[1000];
  p = findNextParam(p, 2);
  _stprintf_s(buff, SIZE_A(buff), _T("%d,%d,%s"), x + Offset.x, y + Offset.y, p);
  return str_newdup(buff);
}
//----------------------------------------------------------------------------
void P_ManDynaBody::simpleAddObj(int idCount, int idInit, int ixObj)
{
  LPCTSTR p = toAdd.getString(idCount);
  if(p) {
    int nElem = _ttoi(p);
    for(int i = 0; i < nElem; ++i) {
      manageObjId moi(0, idInit);
      uint id = moi.calcAndSetExtendId(i);
      p = toAdd.getString(id);
      if(p) {
        if(needOffset())
          p = addOffset(p);
        else
          toAdd.replaceString(id, 0, false, false);
        if(!i && ID_INIT_BMP == idInit) {
          LPCTSTR p2 = Base.getString(idInit);
          if(!p2)
            Base.replaceString(idInit, (LPTSTR)p, true, true);
          else
            delete []p;
          continue;
          }
        int ix = idObjCurr[ixObj]++;
        manageObjId moi2(0, idInit);
        uint id2 = moi2.calcAndSetExtendId(ix);
        Base.replaceString(id2, (LPTSTR)p, true, true);
        uint idAdd = moi.getFirstExtendId();
        p = toAdd.getString(idAdd);
        if(p) {
          toAdd.replaceString(idAdd, 0, false, false);
          uint idAdd2 = moi2.getFirstExtendId();
          p = checkMovement(p); /* <- aggiunta */
          Base.replaceString(idAdd2, (LPTSTR)p, true, true);
          }
        }
      }
    TCHAR buff[10];
    wsprintf(buff, _T("%d"), idObjCurr[ixObj]);
    Base.replaceString(idCount, str_newdup(buff), true, true);
    }
}
//----------------------------------------------------------------------------
void P_ManDynaBody::addBitmap()
{
  simpleAddObj(ID_BMP, ID_INIT_BMP, ixBMP);
}
//----------------------------------------------------------------------------
void P_ManDynaBody::addPanel()
{
  simpleAddObj(ID_SIMPLE_PANEL, ID_INIT_SIMPLE_PANEL, ixSIMPLE_PANEL);
}
//----------------------------------------------------------------------------
int P_ManDynaBody::addIdFont(int idfont)
{
  return idfont + idObjCurr[ixFONT] - ID_INIT_FONT;
}
/*
    // permette di specificare la routine di confronto per l'inserimento ordinato
    int insertEx(const T& val, int (*cmp)(const T& v, const T& ins));
    bool find(const T& val, uint& pos, int (*cmp)(const T& v, const T& ins), bool sorted = true) const;
*/
//----------------------------------------------------------------------------
int fz_cmp_dualId(const dual_id& v, const dual_id& ins)
{
  return ins.old - v.old;
}
//----------------------------------------------------------------------------
#if 0
int P_ManDynaBody::addIdObject(int idMsg, int ix)
{
  uint pos = 0;
  dual_id tdid = { idMsg, 0 };
  if(DualId.find(tdid, pos, fz_cmp_dualId))
    return DualId[pos].replaced;
  dual_id did = { idMsg, idObjCurr[ix] };
  DualId.insertEx(did, fz_cmp_dualId);
  ++idObjCurr[ix];
  return did.replaced;
}
#else
//----------------------------------------------------------------------------
int P_ManDynaBody::addIdObject(int idMsg, int ix)
{
  uint pos = 0;
  dual_id tdid = { idMsg, 0 };
  if(DualId.find(tdid, pos, fz_cmp_dualId))
    return DualId[pos].replaced;
  return addIdObjectAlways(idMsg, ix);
}
#endif
//----------------------------------------------------------------------------
int P_ManDynaBody::addIdObjectAlways(int idMsg, int ix)
{
  dual_id did = { idMsg, idObjCurr[ix] };
  DualId.insertEx(did, fz_cmp_dualId);
  ++idObjCurr[ix];
  return did.replaced;
}
//----------------------------------------------------------------------------
int P_ManDynaBody::addIdMsgSimple(uint idColors)
{
  uint pos = 0;
  dual_id tdid = { idColors, 0 };
  if(DualId.find(tdid, pos, fz_cmp_dualId))
    return DualId[pos].replaced;
  dual_id did = { idColors, idObjCurr[ixID_MSG] };
  DualId.insertEx(did, fz_cmp_dualId);
  ++idObjCurr[ixID_MSG];

  LPCTSTR p = toAdd.getString(did.old);
  toAdd.replaceString(did.old, 0, false, false);
  Base.replaceString(did.replaced, (LPTSTR)p, true, true);
  return did.replaced;
}
//----------------------------------------------------------------------------
int P_ManDynaBody::addIdFunction(int idMsg, int ix)
{
  uint pos = 0;
  dual_id tdid = { idMsg, 0 };
  if(DualId.find(tdid, pos, fz_cmp_dualId))
    return DualId[pos].replaced;
  dual_id did = { idMsg, idObjCurr[ix] };
  DualId.insertEx(did, fz_cmp_dualId);
  ++idObjCurr[ix];

  LPCTSTR p = toAdd.getString(did.old);

  if(p) {
    int act = 0;
    int act2 = 0;
    int obj = 0;
    _stscanf_s(p, _T("%d,%d,%d"), &act, &act2, &obj);
    LPCTSTR p2 = findNextParamTrim(p, 3);
    if(obj && act != P_DefBody::ID_SEND_CTRL_ACTION) { // da verificare se gli unici oggetti (nascosti) siano btn
      int ix = addIdObject(obj, ixVAR_BTN);

      manageObjId moi2(0, ID_INIT_VAR_BTN);
      uint id2 = moi2.calcAndSetExtendId(ix);
      TCHAR buff[1024];
      wsprintf(buff, _T("%d,%d,%d"), act, act2, id2);
      if(p2)
        _tcscat_s(buff, p2);
      delete []p;
      p = str_newdup(buff);
      }
    }
  toAdd.replaceString(did.old, 0, false, false);
  Base.replaceString(did.replaced, (LPTSTR)p, true, true);

  return did.replaced;
}
//----------------------------------------------------------------------------
int P_ManDynaBody::addIdMsg(int idMsg)
{
  return addIdFunction(idMsg, ixID_MSG);
}
//----------------------------------------------------------------------------
LPCTSTR P_ManDynaBody::addInfoLabel(LPCTSTR p)
{
//2001,68,144,70,26,101,0,0,0,212,208,200,2,0,800001
  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  uint idfont = 0;
  int Rfg = 0;
  int Gfg = 0;
  int Bfg = 0;
  int Rbk = 220;
  int Gbk = 220;
  int Bbk = 220;
  int no_up_dn = 0;
  int align = 0;
  int idMsg = 0;

  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &idfont,
                    &Rfg, &Gfg, &Bfg,
                    &Rbk, &Gbk, &Bbk,
                    &no_up_dn, &align, &idMsg);

  x += Offset.x;
  y += Offset.y;

  idfont = addIdFont(idfont);
  if(idMsg > 1)
    idMsg = addIdMsg(idMsg);

  TCHAR buff[1000];
  p = findNextParam(p, 2);
  _stprintf_s(buff, SIZE_A(buff), _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    x, y, w, h,
                    idfont,
                    Rfg, Gfg, Bfg,
                    Rbk, Gbk, Bbk,
                    no_up_dn, align, idMsg);
  return str_newdup(buff);
}
//----------------------------------------------------------------------------
void P_ManDynaBody::addLabel()
{
  LPCTSTR p = toAdd.getString(ID_SIMPLE_TXT);
  if(p) {
    int nElem = _ttoi(p);
    for(int i = 0; i < nElem; ++i) {
      manageObjId moi(0, ID_INIT_SIMPLE_TXT);
      uint id = moi.calcAndSetExtendId(i);

      p = toAdd.getString(id);
      if(p) {
        p = addInfoLabel(p);
        int ix = idObjCurr[ixSIMPLE_TXT]++;
        manageObjId moi2(0, ID_INIT_SIMPLE_TXT);
        uint id2 = moi2.calcAndSetExtendId(ix);
        Base.replaceString(id2, (LPTSTR)p, true, true);

        id = moi.getSecondExtendId();
        id2 = moi2.getSecondExtendId();
        p = toAdd.getString(id);
        if(p) {
          toAdd.replaceString(id, 0, false, false);
          p = checkMovement(p); /* <- aggiunta */
          Base.replaceString(id2, (LPTSTR)p, true, true);
          }
        }
      }
    TCHAR buff[10];
    wsprintf(buff, _T("%d"), idObjCurr[ixSIMPLE_TXT]);
    Base.replaceString(ID_SIMPLE_TXT, str_newdup(buff), true, true);
    }
}
//----------------------------------------------------------------------------
int P_ManDynaBody::addIdBmp(int idBmp)
{
  return idBmp + idObjCurr[ixBMP4BTN] - ID_INIT_BMP_4_BTN;
}
//----------------------------------------------------------------------------
void P_ManDynaBody::addBtn()
{
  LPCTSTR p = toAdd.getString(ID_VAR_BTN);
  if(p) {
    int nElem = _ttoi(p);
    for(int i = 0; i < nElem; ++i) {
      manageObjId moi(0, ID_INIT_VAR_BTN);
      uint id = moi.calcAndSetExtendId(i);

      p = toAdd.getString(id);
      int ix = -1;
      manageObjId moi2(0, ID_INIT_VAR_BTN);
      if(p) {
#if 1
        ix = addIdObject(id, ixVAR_BTN);
#else
        ix = idObjCurr[ixVAR_BTN]++;
#endif
        uint id2 = moi2.calcAndSetExtendId(ix);
        LPCTSTR p2 = findNextParam(p, 7);
        if(!p2) {
          if(needOffset())
            p = addOffset(p);
          else
            toAdd.replaceString(id, 0, false, false);
          Base.replaceString(id2, (LPTSTR)p, true, true);

          }
        else {
          int x = 0;
          int y = 0;
          int w = 80;
          int h = 20;
          int typeShow = 0;
          int typeCommand = 0;
          int numBmp = 0;

          _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d"),
                      &x, &y, &w, &h,
                      &typeShow, &typeCommand, &numBmp);
          // c'è qualche errore, meglio passare al successivo
          // if(numBmp > 4) // con il nuovo tipo possono esserci più bitmap
          //  continue;
          TCHAR buff[1000];
          x += Offset.x;
          y += Offset.y;
          _stprintf_s(buff, SIZE_A(buff), _T("%d,%d,%d,%d,%d,%d,%d"),
                    x, y, w, h,
                    typeShow, typeCommand, numBmp);
          p = p2;
          TCHAR t[20];
          for(int i = 0; i < numBmp && p; ++i) {
            int iBmp = addIdBmp(_ttoi(p));
            wsprintf(t, _T(",%d"), iBmp);
            _tcscat_s(buff, SIZE_A(buff), t);
            p = findNextParam(p, 1);
            }
          #define MAX_NUM_COLOR 12
          int maxColor = MAX_NUM_COLOR;
          if(1 == typeShow)
            maxColor /= 2;
          for(int i = 0; i < maxColor && p; ++i) {
            int c = _ttoi(p);
            wsprintf(t, _T(",%d"), c);
            _tcscat_s(buff, SIZE_A(buff), t);
            p = findNextParam(p, 1);
            }
          if(p) {
            int idFont = _ttoi(p);
            idFont = addIdFont(idFont);
            wsprintf(t, _T(",%d"), idFont);
            _tcscat_s(buff, SIZE_A(buff), t);
            p = findNextParam(p, 1);
            }

          for(int i = 0; i < 3 && p; ++i) {
            int idMsg = addIdMsg(_ttoi(p));
            wsprintf(t, _T(",%d"), idMsg);
            _tcscat_s(buff, SIZE_A(buff), t);
            p = findNextParam(p, 1);
            }
          if(p) {
            int idt = addFieldEdit(_ttoi(p));
            wsprintf(t, _T(",%d"), idt);
            _tcscat_s(buff, SIZE_A(buff), t);
            p = findNextParam(p, 1);
            }
          if(p) {
            int idMsg = addIdMsg(_ttoi(p));
            wsprintf(t, _T(",%d"), idMsg);
            _tcscat_s(buff, SIZE_A(buff), t);
            p = findNextParam(p, 1);
            }
          for(int i = 0; i < 2 && p; ++i) {
            int idt = addFieldEdit(_ttoi(p));
            wsprintf(t, _T(",%d"), idt);
            _tcscat_s(buff, SIZE_A(buff), t);
            p = findNextParam(p, 1);
            }
          if(p) {
            int flag = _ttoi(p);
            wsprintf(t, _T(",%d"), flag);
            _tcscat_s(buff, SIZE_A(buff), t);
            p = findNextParam(p, 1);
            }
          for(int i = 0; i < 2 && p; ++i) {
            int idMsg = addIdMsg(_ttoi(p));
            wsprintf(t, _T(",%d"), idMsg);
            _tcscat_s(buff, SIZE_A(buff), t);
            p = findNextParam(p, 1);
            }

          Base.replaceString(id2, str_newdup(buff), true, true);
          }
        }

      uint id_2 = moi.getFirstExtendId();
      p = toAdd.getString(id_2);
      if(p) {
        if(-1 == ix) {
          ix = idObjCurr[ixVAR_BTN]++;
          moi2.calcAndSetExtendId(ix);
          }
        uint id2 = moi2.getFirstExtendId();
        // se periferica zero, è abbinato a funzione
        if(!_ttoi(p)) {
          LPCTSTR pf = findNextParamTrim(p);
          if(pf) {
            int idfO = _ttoi(pf);
            if(idfO < MAX_ID_CODE_FUNCT) { // se è superiore è un errore
              if(idfO < FIRST_ID_FUNCT) // è abbinato a tasto funzione, nessuna modifica
                Base.replaceString(id2, (LPTSTR)p, true, true);
              else {
                int ixcode = idfO >= FIRST_ID_OPEN_BODY ? ixOPEN_BODY : ixID_FUNCT;
                int idf = addIdFunction(idfO, ixcode);
                TCHAR buff[64];
                wsprintf(buff, _T("0,%d"), idf);
                Base.replaceString(id2, str_newdup(buff), true, true);
                delete []p;
                }
              }
            }
          }
        else
          Base.replaceString(id2, (LPTSTR)p, true, true);
        toAdd.replaceString(id_2, 0, false, false);
        }

      id_2 = moi.getSecondExtendId();
      p = toAdd.getString(id_2);
      if(p) {
        if(-1 == ix) {
          ix = idObjCurr[ixVAR_BTN]++;
          moi2.calcAndSetExtendId(ix);
          }
        uint id2 = moi2.getSecondExtendId();
        toAdd.replaceString(id_2, 0, false, false);
        p = checkMovement(p); /* <- aggiunta */
        Base.replaceString(id2, (LPTSTR)p, true, true);
        }

      id_2 = moi.getThirdExtendId();
      p = toAdd.getString(id_2);
      if(p) {
        if(-1 == ix) {
          ix = idObjCurr[ixVAR_BTN]++;
          moi2.calcAndSetExtendId(ix);
          }
        uint id2 = moi2.getThirdExtendId();
        toAdd.replaceString(id_2, 0, false, false);
        Base.replaceString(id2, (LPTSTR)p, true, true);
        }
      }
    TCHAR buff[10];
    wsprintf(buff, _T("%d"), idObjCurr[ixVAR_BTN]);
    Base.replaceString(ID_VAR_BTN, str_newdup(buff), true, true);
    }
}
//----------------------------------------------------------------------------
LPCTSTR P_ManDynaBody::addInfoVarTxt(LPCTSTR p, int newIdColor, int offsAlt)
{
  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  uint idfont = 0;
  int Rfg = 0;
  int Gfg = 0;
  int Bfg = 0;
  int Rbk = 220;
  int Gbk = 220;
  int Bbk = 220;
  int no_up_dn = 0;
  int align = 0;
  int idColor = 0;

  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &idfont,
                    &Rfg, &Gfg, &Bfg,
                    &Rbk, &Gbk, &Bbk,
                    &no_up_dn, &align, &idColor);

  x += Offset.x;
  y += Offset.y;

  idfont = addIdFont(idfont);

  TCHAR buff[1000];
  // codificato a mano :-( purtroppo, per adesso lasciamo così
  p = findNextParamTrim(p, 14);
  LPCTSTR p2 = findNextParamTrim(p, offsAlt);
  int idAlt = 0;
  if(p2)
    idAlt = _ttoi(p2);
  if(idAlt) {
    LPCTSTR pp = toAdd.getString(idAlt);
    if(pp) {
      toAdd.replaceString(idAlt, 0, false, false);
      idAlt = idObjCurr[ixID_MSG]++;
      Base.replaceString(idAlt, (LPTSTR)pp, true, true);
      }
    else
      idAlt = 0;
    }
  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"),
                    x, y, w, h,
                    idfont,
                    Rfg, Gfg, Bfg,
                    Rbk, Gbk, Bbk,
                    no_up_dn, align, newIdColor);

  if(p)
    _tcscat_s(buff, p);
  if(idAlt) {
    int len = _tcslen(buff);
    for(int i = len - 1; i > 0; --i) {
      if(_T(',') == buff[i]) {
        wsprintf(buff + i + 1, _T("%d"), idAlt);
        break;
        }
      }
    }
  return str_newdup(buff);
}
//----------------------------------------------------------------------------
void P_ManDynaBody::addVarTxtAdvanced(int ix, int id, LPCTSTR p1, LPCTSTR p2, int offsAlt, int idObj)
{
  int idprf = 0;
  int addr = 0;
  int type = 0;
  int norm = 0;
  int dec = 0;

  _stscanf_s(p2, _T("%d,%d,%d,%d,%d"), &idprf, &addr, &type, &norm, &dec);

  LPCTSTR p = findNextParam(p1, 13);
  int idColor = _ttoi(p);
  if(idColor) {
    int newIdColor = idObjCurr[ixID_MSG]++;
    bool idNeg = idColor < 0;
    if(idNeg)
      idColor = -idColor;
    int nElem = 0x7ffffff;
    // se norm non negativo idColor punta al numero di elementi che seguono
    if(norm >= 0) {
      LPCTSTR t = toAdd.getString(idColor);
      nElem = _ttoi(t);
      }
    for(int i = 0; i <= nElem; ++i) {
      LPCTSTR t = toAdd.getString(idColor + i);
      // viene messo prima perché deve comunque lasciare un buco per indicare
      // la fine della lista
      ++idObjCurr[ixID_MSG];
      if(t) {
        toAdd.replaceString(idColor + i, 0, false, false);
        Base.replaceString(newIdColor + i, (LPTSTR)t, true, true);
        }
      else
        break;
      }

    if(idNeg)
      newIdColor = -newIdColor;
    idColor = newIdColor;
    }
  LPCTSTR t = addInfoVarTxt(p1, idColor, offsAlt);
  toAdd.replaceString(id, 0, false, false);
  Base.replaceString(ix, (LPTSTR)t, true, true);

  // fine prima parte

  if(norm < 0) {
    int newIdMsg = idObjCurr[ixID_MSG]++;
    for(int i = 0; ; ++i) {
      // dec indica una serie di testi
      LPCTSTR t = toAdd.getString(dec + i);
      // viene messo prima perché deve comunque lasciare un buco per indicare
      // la fine della lista
      ++idObjCurr[ixID_MSG];
      if(t) {
        toAdd.replaceString(dec + i, 0, false, false);
        Base.replaceString(newIdMsg + i, (LPTSTR)t, true, true);
        }
      else
        break;
      }

    dec = newIdMsg;
    }
  TCHAR buff[200];
  wsprintf(buff, _T("%d,%d,%d,%d,%d"), idprf, addr, type, norm, dec);
  manageObjId moi(ix, idObj);
  uint id2 = moi.getFirstExtendId();
  Base.replaceString(id2, str_newdup(buff), true, true);

}
//----------------------------------------------------------------------------
void P_ManDynaBody::addVarTxt()
{
  LPCTSTR p = toAdd.getString(ID_VAR_TXT);
  if(p) {
    int nElem = _ttoi(p);
    for(int i = 0; i < nElem; ++i) {
      manageObjId moi(0, ID_INIT_VAR_TXT);
      uint id = moi.calcAndSetExtendId(i);
      p = toAdd.getString(id);
      uint id2 = moi.getFirstExtendId();
      LPCTSTR p2 = toAdd.getString(id2);
      int ix = idObjCurr[ixVAR_TXT]++;
      manageObjId moi2(0, ID_INIT_VAR_TXT);
      id2 = moi2.calcAndSetExtendId(ix);

      addVarTxtAdvanced(id2, id, p, p2, 21, ID_INIT_VAR_TXT);

      id = moi.getSecondExtendId();
      p = toAdd.getString(id);
      if(p) {
        toAdd.replaceString(id, 0, false, false);
        id2 = moi2.getSecondExtendId();
        p = checkMovement(p); /* <- aggiunta */
        Base.replaceString(id2, (LPTSTR)p, true, true);
        }
      }
    TCHAR buff[10];
    wsprintf(buff, _T("%d"), idObjCurr[ixVAR_TXT]);
    Base.replaceString(ID_VAR_TXT, str_newdup(buff), true, true);
    }
}
//----------------------------------------------------------------------------
LPCTSTR P_ManDynaBody::addInfoBar(LPCTSTR p)
{

  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  int no_up_dn = 0;
  int Rbk = 110;
  int Gbk = 110;
  int Bbk = 110;

  int idMax = 0;
  int idVal = 0;
  int idColors = 0;
  int trunc = 0;
  int idBmpBar = 0;
  int flagBar = 0;
  int idBmpBkg = 0;
  int flagBkg = 0;
  int idMin = 0;

  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &no_up_dn,
                    &Rbk, &Gbk, &Bbk,
                    &idMax, &idVal, &idColors, &trunc,
                    &idBmpBar, &flagBar, &idBmpBkg, &flagBkg,
                    &idMin
                    );
  idMax = addFieldEdit(idMax);
  idVal = addFieldEdit(idVal);
  idMin = addFieldEdit(idMin);
  idColors = addIdMsgSimple(idColors);
  idBmpBar = addIdBmp(idBmpBar);
  idBmpBkg = addIdBmp(idBmpBkg);

  TCHAR buff[1000];
  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    x, y, w, h,
                    no_up_dn,
                    Rbk, Gbk, Bbk,
                    idMax, idVal, idColors, trunc,
                    idBmpBar, flagBar, idBmpBkg, flagBkg,
                    idMin
                    );
  return str_newdup(buff);
}
//----------------------------------------------------------------------------
void P_ManDynaBody::addBar()
{
  LPCTSTR p = toAdd.getString(ID_BARGRAPH);
  if(p) {
    int nElem = _ttoi(p);
    for(int i = 0; i < nElem; ++i) {
      manageObjId moi(0, ID_INIT_BARGRAPH);
      uint id = moi.calcAndSetExtendId(i);
      p = toAdd.getString(id);
      if(p) {
        int ix = idObjCurr[ixBARGRAPH]++;
        manageObjId moi2(0, ID_INIT_BARGRAPH);
        ix = moi2.calcAndSetExtendId(ix);
        p = addInfoBar(p);
        Base.replaceString(ix, (LPTSTR)p, true, true);
        int id =  moi.getFirstExtendId();
        p = toAdd.getString(id);
        if(p) {
          toAdd.replaceString(id, 0, false, false);
          ix =  moi2.getFirstExtendId();
          Base.replaceString(ix, (LPTSTR)p, true, true);
          }
        }
      }
    TCHAR buff[10];
    wsprintf(buff, _T("%d"), idObjCurr[ixBARGRAPH]);
    Base.replaceString(ID_BARGRAPH, str_newdup(buff), true, true);
    }
}
//----------------------------------------------------------------------------
LPCTSTR P_ManDynaBody::addInfoBmp(LPCTSTR p)
{
  int x = 0;
  int y = 0;
  int scale = 0;
  int flag = 0;

  _stscanf_s(p, _T("%d,%d,%d,%d"), &x, &y, &scale, &flag);

  x += Offset.x;
  y += Offset.y;
  p = findNextParam(p, 4);
  if(!p)
    return 0;
  TCHAR buff[1000];
  wsprintf(buff, _T("%d,%d,%d,%d"), x, y, scale, flag);
  while(p && *p) {
    int id = _ttoi(p);
    id = addIdBmp(id);
    TCHAR t[20];
    wsprintf(t, _T(",%d"), id);
    _tcscat_s(buff, SIZE_A(buff), t);
    p = findNextParam(p, 1);
    }
  return str_newdup(buff);
}
//----------------------------------------------------------------------------
void P_ManDynaBody::addVarBmp()
{
  LPCTSTR p = toAdd.getString(ID_VAR_BMP);
  if(p) {
    int nElem = _ttoi(p);
    for(int i = 0; i < nElem; ++i) {
      manageObjId moi(0, ID_INIT_VAR_BMP);
      uint id = moi.calcAndSetExtendId(i);
      p = toAdd.getString(id);
      if(p) {
        p = addInfoBmp(p);
        int ix = idObjCurr[ixVAR_BMP]++;
        manageObjId moi2(0, ID_INIT_VAR_BMP);
        ix = moi2.calcAndSetExtendId(ix);
        Base.replaceString(ix, (LPTSTR)p, true, true);

        id = moi.getFirstExtendId();
        ix = moi2.getFirstExtendId();
        p = toAdd.getString(id);
        if(p) {
          toAdd.replaceString(id, 0, false, false);
          Base.replaceString(ix, (LPTSTR)p, true, true);
          }
        id = moi.getSecondExtendId();
        ix = moi2.getSecondExtendId();
        p = toAdd.getString(id);
        if(p) {
          toAdd.replaceString(id, 0, false, false);
          p = checkMovement(p); /* <- aggiunta */
          Base.replaceString(ix, (LPTSTR)p, true, true);
          }
        id = moi.getThirdExtendId();
        ix = moi2.getThirdExtendId();
        p = toAdd.getString(id);
        if(p) {
          p = addInfoMov(p);
          Base.replaceString(ix, (LPTSTR)p, true, true);
          }
        }
      }
    TCHAR buff[10];
    wsprintf(buff, _T("%d"), idObjCurr[ixVAR_BMP]);
    Base.replaceString(ID_VAR_BMP, str_newdup(buff), true, true);
    }
}
//----------------------------------------------------------------------------
LPCTSTR P_ManDynaBody::addInfoLed(LPCTSTR p)
{
  TCHAR buff[1000] = _T("\0");
  while(p && *p) {
    int id = _ttoi(p);
    id = addIdBmp(id);
    TCHAR t[20];
    wsprintf(t, _T(",%d"), id);
    _tcscat_s(buff, SIZE_A(buff), t);
    p = findNextParam(p, 1);
    }
  return str_newdup(buff + 1); // leviamo la virgola iniziale
}
//----------------------------------------------------------------------------
void P_ManDynaBody::addLed()
{
  LPCTSTR p = toAdd.getString(ID_VAR_LED);
  if(p) {
    int nElem = _ttoi(p);
    for(int i = 0; i < nElem; ++i) {
      manageObjId moi(0, ID_INIT_VAR_LED);
      uint id = moi.calcAndSetExtendId(i);
      p = toAdd.getString(id);
      if(p) {
        if(needOffset())
          p = addOffset(p);
        else
          toAdd.replaceString(id, 0, false, false);
        int ix = idObjCurr[ixVAR_LED]++;
        manageObjId moi2(0, ID_INIT_VAR_LED);
        ix = moi2.calcAndSetExtendId(ix);
        Base.replaceString(ix, (LPTSTR)p, true, true);

        id = moi.getFirstExtendId();
        ix = moi2.getFirstExtendId();
        p = toAdd.getString(id);
        p = addInfoLed(p);
        Base.replaceString(ix, (LPTSTR)p, true, true);

        id = moi.getSecondExtendId();
        ix = moi2.getSecondExtendId();
        p = toAdd.getString(id);
        if(p) {
          toAdd.replaceString(id, 0, false, false);
          p = checkMovement(p); /* <- aggiunta */
          Base.replaceString(ix, (LPTSTR)p, true, true);
          }
        }
      }
    TCHAR buff[10];
    wsprintf(buff, _T("%d"), idObjCurr[ixVAR_LED]);
    Base.replaceString(ID_VAR_LED, str_newdup(buff), true, true);
    }
}
//----------------------------------------------------------------------------
LPCTSTR P_ManDynaBody::addInfoDiam(LPCTSTR p, int& idColor)
{
  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;
  int st1;
  int st2;

  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d"), &x, &y, &w, &h, &st1, &st2);

  x += Offset.x;
  y += Offset.y;
  p = findNextParam(p, 6);
  if(!p)
    return 0;
  TCHAR buff[1000];
  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d"), x, y, w, h, st1, st2);

  for(int i = 0; i < 4; ++i) {
    int id = _ttoi(p);
    id = addFieldEdit(id);
    TCHAR t[20];
    wsprintf(t, _T(",%d"), id);
    _tcscat_s(buff, SIZE_A(buff), t);
    if(3 == i)
      idColor = id;

    p = findNextParam(p, 1);
    if(!p && i < 3)
      return 0;
    }
  return str_newdup(buff);
}
//----------------------------------------------------------------------------
LPCTSTR P_ManDynaBody::addInfoDiamColor(LPCTSTR p)
{
  int nElem = 0;
  int ix = 0;

  _stscanf_s(p, _T("%d,%d"), &nElem, &ix);

  int newIx = addIdMsg(ix);

  for(int i = 1; i < nElem; ++i)
    addIdMsg(ix + i);

  ++idObjCurr[ixID_MSG];

  TCHAR buff[100];
  wsprintf(buff, _T("%d,%d"), nElem, newIx);
  return str_newdup(buff);
}
//----------------------------------------------------------------------------
void P_ManDynaBody::addDiam()
{
  LPCTSTR p = toAdd.getString(ID_VAR_DIAM);
  if(p) {
    int nElem = _ttoi(p);
    for(int i = 0; i < nElem; ++i) {
      manageObjId moi(0, ID_INIT_VAR_DIAM);
      uint id = moi.calcAndSetExtendId(i);
      p = toAdd.getString(id);
      if(p) {
        int idColor = 0;
        p = addInfoDiam(p, idColor);
        int ix = idObjCurr[ixVAR_DIAM]++;
        manageObjId moi2(0, ID_INIT_VAR_DIAM);
        ix = moi2.calcAndSetExtendId(ix);
        Base.replaceString(ix, (LPTSTR)p, true, true);

        id = moi.getFirstExtendId();
        ix = moi2.getFirstExtendId();
        p = toAdd.getString(id);
        if(idColor)
          p = addInfoDiamColor(p);
        else
          toAdd.replaceString(id, 0, false, false);
        Base.replaceString(ix, (LPTSTR)p, true, true);
        }
      }
    TCHAR buff[10];
    wsprintf(buff, _T("%d"), idObjCurr[ixVAR_DIAM]);
    Base.replaceString(ID_VAR_DIAM, str_newdup(buff), true, true);
    }
}
//----------------------------------------------------------------------------
void P_ManDynaBody::addLBox()
{
  LPCTSTR p = toAdd.getString(ID_VAR_LBOX);
  if(p) {
    int nElem = _ttoi(p);
    for(int i = 0; i < nElem; ++i) {
      manageObjId moi(0, ID_INIT_VAR_LBOX);
      uint id = moi.calcAndSetExtendId(i);
      p = toAdd.getString(id);
      if(p) {
        if(needOffset())
          p = addOffset(p);
        else
          toAdd.replaceString(id, 0, false, false);
        int ix = idObjCurr[ixVAR_LBOX]++;
        manageObjId moi2(0, ID_INIT_VAR_LBOX);
        ix = moi2.calcAndSetExtendId(ix);
        Base.replaceString(ix, (LPTSTR)p, true, true);

        id = moi.getFirstExtendId();
        ix = moi2.getFirstExtendId();
        p = toAdd.getString(id);
        if(p) {
          toAdd.replaceString(id, 0, false, false);
          Base.replaceString(ix, (LPTSTR)p, true, true);
          }
        }
      }
    TCHAR buff[10];
    wsprintf(buff, _T("%d"), idObjCurr[ixVAR_LBOX]);
    Base.replaceString(ID_VAR_LBOX, str_newdup(buff), true, true);
    }
}
//----------------------------------------------------------------------------
void P_ManDynaBody::addChoose()
{
  LPCTSTR p = toAdd.getString(ID_VAR_CHOOSE);
  if(p) {
    int nElem = _ttoi(p);
    for(int i = 0; i < nElem; ++i) {
      manageObjId moi(0, ID_INIT_VAR_CHOOSE);
      uint id = moi.calcAndSetExtendId(i);
      p = toAdd.getString(id);
      uint id2 = moi.getFirstExtendId();
      LPCTSTR p2 = toAdd.getString(id2);
      int ix = idObjCurr[ixVAR_CHOOSE]++;
      manageObjId moi2(0, ID_INIT_VAR_CHOOSE);
      ix = moi2.calcAndSetExtendId(ix);
      addVarTxtAdvanced(ix, id, p, p2, 1, ID_INIT_VAR_CHOOSE);

      id = moi.getSecondExtendId();
      ix = moi2.getSecondExtendId();
      p = toAdd.getString(id);
      if(p) {
        toAdd.replaceString(id, 0, false, false);
        p = checkMovement(p); /* <- aggiunta */
        Base.replaceString(ix, (LPTSTR)p, true, true);
        }

      id = moi.getThirdExtendId();
      ix = moi2.getThirdExtendId();
      p = toAdd.getString(id);
      if(p) {
        toAdd.replaceString(id, 0, false, false);
        Base.replaceString(ix, (LPTSTR)p, true, true);
        }
      }
    TCHAR buff[10];
    wsprintf(buff, _T("%d"), idObjCurr[ixVAR_CHOOSE]);
    Base.replaceString(ID_VAR_CHOOSE, str_newdup(buff), true, true);
    }
}
//----------------------------------------------------------------------------
void P_ManDynaBody::addCurve()
{
  LPCTSTR p = toAdd.getString(ID_VAR_CURVE);
  if(p) {
    int nElem = _ttoi(p);
    for(int i = 0; i < nElem; ++i) {
      manageObjId moi(0, ID_INIT_VAR_CURVE);
      uint id = moi.calcAndSetExtendId(i);
      p = toAdd.getString(id);
      if(p) {
        if(needOffset())
          p = addOffset(p);
        else
          toAdd.replaceString(id, 0, false, false);
        int ix = idObjCurr[ixVAR_CURVE]++;
        manageObjId moi2(0, ID_INIT_VAR_CURVE);
        ix = moi2.calcAndSetExtendId(ix);
        Base.replaceString(ix, (LPTSTR)p, true, true);

        id = moi.getFirstExtendId();
        ix = moi2.getFirstExtendId();
        p = toAdd.getString(id);
        if(p) {
          toAdd.replaceString(id, 0, false, false);
          Base.replaceString(ix, (LPTSTR)p, true, true);
          }

        id = moi.getSecondExtendId();
        ix = moi2.getSecondExtendId();
        p = toAdd.getString(id);
        if(p) {
          toAdd.replaceString(id, 0, false, false);
          p = checkMovement(p); /* <- aggiunta */
          Base.replaceString(ix, (LPTSTR)p, true, true);
          }

        id = moi.getThirdExtendId();
        ix = moi2.getThirdExtendId();
        p = toAdd.getString(id);
        if(p) {
          toAdd.replaceString(id, 0, false, false);
          Base.replaceString(ix, (LPTSTR)p, true, true);
          }
        }
      }
    TCHAR buff[10];
    wsprintf(buff, _T("%d"), idObjCurr[ixVAR_CURVE]);
    Base.replaceString(ID_VAR_CURVE, str_newdup(buff), true, true);
    }
}
//----------------------------------------------------------------------------
void P_ManDynaBody::addCam()
{
  LPCTSTR p = toAdd.getString(ID_VAR_CAM);
  if(p) {
    int nElem = _ttoi(p);
    for(int i = 0; i < nElem; ++i) {
      int id = ID_INIT_CAM + i;
      p = toAdd.getString(id);
      if(p) {
        if(needOffset())
          p = addOffset(p);
        else
          toAdd.replaceString(id, 0, false, false);
        int ix = idObjCurr[ixVAR_CAM]++;
        ix += ID_INIT_CAM;
        Base.replaceString(ix, (LPTSTR)p, true, true);
        }
      }
    TCHAR buff[10];
    wsprintf(buff, _T("%d"), idObjCurr[ixVAR_CAM]);
    Base.replaceString(ID_VAR_CAM, str_newdup(buff), true, true);
    }
}
//----------------------------------------------------------------------------
LPCTSTR P_ManDynaBody::addInfoXMeter(LPCTSTR p)
{
  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  int idImage = 0;
  int offsX = 0;
  int offsY = 0;
  int initAngle = 220;
  int range = 120;
  int clockWise = 0;
  int gran = 0;
  int idMin = 0;
  int idMax = 0;

  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &idImage, &offsX, &offsY,
                    &initAngle, &range,
                    &clockWise, &gran
                    ,&idMin, &idMax
                    );

  x += Offset.x;
  y += Offset.y;
  idImage = addIdBmp(idImage);
  idMin = addFieldEdit(idMin);
  idMax = addFieldEdit(idMax);
  TCHAR buff[200];
  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    x, y, w, h,
                    idImage, offsX, offsY,
                    initAngle, range,
                    clockWise, gran, idMin, idMax
                    );
  return str_newdup(buff);
}
//----------------------------------------------------------------------------
void P_ManDynaBody::addXMeter()
{
  LPCTSTR p = toAdd.getString(ID_VAR_X_METER);
  if(p) {
    int nElem = _ttoi(p);
    for(int i = 0; i < nElem; ++i) {
      manageObjId moi(0, ID_INIT_VAR_X_METER);
      uint id = moi.calcAndSetExtendId(i);
      p = toAdd.getString(id);
      if(p) {
        p = addInfoXMeter(p);
        int ix = idObjCurr[ixVAR_X_METER]++;
        manageObjId moi2(0, ID_INIT_VAR_X_METER);
        ix = moi2.calcAndSetExtendId(ix);
        Base.replaceString(ix, (LPTSTR)p, true, true);

        id = moi.getFirstExtendId();
        ix = moi2.getFirstExtendId();
        p = toAdd.getString(id);
        if(p) {
          toAdd.replaceString(id, 0, false, false);
          Base.replaceString(ix, (LPTSTR)p, true, true);
          }

        id = moi.getSecondExtendId();
        ix = moi2.getSecondExtendId();
        p = toAdd.getString(id);
        if(p) {
          toAdd.replaceString(id, 0, false, false);
          p = checkMovement(p); /* <- aggiunta */
          Base.replaceString(ix, (LPTSTR)p, true, true);
          }
        }
      }
    TCHAR buff[10];
    wsprintf(buff, _T("%d"), idObjCurr[ixVAR_X_METER]);
    Base.replaceString(ID_VAR_X_METER, str_newdup(buff), true, true);
    }
}
//----------------------------------------------------------------------------
#define MAX_SHOW_AL 9
//----------------------------------------------------------------------------

LPCTSTR P_ManDynaBody::addInfoAlarm(LPCTSTR p, int new_id_obj)
{
  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;

  int idFont = 0;
  int idObj  = 0;
  int idFilter  = 0;
  int filterFix  = 0;

  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h, &idFont,
                    &idObj, &filterFix,
                    &idFilter
                    );
  x += Offset.x;
  y += Offset.y;

  idFilter = addFieldEdit(idFilter);
  idFont = addIdFont(idFont);

  p = findNextParamTrim(p, 8);
  TCHAR buff[200];
  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%s"),
                    x, y, w, h, idFont,
                    new_id_obj, filterFix,
                    idFilter, p
                    );
  p = findNextParamTrim(buff, 9 + MAX_SHOW_AL * 2);
  if(p) {
    TCHAR tb[300];
    idFilter = _ttoi(p);
    idFilter = addFieldEdit(idFilter);
    filterFix = 0;
    LPCTSTR p2 = findNextParamTrim(p);
    if(p2) {
      filterFix = _ttoi(p2);
      if(filterFix)
        filterFix = addFieldEdit(filterFix);
      }
    buff[p - buff] = 0;
    wsprintf(tb, _T("%s%d,%d"), buff, idFilter, filterFix);
    return str_newdup(tb);
    }
  return str_newdup(buff);
}
//----------------------------------------------------------------------------
void P_ManDynaBody::addAlarm()
{
  LPCTSTR p = toAdd.getString(ID_VAR_ALARM);
  if(p) {
    int nElem = _ttoi(p);
    for(int i = 0; i < nElem; ++i) {
      manageObjId moi(0, ID_INIT_VAR_ALARM);
      uint id = moi.calcAndSetExtendId(i);
      p = toAdd.getString(id);
      if(p) {
        int ix = idObjCurr[ixVAR_ALARM]++;
        p = addInfoAlarm(p, ix + 1);
        manageObjId moi2(0, ID_INIT_VAR_ALARM);
        ix = moi2.calcAndSetExtendId(ix);
        Base.replaceString(ix, (LPTSTR)p, true, true);

        id = moi.getFirstExtendId();
        ix = moi2.getFirstExtendId();
        p = toAdd.getString(id);
        if(p) {
          toAdd.replaceString(id, 0, false, false);
          Base.replaceString(ix, (LPTSTR)p, true, true);
          }

        id = moi.getSecondExtendId();
        ix = moi2.getSecondExtendId();
        p = toAdd.getString(id);
        if(p) {
          toAdd.replaceString(id, 0, false, false);
          p = checkMovement(p); /* <- aggiunta */
          Base.replaceString(ix, (LPTSTR)p, true, true);
          }

        id = moi.getThirdExtendId();
        ix = moi2.getThirdExtendId();
        p = toAdd.getString(id);
        if(p) {
          toAdd.replaceString(id, 0, false, false);
          Base.replaceString(ix, (LPTSTR)p, true, true);
          }
        }
      }
    TCHAR buff[10];
    wsprintf(buff, _T("%d"), idObjCurr[ixVAR_ALARM]);
    Base.replaceString(ID_VAR_ALARM, str_newdup(buff), true, true);
    }
}
//----------------------------------------------------------------------------
LPCTSTR P_ManDynaBody::addInfoEdi(LPCTSTR p)
{
//formato ->id,x,y,w,h,idfont,Rfg,Gfg,Bfg,Rbk,Gbk,Bbk,lenEdit,tab_order
  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  uint idfont = 0;
  int Rfg = 0;
  int Gfg = 0;
  int Bfg = 0;
  int Rbk = 220;
  int Gbk = 220;
  int Bbk = 220;
  int lenEdit = 0;
  int tab_order = 0;

  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &idfont,
                    &Rfg, &Gfg, &Bfg,
                    &Rbk, &Gbk, &Bbk,
                    &lenEdit, &tab_order);

  x += Offset.x;
  y += Offset.y;

  idfont = addIdFont(idfont);

  TCHAR buff[1000];
  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    x, y, w, h,
                    idfont,
                    Rfg, Gfg, Bfg,
                    Rbk, Gbk, Bbk,
                    lenEdit, tab_order);
  return str_newdup(buff);
}
//----------------------------------------------------------------------------
void P_ManDynaBody::addEdi()
{
  LPCTSTR p = toAdd.getString(ID_VAR_EDI);
  if(p) {
    int nElem = _ttoi(p);
    for(int i = 0; i < nElem; ++i) {
      manageObjId moi(0, ID_INIT_VAR_EDI);
      uint id = moi.calcAndSetExtendId(i);
      p = toAdd.getString(id);
      // solo oggetti reali
      if(!p)
        continue;
      p = addInfoEdi(p);

      int ix = idObjCurr[ixVAR_EDI]++;
      manageObjId moi2(0, ID_INIT_VAR_EDI);
      ix = moi2.calcAndSetExtendId(ix);
      Base.replaceString(ix, (LPTSTR)p, true, true);

      id = moi.getFirstExtendId();
      p = toAdd.getString(id);
      if(p) {
        toAdd.replaceString(id , 0, false, false);
        ix = moi2.getFirstExtendId();
        Base.replaceString(ix, (LPTSTR)p, true, true);
        }
      id = moi.getSecondExtendId();
      p = toAdd.getString(id);
      if(p) {
        toAdd.replaceString(id , 0, false, false);
        p = checkMovement(p);
        ix = moi2.getSecondExtendId();
        Base.replaceString(ix, (LPTSTR)p, true, true);
        }
      }
    }
  // va comunque fatto, potrebbero essere stati usati alcuni campi
  // come appoggio
  TCHAR buff[10];
  wsprintf(buff, _T("%d"), idObjCurr[ixVAR_EDI]);
  Base.replaceString(ID_VAR_EDI, str_newdup(buff), true, true);
}
//----------------------------------------------------------------------------
void P_ManDynaBody::addEditExtra()
{
  LPCTSTR p = toAdd.getString(ID_VAR_EDI);
  if(p) {
    int nElem = _ttoi(p);
    DualId.setDim(nElem + 1);
    for(int i = 0; i < nElem; ++i) {
      manageObjId moi(0, ID_INIT_VAR_EDI);
      uint id = moi.calcAndSetExtendId(i);
      p = toAdd.getString(id);
      // se c'è il codice iniziale salta
      if(!p)
        addFieldEdit(id);
      }
    }
}
//----------------------------------------------------------------------------
#define OFFS_MOVE 8
//----------------------------------------------------------------------------
LPCTSTR P_ManDynaBody::checkMovement(LPCTSTR p)
{
  LPCTSTR t = findNextParamTrim(p, OFFS_MOVE);
  if(!t)
    return p;
  int code = _ttoi(t);
  if(!code)
    return p;
  LPCTSTR pt = toAdd.getString(code);
  if(!pt)
    return p;
  LPCTSTR p2 = addInfoMov(pt);
  if(!p2)
    return p;
  int idMov = addIdObject(code, ixID_MSG);
  Base.replaceString(idMov, (LPTSTR)p2, true, true);
  TCHAR t2[512];
  _tcscpy_s(t2, p);
  t2[t - p] = 0;
  LPTSTR pt2 = t2 + _tcslen(t2);
  wsprintf(pt2, _T("%d"), idMov);
  toAdd.replaceString(code, 0, false, true);
  return str_newdup(t2);
}
//----------------------------------------------------------------------------
LPCTSTR P_ManDynaBody::addInfoMov(LPCTSTR p)
{
  int x1 = 0;
  int y1 = 0;
  int x2 = 0;
  int y2 = 0;

  _stscanf_s(p, _T("%d,%d,%d,%d"), &x1, &y1, &x2, &y2);

  x1 += Offset.x;
  y1 += Offset.y;
  x2 += Offset.x;
  y2 += Offset.y;
  p = findNextParam(p, 4);
  if(!p)
    return 0;
  TCHAR buff[1000];
  wsprintf(buff, _T("%d,%d,%d,%d"), x1, y1, x2, y2);
  for(int i = 0; i < 6; ++i) {
    int id = _ttoi(p);
    id = addFieldEdit(id);
    TCHAR t[20];
    wsprintf(t, _T(",%d"), id);
    _tcscat_s(buff, t);
    p = findNextParam(p, 1);
    if(!p)
      return 0;
    }
  _tcscat_s(buff, _T(","));
  _tcscat_s(buff, p);
  return str_newdup(buff);
}
//----------------------------------------------------------------------------
int P_ManDynaBody::addFieldEdit(int oldId)
{
  if(!oldId)
    return 0;
  int result = oldId;
  manageObjId moi(oldId, ID_INIT_VAR_EDI);
  uint id = moi.getFirstExtendId();

  LPCTSTR p = toAdd.getString(id);
  if(p) {
//    int ix = idObjCurr[ixVAR_EDI]++;
    int ix = addIdObject(id, ixVAR_EDI);
    manageObjId moi2(0, ID_INIT_VAR_EDI);
    result = moi2.calcAndSetExtendId(ix);

    toAdd.replaceString(id, 0, false, false);
    uint id2 = moi2.getFirstExtendId();
    Base.replaceString(id2, (LPTSTR)p, true, true);
    id = moi.getSecondExtendId();
    p = toAdd.getString(id);
    if(p) {
      toAdd.replaceString(id, 0, false, false);
      id2 = moi2.getSecondExtendId();
      Base.replaceString(id2, (LPTSTR)p, true, true);
      }
    }
  return result;
}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
LPCTSTR P_ManDynaBody::addInfoSlider(LPCTSTR p)
{
  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;

  int idMax = 0;
  int idMin = 0;

  int idBmp = 0;
  int h_w = 0;

  int fullImage = 0;
  int vert = 0;
  int mirror = 0;
  int reverse = 0;

  int readOnly = 0;
  int updateAlways = 0;

  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    &x, &y, &w, &h,
                    &idMax, &idMin, &idBmp, &h_w,
                    &fullImage, &vert, &mirror, &reverse,
                    &readOnly, &updateAlways
                    );
  x += Offset.x;
  y += Offset.y;
  idBmp = addIdBmp(idBmp);
  idMin = addFieldEdit(idMin);
  idMax = addFieldEdit(idMax);
  TCHAR buff[200];
  wsprintf(buff, _T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"),
                    x, y, w, h,
                    idMax, idMin, idBmp, h_w,
                    fullImage, vert, mirror, reverse,
                    readOnly, updateAlways
                    );
  return str_newdup(buff);
}
//----------------------------------------------------------------------------
void P_ManDynaBody::addSlider()
{
  LPCTSTR p = toAdd.getString(ID_VAR_SLIDER);
  if(p) {
    int nElem = _ttoi(p);
    for(int i = 0; i < nElem; ++i) {
      manageObjId moi(0, ID_INIT_VAR_SLIDER);
      uint id = moi.calcAndSetExtendId(i);
      p = toAdd.getString(id);
      if(p) {
        p = addInfoSlider(p);
        int ix = idObjCurr[ixVAR_SLIDER]++;
        manageObjId moi2(0, ID_INIT_VAR_SLIDER);
        ix = moi2.calcAndSetExtendId(ix);
        Base.replaceString(ix, (LPTSTR)p, true, true);

        id = moi.getFirstExtendId();
        ix = moi2.getFirstExtendId();
        p = toAdd.getString(id);
        if(p) {
          toAdd.replaceString(id, 0, false, false);
          Base.replaceString(ix, (LPTSTR)p, true, true);
          }

        id = moi.getSecondExtendId();
        ix = moi2.getSecondExtendId();
        p = toAdd.getString(id);
        if(p) {
          toAdd.replaceString(id, 0, false, false);
          p = checkMovement(p); /* <- aggiunta */
          Base.replaceString(ix, (LPTSTR)p, true, true);
          }
        }
      }
    TCHAR buff[10];
    wsprintf(buff, _T("%d"), idObjCurr[ixVAR_SLIDER]);
    Base.replaceString(ID_VAR_SLIDER, str_newdup(buff), true, true);
    }
}
