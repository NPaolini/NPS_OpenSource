//--------------- D_Info.h ---------------------------------------------------
//----------------------------------------------------------------------------
#ifndef D_INFO_H_
#define D_INFO_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "PTreeView.h"
#include "svDialog.h"
//----------------------------------------------------------------------------
#include "infoTree.h"
#include "idMsgInfoTree.h"
//----------------------------------------------------------------------------
#define FILE_TMP_PREP _T("~prep_temp.tmp")
/*
id 1 -> main totale + code (0)
id 2 -> riga oper   + code
id 3 -> riga causa  + code
*/
struct headerRow4print
{
  FILETIME ftFrom;
  FILETIME ftTo;
//  codeType codeOper; // 0 se completa // conviene caricare il cod operatore
                                        // dalle righe, se si modifica in multi
                                        // operatore è già pronto
};
//----------------------------------------------------------------------------
struct row4print
{
  int id;
  FILETIME ft;
  codeType code;
};
//----------------------------------------------------------------------------
class TD_Info : public svModDialog
{
  private:
    typedef svModDialog baseClass;
  public:
    TD_Info(PWin* parent, const vCodeType& oper, const vRangeTime& date,
          uint resId = IDD_INFO, HINSTANCE hinst = 0);
    virtual ~TD_Info();

    virtual bool create();
    virtual void print(bool preview, bool setup);

    void makeNameOper(LPTSTR target, size_t len, const codeType& code);
    void makeNameCause(LPTSTR target, size_t len, const codeType& code);

  protected:
    LRESULT windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    enum { I_ARROW, I_LOGS, I_OPER, I_DATE, I_DN };
    PTreeView* Tree;
    HIMAGELIST ImageList;

    codeOper allOper;

    vCodeType Oper;
//    const vCodeType& Oper;
    const vRangeTime& Date;
    manageInfo* InfoOper;
    manageInfo* InfoCause;

    virtual bool fill4print()=0;
    bool fill4print(manageInfo* from, manageInfo* to, bool alsoSub);

    virtual void fill()=0;
    void fill(manageInfo* from, manageInfo* to, int idIcon);

    virtual void makeNameFromCodeMaster(LPTSTR target, size_t len, const codeType& code) = 0;
    virtual codeType useCodeOrCountMaster(const codeType& code, int count) = 0;

    virtual void makeNameFromCodeSlave(LPTSTR target, size_t len, const codeType& code) = 0;
    virtual codeType useCodeOrCountSlave(const codeType& code, int count) = 0;

    virtual void getCodeMasterAndSlave(HTREEITEM parent, codeType& master, codeType& slave) = 0;

    FILETIME calcTime(const infoList& listMaster, const infoList& listSlave);
    FILETIME calcTimeMasterBySlave(const infoList& listMaster, const infoList& listSlave);


    virtual void fill_image(HIMAGELIST iml);
    virtual void fill_sub(LPNM_TREEVIEWW twn);

    virtual void fillSub1(HTREEITEM parent) = 0;
    void fillSub1(HTREEITEM parent, const infoList& listMaster, const infoList& listSlave, int idIcon);

    virtual void fillSub2(HTREEITEM parent) = 0;
    void fillSub2(HTREEITEM parent, manageInfo* infoMaster, manageInfo* infoSlave);

    virtual void fillSubAlarm(HTREEITEM parent);

    bool TVNItemexpanding(LPNM_TREEVIEWW twn);
    void EvSize(uint sizeType, SIZE& size);
    void EvGetMinMaxInfo(MINMAXINFO far& minmaxinfo);

};
//----------------------------------------------------------------------------
void runTree(int type, PWin* parent, const vCodeType& oper, const vRangeTime& date);
void runPrint(int type, PWin* parent, const vCodeType& oper, const vRangeTime& date, bool preview, bool setup);
//----------------------------------------------------------------------------
#endif

