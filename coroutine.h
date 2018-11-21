#ifndef COROUTINE_H_
#define COROUTINE_H_

#include <ucontext.h>

#define STACK_SIZE (1024 * 128)

enum ScheduleStatus { FINISHED, RUNNING };
enum CoroutineStatus { DEAD, READY, RUNNING, SUSPEND };

typedef struct Schedule {
  ucontext_t main_ctx;
  Coroutine **coroutines;
  int running_coroutine;
  int max_index;
} Schedule;

typedef void (*coroutine_func)(Schedule *s, void *args);

typedef struct Coroutine {
  coroutine_func func;
  void *args;
  ucontext_t ctx;
  enum CoroutineStatus status;
  char stack[STACK_SIZE];
} Coroutine;

/**
 * @brief create a schedule
 *
 * @return a new schedule
 */
Schedule *schedule_create();

/**
 * @brief close a schedule
 *
 * @param s the schedule to close
 */
void schedule_close(Schedule *s);

/**
 * @brief get status of a schedule
 *
 * @param s the schedule to get status
 * @return the status of the schedule
 */
enum ScheduleStatus schedule_status(Schedule *s);

/**
 * @brief create a new coroutine in a schedule
 *
 * @param s the schedule in which to create a new coroutine
 * @param func the function to execute in the coroutine
 * @param args the arguments of the func
 * @return the id of the new coroutine
 */
int coroutine_create(Schedule *s, coroutine_func func, void *args);

/**
 * @brief delete a coroutine in a schedule
 *
 * @param s the schedule in which to delete a coroutine
 * @param id the id of the coroutine to delete
 * @return the status of the operation, 1 for success, 0 for failure
 */
int coroutine_delete(Schedule *s, int id);

/**
 * @brief run a coroutine in a schedule
 *
 * @param s the schedule holds the coroutine to run
 * @param id the id of the coroutine to run
 */
void coroutine_run(Schedule *s, int id);

/**
 * @brief yield all coroutines in a schedule
 *
 * @param s the schedule to yield its all coroutines
 */
void coroutine_yield(Schedule *s);

/**
 * @brief resume a coroutine in a schedule
 *
 * @param s the schedule holds the coroutine to resume
 * @param id the id of the coroutine to run
 */
void coroutine_resume(Schedule *s, int id);

/**
 * @brief get status of a coroutine in a schedule
 *
 * @param s the schedule holds the corouine to get status
 * @param id the id of the coroutine to get status
 * @return the status of the coroutine
 */
enum CoroutineState coroutine_status(Schedule *s, int id);

#endif