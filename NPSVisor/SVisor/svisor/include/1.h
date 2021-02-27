#define ID_MAIN_TITLE 1

#define ID_REMOTE_PATH 2
#define ID_USE_LOCAL_PATH_FOR_PAGE_TEXT 3

#define ID_TIMER_APP 4

#define ID_TAB 5
#define ID_SEP_DEC 6

#define ID_HIDE_PART 7
#define ID_HIDE_TIME 8

#define ID_TIMER_4_EDIT 9

#define ID_TITLE_RESULT_PSW 10
#define ID_MSG_RESULT_PSW   11
#define ID_TITLE_INPUT_PSW 12

#define ID_READ_ONLY_PROGRAM 13

#define ID_HEADER_TEXT_FILE 14

#define ID_MSG_WAIT_TX 15
#define ID_MSG_WAIT_RX 16

#define ID_MSG_ERR_TITLE 17
#define ID_MSG_ERR_WAIT_TX 18
#define ID_MSG_ERR_WAIT_RX 19

#define ID_MSG_ERR_WAIT_TIMEOUT 20
#define ID_MSG_ERR_WAIT_FAILED 21

#define ID_USE_PREDEF_BTN  22

#define ID_RESOLUTION_BASE 23

#define ID_LANGUAGE 24
    #define MAX_LANG 10

#define ID_DLL_NAME 25

#define ID_FORCE_DELETE_COMMAND_TITLE 26
#define ID_FORCE_DELETE_COMMAND 27

#define ID_CONFIRM_SEND_TITLE 28
#define ID_CONFIRM_SEND_MSG   29

#define ID_DELAY_BETWEEN_PRPH 30

#define ID_ADDR_CURRENT_DATETIME 31
#define ID_ADDR_ALTERNATE_INPUT_CAUSE 32
//#define ID_MSG_IN_EXEC 30
//#define ID_TITLE_IN_EXEC 31

#define ID_CAM_NAME               33
#define ID_CAM_TIME_REG_ON_EXIT_PAGE  34

#define ID_MSG_PERIF_NOT_READY_TITLE 35
#define ID_MSG_PERIF_NOT_READY       36

#define ID_HIDE_BAR             37

#define ID_SHOW_INACTIVE_ALARM  38
#define ID_SHOW_COLUMN_TYPE_ALARM  39

#define ID_TITLE_ON_EMERG 41
#define ID_MSG_ON_EMERG   40

#define ID_END_TITLE 42
#define ID_END_MSG   43
#define ID_END_PSW   44

#define ID_USE_EXTENTED_KEYB 45
#define ID_USE_PREFIX_BTN    46

#define ID_USE_TOUCH_KEYB   47
#define ID_NUMB_TOUCH_KEYB  48

#define ID_ADDR_ALWAYS_LOCAL 49

#define ID_EXIST 50
#define ID_ALERT 51

#define ID_TITLE_PRPH_NOT_LOADED 52
#define ID_MSG_PRPH_NOT_LOADED   53

#define ID_FILE_SAVED 54
#define ID_FILE_NOT_SAVED 55

#define ID_TIT_NORMAL_EXPORT 56
#define ID_TIT_BINARY_EXPORT 57
//#define ID_MSG_ERR_AZ 60
#define ID_USE_ALTERNATE_BITMAP_4_TOOL 58

#define ID_COLOR_FONT_CHOOSE_PAGE 59

#define ID_PASSWORD_TIME_LEASE 60

#define ID_RUNDELAY 61

#define ID_TITLE_SET_TIME_G    62
#define ID_TIMER_SET_TIME_G    63

#define ID_TITLE_GET_PAGE_G    64
#define ID_GET_PAGE_G          65

#define USE_NUMBER_SEPARATOR   66

#define ID_TITLE_CHOOSE_NUM 70
#define ID_FROM_NUM         71
#define ID_TO_NUM           72
#define ID_TXT_MAX_NUM      73
#define ID_DIALOG_OK        74
#define ID_DIALOG_CANC      75

#define ID_DATA_HEADER 80
// la data può essere nel formato europeo [dd/mm/yyyy] -> 1,
// oppure americano [mm/dd/yyyy] -> 2, oppure altro [yyyy/mm/dd] -> 3
#define ID_DATA_TYPE   81

#define ID_INP_DATE_FROM 85
#define ID_INP_DATE_TO   86

#define ID_TIME_HEADER   87
#define ID_INP_TIME_FROM 88
#define ID_INP_TIME_TO   89

#define ID_PSW_CHAR 99

#define ID_FIRST_MONTH 100

#define ID_NO_MEM_PATH_PAGE 112

#define ID_TITLE_ALARM_HIST 119
#define ID_ALARM 120
#define ID_TITLE_ALARM_CURR 121

#define INIT_COD_ALARM 122

#define ID_SIZE_FONT_ALARM 130
#define ID_NAME_FONT_ALARM 131
#define ID_FIXED_WIDTH_FONT_ALARM 132

#define ID_FILTER_ALARM 135

