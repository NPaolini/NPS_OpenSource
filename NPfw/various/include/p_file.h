//-------------- P_FILE.H -----------------------------------------------
//-----------------------------------------------------------------------
#ifndef P_FILE_H_
#define P_FILE_H_
//-----------------------------------------------------------------------
#include "precHeader.h"
//-----------------------------------------------------------------------
#ifndef DEFIN_H_
  #include "defin.h"
#endif

#include "setPack.h"
//-----------------------------------------------------------------------
void GetModuleDirName(size_t lenBuff, LPTSTR target);
//-----------------------------------------------------------------------
LPWSTR dirName(LPWSTR path);
LPSTR dirName(LPSTR path);
//-----------------------------------------------------------------------
// errori
enum { NOT_ERR, NOT_GEST, NOT_OPEN, NOT_USER,
     NOT_READ, NOT_WRITE, NOT_SEEK, MAX_MSG };
//-----------------------------------------------------------------------
enum ACC_FILE { P_RW, P_READ_ONLY, P_CREAT };
//-----------------------------------------------------------------------
typedef void tFILE;
//-----------------------------------------------------------------------
class P_File  // accesso diretto ai files
{
  public:
    P_File(LPCTSTR n, ACC_FILE mode = P_RW); // costruttore, nome e modo
    ~P_File();

    bool P_exist();  // usato, naturalmente, quando non aperto
    static bool P_exist(LPCTSTR filename);  // usato, naturalmente, quando non aperto

    static void chgExt(LPTSTR filename, LPCTSTR newExt);

    bool P_open(HANDLE opened = 0, bool append = false);
    bool P_close();

    lUDimF P_write(const tFILE *b, lUDimF len); // ritorna numero di caratteri scritti

#ifdef USE_UNICODE_4_WRITE_STRING
  #define P_writeString P_writeToUnicode

#else
//#elif defined(USE_ANSI_4_WRITE_STRING)
  #define P_writeString P_writeToAnsi

//#else
//  #error definire una modalità di scrittura stringhe su file
#endif
    // scrivono in ansi (solo la lunghezza, senza lo zero terminatore)
    lUDimF P_writeToAnsi(LPCWSTR b, int len = -1);
    lUDimF P_writeToAnsi(LPCSTR b, int len = -1);

    // scrivono in unicode (solo la lunghezza, senza lo zero terminatore)
    lUDimF P_writeToUnicode(LPCSTR b, int len = -1);
    lUDimF P_writeToUnicode(LPCWSTR b, int len = -1);

    lUDimF P_read(tFILE *b, lUDimF len);  // ritorna numero di caratteri letti
    bool P_seek(DimF offset, int fromwhere = SEEK_SET_);

    UDimF get_pos();
    UDimF get_len();
    ACC_FILE get_mode();
    LPCTSTR get_name();
    HANDLE get_handle();

    bool chgsize(UDimF new_dim, LPCTSTR back_ext = 0);
    bool reback(LPCTSTR ext = 0);
    bool unback(LPCTSTR ext = 0);

    bool appendback();
    bool unappendback();

    bool P_rename(LPCTSTR new_name);

    // se il file era aperto lo chiude e riapre col nuovo nome, altrimenti cambia solo il nome
    bool P_chgName(LPCTSTR new_name);

    bool eof();
    int Error(int type);
    LPCTSTR get_last_error();
    void reset_error();
    bool flush();

    bool getData(FILETIME& lasttime);
    bool setData(const FILETIME& newtime);

    void setAttribute(DWORD attr) { Attr = attr; }
    DWORD getAttribute() { return Attr; }
  private:
    LPTSTR Name;        // pathname
    HANDLE Handle;      // handle del file
    ACC_FILE Mode;      // modalità di apertura
    int tError;
    UDimF Pos;         // posizione nel file, viene aggiornata ad ogni seek,
                        // read o write
    UDimF Len;        // lunghezza del file, viene aggiornata da write

    bool P_performRename(LPCTSTR new_name);
    DWORD Attr;

    NO_COPY_COSTR_OPER(P_File)
};
//-----------------------------------------------------------------------
inline UDimF P_File::get_pos() { return Pos; }
//-----------------------------------------------------------------------
inline UDimF P_File::get_len() { return Len; }
//-----------------------------------------------------------------------
inline ACC_FILE P_File::get_mode() { return Mode; }
//-----------------------------------------------------------------------
inline LPCTSTR P_File::get_name() { return Name; }
//-----------------------------------------------------------------------
inline HANDLE P_File::get_handle() { return Handle; }
//-----------------------------------------------------------------------
inline bool P_File::eof() { return Pos >= Len; }
//-----------------------------------------------------------------------
inline void P_File::reset_error() { tError = 0; }
//-----------------------------------------------------------------------
#include "restorePack.h"
#endif
