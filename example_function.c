#include <stdio.h>
#include "coroutine.h"

void funcA(Schedule *s, void *args) {
  int *arr = (int *)args;

  for (int i = 0; i < 3; i++) {
    printf("funcA: %d\n", arr[i]);
    coroutine_yield(s);
  }
}

void funcB(Schedule *s, void *args) {
  int *arr = (int *)args;

  for (int i = 0; i < 3; i++) {
    printf("funcB: %d\n", arr[i]);
    coroutine_yield(s);
  }
}

int main(void) {
  Schedule *s = schedule_create();
  int args1[] = {1, 2, 3};
  int args2[] = {4, 5, 6};
  int id1 = coroutine_create(s, funcA, (void *)args1);
  int id2 = coroutine_create(s, funcB, (void *)args2);

  printf("start\n");
  coroutine_run(s, id1);
  coroutine_run(s, id2);

  for (; schedule_status(s) != S_FINISHED;) {
    coroutine_resume(s, id1);
    coroutine_resume(s, id2);
  }

  printf("end\n");
  schedule_close(s);

  return 0;
}