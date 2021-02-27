//-------------------- pBitmap.cpp ---------------------------
//-----------------------------------------------------------
#include "precHeader.h"
#include <stdlib.h>
#include <olectl.h>
//-----------------------------------------------------------
#include "pBitmap.h"
#include "p_file.h"
#include "p_util.h"
#include "simple_helper.h"
//-----------------------------------------------------------
inline HBITMAP load_resource_image(HINSTANCE hinst, LPCTSTR nameId)
{
  return (HBITMAP)LoadImage(hinst, nameId, IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
}
//-----------------------------------------------------------
inline HBITMAP load_resource_image(HINSTANCE hinst, uint Id)
{
  return (HBITMAP)LoadImage(hinst, MAKEINTRESOURCE(Id), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
}
//-----------------------------------------------------------
class initializeGdiPlus
{
  public:
    initializeGdiPlus() { if(!Initialized) { Initialized = Gdiplus::Ok == GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL); } }
    ~initializeGdiPlus() { if(Initialized) Gdiplus::GdiplusShutdown(gdiplusToken); }
  private:
   Gdiplus::GdiplusStartupInput gdiplusStartupInput;
   ULONG_PTR gdiplusToken;
   static bool Initialized;
};
bool initializeGdiPlus::Initialized = false;
static initializeGdiPlus Single;
//-----------------------------------------------------------
CGdiPlusBitmap::CGdiPlusBitmap(LPCWSTR pFile) : m_pBitmap(0) { Load(pFile); }
//-----------------------------------------------------------
CGdiPlusBitmap::CGdiPlusBitmap(LPCSTR pFile) : m_pBitmap(0)
{
  wchar_t t[_MAX_PATH];
  copyStrZ(t, pFile);
  Load(t);
}
//-----------------------------------------------------------
void CGdiPlusBitmap::Empty() { delete m_pBitmap; m_pBitmap = 0; }
//-----------------------------------------------------------
bool CGdiPlusBitmap::Load(LPCWSTR pFile)
{
  Empty();
  m_pBitmap = Gdiplus::Bitmap::FromFile(pFile, TRUE);
  return m_pBitmap && m_pBitmap->GetLastStatus() == Gdiplus::Ok;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
CGdiPlusBitmapResource::CGdiPlusBitmapResource(LPCTSTR pName, HMODULE hInst, LPCTSTR typeId) : m_hBuffer(0) { Load(pName, hInst, typeId); }
//-----------------------------------------------------------
CGdiPlusBitmapResource::CGdiPlusBitmapResource(UINT id, HMODULE hInst, LPCTSTR typeId) : m_hBuffer(0) { Load(id, hInst, typeId); }
//-----------------------------------------------------------
void CGdiPlusBitmapResource::Empty()
{
  baseClass::Empty();
  if (m_hBuffer) {
    ::GlobalUnlock(m_hBuffer);
    ::GlobalFree(m_hBuffer);
    m_hBuffer = 0;
    }
}
//-----------------------------------------------------------
bool CGdiPlusBitmapResource::Load(LPCTSTR pName, HMODULE hInst, LPCTSTR typeId)
{
  Empty();
  if(!typeId) {
    m_pBitmap = Gdiplus::Bitmap::FromResource(hInst, (LPCWSTR)pName);
    if(m_pBitmap) {
      if(m_pBitmap->GetLastStatus() == Gdiplus::Ok)
        return true;
      delete m_pBitmap;
      m_pBitmap = 0;
      }
    }
  if(!typeId)
    typeId = RT_RCDATA;
  HRSRC hResource = ::FindResource(hInst, pName, typeId);
  if (!hResource)
    return false;

  DWORD imageSize = ::SizeofResource(hInst, hResource);
  if (!imageSize)
    return false;

  const void* pResourceData = ::LockResource(::LoadResource(hInst, hResource));
  if (!pResourceData)
    return false;

  m_hBuffer  = ::GlobalAlloc(GMEM_MOVEABLE, imageSize);
  if (m_hBuffer) {
    void* pBuffer = ::GlobalLock(m_hBuffer);
    if (pBuffer) {
      CopyMemory(pBuffer, pResourceData, imageSize);

      IStream* pStream = NULL;
      if (::CreateStreamOnHGlobal(m_hBuffer, FALSE, &pStream) == S_OK) {
        m_pBitmap = Gdiplus::Bitmap::FromStream(pStream);
        pStream->Release();
        if (m_pBitmap) {
          if (m_pBitmap->GetLastStatus() == Gdiplus::Ok)
            return true;

          delete m_pBitmap;
          m_pBitmap = NULL;
          }
        }
      ::GlobalUnlock(m_hBuffer);
      }
    ::GlobalFree(m_hBuffer);
    m_hBuffer = NULL;
    }
  return false;
}
//-----------------------------------------------------------
bool CGdiPlusBitmapResource::Load(UINT id, HMODULE hInst, LPCTSTR typeId)
{
  return Load(MAKEINTRESOURCE(id), hInst, typeId);
}
//-----------------------------------------------------------
//-----------------------------------------------------------
PBitmap::PBitmap()  { InitializeVar(); }
//-----------------------------------------------------------
PBitmap::PBitmap(int id, HINSTANCE hinst, LPCTSTR typeId)
{
  InitializeVar();
  if(!typeId) {
    hBmp = load_resource_image(hinst, id);
    autoDelete = true;
    }
  else {
    manGdiPlusBitmap = new CGdiPlusBitmapResource();
    if(!manGdiPlusBitmap->Load(id, hinst, typeId)) {
      delete manGdiPlusBitmap;
      manGdiPlusBitmap = 0;
      }
    }
}
//-----------------------------------------------------------
PBitmap::PBitmap(LPCTSTR nameId, HINSTANCE hinst, LPCTSTR typeId)
{
  InitializeVar();
  if(!typeId) {
    hBmp = load_resource_image(hinst, nameId);
    autoDelete = true;
    }
  else {
    manGdiPlusBitmap = new CGdiPlusBitmapResource();
    if(!manGdiPlusBitmap->Load(nameId, hinst, typeId)) {
      delete manGdiPlusBitmap;
      manGdiPlusBitmap = 0;
      }
    }
}
//-----------------------------------------------------------
PBitmap::PBitmap(HBITMAP hbmp, bool autodelete) : hBmp(hbmp)
{
  InitializeVar(autodelete);
}
//-----------------------------------------------------------
PBitmap::PBitmap(p_gdiplusBitmap pbmp, bool owner)
{
  InitializeVar();
  p_gdiplusBitmap tmp = owner ? pbmp : pbmp->Clone(Gdiplus::Rect(0, 0, pbmp->GetWidth(), pbmp->GetHeight()), pbmp->GetPixelFormat());
  if(tmp) {
    manGdiPlusBitmap = new CGdiPlusBitmap();
    manGdiPlusBitmap->m_pBitmap = tmp;
    if(ManGif) {
      HWND Hwnd = ManGif->Hwnd;
      delete ManGif;
      ManGif = 0;
      addGifControl(Hwnd);
      }
    }
}
//-----------------------------------------------------------
PBitmap::PBitmap(LPCWSTR filename)
{
  InitializeVar();
  manGdiPlusBitmap = new CGdiPlusBitmap();
  if(!manGdiPlusBitmap->Load(filename)) {
    delete manGdiPlusBitmap;
    manGdiPlusBitmap = 0;
    }
}
//-----------------------------------------------------------
PBitmap::PBitmap(LPCSTR filename)
{
  InitializeVar();
  manGdiPlusBitmap = new CGdiPlusBitmap();
  wchar_t t[_MAX_PATH];
  copyStrZ(t, filename);
  if(!manGdiPlusBitmap->Load(t)) {
    delete manGdiPlusBitmap;
    manGdiPlusBitmap = 0;
    }
}
//-----------------------------------------------------------
PBitmap::PBitmap(const PBitmap& pbmp)
{
  InitializeVar();
  if(pbmp.manGdiPlusBitmap) {
    p_gdiplusBitmap p_bmp = pbmp.manGdiPlusBitmap->m_pBitmap;
    p_gdiplusBitmap tmp =  p_bmp->Clone(Gdiplus::Rect(0, 0, p_bmp->GetWidth(), p_bmp->GetHeight()), p_bmp->GetPixelFormat());
    if(tmp) {
      manGdiPlusBitmap = new CGdiPlusBitmap();
      manGdiPlusBitmap->m_pBitmap = tmp;
      }
    }
  else if(pbmp.hBmp) {
    hBmp = pbmp.hBmp;
    createFromHBitmap();
    hBmp = 0;
    }

  qualityOnStretch = pbmp.qualityOnStretch;
  smooth = pbmp.smooth;
}
//-----------------------------------------------------------
PBitmap::PBitmap(HDC hdc, const POINT& pt, const SIZE& sz)
{
  InitializeVar(true);
  hBmp = CreateCompatibleBitmap(hdc, sz.cx, sz.cy);
  HDC mdc = CreateCompatibleDC(hdc);
  HGDIOBJ oldObj = SelectObject(mdc, hBmp);
  BitBlt(mdc, 0, 0, sz.cx, sz.cy, hdc, pt.x, pt.y, SRCCOPY);
  SelectObject(mdc, oldObj);
  DeleteDC(mdc);
}
//-----------------------------------------------------------
PBitmap::~PBitmap()
{
  delete ManGif;
  if(manGdiPlusBitmap)
    delete manGdiPlusBitmap;
  if(autoDelete && hBmp)
    DeleteObject(hBmp);
}
//-----------------------------------------------------------
void PBitmap::InitializeVar(bool auto_delete)
{
  manGdiPlusBitmap = 0;
  hBmp = 0;
  autoDelete = auto_delete;
  qualityOnStretch = true;
  smooth = false;
  ManGif = 0;
}
//-----------------------------------------------------------
PBitmap::operator HBITMAP() const
{
  HBITMAP tmpHbmp =  hBmp;
  if(!tmpHbmp && manGdiPlusBitmap) {
    autoDelete = true;
    Gdiplus::Color color;
    manGdiPlusBitmap->m_pBitmap->GetHBITMAP(color, &tmpHbmp);
    hBmp = tmpHbmp;
    }
  return tmpHbmp;
}
//-----------------------------------------------------------
void PBitmap::chgHandle(HBITMAP new_hBmp)
{
  if(hBmp == new_hBmp)
    return;
  if(hBmp && autoDelete)
    DeleteObject(hBmp);
  hBmp = new_hBmp;
  autoDelete = true;
  delete manGdiPlusBitmap;
  manGdiPlusBitmap = 0;
}
//-----------------------------------------------------------
const PBitmap& PBitmap::operator =(const PBitmap& bmp)
{
  if(&bmp == this)
    return *this;
  if(hBmp && autoDelete)
    DeleteObject(hBmp);
  qualityOnStretch = bmp.qualityOnStretch;
  smooth = bmp.smooth;
  hBmp = 0;
  autoDelete = false;
  if(bmp.manGdiPlusBitmap) {
    p_gdiplusBitmap p_bmp = bmp.manGdiPlusBitmap->m_pBitmap;
    p_gdiplusBitmap tmp =  p_bmp->Clone(Gdiplus::Rect(0, 0, p_bmp->GetWidth(), p_bmp->GetHeight()), p_bmp->GetPixelFormat());
    if(tmp) {
      manGdiPlusBitmap = new CGdiPlusBitmap();
      manGdiPlusBitmap->m_pBitmap = tmp;
      }
    }
  else if(bmp.hBmp) {
    hBmp = bmp.hBmp;
    createFromHBitmap();
    hBmp = 0;
    }
 return *this;
}
//-----------------------------------------------------------
bool PBitmap::setSmooth(bool set)
{
  bool old = smooth;
  smooth = set;
  return old;
}
//-----------------------------------------------------------
bool PBitmap::setQualityOnStretch(bool set)
{
  bool old = qualityOnStretch;
  qualityOnStretch = set;
  return old;
}
//-----------------------------------------------------------
bool PBitmap::clone(p_gdiplusBitmap& target)
{
  if(manGdiPlusBitmap) {
    p_gdiplusBitmap pbmp = manGdiPlusBitmap->m_pBitmap;
    p_gdiplusBitmap tmp =  pbmp->Clone(Gdiplus::Rect(0, 0, pbmp->GetWidth(), pbmp->GetHeight()), pbmp->GetPixelFormat());
    if(tmp) {
      target = tmp;
      return true;
      }
    }
  return false;
}
//-----------------------------------------------------------
HBITMAP PBitmap::clone(HDC hDc)  const
{
  if(hBmp) {
    HBITMAP clonedBmp = (HBITMAP)CopyImage(
      hBmp,             // handle to the image to copy
      IMAGE_BITMAP,     // type of image to copy
      0,                // desired width of new image
      0,                // desired height of new image
      LR_CREATEDIBSECTION  // copy flags
      );
    return clonedBmp;
    }
  if(!manGdiPlusBitmap)
    return 0;
  p_gdiplusBitmap m_pBitmap = manGdiPlusBitmap->m_pBitmap;
  HBITMAP tmpHbmp = 0;
  m_pBitmap->GetHBITMAP(Gdiplus::Color(), &tmpHbmp);
  HBITMAP clonedBmp = (HBITMAP)CopyImage(
    tmpHbmp,             // handle to the image to copy
    IMAGE_BITMAP,     // type of image to copy
    0,                // desired width of new image
    0,                // desired height of new image
    LR_CREATEDIBSECTION  // copy flags
    );
  DeleteObject(tmpHbmp);
  return clonedBmp;
}
//-----------------------------------------------------------
bool PBitmap::createFromHBitmap() const
{
  if(!hBmp)
    return false;
  HPALETTE hpal = (HPALETTE)GetStockObject(DEFAULT_PALETTE);
  p_gdiplusBitmap tmp = Gdiplus::Bitmap::FromHBITMAP(hBmp, hpal);
  if(tmp) {
    manGdiPlusBitmap = new CGdiPlusBitmap();
    manGdiPlusBitmap->m_pBitmap = tmp;
    return true;
    }
  return false;
}
//-----------------------------------------------------------
bool PBitmap::drawTiled(HDC hdc, const PRect& target) const
{
  if(!manGdiPlusBitmap)
    if(!createFromHBitmap())
      return false;

  Gdiplus::Graphics graphics(hdc);
  return drawTiled(graphics, target);
}
//-----------------------------------------------------------
bool PBitmap::drawTiled(Gdiplus::Graphics& graphics, const PRect& target) const
{
  if(!manGdiPlusBitmap)
    if(!createFromHBitmap())
      return false;
  if(ManGif)
    ManGif->evPaint(target);
  p_gdiplusBitmap m_pBitmap = manGdiPlusBitmap->m_pBitmap;
  SIZE sz = { m_pBitmap->GetWidth(), m_pBitmap->GetHeight() };
  POINT pt = { target.left, target.top };
  Gdiplus::Status result = Gdiplus::Ok;
  while((pt.y < target.bottom) && result == Gdiplus::Ok) {
    POINT ptt = pt;
    while(ptt.x < target.right) {
      result = graphics.DrawImage(m_pBitmap, ptt.x, ptt.y);
      if(result != Gdiplus::Ok)
        break;
      ptt.x += sz.cx;
      }
    pt.y += sz.cy;
    }
  return result == Gdiplus::Ok;
}
//-----------------------------------------------------------
bool PBitmap::draw(HDC hdc, const POINT& pt) const
{
  if(!manGdiPlusBitmap)
    if(!createFromHBitmap())
      return false;
  Gdiplus::Graphics graphics(hdc);
  return draw(graphics, pt);
}
//-----------------------------------------------------------
bool PBitmap::draw(Gdiplus::Graphics& graphics, const POINT& pt) const
{
  if(!manGdiPlusBitmap)
    if(!createFromHBitmap())
      return false;
  if(ManGif) {
    SIZE sz = getSize();
    PRect r(0, 0, sz.cx, sz.cy);
    r.MoveTo(pt.x, pt.y);
    ManGif->evPaint(r);
    }
  Gdiplus::Status result = graphics.DrawImage(manGdiPlusBitmap->m_pBitmap, pt.x, pt.y);
  return result == Gdiplus::Ok;
}
//-----------------------------------------------------------
SIZE PBitmap::getSize() const
{
  SIZE sz = { 0, 0 };
  if(!manGdiPlusBitmap)
    if(!createFromHBitmap())
      return sz;
  p_gdiplusBitmap m_pBitmap = manGdiPlusBitmap->m_pBitmap;
  sz.cx = m_pBitmap->GetWidth();
  sz.cy = m_pBitmap->GetHeight();
  return sz;
}
//-----------------------------------------------------------
bool PBitmap::draw(HDC hdc, const POINT& pt, const SIZE& sz) const
{
  if(!manGdiPlusBitmap)
    if(!createFromHBitmap())
      return false;
  Gdiplus::Graphics graphics(hdc);
  return draw(graphics, pt, sz);
}
//-----------------------------------------------------------
bool PBitmap::draw(Gdiplus::Graphics& graphics, const POINT& pt, const SIZE& sz) const
{
  if(!manGdiPlusBitmap)
    if(!createFromHBitmap())
      return false;
  SIZE szBmp = getSize();
  PRect s(0, 0, szBmp.cx, szBmp.cy);
  PRect t(0, 0, sz.cx, sz.cy);
  t.MoveTo(pt.x, pt.y);
  if(ManGif)
    ManGif->evPaint(t);
  return drawSlice(graphics, t, s);
}
//-----------------------------------------------------------
bool PBitmap::drawSlice(HDC hdc, const PRect& target, const PRect& source) const
{
  if(!manGdiPlusBitmap)
    if(!createFromHBitmap())
      return false;
  Gdiplus::Graphics graphics(hdc);
  return drawSlice(graphics, target, source);
}
//-----------------------------------------------------------
bool PBitmap::drawSlice(Gdiplus::Graphics& graphics, const PRect& target, const PRect& source) const
{
  if(!manGdiPlusBitmap)
    if(!createFromHBitmap())
      return false;
  Gdiplus::Rect r(target.left, target.top, target.Width(), target.Height());
  if(ManGif)
    ManGif->evPaint(target);
  Gdiplus::ImageAttributes imAtt;
  if(!smooth)
    imAtt.SetWrapMode(Gdiplus::WrapModeTileFlipXY);
  if(qualityOnStretch)
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
  return Gdiplus::Ok == graphics.DrawImage(manGdiPlusBitmap->m_pBitmap, r, source.left, source.top, source.Width(), source.Height(), Gdiplus::UnitPixel, &imAtt);
}
//-----------------------------------------------------------
bool PBitmap::drawTransp(HDC hdc, const PRect& target) const
{
  if(!manGdiPlusBitmap)
    if(!createFromHBitmap())
      return false;
  Gdiplus::Graphics graphics(hdc);
  return drawTransp(graphics, target);
}
//-----------------------------------------------------------
bool PBitmap::drawTransp(Gdiplus::Graphics& graphics, const PRect& target) const
{
  if(!manGdiPlusBitmap)
    if(!createFromHBitmap())
      return false;
  p_gdiplusBitmap pBmp =  manGdiPlusBitmap->m_pBitmap;
#if 1
  bool needTransp = !(Gdiplus::ImageFlagsHasAlpha & pBmp->GetFlags());
#else
  Gdiplus::PixelFormat pxf = pBmp->GetPixelFormat();
  bool needTransp = true;
  switch(pxf) {
    case PixelFormat16bppARGB1555:
    case PixelFormat32bppARGB:
    case PixelFormat32bppPARGB:
    case PixelFormat64bppARGB:
    case PixelFormat64bppPARGB:
      needTransp = false;
      break;
    }
#endif
  if(ManGif)
    ManGif->evPaint(target);
  Gdiplus::Rect r(target.left, target.top, target.Width(), target.Height());
  if(needTransp) {
    Gdiplus::Color color;
    pBmp->GetPixel(0, 0, &color);

    Gdiplus::ImageAttributes imAtt;
    imAtt.SetColorKey( color, color, Gdiplus::ColorAdjustTypeBitmap);
    return Gdiplus::Ok == graphics.DrawImage(pBmp, r, 0, 0, pBmp->GetWidth(), pBmp->GetHeight(), Gdiplus::UnitPixel, &imAtt);
    }
  return Gdiplus::Ok == graphics.DrawImage(pBmp, r);
}
//----------------------------------------------------------------------------
void PBitmap::rotateInside(PBitmap* bmp, Gdiplus::RotateFlipType type)
{
  bmp->setQualityOnStretch(true);
  const p_gdiplusBitmap m_pBitmap = bmp->getManBmp();
  m_pBitmap->RotateFlip(type);
}
//----------------------------------------------------------------------------
PBitmap* PBitmap::rotate(const PBitmap* bmp, Gdiplus::RotateFlipType type)
{
  PBitmap* bmp2 = new PBitmap(*bmp);
  rotateInside(bmp2, type);
  return bmp2;
}
//----------------------------------------------------------------------------
PBitmap* PBitmap::resizeBmp(const PBitmap* bmp, double perc)
{
  SIZE sz = bmp->getSize();
  int n_width = ROUND_REAL(sz.cx * perc);
  int n_height = ROUND_REAL(sz.cy * perc);

  const p_gdiplusBitmap m_pBitmap = bmp->getManBmp();
  Gdiplus::Bitmap* newBitmap = new Gdiplus::Bitmap(n_width, n_height, m_pBitmap->GetPixelFormat());
  Gdiplus::Graphics graphics(newBitmap);
  graphics.DrawImage(m_pBitmap, 0, 0, n_width, n_height);
  PBitmap* nb = new PBitmap(newBitmap);
//  delete newBitmap;
  return nb;
}
//----------------------------------------------------------------------------
static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
   UINT  num = 0;          // number of image encoders
   UINT  size = 0;         // size of the image encoder array in bytes

   Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

   Gdiplus::GetImageEncodersSize(&num, &size);
   if(size == 0)
      return -1;  // Failure

   pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
   if(pImageCodecInfo == NULL)
      return -1;  // Failure

   Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

   for(UINT j = 0; j < num; ++j)
   {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }
   }

   free(pImageCodecInfo);
   return -1;  // Failure
}
//----------------------------------------------------------------------------
static int GetEncoderClsidByName(LPCWSTR path, CLSID* pClsid)
{
  struct dual { LPCWSTR ext; LPCWSTR format; }
    d[] = {
      { L".bmp", L"image/bmp"},
      { L".jpg", L"image/jpeg"},
      { L".gif", L"image/gif"},
      { L".tif", L"image/tiff"},
      { L".png", L"image/png"},
      };
  int len = wcslen(path);
  path += len - 4;
  for(uint i = 0; i < SIZE_A(d); ++i) {
    if(!_wcsicmp(d[i].ext, path))
      return GetEncoderClsid(d[i].format, pClsid);
    }
  return -1;
}
//-----------------------------------------------------------
bool PBitmap::saveAs(LPCSTR filename, uint quality)
{
  wchar_t path[_MAX_PATH];
  copyStrZ(path, filename);
  return saveAs(path, quality);
}
//-----------------------------------------------------------
bool PBitmap::saveAs(LPCWSTR filename, uint quality)
{
  if(!manGdiPlusBitmap)
    if(!createFromHBitmap())
      return false;

  CLSID encoderClsid;
  INT result = GetEncoderClsidByName(filename, &encoderClsid);
  if(result < 0)
    return false;
  bool success = false;
  const p_gdiplusBitmap m_pBitmap = getManBmp();
  Gdiplus::EncoderParameters encoderParameters;
  encoderParameters.Count = 1;
  encoderParameters.Parameter[0].Guid = Gdiplus::EncoderQuality;
  encoderParameters.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
  encoderParameters.Parameter[0].NumberOfValues = 1;
  encoderParameters.Parameter[0].Value = &quality;

  success = Gdiplus::Ok == m_pBitmap->Save(filename, &encoderClsid, &encoderParameters);
  return success;
}
//-----------------------------------------------------------
bool PBitmap::addGifControl(PWin* owner)
{
  return addGifControl(*owner);
}
//-----------------------------------------------------------
bool PBitmap::addGifControl(HWND owner)
{
  if(!manGdiPlusBitmap && !createFromHBitmap() || ManGif)
    return false;

  p_gdiplusBitmap m_pBitmap = manGdiPlusBitmap->m_pBitmap;

  UINT count = m_pBitmap->GetFrameDimensionsCount();
  if(!count)
    return false;
  GUID* pDimensionIDs = new GUID[count];

  m_pBitmap->GetFrameDimensionsList(pDimensionIDs, count);
  int frameCount = m_pBitmap->GetFrameCount(&pDimensionIDs[0]);
  delete []pDimensionIDs;
  if(frameCount <= 1)
    return false;

  uint TotalBuffer = m_pBitmap->GetPropertyItemSize(PropertyTagFrameDelay);
  Gdiplus::PropertyItem* pItem = (Gdiplus::PropertyItem*) new BYTE[TotalBuffer];
  m_pBitmap->GetPropertyItem(PropertyTagFrameDelay, TotalBuffer, pItem);

  PVect<uint> delayTime;
  delayTime.setDim(frameCount);

  for(int i = 0; i < frameCount; ++i) {
    uint tm = ((uint*)pItem[0].value)[i];
    if(tm <= 1)
      tm = 10;
    delayTime[i] = tm * 10;
    }
  delete []pItem;

  manageGif *mg = new manageGif(this, owner, delayTime);
  if(!mg->run()) {
    delete mg;
    return false;
    }
  ManGif = mg;
  return true;
}
//-----------------------------------------------------------
void PBitmap::removeGifControl()
{
  delete ManGif;
  ManGif = 0;
}
//-----------------------------------------------------------
manageGif::~manageGif()
{
  SetEvent(hEvClose);
  WaitForSingleObject(hThread, INFINITE);
  CloseHandle(hEvClose);
  CloseHandle(hThread);
}
//-----------------------------------------------------------
bool manageGif::run()
{
  hEvClose = CreateEvent(0, TRUE, 0, 0);
  DWORD idThread;
  hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)AnimProc, this, 0, &idThread);
  if(!hThread)
    return false;
  return true;
}
//-----------------------------------------------------------
void manageGif::evPaint(const PRect& r)
{
  criticalLock CL(CS);
  Rect = r;
  if(byThread) {
    byThread = false;
    GUID Guid = Gdiplus::FrameDimensionTime;
    Owner->manGdiPlusBitmap->m_pBitmap->SelectActiveFrame(&Guid, curr);
    curr = ++curr % delayTime.getElem();
    }
}
//-----------------------------------------------------------
unsigned FAR PASCAL AnimProc(void* cls)
{
  manageGif* mg = (manageGif*)cls;
  DWORD timeout = 0;
  bool terminate = false;
  while(!terminate) {
    DWORD result = WaitForSingleObject(mg->hEvClose, timeout);
    switch(result) {
      case WAIT_TIMEOUT:
        do {
          criticalLock CL(mg->CS);
          timeout = mg->delayTime[mg->curr];
          if(!mg->Rect.Width() || !mg->Rect.Height())
            break;
          mg->byThread = true;
          InvalidateRect(mg->Hwnd, mg->Rect, 0);
          } while(false);
        break;
      default:
        terminate = true;
        break;
      }
    }
  return EXIT_SUCCESS;
}
