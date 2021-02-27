//----- p_checkKey.cpp -------------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include <shlobj.h>
#include "p_checkKey.h"
#include "p_param_v.h"
#include "p_file.h"
#include "p_util.h"
#include "p_date.h"
#include <winioctl.h>
#include "ntddndis.h"        // This defines the IOCTL constants.
//---------------------------------------------------------------------
enum eSerial { esNone, esNet, esDisk };
//---------------------------------------------------------------------
#define WITH_ADMIN_RIGHT
//---------------------------------------------------------------------
#ifdef WITH_ADMIN_RIGHT
static void addSerialHDD_withAdminRight(HANDLE hDisk, LPSTR serialNumber);
//---------------------------------------------------------------------
#define  DFP_GET_VERSION          0x00074080
#define  DFP_RECEIVE_DRIVE_DATA   0x0007c088
#define  IDENTIFY_BUFFER_SIZE  512
//---------------------------------------------------------------------
#if 1
#pragma pack(push, 1)
//---------------------------------------------------------------------
   //  GETVERSIONOUTPARAMS contains the data returned from the
   //  Get Driver Version function.
typedef struct _GETVERSIONOUTPARAMS
{
   BYTE bVersion;      // Binary driver version.
   BYTE bRevision;     // Binary driver revision.
   BYTE bReserved;     // Not used.
   BYTE bIDEDeviceMap; // Bit map of IDE devices.
   DWORD fCapabilities; // Bit mask of driver capabilities.
   DWORD dwReserved[4]; // For future use.
} GETVERSIONOUTPARAMS, *PGETVERSIONOUTPARAMS, *LPGETVERSIONOUTPARAMS;
//---------------------------------------------------------------------
#pragma pack(pop)
#endif
//---------------------------------------------------------------------
   //  Valid values for the bCommandReg member of IDEREGS.
#define  IDE_ATAPI_IDENTIFY  0xA1  //  Returns ID sector for ATAPI.
#define  IDE_ATA_IDENTIFY    0xEC  //  Returns ID sector for ATA.
//---------------------------------------------------------------------
   // DoIDENTIFY
   // FUNCTION: Send an IDENTIFY command to the drive
   // bDriveNum = 0-3
   // bIDCmd = IDE_ATA_IDENTIFY or IDE_ATAPI_IDENTIFY
