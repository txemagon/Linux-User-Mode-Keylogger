/*
 * =====================================================================================
 *
 *       Filename:  logger.h
 *
 *    Description:  Utilities for logging.
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


void exit_because (const char *msg, int code); /* Program works, but conditions are not given. */
void exit_because_error (const char *msg, int code); /* Program works, but conditions are not given and a system call has failed. */

#ifdef __cplusplus
}
#endif

#endif

