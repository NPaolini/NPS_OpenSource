//-------- PVarText.h --------------------------------------------------------
#ifndef PVARTEXT_H_
#define PVARTEXT_H_
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
class PVarText : public baseVar
{
  public:
    PVarText(P_BaseBody* owner, uint id);
    ~PVarText();

    virtual bool allocObj(LPVOID param = 0);
    virtual bool update(bool force);
    virtual void set_Text(LPCTSTR txt) { Text->setText(txt); }

  protected:
    virtual void performDraw(HDC hdc) { if(Text) { Text->setRect(get_Rect()); Text->draw(hdc); } }
    virtual uint getBaseId() const { return ID_INIT_VAR_TXT; }
  private:
    int idColor;
    uint idTextOrDec;
    int useZeroPad;

    enum typeShow { tsNorm, tsIxText, tsIxColor, tsValColor, tsValColorAndText, tsValColorAndTextExact };
    DWORD TypeShow;

    PTextFixedPanel* Text;
    baseColor Colors;
    baseVarColor VarColors;
    static int useSeparator;

    bool getAlternateText(LPTSTR buff, uint sz, uint ix);
    LPCTSTR AlternateTextFile;
    pVariable* TextVar;
};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

