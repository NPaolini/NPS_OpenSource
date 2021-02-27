//-------- setPack1.h ----------------------------------------------
//------------------------------------------------------------------
#if defined (__BORLANDC__)
  #include <pshpack1.h>
  #pragma nopackwarning

#elif defined(_MSC_VER)
  #pragma pack(push, 1)

#else
  #error Occorre definire l'allineamento a 1 byte per le strutture seguenti
#endif
//------------------------------------------------------------------
