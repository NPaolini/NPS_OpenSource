//-------- dmainmen.h --------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef DMAINMEN_H_
#define DMAINMEN_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "p_defbody.h"
#include "p_txt.h"
#include "ptextpanel.h"
#include "pedit.h"
//----------------------------------------------------------------------------
class TD_MainMenu : public P_DefBody {
  public:
    TD_MainMenu(int idPar, PWin* parent, int resId = IDD_MAIN_MENU, HINSTANCE hinst = 0);
    virtual ~TD_MainMenu();

  protected:
    virtual void getFileStr(LPTSTR path);

  private:
    typedef P_DefBody baseClass;

};
//----------------------------------------------------------------------------
// nel caso di non memorizzazione percorso, quando si preme F1, con comportamento
// predefinito occorre controllare se si è nella prima pagina
extern bool isMainPage(LPCTSTR page);
//----------------------------------------------------------------------------
extern P_Body *allocMainMenu(PWin *parent);
//----------------------------------------------------------------------------
#endif
