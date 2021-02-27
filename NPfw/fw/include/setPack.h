//-------- setPack.h -----------------------------------------------
//------------------------------------------------------------------
#if defined (__BORLANDC__)
  #include <pshpack8.h>
  #pragma nopackwarning

#elif defined(_MSC_VER)
  #pragma pack(push, 8)

#else
  #error Occorre definire l'allineamento a 8 byte per le strutture seguenti
#endif
//------------------------------------------------------------------
