//--------- memPerif.h -------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef MEMPERIF__
#define MEMPERIF__
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "defgPerif.h"
#include "Perif.h"
#include "P_Bits.h"
//-------------------------------------------------------------
#ifndef MAX_EXT_ONMEM
  #define WM_FIRST_EXT_ONMEM 10
  #define WM_LAST_EXT_ONMEM  20
//-------------------------------------------------------------
  #define MAX_EXT_ONMEM (WM_LAST_EXT_ONMEM - WM_FIRST_EXT_ONMEM + 1)
#endif
//-------------------------------------------------------------
class memPerif : public perif
{
  public:
    memPerif(mainClient* par, uint id);
    ~memPerif();

    virtual void refresh();
    virtual void commit();

    // la get va bene quella della classe base
    virtual prfData::tResultData set(const prfData& data, bool noCommit = false);
    virtual prfData::tResultData getset(prfData& data, bool noCommit = false);
    virtual prfData::tResultData multiSet(const prfData* data, int num, bool show = true,
            sorted_multi_set sms = sms_noSort);

    // non vanno usate, solo quelle sopra sono attive
    virtual bool SendCustom(WORD, howSend, WORD, DWORD&, WORD) const { return false; }
    virtual bool Send(const void *buff, int len, bool showWait = true) { return false; }
    virtual bool SendHide(const void *buff, int len) const { return false; }

    // queste non devono eseguire alcuna azione
    virtual bool Receive(int addr_init, int addr_end, WORD flag = OK_READ) const { return true; }
    virtual bool ReceiveHide(int addr_init, int addr_end) const { return true; }

  protected:
    virtual void openMapping(bool useFileMapped);

  private:
    typedef perif baseClass;

    P_Bits BitsNoCommit;

    bool errFileMapped;
    bool onLocalUseAll;
    PVect<int> vNoReload;
    int needReload;
    bool needSave;
    void makePath(LPTSTR path, uint sz, bool local);
    bool loadFile(LPTSTR path, LPBYTE buff);
};
//-----------------------------------------------------------------
#endif

