//--------------- printScreen.cpp ---------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "printScreen.h"
#include "PrintImage.h"
#include "p_file.h"
#include "pBitmap.h"
#include "sizer.h"
//----------------------------------------------------------------------------
#define USE_GDIPLUS_4_SAVE
//----------------------------------------------------------------------------
#ifdef USE_GDIPLUS_4_SAVE
//----------------------------------------------------------------------------
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
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
int GetEncoderClsidByName(LPCTSTR path, CLSID* pClsid)
{
  struct dual { LPCTSTR ext; wchar_t* format; }
    d[] = {
      {_T(".bmp"), L"image/bmp"},
      {_T(".jpg"), L"image/jpeg"},
      {_T(".gif"), L"image/gif"},
      {_T(".tif"), L"image/tiff"},
      {_T(".png"), L"image/png"},
      };
  int len = _tcslen(path);
  path += len - 4;
  for(uint i = 0; i < SIZE_A(d); ++i) {
    if(!_tcsicmp(d[i].ext, path))
      return GetEncoderClsid(d[i].format, pClsid);
    }
  return -1;
}
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
#include "jpegfile.h"
//----------------------------------------------------------------------------
static void moveReverse(LPBYTE lpBits, int width, int height)
{
  int lenRow = (width * 3 + 3) & ~3;
  LPBYTE tmp = new BYTE[lenRow];
  LPBYTE bottom = lpBits + lenRow * (height - 1);

  LPBYTE top = lpBits;
  for(int y = 0; y < height / 2; ++y) {
    memcpy(tmp, top, lenRow);
    memcpy(top, bottom, lenRow);
    memcpy(bottom, tmp, lenRow);
    bottom -= lenRow;
    top += lenRow;
    }
  delete []tmp;
}
//-----------------------------------------------------------
bool isJpeg(LPCTSTR name)
{
  int len = _tcslen(name);
  return !_tcsicmp(_T(".jpg"), name + len - 4);
}
#endif
//----------------------------------------------------------------------------
extern void forceCloseAlarm(bool alsoModal);
//----------------------------------------------------------------------------
static bool saveImageScreen(PWin* owner, bool fullscreen, LPCTSTR filename = 0)
{
  // se richiamata da script potrebbe essere aperta, manualmente, la finestra degli allarmi, ne forziamo la chiusura
  forceCloseAlarm(!fullscreen);
  // se c'è la richiesta di conferma o la psw, resta aperta la finestrina
  // aspettiamo almeno un messaggio
  ::WaitMessage();
  while(owner->getParent())
    owner = owner->getParent();
  HDC hdc = GetDC(NULL);
  SIZE sz = sizer::getDim();
  PRect r;
  GetWindowRect(*owner, r);
  POINT pt = { r.left, r.top };
  if(fullscreen) {
    pt.x = 0;
    pt.y = 0;
    sz.cx = GetSystemMetrics(SM_CXSCREEN);
    sz.cy = GetSystemMetrics(SM_CYSCREEN);
    }
  PBitmap pbmp(hdc, pt, sz);

#ifdef USE_GDIPLUS_4_SAVE
  if(!filename)
    filename = DEF_BMP_NAME_4_PRINT;
  bool success = pbmp.saveAs(filename);
  ReleaseDC(NULL, hdc);
  return success;

#else
  TCHAR path[_MAX_PATH];
  bool asJpeg = false;
  if(filename) {
    asJpeg = isJpeg(filename);
    _tcscpy_s(path, filename);
    }
  else
    _tcscpy_s(path, DEF_BMP_NAME_4_PRINT);
  BITMAP bmp;
  HBITMAP tmpHbmp = pbmp;
  GetObject((HGDIOBJ)tmpHbmp, sizeof(bmp), &bmp);

  BITMAPFILEHEADER bmfh;
  DWORD image_length = ((bmp.bmWidth * 3 + 3) & ~3) * bmp.bmHeight;
  memcpy (&bmfh.bfType, "BM", 2);
  bmfh.bfReserved1 = 0;
  bmfh.bfReserved2 = 0;
  bmfh.bfSize = LEN_BMPFILEHEADER + sizeof(BITMAPINFOHEADER) + image_length;
  bmfh.bfOffBits = LEN_BMPFILEHEADER + sizeof(BITMAPINFOHEADER);

  P_File* f = 0;
  bool success = false;
  LPBYTE pBits = 0;
  do {
    if(!asJpeg) {
      f = new P_File(path, P_CREAT);
      if(!f->P_open())
        break;
      }
    if(!asJpeg && LEN_BMPFILEHEADER != f->P_write(&bmfh, LEN_BMPFILEHEADER))
      break;

    BITMAPINFOHEADER bmih;

    memset(&bmih, 0, sizeof(bmih));
    bmih.biSize = sizeof(BITMAPINFOHEADER);
    bmih.biWidth = bmp.bmWidth;
    bmih.biHeight = bmp.bmHeight;
    bmih.biPlanes = bmp.bmPlanes;
    bmih.biBitCount =  24;//(WORD)(bmp.bmPlanes * bmp.bmBitsPixel);
    bmih.biCompression = BI_RGB;
    bmih.biSizeImage = image_length;

    if(!asJpeg && sizeof(bmih) != f->P_write(&bmih, sizeof(bmih)))
      break;
    pBits = new BYTE[image_length + 2];
    HBITMAP tmpHbmp = pbmp;
    GetDIBits(hdc, tmpHbmp, 0L, (DWORD)bmih.biHeight, pBits, (LPBITMAPINFO)&bmih, (DWORD)DIB_RGB_COLORS);

    if(!asJpeg)
      success = image_length == f->P_write(pBits, image_length);
    else {
      moveReverse(pBits, bmih.biWidth, bmih.biHeight);
      LPBYTE tmp = JpegFile::RGBFromDWORDAligned(pBits,
                      bmih.biWidth,
                      (bmih.biWidth * 3 + 3) & ~3,
                      bmih.biHeight);
      JpegFile::BGRFromRGB(tmp, bmih.biWidth, bmih.biHeight);
      char f[_MAX_PATH];
      copyStr(f, path, _tcslen(path) + 1);
      success = toBool(JpegFile::RGBToJpegFile(f,
            tmp,
            bmih.biWidth,
            bmih.biHeight,
            TRUE,
            75));
      delete []tmp;
      }
    } while(false);
  delete []pBits;
  delete f;
  ReleaseDC(NULL, hdc);
  return success;
#endif
}
//----------------------------------------------------------------------------
int checkExistNumb(LPCTSTR path, LPCTSTR ext)
{
  int initCode = 1;
  TCHAR tmp[_MAX_PATH];
  do {
    wsprintf(tmp, _T("%s_%03d.%s"), path,  initCode, ext);
    if(!P_File::P_exist(tmp))
      return initCode;
    ++initCode;
    } while(initCode < 999);
  return 1;
}
//----------------------------------------------------------------------------
static void makeSaveDir(LPCTSTR file)
{
  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, file);
  int len = _tcslen(path);
  for(int i = len - 1; i > 0; --i) {
    if(_T('\\') == path[i] || _T('/') == path[i]) {
      path[i] = 0;
      createDirectoryRecurse(path);
      break;
      }
    }
}
//----------------------------------------------------------------------------
bool saveScreen(PWin* owner, LPCTSTR filename)
{
  if(!filename || !*filename) {
    filename = getString(ADDR_SAVE_SCREEN);
    filename = findNextParamTrim(filename, 7);
    if(!filename)
      return false;
    }
  makeSaveDir(filename);
#ifdef USE_GDIPLUS_4_SAVE
  TCHAR ext[20] = _T("");
  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, filename);
  int len = _tcslen(path);
  for(int i = len - 1; i > 0; --i) {
    if(_T('.') == path[i]) {
      copyStrZ(ext, path + i);
      path[i] = 0;
      break;
      }
    }
