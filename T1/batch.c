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

typedef struct
{
  nMonitor m;
  nTask *jTask;
  FifoQueue q;
  int size;
  int submitted;
  int waiting;
  int stop;
} Ctrl;

int runFun();

Ctrl *makeCtrl(int n)
{
  Ctrl *c = nMalloc(sizeof(*c));
  c->m = nMakeMonitor();
  c->jTask = nMalloc(n * sizeof(nTask));
  c->q = MakeFifoQueue();
  c->size = n;
  c->submitted = 0;
  c->waiting = 0;
  c->stop = FALSE;
  return c;
}

Ctrl *ctrl;

void startBatch(int n)
{
  ctrl = makeCtrl(n);
  for (int i = 0; i < n; i++)
    (ctrl->jTask)[i] = nEmitTask(runFun);
}

void stopBatch()
{
  nEnter(ctrl->m);

  while (ctrl->waiting < ctrl->submitted)
  {
    nWait(ctrl->m);
  }
  ctrl->stop = TRUE;
  nNotifyAll(ctrl->m);
  nExit(ctrl->m);
  for (int i = 0; i < ctrl->size; i++)
  {
    nWaitTask((ctrl->jTask)[i]);
  }
  //nFree(jobTask);
}

Job *submitJob(JobFun fun, void *input)
{
  Job *job = nMalloc(sizeof(Job));
  job->fun = fun;
  job->done = FALSE;
  job->input = input;
  nEnter(ctrl->m);
  PutObj(ctrl->q, job);
  ctrl->submitted++;
  nNotifyAll(ctrl->m);
  nExit(ctrl->m);
  return job;
}

void *waitJob(Job *job)
{
  nEnter(ctrl->m);
  ctrl->waiting++;
  nNotifyAll(ctrl->m);
  while (!job->done)
  {
    nWait(ctrl->m);
  }
  nExit(ctrl->m);
  return job->val;
}

// Ademas necesitara una funcion para el cuerpo de los n threads del sistema
// batch

int runFun()
{

  for (;;)
  {
    Job *jb = NULL;
    nEnter(ctrl->m);
    while (EmptyFifoQueue(ctrl->q) && !ctrl->stop)
    {
      nWait(ctrl->m);
    }
    if (EmptyFifoQueue(ctrl->q) && ctrl->stop)
    {
      nExit(ctrl->m);
      break;
    }

    jb = (Job *)GetObj(ctrl->q);
    nExit(ctrl->m);
    jb->val = (jb->fun)(jb->input);
    jb->done = TRUE;
    nEnter(ctrl->m);
    nNotifyAll(ctrl->m);
    nExit(ctrl->m);
  }
  return 0;
}
