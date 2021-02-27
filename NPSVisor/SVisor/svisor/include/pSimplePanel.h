//-------- PSimplePanel.h ---------------------------------------------------------
#ifndef PSimplePanel_H_
#define PSimplePanel_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "p_baseObj.h"
#include "PPanel.h"
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
class PSimplePanel : public baseSimple
{
  public:
    PSimplePanel(P_BaseBody* owner, uint id);

    ~PSimplePanel();

    virtual bool allocObj(LPVOID param = 0);
    virtual bool update(bool force);

  protected:
    void performDraw(HDC hdc);
    virtual uint getBaseId() const { return ID_INIT_SIMPLE_PANEL; }

  private:
    PPanel* Panel;
};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