#else
  bool asJpeg = isJpeg(filename);
  LPCTSTR ext = asJpeg  ? _T("jpg") : _T("bmp");
  TCHAR path[_MAX_PATH];
  _tcscpy_s(path, filename);
  int len = _tcslen(path);
  for(int i = len - 1; i > 0; --i) {
    if(_T('.') == path[i]) {
      path[i] = 0;
      break;
      }
    }
#endif
  LPCTSTR p = getString(SAVE_SCREEN_ONLY_ONE_FILE);
  if(p && 1 == _ttoi(p)) {
    LPTSTR pp = path + _tcslen(path);
    wsprintf(pp, _T(".%s"), ext);
    }
  else {
    int initCode = checkExistNumb(path, ext);
    LPTSTR p = path + _tcslen(path);
    wsprintf(p, _T("_%03d.%s"), initCode, ext);
    }
  p = findNextParamTrim(p);
  bool fullscreen = p && _ttoi(p);
  return saveImageScreen(owner, fullscreen, path);
}
//----------------------------------------------------------------------------
bool printScreen(PWin* owner, bool preview, bool showDialog, bool fullscreen)
{
  if(!saveImageScreen(owner, fullscreen))
    return false;
  svPrinter printer(owner);
  printImage pi(owner, &printer, _T("~file_4_print.tmp"));
  if(!pi.beginDoc(showDialog))
    return false;

  if(preview)
    pi.Preview();
  else
    pi.Print();
  return true;
}
