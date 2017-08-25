
#include "nSystem.h"
#include "batch.h"

typedef struct {
  int delay;
  void *tag;
  int startTime;
} Test;

static nMonitor m;
static int terminados;
static int lanzados;
static int chequeados;
static int waitTerminados;

static void *testFun(void *ptr) {
  Test *test= ptr;
  test->startTime= nGetTime();
  if (test->delay!=0)
    nSleep(test->delay);
  else {
    nEnter(m);
    terminados++;
    int punto= terminados%1000==0;
    nExit(m);
    if (punto) nPrintf(".");
  }
  return test;
}

static Job *submitTest(int delay) {
  Test *test= nMalloc(sizeof(Test));
  test->delay= delay;
  test->tag= test;
  return submitJob(testFun, test);
}

static void checkTest(Job *job) {
  Test *res= waitJob(job);
  if (res->tag!=res)
    nFatalError("checkTest", "El resultado %p es incorrecto\n", res);
  nFree(res);
}

int submitFun(int first, int step, int nr, Job **jobs) {
  for (int i=first; i<nr; i+=step) {
    jobs[i]= submitTest(0);
    nEnter(m);
    lanzados++;
    int dospuntos= lanzados%1000==0;
    nExit(m);
    if (dospuntos) nPrintf(":");
  }
  return 0;
}

int waitFun(int first, int step, int nr, Job **jobs) {
  for (int i= first; i<nr; i+=step) {
    nEnter(m);
    while (jobs[i]==NULL)
      nWait(m); // todavia no se crea el job, hay que esperar
    chequeados++;
    int guion= chequeados%1000==0;
    nExit(m);
    checkTest(jobs[i]);
    if (guion) nPrintf("-");
  }
  nEnter(m);
  waitTerminados++;
  nExit(m);
  return 0;
}

int waitTaskFun(nTask *submitters, int nsubmitters) {
  for (int j= 0; j<nsubmitters; j++) {
    nWaitTask(submitters[j]);
  }
  nPrintf("$");
  stopBatch();
  return 0;
}

