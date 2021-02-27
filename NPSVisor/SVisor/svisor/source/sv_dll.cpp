//----------- sv_dll.cpp -----------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#ifndef USE_DLL
  #define USE_DLL
#endif
//----------------------------------------------------------------------------
#include "password.h"
#include "mainClient.h"
#include "sv_dll.h"
#include "newNormal.h"
#include "dserchfl.h"
#include "pvarlistbox.h"
#include "def_dir.h"
//----------------------------------------------------------------------------
ExpImp_SV
LPCTSTR npSV_GetGlobalString(uint id)
{
  return getString(id);
}
//------------------------------------------------------------------
// solo per forzare l'inclusione del modulo oggetto nell'eseguibile finale,
// altrimenti il linker, non vedendo alcun richiamo alle routine, non lo
// inserisce
int SV_DLL_DUMMY;
//------------------------------------------------------------------
    // legge una stringa specifica della pagina corrente, torna 0 se non esiste
ExpImp_SV
LPCTSTR npSV_GetPageString(uint id)
{
  P_Body* bd = getCurrBody();
  if(bd)
    return bd->getPageString(id);
  return 0;
}
//------------------------------------------------------------------
ExpImp_SV
LPCTSTR
npSV_GetLocalString(uint id, HINSTANCE hdll)
{
  return getLocalString(id, hdll);
}
//------------------------------------------------------------------
static SV_prfData::tResultData SV_NeedSend(uint idPrf, const pSV_prfData data)
{
  SV_prfData dataCurr = *data;
  SV_prfData::tResultData result = npSV_Get(idPrf, &dataCurr);
  if(result < SV_prfData::okData)
    return result;
  if(SV_prfData::tStrData != data->typeVar) {
    if(data->U.li.QuadPart == dataCurr.U.li.QuadPart)
      return SV_prfData::okData;
    }
  else {
    uint len = data->U.str.len;
    if(len == dataCurr.U.str.len)
      if(!memcmp(data->U.str.buff, dataCurr.U.str.buff, len))
        return SV_prfData::okData;
    }
  return SV_prfData::isChanged;
}
//------------------------------------------------------------------
ExpImp_SV
SV_prfData::tResultData
npSV_GetSet(uint idPrf, pSV_prfData data)
{
  mainClient* mc = getMain();
  if(mc) {
    genericPerif* prf = mc->getGenPerif(idPrf);
    if(prf) {
      prfData d;
      if(SV_prfData::tStrData == data->typeVar)
        d.setStr(data->U.str.len, (LPBYTE)data->U.str.buff);
      else {
        d.U.li = data->U.li;
        d.typeVar = data->typeVar;
        }
      d.lAddr = data->lAddr;
      gestPerif::howSend old = gestPerif::NOT_DEFINED;
      if(idPrf != PRF_MEMORY) {
        gestPerif* prph = (gestPerif*)prf;
        if(!prph->isReady())
          return SV_prfData::invalidPerif;

        old = prph->getAutoShow();
        prph->setAutoShow(gestPerif::WAIT_ASYNC);
        }
      SV_prfData::tResultData result = (SV_prfData::tResultData)prf->getset(d, false);
      if(result >=  prfData::okData) {
        if(SV_prfData::tStrData == data->typeVar) {
          uint len = min(d.U.str.len, sizeof(data->U.str.buff) - 1);
          memcpy(data->U.str.buff, d.U.str.buff, len);
          data->U.str.buff[len] = 0;
          }
        else
          data->U.li = d.U.li;
        }

      if(old != gestPerif::NOT_DEFINED)
        ((gestPerif*)prf)->setAutoShow(old);
      return result;
      }
    }
  return SV_prfData::invalidPerif;
}
//------------------------------------------------------------------
ExpImp_SV
SV_prfData::tResultData
npSV_Set(uint idPrf, const pSV_prfData data)
{
  SV_prfData::tResultData result = SV_NeedSend(idPrf, data);
  if(result != SV_prfData::isChanged)
    return result;
  mainClient* mc = getMain();
  if(mc) {
    genericPerif* prf = mc->getGenPerif(idPrf);
    if(prf) {
      prfData d;
      if(SV_prfData::tStrData == data->typeVar)
        d.setStr(data->U.str.len, (LPBYTE)data->U.str.buff);
      else {
        d.U.li = data->U.li;
        d.typeVar = data->typeVar;
        }
      d.lAddr = data->lAddr;
      gestPerif::howSend old = gestPerif::NOT_DEFINED;
      if(idPrf != PRF_MEMORY) {
        gestPerif* prph = (gestPerif*)prf;
        if(!prph->isReady())
          return SV_prfData::okData;

        old = prph->getAutoShow();
        prph->setAutoShow(gestPerif::WAIT_ASYNC);
        }
      SV_prfData::tResultData result = (SV_prfData::tResultData)prf->set(d, false);
      if(old != gestPerif::NOT_DEFINED)
        ((gestPerif*)prf)->setAutoShow(old);
      return result;
      }
    }
  return SV_prfData::invalidPerif;
}
//------------------------------------------------------------------
ExpImp_SV
SV_prfData::tResultData
npSV_Get(uint idPrf, pSV_prfData data)
{
  mainClient* mc = getMain();
  if(mc) {
    genericPerif* prf = mc->getGenPerif(idPrf);
    if(prf) {
      prfData d;
      d.U.li.QuadPart = 0;
      if(SV_prfData::tStrData == data->typeVar)
        d.setStr(data->U.str.len);
      else {
        d.typeVar = data->typeVar;
        d.U.dw = data->U.dw;
        }
      d.lAddr = data->lAddr;
      prfData::tResultData result = prf->get(d);
      if(result >=  prfData::okData) {
        if(SV_prfData::tStrData == data->typeVar) {
          uint len = min(d.U.str.len, sizeof(data->U.str.buff) - 1);
          memcpy(data->U.str.buff, d.U.str.buff, len);
          data->U.str.buff[len] = 0;
          }
        else
          data->U.li = d.U.li;
        }
      return (SV_prfData::tResultData)result;
      }
    }
  return SV_prfData::invalidPerif;
}

