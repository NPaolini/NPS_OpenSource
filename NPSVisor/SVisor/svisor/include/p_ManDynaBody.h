//--------p_ManDynaBody.h -----------------------------------------------------------
#ifndef P_MANDYNABODY_H_
#define P_MANDYNABODY_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "p_basebody.h"
#include "setPack.h"
//----------------------------------------------------------------------------
struct dual_id { int old; int replaced; };
//----------------------------------------------------------------------------
class P_ManDynaBody
{
  public:
    P_ManDynaBody(P_BaseBody* owner, setOfString& base, setOfString& to_add, POINT offset);
//    ~P_ManDynaBody();

    void run();

  private:
    P_BaseBody* Owner;
    setOfString& Base;
    setOfString& toAdd;

    // un indice per ogni oggetto
    // + uno per i testi
    // + uno per le funzioni
    // + uno per i bitmap per i pulsanti
    // + 1 per i font
    // + 1 per di più
    uint idObjCurr[MAX_OBJ + 5];

    POINT Offset;
    void addBmp4Btn();  // *
    void addFont();     // *

    void addBitmap();   // *
    void addPanel();    // *
    void addLabel();    // *
    void addBtn();      // *
    void addVarTxt();   // *
    void addBar();      // *
    void addVarBmp();
    void addLed();
    void addDiam();
    void addLBox();
    void addChoose();
    void addCurve();
    void addCam();
    void addXMeter();
    void addSlider();
    void addEdi();      // *
    void addAlarm();

    void addScope();

    void addMenu();
    void add_Menu(setOfString& Base, setOfString& toAdd, int baseId);

    void addCustom();

    bool needOffset() { return Offset.x || Offset.y; }
    LPCTSTR addOffset(LPCTSTR p);

    PVect<dual_id> DualId;

    void simpleAddObj(int idCount, int idInit, int ixObj);
    int  addIdFont(int idfont);
    int  addIdMsg(int idMsg);
    int  addIdMsgSimple(uint idColors);

    int addIdObject(int idMsg, int ix);
    int addIdFunction(int idMsg, int ix);
    void initObjDef(uint ix, int first);

    LPCTSTR addInfoLabel(LPCTSTR p);
    int  addIdBmp(int idBmp);
    LPCTSTR addInfoVarTxt(LPCTSTR p, int newIdColor, int offsAlt);
    void addVarTxtAdvanced(int ix, int id, LPCTSTR p1, LPCTSTR p2, int offsAlt, int idObj);
    LPCTSTR addInfoEdi(LPCTSTR p);
    int  addFieldEdit(int oldId);
    LPCTSTR addInfoBar(LPCTSTR p);
    LPCTSTR addInfoBmp(LPCTSTR p);
//    LPCTSTR addInfoMovBmp(LPCTSTR p);
    LPCTSTR addInfoLed(LPCTSTR p);
    LPCTSTR addInfoDiam(LPCTSTR p, int& idColor);
    LPCTSTR addInfoDiamColor(LPCTSTR p);
    LPCTSTR addInfoXMeter(LPCTSTR p);
    LPCTSTR addInfoSlider(LPCTSTR p);
    LPCTSTR addInfoAlarm(LPCTSTR p, int new_id_obj);

    void addEditExtra();
    LPCTSTR checkMovement(LPCTSTR p);
    LPCTSTR addInfoMov(LPCTSTR p);
    int addIdObjectAlways(int idMsg, int ix);

};
//----------------------------------------------------------------------------
#include "restorePack.h"
#endif

