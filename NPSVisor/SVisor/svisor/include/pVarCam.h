//-------- PVarCam.h ---------------------------------------------------------
#ifndef PVarCam_H_
#define PVarCam_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef P_BASEOBJ_H_
  #include "p_baseObj.h"
#endif
//----------------------------------------------------------------------------
#include "PAviCap.h"
//----------------------------------------------------------------------------
#include "setPack.h"
//----------------------------------------------------------------------------
class PVarCam : public baseVar
{
  private:
    typedef baseVar baseClass;
  public:
    PVarCam(P_BaseBody* owner, uint id, bool isGlobal);
    ~PVarCam();

    virtual bool allocObj(LPVOID param = 0);
    virtual bool update(bool force);

    virtual void draw(HDC hdc, const PRect& rect) {}

    void Show();
    void Hide();

    void repos(LPCTSTR p);
    void restoreMain();
    virtual void addReqVar(PVect<P_Bits*>& allBits);

    bool isCamGlobal() const { return isGlobal; }
    PWin* getParent();
  protected:
    virtual void performDraw(HDC hdc) {}
    virtual uint getBaseId() const { return ID_INIT_CAM; }
  private:
    LPCTSTR Pagename;

    bool isGlobal;

    DWORD oldBits;
    DWORD oldFrameRate;

    DWDATA setBits(DWDATA bits);
    void checkConn(DWDATA& bits);

    class PAviCap* aviCap;
    class PAviPlay* aviPlay;

    enum command {
          eStop,
          eSetup,
          ePreview,
          eAutoCapture,
          eCapture,
          eIntPlay,
          eExtPlay,
          eConnection,
          ePause,
          eMaxCommand
          };

    bool createCap();
    bool createPlayer();
    DWDATA stopAll();
    DWDATA setupCapture();
    DWDATA preview(bool start);
    DWDATA startCapture(bool start, bool autoName);
    DWDATA play(bool start, bool internal);
    DWDATA pauseVideo(bool stop);
    void makeAutoname(LPTSTR file);
    void setShowCap(bool set);
    DWDATA connect(bool start);

    bool onCap;
    bool onExec;
    bool ComprSet;

    FILETIME ftStartedRec;

    DWORD framePos;
    uint typePlayer;
};
//----------------------------------------------------------------------------
#include "restorePack.h"
//----------------------------------------------------------------------------
#endif

