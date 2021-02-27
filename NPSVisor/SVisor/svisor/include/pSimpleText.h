//-------- PSimpleText.h --------------------------------------------------------
#ifndef PSimpleText_H_
#define PSimpleText_H_
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
class PSimpleText : public baseSimple
{
  public:
    PSimpleText(P_BaseBody* owner, uint id);
    ~PSimpleText() { delete Text; }

    virtual bool allocObj(LPVOID param = 0);
    virtual bool update(bool force);
    virtual void set_Text(LPCTSTR txt) { if(Text->setText(txt)) invalidate(); }
    virtual LPCTSTR get_Text() { return Text->getText(); }


  protected:
    virtual void performDraw(HDC hdc) { if(Text) { Text->setRect(get_Rect()); Text->draw(hdc); } }
    virtual uint getBaseId() const { return ID_INIT_SIMPLE_TXT; }
  private:
    PTextFixedPanel* Text;
};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

