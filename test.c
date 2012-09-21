#include <stdio.h>
#include "user_thread.h"

void* ThreadFunc(void *arg) {
  int i;
  for (i = 0; 1; i++) {
    if (i % 10000 == 0)
      printf("(%s):i =%d\n", (char*)arg, i);
  }
}
int main(int argc, char *argv[]) {
  userthread_id ut_id;
  UserThreadInit();
  UserThreadCreate(&ut_id, ThreadFunc, "I am thread 1");
  UserThreadCreate(&ut_id, ThreadFunc, "I am thread 2");
  Loop();
  return 0;
}
