//------------header2.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef HEADER2_H_
#define HEADER2_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "header.h"
#include "p_body.h"
//----------------------------------------------------------------------------
//#define REP_ALWAYS 0x7fffffff
//----------------------------------------------------------------------------
class Header2 : public Header
{
  private:
    typedef Header baseClass;
  public:
    Header2(PWin* parent, int resId = IDD_HEADER, HINSTANCE hinst = 0);
    virtual ~Header2();
    virtual void setReady(bool first);

  protected:
    virtual void getFileStr(LPTSTR path);
    void refresh();
  private:
    // nel caso di ricette su righe con informazioni testuali nel primo campo
    // queste non sono memorizzate in memoria. Occorre quindi richiamare una
    // routine apposita leggendo le informazioni necessarie dal file di testo.
    // Se non esiste una ricetta di quel tipo basta non inserire il codice
    // abbinato al campo di info 1000001,x (deve essere del tipo 300x)
    uint idRecipeInfo;
    uint idRecipeInMem;
    uint idRecipePrph;

};
//----------------------------------------------------------------------------
#endif

