//-------- PVarChoose.h ---------------------------------------------------------
#ifndef PVARCHOOSE_H_
#define PVARCHOOSE_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef P_BASEOBJ_H_
  #include "p_baseObj.h"
#endif

#ifndef PTEXTPANEL_H_
  #include "ptextpanel.h"
#endif
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
enum typeShow { tsNorm, tsIxColor, tsValColor, tsValColorAndText };
//----------------------------------------------------------------------------
class PVarChoose : virtual public baseActive
{
  public:
    PVarChoose(P_BaseBody* owner, uint id);

    virtual ~PVarChoose();

    bool setFocus(pAround::around where) { Around.setFocus(where); return true; }

    virtual bool allocObj(LPVOID param = 0);
    virtual bool update(bool force);

    virtual HWND getHwnd();

    int action(prfData& data);
  protected:
    virtual uint getBaseId() const { return ID_INIT_VAR_CHOOSE; }
  private:
    class POwnBtnChoose* Btn;
    PTextFixedPanel* Panel;
    uint idColor;
    uint idText;

    baseColor Colors;
//    baseVarColor VarColors;

    enum typeShow { tsNorm, tsIxText, tsIxColor, tsValColor, tsValColorAndText };
    DWORD TypeShow;

    uint currChoice;
    int customSend;
    PVect<double> Vals;
    void makeVals(int idText);

    bool getAlternateText(LPTSTR buff, uint sz, uint ix);
    LPCTSTR AlternateTextFile;
    pVariable* TextVar;
    friend class PVarChooseDialog;
};
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

