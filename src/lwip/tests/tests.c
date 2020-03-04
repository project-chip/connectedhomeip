// This is a sample unit test.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

//#include "standalone/TapInterface.c"

void TapInterface_Init_test1() {
  printf("---Running Test--- %s\n", __FUNCTION__);
  //assert(TapInterface_Init(NULL, NULL, NULL) == ERR_ARG);
}

int main() {
  printf("---Running Test--- tests from %s\n", __FILE__);
  TapInterface_Init_test1();
  return 0;
}