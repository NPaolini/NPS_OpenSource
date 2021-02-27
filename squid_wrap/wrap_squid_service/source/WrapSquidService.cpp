//------WrapSquidService.cpp -------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
#include "WrapSquidService.h"
//----------------------------------------------------------------------------
WrapSquidService::WrapSquidService()
  : CNTService(TEXT("wrapSquid"), TEXT("Wrap for Squid"))
  , m_hStop(0)
{
  m_dwControlsAccepted = 0;
  m_dwControlsAccepted |= SERVICE_ACCEPT_STOP;
  m_dwControlsAccepted |= SERVICE_ACCEPT_SHUTDOWN;
  m_dwServiceType |= SERVICE_INTERACTIVE_PROCESS;
}
//----------------------------------------------------------------------------
void WrapSquidService::Run(DWORD dwArgc, LPTSTR * ppszArgv) {
  // report to the SCM that we're about to start
  ReportStatus(SERVICE_START_PENDING);

  m_hStop = ::CreateEvent(0, TRUE, FALSE, 0);

  // TODO: You might do some initialization here.
  //     Parameter processing for instance ...
  //     If this initialization takes a long time,
  //     don't forget to call "ReportStatus()"
  //     frequently or adjust the number of milliseconds
  //     in the "ReportStatus()" above.

  // report SERVICE_RUNNING immediately before you enter the main-loop
  // DON'T FORGET THIS!
  ReportStatus(SERVICE_RUNNING);

  startMain();

  // main-loop
  // If the Stop() method sets the event, then we will break out of
  // this loop.

//  while( ::WaitForSingleObject(m_hStop, 10) != WAIT_OBJECT_0 ) {

    // TODO: Enter your service's real functionality here

//  }
  if( m_hStop )
    ::CloseHandle(m_hStop);
  ReportStatus(SERVICE_STOPPED, 0, 0);
}
//----------------------------------------------------------------------------
void WrapSquidService::Stop() {
  // report to the SCM that we're about to stop

  // TODO: Adjust the number of milliseconds you think
  //     the stop-operation may take.
  ReportStatus(SERVICE_STOP_PENDING, 5000);

  if( m_hStop )
    ::SetEvent(m_hStop);

  if(stopMain())
    ReportStatus(SERVICE_STOPPED, 0, 0);

}
//----------------------------------------------------------------------------
void WrapSquidService::Shutdown() {
  if(m_hStop)
    ::SetEvent(m_hStop);
  stopMain();
  // TODO: add code here that will be executed when the system shuts down
}
