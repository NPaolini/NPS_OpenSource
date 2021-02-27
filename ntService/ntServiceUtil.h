//------------- ntServiceUtil.h ------------------------------------------------
//----------------------------------------------------------------------------
#ifndef ntServiceUtil_H_
#define ntServiceUtil_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
namespace ntServiceUtil {

DWORD controlSampleService(DWORD fdwControl, LPCTSTR serviceName);
bool isInstalled(LPCTSTR serviceName);
bool isRunning(LPCTSTR serviceName);

// in caso di disinstallazione, il parametro user indica se va anche tolto dai privilegi di accesso ai servizi
// se user == 0, lo lascia, se user != 0 lo toglie
bool install(LPCTSTR serviceExecName, bool add = true, LPCTSTR user = 0, LPCTSTR psw = 0);
bool start(LPCTSTR serviceExecName, bool run = true);

};
//----------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------
