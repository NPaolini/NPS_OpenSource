//----------- checkAddr.cpp --------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include "checkAddr.h"
#include "defgPerif.h"
#include "manageCryptPage.h"
#include "p_manage_ini.h"
//----------------------------------------------------------------------------
#define ROUND_DWORD_LEN(a) ((int)((a) + 3) / 4)
//#define ROUND_DWORD_LEN(a) ((a) / 4 + (((a) % 4) != 0))
//----------------------------------------------------------------------------
#define ID_TITLE 1
#define ID_INIT_MENU (ID_TITLE + 1)
//#define ID_INIT_MENU 11
#define ID_INIT_ACTION_MENU (ID_INIT_MENU + 12)

#define ID_DEF_SEND_EDIT (ID_INIT_ACTION_MENU + 12)


#define ID_SHOW_SIMPLE_SEND 27

#define ID_PAGE_DYNAMIC_LINK 28

#define ID_PAGE_DYNAMIC_LINK_V2 29

#define ID_OFFSET_X 30
#define ID_OFFSET_Y 31

#define ID_VERSION  32

#define ID_MODAL_RECT   33
#define ID_MODELESS_MEMO_POS 34

#define ID_BMP          35
#define ID_BARGRAPH     36
#define ID_SIMPLE_PANEL 40
#define ID_SIMPLE_TXT   41
#define ID_VAR_TXT      42
#define ID_VAR_EDI      43
#define ID_VAR_BTN      44
#define ID_VAR_BMP      45
#define ID_VAR_LED      46
#define ID_VAR_DIAM     47
#define ID_VAR_LBOX     48
#define ID_VAR_CHOOSE   49
#define ID_VAR_CURVE    50
#define ID_VAR_X_METER  51
#define ID_VAR_CAM      52
#define ID_VAR_SCOPE    53
#define ID_VAR_ALARM    54
#define ID_VAR_TREND    55
#define ID_VAR_SPIN     56
#define ID_VAR_TABLE_INFO 57
#define ID_VAR_PLOT_XY  58
#define ID_VAR_SLIDER   59

#define ID_INIT_BMP_4_BTN  151

//#define ID_INIT_BMP_4_VAR  251
#define ID_INIT_BMP_4_VAR  ID_INIT_BMP_4_BTN

#define ID_RESOLUTION 99
#define ID_INIT_FONT 100
#define MAX_FONT 50

#define ID_INIT_CAM            480

#define ID_INIT_BMP            501
#define ID_INIT_BARGRAPH       601
#define ID_INIT_SIMPLE_PANEL  1001
#define ID_INIT_SIMPLE_TXT    2001
#define ID_INIT_VAR_TXT       3001
#define ID_INIT_VAR_EDI       4001
#define ID_INIT_VAR_BTN       5001
#define ID_INIT_VAR_BMP       6001
#define ID_INIT_VAR_LED       7001
#define ID_INIT_VAR_DIAM      8001
#define ID_INIT_VAR_LBOX      9001
#define ID_INIT_VAR_CHOOSE   10001
#define ID_INIT_VAR_CURVE    11001
#define ID_INIT_VAR_X_METER  12001
#define ID_INIT_VAR_SCOPE    13001
#define ID_INIT_VAR_ALARM    14001
#define ID_INIT_VAR_TREND    15001
#define ID_INIT_VAR_SPIN     16001
#define ID_INIT_VAR_TABLE_INFO  17001
#define ID_INIT_VAR_PLOT_XY  18001
#define ID_INIT_VAR_SLIDER   19001
//----------------------------------------------------------------------------
// usato per i testi semplici
#define ADD_MIDDLE_VAR        400
//----------------------------------------------------------------------------
#define ADD_INIT_VAR          300
#define ADD_INIT_SECOND       300
#define ADD_INIT_BITFIELD     ADD_INIT_SECOND
//----------------------------------------------------------------------------
#define OFFS_INIT_VAR_EXT     250000
#define ADD_INIT_VAR_EXT      2500
#define ADD_INIT_SECOND_EXT   2500
#define ADD_INIT_THIRD_EXT    ADD_INIT_SECOND_EXT
//----------------------------------------------------------------------------
#define ADD_INIT_SIMPLE_BMP 200000

#define OFFSET_BTN_OPEN_MODELES  (ADD_INIT_SIMPLE_BMP + 1000)
#define MAX_NORMAL_OPEN_MODELESS (ID_INIT_VAR_BTN + 99)
//----------------------------------------------------------------------------
// range di utilizzo normale dei barGraph
#define GRAPH_VISIBILITY_OFFS 200

