//-------- fv_miniat_images.cpp -------------------------------------
//-------------------------------------------------------------------
#include "precHeader.h"
//-------------------------------------------------------------------
#include "fv_miniat_images.h"
#include "clientWin.h"
#include "extMonitor.h"
//-------------------------------------------------------------------
void container_miniat_images::setBasePath(LPCTSTR newPath)
{
  ((client_miniat_images*)client)->setBasePath(newPath);
}
//-------------------------------------------------------------------
int container_miniat_images::getSelected() const
{
  return ((client_miniat_images*)client)->getSelected();
}
//-------------------------------------------------------------------
void container_miniat_images::setSelected(uint idc)
{
  ((client_miniat_images*)client)->setSelected(idc);
}
//-------------------------------------------------------------------
client_miniat_images::~client_miniat_images()
{
  delete []basePath;
  flushPAV(images);
  destroy();
}
//-------------------------------------------------------------------
bool client_miniat_images::create()
{
  if(!baseClass::create())
    return false;
  return true;
}
//-------------------------------------------------------------------
void client_miniat_images::setSelected(uint idc)
{
  baseClass::setSelected(idc);
  if(!basePath)
    return;
  uint nElem = clients.getElem();
  uint nImg = images.getElem();
  nElem = min(nElem, nImg);
  for(uint i = 0; i < nElem; ++i) {
    if(clients[i]->getId() == idc) {
      TCHAR t[_MAX_PATH];
      _tcscpy_s(t, basePath);
      appendPath(t, images[i]);
      clientWin* cw = getClientImage(this);
      if(cw)
        cw->setBackground(t);
      extMonitor* m = getMonitor(this);
      if(m)
        m->setBackground(t);
      break;
      }
    }
}
//-------------------------------------------------------------------
void client_miniat_images::recreate_miniat()
{
  if(!basePath)
    return;
  uint newImg = images.getElem();
  if(!newImg)
    return;
  uint exists = clients.getElem();
  uint allowed = min(newImg, exists);

  SIZE sz = clients[0]->getBlockSize();
  PRect r(0, 0, sz.cx, sz.cy);
  for(uint i = 0; i < allowed; ++i) {
    ((miniat_image*)clients[i])->loadImage(basePath, images[i]);
    r.Offset(0, r.Height());
    }

  for(uint i = allowed; i < newImg; ++i) {
    clients[i] = makeMiniat(i, r);
    clients[i]->create();
    r.Offset(0, r.Height());
    ((miniat_image*)clients[i])->loadImage(basePath, images[i]);
    }
  for(uint i = newImg; i < exists; ++i)
    delete clients[i];
  clients.setElem(newImg);
  nVert = newImg;
  nHorz = 1;
  PscrollContainer* par = getParentWin<PscrollContainer>(this);
  if(par) {
    PscrollInfo si = par->getInfo();
    si.setMaxVert(nVert);
    par->setInfo(si);
    }
  resize();
}
//-------------------------------------------------------------------
static bool isValidFile(LPCTSTR filename)
{
  TCHAR ext[64];
  _tsplitpath_s(filename, 0, 0, 0, 0, 0, 0, ext, SIZE_A(ext));
  LPCTSTR enabled[] = { _T(".jpg"), _T(".png"), _T(".gif"), _T(".bmp") };
  for(uint i = 0; i < SIZE_A(enabled); ++i)
    if(!_tcsicmp(enabled[i] , ext))
      return true;
  return false;
}
//-------------------------------------------------------------------
void client_miniat_images::fillList()
{
  if(!basePath)
    return;
  flushPAV(images);
  TCHAR old[_MAX_PATH];
  GetCurrentDirectory(SIZE_A(old), old);

  SetCurrentDirectory(basePath);

 WIN32_FIND_DATA FindFileData;
  TCHAR files[] = _T("*.*");
  HANDLE hf = FindFirstFile(files, &FindFileData);
  if(hf != INVALID_HANDLE_VALUE) {
    do {
      if(!(FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes)) {
        if(isValidFile(FindFileData.cFileName))
          images[images.getElem()] = str_newdup(FindFileData.cFileName);
        }
      } while(FindNextFile(hf, &FindFileData));

    FindClose(hf);
    }
}
//-------------------------------------------------------------------
void client_miniat_images::setBasePath(LPCTSTR newPath)
{
  if (!newPath)
    return;
  TCHAR full[_MAX_PATH];
  GetModuleDirName(SIZE_A(full), full);
#ifdef _DEBUG
  dirName(full);
#endif
  SetCurrentDirectory(full);
  _tfullpath(full, newPath, _MAX_PATH);
  if ((basePath && !_tcsicmp(full, basePath)))
    return;
  delete []basePath;
  basePath = str_newdup(full);

  fillList();
  recreate_miniat();
  setSelected(currSel);
}
//-------------------------------------------------------------------
#define ADD_ID 200
Pminiat* client_miniat_images::makeMiniat(uint ix, const PRect& r)
{
  return new miniat_image(this, ix + ADD_ID, r);
}
//-------------------------------------------------------------------
HBRUSH miniat_image::createBkg(bool& needDestroy)
{
  needDestroy = false;
  if(isSelected())
    return (HBRUSH)GetStockObject(LTGRAY_BRUSH);
  return (HBRUSH)GetStockObject(WHITE_BRUSH);
}
//-------------------------------------------------------------------
extern void drawRect(HDC hdc, const PRect& r, COLORREF color, HPEN usePen);
//-------------------------------------------------------------------
void miniat_image::paintSel(HDC hdc)
{
  if(Selected) {
    baseClass::paintSel(hdc);
    PRect rC;
    GetClientRect(*this, rC);
    rC.Inflate(-1, -1);
    HPEN pen = CreatePen(PS_SOLID, 0, RGB(255, 255, 0));
    drawRect(hdc, rC, 0, pen);
    }
}
//-------------------------------------------------------------------
void miniat_image::loadImage(LPCTSTR basePath, LPCTSTR image)
{
  delete Bmp;
  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, basePath);
  appendPath(path, image);
  PBitmap tmp(path);
  HDC hdc = GetWindowDC(*this);
  pMDC_custom mdc_resized;
  HDC m = mdc_resized.getMdc(this, hdc);
  POINT pt = { 0, 0 };
  SIZE sz = getBlockSize();
  tmp.draw(m, pt, sz);
  Bmp = new PBitmap(m, pt, sz);
}
//-------------------------------------------------------------------
void miniat_image::evPaint(HDC hdc)
{
  SIZE sz = getBlockSize();
  if(Bmp) {
    POINT pt = { 0, 0 };
    Bmp->draw(hdc, pt, sz);
    return;
    }
  TCHAR t[256];
  wsprintf(t, _T("[ %02d ]"), getId() - ADD_ID + 1);
  SetTextAlign(hdc, TA_CENTER | TA_TOP);
  COLORREF old = SetTextColor(hdc, RGB(0, 0, 0));
  SetBkMode(hdc, TRANSPARENT);
  TextOut(hdc, sz.cx / 2, sz.cy / 2, t, (int)_tcslen(t));
  SetTextColor(hdc, old);
}

//-------------------------------------------------------------------
bool fv_miniat_images::create()
{
  PscrollInfo si(PscrollInfo::stVert, 5, 1);
  List = new container_miniat_images(this, 111, si);
  return baseClass::create();
}
