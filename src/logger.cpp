/*
 * =====================================================================================
 *
 *       Filename:  logger.cpp
 *
 *    Description:  Utilities for logging.
 *
 *        Version:  1.0
 *        Created:  13/03/11 15:05:08
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  José M. González (imasen, txemagon), txema.gonz@gmail.com
 *        Company:  nova Web Studio. http://www.novaws.es
 *
 * =====================================================================================
 */
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  exit_because
 *  Description: Program works fine, but the necessary conditions are not given. 
 * =====================================================================================
 */
void
exit_because (const char *msg, int code)
{
  fprintf (stderr, "%s\n", msg);
  exit (code);
}				/* -----  end of function exit_because  ----- */



/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  exit_because_error
 *  Description:  The program exists because there is an error on a system call.
 *                Prints an error message along with the system error explanation.
 * =====================================================================================
 */
  void
exit_because_error ( const char *msg, int code )
{
   perror(msg);
   exit(code);
}		/* -----  end of function exit_because_error  ----- */
