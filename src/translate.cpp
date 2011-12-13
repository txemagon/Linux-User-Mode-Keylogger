#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/io.h>
#include "map_es.h"

#define KB_IO 0x60
#define KB_ST 0x64
#define SLEEP 50

#define MAKE_CODE 0

#define NOT_FOUND NULL
#define MAKE_CODE_FOUND NULL

char *translate(int code){
  int offset;
  static int pre_code = 0;

  if (pre_code)
    code += 0x100 * pre_code;

  if (code == 0xE0){
    pre_code = 0xE0;
    return NULL;
  } else
    pre_code = 0;

  /* MAYS */
  if ( code == ctrl[LEFT_SHIFT].make_code     || 
       code == ctrl[RIGHT_SHIFT].make_code    || 
       code == ctrl[LEFT_SHIFT].release_code  || 
       code == ctrl[RIGHT_SHIFT].release_code ||
       code == ctrl[BLOQ_MAYS].release_code )
    status[SHIFT] = !status[SHIFT];

  /* CTRL */
  if ( code == ctrl[LEFT_CONTROL].make_code     || 
       code == ctrl[RIGHT_CONTROL].make_code    || 
       code == ctrl[LEFT_CONTROL].release_code  || 
       code == ctrl[RIGHT_CONTROL].release_code ) 
    status[CTRL] = !status[CTRL];

  if ( code == ctrl[LEFT_CONTROL].make_code  || 
       code == ctrl[RIGHT_CONTROL].make_code ) 
    return ctrl[LEFT_CONTROL].symbol;

  if ( code == ctrl[LEFT_CONTROL].release_code  || 
       code == ctrl[RIGHT_CONTROL].release_code ) 
    return (char *) esp_sep;

  /* ALT GR */
  if ( code == ctrl[ALT_GR].make_code)
    status[SALT_GR] = true;

  if ( code == ctrl[ALT_GR].release_code)
    status[SALT_GR] = false;

  if ( code == ctrl[ALT_GR].make_code     || 
      code == ctrl[ALT_GR].release_code)
    return NULL;


  /* ALT */
  if ( code == ctrl[ALT].make_code     || 
      code == ctrl[ALT].release_code)  
    status[SALT] = !status[SALT];

  if ( code == ctrl[ALT].make_code ) 
    return ctrl[ALT].symbol;

  if ( code == ctrl[ALT].release_code ) 
    return (char *) esp_sep;

  /* ESC */
  if ( code == ctrl[ESC].release_code)
    return (char *) ctrl[ESC].symbol;

  /* END */

  for(offset=0; offset<sizeof(key_codes); offset++)
    if (key_codes[offset] == code)
      break;
  if (offset == sizeof(key_codes) ) // We haven't found it
    return NOT_FOUND;

  if (offset % 2 == MAKE_CODE ) // We haven't found it
    return MAKE_CODE_FOUND;

  return (char *) key_chars[offset / 2 * 4 + ( status[SALT_GR] ? 2 : 0 ) + (status[SHIFT] ? 1 : 0) ]; // We've found a release code and we paint it.
}

