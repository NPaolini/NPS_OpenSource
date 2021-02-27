//----------- PVarCam.cpp -------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <process.h>
#include <shellapi.h>
//----------------------------------------------------------------------------
#include "PVarCam.h"
#include "mainClient.h"
#include "pOpensave.h"
#include "p_util.h"
#include "def_dir.h"
#include "p_date.h"
//----------------------------------------------------------
bool openFileAVI(HWND owner, LPTSTR file, infoOpenSave::whichAction how);
//----------------------------------------------------------------------------
PVarCam::PVarCam(P_BaseBody* owner, uint id, bool isGlobal) : baseVar(owner, id),
    aviCap(0), aviPlay(0), Pagename(0), oldBits(0),
    onCap(false), onExec(false), ComprSet(0), isGlobal(isGlobal), framePos(0),
    typePlayer(0), oldFrameRate(0)
{}
//----------------------------------------------------------------------------
bool PVarCam::allocObj(LPVOID param)
{
  LPCTSTR p = getOwner()->getPageString(getId());
  if(!p)
    return false;

  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  int idprf = 0;
  int addr = 0;
  _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d"), &x, &y, &w, &h, &idprf, &addr);

  x += getOwner()->getOffset().x;
  y += getOwner()->getOffset().y;

  PRect rect(0, 0, R__X(w), R__Y(h));
  rect.MoveTo(R__X(x), R__Y(y));

  setRect(rect);

  BaseVar.setPrph(idprf);
  BaseVar.setAddr(addr);
  BaseVar.setType(prfData::tDWData);

  p = getOwner()->getPageString(getId() + ADD_INIT_VAR);
  if(p)
    typePlayer = _ttoi(p);

  if(!typePlayer)
    createCap();
  createPlayer();
  return true;
}
//----------------------------------------------------------------------------
void PVarCam::repos(LPCTSTR p)
{
  int x = 0;
  int y = 0;
  int w = 100;
  int h = 20;
  int idprf = 0;
  int addr = 0;
  _stscanf_s(p, _T("%d,%d,%d,%d"), &x, &y, &w, &h);

  x += getOwner()->getOffset().x;
  y += getOwner()->getOffset().y;

  PRect rect(0, 0, R__X(w), R__Y(h));
  rect.MoveTo(R__X(x), R__Y(y));

  if(isGlobal) {
    mainClient* mc = getMain();
    if(aviCap) {
      if(aviCap->getParent() == mc) {
        mc->removeChild(aviCap);
        getOwner()->addChild(aviCap);
        }
      aviCap->setParent(getOwner());
      }
    if(aviPlay) {
      if(aviPlay->getParent() == mc) {
        mc->removeChild(aviPlay);
        getOwner()->addChild(aviPlay);
        }
      aviPlay->setParent(getOwner());
      }
    }
  setRect(rect);
}
//----------------------------------------------------------------------------
void PVarCam::restoreMain()
{
  if(!isGlobal)
    return;
  Hide();
  mainClient* mc = getMain();
  if(aviCap) {
    if(aviCap->getParent() != mc) {
      getOwner()->removeChild(aviCap);
      mc->addChild(aviCap);
      }
    aviCap->setParent(mc);
    }
  if(aviPlay) {
    if(aviPlay->getParent() != mc) {
      getOwner()->removeChild(aviPlay);
      mc->addChild(aviPlay);
      }
    aviPlay->setParent(mc);
    }
  setOwner(0);
}
//----------------------------------------------------------------------------
PVarCam::~PVarCam()
{
  delete []Pagename;
}
//----------------------------------------------------------------------------
PWin* PVarCam::getParent()
{
  if(aviPlay)
    return aviPlay->getParent();
  if(aviCap)
    return aviCap->getParent();
  return 0;
}
//----------------------------------------------------------------------------
void PVarCam::Show()
{
  PRect r = get_Rect();
  if(aviCap) {
    aviCap->setWindowPos(0, r, SWP_NOZORDER);
    ShowWindow(*aviCap, onCap ? SW_SHOW : SW_HIDE);
    }
  if(aviPlay) {
    aviPlay->setWindowPos(0, r, SWP_NOZORDER);
    ShowWindow(*aviPlay, onCap ? SW_HIDE : SW_SHOW);
    }
}
//----------------------------------------------------------------------------
void PVarCam::Hide()
{
  if(aviCap)
    ShowWindow(*aviCap, SW_HIDE);
  if(aviPlay)
    ShowWindow(*aviPlay, SW_HIDE);
}
//----------------------------------------------------------------------------
bool PVarCam::update(bool force)
{
  if(!BaseVar.getPrph())
    return false;
  mainClient* Par = getMain();
  if(!Par)
    return false;
  genericPerif* perif = Par->getGenPerif(BaseVar.getPrph());
  if(!perif)
    return false;

  prfData data;
  data.typeVar = prfData::tDWData;
  data.U.dw = 0;
  data.lAddr = BaseVar.getAddr();
  perif->get(data);

  DWDATA bits = setBits(data.U.dw);
  if(bits != data.U.dw) {
    data.U.dw = bits;
    perif->set(data, true);
    }
  DWDATA capturing = bits & (1 << eAutoCapture);
  if(!capturing)
    capturing = bits & (1 << eCapture);
  if(capturing) {
    ++data.lAddr;
    data.typeVar = prfData::tHour;
    perif->get(data);
    FILETIME ftMax = data.U.ft;
    FILETIME ftCurr = getFileTimeCurr();
    ftCurr -= ftStartedRec;
    if(MK_I64(ftMax) && ftCurr >= ftMax) {
      bits &= ~capturing;
      bits = setBits(bits);
      ftCurr = ftMax;
      prfData t;
      t.lAddr = BaseVar.getAddr();
      t.typeVar = prfData::tDWData;
      t.U.dw = bits;
      perif->set(t, true);
      }
    data.lAddr += 2;
    perif->get(data);
    if(!(ftCurr == data.U.ft)) {
      data.U.ft = ftCurr;
      perif->set(data, true);
      }
    }
/*
  else if(aviPlay) {
    data.typeVar = prfData::tDWData;
    data.U.dw = 0;
    data.lAddr = BaseVar.getAddr() + 4;
    perif->get(data);
    if(oldFrameRate != data.U.dw) {
      oldFrameRate = data.U.dw;
      aviPlay->setFrameRate(oldFrameRate);
      }
    }
*/
  return true;
}
//-----------------------------------------------------------
void PVarCam::addReqVar(PVect<P_Bits*>& allBits)
{
  baseClass::addReqVar(allBits);
  int addr = BaseVar.getAddr();
  addReqVar2(allBits, BaseVar.getPrph(), addr + 1);
  addReqVar2(allBits, BaseVar.getPrph(), addr + 2);
}
//----------------------------------------------------------------------------
#define MAX_BITS_COMMAND eMaxCommand
//----------------------------------------------------------------------------
void PVarCam::checkConn(DWDATA& bits)
{
  DWDATA check = (1 << eSetup) | (1 << ePreview) | (1 << eAutoCapture) | (1 << eCapture);
  if(bits & check)
    if(!aviCap || !aviCap->isValid())
      bits &= ~(check | (1 << eConnection));
}
//----------------------------------------------------------------------------
DWDATA PVarCam::setBits(DWDATA bits)
{
  if(onExec)
    return bits;
  onExec = true;
  checkConn(bits);
  if(bits == oldBits) {
    onExec = false;
    return bits;
    }
  DWDATA xor = bits ^ oldBits;
  for(int i = 0; i < MAX_BITS_COMMAND; ++i, xor >>= 1) {
    if(xor & 1) {
      switch(i) {
        case eStop:
          bits = stopAll();
//          bits = 0;
          break;
        case eSetup:
          bits = setupCapture();
//          bits = 0;
          break;
        case ePreview:
          bits = preview(toBool(bits & (1 << ePreview)));
          break;
        case eAutoCapture:
          bits = startCapture(toBool(bits & (1 << eAutoCapture)), true);
          break;
        case eCapture:
          bits = startCapture(toBool(bits & (1 << eCapture)), false);
          break;
        case eIntPlay:
          bits = play(toBool(bits & (1 << eIntPlay)), true);
          break;
        case eExtPlay:
          bits = play(toBool(bits & (1 << eExtPlay)), false);
          break;
        case eConnection:
          bits = connect(toBool(bits & (1 << eConnection)));
          break;
        case ePause:
          bits = pauseVideo(toBool(bits & (1 << ePause)));
          break;
        }
      break;
      }
    }
  oldBits = bits & ~((1 << eStop) | (1 << eSetup) | (1 << eExtPlay));
  onExec = false;
  return oldBits;
}
//----------------------------------------------------------------------------
DWDATA PVarCam::connect(bool start)
{
  if(!aviCap)
    if(!createCap())
      return oldBits;
  setShowCap(true);
  if(start) {
    if(!aviCap->connect())
      return oldBits;
    return 1 << eConnection;
    }
  else {
    aviCap->disconnect();
    return 0;
    }
}
//----------------------------------------------------------------------------
void PVarCam::setShowCap(bool set)
{
  if(set && !onCap) {
    onCap = true;
    stopAll();
    if(aviCap)
      ShowWindow(*aviCap, SW_SHOWNORMAL);
    if(aviPlay)
      ShowWindow(*aviPlay, SW_HIDE);
    }
  else if(!set && onCap) {
    onCap = false;
    stopAll();
    if(aviPlay)
      ShowWindow(*aviPlay, SW_SHOWNORMAL);
    if(aviCap)
      ShowWindow(*aviCap, SW_HIDE);
    }
  if(!onCap)
    InvalidateRect(*aviPlay, 0, 0);
}
//----------------------------------------------------------------------------
DWDATA PVarCam::stopAll()
{
  if(aviCap) {
    aviCap->stopPreview();
    aviCap->stopCaptureFile();
    }
  if(aviPlay)
    aviPlay->stopPlay();
  return oldBits & (1 << eConnection);
}
//----------------------------------------------------------------------------
inline LPCTSTR makeSetupName(LPTSTR target, LPCTSTR source)
{
  target[0] = 0;
  getBasePath(target);
  appendPath(target, source);
  _tcscat_s(target, _MAX_PATH, _T(".dat"));
  return target;
}
//----------------------------------------------------------------------------
bool PVarCam::createCap()
{
  PWin* w = getOwner();
  if(!w)
    return false;
  TCHAR file[_MAX_PATH];
  makeSetupName(file, Pagename);

  oldBits &= ~(1 << eConnection);
  aviCap = new PAviCap(w, getId(), file, get_Rect().left, get_Rect().top, get_Rect().Width(), get_Rect().Height());
  if(!aviCap->create())
    return false;

  aviCap->setWindowPos(0, get_Rect(), SWP_NOZORDER);
  ShowWindow(*aviCap, SW_HIDE);
  return true;
}
//----------------------------------------------------------------------------
bool PVarCam::createPlayer()
{
  PWin* w = getOwner();
  if(!w)
    return false;
  aviPlay = new PAviPlay(w, getId() + 10, get_Rect().left, get_Rect().top, get_Rect().Width(), get_Rect().Height());
  if(aviPlay->create()) {
    aviPlay->setWindowPos(0, get_Rect(), SWP_NOZORDER);
    return true;
    }
  return false;
}
//-----------------------------------------------------------------------------
DWDATA PVarCam::setupCapture()
{
  if(!aviCap)
    if(!createCap())
      return oldBits;
  setShowCap(true);
  DWORD curr = aviCap->preDialog();
  aviCap->VideoSourceDialog();
  aviCap->VideoFormatDialog();
  aviCap->VideoDisplayDialog();
  aviCap->setCompressionType();
  TCHAR file[_MAX_PATH];
  aviCap->save(makeSetupName(file, Pagename));
  ComprSet = true;
  aviCap->postDialog(curr);
  return oldBits;
}
//----------------------------------------------------------------------------
DWDATA PVarCam::preview(bool start)
{
  if(!aviCap)
    if(!createCap())
      return oldBits;
  setShowCap(true);
  if(start) {
    aviCap->startPreview();
    if(oldBits & (1 << eIntPlay))
      return 1 << ePreview;
    else
      return oldBits | (1 << ePreview);
    }
  else {
    aviCap->stopPreview();
    return oldBits & ~(1 << ePreview);
    }
}
//----------------------------------------------------------------------------
void PVarCam::makeAutoname(LPTSTR file)
{
  TCHAR path[_MAX_PATH] = _T("\0");
  file[0] = 0;
  getPathExt(file);
  appendPath(file, Pagename);
  createDirectoryRecurse(file);
  SYSTEMTIME st;
  GetLocalTime(&st);
  TCHAR name[30];
  wsprintf(name, _T("%04d%02d%02d_%02d%02d%02d.avi"),
      st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
  appendPath(file, name);
}
//----------------------------------------------------------------------------
DWDATA PVarCam::startCapture(bool start, bool autoName)
{
  DWDATA currBit = 1 << (autoName ? eAutoCapture : eCapture);
  if(!aviCap)
    if(!createCap())
      return oldBits;
  setShowCap(true);
  if(!start) {
    aviCap->stopCaptureFile();
    return oldBits & ~currBit;
    }
  if(!ComprSet) {
    ComprSet = true;
    aviCap->setCompressionType();
    }
  TCHAR file[_MAX_PATH] = _T("\0");
  if(autoName)
    makeAutoname(file);
  else {
    PWin* w = getOwner();
    if(!w)
      return oldBits;
    if(!openFileAVI(*w, file, infoOpenSave::SAVE_AS))
      return oldBits;
    }
  aviCap->startCaptureFile(file);
  ftStartedRec = getFileTimeCurr();

  if(oldBits & (1 << eIntPlay))
    return currBit;
  else
    return oldBits | currBit;
}
//----------------------------------------------------------------------------
DWDATA PVarCam::pauseVideo(bool stop)
{
  if(!aviPlay || !(oldBits & (1 << eIntPlay)))
    return oldBits & ~(1 << ePause);
  if(stop) {
    aviPlay->stopPlay(&framePos);
    return oldBits | (1 << ePause);
    }
  aviPlay->startPlay(0, framePos);
  return oldBits & ~(1 << ePause);
}
//----------------------------------------------------------------------------
DWDATA PVarCam::play(bool start, bool internal)
{
  if(!getOwner())
    return oldBits;
  if(!aviPlay)
    if(!createPlayer() && internal)
      return oldBits;
  if(internal)
    setShowCap(false);
  if(!start) {
    if(internal) {
      aviPlay->stopPlay();
      framePos = 0;
      return oldBits & ~(1 << eIntPlay);
      }
    return oldBits;
    }
  TCHAR file[_MAX_PATH] = _T("\0");
  if(internal && 1 == typePlayer) {
    LPCTSTR p = getOwner()->getPageString(getId());
    p = findNextParamTrim(p, 6);
    if(p)
      _tcscpy_s(file, p);
    }
  if(!*file) {
    PWin* w = getOwner();
    if(!openFileAVI(*w, file, infoOpenSave::OPEN_F))
      return oldBits;
    }
  if(internal) {
    framePos = 0;
    aviPlay->startPlay(file);
    return 1 << eIntPlay;
    }
  else {
    ShellExecute(0, _T("open"), file, 0, 0, SW_SHOWNORMAL);
    return oldBits;
    }
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
LPCTSTR filterExt[] = { _T(".avi"), 0 };
//----------------------------------------------------------------------------
LPCTSTR filter =
  _T("VideoClip (avi)\0")
  _T("*.avi\0");
//----------------------------------------------------------------------------
class aviOpenSave : public POpenSave
{
  public:
    aviOpenSave(HWND owner) : POpenSave(owner)   {  }
  protected:
    static TCHAR Path[_MAX_PATH];
    virtual void setPathFromHistory();
    virtual void savePathToHistory();
};
//----------------------------------------------------------------------------
TCHAR aviOpenSave::Path[_MAX_PATH];
//----------------------------------------------------------------------------
void aviOpenSave::setPathFromHistory()
{
  if(*Path)
    SetCurrentDirectory(Path);
}
//----------------------------------------------------------------------------
void aviOpenSave::savePathToHistory()
{
  GetCurrentDirectory(_MAX_PATH, Path);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static
bool openFileAVI(HWND owner, LPTSTR file, infoOpenSave::whichAction how)
{
  infoOpenSave Info(filterExt, filter, how, 0);
  aviOpenSave open(owner);

  if(open.run(Info)) {
    _tcscpy_s(file, _MAX_PATH, open.getFile());
    return true;
    }
  return false;
}
