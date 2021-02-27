//--------- svmMoveInfo.h --------------------------------------
#ifndef svmMoveInfo_H_
#define svmMoveInfo_H_
//-----------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------
#include "p_File.h"
#include "macro_utils.h"
#include "pModDialog.h"
//-----------------------------------------------------------
class objMoveInfo
{
  public:
    objMoveInfo() : dir(0), absoluteSize(true) { sz.cx = 0; sz.cy = 0; }
    objMoveInfo(const objMoveInfo& other);
    const objMoveInfo& operator =(const objMoveInfo& other);

    struct varInfo
    {
      UINT perif;
      UINT addr;
      UINT typeVal;
      int normaliz;

      varInfo() : perif(0), addr(0), typeVal(0), normaliz(0) {}
    };

    enum var { omi_minX, omi_minY, omi_maxX, omi_maxY, omi_currX, omi_currY, MAX_MOVE_INFO };

//  private:
    SIZE sz;
    varInfo vI[MAX_MOVE_INFO];
    bool absoluteSize;

    int dir;
};
//-----------------------------------------------------------
class setOfString;
//-----------------------------------------------------------
class wrapMoveInfo
{
  public:
    wrapMoveInfo() : enabled(false), Obj(0) {}
    ~wrapMoveInfo() { delete Obj; }

    wrapMoveInfo(const wrapMoveInfo& other);
    wrapMoveInfo(const objMoveInfo& other);
    const wrapMoveInfo& operator =(const wrapMoveInfo& other);
    const wrapMoveInfo& operator =(const objMoveInfo& other);

    bool config(PWin* w, const SIZE& sz);
    int save(P_File& pf, const POINT& pt);
    bool load(setOfString& set, uint id);

    bool isEnabled() { return enabled; }
    objMoveInfo* getObj() { return Obj; }
  protected:
    bool enabled;
    objMoveInfo* Obj;

    bool saveVarMoveInfo(P_File& pf, uint baseId, const objMoveInfo::varInfo& vI);
    void loadVarMoveInfo(uint id, setOfString& set, objMoveInfo::varInfo& vI);
};
//-----------------------------------------------------------
//-----------------------------------------------------------
class svmMovingData : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    svmMovingData(objMoveInfo& mInfo, PWin* parent, uint id, HINSTANCE hInst = 0);
    ~svmMovingData();

    virtual bool create();
  protected:
    objMoveInfo& mInfo;

    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    virtual void CmOk();
    void fill();
    void chooseNorm(int idBtn);
    void check_const_prph(uint idcb);
};
//-----------------------------------------------------------
#endif
