//+++2006-01-17
//    Copyright (C) 2006  Mike Rieker, Beverly, MA USA
//    EXPECT it to FAIL when someone's HeALTh or PROpeRTy is at RISk
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; version 2 of the License.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//---2006-01-17

/********************************************************************************************************************************/
/*																*/
/*  Simple multi-threaded test program												*/
/*  Checkpoint is written to testmtcp3.mtcp every 10 seconds									*/
/*  It uses pthread_create routine to create the threads									*/
/*																*/
/*  Four threads are created													*/
/*  First thread prints 1 11 21 31 41 51 61 ...											*/
/*  Second thread prints 2 12 22 32 42 52 62 ...										*/
/*  Third thread prints 3 13 23 33 43 53 63 ...											*/
/*  Fourth thread prints 4 14 24 34 44 54 64 ...										*/
/*  When checkpoint is restored, the counts should resume where they left off							*/
/*																*/
/********************************************************************************************************************************/

#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mtcp.h"

/* Set *loc to newval iff *loc equal to oldval */
/* Return 0 if failed, 1 if succeeded          */

static inline int atomic_setif_int (volatile int *loc, int newval, int oldval)

{
  char rc;

  asm volatile ("lock\n\t"
                "cmpxchgl %2,%3\n\t"
                "sete     %%al"
                : "=a" (rc)
                :  "a" (oldval), "r" (newval), "m" (*loc)
                : "cc", "memory");

  return (rc);
}

static void *thread_func (void *dummy);

int main ()

{
  int i;
  pthread_t thread_tid;

  mtcp_init ("testmtcp3.mtcp", 10, 1);

  for (i = 0; i < 3; i ++) {
    if (pthread_create (&thread_tid, NULL, thread_func, NULL) < 0) {
      fprintf (stderr, "error creating thread: %s\n", strerror (errno));
      return (-1);
    }
  }

  mtcp_ok ();
  thread_func (NULL);
  return (0);
}

static int volatile threadno = 0;

static void *thread_func (void *dummy)

{
  int count, delay;

  do count = threadno;
  while (!atomic_setif_int (&threadno, count + 1, count));

  count ++;

  while (1) {
    for (delay = 100; -- delay >= 0;) usleep (10);
    printf (" %d", count);
    fflush (stdout);
    count += 10;
  }

  return (NULL);
}
