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

nMonitor ctrl = NULL;
nTask *jobTask = NULL;
FifoQueue queue = NULL;
int size = 0;
int submitted = 0;
int waiting = 0;
int stop = FALSE;

int runFun();

void startBatch(int n)
{
  submitted = 0;
  waiting = 0;
  stop = FALSE;
  ctrl = nMakeMonitor();
  queue = MakeFifoQueue();
  jobTask = nMalloc(n * sizeof(nTask));
  size = n;
  for (int i = 0; i < n; i++)
    jobTask[i] = nEmitTask(runFun);
}

void stopBatch()
{
  nEnter(ctrl);

  while (waiting < submitted)
  {
    nWait(ctrl);
  }
  stop = TRUE;
  nNotifyAll(ctrl);
  nExit(ctrl);
  for (int i = 0; i < size; i++)
  {
    nWaitTask(jobTask[i]);
  }
  //nFree(jobTask);
}

Job *submitJob(JobFun fun, void *input)
{
  Job *job = nMalloc(sizeof(Job));
  job->fun = fun;
  job->done = FALSE;
  job->input = input;
  nEnter(ctrl);
  PutObj(queue, job);
  submitted++;
  nNotifyAll(ctrl);
  nExit(ctrl);
  return job;
}

void *waitJob(Job *job)
{
  nEnter(ctrl);
  waiting++;
  nNotifyAll(ctrl);
  while (!job->done)
  {
    nWait(ctrl);
  }
  nExit(ctrl);
  return job->val;
}

// Ademas necesitara una funcion para el cuerpo de los n threads del sistema
// batch

int runFun()
{

  for (;;)
  {
    Job *jb = NULL;
    nEnter(ctrl);
    while (EmptyFifoQueue(queue) && !stop)
    {
      nWait(ctrl);
    }
    if (EmptyFifoQueue(queue) && stop)
    {
      nExit(ctrl);
      break;
    }

    jb = (Job *)GetObj(queue);
    nExit(ctrl);
    jb->val = (jb->fun)(jb->input);
    jb->done = TRUE;
    nEnter(ctrl);
    nNotifyAll(ctrl);
    nExit(ctrl);
  }
  return 0;
}
