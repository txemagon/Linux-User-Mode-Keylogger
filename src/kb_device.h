/*
 * =====================================================================================
 *
 *       Filename:  kb_device.h
 *
 *    Description:  Api for finding the kb device.
 *
 *        Version:  1.0
 *        Created:  13/03/11 14:50:58
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  José M. González (imasen, txemagon), txema.gonz@gmail.com
 *        Company:  nova Web Studio. http://www.novaws.es
 *
 * =====================================================================================
 */

#ifndef __KB_DEVICE_H__
#define	__KB_DEVICE_H_

#ifdef __cplusplus
extern "C" {
#endif


  int open_keyboard_device (); /* Return the file handle of the keyboard driver. */ 

#ifdef __cplusplus
}
#endif

#endif

