#include "nSystem.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

int pedirHalf(nLRLock l, int timeout, int dt, int *ps) {
  int s= nHalfLock(l, timeout);
  nPrintf("Half lock val: %d \n",s);
  *ps= s;
  nPrintf("If none\n");
  if (s==NONE){
    nPrintf("Returning none\n");
    return NONE;
  }
  nPrintf("Sleep\n");
  if (dt!=0)
    nSleep(dt);
  nPrintf("half unlock pedir\n");
  nHalfUnlock(l, s);
  return s;
}

int pedirFull(nLRLock l, int timeout, int dt, int *ps) {
  nPrintf("Pidiendo full \n");
  int s= nFullLock(l, timeout);
  *ps= s;
  if (!s)
    return FALSE;
  if (dt!=0)
    nSleep(dt);
  nPrintf("Unlocking full \n");
  nFullUnlock(l);
  nPrintf("Unlocked full \n");
  return s;
}

int test1(int v) {
  nTask t1, t2, t3;
  int st1= NONE, st2= NONE, st3= FALSE;
  nLRLock l;
  int s1, s2;

  if (v)
    nPrintf("Comenzando test1: funcionamiento general\n");

  l= nMakeLeftRightLock();
  /* Se chequea que se entreguen las 2 mitades */
  s1= nHalfLock(l, -1);
  s2= nHalfLock(l, -1);
  if (!(s1==LEFT || s1==RIGHT) || !(s2==LEFT || s2==RIGHT))
    nFatalError("test1", "nHalfLock incorrectamente no retorno LEFT o RIGHT\n");
  if (s1==s2)
    nFatalError("test1", "nHalfLock retorno incorrectamente el mismo lado\n");
  if (v)
    nPrintf("Correcto: se otorgaron las 2 mitades\n");

  /* Se chequea que no se obtenga otro medio candado */
  t1= nEmitTask(pedirHalf, l, -1, 200, &st1);
  nPrintf("Termino t1\n");
  t2= nEmitTask(pedirHalf, l, -1, 400, &st2);
  nPrintf("Termino t2\n");
  nSleep(100);
  if (st1!=NONE || st2!=NONE)
    nFatalError("test1", "Se otorgo incorrectamente un medio candado que estaba ocupado\n");
  if (v)
    nPrintf("Correcto: no se otorgo ninguna otra mitad todavía.\n");
  nHalfUnlock(l, s1);
  nSleep(100);
  if (!(st1==LEFT || st1==RIGHT) && !(st2==LEFT || st2==RIGHT))
    nFatalError("test1", "nHalfLock incorrectamente no entrego LEFT o RIGHT\n");
  if (! (st1==NONE || st2==NONE))
    nFatalError("test1", "nHalfLock entrego un candado medio ocupado\n");
  nHalfUnlock(l, s2);
  nSleep(100);
  if (!(st1==LEFT || st1==RIGHT) && !(st2==LEFT || st2==RIGHT))
    nFatalError("test1", "nHalfLock incorrectamente no entrego LEFT o RIGHT\n");
  nPrintf("Pidiendo full");
  t3= nEmitTask(pedirFull, l, -1, 0, &st3);
  nSleep(100);
  if (st3)
    nFatalError("test1", "Se otorgo incorrectamente el candado medio ocupado\n");
  if (v)
    nPrintf("Correcto: el candado completo no fue otorgado todavía.\n");
  nWaitTask(t1);
  nWaitTask(t2);
  if (!(st1==LEFT || st1==RIGHT) && !(st2==LEFT || st2==RIGHT))
    nFatalError("test1", "nHalfLock incorrectamente no entrego LEFT o RIGHT\n");
  if (st1==st2)
    nFatalError("test1", "nHalfLock incorrectamente otorgo el mismo lado\n");
  nWaitTask(t3);
  if (!st3)
    nFatalError("test1", "nFullLock incorrectamente no entrego verdadero\n");
  if (v)
    nPrintf("Correcto: el candado completo fue otorgado\n");
  nDestroyLeftRightLock(l);
  return 0;
}

void test2() {
  nTask t1, t2, t3;
  /* Prueba 3 veces test1 en paralelo con un desfase de 33 milisegundos */
  nPrintf("Comenzando test2: funcionamiento independiente de los candados\n");
  t1= nEmitTask(test1, FALSE);
  nSleep(33);
  t2= nEmitTask(test1, FALSE);
  nSleep(33);
  t3= nEmitTask(test1, FALSE);
  nWaitTask(t1);
  nWaitTask(t2);
  nWaitTask(t3);
  nPrintf("Correcto: los 3 test1 funcionan en paralelo\n");
}