#define ID_TXT_EXIT 140
#define ID_TXT_RESET  (ID_TXT_EXIT + 1)
#define ID_TXT_NO_ACTIVE_REP (ID_TXT_RESET + 1)
#define ID_TXT_ONLY_ALARM    (ID_TXT_NO_ACTIVE_REP + 1)
#define ID_TXT_ALL_REPORT    (ID_TXT_ONLY_ALARM + 1)
#define ID_TXT_SHOW_ALARM    (ID_TXT_ALL_REPORT + 1)
#define ID_TXT_SHOW_REPORT    (ID_TXT_SHOW_ALARM + 1)
#define ID_TXT_ADVANCED_REPORT (ID_TXT_SHOW_REPORT + 1)
#define ID_TXT_FILTER_TEXT (ID_TXT_ADVANCED_REPORT + 1)

//#define ID_ERR_AZ 150

#define ID_ALARM_REFRESH_MSG   151
#define ID_ALARM_REFRESH_TITLE 152

#define ID_NUM_PRF_PLC      154

#define ID_MAX_DELAY_RESPONCE 155

#define ID_TYPE_ADDRESS_PLC  156
#define ID_TYPE_ADDRESS_PRF2 157
#define ID_TYPE_ADDRESS_PRF3 158
#define ID_TYPE_ADDRESS_PRF4 159
#define ID_TYPE_ADDRESS_PRF5 160
#define ID_TYPE_ADDRESS_PRF6 161
#define ID_TYPE_ADDRESS_PRF7 162
#define ID_TYPE_ADDRESS_PRF8 163

#define ID_USE_MAPPING_BY_STD_MSG 164

#define ID_FORMAT_MSG_ERROR_DATA 165

#define ID_ERROR_INVALID_TYPE 166
#define ID_ERROR_INVALID_ADDRESS 167
#define ID_ERROR_INVALID_PERIF 168
#define ID_ERROR_DATA_UNKNOW 170
#define ID_INIT_TYPE_VAR 171

#define ID_INIT_NAME_PERIF 190

// nome file report mensile extra, si dovrebbe usare la stessa
// directory del report degli allarmi ed usare l'estensione ".ext"
#define NAME_FILE_EX_MONTH   _T("MH")

#define NAME_FILE_REP_ALARM   _T("AL")
#define NAME_FILE_REP_ALARM2   _T("AR")
#define NAME_FILE_GLOB_ALARM  _T("STAT")
//#define NAME_FILE_REP_ALARM   _T("ALmmaaaa")
//#define NAME_FILE_GLOB_ALARM  _T("STATaaaa")

// sono stati spostati nell'implementazione,
// generalmente nel file 5500.h
//#define ID_EXP_TITLE_INIT 200
//#define ID_EXP_TITLE_REP_ALARM 201
//#define ID_EXP_TITLE_END  206

#define ID_TITLE_HISTORY  209

#define ID_EXP_ALARM1     210
#define ID_EXP_ALARM_END  217

#define ID_REP_CAUSA  220
#define ID_COD_REP_ALARM  221
#define ID_REP_ALARM  222
#define ID_REP_DATA   223
#define ID_REP_ORA    224
#define ID_DATA_TYPE_INIT 225
#define ID_DATA_TYPE_END  226
#define ID_DATA_TYPE_ACK  227


#define ID_EXP_SUCCESS_TITLE 230
#define ID_EXP_FAILED        231
#define ID_EXP_SUCCESS       232

#define ID_CREATE_FOLDER     240
#define MAX_ID_CREATE_FOLDER  11
#define MAX_LEN_NAME_FOLDER   20

// stampe e preview
#define ID_TITLE_LOGO_GRAPH 300
// se non esiste
#define ID_BMP_LOGO_GRAPH   301

#define ID_PREVIEW_PAGE_TITLE 302
#define ID_PREVIEW_PAGE_NUM   303

#define ID_MSG_PRINTER_DRIVER   305
#define ID_MSG_PRINT_PAGE_WAIT  306
#define ID_MSG_PRINT_PAGE       307
/**/
// gestione manutenzioni
// viene mantenuta globale perché utilizzata
// sia dal body di configurazione che dal gestore.
// I testi del menù per il body va comunque in un file a parte
#define ID_FIRST_MAINT 400
// ... fino a max 490 codici manutenzione


// stringhe varie
#define ID_TITLE_LIST_MANUT 493
// stringhe da cui ricavare il numero di ore di alert o warning
#define ID_ALERT_HOURS      494
#define ID_WARNING_HOURS    495

// window di preset ore lavorate
#define ID_TITLE_MAINT_SET_TIME 496
#define ID_MAINT_SET_HOUR       497
#define ID_MAINT_SET_MINUTE     498

#define ID_TITLE_CONFIRM_RESET 499
#define ID_CONFIRM_RESET       500
/**/

#define ID_MANAGE_RUNNING 520
#define ID_MANAGE_EMERG   521
#define ID_MANAGE_RESET   530

#define ID_MANAGE_ALARM_PLC 531
//#define ID_MANAGE_INIT_WORD_PLC  532
//#define ID_MANAGE_WORD_PLC  550

