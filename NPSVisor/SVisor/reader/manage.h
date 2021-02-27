//------ manage.h ------------------------------------------
//----------------------------------------------------------
#ifndef MANAGE_H_
#define MANAGE_H_
//----------------------------------------------------------
#include "precHeader.h"
#include "mainclient.h"
#include "p_vect.h"
//----------------------------------------------------------
// valido sia per causa che per operatore
typedef DWORD codeType;
#define MAX_NAME_OPER 60
//----------------------------------------------------------
//#define ID_CAUSE_ALARM       6001
//#define ID_CAUSE_NORMAL_STOP 6002
//----------------------------------------------------------
#define FIRST_ID_DEFAULT ID_CAUSE_ALARM
#define LAST_ID_DEFAULT ID_CAUSE_NORMAL_STOP
//--------------------------------------------------------------------------
#define NUM_DEF_CAUSE (ID_CAUSE_NORMAL_STOP - ID_CAUSE_ALARM + 1)
//----------------------------------------------------------
//#define ID_CAUSE_TITLE_INIT 6011
//#define ID_CAUSE_TITLE_END (ID_CAUSE_TITLE_INIT + 300)
//----------------------------------------------------------
class codeOper
{
  public:
    codeOper();
//    ~codeOper();

    LPCTSTR getName(const codeType& code);
    const codeType& getCode(LPCTSTR name);

    LPCTSTR getName(int pos);
    const codeType& getCode(int pos);

    void reload();

    uint getElem() const;

    struct dual
    {
      codeType code;
      TCHAR name[MAX_NAME_OPER];
    };
  private:
    PVect<dual> oper;

    void load();
};
//----------------------------------------------------------
inline codeOper::codeOper()
{
  load();
}
//----------------------------------------------------------
inline void codeOper::reload()
{
  oper.reset();
  load();
}
//----------------------------------------------------------
inline LPCTSTR codeOper::getName(int pos)
{
  const PVect<dual>& op = oper;
  return op[pos].name;
}
//----------------------------------------------------------
inline const codeType& codeOper::getCode(int pos)
{
  const PVect<dual>& op = oper;
  return op[pos].code;
}
//----------------------------------------------------------
inline uint codeOper::getElem() const
{
  return oper.getElem();
}
//----------------------------------------------------------
//----------------------------------------------------------
struct fileCheck
{
  codeType code;
  FILETIME init;
  FILETIME end;
};
//----------------------------------------------------------
// alla chiusura dell'oggetto salva il file temporaneo nel definito.
//----------------------------------------------------------
#define FIRST_CODE_DEFAULT  CODE_ALARM_STOP
//----------------------------------------------------------
#define CODE_ALARM_STOP   998
#define CODE_NORMAL_STOP  999
#define TEMP_CODE        9999
//----------------------------------------------------------
class manageGen
{
  public:
    manageGen(mainClient* parent);
    virtual ~manageGen() { }

//    virtual int notify(gestPerif* plc, gestPerif* reader) = 0;

    void close();

  protected:
    mainClient* Parent;

    enum status {
        run,              // normale lavoro

        stop,             // normale stop, già scelta la causa o riavvio dopo pausa

        stopped_OnShow,   // in stop con la finestra di scelta causa/messaggio aperta

        stopped_EndShow,  // si può chiudere la finestra di scelta/messaggio

        stopped_NeedShow  // è stato riavviato il programma dopo una fermata anomala,
                          // alla prima notify occorrerà aprire la finestra di scelta
                          // per riavviare il time
        };
    status Stat;
    fileCheck check;

    void saveTemp();
    virtual void tempToStore(bool remove);
    void removeTemp();

    // torna true se esiste il file
    bool loadTemp();

    virtual void makeTempPath(LPTSTR path) = 0;
    virtual void makeStorePath(LPTSTR path) = 0;
};
//----------------------------------------------------------
//----------------------------------------------------------
#define SUFFIX_NAME_TEMP _T("_TEMP")
#define PREFIX_NAME_MANAGE_STOP _T("CA")
#define NAME_MANAGE_STOP PREFIX_NAME_MANAGE_STOP _T("aaaamm")
#define NAME_MANAGE_STOP_TEMP PREFIX_NAME_MANAGE_STOP SUFFIX_NAME_TEMP

#define PREFIX_NAME_MANAGE_OPER _T("OP")
#define NAME_MANAGE_OPER PREFIX_NAME_MANAGE_OPER _T("aaaamm")
#define NAME_MANAGE_OPER_TEMP PREFIX_NAME_MANAGE_OPER SUFFIX_NAME_TEMP

#define OFFS_DATE 2
//----------------------------------------------------------
class manageStop : public manageGen
{
  public:
    manageStop(mainClient* parent);
    ~manageStop();

    // codici di ritorno dalla notify
    enum resultStopNotify {
      rInitWaitForShow = -3, // inizio attesa conferma per apertura finestra
      rWaitForShow,   // attesa conferma per apertura finestra
                      // (l'operatore deve inserire la tessera)
      rNotChanged,    // nessun cambiamento
      rShowMsg,       // si è aperta la finestra per scelta causa
      rHideMsg,       // l'operatore ha scelto la causa
      };
    virtual int notify(gestPerif* plc, bool requestCode);

    void enableShow(bool enable) { waitBeforeShow = !enable; }

    void setAlternateChoose(DWORD value);
  protected:
		void performSetAlternateChoose(DWORD value);

    bool waitBeforeShow;
    // torna true se fine lavoro e quindi deve rimuovere il file temporaneo
    bool showList();
    PWin* listOnShow;

    virtual void tempToStore(bool remove);

    virtual void makeTempPath(LPTSTR path);
    virtual void makeStorePath(LPTSTR path);
    bool checkListOnShow;
    DWORD Value;
};
//----------------------------------------------------------
//----------------------------------------------------------
class manageOperator : public manageGen
{
  public:
    manageOperator(mainClient* parent);
    virtual ~manageOperator();

    // codici di ritorno dalla notify
    enum resultOperNotify {
      rCodeNotValid = -2,
      rShowMsg,
      rNotChanged,
      rChanged,
      rHideMsg,
      };
    virtual int notify(gestPerif* plc, genericPerif* reader);

    void showRequest();
    bool isOnRequest()   { return toBool(Request); }

  protected:

    PWin* Request;

    bool isValid(const codeType& curr);
    void showMsg();
    void hideMsg();
    virtual void makeTempPath(LPTSTR path);
    virtual void makeStorePath(LPTSTR path);
    void setOperatorToJob();

    bool notUseReader;
};
//----------------------------------------------------------
//----------------------------------------------------------
#endif
