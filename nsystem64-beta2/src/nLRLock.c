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

static void ReadyFirstTask(FifoQueue queue)
{
  nTask task = (nTask)GetObj(queue);
  if (task != NULL)
  {
    //CancelTask(task);
    nPrintf("tn: %s \n",task->taskname);
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
  current_task->status = WAIT_TASK;
  PushObj(l->wqueue, current_task);
  if (l->state == CLOSED)
  {
    nPrintf("Closed state\n");

    if (timeout > 0)
    {
      ProgramTask(timeout);
      nPrintf("Program task end\n");
      output = NONE;
    }
    else
    {
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
  ReadyFirstTask(l->wqueue);
  END_CRITICAL();
}

int nFullLock(nLRLock l, int timeout)
{
  START_CRITICAL();
  int output;
  current_task->status = WAIT_TASK;
  PutObj(l->wqueue, current_task);

  if (l->state == CLOSED)
  {
    if (timeout > 0){
    nPrintf("Full lock Closed state\n");
    ProgramTask(timeout);
    nPrintf("Full lock program task end\n");
    output = FALSE;
    }
    else {
      nPrintf("Full lock timeout < 0 \n");
      ResumeNextReadyTask();
    }

    if (l->state == OPEN)
    {
      l->state = CLOSED;
      output = TRUE;
    }
  }

  else{
    
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
  PushTask(ready_queue, current_task);
  ReadyFirstTask(l->wqueue);

  END_CRITICAL();
}

void nDestroyLeftRightLock(nLRLock l)
{
  DestroyFifoQueue(l->wqueue);
  nFree(l);
}
