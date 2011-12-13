/*
 * =====================================================================================
 *
 *       Filename:  devices.cpp
 *
 *    Description:  Utilites for finding and handling the keyboard device.
 *
 *        Version:  1.0
 *        Created:  13/03/11 14:46:43
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  José M. González (imasen, txemagon), txema.gonz@gmail.com
 *        Company:  nova Web Studio. http://www.novaws.es
 *
 * =====================================================================================
 */

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "kb_device.h"
#include "logger.h"
#include <sys/ioctl.h>
#include <linux/input.h>
#include <fnmatch.h>

#define	DEVICE_PATH "/dev/input/"	/* Path to the input device drivers */
#define	STR_LEN 0x100		/* Max length for static strings  */

#define	WRONG_DEVICE false	/* This is not the keyboard device */
#define	KB_DEVICE true		/* This is the keyboard device */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  dev_path
 *  Description: Adds the devices path to the file name. 
 * =====================================================================================
 */
void
dev_path (char destiny[STR_LEN], const char *file_name)
{
  memset (destiny, '\0', STR_LEN);
  strcat (destiny, DEVICE_PATH);
  strcat (destiny, file_name);
}				/* -----  end of function dev_path(const char *file_name)  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  is_a_keyboard
 *  Description:  Guess if its a keyboard based on device name.
 * =====================================================================================
 */
bool
is_a_keyboard (const char file_name[])
{
  char abs_path[STR_LEN];
  char kb_event[STR_LEN];
  int dev = -1;
  char dev_name[STR_LEN];

  dev_path (abs_path, file_name);
  if (!(dev = open (abs_path, O_RDONLY | O_NONBLOCK)))
    return WRONG_DEVICE;

  ioctl (dev, EVIOCGNAME (sizeof (dev_name)), dev_name);
  close (dev);

  return fnmatch ("*keyboard*", dev_name, FNM_CASEFOLD) == 0;
}				/* -----  end of function is_keyboard  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  is_keyboard
 *  Description: Check wether the given device is fetching keyboard input.
 * =====================================================================================
 */
bool
is_keyboard (const char file_name[])
{
  char abs_path[STR_LEN];
  char kb_event[STR_LEN];
  int bytes_r;
  int dev = -1;
  char dev_name[STR_LEN];

  dev_path (abs_path, file_name);
  if (!(dev = open (abs_path, O_RDONLY | O_NONBLOCK)))
    return WRONG_DEVICE;

  ioctl (dev, EVIOCGNAME (sizeof (dev_name)), dev_name);
  fprintf (stderr, "\n(%s).\n", dev_name);
  fprintf (stderr, "%s => Opened.\n", abs_path);
  fprintf (stderr, "Enter new line to test the device.\n");
  getchar ();
  bytes_r = read (dev, kb_event, STR_LEN);
  close (dev);

  if (bytes_r > 0)
    return KB_DEVICE;


  return WRONG_DEVICE;
}				/* -----  end of function is_keyboard  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  open_keyboard_device
 *  Description:  Opens and returns the handle of the keyboard input device file.  
 * =====================================================================================
 */
int
open_keyboard_device ()
{
  /* -----  end of function open_keyboard  ----- */
  DIR *dev_dir;
  struct dirent *dir_entry;
  bool found_dev = false;
  char abs_path[STR_LEN];

  if (!(dev_dir = opendir (DEVICE_PATH)))
    exit_because ("Cannot access to input devices directory.", -2);

  while (!found_dev && (dir_entry = readdir (dev_dir)))
    {
      if ((int) dir_entry->d_type == DT_CHR)
	{
	  fprintf (stderr, "\nFound character device: %s.",
		   dir_entry->d_name);
	  found_dev = is_a_keyboard (dir_entry->d_name);
	  fprintf (stderr, "  Keyboard? => %s \n", found_dev ? "Yes" : "No");
	}
    }

  closedir (dev_dir);

  if (found_dev)
    {
      dev_path (abs_path, dir_entry->d_name);
      fprintf (stderr, "%s\n", abs_path);
      return open (abs_path, O_RDONLY);
    }
  return -1;
}
