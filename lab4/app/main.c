#include "lib.h"
#include "types.h"

int uEntry(void) {
	// For lab4.1
	// Test 'scanf' 
	// int dec = 0;
	// int hex = 0;
	// char str[6];
	// char cha = 0;
	// int ret = 0;
	// while(1){
	// 	printf("Input:\" Test %%c Test %%6s %%d %%x\"\n");
	// 	ret = scanf(" Test %c Test %6s %d %x", &cha, str, &dec, &hex);
	// 	printf("Ret: %d; %c, %s, %d, %x.\n", ret, cha, str, dec, hex);
	// 	if (ret == 4)
	// 		break;
	// }
	
	// For lab4.2
	// Test 'Semaphore'
	// int i = 4;

	// sem_t sem;
	// printf("Father Process: Semaphore Initializing.\n");
	// int ret = sem_init(&sem, 2);
	// if (ret == -1) {
	// 	printf("Father Process: Semaphore Initializing Failed.\n");
	// 	exit();
	// }

	// ret = fork();
	// if (ret == 0) {
	// 	while( i != 0) {
	// 		i --;
	// 		printf("Child Process: Semaphore Waiting.\n");
	// 		sem_wait(&sem);
	// 		printf("Child Process: In Critical Area.\n");
	// 	}
	// 	printf("Child Process: Semaphore Destroying.\n");
	// 	sem_destroy(&sem);
	// 	exit();
	// }
	// else if (ret != -1) {
	// 	while( i != 0) {
	// 		i --;
	// 		printf("Father Process: Sleeping.\n");
	// 		sleep(128);
	// 		printf("Father Process: Semaphore Posting.\n");
	// 		sem_post(&sem);
	// 	}
	// 	printf("Father Process: Semaphore Destroying.\n");
	// 	sem_destroy(&sem);
	// 	exit();
	// }

	// For lab4.3
	// TODO: You need to design and test the philosopher problem.
	// Note that you can create your own functions.
	// Requirements are demonstrated in the guide.
  // For Lab4.3.1
  int ret = 0;
  sem_t forks[5];
  // 信号量初始化
  for(int i = 0; i < 5; i++)
  {
    sem_init(&forks[i], 1);
  }
  // 创建四个子进程，加上父进程共5个进程，对应五位哲学家
  for(int i = 0; i < 4; i++)
  {
    if(ret == 0)
    {
      ret = fork();
    }
    // 父进程跳出循环
    else if(ret > 0)
    {
      break;
    }
  }
  // 获取进程ID
  int pid = getpid();
  // 计算哲学家ID
  int id = pid - 1;
  // 每个哲学家执行2次完整的"思考-进餐"循环
  for(int i = 0; i < 2; i++)
  {
    printf("Philosopher %d: think\n", id);
    sleep(128);
    if(id % 2 == 0)
    {
			sem_wait(&forks[id]);
			sem_wait(&forks[(id + 1) % 5]);
		}
		else
    {
			sem_wait(&forks[(id + 1) % 5]);
			sem_wait(&forks[id]);
		}
    // 进餐(拿起叉子)
		printf("Philosopher %d: eat\n", id);
		sleep(128);
    // 放下叉子
		sem_post(&forks[id]);
		sem_post(&forks[(id + 1) % 5]);
  }
  if(id != 0)
  {
    exit();
  }
  for(int i = 0; i < 5; i++)
  {
    sem_destroy(&forks[i]);
  }
  exit();
	return 0;
}