// nel OFFSET_BTN_OPEN_MODELES viene aggiunto anche l'id, quindi il valore reale
// diventa OFFSET_BTN_OPEN_MODELES + ID_INIT_VAR_BTN.
// qui lo usiamo come base e quindi è effettivo
#define ADD_INIT_GRAPH       (ADD_INIT_SIMPLE_BMP + 1000)
#define GRAPH_VISIBILITY_OFFS_EXT 2000
//----------------------------------------------------------------------------
#define PRF_MEMORY  1
#define PRF_PLC     2
//----------------------------------------------------------------------------
#define ID_CODE_PAGE_TYPE 999999
//----------------------------------------------------------------------------
#define ID_SEND_VALUE_ON_OPEN  999996
#define ID_SEND_VALUE_ON_CLOSE 999997
//----------------------------------------------------------------------------
//#define SUB_BASE_KEY _T("Software\\NPS_Programs\\svControl")
#define REL_SYSTEM_PATH _T("SystemPath")
//----------------------------------------------------------------------------
#define CHECK_ADDR_INI_FILE _T("checkAddr.ini")
#define MAIN_PATH _T("\\Main")
//----------------------------------------------------------------------------
#define ID_USE_TOUCH_KEYB   47
#define ID_NUMB_TOUCH_KEYB  48
//----------------------------------------------------------------------------
#define INIT_TREND_DATA   1000
//----------------------------------------------------------------------------
void myStrImage::fillBuff(LPTSTR buff) const
{
  if(P && Page)
  wsprintf(buff, _T("%s - %s"), P, Page);
  else if(P)
  wsprintf(buff, _T("%s - ?"), P);
  else
    *buff = 0;
}
//----------------------------------------------------------------------------
uint manageObjId::calcBase(uint id)
{
  if(OFFS_INIT_VAR_EXT <= id)
    return ((id - OFFS_INIT_VAR_EXT) / 1000) * 100 + 1;
  if(id < 1000)
    return (id / 100) * 100 + 1;
  return (id / 1000) * 1000 + 1;
}
//----------------------------------------------------------------------------
uint manageObjId::calcBaseExtendId(uint ix)
{
  switch(getBaseId()) {
    case ID_INIT_BARGRAPH:
      if(ix < GRAPH_VISIBILITY_OFFS)
        return getBaseId() + ix;
      return ix + ADD_INIT_GRAPH + 1;

    case ID_INIT_BMP:
      return ix + getBaseId();
    }
  uint idSec = ID_INIT_SIMPLE_TXT == getBaseId() ? ADD_MIDDLE_VAR : ADD_INIT_VAR;
  if(ix < idSec)
    return ix + getBaseId();

  return ix + (getBaseId() - 1) * 10 + OFFS_INIT_VAR_EXT - ADD_INIT_VAR;
}
//----------------------------------------------------------------------------
uint manageObjId::calcAndSetExtendId(uint ix)
{
  Id = calcBaseExtendId(ix);
  return Id;
}
//----------------------------------------------------------------------------
uint manageObjId::getFirstExtendId()
{
  if(ID_INIT_BARGRAPH == getBaseId()) {
    if(getId() < getBaseId() + GRAPH_VISIBILITY_OFFS)
      return getId() + GRAPH_VISIBILITY_OFFS;
    return getId() - getBaseId() + ADD_INIT_GRAPH + 1;
    }
  if(ID_INIT_BMP == getBaseId())
    return getId() + ADD_INIT_SIMPLE_BMP;

  if(ID_INIT_SIMPLE_PANEL == getBaseId())
    return getId() + ADD_INIT_VAR * 2;

  uint idSec = ID_INIT_SIMPLE_TXT == getBaseId() ? ADD_MIDDLE_VAR : ADD_INIT_VAR;
  if(getId() < getBaseId() + idSec)
    return getId() + ADD_INIT_VAR;

  return getId() + ADD_INIT_VAR_EXT;
}
//----------------------------------------------------------------------------
uint manageObjId::getSecondExtendId()
{
  uint idSec = ID_INIT_SIMPLE_TXT == getBaseId() ? ADD_MIDDLE_VAR : ADD_INIT_VAR;
  if(getId() < getBaseId() + idSec)
    return getId() + ADD_INIT_VAR + ADD_INIT_SECOND;

  return getId() + ADD_INIT_VAR_EXT + ADD_INIT_SECOND_EXT;// + OFFS_INIT_VAR_EXT;
}
//----------------------------------------------------------------------------
#define MAX_THIRD_OFFSET MAX_NORMAL_OPEN_MODELESS
//----------------------------------------------------------------------------
uint manageObjId::getThirdExtendId()
{
  if(ID_INIT_VAR_BTN == getBaseId()) {
    if(getId() < MAX_NORMAL_OPEN_MODELESS)
      return getId() + ADD_INIT_VAR + ADD_INIT_SECOND + ADD_INIT_BITFIELD;
    // se l'id è già esteso occorre ricavare l'indice aumentato di ADD_INIT_VAR prima di sommarlo all'offset
    if(getId() > OFFS_INIT_VAR_EXT)
      return getId() - ((getBaseId() - 1) * 10 + OFFS_INIT_VAR_EXT) + OFFSET_BTN_OPEN_MODELES + ID_INIT_VAR_BTN + ADD_INIT_VAR;
    return getId() + OFFSET_BTN_OPEN_MODELES;
    }

  uint idSec = ID_INIT_SIMPLE_TXT == getBaseId() ? ADD_MIDDLE_VAR : MAX_THIRD_OFFSET;
  if(getId() < getBaseId() + idSec)
    return getId() + ADD_INIT_VAR + ADD_INIT_SECOND + ADD_INIT_BITFIELD;

  return getId() + ADD_INIT_VAR_EXT + ADD_INIT_SECOND_EXT + ADD_INIT_THIRD_EXT;// + OFFS_INIT_VAR_EXT;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static
LPCTSTR skipFirstFolder(LPCTSTR p)
{
  LPCTSTR p2 = p;
  while(*p2) {
    if(_T('\\') == *p2 || _T('/') == *p2) {
      p = p2 + 1;
      break;
      }
    ++p2;
    }
  return p;
}
//----------------------------------------------------------------------------
class myManageIni : public manageIni
{
  private:
    typedef manageIni baseClass;
  public:
    myManageIni(LPCTSTR filename);
};
//----------------------------------------------------------------------------
static void makeModulePath(LPTSTR target, size_t dim, LPCTSTR filename)
{
  LPCTSTR pf = filename + _tcslen(filename) - 1;
  while (pf > filename) {
    if (_T('\\') == *pf || _T('/') == *pf) {
      ++pf;
      break;
    }
    --pf;
  }
  GetModuleDirName(dim, target);
  appendPath(target, pf);
}
//----------------------------------------------------------------------------
myManageIni::myManageIni(LPCTSTR filename) : baseClass(filename)
{
  TCHAR t[_MAX_PATH];
  makeModulePath(t, SIZE_A(t), filename);
  delete []Filename;
  Filename = str_newdup(t);
}
//----------------------------------------------------------------------------
void setKeyPath(LPCTSTR keyName, LPCTSTR path)
{
  myManageIni ini(CHECK_ADDR_INI_FILE);
  ini.parse();
  ini_param param = { keyName, path };
  ini.addParam(param, MAIN_PATH);
  ini.save();
}
//----------------------------------------------------------------------------
void getKeyPath(LPCTSTR keyName, LPTSTR path)
{
  path[0] = 0;
  myManageIni ini(CHECK_ADDR_INI_FILE);
  ini.parse();
  LPCTSTR str = ini.getValue(keyName, MAIN_PATH);
  if(str)
    _tcscpy_s(path, _MAX_PATH, str);
}
//----------------------------------------------------------------------------
void setPath(LPCTSTR path)
{
  setKeyPath(REL_SYSTEM_PATH, path);
}
//----------------------------------------------------------------------------
void getPath(LPTSTR path)
{
  getKeyPath(REL_SYSTEM_PATH, path);
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void checkBasePage()
{
  int successCount = 0;
  WIN32_FIND_DATA FindFileData;
  LPCTSTR toFind[] = {
    _T("nps*.exe"),
    _T("prph_*.exe"),
    _T("prph_*.ad7"),
    _T("prph_*.dat"),
    _T("job_base.dat"),
    _T("system\\") FIRST_PAGE,
    _T("system\\") STD_MSG_NAME,
    };
  for(uint i = 0; i < SIZE_A(toFind); ++i) {
    HANDLE hf = FindFirstFile(toFind[i], &FindFileData);
    if(hf != INVALID_HANDLE_VALUE) {
      ++successCount;
      FindClose(hf);
      }
    }
  if(successCount >= 4) {
    TCHAR path[_MAX_PATH];
    GetCurrentDirectory(SIZE_A(path), path);
    setPath(path);
    }
}
//----------------------------------------------------------------------------
//------------------------------------------------------------------------------
setOfImages::~setOfImages()
{
  reset();
}
//------------------------------------------------------------------------------
void setOfImages::addImage(LPCTSTR image, LPCTSTR page)
{
  myStrImage str(image, page);
  int pos = Images.insertEx(str);
  if(pos < 0)
    return;
  Images[pos].P = str_newdup(image);
  Images[pos].Page =  str_newdup(page);
}
//------------------------------------------------------------------------------
const myStrImage& setOfImages::getImage(uint pos) const
{
  uint nElem = getElem();
  if(pos >= nElem)
    pos = 0;
  return Images[pos];
}
//------------------------------------------------------------------------------
bool setOfImages::exist(LPCTSTR image) const
{
  myStrImage str(image);
  uint pos;
  return Images.find(str, pos);
}
//------------------------------------------------------------------------------
void setOfImages::reset()
{
  uint nElem = getElem();
  for(uint i = 0; i < nElem; ++i) {
    delete []Images[i].P;
    delete []Images[i].Page;
  }
  Images.setElem(0);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
setOfPages::~setOfPages()
{
  reset();
}
//------------------------------------------------------------------------------
    // se result != failed torna il puntatore all'oggetto (nuovo o esistente)
    // level, in caso di esistenza, viene aggiornato al livello più alto (valore minore)
setOfPages::result setOfPages::addPage(LPCTSTR page, const infoPages*  infoParent,
            const infoPrph* openedBy, uint level, infoPages** target)
{
  uint nElem = getElem();
#if 1
  int upp = nElem - 1;
  int mid = 0;
  int low = 0;
  while(low <= upp) {
    mid = (upp + low) / 2;
    infoPages* ip = Pages[mid];
    int result = _tcsicmp(page, ip->getPageCurr());
    if(!result) {
      const infoPrph* curr = ip->getInfoOpenedBy();
      if(curr)
        result = openedBy->getOffsLink() - curr->getOffsLink();
      if(!result) {
        ip->setLevel(level);
        *target = ip;
        ip->addParent(infoParent);
        return exist;
        }
      }
    if(result < 0)
      upp = mid - 1;
    else
      low = ++mid;
    }
  *target = new infoPages(page, infoParent, openedBy, level);
  if(!nElem)
    Pages[nElem] = *target;
  else
    Pages.insert(*target, mid);
  return added;

#else
  for(uint i = 0; i < nElem; ++i) {
    infoPages* ip = Pages[i];
    if(!_tcsicmp(ip->getPageCurr(), page)) {
      ip->setLevel(level);
      *target = ip;
      ip->addParent(infoParent);
      return exist;
      }
    }
  *target = new infoPages(page, infoParent, openedBy, level);
  Pages[nElem] = *target;
  return added;
#endif
}
//------------------------------------------------------------------------------
bool setOfPages::existPage(LPCTSTR page) const
{
#if 1
  return toBool(getPageInfo(page));
#else
  uint nElem = getElem();
  int upp = nElem - 1;
  int mid = 0;
  int low = 0;
  while(low <= upp) {
    mid = (upp + low) / 2;
    infoPages* ip = Pages[mid];
    int result = _tcsicmp(page, ip->getPageCurr());
    if(!result)
      return true;
    if(result < 0)
      upp = mid - 1;
    else
      low = ++mid;
    }
  return false;
#endif
}
//------------------------------------------------------------------------------
LPCTSTR setOfPages::getPage(uint pos) const
{
  uint nElem = getElem();
  if(pos >= nElem)
    return 0;
  return Pages[pos]->getPageCurr();
}
//------------------------------------------------------------------------------
LPCTSTR setOfPages::getPage(LPCTSTR name) const
{
  const infoPages* ip = getPageInfo(name);
  if(ip)
    return ip->getPageCurr();

  return 0;
}
//------------------------------------------------------------------------------
const infoPages* setOfPages::getPageInfo(LPCTSTR name) const
{
#if 1
  uint nElem = getElem();
  int upp = nElem - 1;
  int mid = 0;
  int low = 0;
  while(low <= upp) {
    mid = (upp + low) / 2;
    infoPages* ip = Pages[mid];
    int result = _tcsicmp(name, ip->getPageCurr());
    if(!result)
      return ip;
    if(result < 0)
      upp = mid - 1;
    else
      low = ++mid;
    }
  return 0;
#else
  uint nElem = getElem();
  for(uint i = 0; i < nElem; ++i) {
    infoPages* ip = Pages[i];
    if(!_tcsicmp(ip->getPageCurr(), name))
      return ip;
    }
  return 0;
#endif
}
//------------------------------------------------------------------------------
const infoPages* setOfPages::getPageInfo(uint pos) const
{
  uint nElem = getElem();
  if(pos >= nElem)
    return 0;
  return Pages[pos];
}
//------------------------------------------------------------------------------
void addSymbFile(LPTSTR filename)
{
//  TCHAR tmp[_MAX_PATH];
  TCHAR disk[_MAX_DRIVE];
  TCHAR dir[_MAX_DIR];
  TCHAR file[_MAX_FNAME] = _T("#");
  TCHAR ext[_MAX_FNAME];
  _tsplitpath(filename, disk, dir, file + 1, ext);
  _tmakepath(filename, disk, dir, file, ext);
}
//---------------------------------------------------------------------
void pageStrToSet(setOfString& sos, LPCTSTR filename)
{
  TCHAR file2[_MAX_PATH];
  _tcscpy(file2, filename);
  addSymbFile(file2);

  manageCryptPage mcp;
  bool crypted = mcp.isCrypted(file2);
  if(crypted)
    mcp.makePathAndDecrypt(file2);
  sos.add(file2);
  if(crypted)
    mcp.releaseFileDecrypt(file2);
  sos.add(filename);
}
//----------------------------------------------------------------------------
void infoPrph::init()
{
  Prph = 0;
  Addr = 0;
  dataType = 0;
  nBit = 0;
  Offs = 0;

  Page = 0;
  pageType = 0;
  objType = 0;
  action = 0;
  openPage = 0;
  X = 0;
  Y = 0;

  Norm = 0;

  OffsLink = 0;
}
//----------------------------------------------------------------------------
#define RET_CHECK(v) { \
  int result = GET_pV(toCheck, v) - GET_pV(current, v); \
  if(result) \
    return result; }
//----------------------------------------------------------------------------
#define RET_CHECK_STR(v) { \
  LPCTSTR t1 = GET_pV(toCheck, v); \
  if(!t1)\
    t1 = _T("\0");\
  LPCTSTR t2 = GET_pV(current, v); \
  if(!t2)\
    t2 = _T("\0");\
  int result = _tcsicmp(t1, t2); \
  if(result) \
    return result; }
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
int setOfPrphByPage::Cmp(const TreeData toCheck, const TreeData current) const
{
  RET_CHECK_STR(Page);

  RET_CHECK(Prph);
  RET_CHECK(Addr);
  RET_CHECK(dataType);
  RET_CHECK(nBit);
  RET_CHECK(Offs);
  RET_CHECK(Norm);

  RET_CHECK(objType);
  RET_CHECK(X);
  RET_CHECK(Y);
  return 0;
}
//----------------------------------------------------------------------------
int setOfPrphByPrph::Cmp(const TreeData toCheck, const TreeData current) const
{
  RET_CHECK(Prph);
  RET_CHECK(Addr);
  RET_CHECK(dataType);
  RET_CHECK(nBit);
  RET_CHECK(Offs);
  RET_CHECK(Norm);

  RET_CHECK_STR(Page);
  RET_CHECK(objType);
  RET_CHECK(X);
  RET_CHECK(Y);
  return 0;
}
//----------------------------------------------------------------------------
bool setOfPrphNorm::Add(infoPrph* prph)
{
  int norm = GET_pV(prph, Norm);
  // è stato normalizzato
  if(norm > 2)
//  if(norm > 2 || norm < -2)
    return baseClass::Add(prph);
  return true;
}
//----------------------------------------------------------------------------
int setOfPrphNorm::Cmp(const TreeData toCheck, const TreeData current) const
{
  RET_CHECK(Norm);
  RET_CHECK(Prph);
  RET_CHECK(Addr);
  RET_CHECK(dataType);
  RET_CHECK(nBit);
  RET_CHECK(Offs);

  RET_CHECK_STR(Page);
  RET_CHECK(objType);
  RET_CHECK(X);
  RET_CHECK(Y);
  return 0;
}
//----------------------------------------------------------------------------
bool setOfPrphOther::Add(infoPrph* prph)
{
  uint type = GET_pV(prph, pageType);
  if(infoPrph::eStdMsg == type || infoPrph::eDll == type || infoPrph::eNorm == type || infoPrph::eAlarmAssoc == type)
    return baseClass::Add(prph);
  return true;
}
//----------------------------------------------------------------------------
int setOfPrphOther::Cmp(const TreeData toCheck, const TreeData current) const
{
  // std_msg o dll
  RET_CHECK(pageType);
  // std_msg -> nome del trend (se abbinata al trend), altrimenti conterrà l'informazione sull'utilizzo
  // dll -> nome della dll
  RET_CHECK_STR(Page);

  RET_CHECK(Prph);
  RET_CHECK(Addr);
  RET_CHECK(dataType);
  RET_CHECK(nBit);
  RET_CHECK(Offs);

  // dll -> numero della variabile (es. $2)
  RET_CHECK(Y);
  return 0;
}
//----------------------------------------------------------------------------
//#undef RET_CHECK_STR
//----------------------------------------------------------------------------
uint actionToPagetype(uint action)
{
  uint pageType = 0;
  switch(action) {
    case infoPrph::eOpenBody:
    case infoPrph::eOpenTrend:
    case infoPrph::eOpenRecipe:
    case infoPrph::eOpenRecipeRow:
    case infoPrph::eOpenMaint:
      pageType = infoPrph::eBody;
      break;
    case infoPrph::eOpenModal:
      pageType = infoPrph::eModal;
      break;
    case infoPrph::eOpenModeless:
      pageType = infoPrph::eModeless;
      break;
    case infoPrph::eOpenLink1:
      pageType = infoPrph::eLink1;
      break;
    case infoPrph::eOpenLink2:
      pageType = infoPrph::eLink2;
      break;

    case infoPrph::noAct:
    case infoPrph::eVisibility:
    case infoPrph::eActionMove:
      break;
    }
  return pageType;
}
//----------------------------------------------------------------------------
infoPages::infoPages(LPCTSTR PageCurr, const infoPages*  info_parent, const infoPrph* openedBy, uint level) :
        PageCurr(str_newdup(PageCurr)), openedBy(openedBy), Level(level), pageType(0)
{
  if(info_parent) {
    infoParent[0] = info_parent;
    }
  if(openedBy)
    pageType = actionToPagetype(GET_pV(openedBy, action));
}
//------------------------------------------------------------------------------
infoPages::~infoPages()
{
  delete []PageCurr;
}
//------------------------------------------------------------------------------
void infoPages::addChild(infoPages* child)
{
  uint nElem = PageChild.getElem();

  const PVect<infoPages*>&cpc = PageChild;
  for(uint i = 0; i < nElem; ++i)
    if(cpc[i] == child) {
//      delete child;
      return;
      }
  PageChild[nElem] = child;
}
//------------------------------------------------------------------------------
void infoPages::addParent(const infoPages* parent)
{
  uint nElem = infoParent.getElem();

  const PVect<const infoPages*>&cip = infoParent;
  for(uint i = 0; i < nElem; ++i)
    if(cip[i] == parent)
      return;

  infoParent[nElem] = parent;
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
managePages::managePages(LPCTSTR path, LPCTSTR firstPage) :
      Path(str_newdup(path)), firstPage(str_newdup(firstPage)),
      currOrder(byPrph), Root(0), Header(0), currPage(0),
      defPageType(infoPrph::eBody), realPlc(2), use64bit4Alarmfilter(0)
{
  TCHAR t[_MAX_PATH];
  _tcscpy(t, Path);
  appendPath(t, firstPage);
  if(!P_File::P_exist(t)) {
    _tcscpy(t, Path);
    appendPath(t, _T("system"));
    delete []Path;
    Path = str_newdup(t);
    }

  GetCurrentDirectory(SIZE_A(oldPath), oldPath);
  SetCurrentDirectory(Path);
}
//----------------------------------------------------------------------------
static void clearNames(PVect<myStr>& names)
{
  uint nElem = names.getElem();
  for(uint i = 0; i < nElem; ++i)
    delete []names[i].P;
  names.reset();
}
//----------------------------------------------------------------------------
managePages::~managePages()
{
  clearNames(stdPages);
  delete []Path;
  delete []firstPage;
  SetCurrentDirectory(oldPath);
  flushPV(setScript);
}
//---------------------------------------------------------
inline
bool isValid(unsigned v)
{
  return _istdigit(v) || _istalpha(v) || _T('_') == v;
}
//---------------------------------------------------------
#define MAX_VAR_LEN 64
//---------------------------------------------------------
LPCTSTR extractVar(LPTSTR target, LPCTSTR buff)
{
  if(!buff || !*buff)
    return 0;

  for(uint i = 0; i < MAX_VAR_LEN; ++i, ++buff) {
    target[i] = *buff;
    if(!isValid(target[i])) {
      target[i] = 0;
      break;
      }
    }
  return buff;
}
//----------------------------------------------------------------------------
uint managePages::getIdVarScript(LPCTSTR varName)
{
  uint nElem = setScript.getElem();
  for(uint i = 0; i < nElem; ++i)
    if(!_tcscmp(varName, setScript[i]->name))
      return setScript[i]->id;

  scriptMan* sm = new scriptMan;
  sm->id = nElem + 1;
  sm->name = str_newdup(varName);
  setScript[nElem] = sm;
  return sm->id;
}
//----------------------------------------------------------------------------
LPCTSTR managePages::getVarNameScript(uint id)
{
  uint nElem = setScript.getElem();
  for(uint i = 0; i < nElem; ++i)
    if(id == setScript[i]->id)
      return setScript[i]->name;
  return 0;
}
//----------------------------------------------------------------------------
void managePages::addGlobalPrph(infoPrph* prph)
{
  soPrph.add(prph);
}
//----------------------------------------------------------------------------
#define MAX_FUNCT_BTN 12

#define ID_INIT_FUNCT_BTN 14
//----------------------------------------------------------------------------
void managePages::addFromFunct(LPCTSTR name, infoPages* iP, infoPrph::eAction action, uint level, int x, int y, infoPrph::eObjType obj)
{
  if(!name)
    return;

  const infoPrph* prphT = iP->getInfoOpenedBy();
  uint pgType = defPageType;
  if(prphT)
    pgType = actionToPagetype(GET_pV(prphT, action));

  infoPrph* prph = new infoPrph();

  // fill prph
  SET_pO(prph, Page, iP->getPageCurr());
  SET_pO(prph, pageType, pgType);
  SET_pO(prph, objType, obj);
  SET_pO(prph, action, action);
  SET_pO(prph, X, x);
  SET_pO(prph, Y, y);
  // se è una azione sullo spin non carica una pagina, ma va salvato così com'è
  if(infoPrph::eActionSpin == action) {
    // poiché il nome viene salvato in un set apposito, andrebbe aggiunto in quel set
    // prima di includerlo nelle info della periferica, meglio mettere i due valori
    // rilevanti, up/down(1/0) e idspin, direttamente nei campi nbit e offset
    int act = _ttoi(name);
    SET_pO(prph, nBit, act); // => up-down (1-0)
    name = findNextParam(name);
    if(name) {
      act = _ttoi(name);
      SET_pO(prph, Offs, act); // => idSpin
      }
    addGlobalPrph(prph);
    }
  else {
  // lo mettiamo temporaneamente, ma poi ci andrà quello allocato dalla loadPage()
    SET_pO(prph, openPage, name);
    infoPages* child = loadPage(name, iP, prph, level + 1);
    if(child) {
      SET_pO(prph, openPage, child->getPageCurr());
      iP->addChild(child);
      addGlobalPrph(prph);
      }
    else
      delete prph;
    }
}
//----------------------------------------------------------------------------
LPCTSTR checkAction(uint id, infoPrph::eAction& act, LPCTSTR p)
{
  switch(id) {
    case 1: // new body standard or by id
      act = infoPrph::eOpenBody;
      return findNextParam(p, 3);

    case 6: // open trend
      act = infoPrph::eOpenTrend;
      do {
        LPCTSTR p2 = findNextParam(p, 3);
        if(p2)
          return p2;
        } while(false);
      return findNextParam(p, 2);

    case 7:  // open recipe
      do {
        LPCTSTR p2 = findNextParam(p, 3);
        if(p2)
          p = p2;
        else
          p = findNextParam(p, 2);
        } while(false);

      act = infoPrph::eOpenRecipe;
      return p;

    case 8:  // open recipe row
      act = infoPrph::eOpenRecipeRow;
      return findNextParam(p, 2);

    case 9: // open modal
      act = infoPrph::eOpenModal;
      p = findNextParam(p, 2);
      if(_T('@') == *p)
        ++p;
      return p;

    case 11: // open modeless
      act = infoPrph::eOpenModeless;
      p = findNextParam(p, 2);
      if(_T('@') == *p)
        ++p;
      return p;

    case 18: // open maint
      act = infoPrph::eOpenMaint;
      return findNextParam(p, 2);

    case 22: // spin
      act = infoPrph::eActionSpin;
      return findNextParam(p, 2);
    }
  return 0;
}
//----------------------------------------------------------------------------
#define ID_SHIFT_F1   90001
#define ID_CTRL_F1    90031
//----------------------------------------------------------------------------
#define ID_SHIFT_F1_ACT  (ID_SHIFT_F1 + MAX_FUNCT_BTN)
#define ID_CTRL_F1_ACT   (ID_CTRL_F1 + MAX_FUNCT_BTN)
//----------------------------------------------------------------------------
void managePages::checkMenu(infoPages* iP, setOfString& sos, uint level)
{
  // verifica tasti funzione
  uint ids[] = { ID_INIT_FUNCT_BTN, ID_SHIFT_F1_ACT, ID_CTRL_F1_ACT };
  for(uint j = 0; j < SIZE_A(ids); ++j) {
    for(uint i = 0; i < MAX_FUNCT_BTN; ++i) {
      LPCTSTR p = sos.getString(ids[j] + i);
      if(!p)
        continue;
      uint id = _ttoi(p);
      infoPrph::eAction act = infoPrph::noAct;
      p = checkAction(id, act, p);
      if(p)
        addFromFunct(p, iP, act, level, COORD_BY_FUNCT, i + MAX_FUNCT_BTN * j, infoPrph::eFunct);
      }
    }
}
//----------------------------------------------------------------------------
#define ID_PAGE_DYNAMIC_LINK    28
#define ID_PAGE_DYNAMIC_LINK_V2 29
#define ID_PAGE_DYNAMIC_LINK_BITS_OFFS 97
//----------------------------------------------------------------------------
void managePages::checkLinked(infoPages* iP, setOfString& sos, uint level)
{
  uint ids[] = { ID_PAGE_DYNAMIC_LINK, ID_PAGE_DYNAMIC_LINK_V2 };
  for(uint i = 0; i < SIZE_A(ids); ++i) {
    LPCTSTR p = sos.getString(ids[i]);
    if(!p)
      continue;
    dualOffset dOffs;
    dOffs.offset = 0;

    dOffs.offsAddr = _ttoi(p);

    p = findNextParam(p, 1 + i);
    if(!p)
      continue;
    LPCTSTR p2 = sos.getString(ID_PAGE_DYNAMIC_LINK_BITS_OFFS);
    if(p2) {
      dOffs.offsBit = _ttoi(p2);
      int alsoVisib = 0;
      p2 = findNextParamTrim(p2);
      if(p2)
        dOffs.alsoVisib = _ttoi(p2);
      }
    infoPrph* prph = new infoPrph();

    // fill prph
    SET_pO(prph, Page, iP->getPageCurr());
    SET_pO(prph, pageType, infoPrph::eLink1 + i);
    SET_pO(prph, openPage, p);
    SET_pO(prph, X, COORD_BY_FUNCT);
    SET_pO(prph, Y, COORD_BY_FUNCT);
    SET_pO(prph, action, infoPrph::eOpenLink1 + i);
    SET_pO(prph, OffsLink, i ? 0 : dOffs.offset);
    infoPages* child = loadPage(p, iP, prph, level + 1);
    if(child) {
      SET_pO(prph, openPage, child->getPageCurr());
      iP->addChild(child);
      addGlobalPrph(prph);
      }
    else
      delete prph;
    }
}
//---------------------------------------------------------------------
#define DEF_BIT_4_VISIB 32
//---------------------------------------------------------------------
template <typename TA, typename TOFFS>
static void calcAddrAndOffs(TA &addr, TOFFS& offsvar, int offsaddr, int offsbit, uint nbitType)
{
  if(!nbitType)
    nbitType = DEF_BIT_4_VISIB;
  addr += offsaddr;
  addr *= nbitType;
  addr += offsvar + offsbit;
  offsvar = addr % nbitType;
  addr /= nbitType;
}
//----------------------------------------------------------------------------
static int getNBit(int type)
{
    enum tData {  tNoData,  // nessun tipo definito -  0
                  tBitData, // campo di bit         -  1
                  tBData,   // 8 bit -> 1 byte      -  2
                  tWData,   // 16 bit-> 2 byte      -  3
                  tDWData,  // 32 bit-> 4 byte      -  4
                  tFRData,  // 32 bit float         -  5
                  ti64Data, // 64 bit               -  6
                  tRData,   // 64 bit float         -  7
                  tDateHour,// 64 bit per data + ora-  8
                  tDate,    // 64 bit per data      -  9
                  tHour,     // 64 bit per ora      - 10
                  // valori signed
                  tBsData,   // 8 bit -> 1 byte      -  11
                  tWsData,   // 16 bit-> 2 byte      -  12
                  tDWsData,  // 32 bit-> 4 byte      -  13

                  tStrData,  // struttura formata dalla dimensione del buffer
                             // ed il puntatore (32 + 32 bit)

                  tNegative = 1 << 30,
//                  tArray = 1 << 31,
               };

  switch(type) {
    case tBData:
    case tBsData:
    case tStrData:
      return 8;
    case tWData:
    case tWsData:
      return 16;
    case tDWData:
    case tDWsData:
    case tFRData:
      return 32;
    case ti64Data:
    case tRData:
    case tDateHour:
    case tDate:
    case tHour:
      return 64;
    case tBitData:
    case tNoData:
      return 0;
    }
  return 0;
}
//----------------------------------------------------------------------------
#define GET_NBIT(a) getNBit(a)
//----------------------------------------------------------------------------
infoPrph* managePages::addSingleObj(setOfString& sos, LPCTSTR p1, LPCTSTR p2, infoPages* iP, uint obj, int x, int y, uint nVar)
{
  if(!p1)
    return 0;
  int pr = 0;
  int addr = 0;
  int type = 0;
  int norm = 0;
  int dec = 0;
  _stscanf(p1, _T("%d,%d,%d,%d,%d"),  &pr, &addr, &type, &norm, &dec);

  int nBit = 0;
  int offs = 0;
  if(p2) {
    _stscanf(p2, _T("%d,%d"),  &nBit, &offs);
    p2 = findNextParam(p2, 2);
    if(p2) {
      addVisibility(p2, iP, obj, x, y);
      addMovement(sos, p2, iP, obj, x, y);
      }
    }

  const infoPrph* prphT = iP->getInfoOpenedBy();
  uint pgType = defPageType;
  dualOffset dOffs;
  dOffs.offset = 0;

  if(prphT) {
    GET_pO(prphT, pageType, pgType);
    GET_pO(prphT, OffsLink, dOffs.offset);
    }
  uint repeat = 1;
  infoPrph* firstPrph = 0;
  for(uint j = 0; j < nVar; ++j) {
    repeat = 1;
    switch(type) {
      case 14:
        repeat = ROUND_DWORD_LEN(dec);
        break;
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
        repeat = 2;
        break;
      }

    for(uint i = 0; i < repeat; ++i, ++addr) {
      infoPrph* prph = new infoPrph;
      if(!j && !i)
        firstPrph = prph;

    // fill prph
      SET_pO(prph, Page, iP->getPageCurr());
      SET_pO(prph, pageType, pgType);

      SET_pO(prph, X, x);
      SET_pO(prph, Y, y);

      SET_pO(prph, Prph, pr);
      if(PRF_MEMORY == pr && addr <= 1)
        SET_pO(prph, Addr, addr);
      else {
        int addr2 = addr;
        if(dOffs.offset)
          calcAddrAndOffs(addr2, offs, dOffs.offsAddr, dOffs.offsBit, GET_NBIT(type));
        SET_pO(prph, Addr, addr2);
        SET_pO(prph, OffsLink, dOffs.offset);
        }
      SET_pO(prph, Norm, norm);
      SET_pO(prph, dataType, type);
      SET_pO(prph, nBit, nBit);
      SET_pO(prph, Offs, offs);

      SET_pO(prph, objType, obj);
      addGlobalPrph(prph);
      }
    }
  return firstPrph;
}
//----------------------------------------------------------------------------
void managePages::addSingleObjByMov(LPCTSTR p1, LPCTSTR p2, infoPages* iP, uint obj, int x, int y)
{
  if(!p1)
    return;
  int pr = 0;
  int addr = 0;
  int type = 0;
  int norm = 0;
  int dec = 0;
  _stscanf(p1, _T("%d,%d,%d,%d,%d"),  &pr, &addr, &type, &norm, &dec);

  int nBit = 0;
  int offs = 0;
  if(p2)
    _stscanf(p2, _T("%d,%d"),  &nBit, &offs);

  const infoPrph* prphT = iP->getInfoOpenedBy();
  uint pgType = defPageType;

  dualOffset dOffs;
  dOffs.offset = 0;

  if(prphT) {
    GET_pO(prphT, pageType, pgType);
    GET_pO(prphT, OffsLink, dOffs.offset);
    }

  uint repeat = 1;
  switch(type) {
    case 14:
      repeat = ROUND_DWORD_LEN(dec);
      break;
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
      repeat = 2;
      break;
    }

  for(uint i = 0; i < repeat; ++i, ++addr) {
    infoPrph* prph = new infoPrph;

  // fill prph
    SET_pO(prph, Page, iP->getPageCurr());
    SET_pO(prph, pageType, pgType);

    SET_pO(prph, X, x);
    SET_pO(prph, Y, y);

    SET_pO(prph, Prph, pr);
    if(!pr ||(PRF_MEMORY == pr && addr <= 1))
      SET_pO(prph, Addr, addr);
    else {
      if(dOffs.offset)
        calcAddrAndOffs(addr, offs, dOffs.offsAddr, dOffs.offsBit, GET_NBIT(type));
      SET_pO(prph, Addr, addr);
      SET_pO(prph, OffsLink, dOffs.offset);
      }
    SET_pO(prph, Norm, norm);
    SET_pO(prph, dataType, type);
    SET_pO(prph, nBit, nBit);
    SET_pO(prph, Offs, offs);
    SET_pO(prph, action, infoPrph::eActionMove);

    SET_pO(prph, objType, obj);
    addGlobalPrph(prph);
    }
}
//----------------------------------------------------------------------------
#define OFFS_MOVE 6
//----------------------------------------------------------------------------
void managePages::addMovement(setOfString& sos, LPCTSTR p, infoPages* iP, uint obj, int x, int y)
{
  p = findNextParamTrim(p, OFFS_MOVE);
  if(!p)
    return;
  int code = _ttoi(p);
  if(!code)
    return;
  p = sos.getString(code);
  if(!p)
    return;

  p = findNextParamTrim(p, 4);

  const uint nElem = 6;
  for(uint j = 0; j < nElem && p; ++j) {
    int id = _ttoi(p);
    p = findNextParam(p, 1);
    manageObjId moi2(id);
    LPCTSTR pt = sos.getString(moi2.getFirstExtendId());
    LPCTSTR pt2 = sos.getString(moi2.getSecondExtendId());
    addSingleObjByMov(pt, pt2, iP, obj, x, y);
    }
}
//----------------------------------------------------------------------------
void managePages::addVisibility(LPCTSTR p, infoPages* iP, uint obj, int x, int y)
{
  int pr = 0;
  int addr = 0;
  int nBit = 0;
  int offs = 0;
  _stscanf(p, _T("%d,%d,%d,%d"),  &pr, &addr, &nBit, &offs);
  if(!pr)
    return;

  const infoPrph* prphT = iP->getInfoOpenedBy();
  uint pgType = defPageType;
  dualOffset dOffs;
  dOffs.offset = 0;
  if(prphT) {
    GET_pO(prphT, pageType, pgType);
    GET_pO(prphT, OffsLink, dOffs.offset);
    }

  infoPrph* prph = new infoPrph;

  // fill prph
  SET_pO(prph, Page, iP->getPageCurr());
  SET_pO(prph, pageType, pgType);

  SET_pO(prph, X, x);
  SET_pO(prph, Y, y);

  SET_pO(prph, Prph, pr);
  if(PRF_MEMORY == pr && addr <= 1)
    SET_pO(prph, Addr, addr);
  else {
    if(dOffs.alsoVisib)
      calcAddrAndOffs(addr, offs, dOffs.offsAddr, dOffs.offsBit, GET_NBIT(4));
    SET_pO(prph, Addr, addr);
    SET_pO(prph, OffsLink, dOffs.offset);
    }
  SET_pO(prph, dataType, 4);
  SET_pO(prph, nBit, nBit);
  SET_pO(prph, Offs, offs);

  SET_pO(prph, objType, obj);
  SET_pO(prph, action, infoPrph::eVisibility);
  addGlobalPrph(prph);

}
//----------------------------------------------------------------------------
infoPrph* managePages::addInfoPrphByVars(infoPages* iP, setOfString& sos, uint idBase, uint ix, infoPrph::eObjType obj, uint pgType)
{
  manageObjId moi(0, idBase);
  uint id = moi.calcAndSetExtendId(ix);
  LPCTSTR p = sos.getString(id);
  if(!p)
    return 0;

  int x = _ttoi(p);
  p = findNextParam(p, 1);
  if(!p)
    return 0;
  int y = _ttoi(p);

  p = sos.getString(moi.getFirstExtendId());

  LPCTSTR p2 = sos.getString(moi.getSecondExtendId());
  infoPrph* prph = addSingleObj(sos, p, p2, iP, obj, x, y);
  return prph;
}
//----------------------------------------------------------------------------
infoPrph* managePages::addInfoPrphByAltText(infoPages* iP, setOfString& sos, uint idBase, uint ix, infoPrph::eObjType obj, uint pgType, int offsAltText)
{
  manageObjId moi(0, idBase);
  uint id = moi.calcAndSetExtendId(ix);
  LPCTSTR p = sos.getString(id);
  if(!p)
    return 0;
  int x = _ttoi(p);
  p = findNextParam(p, 1);
  if(!p)
    return 0;
  int y = _ttoi(p);

  p = findNextParam(p, offsAltText - 1);
  if(!p)
    return 0;

  id = _ttoi(p);
  p = sos.getString(id);
  if(!p)
    return 0;

  int prp = _ttoi(p);
  if(!prp)
    return 0;
  p = findNextParam(p, 1);
  if(!p)
    return 0;
  int addr = _ttoi(p);
  p = findNextParam(p, 1);
  if(!p)
    return 0;
  int dim = _ttoi(p);

  TCHAR tmp[256];
  wsprintf(tmp, _T("%d,%d,14,0,%d"), prp, addr, dim);
  infoPrph* prph = addSingleObj(sos, tmp, 0, iP, obj, x, y);
  return prph;
}

//----------------------------------------------------------------------------
void managePages::checkVars(infoPages* iP, setOfString& sos, uint idGroup, uint idBase, infoPrph::eObjType obj)
{
  LPCTSTR p = sos.getString(idGroup);
  if(!p)
    return;

  const infoPrph* prphT = iP->getInfoOpenedBy();
  uint pgType = defPageType;
  if(prphT)
    GET_pO(prphT, pageType, pgType);

  int nObj = _ttoi(p);
  for(int i = 0; i < nObj; ++i)
    addInfoPrphByVars(iP, sos, idBase, i, obj, pgType);
}
//----------------------------------------------------------------------------
void managePages::checkVarsAlt(infoPages* iP, setOfString& sos, uint idGroup, uint idBase, infoPrph::eObjType obj, int offsAltText)
{
  LPCTSTR p = sos.getString(idGroup);
  if(!p)
    return;

  const infoPrph* prphT = iP->getInfoOpenedBy();
  uint pgType = defPageType;
  if(prphT)
    GET_pO(prphT, pageType, pgType);

  int nObj = _ttoi(p);
  for(int i = 0; i < nObj; ++i) {
    addInfoPrphByVars(iP, sos, idBase, i, obj, pgType);
    addInfoPrphByAltText(iP, sos, idBase, i, obj, pgType, offsAltText);
    }
}

//----------------------------------------------------------------------------
void managePages::checkBmp(infoPages* iP, setOfString& sos)
{
  LPCTSTR p = sos.getString(ID_VAR_BMP);
  if(!p)
    return;

  const infoPrph* prphT = iP->getInfoOpenedBy();
  uint pgType = defPageType;
  if(prphT)
    GET_pO(prphT, pageType, pgType);

  int nObj = _ttoi(p);
  for(int i = 0; i < nObj; ++i) {
    addInfoPrphByVars(iP, sos, ID_INIT_VAR_BMP, i, infoPrph::eBmp, pgType);

    manageObjId moi(0, ID_INIT_VAR_BMP);
    moi.calcAndSetExtendId(i);
    uint id = moi.getThirdExtendId();
    p = sos.getString(id);
    if(!p)
      continue;
    int x = _ttoi(p);
    p = findNextParamTrim(p);
    if(!p)
      continue;
    int y = _ttoi(p);
    p = findNextParamTrim(p, 3);
/*
    if(!p)
      continue;
    const infoPrph* prphT = iP->getInfoOpenedBy();
    uint pgType = defPageType;
    if(prphT)
      GET_pO(prphT, pageType, pgType);
*/
    const uint nElem = 6;
    for(uint j = 0; j < nElem && p; ++j) {
      int id = _ttoi(p);
      p = findNextParamTrim(p);
      manageObjId moi2(id);
      LPCTSTR pt = sos.getString(moi2.getFirstExtendId());
      LPCTSTR pt2 = sos.getString(moi2.getSecondExtendId());
      addSingleObj(sos, pt, pt2, iP, infoPrph::eBmp, x, y);
      }
    }
}
//----------------------------------------------------------------------------
void managePages::checkBarGraph(infoPages* iP, setOfString& sos)
{

  LPCTSTR p = sos.getString(ID_BARGRAPH);
  if(!p)
    return;

  const infoPrph* prphT = iP->getInfoOpenedBy();
  uint pgType = defPageType;
  if(prphT)
    GET_pO(prphT, pageType, pgType);

  int nObj = _ttoi(p);
  for(int i = 0; i < nObj; ++i) {
    manageObjId moi(0, ID_INIT_BARGRAPH);
    uint id = moi.calcAndSetExtendId(i);
    p = sos.getString(id);
    if(!p)
      continue;


    int x = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      continue;
    int y = _ttoi(p);

    p = findNextParam(p, 7);

    for(uint j = 0; j < 2 && p;++j) {
      uint idV = _ttoi(p);
      p = findNextParam(p, 1);
      manageObjId moi2(idV);
      LPCTSTR pt = sos.getString(moi2.getFirstExtendId());
      LPCTSTR pt2 = sos.getString(moi2.getSecondExtendId());
      addSingleObj(sos, pt, pt2, iP, infoPrph::eBarGraph, x, y);
      }

    p = sos.getString(moi.getFirstExtendId());
    p = findNextParam(p, 2);
    if(p) {
      addVisibility(p, iP, infoPrph::eBarGraph, x, y);
      addMovement(sos, p, iP, infoPrph::eBarGraph, x, y);
      }
    }
}
//----------------------------------------------------------------------------
void managePages::checkSimple(infoPages* iP, setOfString& sos, uint idGroup, uint idInit, infoPrph::eObjType obj)
{
  LPCTSTR p = sos.getString(idGroup);
  if(!p)
    return;

  const infoPrph* prphT = iP->getInfoOpenedBy();
  uint pgType = defPageType;
  if(prphT)
    GET_pO(prphT, pageType, pgType);

  int nObj = _ttoi(p);
  for(int i = 0; i < nObj; ++i) {
    manageObjId moi(0, idInit);
    uint id = moi.calcAndSetExtendId(i);
    p = sos.getString(id);
    if(!p)
      continue;


    int x = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      continue;
    int y = _ttoi(p);

    p = sos.getString(moi.getSecondExtendId());
    p = findNextParam(p, 2);
    if(p) {
      addVisibility(p, iP, obj, x, y);
      addMovement(sos, p, iP, obj, x, y);
      }
    }
}
//----------------------------------------------------------------------------
void managePages::checkPanel(infoPages* iP, setOfString& sos)
{
  checkSimple(iP, sos, ID_SIMPLE_PANEL, ID_INIT_SIMPLE_PANEL, infoPrph::ePanel);
}
//----------------------------------------------------------------------------
void managePages::checkSimpleTxt(infoPages* iP, setOfString& sos)
{
  checkSimple(iP, sos, ID_SIMPLE_TXT, ID_INIT_SIMPLE_TXT, infoPrph::eTxt);
}
//----------------------------------------------------------------------------
void managePages::checkEdit(infoPages* iP, setOfString& sos)
{
  checkVars(iP, sos, ID_VAR_EDI, ID_INIT_VAR_EDI, infoPrph::eEdit);
}
//----------------------------------------------------------------------------
void managePages::checkTxt(infoPages* iP, setOfString& sos)
{
  checkVarsAlt(iP, sos, ID_VAR_TXT, ID_INIT_VAR_TXT, infoPrph::eTxt, 35);
}
//----------------------------------------------------------------------------
void managePages::checkChoise(infoPages* iP, setOfString& sos)
{
  checkVarsAlt(iP, sos, ID_VAR_CHOOSE, ID_INIT_VAR_CHOOSE, infoPrph::eChoose, 15);
}
//----------------------------------------------------------------------------
infoPrph* managePages::addInfoPrphByVarsLed(infoPages* iP, setOfString& sos, uint ix, uint pgType)
{
  manageObjId moi(0, ID_INIT_VAR_LED);
  uint id = moi.calcAndSetExtendId(ix);
  LPCTSTR p = sos.getString(id);
  if(!p)
    return 0;

  int x = _ttoi(p);
  p = findNextParam(p, 1);
  if(!p)
    return 0;
  int y = _ttoi(p);

  p = sos.getString(moi.getSecondExtendId());

  LPCTSTR p2 = sos.getString(moi.getThirdExtendId());
  infoPrph* prph = addSingleObj(sos, p, p2, iP, infoPrph::eLed, x, y);
  return prph;
}
//----------------------------------------------------------------------------
void managePages::checkLed(infoPages* iP, setOfString& sos)
{
  LPCTSTR p = sos.getString(ID_VAR_LED);
  if(!p)
    return;

  const infoPrph* prphT = iP->getInfoOpenedBy();
  uint pgType = defPageType;
  if(prphT)
    GET_pO(prphT, pageType, pgType);

  int nObj = _ttoi(p);
  for(int i = 0; i < nObj; ++i)
    addInfoPrphByVarsLed(iP, sos, i, pgType);
}
//----------------------------------------------------------------------------
void managePages::checkDiam(infoPages* iP, setOfString& sos)
{
  LPCTSTR p = sos.getString(ID_VAR_DIAM);
  if(!p)
    return;
  const infoPrph* prphT = iP->getInfoOpenedBy();
  uint pgType = defPageType;
  if(prphT)
    GET_pO(prphT, pageType, pgType);
  uint nElem = _ttoi(p);
  for(uint j = 0; j < nElem; ++j) {
    manageObjId moi(0, ID_INIT_VAR_DIAM);
    uint id = moi.calcAndSetExtendId(j);
    p = sos.getString(id);
    if(!p)
      continue;
    int x = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      continue;
    int y = _ttoi(p);
    p = findNextParam(p, 5);

    for(uint i = 0; i < 3; ++i) {
      if(!p)
        break;
      int id = _ttoi(p);
      p = findNextParam(p, 1);
      manageObjId moi2(id);
      LPCTSTR pt = sos.getString(moi2.getFirstExtendId());
      LPCTSTR pt2 = sos.getString(moi2.getSecondExtendId());
      addSingleObj(sos, pt, pt2, iP, infoPrph::eDiam, x, y);
      }
    }
}
//----------------------------------------------------------------------------
infoPrph* managePages::addInfoPrphByCurve(infoPages* iP, setOfString& sos, uint ix, uint pgType)
{
  manageObjId moi(0, ID_INIT_VAR_CURVE);
  uint id = moi.calcAndSetExtendId(ix);
  LPCTSTR p = sos.getString(id);
  if(!p)
    return 0;

  int x = _ttoi(p);
  p = findNextParam(p, 1);
  if(!p)
    return 0;
  int y = _ttoi(p);
  p = findNextParam(p, 10);
  if(!p)
    return 0;

  int nPoint = _ttoi(p);

  p = sos.getString(moi.getFirstExtendId());

  LPCTSTR p2 = sos.getString(moi.getSecondExtendId());
  infoPrph* prph = addSingleObj(sos, p, p2, iP, infoPrph::eCurve, x, y);
  int addr = GET_pV(prph, Addr) + 1;
  for(int i = 1; i < nPoint; ++i, ++addr) {
    infoPrph* prph2 = new infoPrph;
    *prph2 = *prph;
    SET_pO(prph2, Addr, addr);
    addGlobalPrph(prph2);
    }
  return prph;
}
//----------------------------------------------------------------------------
void managePages::checkCurve(infoPages* iP, setOfString& sos)
{
  LPCTSTR p = sos.getString(ID_VAR_CURVE);
  if(!p)
    return;

  const infoPrph* prphT = iP->getInfoOpenedBy();
  uint pgType = defPageType;
  if(prphT)
    GET_pO(prphT, pageType, pgType);

  int nObj = _ttoi(p);
  for(int i = 0; i < nObj; ++i)
    addInfoPrphByCurve(iP, sos, i, pgType);

}
//----------------------------------------------------------------------------
void managePages::checkPlotXY(infoPages* iP, setOfString& sos)
{
  LPCTSTR p = sos.getString(ID_VAR_PLOT_XY);
  if(!p)
    return;

  const infoPrph* prphT = iP->getInfoOpenedBy();
  uint pgType = defPageType;
  if(prphT)
    GET_pO(prphT, pageType, pgType);
  uint nElem = _ttoi(p);
  for(uint j = 0; j < nElem; ++j) {
    manageObjId moi(0, ID_INIT_VAR_PLOT_XY);
    uint id = moi.calcAndSetExtendId(j);
    p = sos.getString(id);
    if(!p)
      continue;
    int x = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      continue;
    int y = _ttoi(p);
    p = findNextParam(p, 7);

    for(uint i = 0; i < 8; ++i) {
      if(!p)
        break;
      int id = _ttoi(p);
      p = findNextParam(p, 1);
      manageObjId moi2(id);
      LPCTSTR pt = sos.getString(moi2.getFirstExtendId());
      LPCTSTR pt2 = sos.getString(moi2.getSecondExtendId());
      addSingleObj(sos, pt, pt2, iP, infoPrph::ePlotXY, x, y);
      }
    LPCTSTR pt = sos.getString(moi.getFirstExtendId());
    LPCTSTR pt2 = sos.getString(moi.getSecondExtendId());
    addSingleObj(sos, pt, pt2, iP, infoPrph::ePlotXY, x, y);
    }
}
//----------------------------------------------------------------------------
void managePages::checkXMeter(infoPages* iP, setOfString& sos)
{
  LPCTSTR p = sos.getString(ID_VAR_X_METER);
  if(!p)
    return;
  const infoPrph* prphT = iP->getInfoOpenedBy();
  uint pgType = defPageType;
  if(prphT)
    GET_pO(prphT, pageType, pgType);
  uint nElem = _ttoi(p);
  for(uint j = 0; j < nElem; ++j) {
    manageObjId moi(0, ID_INIT_VAR_X_METER);
    uint id = moi.calcAndSetExtendId(j);
    p = sos.getString(id);
    if(!p)
      continue;
    int x = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      continue;
    int y = _ttoi(p);
    p = findNextParam(p, 10);

    for(uint i = 0; i < 2; ++i) {
      if(!p)
        break;
      int id = _ttoi(p);
      p = findNextParam(p, 1);
      manageObjId moi2(id);
      LPCTSTR pt = sos.getString(moi2.getFirstExtendId());
      LPCTSTR pt2 = sos.getString(moi2.getSecondExtendId());
      addSingleObj(sos, pt, pt2, iP, infoPrph::eMeter, x, y);
      }
    LPCTSTR pt = sos.getString(moi.getFirstExtendId());
    LPCTSTR pt2 = sos.getString(moi.getSecondExtendId());
    addSingleObj(sos, pt, pt2, iP, infoPrph::eMeter, x, y);
    }
}
//----------------------------------------------------------------------------
void managePages::checkSlider(infoPages* iP, setOfString& sos)
{
  LPCTSTR p = sos.getString(ID_VAR_SLIDER);
  if(!p)
    return;
  const infoPrph* prphT = iP->getInfoOpenedBy();
  uint pgType = defPageType;
  if(prphT)
    GET_pO(prphT, pageType, pgType);
  uint nElem = _ttoi(p);
  for(uint j = 0; j < nElem; ++j) {
    manageObjId moi(0, ID_INIT_VAR_SLIDER);
    uint id = moi.calcAndSetExtendId(j);
    p = sos.getString(id);
    if(!p)
      continue;
    int x = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      continue;
    int y = _ttoi(p);
    p = findNextParam(p, 10);

    for(uint i = 0; i < 2; ++i) {
      if(!p)
        break;
      int id = _ttoi(p);
      p = findNextParam(p, 1);
      manageObjId moi2(id);
      LPCTSTR pt = sos.getString(moi2.getFirstExtendId());
      LPCTSTR pt2 = sos.getString(moi2.getSecondExtendId());
      addSingleObj(sos, pt, pt2, iP, infoPrph::eSlider, x, y);
      }
    LPCTSTR pt = sos.getString(moi.getFirstExtendId());
    LPCTSTR pt2 = sos.getString(moi.getSecondExtendId());
    addSingleObj(sos, pt, pt2, iP, infoPrph::eSlider, x, y);
    }
}
//----------------------------------------------------------------------------
infoPrph* managePages::addInfoPrphByXScope(infoPages* iP, setOfString& sos, uint ix, uint pgType)
{
  manageObjId moi(0, ID_INIT_VAR_SCOPE);
  uint id = moi.calcAndSetExtendId(ix);

  LPCTSTR p = sos.getString(id);
  if(!p)
    return 0;

  int x = _ttoi(p);
  p = findNextParam(p, 1);
  if(!p)
    return 0;
  int y = _ttoi(p);
  p = findNextParam(p, 6);
  if(!p)
    return 0;

  LPCTSTR p1 = sos.getString(moi.getFirstExtendId());
  LPCTSTR p2 = sos.getString(moi.getSecondExtendId());
  infoPrph* prph = addSingleObj(sos, p1, p2, iP, infoPrph::eXScope, x, y);

  p2 = findNextParam(p, 5);
  int addrReady = 0;
  if(p2)
    addrReady = _ttoi(p2);

  p2 = findNextParam(p2);
  if(p2 && 1 == _ttoi(p2)) {
    if(addrReady) {
      infoPrph* prph2 = new infoPrph;
      *prph2 = *prph;
      SET_pO(prph2, Prph, 1);
      SET_pO(prph2, Addr, addrReady);
      SET_pO(prph2, dataType, 13);
      addGlobalPrph(prph2);
      }
    }
  else {
    infoPrph* prph2 = new infoPrph;
    *prph2 = *prph;
    SET_pO(prph2, Addr, addrReady);
    SET_pO(prph2, dataType, 4);
    addGlobalPrph(prph2);
    }
  for(uint i = 0; i < 5; ++i) {
    if(!p)
      break;
    int id = _ttoi(p);
    p = findNextParam(p, 1);
    manageObjId moi2(id);
    LPCTSTR pt = sos.getString(moi2.getFirstExtendId());
    LPCTSTR pt2 = sos.getString(moi2.getSecondExtendId());
    addSingleObj(sos, pt, pt2, iP, infoPrph::eXScope, x, y);
    }
  p = findNextParam(p, 2);
  if(p) {
    int typeX = _ttoi(p);
    if(typeX & 1) {
      p = findNextParam(p, 1);
      for(uint i = 0; i < 2; ++i) {
        if(!p)
          break;
        int id = _ttoi(p);
        p = findNextParam(p, 1);
        manageObjId moi2(id);
        LPCTSTR pt = sos.getString(moi2.getFirstExtendId());
        LPCTSTR pt2 = sos.getString(moi2.getSecondExtendId());
        addSingleObj(sos, pt, pt2, iP, infoPrph::eXScope, x, y);
        }
      }
    }
  return prph;
}
//----------------------------------------------------------------------------
void managePages::checkXScope(infoPages* iP, setOfString& sos)
{
  LPCTSTR p = sos.getString(ID_VAR_SCOPE);
  if(!p)
    return;

  const infoPrph* prphT = iP->getInfoOpenedBy();
  uint pgType = defPageType;
  if(prphT)
    GET_pO(prphT, pageType, pgType);

  int nObj = _ttoi(p);
  for(int i = 0; i < nObj; ++i)
    addInfoPrphByXScope(iP, sos, i, pgType);
}
//----------------------------------------------------------------------------
static bool isReal(LPCTSTR p)
{
  int len = _tcslen(p);
  for(int i = 0; i < len; ++i)
    if(_T('.') == p[i])
      return true;
    else if(_T(',') == p[i])
      return false;
  return false;
}
//----------------------------------------------------------------------------
infoPrph* managePages::addInfoPrphBySpin(infoPages* iP, setOfString& sos, uint ix, uint pgType)
{
  manageObjId moi(0, ID_INIT_VAR_SPIN);
  uint id = moi.calcAndSetExtendId(ix);
  LPCTSTR p = sos.getString(id);
  if(!p)
    return 0;

  int x = _ttoi(p);
  p = findNextParam(p);
  if(!p)
    return 0;
  int y = _ttoi(p);
  p = findNextParam(p);
  if(!p)
    return 0;
  int idSpin = _ttoi(p);

  p = findNextParam(p);
  if(!p)
    return 0;
  int addrMem = _ttoi(p);
  p = findNextParam(p, 2);
  if(!p)
    return 0;

  bool useReal = isReal(p);
  if(!useReal) {
    p = findNextParam(p);
    if(p)
      useReal = isReal(p);
    }

  LPCTSTR p1 = sos.getString(moi.getFirstExtendId());
  LPCTSTR p2 = sos.getString(moi.getSecondExtendId());
  infoPrph* prph = addSingleObj(sos, p1, p2, iP, infoPrph::eSpin, x, y);

  infoPrph* prph2 = new infoPrph;
  *prph2 = *prph;
  SET_pO(prph2, Prph, 1);
  SET_pO(prph2, Addr, addrMem);
  if(useReal)
    SET_pO(prph2, dataType, 5);
  else
    SET_pO(prph2, dataType, 13);
  addGlobalPrph(prph2);

  return prph;
}
//----------------------------------------------------------------------------
void managePages::checkSpin(infoPages* iP, setOfString& sos)
{
  LPCTSTR p = sos.getString(ID_VAR_SPIN);
  if(!p)
    return;

  const infoPrph* prphT = iP->getInfoOpenedBy();
  uint pgType = defPageType;
  if(prphT)
    GET_pO(prphT, pageType, pgType);

  int nObj = _ttoi(p);
  for(int i = 0; i < nObj; ++i)
    addInfoPrphBySpin(iP, sos, i, pgType);
}
//----------------------------------------------------------------------------
void managePages::checkObjAlarm(infoPages* iP, setOfString& sos)
{
  LPCTSTR p = sos.getString(ID_VAR_ALARM);
  if(!p)
    return;

  const infoPrph* prphT = iP->getInfoOpenedBy();
  uint pgType = defPageType;
  if(prphT)
    GET_pO(prphT, pageType, pgType);

  int nObj = _ttoi(p);
  for(int i = 0; i < nObj; ++i)
    addInfoPrphByObjAlarm(iP, sos, i, pgType);
}
//----------------------------------------------------------------------------
infoPrph* managePages::addInfoPrphByObjAlarm(infoPages* iP, setOfString& sos, uint ix, uint pgType)
{
  manageObjId moi(0, ID_INIT_VAR_ALARM);
  uint id = moi.calcAndSetExtendId(ix);
  LPCTSTR p = sos.getString(id);
  if(!p)
    return 0;
  int x = _ttoi(p);
  p = findNextParam(p, 1);
  if(!p)
    return 0;
  int y = _ttoi(p);
  p = findNextParam(p, 6);
  if(!p)
    return 0;

  LPCTSTR p1 = sos.getString(moi.getFirstExtendId());
  LPCTSTR p2 = sos.getString(moi.getSecondExtendId());
  infoPrph* prph = addSingleObj(sos, p1, p2, iP, infoPrph::eObjAlarm, x, y);

  int idFilter = _ttoi(p);
  manageObjId moi2(idFilter);
  LPCTSTR pt = sos.getString(moi2.getFirstExtendId());
  LPCTSTR pt2 = sos.getString(moi2.getSecondExtendId());
  infoPrph* prph2 = addSingleObj(sos, pt, pt2, iP, infoPrph::eObjAlarm, x, y);
  if(use64bit4Alarmfilter) {
    prph2 = new infoPrph(*prph2);
    prph2->setAddr(prph2->getAddr() + 1);
    addGlobalPrph(prph2);
    }

  p = findNextParam(p, 20);
  if(p) {
    int idFilterGrp = _ttoi(p);
    manageObjId moi2(idFilterGrp);
    pt = sos.getString(moi2.getFirstExtendId());
    pt2 = sos.getString(moi2.getSecondExtendId());
    addSingleObj(sos, pt, pt2, iP, infoPrph::eObjAlarm, x, y);
    }
  p = findNextParam(p);
  if(p) {
    int idFilterText = _ttoi(p);
    manageObjId moi2(idFilterText);
    pt = sos.getString(moi2.getFirstExtendId());
    pt2 = sos.getString(moi2.getSecondExtendId());
    addSingleObj(sos, pt, pt2, iP, infoPrph::eObjAlarm, x, y);
    }

  return prph;
}
//----------------------------------------------------------------------------
void managePages::checkObjTnlInfo(infoPages* iP, setOfString& sos)
{
  LPCTSTR p = sos.getString(ID_VAR_TABLE_INFO);
  if(!p)
    return;

  const infoPrph* prphT = iP->getInfoOpenedBy();
  uint pgType = defPageType;
  if(prphT)
    GET_pO(prphT, pageType, pgType);

  int nObj = _ttoi(p);
  for(int i = 0; i < nObj; ++i)
    addInfoPrphByObjTblInfo(iP, sos, i, pgType);
}
//----------------------------------------------------------------------------
infoPrph* managePages::addInfoPrphByObjTblInfo(infoPages* iP, setOfString& sos, uint ix, uint pgType)
{
  manageObjId moi(0, ID_INIT_VAR_TABLE_INFO);
  uint id = moi.calcAndSetExtendId(ix);
  LPCTSTR p = sos.getString(id);
  if(!p)
    return 0;
  int x = _ttoi(p);
  p = findNextParam(p, 1);
  if(!p)
    return 0;
  int y = _ttoi(p);

  LPCTSTR p1 = sos.getString(moi.getFirstExtendId());
  LPCTSTR p2 = sos.getString(moi.getSecondExtendId());
  infoPrph* prph = addSingleObj(sos, p1, p2, iP, infoPrph::eObjTblInfo, x, y);

  return prph;
}
//----------------------------------------------------------------------------
void managePages::checkObjTrend(infoPages* iP, setOfString& sos)
{
  LPCTSTR p = sos.getString(ID_VAR_TREND);
  if(!p)
    return;

  const infoPrph* prphT = iP->getInfoOpenedBy();
  uint pgType = defPageType;
  if(prphT)
    GET_pO(prphT, pageType, pgType);

  int nObj = _ttoi(p);
  for(int i = 0; i < nObj; ++i)
    addInfoPrphByObjTrend(iP, sos, i, pgType);
}
//----------------------------------------------------------------------------
uint managePages::getNumFieldFromTrendByName(LPCTSTR trendName)
{
  LPCTSTR p = sosStdMsg.getString(INIT_TREND_DATA);
  while(p) {
    uint code = _ttol(p);
    if(!code)
      break;
    LPCTSTR p2 = sosStdMsg.getString(code);
    LPCTSTR p3 = findNextParamTrim(p2, 6);
    if(p3 && !_tcsicmp(p3, trendName)) {
      p2 = findNextParam(p2);
      if(!p2)
        return 0;
      uint dataCod = _ttoi(p2);
      p2 = sosStdMsg.getString(dataCod);
      if(!p2)
        return 0;
      return _ttoi(p2);
      }
    p = findNextParamTrim(p);
    }
  return 0;
}
//----------------------------------------------------------------------------
infoPrph* managePages::addInfoPrphByObjTrend(infoPages* iP, setOfString& sos, uint ix, uint pgType)
{
  manageObjId moi(0, ID_INIT_VAR_TREND);
  uint id = moi.calcAndSetExtendId(ix);
  LPCTSTR p = sos.getString(id);
  if(!p)
    return 0;

  int x = _ttoi(p);
  p = findNextParam(p, 1);
  if(!p)
    return 0;
  int y = _ttoi(p);
  LPCTSTR pName = findNextParam(p, 24);
  if(!pName)
    return 0;

  uint nVar = getNumFieldFromTrendByName(pName);

  LPCTSTR p1 = sos.getString(moi.getFirstExtendId());
  LPCTSTR p2 = sos.getString(moi.getSecondExtendId());
  infoPrph* prph = addSingleObj(sos, p1, p2, iP, infoPrph::eObjTrend, x, y, nVar * 2);

  p = findNextParamTrim(p, 20);
  for(uint i = 0; i < 3; ++i) {
    int idVar = _ttoi(p);
    manageObjId moi2(idVar);
    LPCTSTR pt = sos.getString(moi2.getFirstExtendId());
    LPCTSTR pt2 = sos.getString(moi2.getSecondExtendId());
    addSingleObj(sos, pt, pt2, iP, infoPrph::eObjTrend, x, y);
    p = findNextParamTrim(p);
    }

  int idVar = _ttoi(p);
  manageObjId moi2(idVar);
  LPCTSTR pt = sos.getString(moi2.getFirstExtendId());
  LPCTSTR pt2 = sos.getString(moi2.getSecondExtendId());
  infoPrph* prphDate = addSingleObj(sos, pt, pt2, iP, infoPrph::eObjTrend, x, y);
  TCHAR tmp[64];
  wsprintf(tmp, _T("%d,%d,%d,%d,%d"), prphDate->getPrph(), prphDate->getAddr() + 2, 10, 0, 0);
  addSingleObj(sos, tmp, pt2, iP, infoPrph::eObjTrend, x, y);
  wsprintf(tmp, _T("%d,%d,%d,%d,%d"), prphDate->getPrph(), prphDate->getAddr() + 4, 10, 0, 0);
  addSingleObj(sos, tmp, pt2, iP, infoPrph::eObjTrend, x, y);

  return prph;
}
//----------------------------------------------------------------------------
void managePages::checkCam(infoPages* iP, setOfString& sos)
{
  LPCTSTR p = sos.getString(ID_VAR_CAM);
  if(!p)
    return;
  const infoPrph* prphT = iP->getInfoOpenedBy();
  uint pgType = defPageType;
  dualOffset dOffs;
  dOffs.offset = 0;
  if(prphT) {
    GET_pO(prphT, pageType, pgType);
    GET_pO(prphT, OffsLink, dOffs.offset);
    }
  uint nElem = _ttoi(p);
  for(uint j = 0; j < nElem; ++j) {
    p = sos.getString(ID_INIT_CAM + j);
    if(!p)
      continue;
    int x = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      continue;
    int y = _ttoi(p);
    p = findNextParam(p, 3);
    int pr = 0;
    int addr = 0;
    _stscanf(p, _T("%d,%d"),  &pr, &addr);
    if(pr) {
      for(uint i = 0; i < 5; ++i, ++addr) {
        infoPrph* prph = new infoPrph;

        // fill prph
        SET_pO(prph, Page, iP->getPageCurr());
        SET_pO(prph, pageType, pgType);

        SET_pO(prph, X, x);
        SET_pO(prph, Y, y);

        SET_pO(prph, Prph, pr);
        if(PRF_MEMORY == pr && addr <= 1)
          SET_pO(prph, Addr, addr);
        else {
          SET_pO(prph, Addr, addr + dOffs.offsAddr);
          SET_pO(prph, OffsLink, dOffs.offset);
          }
        SET_pO(prph, dataType, 4);
        SET_pO(prph, nBit, 0);
        SET_pO(prph, Offs, 0);

        SET_pO(prph, objType, infoPrph::eCam);
        addGlobalPrph(prph);
        }
      }
    }
}
//----------------------------------------------------------------------------
void managePages::checkBtn(infoPages* iP, setOfString& sos, uint level)
{
  LPCTSTR p = sos.getString(ID_VAR_BTN);
  if(!p)
    return;

  const infoPrph* prphT = iP->getInfoOpenedBy();
  uint pgType = defPageType;
  if(prphT)
    GET_pO(prphT, pageType, pgType);

  int nObj = _ttoi(p);
  for(int i = 0; i < nObj; ++i) {
    manageObjId moi(0, ID_INIT_VAR_BTN);
    uint id = moi.calcAndSetExtendId(i);
    p = sos.getString(id);
    p = findNextParam(p, 5);
    if(!p)
      continue;
    uint typeCommand = _ttoi(p);

    switch(typeCommand) {
      case 0:
        checkActionBtn(iP, sos, level, ID_INIT_VAR_BTN, i);
        break;
      case 6:
        checkModelessByBtn(iP, sos, level, ID_INIT_VAR_BTN, i, pgType);
        break;
      default:
        addInfoPrphByVars(iP, sos, ID_INIT_VAR_BTN, i, infoPrph::eBtn, pgType);
        break;
      }
    }
}
//----------------------------------------------------------------------------
void managePages::checkModelessByBtn(infoPages* iP, setOfString& sos, uint level, uint idBase, uint ix, uint pgType)
{
  infoPrph* prph = addInfoPrphByVars(iP, sos, idBase, ix, infoPrph::eBtn, pgType);
  if(!prph)
    return;
  SET_pO(prph, action, infoPrph::eOpenModeless);

  manageObjId moi(0, idBase);
  moi.calcAndSetExtendId(ix);

  LPCTSTR p = sos.getString(moi.getThirdExtendId());
  p = findNextParam(p, 1);
  if(!p)
    return;
  if(_T('@') == *p)
    ++p;
  infoPages* child = loadPage(p, iP, prph, level + 1);
  if(child) {
    SET_pO(prph, openPage, child->getPageCurr());
    iP->addChild(child);
    }
}
//----------------------------------------------------------------------------
void managePages::addFromBtn(LPCTSTR name, infoPages* iP, infoPrph::eAction action, uint level, int x, int y)
{
  addFromFunct(name, iP, action, level, x, y, infoPrph::eBtn);
}
//----------------------------------------------------------------------------
void managePages::checkActionBtn(infoPages* iP, setOfString& sos, uint level, uint idBase, uint ix)
{
  manageObjId moi(0, idBase);
  uint idInit = moi.calcAndSetExtendId(ix);
  LPCTSTR p = sos.getString(idInit);
  int x = _ttoi(p);
  p = findNextParam(p, 1);
  if(!p)
    return;
  int y = _ttoi(p);

  p = sos.getString(moi.getFirstExtendId());
  if(!p || _ttoi(p))
    return;
  p = findNextParam(p, 1);
  if(!p)
    return;
  uint id = _ttoi(p);
  if(id >= 1 && id <= 12)
    id += 13;
  else if(id >= 13 && id <= 24)
    id += ID_SHIFT_F1_ACT - 13;
  else if(id >= 25 && id <= 36)
    id += ID_CTRL_F1_ACT - 25;

  p = sos.getString(id);
  if(!p)
    return;
  id = _ttoi(p);

  infoPrph::eAction act = infoPrph::noAct;
  p = checkAction(id, act, p);
  if(p)
    addFromBtn(p, iP, act, level, x, y);
}
//----------------------------------------------------------------------------
// per ricette (anche con listbox)
#define ID_RECIPE_BIT_ACTIVATE  1000006
#define ID_RECIPE_BIT_ENABLE    1000007
#define ID_RECIPE_INIT_ON_MEM   1001001
#define ID_RECIPE_SEND_PRPH     1001002
#define ID_RECIPE_SEND_ADDR     1001003
#define ID_RECIPE_SEND_TYPE     1001004
#define ID_RECIPE_SEND_DIM      1001005

#define ID_RECIPE_COPY_PRPH     1001101
#define ID_RECIPE_COPY_ADDR     1001102
#define ID_RECIPE_COPY_TYPE     1001103
#define ID_RECIPE_COPY_DIM      1001104
//----------------------------------------------------------------------------
#define ID_RECIPE_BIT_TO_FUNCT  1001155
//----------------------------------------------------------------------------
void managePages::checkRecipe(infoPages* iP, setOfString& sos)
{
  const infoPrph* prphT = iP->getInfoOpenedBy();
  uint pgType = defPageType;
  dualOffset dOffs;
  dOffs.offset = 0;
  if(prphT) {
    GET_pO(prphT, pageType, pgType);
    GET_pO(prphT, OffsLink, dOffs.offset);
    }

  uint ids[] = { ID_RECIPE_BIT_ACTIVATE, ID_RECIPE_BIT_ENABLE };
  for(uint i = 0; i < SIZE_A(ids); ++i) {
    LPCTSTR p = sos.getString(ids[i]);
    if(p) {
      int pr = 0;
      int addr = 0;
      int offs = 0;
      _stscanf(p, _T("%d,%d,%d"), &addr, &offs,  &pr);

      infoPrph* prph = new infoPrph;

      // fill prph
      SET_pO(prph, Page, iP->getPageCurr());
      SET_pO(prph, pageType, pgType);

      SET_pO(prph, X, COORD_BY_FUNCT);
      SET_pO(prph, Y, COORD_BY_RECIPE_PAGE);

      SET_pO(prph, Prph, pr);
      if(PRF_MEMORY == pr && addr <= 1)
        SET_pO(prph, Addr, addr);
      else {
        if(dOffs.offset)
          calcAddrAndOffs(addr, offs, dOffs.offsAddr, dOffs.offsBit, GET_NBIT(4));
        SET_pO(prph, Addr, addr);
        SET_pO(prph, OffsLink, dOffs.offset);
        }
      SET_pO(prph, dataType, 4);
      SET_pO(prph, nBit, 1);
      SET_pO(prph, Offs, offs);
      addGlobalPrph(prph);
      }
    }
  LPCTSTR p = sos.getString(ID_RECIPE_BIT_TO_FUNCT);
  while(p) {
    int pr = 0;
    int addr = 0;
    int type = 0;
    _stscanf(p, _T("%d,%d,%d"), &pr, &addr, &type);
    if(PRF_MEMORY == pr && addr <= 1)
      break;
    for(uint i = 0; i < 2; ++i) {
      infoPrph* prph = new infoPrph;

      // fill prph
      SET_pO(prph, Page, iP->getPageCurr());
      SET_pO(prph, pageType, pgType);

      SET_pO(prph, X, COORD_BY_FUNCT);
      SET_pO(prph, Y, COORD_BY_RECIPE_PAGE);

      SET_pO(prph, Prph, pr);
      SET_pO(prph, Addr, addr + dOffs.offsAddr + i);
      SET_pO(prph, OffsLink, dOffs.offset);
      SET_pO(prph, dataType, type);
      addGlobalPrph(prph);
      }
    break;
    }

  p = sos.getString(ID_RECIPE_INIT_ON_MEM);
  if(!p)
    return;
  int initAddrMem;
  int indT1;
  int len;
  int indT2;
  int pswLevel = 0;
  // nuova specifica per periferica memoria
  int idMemPrph = 1;
  _stscanf(p, _T("%d,%d,%d,%d,%d,%d"), &initAddrMem, &indT1, &len, &indT2, &pswLevel, &idMemPrph);

  int len4 = ROUND_DWORD_LEN(len);
  for(int i = 0; i < len4; ++i, ++indT1, ++indT2) {
    infoPrph* prph = new infoPrph;
    SET_pO(prph, Page, iP->getPageCurr());
    SET_pO(prph, pageType, pgType);

    SET_pO(prph, X, COORD_BY_FUNCT);
    SET_pO(prph, Y, COORD_BY_RECIPE_PAGE);

    SET_pO(prph, Prph, idMemPrph);
    if(PRF_MEMORY == idMemPrph && indT1 <= 1)
      SET_pO(prph, Addr, indT1);
    else {
      SET_pO(prph, Addr, indT1 + dOffs.offsAddr);
      SET_pO(prph, OffsLink, dOffs.offset);
      }
    SET_pO(prph, dataType, 14);
    addGlobalPrph(prph);

    infoPrph* prph2 = new infoPrph;
    *prph2 = *prph;
    SET_pO(prph2, Addr, indT2);
    addGlobalPrph(prph2);
    }

  bool noCopy = true;
  do {
    LPCTSTR p1 = sos.getString(ID_RECIPE_SEND_PRPH);
    LPCTSTR p2 = sos.getString(ID_RECIPE_COPY_PRPH);
    if(!p1 || !p2 || _tcscmp(p1, p2))
      break;
    p1 = sos.getString(ID_RECIPE_SEND_ADDR);
    p2 = sos.getString(ID_RECIPE_COPY_ADDR);
    if(!p1 || !p2 || _tcscmp(p1, p2))
      break;
    noCopy = false;
    } while(false);

  struct idRecipeBlock
  {
    uint idPrph;
    uint idAddr;
    uint idType;
    uint idDim;
  } rb[] =
  {
    {
      ID_RECIPE_SEND_PRPH,
      ID_RECIPE_SEND_ADDR,
      ID_RECIPE_SEND_TYPE,
      ID_RECIPE_SEND_DIM,
    },
    {
      ID_RECIPE_COPY_PRPH,
      ID_RECIPE_COPY_ADDR,
      ID_RECIPE_COPY_TYPE,
      ID_RECIPE_COPY_DIM,
    }
  };

  uint nBlock = 2 - noCopy;
  for(uint j = 0; j < nBlock; ++j) {
    LPCTSTR pPrph = sos.getString(rb[j].idPrph);
    LPCTSTR pAddr = sos.getString(rb[j].idAddr);
    LPCTSTR pType = sos.getString(rb[j].idType);
    LPCTSTR pDim = sos.getString(rb[j].idDim);

    while(pPrph && pAddr && pType) {
      int idPrph = _ttoi(pPrph);
      pPrph = findNextParam(pPrph, 1);
      int addr = _ttoi(pAddr);
      pAddr = findNextParam(pAddr, 1);
      int type = _ttoi(pType);
      pType = findNextParam(pType, 1);
      int dim = 4;
      if(pDim) {
        dim = _ttoi(pDim);
        pDim = findNextParam(pDim, 1);
        }
      if(!idPrph)
        continue;
      int nRepeat = 1;
      if(14 == type)
        nRepeat = ROUND_DWORD_LEN(dim);
      for(int i = 0; i < nRepeat; ++i) {
        infoPrph* prph = new infoPrph;
        SET_pO(prph, Page, iP->getPageCurr());
        SET_pO(prph, pageType, pgType);

        SET_pO(prph, X, COORD_BY_FUNCT);
        SET_pO(prph, Y, COORD_BY_RECIPE_PAGE);

        SET_pO(prph, Prph, idPrph);
        if(PRF_MEMORY == idPrph && addr + i <= 1)
          SET_pO(prph, Addr, addr + i);
        else {
          SET_pO(prph, Addr, addr + i + dOffs.offsAddr);
          SET_pO(prph, OffsLink, dOffs.offset);
          }
        SET_pO(prph, dataType, type);
        addGlobalPrph(prph);
        }
      }
    }
}
//----------------------------------------------------------------------------
// per ricette su righe
#define ID_RECIPE_ROW_N_ROW           1000002
#define ID_RECIPE_ROW_N_COL           1000003
#define ID_RECIPE_ROW_FIRST_EDIT      1000004
#define ID_RECIPE_ROW_PRPH_ADDR       1000005
#define ID_RECIPE_ROW_INIT_ON_MEM     1000006
#define ID_RECIPE_ROW_USE_FIRST_EDIT  1000007
#define ID_RECIPE_ROW_SAVE_ACTIVE     1000010
#define ID_RECIPE_ROW_BIT_ENABLE      1000011
#define ID_RECIPE_ROW_BIT_ACTIVATE    1000012
#define ID_INDICATOR_BMP              1000015

#define MAX_BUFF_TYPE 30
#define DIM_NAME_RECIPE_ROW ROUND_DWORD_LEN(MAX_BUFF_TYPE)
//----------------------------------------------------------------------------
void managePages::checkRecipeRow(infoPages* iP, setOfString& sos)
{
  const infoPrph* prphT = iP->getInfoOpenedBy();
  uint pgType = defPageType;
  dualOffset dOffs;
  dOffs.offset = 0;
  if(prphT) {
    GET_pO(prphT, pageType, pgType);
    GET_pO(prphT, OffsLink, dOffs.offset);
    }

  LPCTSTR pAddr = sos.getString(ID_RECIPE_ROW_PRPH_ADDR);

  while(pAddr) {
    int addr = _ttoi(pAddr);
    pAddr = findNextParam(pAddr, 1);
    if(addr <= 0)
      continue;
    infoPrph* prph = new infoPrph;
    SET_pO(prph, Page, iP->getPageCurr());
    SET_pO(prph, pageType, pgType);

    SET_pO(prph, X, COORD_BY_FUNCT);
    SET_pO(prph, Y, COORD_BY_RECIPE_ROW_PAGE);

    SET_pO(prph, Prph, realPlc);
    if(PRF_MEMORY == realPlc && addr <= 1)
      SET_pO(prph, Addr, addr);
    else {
      SET_pO(prph, Addr, addr + dOffs.offsAddr);
      SET_pO(prph, OffsLink, dOffs.offset);
      }
    SET_pO(prph, dataType, 14 + 1);
    addGlobalPrph(prph);
    }
  LPCTSTR p = sos.getString(ID_RECIPE_ROW_USE_FIRST_EDIT);
  bool useFirstEdit = p && _ttoi(p);
  if(useFirstEdit) {
    p = sos.getString(ID_RECIPE_ROW_SAVE_ACTIVE);
    if(p) {
      int pr = 0;
      int addr = 0;
      _stscanf(p, _T("%d,%d"), &pr, &addr);

      for(uint i = 0; i < DIM_NAME_RECIPE_ROW; ++i) {
        infoPrph* prph = new infoPrph;
        SET_pO(prph, Page, iP->getPageCurr());
        SET_pO(prph, pageType, pgType);

        SET_pO(prph, X, COORD_BY_FUNCT);
        SET_pO(prph, Y, COORD_BY_RECIPE_ROW_PAGE);

        SET_pO(prph, Prph, pr);
        if(PRF_MEMORY == pr && addr + i <= 1)
          SET_pO(prph, Addr, addr + i);
        else {
          SET_pO(prph, Addr, addr + i + dOffs.offsAddr);
          SET_pO(prph, OffsLink, dOffs.offset);
          }
        SET_pO(prph, dataType, 14 + 1);
        addGlobalPrph(prph);
        }
      }
    }
  uint ids[] = { ID_RECIPE_ROW_BIT_ENABLE, ID_RECIPE_ROW_BIT_ACTIVATE };
  for(uint i = 0; i < SIZE_A(ids); ++i) {
    LPCTSTR p = sos.getString(ids[i]);
    if(p) {
      int idPrph = 0;
      int addr = 0;
      int type = 0;
      int nbit = 0;
      int offs = 0;
      _stscanf(p, _T("%d,%d,%d,%d,%d"), &idPrph, &addr, &type, &nbit, &offs);

      infoPrph* prph = new infoPrph;

      // fill prph
      SET_pO(prph, Page, iP->getPageCurr());
      SET_pO(prph, pageType, pgType);

      SET_pO(prph, X, COORD_BY_FUNCT);
      SET_pO(prph, Y, COORD_BY_RECIPE_ROW_PAGE);

      SET_pO(prph, Prph, idPrph);
      if(PRF_MEMORY == idPrph && addr <= 1)
        SET_pO(prph, Addr, addr);
      else {
        if(dOffs.offset)
          calcAddrAndOffs(addr, offs, dOffs.offsAddr, dOffs.offsBit, GET_NBIT(type));
        SET_pO(prph, Addr, addr);
        SET_pO(prph, OffsLink, dOffs.offset);
        }
      SET_pO(prph, dataType, type);
      SET_pO(prph, nBit, nbit);
      SET_pO(prph, Offs, offs);
      addGlobalPrph(prph);
      }
    }
  p = sos.getString(ID_INDICATOR_BMP);
  if(p)
    p = skipFirstFolder(p);
  else { // presume che ci sia
    if(P_File::P_exist(_T("..\\image\\indicator.bmp")))
      p = _T("indicator.bmp");
    }
  if(p)
    soImages.addImage(p, iP->getPageCurr());

  p = sos.getString(ID_RECIPE_ROW_INIT_ON_MEM);
  if(!p)
    return;
  uint addrInit = _ttoi(p);
  p = sos.getString(ID_RECIPE_ROW_N_ROW);
  uint nrow = p ? _ttoi(p) : 0;
  p = sos.getString(ID_RECIPE_ROW_N_COL);
  uint ncol = p ? _ttoi(p) : 0;

  uint idPrph = 0;
  do {
    p = sos.getString(ID_RECIPE_ROW_FIRST_EDIT);
    uint fe = p ? _ttoi(p) : 4001;
    p = sos.getString(fe + 300);
    idPrph = p ? _ttoi(p) : 0;
    if(!idPrph) {
      p = sos.getString(fe + 300 + nrow);
      idPrph = p ? _ttoi(p) : 0;
      }
    } while(false);
  if(!idPrph)
    return;

  do {
    infoPrph* prph = new infoPrph;

    // fill prph
    SET_pO(prph, Page, iP->getPageCurr());
    SET_pO(prph, pageType, pgType);

    SET_pO(prph, X, COORD_BY_FUNCT);
    SET_pO(prph, Y, COORD_BY_RECIPE_ROW_PAGE);

    SET_pO(prph, Prph, idPrph);
    if(PRF_MEMORY == idPrph && addrInit <= 1)
      SET_pO(prph, Addr, addrInit);
    else {
      SET_pO(prph, Addr, addrInit + dOffs.offsAddr);
      SET_pO(prph, OffsLink, dOffs.offset);
      }
    SET_pO(prph, dataType, 4);
    SET_pO(prph, nBit, 0);
    SET_pO(prph, Offs, 0);
    addGlobalPrph(prph);
    } while(false);


  int addr = addrInit + 1;
  for(uint j = 0; j < ncol; ++j, addr += 40) {
    if(j || !useFirstEdit) {
      int addr2 = addr + nrow;
      for(uint i = nrow; i < 40; ++i, ++addr2) {
        infoPrph* prph = new infoPrph;
        SET_pO(prph, Page, iP->getPageCurr());
        SET_pO(prph, pageType, pgType);

        SET_pO(prph, X, COORD_BY_FUNCT);
        SET_pO(prph, Y, COORD_BY_RECIPE_ROW_PAGE);

        SET_pO(prph, Prph, idPrph);
        if(PRF_MEMORY == idPrph && addr2 <= 1)
          SET_pO(prph, Addr, addr2);
        else {
          SET_pO(prph, Addr, addrInit + dOffs.offsAddr);
          SET_pO(prph, OffsLink, dOffs.offset);
          }
        SET_pO(prph, dataType, 5);
        SET_pO(prph, nBit, 0);
        SET_pO(prph, Offs, 0);
        SET_pO(prph, objType, infoPrph::eEdit);
        addGlobalPrph(prph);
        }
      }
    }

}
//----------------------------------------------------------------------------
void managePages::checkDefaultPages(infoPages* iP, setOfString& sos)
{
  enum defaultPageType {
      P_STANDARD,
      P_RECIPE,
      P_RECIPE_LB,
      P_RECIPE_ROW,
      P_GRAPH_TREND,
      P_MAINT
      };
  LPCTSTR p = sos.getString(ID_CODE_PAGE_TYPE);
  if(!p)
    return;
  int code = _ttoi(p);
  switch(code) {
    case P_STANDARD:
    case P_GRAPH_TREND:
    case P_MAINT:
      // non ci sono variabili usate nei codici personalizzati
      return;

    case P_RECIPE_LB:
    case P_RECIPE:
      checkRecipe(iP, sos);
      break;
    case P_RECIPE_ROW:
      checkRecipeRow(iP, sos);
      break;
    }
}
//----------------------------------------------------------------------------
void managePages::checkAllVars(infoPages* iP, setOfString& sos, uint level)
{
  checkBmp(iP, sos);
  checkBtn(iP, sos, level);
  checkEdit(iP, sos);
  checkSimpleTxt(iP, sos);
  checkTxt(iP, sos);
  checkBarGraph(iP, sos);
  checkPanel(iP, sos);
  checkChoise(iP, sos);
  checkLed(iP, sos);
  checkDiam(iP, sos);
  checkCurve(iP, sos);
  checkXMeter(iP, sos);
  checkCam(iP, sos);
  checkPlotXY(iP, sos);
  checkXScope(iP, sos);
  checkObjAlarm(iP, sos);
  checkObjTrend(iP, sos);
  checkSpin(iP, sos);
  checkSlider(iP, sos);
  checkObjTnlInfo(iP, sos);

  checkDefaultPages(iP, sos);
}
//----------------------------------------------------------------------------
void managePages::checkBitmap(infoPages* iP, setOfString& sos, LPCTSTR page)
{
  LPCTSTR p = sos.getString(ID_BMP);
  if(p) {
    uint nElem = _ttoi(p);
    for(uint i = 0; i < nElem; ++i) {
      manageObjId moi(0, ID_INIT_BMP);
      uint id = moi.calcAndSetExtendId(i);
      p = sos.getString(id);
      if(p) {
        LPCTSTR p2 = findNextParam(p, 5);
        p2 = skipFirstFolder(p2);
        soImages.addImage(p2, page);
        p2 = sos.getString(moi.getFirstExtendId());
        p2 = findNextParamTrim(p2, 2);
        if(p2) {
          int x = _ttoi(p);
          int y = 0;
          p = findNextParamTrim(p);
          if(p)
            y =  _ttoi(p);
          addVisibility(p2, iP, infoPrph::eBmp, x, y);
          }
        }
      }
    }
  #define MAX_BMP_4_BTN 250
  for(uint i = 0; i < MAX_BMP_4_BTN; ++i) {
    p = sos.getString(ID_INIT_BMP_4_BTN + i);
    if(!p)
      break;
    p = skipFirstFolder(p);
    soImages.addImage(p, page);
    }
}
//----------------------------------------------------------------------------
// la prima chiamata avviene con infoPages* root = loadPage("page1.txt", 0, 0, 0);
//----------------------------------------------------------------------------
infoPages* managePages::loadPage(LPCTSTR filename, infoPages* iP_Parent, infoPrph* openedBy, uint level)
{
  infoPages* iP = 0;

  do {
    wrapText* t = Dlg->getFree();
    _tcscpy(t->txt, filename);
    Dlg->setTextInfo(t);
    } while(false);

  setOfPages::result res = soPages.addPage(filename, iP_Parent, openedBy, level, &iP);
  if(setOfPages::failed == res)
    return 0;

  if(setOfPages::exist == res) {
    if(!iP_Parent)
      return 0;
    return iP;
    }
  if(!iP)
    return 0;

  setOfString sos;
  pageStrToSet(sos, filename);

  checkBitmap(iP, sos, filename);

  checkMenu(iP, sos, level);
  checkLinked(iP, sos, level);

  checkAllVars(iP, sos, level);

  loadSendOnOpen_Close(iP, sos);
  return iP;
}
//----------------------------------------------------------------------------
#define NORM_NAME _T("norm.dat")
#define MAX_NORMAL 256
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
#define STD_HEADER_NAME _T("header2" PAGE_EXT)
//----------------------------------------------------------------------------
#define ID_MSG_HIDE_HEADER    7
#define ID_MSG_NAME_HEADER   14
//----------------------------------------------------------------------------
#define ID_PASSWORD_TIME_LEASE 60
//----------------------------------------------------------------------------
#define ID_MSG_DLL           25
#define ID_ADDR_CURRENT_DATETIME 31
#define ID_MSG_FILTERALARM  135
#define ID_MSG_SAVESCREEN   558
#define ID_MSG_PRINTSCREEN  559
#define ID_MSG_PRINT_VARS   600
#define ID_MSG_TREND       1000

#define ID_REAL_PLC         154
#define ID_PRF_READER      6400
#define ID_MSG_MAN_STOP    6403
#define ID_ADDR_CODE_OPER  5991
#define ID_ADDR_ALTERNATE_INPUT_CAUSE 32
//----------------------------------------------------------------------------
#define ID_MSG_INIT_LINE   520
#define ID_MANAGE_RESET    530
#define ID_MSG_INIT_ALARM  531
#define ID_MSG_INIZIALIZED 800

#define INIT_TRAY_DATA    1100
//----------------------------------------------------------------------------
#define TXT_MSG_ALARM       _T("Addr 4 Alarm/Event Word")
#define TXT_MSG_ALARM_STAT  _T("Addr 4 Alarm/Event Status")
#define TXT_MSG_ALARM_ASSOC _T("Addr 4 Alarm Assoc")
#define TXT_MSG_ALARM_RESET _T("Addr 4 Alarm Reset")
#define TXT_MSG_LINE        _T("Addr 4 Line status")
//----------------------------------------------------------------------------
#define TXT_MSG_FILTERALARM1  _T("Addr 4 Filter alarm set")
#define TXT_MSG_FILTERALARM2  _T("Addr 4 Filter alarm show")
#define TXT_MSG_PRINTSCREEN   _T("Print Screen")
#define TXT_MSG_SAVESCREEN    _T("Save Screen")
#define TXT_MSG_PRINTVARS     _T("Print Vars")
#define TXT_MSG_TREND         _T("Trend")
#define TXT_MSG_MAN_STOP      _T("Manage Bit Stop by Card Reader")
#define TXT_MSG_MAN_ADDR      _T("Addr 4 Code Operator")
#define TXT_MSG_AUTO_LISTBOX  _T("Addr 4 ListBox Cause")
#define TXT_MSG_NORM          _T("Addr 4 Normalizer jump")
#define TXT_MSG_RESERVED      _T("Reserved")

#define TXT_MSG_DATETIME      _T("Current DateTime")

#define TXT_MSG_INITIALIZED   _T("Initialized")

#define TXT_MSG_SEND_ON_START _T("Send on start")
#define TXT_MSG_SEND_ON_CLOSE _T("Send on close")

#define TXT_MSG_SYSTRAY       _T("SysTray")

#define TXT_MSG_PSW           _T("Addr 4 Password")
//----------------------------------------------------------------------------
void managePages::loadReserved(uint idPrph)
{
  uint addr = MAX_DWORD_PERIF;
  if(1 == idPrph)
    addr = SIZE_OTHER_DATA;
  --addr;

  infoPages* iP = 0;

  setOfPages::result res = soPages.addPage(TXT_MSG_RESERVED, 0, 0, 0, &iP);
  if(setOfPages::failed == res)
    return;

  iP->setPageType(infoPrph::eStdMsg);

  infoPrph* prph = new infoPrph;

  // fill prph
  SET_pO(prph, Page, iP->getPageCurr());
  SET_pO(prph, pageType, infoPrph::eStdMsg);

  SET_pO(prph, X, COORD_BY_FUNCT);
  SET_pO(prph, Y, 0);

  SET_pO(prph, Prph, idPrph);
  SET_pO(prph, Addr, addr);
  SET_pO(prph, dataType, 4);
  SET_pO(prph, nBit, 0);
  SET_pO(prph, Offs, 0);

  SET_pO(prph, objType, infoPrph::noObj);
  addGlobalPrph(prph);
}
//----------------------------------------------------------------------------
void managePages::loadStdMsgPswManage(const setOfString& sos)
{
// id,maxTime,prph,addr,maxLevel
  LPCTSTR p = sos.getString(ID_PASSWORD_TIME_LEASE);
  while(p) {
    int maxTime = 0;
    int prp = 0;
    int addr = 0;
    int maxLevel = 0;
    _stscanf(p, _T("%d,%d,%d,%d"), &maxTime, &prp, &addr, &maxLevel);

    infoPages* iP = 0;
    setOfPages::result res = soPages.addPage(TXT_MSG_PSW, 0, 0, 0, &iP);
    if(setOfPages::failed == res)
        break;

    iP->setPageType(infoPrph::eStdMsg);

    infoPrph* prph = new infoPrph;

    // il numero risultante sarà: maxLevel zero maxTime, es. 2015
    uint t = 10;
    uint t2 = maxTime;
    while(t2) {
      t *= 10;
      t2 /= 10;
      }
    maxLevel *= t;
    maxLevel += maxTime;

      // fill prph
    SET_pO(prph, Page, iP->getPageCurr());
    SET_pO(prph, pageType, infoPrph::eStdMsg);

    SET_pO(prph, X, COORD_BY_FUNCT);
    SET_pO(prph, Y, maxLevel);

    SET_pO(prph, Prph, prp);
    SET_pO(prph, Addr, addr);
    SET_pO(prph, dataType, 3);
    SET_pO(prph, nBit, 0);
    SET_pO(prph, Offs, 0);

    SET_pO(prph, objType, infoPrph::noObj);
    addGlobalPrph(prph);
    break;
    }
}
//----------------------------------------------------------------------------
#define STEP_GRP_ALARM_BITS 10
#define MAX_VAR_4_FILTER_ALARM 6
//----------------------------------------------------------------------------
void managePages::loadStdMsgFilterAlarm(const setOfString& sos)
{
  LPCTSTR p = sos.getString(ID_MSG_FILTERALARM);
  if(p) {
    uint firstAddr = _ttoi(p);
    p = findNextParamTrim(p, 3);
    use64bit4Alarmfilter = (p ? _ttoi(p) : 0);
    int repeat = use64bit4Alarmfilter + 1;


    for(int j = 0; j < repeat; ++j) {
      uint addr = firstAddr;
      for(uint i = 0; i < MAX_VAR_4_FILTER_ALARM; ++i, ++addr) {
        infoPages* iP = 0;

        setOfPages::result res = soPages.addPage(i ? TXT_MSG_FILTERALARM2 : TXT_MSG_FILTERALARM1, 0, 0, 0, &iP);
        if(setOfPages::failed == res)
          break;

        iP->setPageType(infoPrph::eStdMsg);

        infoPrph* prph = new infoPrph;

        // fill prph
        SET_pO(prph, Page, iP->getPageCurr());
        SET_pO(prph, pageType, infoPrph::eStdMsg);

        SET_pO(prph, X, COORD_BY_FUNCT);
        SET_pO(prph, Y, COORD_BY_FILTER_ALARM);

        SET_pO(prph, Prph, 1);
        SET_pO(prph, Addr, addr);
        SET_pO(prph, dataType, 4);
        SET_pO(prph, nBit, 0);
        SET_pO(prph, Offs, 0);

        SET_pO(prph, objType, infoPrph::eAlarm);
        addGlobalPrph(prph);
        }
      firstAddr += STEP_GRP_ALARM_BITS;
      }
    }
}
//----------------------------------------------------------------------------
/*
#define ID_MSG_INIT_LINE   520
#define ID_MSG_INIT_ALARM  531
//----------------------------------------------------------------------------
#define TXT_MSG_ALARM  _T("Addr 4 Alarm Word")
#define TXT_MSG_LINE   _T("Addr 4 Line status")
addrAlarm,numWordAlarm,offsEvent,numWordEvent,ifUseStatusFlag,wordStatus,bitMaskAlarm,bitMaskEvent,typeWord

*/
//----------------------------------------------------------------------------
void managePages::loadStdMsgLineStat(const setOfString& sos)
{
  LPCTSTR p = sos.getString(ID_MSG_INIT_LINE);
  while(p) {
    int type = 0;
    int addr = 0;
    _stscanf(p, _T("%d,%d"), &type, &addr);

    if(type < 2)
      break;

    infoPages* iP = 0;
    setOfPages::result res = soPages.addPage(TXT_MSG_LINE, 0, 0, 0, &iP);
    if(setOfPages::failed == res)
        break;

    iP->setPageType(infoPrph::eStdMsg);

    infoPrph* prph = new infoPrph;

      // fill prph
    SET_pO(prph, Page, iP->getPageCurr());
    SET_pO(prph, pageType, infoPrph::eStdMsg);

    SET_pO(prph, X, COORD_BY_FUNCT);
    SET_pO(prph, Y, COORD_BY_FILTER_ALARM);

    SET_pO(prph, Prph, realPlc);
    SET_pO(prph, Addr, addr);
    SET_pO(prph, dataType, 4);
    SET_pO(prph, nBit, 0);
    SET_pO(prph, Offs, 0);

    SET_pO(prph, objType, infoPrph::eAlarm);
    addGlobalPrph(prph);
    break;
    }
}
//----------------------------------------------------------------------------
#define MAX_PERIPH 9
//----------------------------------------------------------------------------
static int getTruePrph(int k)
{
  k += 2;
  if(k >= MAX_PERIPH)
    return 1;
  return k;
}
//----------------------------------------------------------------------------
void managePages::loadStdMsgAlarmGroup(const setOfString& sos)
{
  infoPages* iP = 0;

  setOfPages::result res = soPages.addPage(TXT_MSG_ALARM, 0, 0, 0, &iP);
  if(setOfPages::failed == res)
    return;

  iP->setPageType(infoPrph::eStdMsg);

  LPCTSTR p2 = sos.getString(ID_MANAGE_RESET);

  for(uint k = 0; k < MAX_PERIPH; ++k) {
    int wordResetAlarm = 0;
    int bitReset = -1;
    if(p2) {
      _stscanf(p2, _T("%d,%d"), &wordResetAlarm, &bitReset);
      p2 = findNextParam(p2, 2);
      }

    LPCTSTR p = sos.getString(ID_MSG_INIT_ALARM + k);
    if(!p)
      continue;
    uint addr = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      continue;
    uint nWord = _ttoi(p);
    p = findNextParam(p, 3);
    if(!p)
      continue;

    bool useWordStat = toBool(_ttoi(p));
    p = findNextParam(p, 1);
    if(!p)
      continue;
    uint wordStat = _ttoi(p);

    p = findNextParam(p, 3);
    if(!p)
      continue;
    uint type = _ttoi(p);

    if(-1 != bitReset) {
      infoPages* iPt = 0;

      setOfPages::result res = soPages.addPage(TXT_MSG_ALARM_RESET, 0, 0, 0, &iPt);
      if(setOfPages::failed == res)
        continue;

      iPt->setPageType(infoPrph::eStdMsg);
      infoPrph* prph = new infoPrph;

      // fill prph
      SET_pO(prph, Page, iPt->getPageCurr());
      SET_pO(prph, pageType, infoPrph::eStdMsg);

      SET_pO(prph, X, COORD_BY_FUNCT);
      SET_pO(prph, Y, COORD_BY_FILTER_ALARM);

      SET_pO(prph, Prph, getTruePrph(k));
      SET_pO(prph, Addr, wordResetAlarm);
      SET_pO(prph, dataType, 1);
      SET_pO(prph, nBit, 1);
      SET_pO(prph, Offs, bitReset);

      SET_pO(prph, objType, infoPrph::eAlarm);
      addGlobalPrph(prph);
      }

    if(useWordStat) {
      infoPages* iPt = 0;

      setOfPages::result res = soPages.addPage(TXT_MSG_ALARM_STAT, 0, 0, 0, &iPt);
      if(setOfPages::failed == res)
        continue;

      iPt->setPageType(infoPrph::eStdMsg);
      infoPrph* prph = new infoPrph;

      // fill prph
      SET_pO(prph, Page, iPt->getPageCurr());
      SET_pO(prph, pageType, infoPrph::eStdMsg);

      SET_pO(prph, X, COORD_BY_FUNCT);
      SET_pO(prph, Y, COORD_BY_FILTER_ALARM);

      SET_pO(prph, Prph, getTruePrph(k));
      SET_pO(prph, Addr, wordStat);
      SET_pO(prph, dataType, type);
      SET_pO(prph, nBit, 0);
      SET_pO(prph, Offs, 0);

      SET_pO(prph, objType, infoPrph::eAlarm);
      addGlobalPrph(prph);
      }


    for(uint i = 0; i < nWord; ++i, ++addr) {

      infoPrph* prph = new infoPrph;

      // fill prph
      SET_pO(prph, Page, iP->getPageCurr());
      SET_pO(prph, pageType, infoPrph::eStdMsg);

      SET_pO(prph, X, COORD_BY_FUNCT);
      SET_pO(prph, Y, COORD_BY_FILTER_ALARM);

      SET_pO(prph, Prph, getTruePrph(k));
      SET_pO(prph, Addr, addr);
      SET_pO(prph, dataType, type);
      SET_pO(prph, nBit, 0);
      SET_pO(prph, Offs, 0);

      SET_pO(prph, objType, infoPrph::eAlarm);
      addGlobalPrph(prph);
      }
    addAlarmAssoc(getTruePrph(k));
    }
}
//----------------------------------------------------------------------------
#define ID_FIRST_ASSOC 100000
//----------------------------------------------------------------------------
void managePages::addAlarmAssoc(int prphAl)
{
  TCHAR path[_MAX_PATH];
  wsprintf(path, _T("#alarm_%d" PAGE_EXT), prphAl);
  setOfString sos;
  pageStrToSet(sos, path);
  if(!sos.setFirst())
    return;

  infoPages* iP = 0;

  do {
    uint id = sos.getCurrId();
    if(id >= ID_FIRST_ASSOC && id < ID_FIRST_ASSOC + 100000) {
      if(!iP) {
        setOfPages::result res = soPages.addPage(TXT_MSG_ALARM_ASSOC, 0, 0, 0, &iP);
        if(setOfPages::failed == res)
          return;
        iP->setPageType(infoPrph::eAlarmAssoc);
        }
      LPCTSTR p = sos.getCurrString();
      int idprph = 0;
      int addr = 0;
      int type = 0;
      int nbit = 0;
      int offs = 0;
      _stscanf(p, _T("%d,%d,%d,%d,%d"), &idprph, &addr, &type, &nbit, &offs);

      infoPrph* prph = new infoPrph;

      // fill prph
      SET_pO(prph, Page, iP->getPageCurr());
      SET_pO(prph, pageType, infoPrph::eAlarmAssoc);

      p = findNextParamTrim(p, 5);
      TCHAR assocFile[_MAX_PATH] = { 0 };
      if(p && !_ttoi(p)) {
        switch(*p) {
          case _T('N'):
          case _T('n'):
//          case _T('V'):
//          case _T('v'):
          case _T('B'):
          case _T('b'):
            _tcscpy_s(assocFile, findNextParamTrim(p));
            _tcscat_s(assocFile, PAGE_EXT);
            break;
          }
        }
      if(*assocFile) {
        LPCTSTR loadedPage = soPages.getPage(assocFile);
        if(!loadedPage) {
          infoPages* iP = 0;
          setOfPages::result res = soPages.addPage(assocFile, 0, 0, *p, &iP);
          if(setOfPages::failed == res)
            *assocFile = 0;
          else {
            loadedPage = soPages.getPage(assocFile);
            SET_pO(prph, action, (uint)loadedPage);
            SET_pO(prph, X, COORD_BY_ASSOC_FILENAME);
            }
         }
        }
      if(!*assocFile)
        SET_pO(prph, X, COORD_BY_FUNCT);
      SET_pO(prph, Y, prphAl);
//      SET_pO(prph, Y, COORD_BY_FILTER_ALARM_ASSOC + prphAl);

      SET_pO(prph, Prph, idprph);
      SET_pO(prph, Addr, addr);
      SET_pO(prph, dataType, type);
      SET_pO(prph, nBit, nbit);
      SET_pO(prph, Offs, offs);

      SET_pO(prph, objType, infoPrph::eAlarm);
      addGlobalPrph(prph);
      }
    } while(sos.setNext());
}
//----------------------------------------------------------------------------
void managePages::loadStdMsgCurrDateTime(const setOfString& sos)
{
  LPCTSTR p = sos.getString(ID_ADDR_CURRENT_DATETIME);
  while(p) {
    int addr = 0;
    _stscanf(p, _T("%d"), &addr);

    infoPages* iP = 0;
    setOfPages::result res = soPages.addPage(TXT_MSG_DATETIME, 0, 0, 0, &iP);
    if(setOfPages::failed == res)
        break;

    iP->setPageType(infoPrph::eStdMsg);
    int type[] = { 8, 8, 10, 10 };
    for(uint i = 0; i < 4; ++i, ++addr) {
      infoPrph* prph = new infoPrph;

      // fill prph
      SET_pO(prph, Page, iP->getPageCurr());
      SET_pO(prph, pageType, infoPrph::eStdMsg);

      SET_pO(prph, X, COORD_BY_FUNCT);
      SET_pO(prph, Y, COORD_BY_CURR_DATETIME);

      SET_pO(prph, Prph, 1);
      SET_pO(prph, Addr, addr);
      SET_pO(prph, dataType, type[i]);
      SET_pO(prph, nBit, 0);
      SET_pO(prph, Offs, 0);

      SET_pO(prph, objType, infoPrph::noObj);
      addGlobalPrph(prph);
      }
    break;
    }
}
//----------------------------------------------------------------------------
void managePages::loadStdMsgSaveScreen(const setOfString& sos)
{
  LPCTSTR p = sos.getString(ID_MSG_SAVESCREEN);
  while(p) {
    int idPrph = 0;
    int addr = 0;
    int type = 0;
    int bit = 0;
    int idPrph2 = 0;
    int addr2 = 0;
    int len = 0;
//  558,1,49,4,0,1,50,80,expimage\\screen.jpg
    _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d,%d"), &idPrph, &addr, &type, &bit, &idPrph2, &addr2, &len);

    if(!idPrph)
      break;
    infoPages* iP = 0;
    setOfPages::result res = soPages.addPage(TXT_MSG_SAVESCREEN, 0, 0, 0, &iP);
    if(setOfPages::failed == res)
        break;

    iP->setPageType(infoPrph::eStdMsg);

    infoPrph* prph = new infoPrph;

      // fill prph
    SET_pO(prph, Page, iP->getPageCurr());
    SET_pO(prph, pageType, infoPrph::eStdMsg);

    SET_pO(prph, X, COORD_BY_FUNCT);
    SET_pO(prph, Y, COORD_BY_SAVE_SCREEN);

    SET_pO(prph, Prph, idPrph);
    SET_pO(prph, Addr, addr);
    SET_pO(prph, dataType, type);
    SET_pO(prph, nBit, 1);
    SET_pO(prph, Offs, bit);

    SET_pO(prph, objType, infoPrph::ePrint);
    addGlobalPrph(prph);
    int len4 = ROUND_DWORD_LEN(len);
    for(int i = 0; i < len4; ++i, ++addr2) {
      infoPrph* prph = new infoPrph;
      SET_pO(prph, Page, iP->getPageCurr());
      SET_pO(prph, pageType, infoPrph::eStdMsg);

      SET_pO(prph, X, COORD_BY_FUNCT);
      SET_pO(prph, Y, COORD_BY_SAVE_SCREEN);

      SET_pO(prph, Prph, idPrph2);
      SET_pO(prph, Addr, addr2);
      SET_pO(prph, dataType, 14);
      addGlobalPrph(prph);
      }
    break;
    }
}
//----------------------------------------------------------------------------
void managePages::loadStdMsgPrintScreen(const setOfString& sos)
{
  LPCTSTR p = sos.getString(ID_MSG_PRINTSCREEN);
  while(p) {
    int idPrph = 0;
    int addr = 0;
    int type = 0;
    int bit = 0;
    _stscanf(p, _T("%d,%d,%d,%d"), &idPrph, &addr, &type, &bit);

    if(!idPrph)
      break;
    infoPages* iP = 0;
    setOfPages::result res = soPages.addPage(TXT_MSG_PRINTSCREEN, 0, 0, 0, &iP);
    if(setOfPages::failed == res)
        break;

    iP->setPageType(infoPrph::eStdMsg);

    infoPrph* prph = new infoPrph;

      // fill prph
    SET_pO(prph, Page, iP->getPageCurr());
    SET_pO(prph, pageType, infoPrph::eStdMsg);

    SET_pO(prph, X, COORD_BY_FUNCT);
    SET_pO(prph, Y, COORD_BY_PRINT_SCREEN);

    SET_pO(prph, Prph, idPrph);
    SET_pO(prph, Addr, addr);
    SET_pO(prph, dataType, type);
    SET_pO(prph, nBit, 1);
    SET_pO(prph, Offs, bit);

    SET_pO(prph, objType, infoPrph::ePrint);
    addGlobalPrph(prph);
    break;
    }
}
//----------------------------------------------------------------------------
void managePages::loadStdMsgPrintVars(const setOfString& sos)
{
  LPCTSTR p = sos.getString(ID_MSG_PRINT_VARS);
  while(p) {
    uint idGroup = _ttoi(p);
    p = findNextParam(p, 1);
    LPCTSTR pg = sos.getString(idGroup);
    if(!pg)
      continue;

    int idPrph = 0;
    int addr = 0;
    int bit = 0;
    _stscanf(pg, _T("%d,%d,%d"), &idPrph, &addr, &bit);
    if(!idPrph)
      continue;

    infoPages* iP = 0;
    setOfPages::result res = soPages.addPage(TXT_MSG_PRINTVARS, 0, 0, 0, &iP);
    if(setOfPages::failed == res)
      break;

    iP->setPageType(infoPrph::eStdMsg);

    infoPrph* prph = new infoPrph;

      // fill prph
    SET_pO(prph, Page, iP->getPageCurr());
    SET_pO(prph, pageType, infoPrph::eStdMsg);

    SET_pO(prph, X, COORD_BY_FUNCT);
    SET_pO(prph, Y, COORD_BY_PRINT_VARS);

    SET_pO(prph, Prph, idPrph);
    SET_pO(prph, Addr, addr);
    SET_pO(prph, dataType, 4);
    SET_pO(prph, nBit, 1);
    SET_pO(prph, Offs, bit);

    SET_pO(prph, objType, infoPrph::ePrint);
    addGlobalPrph(prph);
    // non dovrebbe essere necessario aggiungere anche le variabili utilizzate
    // per la stampa, questa funzione è poco usata e poi si vedrebbe in stampa
    // se le variabili fossero state sporcate.
    LPCTSTR tfile = sos.getString(idGroup + 1);
    if(!tfile)
      continue;
    LPCTSTR p2 = sos.getString(idGroup + 3);
    if(!p2)
      continue;

    do {
      LPCTSTR t = skipFirstFolder(tfile);
      myStr mstr(t);
      uint dummy;
      if(!stdPages.find(mstr, dummy)) {
        mstr.P = str_newdup(t);
        stdPages.insert(mstr);
        }
      } while(false);
    do {
      LPCTSTR tfile = sos.getString(idGroup + 2);
      if(!tfile)
        break;
      LPCTSTR t = skipFirstFolder(tfile);
      myStr mstr(t);
      uint dummy;
      if(!stdPages.find(mstr, dummy)) {
        mstr.P = str_newdup(t);
        stdPages.insert(mstr);
        }
      } while(false);

    idPrph = 0;
    addr = 0;
    int type = 0;
    _stscanf(p2, _T("%d,%d,%d"), &idPrph, &addr, &type);
    if(!idPrph ||!addr || !type)
      continue;
    TCHAR path[_MAX_PATH];
    _tcscpy(path, Path);
    int len = _tcslen(path);
    // si parte dal penultimo perché l'ultimo potrebbe essere una '\\'
    for(int i = len - 2; i >= 0; --i) {
      if(_T('\\') == path[i]) {
        path[i] = 0;
        break;
        }
      }
    appendPath(path, tfile);
    setOfString SOS;
    pageStrToSet(SOS, path);
    LPCTSTR pPrph = SOS.getString(idPrph);
    if(!pPrph)
      continue;
    LPCTSTR pAddr = SOS.getString(addr);
    if(!pAddr)
      continue;
    LPCTSTR pType = SOS.getString(type);
    if(!pType)
      continue;

    while(pPrph && pAddr && pType) {
      idPrph = _ttoi(pPrph);
      pPrph = findNextParam(pPrph, 1);

      addr = _ttoi(pAddr);
      pAddr = findNextParam(pAddr, 1);

      type = _ttoi(pType);
      pType = findNextParam(pType, 1);

      infoPrph* prph = new infoPrph;

        // fill prph
      SET_pO(prph, Page, iP->getPageCurr());
      SET_pO(prph, pageType, infoPrph::eStdMsg);

      SET_pO(prph, X, COORD_BY_FUNCT);
      SET_pO(prph, Y, COORD_BY_PRINT_VARS);

      SET_pO(prph, Prph, idPrph);
      SET_pO(prph, Addr, addr);
      SET_pO(prph, dataType, type);

      SET_pO(prph, objType, infoPrph::ePrint);
      addGlobalPrph(prph);
      }
    }
}
//----------------------------------------------------------------------------
void managePages::loadStdMsgTrend(const setOfString& sos)
{
  LPCTSTR p = sos.getString(ID_MSG_TREND);
  while(p) {
    uint idGroup = _ttoi(p);
    p = findNextParam(p, 1);
    LPCTSTR pg = sos.getString(idGroup);
    if(!pg)
      continue;
    TCHAR buff[500];
    pvvChar target;
    uint nElem = splitParam(target, pg);
    if(7 != nElem)
      continue;
    wsprintf(buff, _T("%s (%s_%s_%s)"),
//          TXT_MSG_TREND,
          target[6].getVect(),
          target[3].getVect(),
          target[5].getVect(),
          target[4].getVect());
    infoPages* iP = 0;
    setOfPages::result res = soPages.addPage(buff, 0, 0, 0, &iP);
    if(setOfPages::failed == res)
        break;

    iP->setPageType(infoPrph::eStdMsg);
    infoPrph* prph = new infoPrph;
      // fill prph
    SET_pO(prph, Page, iP->getPageCurr());
    SET_pO(prph, pageType, infoPrph::eStdMsg);

    SET_pO(prph, X, COORD_BY_FUNCT);
    SET_pO(prph, Y, COORD_BY_TIMER_TREND);

    SET_pO(prph, Prph, 1);
    SET_pO(prph, Addr, _ttoi(target[0].getVect()));
    SET_pO(prph, dataType, 4);
    SET_pO(prph, nBit, 0);
    SET_pO(prph, Offs, 0);

    SET_pO(prph, objType, infoPrph::eTrend);
    addGlobalPrph(prph);

    pg = sos.getString(_ttoi(target[1].getVect()));
    if(!pg)
      continue;

    int idTotData = 0;
    int idInitData = 0;
    int idEnableAddr = 0;
    _stscanf(pg, _T("%d,%d,%d"), &idTotData, &idInitData, &idEnableAddr);


    pg = sos.getString(idEnableAddr);
    if(pg) {
      infoPages* iP = 0;
      setOfPages::result res = soPages.addPage(buff, 0, 0, 0, &iP);
      if(setOfPages::failed != res) {

        int idPrph = 0;
        int addr = 0;
        int type = 0;
        int nBit = 0;
        int offs = 0;
        _stscanf(pg, _T("%d,%d,%d,%d,%d"), &idPrph, &addr, &type, &nBit, &offs);

        iP->setPageType(infoPrph::eStdMsg);
        infoPrph* prph = new infoPrph;
          // fill prph
        SET_pO(prph, Page, iP->getPageCurr());
        SET_pO(prph, pageType, infoPrph::eStdMsg);

        SET_pO(prph, X, COORD_BY_FUNCT);
        SET_pO(prph, Y, COORD_BY_ENABLE_TREND);

        SET_pO(prph, Prph, idPrph);
        SET_pO(prph, Addr, addr);
        SET_pO(prph, dataType, type);
        SET_pO(prph, nBit, nBit);
        SET_pO(prph, Offs, offs);

        SET_pO(prph, objType, infoPrph::eTrend);
        addGlobalPrph(prph);
        }
      }
    for(int i = 0; i < idTotData; ++i) {
      pg = sos.getString(idInitData + i);
      if(!pg)
        break;

      infoPages* iP = 0;
      setOfPages::result res = soPages.addPage(buff, 0, 0, 0, &iP);
      if(setOfPages::failed == res)
          break;

      int idPrph = 0;
      int addr = 0;
      int type = 0;
      int norm = 0;
      _stscanf(pg, _T("%d,%d,%d,%d"), &idPrph, &addr, &type, &norm);

      iP->setPageType(infoPrph::eStdMsg);

      infoPrph* prph = new infoPrph;

      // fill prph
      SET_pO(prph, Page, iP->getPageCurr());
      SET_pO(prph, pageType, infoPrph::eStdMsg);

      SET_pO(prph, X, COORD_BY_FUNCT);
      SET_pO(prph, Y, i);

      SET_pO(prph, Prph, idPrph);
      SET_pO(prph, Addr, addr);
      SET_pO(prph, dataType, type);
      SET_pO(prph, Norm, norm);
      SET_pO(prph, nBit, 0);
      SET_pO(prph, Offs, 0);

      SET_pO(prph, objType, infoPrph::eTrend);
      addGlobalPrph(prph);
      }
    }
}
//----------------------------------------------------------------------------
void managePages::loadStdMsgManageStop(const setOfString& sos)
{
  LPCTSTR p = sos.getString(ID_PRF_READER);
  while(p) {
    p = sos.getString(ID_MSG_MAN_STOP);
    if(p) {
      int idPrph = realPlc;
      int addr = 0;
      int type = 0;
      int bit = 0;
      _stscanf(p, _T("%d,%d,%d"), &addr, &type, &bit);

      infoPages* iP = 0;
      setOfPages::result res = soPages.addPage(TXT_MSG_MAN_STOP, 0, 0, 0, &iP);
      if(setOfPages::failed == res)
          break;

      iP->setPageType(infoPrph::eStdMsg);

      infoPrph* prph = new infoPrph;

        // fill prph
      SET_pO(prph, Page, iP->getPageCurr());
      SET_pO(prph, pageType, infoPrph::eStdMsg);

      SET_pO(prph, X, COORD_BY_FUNCT);
      SET_pO(prph, Y, COORD_BY_MAN_STOP);

      SET_pO(prph, Prph, idPrph);
      SET_pO(prph, Addr, addr);
      SET_pO(prph, dataType, type);
      SET_pO(prph, nBit, 1);
      SET_pO(prph, Offs, bit);

      SET_pO(prph, objType, infoPrph::eManStop);
      addGlobalPrph(prph);
      }
    p = sos.getString(ID_ADDR_CODE_OPER);
    if(p) {
      int idPrph = realPlc;
      int addr = 0;
      _stscanf(p, _T("%d,%d"), &idPrph, &addr);

      infoPages* iP = 0;
      setOfPages::result res = soPages.addPage(TXT_MSG_MAN_ADDR, 0, 0, 0, &iP);
      if(setOfPages::failed == res)
        break;

      iP->setPageType(infoPrph::eStdMsg);

      infoPrph* prph = new infoPrph;

        // fill prph
      SET_pO(prph, Page, iP->getPageCurr());
      SET_pO(prph, pageType, infoPrph::eStdMsg);

      SET_pO(prph, X, COORD_BY_FUNCT);
      SET_pO(prph, Y, COORD_BY_MAN_STOP_OPER);

      SET_pO(prph, Prph, idPrph);
      SET_pO(prph, Addr, addr);
      SET_pO(prph, dataType, 4);
      SET_pO(prph, nBit, 0);
      SET_pO(prph, Offs, 0);

      SET_pO(prph, objType, infoPrph::eManStopAddrOper);
      addGlobalPrph(prph);
      }
    p = sos.getString(ID_ADDR_ALTERNATE_INPUT_CAUSE);
    if(!p)
      break;

    int addr = _ttoi(p);
    if(!addr)
      break;
    int idPrph = realPlc;

    infoPages* iP = 0;
    setOfPages::result res = soPages.addPage(TXT_MSG_AUTO_LISTBOX, 0, 0, 0, &iP);
    if(setOfPages::failed == res)
        break;

    iP->setPageType(infoPrph::eStdMsg);

    infoPrph* prph = new infoPrph;

      // fill prph
    SET_pO(prph, Page, iP->getPageCurr());
    SET_pO(prph, pageType, infoPrph::eStdMsg);

    SET_pO(prph, X, COORD_BY_FUNCT);
    SET_pO(prph, Y, COORD_BY_MAN_STOP_OPER);

    SET_pO(prph, Prph, idPrph);
    SET_pO(prph, Addr, addr);
    SET_pO(prph, dataType, 4);
    SET_pO(prph, nBit, 0);
    SET_pO(prph, Offs, 0);

    SET_pO(prph, objType, infoPrph::eManStopAddrListBox);
    addGlobalPrph(prph);
    break;
    }
}
//----------------------------------------------------------------------------
LPCTSTR managePages::performLoadSendOnOpen_Close(infoPages* iP, LPCTSTR p, infoPrph::eAction act, LPCTSTR txt)
{
  if(!p)
    return 0;
  uint idprph = 0;
  uint addr = 0;
  uint type = 0;
  uint nbit = 0;
  uint offs = 0;
  uint norm = 0;
  int n = _stscanf_s(p, _T("%d,%d,%d,%d,%d,%d"), &idprph, &addr, &type, &nbit, &offs, &norm);
  if(!idprph)
    return 0;

  const infoPrph* prphT = iP->getInfoOpenedBy();
  uint pgType = defPageType;
  dualOffset dOffs;
  dOffs.offset = 0;
  if(prphT) {
    pgType = actionToPagetype(GET_pV(prphT, action));
    GET_pO(prphT, OffsLink, dOffs.offset);
    }

/*
  infoPages* iP = 0;
  setOfPages::result res = soPages.addPage(txt, 0, 0, 0, &iP);
  if(setOfPages::failed == res)
    return 0;

  iP->setPageType(infoPrph::eNorm);
*/
  infoPrph* prph = new infoPrph;

    // fill prph
  SET_pO(prph, Page, iP->getPageCurr());
  SET_pO(prph, pageType, pgType);

  SET_pO(prph, X, 0);
  SET_pO(prph, Y, 0);

  SET_pO(prph, Prph, idprph);
  if(PRF_MEMORY == idprph && addr <= 1)
    SET_pO(prph, Addr, addr);
  else {
    if(dOffs.offset)
      calcAddrAndOffs(addr, offs, dOffs.offsAddr, dOffs.offsBit, GET_NBIT(type));
    SET_pO(prph, Addr, addr);
    SET_pO(prph, OffsLink, dOffs.offset);
    }
  SET_pO(prph, dataType, type);
  SET_pO(prph, nBit, nbit);
  SET_pO(prph, Offs, offs);
  SET_pO(prph, action, act);

  prph->setNorm(norm);

  SET_pO(prph, objType, infoPrph::noObj);
  addGlobalPrph(prph);

  return findNextParamTrim(p, n + 1);
}
//----------------------------------------------------------------------------
void managePages::loadSendOnOpen_Close(infoPages* iP, const setOfString& sos)
{
  uint ids[] = { ID_SEND_VALUE_ON_OPEN, ID_SEND_VALUE_ON_CLOSE };
  infoPrph::eAction act[] = { infoPrph::eSendOnOpen, infoPrph::eSendOnClose };
  LPCTSTR txt[] = { TXT_MSG_SEND_ON_START, TXT_MSG_SEND_ON_CLOSE };

  for(uint i = 0; i < SIZE_A(ids); ++i) {
    LPCTSTR p = sos.getString(ids[i]);
    while(p)
      p = performLoadSendOnOpen_Close(iP, p, act[i], txt[i]);
    }
}
//----------------------------------------------------------------------------
void managePages::loadStdMsgInitialized(const setOfString& sos)
{
  LPCTSTR p = sos.getString(ID_MSG_INIZIALIZED);
  if(!p)
    return;
  uint n = _ttoi(p);
  for(uint i = 0; i < n; ++i) {
    p = sos.getString(ID_MSG_INIZIALIZED + 1 + i);
    if(!p)
      continue;
    int addr = 0;
    int value = 0;
    _stscanf(p, _T("%d,%d"), &addr, &value);
    infoPages* iP = 0;
    setOfPages::result res = soPages.addPage(TXT_MSG_INITIALIZED, 0, 0, 0, &iP);
    if(setOfPages::failed == res)
      break;

    iP->setPageType(infoPrph::eStdMsg);

    infoPrph* prph = new infoPrph;

        // fill prph
    SET_pO(prph, Page, iP->getPageCurr());
    SET_pO(prph, pageType, infoPrph::eStdMsg);

    SET_pO(prph, X, COORD_BY_FUNCT);
    SET_pO(prph, Y, value);
//    SET_pO(prph, Y, COORD_BY_INITIALIZED);

    SET_pO(prph, Prph, 1);
    SET_pO(prph, Addr, addr);
    SET_pO(prph, dataType, 4);
    SET_pO(prph, objType, infoPrph::noObj);
    addGlobalPrph(prph);
    }
}
//----------------------------------------------------------------------------
void managePages::loadStdMsgSystray(const setOfString& sos)
{
  LPCTSTR p = sos.getString(INIT_TRAY_DATA);
  if(!p)
    return;
  uint n = _ttoi(p);
  for(uint i = 0; i < n; ++i) {
    p = sos.getString(INIT_TRAY_DATA + 1 + i);
    if(!p)
      continue;
    int prph_ = 0;
    int addr = 0;
    int type = 0;
    int nbit = 0;
    int offs = 0;
    int level = 0;

    _stscanf(p, _T("%d,%d,%d,%d,%d,%d"), &prph_, &addr, &type, &nbit, &offs, &level);
    infoPages* iP = 0;
    setOfPages::result res = soPages.addPage(TXT_MSG_SYSTRAY, 0, 0, 0, &iP);
    if(setOfPages::failed == res)
      break;

    iP->setPageType(infoPrph::eStdMsg);

    infoPrph* prph = new infoPrph;

        // fill prph
    SET_pO(prph, Page, iP->getPageCurr());
    SET_pO(prph, pageType, infoPrph::eStdMsg);

    SET_pO(prph, X, COORD_BY_FUNCT);
    SET_pO(prph, Y, level);

    SET_pO(prph, Prph, prph_);
    SET_pO(prph, Addr, addr);
    SET_pO(prph, dataType, type);
    SET_pO(prph, nBit, nbit);
    SET_pO(prph, Offs, offs);
    SET_pO(prph, objType, infoPrph::noObj);
    addGlobalPrph(prph);
    }
}
//----------------------------------------------------------------------------
void managePages::loadHeader(const setOfString& sos)
{
  LPCTSTR p = sos.getString(ID_MSG_HIDE_HEADER);
  if(p && (1 & _ttoi(p)))
    return;
  p = sos.getString(ID_MSG_NAME_HEADER);
  if(!p)
    p = STD_HEADER_NAME;
  defPageType = infoPrph::eHeader;
  Header = loadPage(p, 0, 0, 0);
//  if(Header)
//    Header->setPageType(infoPrph::eHeader);
}
//----------------------------------------------------------------------------
void makeSetNorm(setOfString& sos)
{
  pageStrToSet(sos, NORM_NAME);
}
//----------------------------------------------------------------------------
void managePages::loadNorm()
{
  setOfString sos;
  pageStrToSet(sos, NORM_NAME);
  if(sos.setFirst()) {
    infoPages* iP = 0;
    do {
      long id = sos.getCurrId();
      while(id > 0 && id <= MAX_NORMAL) {
        LPCTSTR p = sos.getCurrString();
        p = findNextParam(p, 3);
        if(!p)
          break;
        uint type = _ttoi(p);
        if(!type || type > 2)
          break;
        p = findNextParam(p, 1);
        if(!p)
          break;
        uint idPrph = 0;
        uint addr = 0;
        uint ty_lk = 0;
        uint bit = 0;
        _stscanf(p, _T("%d,%d,%d,%d"),  &idPrph, &addr, &ty_lk, &bit);
        if(!iP) {
          setOfPages::result res = soPages.addPage(TXT_MSG_NORM, 0, 0, 0, &iP);
          if(setOfPages::failed == res) {
            sos.setLast();
            break;
            }
          }

        iP->setPageType(infoPrph::eNorm);
        infoPrph* prph = new infoPrph;

        // fill prph
        SET_pO(prph, Page, iP->getPageCurr());
        SET_pO(prph, pageType, infoPrph::eNorm);

        SET_pO(prph, X, COORD_BY_FUNCT);
        SET_pO(prph, Y, 1 == type ? COORD_BY_NORM_LINK_VAR : COORD_BY_NORM_LINK_BIT);

        SET_pO(prph, Prph, idPrph);
        SET_pO(prph, Addr, addr);
        SET_pO(prph, dataType, 1 == type ? ty_lk : 1);
        SET_pO(prph, nBit, 1 == type ? 0 : ty_lk);
        SET_pO(prph, Offs, 1 == type ? 0 : bit);

        SET_pO(prph, objType, infoPrph::noObj);
        addGlobalPrph(prph);
        break;
        }
      } while(sos.setNext());
    }
}
//----------------------------------------------------------------------------
void managePages::loadStdMsg(setOfString& sos)
{
  pageStrToSet(sos, STD_MSG_NAME);

  LPCTSTR p = sos.getString(ID_REAL_PLC);
  if(p)
    realPlc = _ttoi(p);

  loadHeader(sos);

  loadStdMsgFilterAlarm(sos);
  loadStdMsgAlarmGroup(sos);
  loadStdMsgLineStat(sos);

  loadStdMsgSaveScreen(sos);
  loadStdMsgPrintScreen(sos);
  loadStdMsgPrintVars(sos);
  loadStdMsgTrend(sos);
  loadStdMsgManageStop(sos);
  loadStdMsgCurrDateTime(sos);
  loadStdMsgInitialized(sos);
  loadStdMsgDll(sos);
  loadStdMsgSystray(sos);
  loadStdMsgPswManage(sos);
}
//----------------------------------------------------------------------------
#define DEF_NAME_EXPRESS_DLL _T("expressDll.dll")
//----------------------------------------------------------------------------
bool managePages::makeNameDllText(LPTSTR path, LPCTSTR p)
{
  _tcscpy(path, Path);
  appendPath(path, p);
  P_File::chgExt(path, PAGE_EXT);
  if(!P_File::P_exist(path)) {
    _tcscpy(path, Path);
    int len = _tcslen(path);
    // si parte dal penultimo perché l'ultimo potrebbe essere una '\\'
    for(int i = len - 2; i >= 0; --i) {
      if(_T('\\') == path[i]) {
        path[i] = 0;
        break;
        }
      }
    appendPath(path, p);
    P_File::chgExt(path, PAGE_EXT);
    }
  return !_tcsicmp(p, DEF_NAME_EXPRESS_DLL);
}
//----------------------------------------------------------------------------
#define STR_VARS _T("VARS>")
//------------------------------------------------------------------
#define U_(a) ((unsigned)(a))
//----------------------------------------------------------
static
LPTSTR getLine(LPTSTR buff, int len)
{
  if(len < 0)
    return 0;
  int i;
  LPTSTR p = buff;
  for(i = 0; i < len; ++i, ++p)
    if(U_(*p) == _T('\r') || U_(*p) == _T('\n'))
      break;
  if(i == len)
    return 0;
  TCHAR c = *p;
  *p++ = 0;
  if(++i == len)
    return 0;

  for(int j = i; j < len; ++j, ++p)
    if(U_(*p) != _T('\r') && U_(*p) != _T('\n'))
      break;

  return(i < len ? p : 0);
}
//----------------------------------------------------------------------------
bool managePages::loadRowVals(LPCTSTR p, infoPages* iP, bool& insideVars)
{
  while(*p && *p != _T('$') && *p != _T(';') && *p != _T('<'))
    ++p;
  switch(*p) {
    case _T('<'):
      if(!insideVars) {
        if(!_tcsncicmp(p + 1, STR_VARS, SIZE_A(STR_VARS) - 1))
          insideVars = true;
        }
      else {
        if(_T('/') == p[1] && !_tcsncicmp(p + 2, STR_VARS, SIZE_A(STR_VARS) - 1))
          return false;
        }
      break;
    case _T('$'):
      if(insideVars) {
#if 1
        TCHAR tmp[MAX_VAR_LEN];
        p = extractVar(tmp, p + 1);

        if(!*p)
          break;
        uint var = getIdVarScript(tmp);
#else
        uint var = _ttoi(p + 1);
#endif
        for(;*p; ++p) {
        if(_T('=') == *p) {
            ++p;
            break;
            }
          }
        if(!*p)
          break;

        int idPrph = 2;
        int addr = 0;
        int type = 0;
        int nBit = 0;
        int offs = 0;
        int norm = 0;
        _stscanf(p, _T("%d,%d,%d,%d,%d,%d"), &idPrph, &addr, &type, &nBit, &offs, &norm);

        infoPrph* prph = new infoPrph;

        // fill prph
        SET_pO(prph, Page, iP->getPageCurr());
        SET_pO(prph, pageType, infoPrph::eDll);

        SET_pO(prph, X, COORD_BY_FUNCT);
        SET_pO(prph, Y, var);

        SET_pO(prph, Prph, idPrph);
        SET_pO(prph, Addr, addr);
        SET_pO(prph, Norm, norm);
        SET_pO(prph, dataType, type);
        SET_pO(prph, nBit, nBit);
        SET_pO(prph, Offs, offs);

        //SET_pO(prph, objType, infoPrph::eAlarm);
        addGlobalPrph(prph);
        }
      break;
    }
  return true;
}
//---------------------------------------------------------------------
const int CRYPT_STEP_EXPRESS = 1;
LPCSTR Header = "NPS sys ";
LPCSTR Key = "eNnePiSoft";
//---------------------------------------------------------------------
static bool openClear(LPCTSTR file, infoFileCr& result)
{
  P_File pf(file, P_READ_ONLY);
  if(!pf.P_open())
    return false;
  result.dim = (int)pf.get_len();
  if(!result.dim)
    return false;

  result.buff = new BYTE[result.dim];
  pf.P_read(result.buff, result.dim);
  return true;
}
//---------------------------------------------------------------------
LPTSTR openAndDecrypt(LPCTSTR file, uint& dim)
{
  infoFileCr result;
  result.header = (LPCBYTE)Header;
  result.lenHeader = strlen(Header);
  if(!decryptFile(file, (LPCBYTE)Key, strlen(Key), CRYPT_STEP_EXPRESS, result))
    if(!openClear(file, result))
      return 0;
  LPBYTE t = new BYTE[result.dim + 2];
  CopyMemory(t, result.buff, result.dim);
  t[result.dim] = 0;
  t[result.dim + 1] = 0;
  delete []result.buff;
  dim = result.dim + 2;
  LPTSTR text = autoConvert(t, dim);
  if(!text) {
    delete []t;
    return 0;
    }
  return text;
}
//----------------------------------------------------------------------------
void managePages::performLoadExpressScript(LPCTSTR filescript, infoPages* iP)
{
#if 1
  uint len;
//  LPTSTR buff = openFile(filescript, len);
  LPTSTR buff = openAndDecrypt(filescript, len);
  if(!buff)
    return;
#else
  P_File pf(filescript, P_READ_ONLY);
  if(!pf.P_open())
    return;

  uint len = (uint)pf.get_len();
  LPTSTR buff = new TCHAR[len + 1];
  pf.P_read(buff, len);
  buff[len] = 0;
#endif
  LPTSTR p = buff;

  bool insideVars = false;
  while(len && p) {
    LPTSTR p2 = getLine(p, len);
    int offs = p2 ? p2 - p : len;
    if(!loadRowVals(p, iP, insideVars))
      break;
    len -= offs;
    p = p2;
    }
  delete []buff;
}
//----------------------------------------------------------------------------
#define ID_EXPRESS_DLL_NAME_SCRIPT   10
#define ID_EXPRESS_DLL_RELOAD_SCRIPT 11
#define ID_DLL_GROUP_VARS 2000000
#define MAX_VARS_ON_DLL 100

#define MAX_ADD_EXPRESS    10
#define ID_SCRIPT_NAME_ADD 51
//----------------------------------------------------------------------------
void managePages::performLoadExpressDll(LPCTSTR path, LPCTSTR name)
{
  infoPages* iP = 0;

  setOfPages::result res = soPages.addPage(name, 0, 0, 0, &iP);
  if(setOfPages::failed == res)
    return;

  iP->setPageType(infoPrph::eDll);

  setOfString sos;
  pageStrToSet(sos, path);

  LPCTSTR p = sos.getString(ID_EXPRESS_DLL_RELOAD_SCRIPT);
  if(p) {
    int idPrph = 2;
    int addr = 0;
    int type = 0;
    int nBit = 0;
    int offs = 0;
    int norm = 0;
    _stscanf(p, _T("%d,%d,%d,%d,%d,%d"), &idPrph, &addr, &type, &nBit, &offs, &norm);

    infoPrph* prph = new infoPrph;

    // fill prph
    SET_pO(prph, Page, iP->getPageCurr());
    SET_pO(prph, pageType, infoPrph::eDll);

    SET_pO(prph, X, COORD_BY_FUNCT);
    SET_pO(prph, Y, COORD_BY_DLL_RELOAD);

    SET_pO(prph, Prph, idPrph);
    SET_pO(prph, Addr, addr);
    SET_pO(prph, Norm, norm);
    SET_pO(prph, dataType, type);
    SET_pO(prph, nBit, nBit);
    SET_pO(prph, Offs, offs);

    //SET_pO(prph, objType, infoPrph::eAlarm);
    addGlobalPrph(prph);
    }
  p = sos.getString(ID_EXPRESS_DLL_NAME_SCRIPT);
  if(p) {
    TCHAR t[_MAX_PATH];
    _tcscpy(t, path);
    int len = _tcslen(t);
    for(int i = len - 1; i >= 0; --i) {
      if(_T('\\') == t[i]) {
        t[i] = 0;
        break;
        }
      }
    appendPath(t, p);
    performLoadExpressScript(t, iP);
    }


  for(uint i = 0; i < MAX_ADD_EXPRESS; ++i) {
    p = sos.getString(ID_SCRIPT_NAME_ADD + i);
    if(p) {
      TCHAR t[_MAX_PATH];
      _tcscpy(t, path);
      int len = _tcslen(t);
      for(int i = len - 1; i >= 0; --i) {
        if(_T('\\') == t[i]) {
          t[i] = 0;
          break;
          }
        }
      appendPath(t, p);
      performLoadExpressScript(t, iP);
      }
    }
}
//----------------------------------------------------------------------------
void managePages::performLoadGenericDll(LPCTSTR path, LPCTSTR name)
{
  infoPages* iP = 0;

  setOfPages::result res = soPages.addPage(name, 0, 0, 0, &iP);
  if(setOfPages::failed == res)
    return;

  iP->setPageType(infoPrph::eDll);

  setOfString sos;
  pageStrToSet(sos, path);
  for(uint i = 0; i < MAX_VARS_ON_DLL; ++i) {
    LPCTSTR p = sos.getString(ID_DLL_GROUP_VARS + i);
    if(!p)
      break;
    uint idPrph = _ttoi(p);
    p = findNextParam(p, 1);
    if(!p)
      continue;
    int addr1 = _ttoi(p);
    int addr2 = -1;

    while(*p) {
      if(_T('-') == *p) {
        addr2 = _ttoi(p + 1);
        p = findNextParam(p, 1);
        break;
        }
      if(_T(',') == *p) {
        ++p;
        break;
        }
      ++p;
      }
    if(!p || !*p)
      continue;
    uint type = _ttoi(p);
    p = findNextParam(p, 1);
    int nBit = 0;
    int offs = 0;
    int norm = 0;
    if(p) {
      nBit = _ttoi(p);
      p = findNextParam(p, 1);
      if(p)
        offs = _ttoi(p);
      p = findNextParam(p, 1);
      if(p)
        norm = _ttoi(p);
      }
    uint nRepeat = 1;
    if(-1 != addr2) {
      if(addr2 < addr1) {
        int t = addr1;
        addr1 = addr2;
        addr2 = t;
        }
      nRepeat = addr2 - addr1 + 1;
      }
    for(uint j = 0; j < nRepeat; ++j) {
      infoPrph* prph = new infoPrph;

      // fill prph
      SET_pO(prph, Page, iP->getPageCurr());
      SET_pO(prph, pageType, infoPrph::eDll);

      SET_pO(prph, X, COORD_BY_FUNCT);
      SET_pO(prph, Y, COORD_BY_FUNCT);

      SET_pO(prph, Prph, idPrph);
      SET_pO(prph, Addr, addr1 + j);
      SET_pO(prph, Norm, norm);
      SET_pO(prph, dataType, type);
      SET_pO(prph, nBit, nBit);
      SET_pO(prph, Offs, offs);

      //SET_pO(prph, objType, infoPrph::eAlarm);
      addGlobalPrph(prph);
      }
    }
}
//----------------------------------------------------------------------------
void managePages::loadStdMsgDll(const setOfString& sos)
{
  LPCTSTR p = sos.getString(ID_MSG_DLL);
  if(!p)
    return;
  pvvChar target;
  uint nElem = splitParam(target, p);
  for(uint i = 0; i < nElem; ++i) {
    TCHAR path[_MAX_PATH];
    bool isExpress = makeNameDllText(path, &target[i]);
    if(isExpress)
      performLoadExpressDll(path, &target[i]);
    // controlla anche la parte standard della dll delle espressioni
//    else
      performLoadGenericDll(path, &target[i]);
    }
}
//----------------------------------------------------------------------------
bool managePages::makeAll(PWin* owner)
{
  soPages.reset();
  soPrph.reset();

  clearNames(stdPages);
  // va chiamata prima per memorizzare l'id del plc reale
  sosStdMsg.reset();

  Dlg = new infoDialog(owner, this);
  bool success = IDOK == Dlg->modal();
  delete Dlg;
  Dlg = 0;
  return success;
}
//----------------------------------------------------------------------------
bool managePages::thPerformMakeAll()
{
  loadStdMsg(sosStdMsg);

  loadNorm();

  {
  setOfString s;
  pageStrToSet(s, _T("..\\infoSplash.txt"));

  LPCTSTR p = s.getString(1);
  if(p) {
    p = skipFirstFolder(p);
    soImages.addImage(p, _T("infoSplash.txt"));
    }
  }
  {
  setOfString s;
  pageStrToSet(s, _T("..\\svScreenSaver.txt"));

  const int id_init = 41;
  const int id_end = 60;
  for(int i = id_init; i <= id_end; ++i) {
    LPCTSTR p = s.getString(i);
    if(!p)
      break;
    p = skipFirstFolder(p);
    soImages.addImage(p, _T("svScreenSaver.txt"));
    }
  }
  {
  LPCTSTR p = sosStdMsg.getString(ID_USE_TOUCH_KEYB);
  if(p) {
    p = findNextParamTrim(p);
    if(p) {
      infoPages* iP = 0;
      soPages.addPage(p, 0, 0, 0, &iP);
      {
      setOfString sos;
      pageStrToSet(sos, p);
      checkBitmap(iP, sos, p);
      }
      p = sosStdMsg.getString(ID_NUMB_TOUCH_KEYB);
      if(p) {
        soPages.addPage(p, 0, 0, 0, &iP);
        setOfString sos;
        pageStrToSet(sos, p);
        checkBitmap(iP, sos, p);
        }
      }
    }
  }
  defPageType = infoPrph::eBody;
  Root = loadPage(firstPage, 0, 0, 0);


  const bool* bPrph = getListPrph();
  for(uint i = 1; i < MAX_PRPH; ++i)
    if(bPrph[i])
      loadReserved(i);

  currPage = 0;
  sosStdMsg.reset();
  return toBool(Root);
}
//----------------------------------------------------------------------------
#define SET_MOV_PRPH(d) \
  switch(currOrder) { \
    case byPrph:\
      return soPrph.sPbPrph.set##d();\
    case byPages:\
      return soPrph.sPbPage.set##d();\
    case byOther:\
      return soPrph.sPOther.set##d();\
    case byNorm:\
      return soPrph.sPNorm.set##d();\
    default:\
      return false;\
    }
//----------------------------------------------------------------------------
bool managePages::setFirstPrph()
{
  SET_MOV_PRPH(First)
}
//----------------------------------------------------------------------------
bool managePages::setLastPrph()
{
  SET_MOV_PRPH(Last)
}
//----------------------------------------------------------------------------
bool managePages::setPrevPrph()
{
  SET_MOV_PRPH(Prev)
}
//----------------------------------------------------------------------------
bool managePages::setNextPrph()
{
  SET_MOV_PRPH(Next)
}
//----------------------------------------------------------------------------
bool managePages::setFirstPage()
{
  currPage = 0;
  return toBool(soPages.getElem());
}
//----------------------------------------------------------------------------
bool managePages::setLastPage()
{
  uint nElem = soPages.getElem();
  if(nElem) {
    currPage = nElem - 1;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool managePages::setPrevPage()
{
  if(currPage > 0) {
    --currPage;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool managePages::setNextPage()
{
  uint nElem = soPages.getElem();
  if(currPage < nElem - 1) {
    ++currPage;
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
bool managePages::find(const infoPrph* target)
{
  switch(currOrder) {
    case byPrph:
      return soPrph.sPbPrph.Search((TreeData)target);

    case byPages:
      return soPrph.sPbPage.Search((TreeData)target);

    case byOther:
      return soPrph.sPOther.Search((TreeData)target);

    case byNorm:
      return soPrph.sPNorm.Search((TreeData)target);

    }
  return false;
}
//----------------------------------------------------------------------------
bool managePages::getCurr(const infoPrph* &target)
{
  switch(currOrder) {
    case byPrph:
      target = (const infoPrph*)(soPrph.sPbPrph.getCurr());
      break;
    case byPages:
      target = (const infoPrph*)(soPrph.sPbPage.getCurr());
      break;
    case byNorm:
      target = (const infoPrph*)(soPrph.sPNorm.getCurr());
      break;
    case byOther:
      target = (const infoPrph*)(soPrph.sPOther.getCurr());
      break;
    default:
      return false;
    }
  return toBool(target);
}
//----------------------------------------------------------------------------
bool managePages::getCurr(const infoPages* &target)
{
  uint nElem = soPages.getElem();
  if(nElem) {
    target = soPages.getPageInfo(currPage);
    return true;
    }
  return toBool(target);
}
//------------------------------------------------------------------
unsigned FAR PASCAL LoadAllProc(void* cl)
{
  infoDialog *dlg = reinterpret_cast<infoDialog*>(cl);
  managePages* owner = dlg->getOwner();
  bool success = owner->thPerformMakeAll();
  dlg->finished(success);
  return 0;
}
//----------------------------------------------------------------------------
infoDialog::infoDialog(PWin* parent, managePages* owner) :
  Owner(owner), baseClass(parent, IDD_INFO_DIALOG) {}
//----------------------------------------------------------------------------
bool infoDialog::create()
{
  if(!baseClass::create())
    return false;

  DWORD idThread = 0;
  HANDLE hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)LoadAllProc, this, 0, &idThread);
  if(!hThread)
    return false;
  CloseHandle(hThread);
  return true;
}
//----------------------------------------------------------------------------
void infoDialog::setTextInfo(wrapText* txt)
{
  if(freeTxt.getFree())
    freeTxt.push(txt);
  else
    releaseFree(txt);
  PostMessage(*this, WM_FW_FIRST_FREE, 0, 0);
}
//----------------------------------------------------------------------------
void infoDialog::showText()
{
  while(true) {
    wrapText* t = freeTxt.retrieve();
    if(!t)
      break;
    SetDlgItemText(*this, IDS_INFO, t->txt);
    releaseFree(t);
    }
}
//----------------------------------------------------------------------------
void infoDialog::finished(bool success)
{
  if(success)
    EndDialog(getHandle(), IDOK);
  else
    EndDialog(getHandle(), IDCANCEL);
}
//----------------------------------------------------------------------------
void infoDialog::releaseFree(wrapText* txt)
{
  freeTxt.release(txt);
}
//----------------------------------------------------------------------------
wrapText* infoDialog::getFree()
{
  return freeTxt.get();
}
//----------------------------------------------------------------------------
LRESULT infoDialog::windowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_FW_FIRST_FREE:
      showText();
      break;
    }
  return baseClass::windowProc(hwnd, message, wParam, lParam);
}