static BOOL DoIDENTIFY (HANDLE hPhysicalDriveIOCTL, PSENDCMDINPARAMS pSCIP,
                 PSENDCMDOUTPARAMS pSCOP, BYTE bIDCmd, BYTE bDriveNum,
                 LPDWORD lpcbBytesReturned)
{
      // Set up data structures for IDENTIFY command.
   pSCIP -> cBufferSize = IDENTIFY_BUFFER_SIZE;
   pSCIP -> irDriveRegs.bFeaturesReg = 0;
   pSCIP -> irDriveRegs.bSectorCountReg = 1;
   //pSCIP -> irDriveRegs.bSectorNumberReg = 1;
   pSCIP -> irDriveRegs.bCylLowReg = 0;
   pSCIP -> irDriveRegs.bCylHighReg = 0;

      // Compute the drive number.
   pSCIP -> irDriveRegs.bDriveHeadReg = 0xA0 | ((bDriveNum & 1) << 4);

      // The command can either be IDE identify or ATAPI identify.
   pSCIP -> irDriveRegs.bCommandReg = bIDCmd;
   pSCIP -> bDriveNumber = bDriveNum;
   pSCIP -> cBufferSize = IDENTIFY_BUFFER_SIZE;

   return ( DeviceIoControl (hPhysicalDriveIOCTL, DFP_RECEIVE_DRIVE_DATA,
               (LPVOID) pSCIP,
               sizeof(SENDCMDINPARAMS) - 1,
               (LPVOID) pSCOP,
               sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1,
               lpcbBytesReturned, NULL) );
}
//---------------------------------------------------------------------
static char *ConvertToString (DWORD diskdata [256],
           int firstIndex,
           int lastIndex,
           char* buf)
{
  int index = 0;
  int position = 0;

  //  each integer has two characters stored in it backwards
  for (index = firstIndex; index <= lastIndex; index++)   {
         //  get high byte for 1st character
    buf [position++] = (char) (diskdata [index] / 256);

         //  get low byte for 2nd character
    buf [position++] = (char) (diskdata [index] % 256);
    }
      //  end the string
  buf[position] = '\0';

  trim(lTrim(buf));
  return buf;
}
//---------------------------------------------------------------------
static void addSerialHDD_withAdminRight(HANDLE hDisk, LPSTR serialNumber)
{
  GETVERSIONOUTPARAMS VersionParams;
  DWORD cbBytesReturned = 0;

  // Get the version, etc of PhysicalDrive IOCTL
  memset((void*)&VersionParams, 0, sizeof(VersionParams));
  if(DeviceIoControl (hDisk, DFP_GET_VERSION, NULL, 0,
                   &VersionParams, sizeof(VersionParams),
                   &cbBytesReturned, NULL)) {
    // If there is a IDE device at number "i" issue commands to the device
    if(VersionParams.bIDEDeviceMap > 0) {
      BYTE bIDCmd = 0;   // IDE or ATAPI IDENTIFY cmd
      SENDCMDINPARAMS scip;
            //SENDCMDOUTPARAMS OutCmd;

      BYTE IdOutCmd [sizeof (SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1];
      BYTE drive = 0;
      // Now, get the ID sector for all IDE devices in the system.
      // If the device is ATAPI use the IDE_ATAPI_IDENTIFY command,
      // otherwise use the IDE_ATA_IDENTIFY command
      bIDCmd = (VersionParams.bIDEDeviceMap >> drive & 0x10) ? IDE_ATAPI_IDENTIFY : IDE_ATA_IDENTIFY;
      memset(&scip, 0, sizeof(scip));
      memset(IdOutCmd, 0, sizeof(IdOutCmd));

      if(DoIDENTIFY (hDisk, &scip, (PSENDCMDOUTPARAMS)&IdOutCmd, (BYTE)bIDCmd, (BYTE)drive, &cbBytesReturned)) {
        DWORD diskdata [256];
        int ijk = 0;
        USHORT *pIdSector = (USHORT *)((PSENDCMDOUTPARAMS) IdOutCmd) -> bBuffer;
        for(ijk = 0; ijk < 256; ijk++)
          diskdata [ijk] = pIdSector [ijk];
        ConvertToString (diskdata, 10, 19, serialNumber);
        }
      }
    }
}
#endif
//---------------------------------------------------------------------
//---------------------------------------------------------------------
#define IOCTL_STORAGE_QUERY_PROPERTY   CTL_CODE(IOCTL_STORAGE_BASE, 0x0500, METHOD_BUFFERED, FILE_ANY_ACCESS)
//---------------------------------------------------------------------
  //  function to decode the serial numbers of IDE hard drives
  //  using the IOCTL_STORAGE_QUERY_PROPERTY command
char * flipAndCodeBytes (const char * str, int pos, int flip, char * buf)
{
  buf [0] = '\0';
  if (pos <= 0)
     return buf;

  int k = 0;
  char p = 0;

  // First try to gather all characters representing hex digits only.
  bool success = true;
  for(int i = pos; str[i] != '\0'; ++i)  {
    char c = tolower(str[i]);

    if (isspace(c))
      c = '0';

    ++p;
    buf[k] <<= 4;

    if(c >= '0' && c <= '9')
      buf[k] |= (unsigned char) (c - '0');
    else if(c >= 'a' && c <= 'f')
      buf[k] |= (unsigned char) (c - 'a' + 10);
    else   {
      success = false;
      break;
      }
    if(p == 2)  {
      if (buf[k] != '\0' && ! isprint(buf[k]))   {
        success = false;
        break;
        }
      ++k;
      p = 0;
      buf[k] = 0;
      }
    }

  if(!success)  {
    // There are non-digit characters, gather them as is.
    success = true;
    k = 0;
    for(int i = pos; str[i] != '\0'; ++i) {
      char c = str[i];
      if( ! isprint(c)) {
        success = false;
        break;
        }
      buf[k++] = c;
      }
    }

  if(!success)  {
     // The characters are not there or are not printable.
     k = 0;
    }

  buf[k] = '\0';

  if(flip) {
    // Flip adjacent characters
    for(int j = 0; j < k; j += 2) {
      char t = buf[j];
      buf[j] = buf[j + 1];
      buf[j + 1] = t;
      }
    }

  // Trim any beginning and end space
  int i = -1;
  int j = -1;
  for(k = 0; buf[k] != '\0'; ++k) {
    if(! isspace(buf[k]))  {
      if(i < 0)
        i = k;
      j = k;
      }
    }

  if((i >= 0) && (j >= 0)) {
    for(k = i; (k <= j) && (buf[k] != '\0'); ++k)
      buf[k - i] = buf[k];
    buf[k - i] = '\0';
    }
  return buf;
}
//----------------------------------------------------------------------------
static bool checkSerialHDD(HANDLE hDisk, LPCTSTR serial, bool openedAsAdmin)
{
  char serialNumber [256] = "";
#ifdef WITH_ADMIN_RIGHT
  if(openedAsAdmin)
    addSerialHDD_withAdminRight(hDisk, serialNumber);
#endif
  if(!*serialNumber) {
    STORAGE_PROPERTY_QUERY query;
    DWORD cbBytesReturned = 0;
    char buffer [10000];
    memset((void*)&query, 0, sizeof (query));
    query.PropertyId = StorageDeviceProperty;
    query.QueryType = PropertyStandardQuery;
    memset (buffer, 0, sizeof (buffer));
    if(DeviceIoControl(hDisk, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query),
               &buffer, sizeof(buffer), &cbBytesReturned, 0)) {
      STORAGE_DEVICE_DESCRIPTOR* descrip = (STORAGE_DEVICE_DESCRIPTOR*)&buffer;
      char serialNumber [256];
      flipAndCodeBytes (buffer, descrip->SerialNumberOffset, 1, serialNumber);
      }
    }
  if(*serialNumber) {
    TCHAR t[256];
    copyStrZ(t, serialNumber);
    return !_tcscmp(serial, t);
    }
  return false;
}
//----------------------------------------------------------------------------
#define MAX_KEY_LENGTH 256
//----------------------------------------------------------------------------
static bool checkNetMAC(HANDLE hMAC, LPCTSTR mac)
{
  ULONG OidCode = OID_802_3_PERMANENT_ADDRESS;
  BYTE tBuff[64] = { 0 };
  DWORD written;
  if(DeviceIoControl(hMAC, IOCTL_NDIS_QUERY_GLOBAL_STATS, &OidCode, sizeof(OidCode),
              tBuff, sizeof(tBuff), &written, 0)) {
    TCHAR t[MAX_KEY_LENGTH];
    wsprintf(t, _T("%02X-%02X-%02X-%02X-%02X-%02X"), tBuff[0], tBuff[1], tBuff[2], tBuff[3], tBuff[4], tBuff[5]);
    return !_tcscmp(mac, t);
    }
  return false;
}
//----------------------------------------------------------------------------
static bool checkSerial(HANDLE hd, uint type, LPCTSTR serial, bool openedAsAdmin)
{
  switch(type) {
    case esNet:
      return checkNetMAC(hd, serial);
    case esDisk:
      return checkSerialHDD(hd, serial, openedAsAdmin);
    }
  return false;
}
//---------------------------------------------------------------------
static HANDLE my_CreateFile(LPCTSTR currKey, bool admin)
{
  DWORD access = admin ? GENERIC_READ | GENERIC_WRITE : 0;
  DWORD share = admin ? FILE_SHARE_READ | FILE_SHARE_WRITE : FILE_SHARE_READ;
  return CreateFile(currKey, access, share, 0, OPEN_EXISTING, 0, 0);
}
//----------------------------------------------------------------------------
static bool openKey(LPSTR key, uint len)
{
  TCHAR file[_MAX_PATH];
  if(!IsTextUnicode(key, len, 0))
    copyStrZ(file, key);
  else
    copyStrZ(file, (LPWSTR)key);
#ifndef UNICODE
  #pragma comment( user,  "andrebbe compilato in UNICODE, key è salvata in unicode")
#endif
  pvvChar part;
  uint nPart = splitParam(part, file, _T('§'));
#ifdef WITH_ADMIN_RIGHT
  bool openedAsAdmin = true;
  HANDLE hd = my_CreateFile(&part[0], true);
  if(INVALID_HANDLE_VALUE == hd) {
    hd = my_CreateFile(&part[0], false);
    openedAsAdmin = false;
    }
#else
  bool openedAsAdmin = false;
  HANDLE hd = my_CreateFile(&part[0], false);
#endif
  if(INVALID_HANDLE_VALUE != hd) {
    bool success = true;
    if(nPart >= 3)
      success = checkSerial(hd, _ttoi(&part[1]), &part[2], openedAsAdmin);
    CloseHandle(hd);
    return success;
    }
  // se ha fallito quella sopra allora non ha codici seriali aggiunti in coda oppure è proprio errato, si può utilizzare direttamente file
  TCHAR tmp[_MAX_PATH];
  pvvChar target;
  uint n = splitParam(target, file, _T('#'));
  pvvChar target2;
  uint n2 = splitParam(target2, &target[2], _T('&'));
  if(n2 <= 1) {
    HANDLE hd = CreateFile(file, 0, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(INVALID_HANDLE_VALUE != hd) {
      CloseHandle(hd);
      return true;
      }
    return false;
    }
  _tcscpy_s(tmp, &target[0]);
  _tcscat_s(tmp, _T("#"));
  _tcscat_s(tmp, &target[1]);
  _tcscat_s(tmp, _T("#"));
  for(uint i = 0; i < n2 - 1; ++i) {
    _tcscat_s(tmp, &target2[i]);
    _tcscat_s(tmp, _T("&"));
    }
  LPTSTR p = tmp + _tcslen(tmp);
  for(uint i = 1; i <= 30; ++i) {
    *p = 0;
    wsprintf(p, _T("%d#"), i);
    _tcscat_s(tmp, &target[3]);
    hd = CreateFile(tmp, 0, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(INVALID_HANDLE_VALUE != hd) {
      CloseHandle(hd);
      return true;
      }
    }
  return false;
}
//----------------------------------------------------------------------------
#define LEN_KEY_C 4
//----------------------------------------------------------------------------
static void crypt(LPSTR target, LPCSTR source, LPCSTR key, int len, WORD& crc)
{
  crc += cryptBufferNoZ((LPBYTE)target, (LPCBYTE)source, len, (LPCBYTE)key, LEN_KEY_C, 1);
}
//----------------------------------------------------------------------------
static void cryptZ(LPSTR target, LPCSTR source, LPCSTR key, int len, WORD& crc)
{
  crypt(target, source, key, len, crc);
  target[len] = 0;
}
//----------------------------------------------------------------------------
#if 1
//----------------------------------------------------------------------------
#define decrypt crypt
#define decryptZ cryptZ
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
static void decrypt(LPSTR target, LPCSTR source, LPCSTR key, int len, WORD& crc)
{
  for(int i = 0; i < len; ++i) {
    crc += source[i];
    target[i] = source[i] ^ key[i & 3] ^ i;
    }
}
//----------------------------------------------------------------------------
static void decryptZ(LPSTR target, LPCSTR source, LPCSTR key, int len, WORD& crc)
{
  decrypt(target, source, key, len, crc);
  target[len] = 0;
}
//----------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------
using namespace p_checkKey;
//----------------------------------------------------------------------------
errCheckKey ckeckDate(key_dataInfo& kdi);
//----------------------------------------------------------------------------
struct fullInfo
{
  infoKey ik;
  int count;
  bool prompt;
  WORD crc;

  fullInfo() : count(0), prompt(false), crc(0) {}
  fullInfo(const infoKey& ik)  : count(0), prompt(false), crc(0), ik(ik) {}
  fullInfo(const infoKeyCheck& ikc)  : count(0), prompt(false), crc(0), ik(ikc) {}
};
//----------------------------------------------------------------------------
#define MSG_NO_HEADER _T("No header on file!")
#define MSG_NO_COMP_HEADER _T("This file is not compatible!")
//----------------------------------------------------------------------------
static
errCheckKey localLoadKey(LPCTSTR filename, fullInfo& fi)
{
  // anche se dobbiamo solo leggerlo occorre aprirlo in scrittura, sembra che, altrimenti,
  // invece del virtualstore cerchi di leggerlo nella vera system32
  P_File pf(filename);
  if(!pf.P_exist())
    return eckNoFile;
  if(!pf.P_open())
    return eckNoReadFile;

  key_head kh;
  if(!pf.P_read(&kh, sizeof(kh)))
    return eckNoReadFile;

  infoKey& ik = fi.ik;

  if(memcmp(kh.nps, ik.head.nps, sizeof(kh.nps))) {
    if(fi.prompt)
      MessageBox(*ik.owner, MSG_NO_COMP_HEADER, _T("Warning!"), MB_OK | MB_ICONSTOP);
    return eckWrongFile;
    }

  uint len = (uint)pf.get_len();
  LPSTR buff = new char[len];
  pf.P_seek(0);
  pf.P_read(buff, len);

  WORD enabled = *(LPWORD)(buff + len - sizeof(WORD));
  fi.crc = *(LPWORD)(buff + len - sizeof(WORD) * 2);
  if(!ik.reversedWord)
    enabled = ~enabled;
  fi.count = (enabled >> 8) & 0xff;
  fi.ik.enabled = (fi.count ^ (enabled & 0xff));

  FILETIME& ftCurr = ik.kdi.ftCurr;
  FILETIME& ftLast = ik.kdi.ftLast;
  MK_I64(ftCurr) = 0;
  ftLast = INFINITE_DATE;
  WORD crc2 = 0;
  LPSTR baseKey = buff + kh.offset;
  LPSTR pt = buff + sizeof(kh) + ik.lenDummyAdded;
  ftCurr = *(FILETIME*)pt;
  pt += sizeof(ftCurr);
  ftLast = *(FILETIME*)pt;
  LPSTR tl = (LPSTR)&ftCurr;
  decrypt(tl, tl, baseKey, sizeof(ftCurr), crc2);
  tl = (LPSTR)&ftLast;
  decrypt(tl, tl, baseKey, sizeof(ftLast), crc2);
  pt += sizeof(ftCurr);
  if(baseKey > pt)
    decrypt(pt, pt, baseKey, int(baseKey - pt), crc2);

  memcpy(ik.kdi.svKey, pt, sizeof(ik.kdi.svKey)); // per 3.1
  pt += sizeof(ik.kdi.svKey);
  ik.maxAllowed = *(LPWORD)pt;
  pt += sizeof(WORD);
  ik.numCode = *(LPWORD)pt;

  LPSTR p = baseKey + 4;
  PVect<infoKeyItem>& Key = ik.items;

  for(int i = 0; i < fi.count; ++i) {
    WORD lenKey = *(LPWORD)p;
    p += sizeof(WORD);
    LPSTR t = (LPSTR)&lenKey;
    decrypt(t, t, baseKey, sizeof(WORD), crc2);
    Key[i].lenKey = lenKey;
    memcpy(Key[i].pKey, p, lenKey);
    Key[i].pKey[lenKey] = 0;
    Key[i].pKey[lenKey + 1] = 0;
    p += lenKey;
    }
  for(int i = 0; i < fi.count; ++i)
    decryptZ(Key[i].pKey, Key[i].pKey, baseKey, Key[i].lenKey, crc2);
  if(ik.numCode) {
    PVect<infoKeyItemV2>& Key2 = ik.items2;
    for(uint i = 0; i < ik.numCode; ++i, p += sizeof(infoKeyItemV2))
      decrypt((LPSTR)&Key2[i], p, baseKey, sizeof(infoKeyItemV2), crc2);
    }
  else {
    bool hasDate = !(INFINITE_DATE == ftLast);
    bool hasCode = !is_svEmptyKey(ik.kdi.svKey, true);
    if(hasDate || hasCode) {
      ik.numCode = 1;
      PVect<infoKeyItemV2>& Key2 = ik.items2;
      if(hasDate)
        Key2[0].ftEnd = ftLast;
      if(hasCode)
        memcpy_s(Key2[0].pKey, sizeof(Key2[0].pKey), ik.kdi.svKey, sizeof(Key2[0].pKey));
      else
        make_svEmptyKey(Key2[0].pKey, true);
      }
    }
  WORD newCrc = *(LPWORD)p;
  delete []buff;

  if(fi.crc != (crc2 ^ newCrc))
    return eckErrCRC;

  return eckNo_Err;
}
//----------------------------------------------------------------------------
static
errCheckKey localSaveKey(LPCTSTR filename, const fullInfo& fi)
{
  srand(GetTickCount());

  if(P_File::P_exist(filename)) {
    P_File pf(filename);

    if (!pf.reback(_T(".bak"))) {
    // l'ambiente del vs 2015 viene eseguito come admin quindi il file invece che nel virtualstore va proprio in windows
    // quindi poi se si esegue come utente normale, non riesce a rinominare perché vede il .bak presente in windows
//    DisplayErrorString(GetLastError());
      ; //return eckNoWriteFile;
      }
    }

  do {
    P_File pf(filename, P_CREAT);
    if(!pf.P_open())
      break;

    const infoKey& ik = fi.ik;
    key_head kh = ik.head;
    kh.ver = MAKE_VER(HI_VER, LO_VER);
    kh.offset = sizeof(kh) + ik.lenDummyAdded + sizeof(ik.kdi);

    if(!pf.P_write(&kh, sizeof(kh)))
      break;
    srand(GetTickCount());
    if(ik.lenDummyAdded) {
      char t[MAX_DUMMY_BYTES + 2];
      for(uint i = 0; i < ik.lenDummyAdded; ++i)
        t[i] = rand();
      if(!pf.P_write(t, ik.lenDummyAdded))
        break;
      }

    char baseKey[4];
    *(LPDWORD)baseKey = ((DWORD)rand() << 16) | rand();
    WORD crc = 0;
    key_dataInfo kdi = ik.kdi;

    const PVect<infoKeyItemV2>& items2 = ik.items2;
    uint nElem2 = items2.getElem();
    kdi.numCode = nElem2;
    kdi.maxAllowed = ik.maxAllowed;

    // per non stravolgere troppo il vecchio codice, le due filetime le criptiamo a parte
    LPSTR tl = (LPSTR)&kdi.ftCurr;
    crypt(tl, tl, baseKey, sizeof(kdi.ftCurr), crc);
    tl = (LPSTR)&kdi.ftLast;
    crypt(tl, tl, baseKey, sizeof(kdi.ftLast), crc);
    // poi il resto in una sola volta
    tl = (LPSTR)&kdi.svKey;
    crypt(tl, tl, baseKey, sizeof(kdi) - sizeof(FILETIME) * 2, crc);
    if(!pf.P_write(&kdi, sizeof(kdi)))
      break;

    if(!pf.P_write(baseKey, sizeof(baseKey)))
      break;

    const PVect<infoKeyItem>& items = ik.items;
    uint nElem = items.getElem();

    bool success = true;
    for(uint i = 0; i < nElem; ++i) {
      const infoKeyItem& key = items[i];
      WORD lenText = key.lenKey;
      LPSTR tl = (LPSTR)&lenText;
      crypt(tl, tl, baseKey, sizeof(lenText), crc);
      if(!pf.P_write(tl, sizeof(lenText))) {
        success = false;
        break;
        }
      char pKey[MAX_DIM_KEY];
      crypt(pKey, key.pKey, baseKey, key.lenKey, crc);
      if(!pf.P_write(pKey, key.lenKey)) {
        success = false;
        break;
        }
      }

    if(!success)
      break;
    // per compatibilità deve salvare il crc della versione precedente
    WORD crc2 = crc;
    for(uint i = 0; i < nElem2; ++i) {
      const infoKeyItemV2& key = items2[i];
      char pKey[sizeof(key)];
      crypt(pKey, (LPCSTR)&key, baseKey, sizeof(key), crc);
      if(!pf.P_write(pKey, sizeof(key))) {
        success = false;
        break;
        }
      }

    if(!success)
      break;
    crc ^= crc2;
    if(!pf.P_write(&crc, sizeof(crc)))
      break;
    if(!pf.P_write(&crc2, sizeof(crc2)))
      break;

    WORD enabled = (nElem ^ fi.ik.enabled) ^ ~(nElem << 8);
    if(ik.reversedWord)
      enabled = ~enabled;

    if(!pf.P_write(&enabled, sizeof(enabled)))
      break;

    return eckNo_Err;

    } while(false);

  P_File pf(filename);
  pf.unback(_T(".bak"));
  return eckNoWriteFile;
}
//----------------------------------------------------------------------------
#define ADD_TIME (HOUR_TO_I64 * 2)
#define MAX_FILE_CHECK 300
//----------------------------------------------------------------------------
#if 0
// ormai l'eseguibile non è più compatibile con versioni precedenti XP-SP3
//----------------------------------------------------------------------------
static bool mySHGetSpecialFolderPath(LPTSTR path)
{
#pragma warning( push )
#pragma warning( disable : 4996 )
  LPCTSTR wdir = _tgetenv(_T("windir"));
#pragma warning( pop )
  if(wdir) {
    _tcscpy_s(path, _MAX_PATH, wdir);
//    appendPath(path, _T("system"));
    return true;
    }
  return false;
}
//----------------------------------------------------------------------------
static bool getCheckDate(LPTSTR path)
{
  if(isWin98orLater() || isWin95()) {
    if(!mySHGetSpecialFolderPath(path))
      return false;
    }
  else
    if(FAILED(SHGetFolderPath(0, CSIDL_LOCAL_APPDATA, NULL, 0, path)))
      if(FAILED(SHGetFolderPath(0, CSIDL_RECENT, NULL, 0, path)))
        if(FAILED(SHGetFolderPath(0, CSIDL_SYSTEM, NULL, 0, path)))
          return false;

  appendPath(path, _T("Microsoft\\NPS_SV"));
  return true;
}
//----------------------------------------------------------------------------
#else
//----------------------------------------------------------------------------
static bool getCheckSystemDate(LPTSTR path)
{
  if(FAILED(SHGetFolderPath(0, CSIDL_LOCAL_APPDATA, NULL, 0, path)))
    if(FAILED(SHGetFolderPath(0, CSIDL_RECENT, NULL, 0, path)))
      if(FAILED(SHGetFolderPath(0, CSIDL_SYSTEM, NULL, 0, path)))
        return false;

  return true;
}
//----------------------------------------------------------------------------
static bool getCheckDate(LPTSTR path)
{
  if(!getCheckSystemDate(path))
    return false;

  appendPath(path, _T("Microsoft\\NPS_SV"));
  return true;
}
//----------------------------------------------------------------------------
#endif
//------------------------------------------------------------------
static void createTemp()
{
  TCHAR path[_MAX_PATH];
  if(!getCheckDate(path))
    return;
  createDirectoryRecurse(path);
  TCHAR file[_MAX_PATH];
  if(!GetTempFileName(path, _T("npsv"), 0, file))
    return;
  P_File pf(file, P_CREAT);
  pf.P_open();
}
//----------------------------------------------------------------------------
static errCheckKey findAllType(LPTSTR path, FILETIME ft)
{
  appendPath(path, _T("*.*"));
  WIN32_FIND_DATA FindFileData;

  HANDLE hf = FindFirstFile(path, &FindFileData);
  if(hf == INVALID_HANDLE_VALUE)
    return eckUnknowErr;

  errCheckKey err = eckNo_Err;
  uint count = 0;
  do {
    if(FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes)
      if(_T('.') == FindFileData.cFileName[0])
        continue;
    if(MK_I64(FindFileData.ftCreationTime) && FindFileData.ftCreationTime > ft) {
      err = eckWrongDate;
      break;
      }
    if(MK_I64(FindFileData.ftLastWriteTime) && FindFileData.ftLastWriteTime > ft) {
      err = eckWrongDate;
      break;
      }
    } while(count++ < MAX_FILE_CHECK && FindNextFile(hf, &FindFileData));

  FindClose(hf);
  return err;
}
//----------------------------------------------------------------------------
static LPTSTR dirName(LPTSTR p, int len)
{
  while(len-- > 0) {
    --p;
    if(_T('\\') == *p) {
      *p = 0;
      break;
      }
    }
  return p;
}
//----------------------------------------------------------------------------
errCheckKey ckeckDate(key_dataInfo& kdi)
{
  // scadenza non impostata
  if(INFINITE_DATE == kdi.ftLast)
    return eckInfiniteDate;

  // è già stata accertata la violazione
  if(0 == MK_I64(kdi.ftLast))
    return eckWrongDate;

  FILETIME ft = getFileTimeUTC();
  if(ft > kdi.ftLast) {
    kdi.ftCurr = ft;
    createTemp();
    return eckOutOfDate;
    }

  // per controllare se è stata modificata l'ora manualmente si usa l'orario globale
  // non influenzato dal cambio dell'ora legale
  if(ft < kdi.ftCurr) {
    MK_I64(kdi.ftLast) = 0;
    return eckWrongDate;
    }
  kdi.ftCurr = ft;
  // esegue una ricerca se c'è un file più recente del time corrente

  TCHAR path[_MAX_PATH];

  errCheckKey err = eckNo_Err;
  if(getCheckSystemDate(path)) {
    err = findAllType(path, ft + ADD_TIME);
    if(eckNo_Err != err) {
      createTemp();
      return err;
      }
    }
  if(!getCheckDate(path))
    return eckUnknowErr;

  LPTSTR p = path + _tcslen(path);
  appendPath(path, _T("npsv*.tmp"));
  WIN32_FIND_DATA FindFileData;

  HANDLE hf = FindFirstFile(path, &FindFileData);

  // se è la prima volta o è stata cancellata la cartella con i file temporanei di test
  // crea cartella e file, poi risale a togliere il path aggiunto e cerca li file più recenti
  if(hf == INVALID_HANDLE_VALUE) {
    createTemp();
    *p = 0;
    p = dirName(p, int(p - path));
    dirName(p, int(p - path));
    return findAllType(path, ft + ADD_TIME);
    }
  ft += ADD_TIME;
  do {
    if(FILE_ATTRIBUTE_DIRECTORY & FindFileData.dwFileAttributes)
      if(_T('.') == FindFileData.cFileName[0])
        continue;
    if(MK_I64(FindFileData.ftCreationTime) && FindFileData.ftCreationTime > ft) {
      err = eckWrongDate;
      break;
      }
    if(MK_I64(FindFileData.ftLastWriteTime) && FindFileData.ftLastWriteTime > ft) {
      err = eckWrongDate;
      break;
      }
    *p = 0;
    appendPath(path, FindFileData.cFileName);
    DeleteFile(path);

    } while(FindNextFile(hf, &FindFileData));

  FindClose(hf);
  createTemp();
  return err;
}
//----------------------------------------------------------------------------
/*
errCheckKey p_checkKey::checkAndSaveKey(LPCTSTR filename, const infoKeyCheck& ikc)
{
  fullInfo fI(ikc);
  errCheckKey err = localLoadKey(filename, fI);
  if(eckNo_Err != err)
    return err;

  err = ckeckDate(fI.ik.kdi);
  if(eckNo_Err != err && eckInfiniteDate != err) {
    localSaveKey(filename, fI);
    return err;
    }

  uint result = 0;
  PVect<infoKeyItem>& Key = fI.ik.items;
  for(int i = 0; i < fI.count && result < fI.ik.enabled; ++i)
    result += openKey(Key[i].pKey, Key[i].lenKey);
  if(result != fI.ik.enabled)
    return eckNoKey;
  if(eckInfiniteDate != err)
    err = localSaveKey(filename, fI);
  return err;
}
*/
//----------------------------------------------------------------------------
errCheckKey p_checkKey::checkAndSaveKey(LPCTSTR filename, const infoKeyCheck& ikc, infoKeyItemResultV2* iki)
{
  fullInfo fI(ikc);
  errCheckKey err = localLoadKey(filename, fI);
  if(eckNo_Err != err)
    return err;

  err = ckeckDate(fI.ik.kdi);
  if(eckNo_Err != err && eckInfiniteDate != err) {
    localSaveKey(filename, fI);
    return err;
    }
  if(iki) {
    iki->maxAllowed = fI.ik.maxAllowed;
    const PVect<infoKeyItemV2>& items2 = fI.ik.items2;
    uint nElem = items2.getElem();
    // se ci sono più elementi oppure ce n'è uno solo con codice, esegue il controllo
    if(nElem > 1 || 1 == nElem && !is_svEmptyKey(items2[0].pKey, true)) {
      key_dataInfo kdi = fI.ik.kdi;
      bool found = false;
      for(uint i = 0; i < nElem; ++i) {
        if(verify_svKey(iki->svKey.pKey, items2[i].pKey)) {
          kdi.ftLast = items2[i].ftEnd;
          err = ckeckDate(kdi);
          // se data scaduta, torna errore
          if(eckNo_Err != err && eckInfiniteDate != err) {
            localSaveKey(filename, fI);
            return err;
            }
          found = true;
          break;
          }
        }
      if(!found)
        return eckNoKey;
      }
    }
  uint result = 0;
  PVect<infoKeyItem>& Key = fI.ik.items;
  for(int i = 0; i < fI.count && result < fI.ik.enabled; ++i)
    result += openKey(Key[i].pKey, Key[i].lenKey);
  if(result != fI.ik.enabled)
    return eckNoKey;
  if(eckInfiniteDate != err)
    err = localSaveKey(filename, fI);
  return err;
}
//----------------------------------------------------------------------------
errCheckKey p_checkKey::loadKey(LPCTSTR filename, infoKey& ik, bool prompt)
{
  fullInfo fI(ik);
  fI.prompt = prompt;
  errCheckKey err = localLoadKey(filename, fI);
  if(eckNo_Err != err)
    return err;
  ik = fI.ik;
  return eckNo_Err;
}
//----------------------------------------------------------------------------
#define MSG_ERR_WRITE _T("Error while write to file")
#define MSG_OK_WRITE _T("File successfully written")
//----------------------------------------------------------------------------
errCheckKey p_checkKey::saveKey(LPCTSTR filename, const infoKey& ik, bool prompt)
{
  fullInfo fI(ik);
  fI.prompt = prompt;
  errCheckKey err = localSaveKey(filename, fI);
  if(prompt) {
    if(eckNo_Err != err)
      MessageBox(*ik.owner, MSG_ERR_WRITE, _T("Warning!"), MB_OK | MB_ICONSTOP);
    else
      MessageBox(*ik.owner, MSG_OK_WRITE, _T("Success!"), MB_OK | MB_ICONINFORMATION);
    }
  return err;
}
//----------------------------------------------------------------------------
void p_checkKey::make_svKey(LPBYTE target, LPCBYTE source)
{
  srand(GetTickCount());
  BYTE rnd[DIM_svKEY];
  *(LPDWORD)rnd = ((DWORD)rand() << 16) | rand();
  *(LPDWORD)(rnd + 4) = ((DWORD)rand() << 16) | rand();
  for(uint i = 0, j = 0; i < SIZE_A(rnd); ++i, j += 2) {
    target[j] = source[i] ^ rnd[i];
    target[j + 1] = rnd[i];
    }
}
//----------------------------------------------------------------------------
void p_checkKey::get_svKey(LPBYTE target, LPCBYTE source)
{
  for(uint i = 0, j = 0; i < DIM_svKEY; ++i, j += 2)
    target[i] = source[j] ^ source[j + 1];
}
//----------------------------------------------------------------------------
static bool verify_svKey_8_16(LPCBYTE k8, LPCBYTE k16)
{
  bool equ = true;
  for(uint i = 0, j = 0; i < DIM_svKEY && equ; ++i, j += 2)
    equ &= !(k8[i] ^ k16[j] ^ k16[j + 1]);
  return equ;
}
//----------------------------------------------------------------------------
errCheckKey p_checkKey::check_svKey(LPCTSTR filename, const infoKeyCheck& ikc, LPBYTE svKeyTarget, bool trueVal)
{
  fullInfo fI(ikc);
  errCheckKey err = localLoadKey(filename, fI);
  if(eckNo_Err != err)
    return err;
  if(fI.ik.head.ver < MAKE_VER(3, 2))
    return eckWrongVersion;

  const PVect<infoKeyItemV2>& items2 = fI.ik.items2;
  uint nElem = items2.getElem();
  // se non ci sono elementi oppure ce n'è uno solo senza codice, torna la chiave vuota
  if(!nElem || 1 == nElem && is_svEmptyKey(items2[0].pKey, true))
    make_svEmptyKey(svKeyTarget, !trueVal);
  else {
    BYTE key[DIM_svKEY * 2];
    if(trueVal)
      get_svKey(key, svKeyTarget);
    else
      memcpy(key, svKeyTarget, sizeof(key));
    key_dataInfo kdi = fI.ik.kdi;
    for(uint i = 0; i < nElem; ++i) {
      if(verify_svKey(key, items2[i].pKey)) {
        kdi.ftLast = items2[i].ftEnd;
        err = ckeckDate(kdi);
        // se data scaduta, torna errore
        if(eckNo_Err != err && eckInfiniteDate != err)
          return err;
        // (ri)carica la chiave nel formato voluto
        if(trueVal)
          get_svKey(svKeyTarget, items2[i].pKey);
        else
          memcpy(svKeyTarget, items2[i].pKey, sizeof(items2[i].pKey));
        return eckNo_Err;
        }
      }
    return eckNoKey;
    }
  return eckNo_Err;
}
//----------------------------------------------------------------------------
bool p_checkKey::verify_svKey(LPCBYTE k1, LPCBYTE k2)
{
  bool equ = true;
  for(uint i = 0, j = 0; i < DIM_svKEY && equ; ++i, j += 2)
    equ &= !(k1[j] ^ k1[j + 1] ^ k2[j] ^ k2[j + 1]);
  return equ;
}
//----------------------------------------------------------------------------
#define EMPTY_KEY_LOW 0xbabecafe
#define EMPTY_KEY_HI  0xfecabeba
//----------------------------------------------------------------------------
#define MAKE_EMPTY_KEY(t) do { *(LPDWORD)t = EMPTY_KEY_HI; *(LPDWORD)(t + 4) = EMPTY_KEY_LOW; } while(false)
//----------------------------------------------------------------------------
void p_checkKey::make_svEmptyKey(LPBYTE target, bool fullVal)
{
  if(fullVal) {
    BYTE t[DIM_svKEY];
    MAKE_EMPTY_KEY(t);
    make_svKey(target, t);
    }
  else
    MAKE_EMPTY_KEY(target);
}
//----------------------------------------------------------------------------
bool p_checkKey::is_svEmptyKey(LPCBYTE source, bool fullVal)
{
  BYTE t[DIM_svKEY];
  MAKE_EMPTY_KEY(t);
  if(fullVal) {
    bool success = true;
    for(uint i = 0, j = 0; i < DIM_svKEY; ++i, j += 2)
      success &= t[i] == (source[j] ^ source[j + 1]);
    return success;
    }
  else
    return !memcmp(source, t, DIM_svKEY);
}
