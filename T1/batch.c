#include <nSystem.h>
#include <fifoqueues.h>
//#include <hihoqueues.h>
#include "batch.h"

struct job
{
  JobFun fun;
  int done;
  void *val;
  void *input;
};

nMonitor ctrl;
nTask jobTask;
FifoQueue waiting;

void startBatch(int n)
{
  ctrl = nMakeMonitor();
  waiting = MakeFifoQueue();
  jobTask = nEmitTask(runJobs);
}

void stopBatch()
{
}

Job *submitJob(JobFun fun, void *input)
{
  Job *job;
  job = nMalloc(sizeof(Job));
  job->fun = fun;
  job->done = FALSE;
  job->input = input;
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
    Job **job_vec = (Job **)nMalloc(n * sizeof(Job *));
    nEnter(ctrl);
    while (EmptyFifoQueue(waiting))
    {
      nWait(ctrl);
    }
    while (!EmptyFifoQueue(waiting) && k < n)
    {
      job_vec[k] = (Job *)GetObj(waiting);
      k++;
    }
    nExit(ctrl);
    for (int j = 0; j < k; j++)
    {
      job_vec[j]->val = (job_vec[j]->fun)(job_vec[j]->input);
    }
    nEnter(ctrl);
    for (i = 0; i < k; i++)
    {
      job_vec[i]->done = TRUE;
    }
    nNotifyAll(ctrl);
    nExit(ctrl);
    nFree(job_vec);
  }
  return 0;
}
