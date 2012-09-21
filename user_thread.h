#ifndef USER_THREAD_H_
#define USER_THREAD_H_

typedef int userthread_id;
void UserThreadInit(void);
int UserThreadCreate(userthread_id *thread,
    void* (*thread_func)(void*), void *arg);
void Loop();

#endif /* USER_THREAD_H_ */
