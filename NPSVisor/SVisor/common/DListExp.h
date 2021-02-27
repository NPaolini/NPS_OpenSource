//----------- dlistexp.h -----------------------------------------------------
//----------------------------------------------------------------------------
#ifndef DLISTEXP_H_
#define DLISTEXP_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "svdialog.h"
#include "plistbox.h"
#include "p_file.h"
#include "p_txt.h"
//----------------------------------------------------------------------------
// l'ordine deve corrispondere ai testi del file dei msg
enum idExport { Glob, Rep, /* Turn, Oper, Order, Bob,*/ expOther };
enum returnExp { rAbort = -1, rFailed, rSuccess };
// routine da implementare nel target finale (almeno per la produzione)
// viene richiamata se la scelta è superiore a Oper. Le prime quattro
// sono già implementate standard
returnExp exportData(int idExp, PWin *parent, int id, bool history);
//--------------------------------------------------------------------------
// routine principale di gestione export
void exportData(PWin*, bool history);
//--------------------------------------------------------------------------
//DWORD findFirstPos(class P_File& fileMonth, int day);
//DWORD findFirstPos(class P_File& fileMonth, const FILETIME& ft);
class mainClient* getMain(PWin* win);
//--------------------------------------------------------------------------
#define DEF_MAIN_TREND_CODE_BY_GRAPH 100
// routine usata per esportare qualsiasi file conforme alle specifiche dei file di trend
// in set devono esserci alcuni dei dati usati nello std_msg.txt per i file di trend,
// vengono caricati nella pagina dei grafici prendendoli dalla pagina stessa.
//** nuova specifica, poiché i dati della pagina vengono caricati dal trend
//   non si crea più un set, ma si passa l'id del trend
returnExp exportExByGraph(PWin* par, P_File& target, P_File& source, uint idTrend);
//returnExp exportExByGraph(PWin* par, P_File& target, P_File& source, setOfString& set);
//--------------------------------------------------------------------------
/*
class BarProgr : public PDialog
{
  public:
    BarProgr(PWin* par, DWORD maxStep);
    ~BarProgr() { destroy(); }
    void setPos(DWORD ix);
    bool create();
  private:
    DWORD MaxStep;
};
*/
//--------------------------------------------------------------------------
class TD_ListExp : public svDialog
{
  public:
    TD_ListExp(PWin* parent, int resId = IDD_EXPORT, HINSTANCE hinst = 0);
    virtual ~TD_ListExp();
    int getChoose() { return choose; }
    virtual bool create();

  protected:
    PListBox* LB;
    static int choose;

    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  private:
    typedef svDialog baseClass;
};
//--------------------------------------------------------------------------
#endif