int nMain() {
  m= nMakeMonitor();
  nPrintf("Test secuencial: 1 solo job a la vez\n");
  {
    startBatch(1);

    int ini= nGetTime();
    Job *r1= submitTest(100);
    Job *r2= submitTest(100);
    Job *r3= submitTest(100);

    checkTest(r1);
    checkTest(r2);
    checkTest(r3);
    int elapsed= nGetTime()-ini;
    nPrintf("Tiempo transcurrido: %d milisegundos\n", elapsed);
    if (elapsed<300 || elapsed>350)
      nFatalError("nMain",
          "Error: tiempo no esta entre [300, 350]\n",
          elapsed);

    nPrintf("\nDeteniendo el sistema batch\n");
    stopBatch();
  }
  nPrintf("----------------------------------------\n\n");

  nPrintf("Test paralelo: 2 jobs simultaneos\n");
  {
    startBatch(2);

    int ini= nGetTime();
    Job *r1= submitTest(100);
    Job *r2= submitTest(100);
    Job *r3= submitTest(100);

    checkTest(r1);
    checkTest(r2);
    checkTest(r3);
    int elapsed= nGetTime()-ini;
    nPrintf("Tiempo transcurrido: %d milisegundos\n", elapsed);
    if (elapsed<200 || elapsed>250)
      nFatalError("nMain",
          "Error: tiempo no esta entre [200, 250]\n",
          elapsed);

    stopBatch();
  }
  nPrintf("----------------------------------------\n\n");

  nPrintf("Test paralelo: 100 jobs y 10 threads\n");
  {
    int nr= 100;
    int ncor= 10;
    startBatch(ncor);

    Job *jobs[nr];
    int ini= nGetTime();
    for (int i= 0; i<nr; i++) {
      jobs[i]= submitTest(100);
    }

    for (int i= 0; i<nr; i++) {
      checkTest(jobs[i]);
    }
    int elapsed= nGetTime()-ini;
    nPrintf("Tiempo transcurrido: %d milisegundos\n", elapsed);
    if (elapsed<1000 || elapsed>1200)
      nFatalError("nMain",
          "Error: tiempo no esta entre [1000, 1200]\n",
          elapsed);

    stopBatch();
  }
  nPrintf("----------------------------------------\n\n");

  nPrintf("Test del orden de llegada: 10 jobs separados por 100 milisegundos\n");
  {
    int nr= 10;
    startBatch(1);
 
    int ini= nGetTime();
    Job *jobs[nr];
    jobs[0]= submitTest(1100);
    for (int i= 1; i<nr; i++) {
      nSleep(100);
      jobs[i]= submitTest(100);
    }

    int lastStartTime= 0;
    for (int i= 0; i<nr; i++) {
      Test *test= waitJob(jobs[i]);
      if (lastStartTime>test->startTime)
        nFatalError("nMain", "no se respeta orden de llegada\n");
      lastStartTime= test->startTime;
      nFree(test);
    }

    int elapsed= nGetTime()-ini;
    nPrintf("Tiempo transcurrido: %d milisegundos\n", elapsed);
    if (elapsed<2000 || elapsed>2200)
      nFatalError("nMain",
          "Error: tiempo no esta entre [2000, 2200]\n",
          elapsed);

    stopBatch();
  }
  nPrintf("----------------------------------------\n\n");

  nSetTimeSlice(1);

  nPrintf("Cada '.' corresponde a 1000 jobs lanzados\n");
  nPrintf("Cada ':' corresponde a 1000 jobs terminados\n");
  nPrintf("Cada '-' corresponde a 1000 waitJob completados\n");
  nPrintf("El '$' corresponde a la llamada de stopBatch\n\n");
  int njobs[]= {1000, 10000, 200000 };
  int ncores[]= {10, 100, 400 };

  for (int k= 0; k<3; k++) {
    int ncor= ncores[k]; // El numero de threads
    int nr= njobs[k];    // Numero de jobs
    nPrintf("test de robustez con %d threads y %d jobs\n", ncor, nr);

    startBatch(ncor);    // Se lanza el sistema batch con ncor threads
    lanzados= 0;         // jobs creados
    terminados= 0;       // jobs terminados
    chequeados= 0;       // jobs completados
    int nsubmitters= 2*ncor; // 2*ncor tareas crearan jobs
                         // ncor tareas se encargaran de completarlos
    waitTerminados= 0;   // tareas que invocan a waitTest terminadas

    int ini= nGetTime(); // hora de inicio
    Job *jobs[nr];       // los nr jobs que se crearan
    for (int j= 0; j<nr; j++)
      jobs[j]= NULL;     // se necesita que partan en NULL
    nPrintf("creando jobs\n");
    nTask submitters[nsubmitters], waitters[ncor];
    for (int j=0; j<nsubmitters; j++) {
      submitters[j]= nEmitTask(submitFun, j, nsubmitters, nr, jobs);
    }
    for (int j=0; j<ncor; j++) {
      waitters[j]= nEmitTask(waitFun, j, ncor, nr, jobs);
    }
    // Lanzamos una tarea encargada de enterrar los submitters y llamar
    // a stopBatch
    nTask waitsubtask= nEmitTask(waitTaskFun, submitters, nsubmitters);
    // Parte truculenta: los waitters llaman a nWait cuando encuentran
    // un job que aun no ha sido creado.  Cada 100 milisegundos llamamos
    // a nNotifyAll para que vuelvan a intentar completar el job
    while (waitTerminados!=ncor) {
      nEnter(m);
      nNotifyAll(m);
      nExit(m);
      nSleep(100);
    }
    // Enterramos los waitters aqui mismo
    nPrintf("\nesperando jobs\n");
    for (int j= 0; j<ncor; j++) {
      nWaitTask(waitters[j]);
    }
    nWaitTask(waitsubtask);
    int elapsed= nGetTime() - ini;
    nPrintf("\nTiempo transcurrido: %d milisegundos\n", elapsed);
  }

  nPrintf("Felicitaciones.  Su tarea paso todos los tests\n");
  return 0;
}
