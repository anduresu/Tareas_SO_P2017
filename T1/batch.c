#include <nSystem.h>
#include <fifoqueues.h>
//#include <hihoqueues.h>
#include "batch.h"

struct Job
{
  JobFun fun;
  int done;
  void *val;
  void input;
};

nMonitor ctrl;
nTask jobTask;
FifoQueue *waiting;

void startBatch(int n)
{
  ctrl = nMakeMonitor();
  jobTask = nEmitTask(runJobs);
  waiting = MakeFifoQueue();
}

void stopBatch(){
    ...}

Job *submitJob(JobFun fun, void *input)
{
  Job *job = (Job *)nMalloc(sizeof(Job));
  Job->fun = &fun;
  Job->done = FALSE;
  Job->input = input
      nEnter(ctrl);
  PutObj(waiting, job);
  nNotifyAll(ctrl);
  nExit(ctrl);
  return job;
}

void *waitJob(Job *job)
{
  nEnter(ctrl);
  while (!job->done)
  {
    nWait(ctrl);
  }
  nExit(ctrl);
  return job->val;
}

// Ademas necesitara una funcion para el cuerpo de los n threads del sistema
// batch

int runJobs(int n)
{
  for (;;)
  {
    int k = 0, i;
    JobFun *jfun_vec[n];
    Job *job_vec[n];
    nEnter(ctrl);
    while (EmptyFifoQueue(waiting))
    {
      nWait(ctrl);
    }
    while (!EmptyFifoQueue(waiting) && k < n)
    {
      job_vec[k] = (Job *)GetObj(waiting);
      jfun_vec[k] = job_vec[k]->fun;
      k++;
    }
    nExit(ctrl);

    for (int j = 0; j < k; j++)
    {
      job_vec[j]->val = *jfun_vec[k](job_vec[j]->input);
    }

    nEnter(ctrl);
    for (i = 0; i < n; i++)
    {
      job_vec[i]->done = TRUE;
    }
    nNotifyAll(ctrl);
    nExit(ctrl);
  }
  return 0;
}