#define ID_INIT_LABEL_PRINT_ALARM  540
#define ID_INIT_LABEL_PRINT_REPORT 550

#define ADDR_SAVE_SCREEN          558

#define ADDR_PRINT_SCREEN          559
#define ID_MARGIN_PRINTSCREEN      560

#define ID_RESET_BIT_AS_PING       561

#define SAVE_SCREEN_ONLY_ONE_FILE  562

#ifndef MAX_EXT_ONMEM
  #define WM_FIRST_EXT_ONMEM 10
  #define WM_LAST_EXT_ONMEM  20
  #define MAX_EXT_ONMEM (WM_LAST_EXT_ONMEM - WM_FIRST_EXT_ONMEM + 1)
#endif

#define ID_ADDR_ALWAYS_LOCALPRPH_MEM 570
#define LAST_ID_ADDR_ALWAYS_LOCALPRPH_MEM (ID_ADDR_ALWAYS_LOCALPRPH_MEM + MAX_EXT_ONMEM)

#define ID_PRINT_DATA_VAR          600


#define INIT_FIXED_DATA 800

#define ID_MAX_TREND_DATA_DIM 998

#define ID_GRAPH_MAX_REC_SHOW 999

#define INIT_TREND_DATA   1000

#define INIT_TRAY_DATA    1100
//#define INIT_DATA_RECIPE  1200


#define ID_MSG_NEED_CODE        5990
#define ID_ADDR_CODE_OPER       5991
//----------------------------------------------------------
#define ID_CAUSE_ALARM       6001
#define ID_CAUSE_NORMAL_STOP 6002
//----------------------------------------------------------
#define ID_CAUSE_TITLE_INIT 6011
#define ID_CAUSE_TITLE_END (ID_CAUSE_TITLE_INIT + 300)

#define ID_PRF_READER             6400
#define ID_NOT_OPER_DEFINED       6401
#define ID_NOT_OPER_DEFINED_TITLE 6402
#define ID_BIT_CARD               6403

#define ID_INIT_TEXT_HEADER_LB_ALARM 6411
#define ID_LAST_TEXT_HEADER_LB_ALARM 6419
#define MAX_TEXT_HEADER_LB_ALARM     (ID_LAST_TEXT_HEADER_LB_ALARM - ID_INIT_TEXT_HEADER_LB_ALARM + 1)
//----------------------------------------------------------
#define  ID_TITLE_FILESIZE_LIMIT  6430
#define  ID_MSG_FILESIZE_LIMIT    6431
//----------------------------------------------------------
#define  ID_CHOOSE_FILE_TITLE     7000
#define  ID_CHOOSE_FILE_DATE      7001
#define  ID_CHOOSE_FILE_DATE_ORD  7002
#define  ID_CHOOSE_FILE_REV_ORD   7003
#define  ID_CHOOSE_FILE_HIST      7004

//----------------------------------------------------------
#define  ID_TEXT_CHOOSE_RES_TITLE  7100
#define  ID_TEXT_CHOOSE_RES_PERSON 7101
#define  ID_TEXT_CHOOSE_RES_FORCE  7102
//----------------------------------------------------------
#define  ID_REPORT_FILTER_TITLE       8000
#define  ID_REPORT_FILTER_GBOX_DATE   8001
#define  ID_REPORT_FILTER_GBOX_ALARM  8002
#define  ID_REPORT_FILTER_ID_ALARM    8003
#define  ID_REPORT_FILTER_ID_GROUP    8004
#define  ID_REPORT_FILTER_ID_SECT     8005
#define  ID_REPORT_FILTER_ID_PRPH     8006
#define  ID_REPORT_FILTER_ORDER       8007
#define  ID_REPORT_FILTER_INFO_REC    8008

#define  ID_RP_LIMIT_REC_TITLE        8011
#define  ID_RP_LIMIT_REC_TOT          8012
#define  ID_RP_LIMIT_REC_SHOW_START   8013
#define  ID_RP_LIMIT_REC_SHOW         8014


#define  ID_RP_TYPE_SECT_FLT_TITLE    8020
#define  ID_RP_TYPE_SECT_FLT_AVAIL    8021
#define  ID_RP_TYPE_SECT_FLT_ACTIVED  8022
#define  ID_RP_TYPE_SECT_FLT_DESCR    8023
#define  ID_RP_TYPE_SECT_FLT_CODE     8024

#define ID_HEAD_ALARM_PRPH   8031
#define ID_HEAD_ALARM_TYPE   8032
#define ID_HEAD_ALARM_CODE   8033
#define ID_HEAD_ALARM_TYDESC 8034
#define ID_HEAD_ALARM_DATE   8035
#define ID_HEAD_ALARM_TIME   8036
#define ID_HEAD_ALARM_DESCR  8037

#define ID_HEAD_REPORT_EVENT  8041
#define ID_HEAD_REPORT_CODE   8042
#define ID_HEAD_REPORT_TYPE   8043
#define ID_HEAD_REPORT_TYDESC 8044
#define ID_HEAD_REPORT_DATE   8045
#define ID_HEAD_REPORT_TIME   8046
#define ID_HEAD_REPORT_DESCR  8047
