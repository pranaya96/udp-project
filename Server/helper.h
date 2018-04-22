/*

  HELPER.H
  ========
  (c) Pranaya Adhikari, 2018
  

  Interface to socket helper functions. 

  Many of these functions are adapted from, inspired by, or 
  otherwise shamelessly plagiarised from "Unix Network 
  Programming", W Richard Stevens (Prentice Hall).

*/


#ifndef PG_SOCK_HELP
#define PG_SOCK_HELP


#include <unistd.h>
#include <stdio.h>             /*  for ssize_t data type  */

#define LISTENQ        (1024)   /*  Backlog for listen()   */


/*  Function declarations  */
int readTypeZero(char*, int, FILE*, char*);
int readTypeOne(char*, int, FILE*, char*);
void printToZeroType(FILE *, uint8_t , uint16_t *);
void printToOneType(FILE *, char* , int,  uint16_t *);

#endif  /*  PG_SOCK_HELP  */

