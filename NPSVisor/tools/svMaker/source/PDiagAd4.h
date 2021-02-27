//------- PDiagAd4.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef PDiagAd4_H_
#define PDiagAd4_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <commctrl.h>
#include "pModDialog.h"
#include "pEdit.h"
#include "pStatic.h"

#include "resource.h"
#include "p_Txt.h"
#include "svmMainClient.h"
#include "macro_utils.h"
#include "p_Vect.h"
#include "language_util.h"
#include "PCustomListViewFullEdit.h"
#include "1.h"
//----------------------------------------------------------------------------
struct ad4Row;
struct infoHead;
//----------------------------------------------------------------------------
class PDiagAd4 : public PModDialog
{
  private:
    typedef PModDialog baseClass;
  public:
    PDiagAd4(PWin* parent, const PVect<infoAd4*>& set, uint resId = IDD_AD4, HINSTANCE hinstance = 0);
    virtual ~PDiagAd4();

    virtual bool create();
  protected:
    virtual LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    enum szAction { szResize, szMaximized };
    void evSize(szAction act);
    virtual void CmOk();
    virtual void CmCancel();

  private:
    const PVect<infoAd4*>& Set;
    // mantiene i vecchi valori per confrontarli se necessita di salvataggio
    PVect<ad4Row*> oldValue;
    PCustomListViewFullEdit* lvE;
    SIZE MinSize;
    void fillLV(LPCTSTR pDim);
    void fillHeader(infoHead* set, uint nElem, LPCTSTR pDim);
    void addItem(PVect<LPTSTR>& set);
    void addAd4Info(PVect<LPTSTR>& set, ad4Row* row);

    ad4Row* makeValue(LPCTSTR p);
    void getItem(PVect<LPTSTR>& set, uint iItem);

    void getAd4Info(const PVect<LPTSTR>& set, ad4Row& row);
    void fillDim(LPTSTR t);
    void saveCurrCoords(HWND hwnd);
    PRect currRect;
};
//----------------------------------------------------------------------------
#endif
