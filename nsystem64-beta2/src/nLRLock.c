#include "nSysimp.h"
#include "fifoqueues.h"

#define OPEN 0     // Todo abierto
#define CLOSED 3   // Todo cerrado
#define L_CLOSED 2 // Solo Left abierto
#define R_CLOSED 1 // Solo Right abierto

typedef struct
{
  FifoQueue wqueue;
  int state;
} * nLRLock;

/* Evite que se defina nLRLock como void * en nSystem.h */
#define NOVOID_NLRLOCK

#include "nSystem.h"

static void ReadyFirstTask(FifoQueue queue, nTask task)
{
  if (task->has_timeout){
    nPrintf("Has timeout");
    DeleteObj(queue,task);
    CancelTask(task);
  }

  if ((task->status != READY) && (!QueryTask(ready_queue, task)))
  {
    nPrintf("task is half:%d \n",task->is_half_lock);
    nPrintf("tn: %s \n", task->taskname);
    if (QueryObj(queue, task))
      nPrintf("En cola \n");
    task->status = READY;
    PushTask(ready_queue, task);
  }
}

nLRLock nMakeLeftRightLock()
{
  nLRLock lock = (nLRLock)nMalloc(sizeof(*lock));
  lock->state = 0;
  lock->wqueue = MakeFifoQueue();
  return lock;
}

int nHalfLock(nLRLock l, int timeout)
{
  START_CRITICAL();
  int output;
  nTask task = current_task;
  task->is_half_lock = 1;
  PutObj(l->wqueue, task);
  if (l->state == CLOSED)
  {
    nPrintf("Closed state\n");

    if (timeout > 0)
    { task->has_timeout = 1;
      ProgramTask(timeout);
      nPrintf("Program task end\n");
      output = NONE;
    }
    else
    {
      task->has_timeout = 0;
      task->status = WAIT_SEND;
      nPrintf("timeout < 0 resumeNextReady \n");
      ResumeNextReadyTask();
    }

    if (l->state == OPEN)
    {
      nPrintf("Now open\n");
      l->state = L_CLOSED;
      output = LEFT;
    }

    else if (l->state == L_CLOSED)
    {
      nPrintf("Now l_closed \n");
      l->state = l->state | R_CLOSED;
      output = RIGHT;
    }

    else if (l->state == R_CLOSED)
    {
      nPrintf("Now r_closed \n");
      l->state = l->state | L_CLOSED;
      output = LEFT;
    }
  }

  else if (l->state == OPEN)
  {
    nPrintf("Open state \n");
    l->state = L_CLOSED;
    output = LEFT;
  }

  else if (l->state == L_CLOSED)
  {
    nPrintf("L closed \n");
    l->state = l->state | R_CLOSED;
    output = RIGHT;
  }

  else if (l->state == R_CLOSED)
  {
    nPrintf("R closed \n");
    l->state = l->state | L_CLOSED;
    output = LEFT;
  }

  nPrintf("Ending critical %d \n", output);
  END_CRITICAL();
  return output;
}

void nHalfUnlock(nLRLock l, int side)
{

  START_CRITICAL();
  nPrintf("Unlocking side: %d with state = %d \n", side, l->state);
  l->state = l->state ^ side;
  nPrintf("Ready first task, state=%d \n", l->state);
  nTask task = (nTask)GetObj(l->wqueue);
  if (task != NULL)
  {
    if (task->is_half_lock == 1 || l->state == OPEN)
    {
      ReadyFirstTask(l->wqueue, task);
    }
  }
  END_CRITICAL();
}

int nFullLock(nLRLock l, int timeout)
{
  START_CRITICAL();
  int output;
  nTask task = current_task;
  task->is_half_lock = 0;
  PutObj(l->wqueue, task);

  if (l->state == CLOSED)
  {
    if (timeout > 0)
    {
      task->has_timeout = 1;
      
      nPrintf("Full lock Closed state\n");
      ProgramTask(timeout);
      nPrintf("Full lock program task end\n");
      output = FALSE;
    }
    else
    { 
      task->has_timeout = 0;
      task->status = WAIT_SEND;
      
      nPrintf("Full lock timeout < 0 \n");
      ResumeNextReadyTask();
    }

    if (l->state == OPEN)
    {
      l->state = CLOSED;
      output = TRUE;
    }
  }

  else
  {

    l->state = CLOSED;
    output = TRUE;
  }
  END_CRITICAL();
  return output;
}

void nFullUnlock(nLRLock l)
{
  START_CRITICAL();
  l->state = OPEN;
  nTask task = (nTask)GetObj(l->wqueue);
  if (task != NULL)
    ReadyFirstTask(l->wqueue, task);
  END_CRITICAL();
}

void nDestroyLeftRightLock(nLRLock l)
{
  DestroyFifoQueue(l->wqueue);
  nFree(l);
}
