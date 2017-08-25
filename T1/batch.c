#include <nSystem.h>
#include <fifoqueues.h>
//#include <hihoqueues.h>
#include "batch.h"

struct job {
  ... defina aca la estructura del tipo Job ...
};

void startBatch(int n) {
  ...
}

void stopBatch() {
  ...
}

Job *submitJob(JobFun fun, void *input) {
  ...
}

void *waitJob(Job *job) {
  ...
}

// Ademas necesitara una funcion para el cuerpo de los n threads del sistema
// batch

...