int test3(int v) {
  nTask t1, t2, t3, t4;
  int st1= -1, st2= -1, st3= -1, st4= -1;
  nLRLock l;
  int s1, s2;
  int start;
  if (v)
    nPrintf("Comenzando test3: timeouts\n");
  l= nMakeLeftRightLock();
  start= nGetTime();
  s1= nHalfLock(l, 500);
  s2= nHalfLock(l, 500);
  if (nGetTime()-start>50)
    nFatalError("test3", "nHalfLock toma incorrectamente demasiado tiempo\n");
  if (v)
    nPrintf("Correcto: las 2 mitades fueron otorgadas de inmediato\n");
  t1= nEmitTask(pedirHalf, l, 300, 400, &st1);
  t2= nEmitTask(pedirHalf, l, 300, 400, &st2); /* Uno de estos no se obtendra */
  nSleep(100);
  nHalfUnlock(l, s1);
  nSleep(100);
  /* s1 lo obtiene t1 o t2, pero no ambos.  El otro espera */
  if (!( (st1==s1 && st2==-1) || (st2==s1 && st1==-1) ))
    nFatalError("test3", "nHalfLock debio haber otorgado medio candado, pero no lo hizo\n");
  if (v)
    nPrintf("Correcto: se otorgo solo medio candado\n");
  t3= nEmitTask(pedirFull, l, 300, 400, &st3);
  t4= nEmitTask(pedirFull, l, 300, 400, &st4);
  nWaitTask(t1);
  nWaitTask(t2);
  nHalfUnlock(l, s2);
  nWaitTask(t3);
  nWaitTask(t4);
  /* s1 lo obtiene t1 o t2, pero no ambos.  El otro debio recibir NONE */
  if (!( (st1==s1 && st2==NONE) || (st2==s1 && st1==NONE) ))
    nFatalError("test3", "nHalfLock debio haber otorgado medio candado, pero no lo hizo\n");
  if (v)
    nPrintf("Correcto: una tarea recibio medio candado, en la otra se cumplio el timeout\n");
  /* el candado lo obtiene t3 o t4, pero no ambos.  El otro debio recibir NONE */
  if (!( (st3 && !st4) || (st4 && !st3) ))
    nFatalError("test3", "nFullLock debio haber otorgado el candado una sola vez, pero no lo hizo\n");
  if (v)
    nPrintf("Correcto: solo una de las tareas recibió el candado completo\n");
  
  nDestroyLeftRightLock(l);

  return 0;
}

void test4() {
  nTask t1, t2, t3;
  /* Prueba 3 veces test3 en paralelo con un desfase de 33 milisegundos */
  nPrintf("Comenzando test4: funcionamiento independiente de los candados con timeout\n");
  t1= nEmitTask(test3, FALSE);
  nSleep(33);
  t2= nEmitTask(test3, FALSE);
  nSleep(33);
  t3= nEmitTask(test3, FALSE);
  nWaitTask(t1);
  nWaitTask(t2);
  nWaitTask(t3);
  nPrintf("Correcto: los 3 test3 funcionan en paralelo\n");
}

int checkHalf(nLRLock l, int *pleft, int *pright, int *pcleft, int *pcright, int n) {
  while (n--) {
    int s= nHalfLock(l, -1);
    int i;
    int *ps;
    if (s==LEFT) {
      ps= pleft;
      (*pcleft)++;
    }
    else if (s==RIGHT) {
      ps= pright;
      (*pcright)++;
    }
    else
      nFatalError("check", "nHalfLock incorrectamente no retorna LEFT o RIGHT\n");
    if (*ps)
      nFatalError("check", "este lado esta siendo ocupado por otra tarea\n");
    *ps= TRUE;
    for (i=0; i<100; i++)
      nGetQueueLength(); /* Solo para gastar algo de CPU */
    *ps= FALSE;
    nHalfUnlock(l, s);
  }
  return 0;
}

#define N 100
#define M 30

int checkFull(nLRLock l, int *pleft, int *pright, int *pcleft, int *pcright, int n) {
  int k= 0;
  for (k==0; k<n; k++) {
    int i;
    nFullLock(l, -1);
    if (k==0 && *pcleft+*pcright>=N*M)
      nFatalError("checkFull", "Hambruna para full lock: se ejecutaron primero todos los half lock\n");
    if (*pleft || *pright)
      nFatalError("check", "un lado esta siendo ocupado por otra tarea\n");
    *pleft= *pright= TRUE;
    for (i=0; i<100; i++)
      nGetQueueLength(); /* Solo para gastar algo de CPU */
    *pleft= *pright= FALSE;
    nFullUnlock(l);
  }
  return 0;
}

int test5(int v) {
  nTask th[N], tf[N];
  int i;
  nLRLock l= nMakeLeftRightLock();
  int left= FALSE, right= FALSE;
  int cleft= 0, cright= 0; /* Contadores */
  if (v)
    nPrintf("Comenzando test5: test de resistencia con 1 l/r lock\n");
  for (i= 0; i<N; i++) {
    th[i]= nEmitTask(checkHalf, l, &left, &right, &cleft, &cright, M);
  }
  nSleep(1);
  for (i= 0; i<N; i++) {
    tf[i]= nEmitTask(checkFull, l, &left, &right, &cleft, &cright, M);
  }
  for (i= 0; i<N; i++) {
    nWaitTask(th[i]);
    nWaitTask(tf[i]);
  }
  nDestroyLeftRightLock(l);
  if (v)
    nPrintf("Correcto: test5 funciona\n");

  return 0;
}

#define P 30

void test6() {
  nTask t[P];
  int i;
  nPrintf("Comenzando test6: test de resistencia con %d l/r locks\n", P);
  for (i= 0; i<P; i++)
    t[i]= nEmitTask(test5, FALSE);
  for (i= 0; i<P; i++)
    nWaitTask(t[i]);
  nPrintf("Correcto: test6 funciona\n");
}

int nMain(int argc, char** argv) {

  nPrintf("Los primeros tests se ejecutan en modo non preemptive\n");
  test1(TRUE);
  test2();
  test3(TRUE);
  test4();


  nSetTimeSlice(1);

  nPrintf("Los ultimos tests se ejecutaran en modo preemptive\n");
  test5(TRUE);
  test6();

  return 0;
}
