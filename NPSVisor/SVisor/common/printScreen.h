//-------- printScreen.h ------------------------------------------------------
//----------------------------------------------------------------------------
#ifndef PRINTSCREEN_H_
#define PRINTSCREEN_H_
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
bool printScreen(PWin* owner, bool preview = false, bool showDialog = false, bool fullscreen = false);
//----------------------------------------------------------------------------
// se non viene passato il nome del file lo prende dallo std_msg
// il file ha un suffisso (_nnn.) autoincrementante
// può avere l'estensione .jpg o .bmp
bool saveScreen(PWin* owner, LPCTSTR filename = 0);
//----------------------------------------------------------------------------
#endif
