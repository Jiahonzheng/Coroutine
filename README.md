# Coroutine

> 基于 ucontex 的协程简单实现

## 说明

本项目不用于线上生产环境，仅用于学习。生产环境下，请使用 [libco](https://github.com/tencent/libco) 等其他协程库。

## 文件说明

```
.
├── coroutine.c         # 实现文件
├── coroutine.h         # 头文件
├── example_function.c  # 测试文件：函数交叉调用
└── ...
```

## 示例

```c
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
```

输出结果为：

```
start
funcA: 1
funcB: 4
funcA: 2
funcB: 5
funcA: 3
funcB: 6
end
```

## 相关资料

- [事件驱动与协程：基本概念介绍](https://zhuanlan.zhihu.com/p/31410589)
- [协程：posix::ucontext 用户级线程实现原理分析](https://anonymalias.github.io/2017/01/09/ucontext-theory)
- [github.com/cloudwu/coroutine](https://github.com/cloudwu/coroutine)
