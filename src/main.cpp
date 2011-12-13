/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:  Parses command line and starts the keylogger
 *
 *        Version:  1.0
 *        Created:  18/03/11 12:27:42
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  José M. González (imasen, txemagon), txema.gonz@gmail.com
 *        Company:  nova Web Studio. http://www.novaws.es
 *
 * =====================================================================================
 */

#include	<stdlib.h>
#include 	<unistd.h>
#include 	<locale.h>
#include 	"logger.h"
#include	"keylog.h"

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  main
 *  Description:  Command Line Interface
 * =====================================================================================
 */
  int
main ( int argc, char *argv[] )
{

  setlocale (LC_ALL, "");

  if (geteuid () != 0 && getuid () != 0)
    exit_because ("Not enough priveleges for running the keylogger."
		  "Try to run it as super user. ", -1);


  start_logging();

  return EXIT_SUCCESS;
}				/* ----------  end of function main  ---------- */
