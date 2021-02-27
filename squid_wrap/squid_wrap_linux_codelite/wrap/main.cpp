#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "pLanConn.h"
#include "base64.h"
//------------------------
void error(const char *msg)
{
    perror(msg);
//    exit(0);
}
//------------------------
int rem_nl(LPSTR buff)
{
  int len = strlen(buff);
  for(int i = len -1; i > 0; --i) {
    if((unsigned char)buff[i] <= ' ') {
      buff[i] = 0;
      --len;
      }
    else
      break;
    }
  return len;
}
//------------------------
int trySend(LPSTR s_buff, LPSTR r_buff, size_t dim, PLanConn& conn)
{
  int n = conn.write(s_buff,strlen(s_buff));
  if (n <= 0) {
    conn.close();
    conn.open();
    return 0;
    }
  n = conn.read(r_buff, dim);
  if (n <= 0) {
    conn.close();
    conn.open();
    return 0;
    }
  r_buff[n] = 0;
  n = rem_nl(r_buff);
  return n;
}
//------------------------
void performEncode(LPSTR target, LPSTR source)
{
  int len = rem_nl(source);
  len = base64_encode(target, source, len);
  target[len++] = '\n';
  target[len] = 0;
}
LPSTR ltrim(LPSTR buffer)
{
    LPSTR p = buffer;
    while(*p && (unsigned char)*p <= ' ')
        ++p;
    return p;
}
//------------------------
int main(int argc, char *argv[])
{
  static char buffer[4096 * 4] = {0};
  static char b[4096 * 4];
  if (argc < 4) {
     fprintf(stderr,"ERR usage %s typeHelper hostname port\n", argv[0]);
     exit(0);
    }
  PLanConn conn(argv[1], argv[2], atoi(argv[3]));
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);
  bzero(buffer, sizeof(buffer));
  while(fgets(buffer, sizeof(buffer) - 1, stdin) != NULL) {
    LPSTR p = ltrim(buffer);
    performEncode(b, p);
    if(!trySend(b, buffer, sizeof(b) - 1, conn)){
//       printf("trySend 1\n");
       if(!trySend(b, buffer, sizeof(b) - 1, conn)) {
          buffer[0] = 0;
//          printf("trySend 2\n");
          }
//      else
//          printf("trySend 2 ok\n");
      }
    if(buffer[0])
      printf("%s\n", buffer);
    bzero(buffer, sizeof(buffer));
    }
  conn.close();
  printf("End\n");
  return 0;
}
