
typedef struct job Job;
typedef void *(*JobFun)(void *ptr);

void startBatch(int n);
void stopBatch();

Job *submitJob(JobFun fun, void *ptr);
void *waitJob(Job *job);

