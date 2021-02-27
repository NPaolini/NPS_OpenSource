//------main.cpp --------------------------------------------------
//-----------------------------------------------------------------
#include "precHeader.h"
#include "WrapSquidService.h"
//-----------------------------------------------------------------
int main( int argc, char ** argv ) 
{
  // create the service-object
  WrapSquidService serv;

  // RegisterService() checks the parameterlist for predefined switches
  // (such as -d or -i etc.; see NTService.h for possible switches) and
  // starts the service's functionality.
  // You can use the return value from "RegisterService()"
  // as your exit-code.
  return serv.RegisterService(argc, argv);
}