//------------------------------------------------------------------
//------------------------------------------------------------------
ExpImp_SV
SV_prfData::tResultData
npSV_MultiSet(uint idPrf, const pSV_prfData data, uint nData)
{
  if(PRF_MEMORY == idPrf) {
    for(int i = 0; i < (int)nData; ++i) {
      SV_prfData::tResultData result = npSV_Set(idPrf, data + i);
      if(SV_prfData::okData > result)
        return result;
      }
    return SV_prfData::okData;
    }

  mainClient* mc = getMain();
  if(mc) {
    gestPerif* prf = mc->getPerif(idPrf);
    if(prf) {
      if(!prf->isReady())
        return SV_prfData::okData;
      prfData* d = new prfData[nData];
      for(int i = 0; i < (int)nData; ++i) {
        if(SV_prfData::tStrData == data[i].typeVar)
          d[i].setStr(data[i].U.str.len, (LPBYTE)data[i].U.str.buff);
        else {
          d[i].U.li = data[i].U.li;
          d[i].typeVar = data[i].typeVar;
          }
        d[i].lAddr = data[i].lAddr;
        }

      gestPerif::howSend old = prf->getAutoShow();
      prf->setAutoShow(gestPerif::WAIT_ASYNC);
      prfData::tResultData result = prf->multiSet(d, nData);
      if(old != gestPerif::NOT_DEFINED)
        prf->setAutoShow(old);
      delete []d;
      return (SV_prfData::tResultData)result;
      }
    }
  return SV_prfData::invalidPerif;
}
//------------------------------------------------------------------
ExpImp_SV
fREALDATA
npSV_GetNormaliz(uint id)
{
  return getNorm(id);
}
//------------------------------------------------------------------
static
LRESULT setListbox(listBoxFields* pl, bool byHwnd)
{
  P_Body* bd = getCurrBody();
  if(!bd)
    return 0;

  PVarListBox* lbx = 0;
  if(byHwnd) {
    PWin* w = PWin::getWindowPtr(pl->hwndLb);
    lbx = dynamic_cast<PVarListBox*>(w);
    }
  else
    lbx = bd->getLBoxFromId(pl->id);
  if(!lbx)
    return 0;

  lbx->SetTabStop(pl->nField, pl->tabs, pl->shows);
  for(uint i = 0; i < pl->nField; ++i)
    if(2 == pl->shows[i])
      lbx->setAlign(i, PListBox::aCenter);
  lbx->recalcWidth();
  lbx->setIntegralHeight();
  return 1;
}
//------------------------------------------------------------------
extern int send_recipe(mainClient* Par, setOfString& sStr);
extern bool load_recipe(mainClient* Par, setOfString& sStr, LPCTSTR name);
extern bool reloadDllText(HINSTANCE hDll);
//------------------------------------------------------------------
static bool getBit(DWORD value, uint offs)
{
  return toBool(value & (1 << offs));
}
//------------------------------------------------------------------
ExpImp_SV
LRESULT
npSV_GetBodyRoutine(uint codeMsg, LPDWORD p1, LPDWORD p2)
{
  switch(codeMsg) {
    case eSV_GET_BODY:
      return (LRESULT)(HANDLE_BODY)getCurrBody();

    case eSV_GET_CURR_MODAL:
      if(p1)
        return (LRESULT)(HANDLE_BODY)((P_Body*)(HANDLE_BODY)p1)->getModal();
      else
        return (LRESULT)(HANDLE_BODY)getCurrBody()->getModal();

    case eSV_OPEN_MODAL:
      getCurrBody()->postOpenModal((LPCTSTR) p1);
      return 1;

    case eSV_GET_FILE:
      do {
        P_Body* bd = getCurrBody();
        while(bd->getModal())
          bd = bd->getModal();


        static infoSearch stInfo;
        infoSearch info((LPCTSTR)p2, (LPCTSTR)p1, stInfo);

        if(!info.chooseParam(bd, true))
          return 0;

        stInfo.copyData(info);
        return gSearchFile(info, bd, (LPTSTR)p1);

        } while(false);
      break;
    case eSV_GET_FILE2:
      do {
        P_Body* bd = getCurrBody();
        while(bd->getModal())
          bd = bd->getModal();

        loadFileInfo* lfi = (loadFileInfo*)p1;
        infoSearch info(lfi->title, lfi->path, getBit(lfi->flag, 0),  getBit(lfi->flag, 1),
                    getBit(lfi->flag, 3),  getBit(lfi->flag, 2));

        return gSearchFile(info, bd, lfi->path);

        } while(false);
      break;

    case eSV_GET_ALARM_STATUS:
      do {
        mainClient* mc = getMain();
        if(!mc)
          break;
        const perifsSet *p = mc->getPerifs();
        gestPerif::statAlarm al = p->hasAlarm();
        switch(al) {
          case gestPerif::sNoAlarm:
            *p1 = 0;
            break;
          case gestPerif::sEvent:
            *p1 = 1;
            break;
          case gestPerif::sAlarm:
            *p1 = 2;
            break;
          }
        return 1;
        } while(false);
      break;

    case eSV_GET_HANDLE_PRPH:
      do {
        *p1 = 0;
        mainClient* mc = getMain();
        if(!mc)
          break;
        gestPerif* p = mc->getPerif((uint)p2);
        if(!p)
          break;
        HWND hwnd = p->getHwndDriver();
        if(!hwnd)
          break;
        *p1 = (DWORD)hwnd;
        return 1;
        } while(false);
      break;

    case eSV_LOAD_RECIPE:
      do {
        mainClient* mc = getMain();
        if(!mc)
          break;
        TCHAR path[_MAX_PATH];
        getFileStrCheckRemote(path, (LPCTSTR)p1);
        setOfString str;
        pageStrToSet(str, path);
        if(!str.setFirst())
          break;
        return (LRESULT)load_recipe(mc, str, (LPCTSTR)p2);
        } while(false);
      break;

    case eSV_SEND_RECIPE:
      do {
        mainClient* mc = getMain();
        if(!mc)
          break;
        TCHAR path[_MAX_PATH];
        getFileStrCheckRemote(path, (LPCTSTR)p1);
        setOfString str;
        pageStrToSet(str, path);
        if(!str.setFirst())
          break;
        if(p2)
          if(!load_recipe(mc, str, (LPCTSTR)p2))
            break;
        return (LRESULT)send_recipe(mc, str);
        } while(false);
      break;

    case eSV_SET_TAB_LISTBOX:
      return setListbox((listBoxFields*)p1, false);
    case eSV_SET_TAB_LISTBOX_BY_HWND:
      return setListbox((listBoxFields*)p1, true);

    case eSV_GET_DEF_TYPE_PRPH:
      do {
        mainClient* mc = getMain();
        if(!mc)
          break;
        const genericPerif* gp = mc->getGenPerif((DWORD)p1);
        if(gp)
          return (LRESULT)gp->getDefaultType();
        } while(false);
      break;
    case eSV_GET_BLOCK_DATA_PRPH:
      do {
        mainClient* mc = getMain();
        if(!mc)
          break;
        blockInfoPrph* bip = (blockInfoPrph*)p1;
        genericPerif* gp = mc->getGenPerif(bip->idPrph);
        if(gp) {
          gp->getBlockData(p2, bip->numdw, bip->addr);
          return 1;
          }
        } while(false);
      break;
    case eSV_RELOAD_TEXT_SET:
      return reloadDllText((HINSTANCE)p1);
    case eSV_GET_PASSWORD:
      do {
        mainClient* mc = getMain();
        if(!mc)
          break;
        PassWord psw;
        return psw.getPsW((int)p1, mc);
        } while(false);
    case eSV_SET_CALL_BACK_LBOX:
      do {
        mainClient* mc = getMain();
        if(!mc)
          break;
        return (LRESULT)mc->setListBoxCallBack((infoCallBackSend*)p2, (DWORD)p1);
        } while(false);
    case eSV_MAKE_PATH:
      do {
        svMakePath* mp = (svMakePath*)p1;
        makePath(mp->file, (eDir)mp->which, mp->history, (eDir)mp->ext);
        } while(false);
      return 1;
    case eSV_MANAGE_DIALOG_FUNCT_KEY:
      do {
        mainClient* mc = getMain();
        if(!mc)
          break;
        if(p1)
          mc->enableDialogFunctionKey();
        else
          mc->disableDialogFunctionKey();
        return 1;
        } while(false);
      case eSV_GET_HWND_MAIN:
      do {
        mainClient* mc = getMain();
        if(!mc)
          break;
        return (LRESULT)mc->getHandle();
        } while(false);
    case eSV_SHOW_OP_FILE_RESULT:
      switch((DWORD)p1) {
        case 0:
          showFileResult(toBool(p2), getCurrBody());
          break;
        case 1:
          return (LRESULT)showMsgFileExist(getCurrBody());
        }
    case eSV_SET_BLOCK_DATA_PRPH:
      do {
        mainClient* mc = getMain();
        if(!mc)
          break;
        blockInfoPrph* bip = (blockInfoPrph*)p1;
        genericPerif* gp = mc->getGenPerif(bip->idPrph);
        if(gp) {
          gp->copyBlockData(p2, bip->numdw, bip->addr);
          return 1;
          }
        } while(false);
      break;
    case eSV_FILL_BLOCK_DATA_PRPH:
      do {
        mainClient* mc = getMain();
        if(!mc)
          break;
        blockInfoPrph* bip = (blockInfoPrph*)p1;
        genericPerif* gp = mc->getGenPerif(bip->idPrph);
        if(gp) {
          gp->fillBlockData((DWORD)p2, bip->numdw, bip->addr);
          return 1;
          }
        } while(false);
      break;
    }
  return 0;
}
