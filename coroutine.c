#include "coroutine.h"

/**
 * @brief create a schedule
 *
 * @return a new schedule
 */
Schedule *schedule_create() {
  Schedule *s = (Schedule *)malloc(sizeof(Schedule));

  s->coroutines =
      (Coroutine **)malloc(sizeof(Coroutine *) * MAX_COROUTINE_SIZE);
  memset(s->coroutines, 0, sizeof(Coroutine *) * MAX_COROUTINE_SIZE);
  s->running_coroutine = -1;
  s->max_index = 0;

  return s;
}

/**
 * @brief close a schedule
 *
 * @param s the schedule to close
 */
void schedule_close(Schedule *s) {
  for (int i = 0; i < s->max_index; i++) coroutine_delete(s, i);

  free(s->coroutines);
  free(s);
}

/**
 * @brief get status of a schedule
 *
 * @param s the schedule to get status
 * @return the status of the schedule
 */
enum ScheduleStatus schedule_status(Schedule *s) {
  if (s->running_coroutine != -1) return S_RUNNING;

  for (int i = 0; i < s->max_index; i++) {
    Coroutine *c = s->coroutines[i];

    if (c != NULL && c->status != C_DEAD) return S_RUNNING;
  }

  return S_FINISHED;
}

/**
 * @brief set wrapper of a coroutine's func
 *
 * @param s the schedule holds the coroutine
 */
void func_wrapper(Schedule *s) {
  int id = s->running_coroutine;

  if (id != -1) {
    Coroutine *c = s->coroutines[id];
    c->func(s, c->args);
    c->status = C_DEAD;
    s->running_coroutine = -1;
  }
}

/**
 * @brief create a new coroutine in a schedule
 *
 * @param s the schedule in which to create a new coroutine
 * @param func the function to execute in the coroutine
 * @param args the arguments of the func
 * @return the id of the new coroutine
 */
int coroutine_create(Schedule *s, coroutine_func func, void *args) {
  Coroutine *c = NULL;
  int i = 0;

  for (; i < s->max_index; i++)
    if (s->coroutines[i]->status == C_DEAD) break;

  if (s->max_index == i || c == NULL)
    s->coroutines[i] = (Coroutine *)malloc(sizeof(Coroutine));

  s->max_index++;
  c = s->coroutines[i];
  c->func = func;
  c->args = args;
  c->status = C_READY;
  getcontext(&(c->ctx));
  c->ctx.uc_stack.ss_sp = c->stack;
  c->ctx.uc_stack.ss_size = STACK_SIZE;
  c->ctx.uc_stack.ss_flags = 0;
  c->ctx.uc_link = &(s->main_ctx);
  makecontext(&(c->ctx), (void (*)(void))func_wrapper, 1, s);

  return i;
}

/**
 * @brief delete a coroutine in a schedule
 *
 * @param s the schedule in which to delete a coroutine
 * @param id the id of the coroutine to delete
 * @return the status of the operation, 1 for success, 0 for failure
 */
int coroutine_delete(Schedule *s, int id) {
  if (id < 0 || id > s->max_index) return 0;

  Coroutine *c = s->coroutines[id];

  if (c != NULL) {
    s->coroutines[id] = NULL;
    free(c);
  }

  return 1;
}

/**
 * @brief run a coroutine in a schedule
 *
 * @param s the schedule holds the coroutine to run
 * @param id the id of the coroutine to run
 */
void coroutine_run(Schedule *s, int id) {
  if (coroutine_status(s, id) == C_DEAD) return;

  Coroutine *c = s->coroutines[id];

  c->status = C_RUNNING;
  s->running_coroutine = id;
  swapcontext(&(s->main_ctx), &(c->ctx));
}

/**
 * @brief yield all coroutines in a schedule
 *
 * @param s the schedule to yield its all coroutines
 */
void coroutine_yield(Schedule *s) {
  int id = s->running_coroutine;

  if (id == -1) return;

  Coroutine *c = s->coroutines[id];

  c->status = C_SUSPEND;
  s->running_coroutine = -1;
  swapcontext(&(c->ctx), &(s->main_ctx));
}

/**
 * @brief resume a coroutine in a schedule
 *
 * @param s the schedule holds the coroutine to resume
 * @param id the id of the coroutine to run
 */
void coroutine_resume(Schedule *s, int id) {
  if (id < 0 || id >= s->max_index) return;

  Coroutine *c = s->coroutines[id];

  if (c != NULL && c->status == C_SUSPEND) {
    c->status = C_RUNNING;
    s->running_coroutine = id;
    swapcontext(&(s->main_ctx), &(c->ctx));
  }
}

/**
 * @brief get status of a coroutine in a schedule
 *
 * @param s the schedule holds the corouine to get status
 * @param id the id of the coroutine to get status
 * @return the status of the coroutine
 */
enum CoroutineStatus coroutine_status(Schedule *s, int id) {
  if (id < 0 || id >= s->max_index) return C_DEAD;

  Coroutine *c = s->coroutines[id];

  if (c == NULL) return C_DEAD;

  return c->status;
}