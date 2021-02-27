//----------- log_alrm.cpp --------------------------------------
#include "precHeader.h"

#include "log_alrm.h"
#include "p_date.h"
//--------------------------------------------------------------
//---------------------------------------------------------------
void status_alarm::startTime()
{
	if(active <= 0) {  // < per sicurezza
		init = getFileTimeCurr();
		active = 0; // per doppia sicurezza
		}
	++active;
}
//---------------------------------------------------------------
bool status_alarm::stopTime()
{
	if(active <= 0)  // non c'è stato nessuno startTime() per questo gruppo
		return false;
	if(--active) // ci sono ancora allarmi attivi dello stesso gruppo
		return true;
	FILETIME ft = getFileTimeCurr();
	LARGE_INTEGER res = *reinterpret_cast<LARGE_INTEGER*>(&ft);
	res.QuadPart -= reinterpret_cast<LARGE_INTEGER*>(&init)->QuadPart;
	(reinterpret_cast<LARGE_INTEGER*>(&total))->QuadPart += res.QuadPart;
	return true;
}
//--------------------------------------------------------------
//--------------------------------------------------------------

