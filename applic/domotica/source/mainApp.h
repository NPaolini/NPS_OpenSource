//------ mainApp.h -----------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef MAIN_APP__
#define MAIN_APP__
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
// funzioni specifiche da implementare nel target finale

// deve tornare il nome della classe
LPCTSTR getClassName();
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// se attiva (per default) fa avviare una sola instanza dell'applicazione,
// per permettere più applicazioni attive contemporaneamente modificare in
// bool ONLY_ONE = false;
// dove definito

extern bool ONLY_ONE;
//-------------------------------------------------------------------
#endif
