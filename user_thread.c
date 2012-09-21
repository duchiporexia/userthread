#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
#include <sys/time.h>
#include <unistd.h>
#include "user_thread.h"

#define kStackSize 32768
#define kUserThreadMaxNum 256

static userthread_id current = 0;

struct UserThread {
  int used;
  ucontext_t context;
  char stack[kStackSize];
  void* (*func)(void *arg);
  void *arg;
  void *exit_status;
};

static struct UserThread userthreads[kUserThreadMaxNum];
void IdleThread(void) {
  while (1)
      sleep(1000);
}

void _UserThreadInit(int tid) {
  getcontext(&userthreads[tid].context);
  userthreads[tid].context.uc_stack.ss_sp = userthreads[tid].stack;
  userthreads[tid].context.uc_stack.ss_size = sizeof(userthreads[tid].stack);
  userthreads[tid].context.uc_link = &userthreads[0].context;
}

void UserThreadInit(void) {
  _UserThreadInit(0);
  userthreads[0].used = 1;
  makecontext(&userthreads[0].context, IdleThread, 0);
}

void ScheduleThreads(void) {
  int i, prev;

  for (i = (current + 1) % kUserThreadMaxNum; i != current; i = (i + 1)
      % kUserThreadMaxNum)
    if (userthreads[i].used)
      break;
  if (i == current) {
    fprintf(stderr, "No userthread to be scheduled...\n");
    exit(1);
  }

  prev = current;
  current = i;
  swapcontext(&userthreads[prev].context, &userthreads[current].context);
}

void UserThreadMain(void) {
  void *exit_status = userthreads[current].func(userthreads[current].arg);
  userthreads[current].exit_status = exit_status;
  userthreads[current].used = 0;
}

int UserThreadCreate(userthread_id *thread, void* (*thread_func)(void*),
    void *arg) {
  static int last_used = 0;
  int i;

  for (i = (last_used + 1) % kUserThreadMaxNum; i != last_used; i = (i + 1)
      % kUserThreadMaxNum)
    if (!userthreads[i].used)
      break;
  if (i == last_used) {
    fprintf(stderr, "Too many userthreads to be created...\n");
    exit(1);
  }

  last_used = i;

  if (thread != NULL)
    *thread = i;
  _UserThreadInit(i);
  userthreads[i].used = 1;
  userthreads[i].func = thread_func;
  userthreads[i].arg = arg;
  userthreads[i].exit_status = 0;
  makecontext(&userthreads[i].context, UserThreadMain, 0);

  return 0;
}

void TimeCallback(int signo) {
  ScheduleThreads();
}

void Loop() {
  struct itimerval internal_timer;
  signal(SIGALRM, TimeCallback);
  internal_timer.it_interval.tv_sec = 0;
  internal_timer.it_interval.tv_usec = 10000;
  internal_timer.it_value.tv_sec = 0;
  internal_timer.it_value.tv_usec = 10000;
  setitimer(ITIMER_REAL, &internal_timer, NULL);

  IdleThread();
}

