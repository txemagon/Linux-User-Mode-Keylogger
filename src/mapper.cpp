/*
 * =====================================================================================
 *
 *       Filename:  mapper.cpp
 *
 *    Description:  Loads and handles keyboard mapping and keyboard status.
 *
 *        Version:  1.0
 *        Created:  15/03/11 13:54:15
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  José M. González (imasen, txemagon), txema.gonz@gmail.com
 *        Company:  nova Web Studio. http://www.novaws.es
 *
 * =====================================================================================
 */

#include "mapper.h"
#include <stdio.h>
#include <stdlib.h>
#include "logger.h"
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <regex.h>

#define STATUS_REG_SIZE 5

#define KEEP_STATUS	0x00
#define SHIFT 		0x01
#define ALT_GR 		0x02
#define ALT 		0x04
#define CTRL		0x08
#define BLOQ_MAYS	0x10

#define PAT_DEF_KC		"([^,]*),?\\s*([^,]*),?\\s*([^,]*),?\\s*([^,]*)\\s*"
#define DEF_KC_STATES	4	/* Keyboard states for each keycode */


#define	LINESIZEMAX 0x100	/* Max line length */

#define KB_PRESS 1
#define KB_RELEASE 0

#define COMPOSED 0
#define SHIFTED  1

#define SHIFT_MAX 0x10
#define MAX_KEYCODES 0x100

#define CTRL_ST "<CTRL>"
#define ALT_ST "<ALT>"

/* Keep track of the keyboard status */
unsigned int kb_status;

struct TKeyCode
{
  char ***keycode;		// Starts looking as a hotel. Each entrance 
  char size;
};				/* ----------  end of struct TKeyCode  ---------- */

typedef struct TKeyCode KeyCode;
KeyCode keyset;

struct TTranslation
{
  const char *input;
  const char *mapped;
  int kb_status;
  int keycode_number;
};				/* ----------  end of struct Translation  ---------- */

typedef struct TTranslation Translation;

Translation translation[] = {
  {"<ENTER>", "\n", KEEP_STATUS},
  {"<CTRL_L>", CTRL_ST, CTRL},
  {"<CTRL_R>", CTRL_ST, CTRL},
  {"<TAB>", "<TAB>\t", KEEP_STATUS},
  {"<DEL>", "<DEL>", KEEP_STATUS},  // Don't use \b. This is not an editor.
  {"<SHIFT_L>", "", SHIFT},
  {"<SHIFT_R>", "", SHIFT},
  {"<COMMA>", ",", KEEP_STATUS},
  {"<SPACE>", " ", KEEP_STATUS},
  {"NUL", "", KEEP_STATUS},
  {"<ALT>", ALT_ST, ALT},
  {"<ALT_GR>", "", ALT_GR},
  {"<BLOQ_MAYS>", "", BLOQ_MAYS},
  {"<UP>", "<↑>", KEEP_STATUS},
  {"<DOWN>", "<↓>", KEEP_STATUS},
  {"<LEFT>", "<←>", KEEP_STATUS},
  {"<RIGHT>", "<→>", KEEP_STATUS}
};


struct CharComp
{				// Refers to the fact of missing two characters. It doesn't have to be with the kb.
  char character;
  char shifted[2][SHIFT_MAX];	// 0: Composed Character. 1: Composed + shifted
};				/* ----------  end of struct CharComp  ---------- */

typedef struct CharComp CharComp;

CharComp *composition[MAX_KEYCODES][DEF_KC_STATES];

/* Examples of the composition data structure:
 *
 *  KC
 * +---+
 * | o-+--x--> NULL
 * +---+
 * | o-+--x-->         ALTGR A+S
 * +---+      +---+---+---+---+            CharComp
 * | o-+----->| o |   | o | o | KB_ST   +---------------+---------------+---------------+ 
 * +---+  `   +-+-+---+---+-+-+         | +---+---+---+ | +---+---+---+ | +---+---+---+ |
 *           Nor|  SHF      +---------->| | a | å | Å | | | u | ů | Ů | | | _ | ° | ° | | _ : is a (SPACE)
 *              |                       | +---+---+---+ | +---+---+---+ | +---+---+---+ |
 *              |                       +---------------+---------------+---------------+
 *              + - - - etc
 *
 * To avoid linked lists, last element is going to have al NUL character (kind of a string).
 * */

typedef struct KeyComp KeyComp;


char letters[40];

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  translate_kc
 *  Description: Translates the string in the key mapping file to the printable
 *               string. In example: <CTRL_L> turns into <CTRL>. It also associates
 *               the code number in the translation table. 
 * =====================================================================================
 */
int
translate_kc (int code_number, char *mapping_text)
{
  for (int i = 0; i < sizeof (translation) / sizeof (Translation); i++)
    if (strcmp (mapping_text, translation[i].input) == 0)
      {
	strcpy (mapping_text, translation[i].mapped);
	translation[i].keycode_number = code_number;
      }
  return strlen (mapping_text);
}				/* -----  end of function translate_kc  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  update_kb_status
 *  Description:  Update the keyboard status
 * =====================================================================================
 */
void
update_kb_status (int code, int value)
{
  int elements = sizeof (translation) / sizeof (Translation);
  for (int i = 0; i < elements; i++)
    if (code == translation[i].keycode_number)
      if (translation[i].kb_status != BLOQ_MAYS)
	if (value == KB_RELEASE)
	  kb_status &= ~translation[i].kb_status;
	else
	  kb_status |= translation[i].kb_status;
      else if (value == KB_RELEASE)
	kb_status ^= BLOQ_MAYS;
}				/* -----  end of function update_kb_status  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  is_a_letter
 *  Description:  
 * =====================================================================================
 */
bool
is_a_letter (int code)
{
  bool it_is = false;

  if (strlen (keyset.keycode[code][0]) == 0)
    return false;

  for (const char *p = letters; *p != '\0'; p++)
    if (strncmp (p, keyset.keycode[code][0], strlen (keyset.keycode[code][0]))
	== 0)
      it_is = true;

  return it_is;
}				/* -----  end of function is_a_letter  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  getCompose
 *  Description:  Check if there is a composition defined for this kc.
 * =====================================================================================
 */
CharComp * 
getCompose ( int code, int kb_status, int *old_kbstatus )
{
  CharComp *comp = NULL;

  if (kb_status > 3)
    return NULL;

  if (composition[code])
    if (composition[code][kb_status]){
      comp = composition[code][kb_status];
      *old_kbstatus = kb_status;
    }

  return comp;
}		/* -----  end of function getCompose  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  the_compose_of
 *  Description:  Returns the composition of a char if any, or the char itself.
 * =====================================================================================
 */
char *
the_compose_of ( CharComp *comp, int code, int offset, int *oldkb_status, int kb_status )
{
  char *composition = keyset.keycode[code][0];

  if (strlen(composition) == 0)
    return composition;
  if (offset > 2)
    return composition;

  while (comp->character != '\0'){
    if ( comp->character == composition[0] ){
      composition = comp->shifted[offset]; 
      *oldkb_status = kb_status;
    }
    comp++;
  }

  return composition;
}		/* -----  end of function the_compose_of  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  dispatch_kc
 *  Description: Takes care of the events, update keyboard status and returns the
 *               printable character in each case. 
 * =====================================================================================
 */
const char *
dispatch_kc (int code, int value)
{
  int offset = 0;
  static CharComp *compose = NULL;
  static int oldkb_status = kb_status;
  static bool half_key = false;

  update_kb_status (code, value);

  if (value == KB_PRESS){
    if (strcmp(keyset.keycode[code][0], CTRL_ST) == 0 )
      printf("%s", CTRL_ST);

    if (strcmp(keyset.keycode[code][0], ALT_ST) == 0 )
      printf("%s", ALT_ST);

    fflush(stdout);
    return "";
  }


  if (value != KB_RELEASE)
    return "";

  if (strlen(keyset.keycode[code][0]) == 0)
    return "";

  if (kb_status & SHIFT)
    offset = 1;

  if (half_key){
    half_key = false;
    //printf("-%c | %s | %s -", compose->character, compose->shifted[COMPOSED], compose->shifted[SHIFT]);
    return the_compose_of(compose, code, offset, &oldkb_status, kb_status);
  }


  compose = getCompose(code, kb_status, &oldkb_status);
  if (compose){
    half_key = true;
    return "";
  }

  if (kb_status & BLOQ_MAYS && is_a_letter (code))
    offset = abs (offset - 1);

  if (kb_status & ALT_GR)
    offset = 2;
  if (kb_status & ALT_GR && kb_status & SHIFT)
    offset = 3;

  return keyset.keycode[code][offset];
}				/* -----  end of function translate_kc  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  init_kc
 *  Description:  Inits a keycode struct and its compositions.
 * =====================================================================================
 */
  void
init_kc (KeyCode * kc, CharComp * composition[MAX_KEYCODES][DEF_KC_STATES])
{
  kc->size = 0;
  kc->keycode = NULL;
  for (int code = 0; code < MAX_KEYCODES; code++)
    for (int state = 0; state < DEF_KC_STATES; state++)
      composition[code][state] = NULL;	// This isn't necesary as long as it's declared global.
}				/* -----  end of function init_kc  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  add_kc
 *  Description:  Add a new entrance to the keycode struct
 * =====================================================================================
 */
  char ***
add_kc (const char *def_line)
{
  static int kcn = 0;		// Number of added keycodes
  char regex_errors[LINESIZEMAX];
  int regex_status;
  regmatch_t definitions[DEF_KC_STATES + 1];
  regex_t keyscan;		// Finding comma separated definitions
  size_t nmatch = DEF_KC_STATES + 1;	// Number of matches.
  char pre_translated_match[LINESIZEMAX];


  if ((regex_status = regcomp (&keyscan, PAT_DEF_KC, REG_EXTENDED)) != 0)
  {
    regerror (regex_status, &keyscan, regex_errors, LINESIZEMAX - 1);
    exit_because_error (regex_errors, EXIT_FAILURE);
  }

  /* A new keycode definition */
  keyset.keycode =
    (char ***) realloc (keyset.keycode, (keyset.size + 1) * sizeof (char **));
  keyset.keycode[keyset.size] =
    (char **) malloc (DEF_KC_STATES * sizeof (char *));

  if ((regex_status =
	regexec (&keyscan, def_line, DEF_KC_STATES + 1, definitions, 0) != 0))
  {
    regerror (regex_status, &keyscan, regex_errors, LINESIZEMAX - 1);
    exit_because_error (regex_errors, EXIT_FAILURE);
  }

  /* For each word found in the definition line: */
  for (int i = 1, last_state = 0; i <= DEF_KC_STATES; i++)
  {
    int sz_match = definitions[i].rm_eo - definitions[i].rm_so;
    if (sz_match > 0)
    {
      strncpy (pre_translated_match,
	  &def_line[definitions[i].rm_so], sz_match);
      pre_translated_match[sz_match] = '\0';
      sz_match = translate_kc (kcn, pre_translated_match);

      keyset.keycode[keyset.size][i - 1] = (char *) malloc (sz_match + 1);
      strcpy (keyset.keycode[keyset.size][i - 1], pre_translated_match);
      last_state++;
    }

    else
    {

      sz_match = strlen (keyset.keycode[keyset.size][last_state - 1]);
      keyset.keycode[keyset.size][i - 1] = (char *) malloc (sz_match + 1);
      strcpy (keyset.keycode[keyset.size][i - 1],
	  keyset.keycode[keyset.size][last_state - 1]);
    }
  }

  keyset.size++;
  kcn++;

  regfree (&keyscan);
  return keyset.keycode;
}				/* -----  end of function add_kc  ----- */

/* 
 * The way we load the keycodes.
 *
 * If it would be static, we'll be making something like this:
 *
 * char keycode[111][4][255];
 *
 * Where:
 *
 *   111: Number of keycodes.
 *   4: The number of printable chars for each keycode (normal, shifted, alt gr'ed, (shift + alt gr)ed ).
 *   255: Maximum length of the string printable. As long as we work with UTF-8 a character can be
 *        a multibyte sequence.
 *
 *  When we become dynamic, we do something like this:
 *
 *   keycode           (char **)                        (char *)
 *  +-------+         +---------+                      +--------+
 *  |   o---+-------->|    o----+--------------------->|    o---+------>"a"
 *  +-------+         |   kc 0  |                      +--------+
 *  (char ***)        +---------+                      |    o---+------>"A"
 *                    |    o----+-----> etc.           +--------+
 *                    |   kc 1  |                      |    o---+------>"æ"
 *                    +---------+                      +--------+
 *                    .         .                      |    o---+------>"Æ"
 *                    .   etc   .                      +--------+
 *
 *  
 *
 *  */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  count_keycodes
 *  Description:  Count the number of keycodes described in the file.
 * =====================================================================================
 */
  int
count_keycodes (FILE * kc_file)
{
  int count = 0;
  assert (kc_file);
  long int original_pos;	// Original position of the file
  int returns_together;		// Number of characters per keycode definition.
  char line_def[LINESIZEMAX];	// Buffer line for counting characters in each def.

  original_pos = ftell (kc_file);
  rewind (kc_file);
  do
  {
    fscanf (kc_file, " %*[^\n]%[\n]", line_def);
    returns_together = strlen (line_def);
    count++;
  }
  while (!feof (kc_file) && returns_together < 2);

  fseek (kc_file, original_pos, SEEK_SET);
  return count;
}				/* -----  end of function count_keycodes  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  dump_keymap
 *  Description:  Dumps current keymap to screen.
 * =====================================================================================
 */
  void
dump_keymap ()
{
  for (int i = 0; i < keyset.size; i++)
    printf ("%X:\t | %s | %s | %s | %s |\n", i, keyset.keycode[i][0],
	keyset.keycode[i][1], keyset.keycode[i][2], keyset.keycode[i][3]);
}				/* -----  end of function dump_keymap  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  number_of_comps
 *  Description:  Counts the number of compositions in a line.
 * =====================================================================================
 */
  int
number_of_comps (char line[])
{
  int nc = 1;			// Number of compositions = Number of commas + 1
  char line_cp[LINESIZEMAX];
  strcpy (line_cp, line);
  strtok (line_cp, ",");
  while (strtok (NULL, ","))
    nc++;

  return nc;
}				/* -----  end of function number_of_comps  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  create_comp
 *  Description:  Creates the matriz of the composition.
 * =====================================================================================
 */
  CharComp *
create_comp (int code, int kb_state, char buffer[])
{
  CharComp *matrix = NULL;
  char *b = buffer;
  char *p = buffer;
  int el = 0;

  int nc = number_of_comps (buffer);
  matrix = (CharComp *) calloc (nc + 1, sizeof (CharComp));

  while (p = strtok (b, ","))
  {
    b = NULL;
    sscanf (p, "%c%*[-]%[^-]%*[-]%[^\n]", &(matrix[el].character),
	matrix[el].shifted[COMPOSED], matrix[el].shifted[SHIFTED]);
    el++;
  }
  matrix[el].character = matrix[el].shifted[COMPOSED][0] =
    matrix[el].shifted[SHIFTED][0] = '\0';

  return matrix;
}				/* -----  end of function create_comp  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  parseCompositions
 *  Description:  Reads till the end of file reading compositions.
 * =====================================================================================
 */
  void
parseCompositions (FILE * pf)
{
  char buffer_line[LINESIZEMAX];
  int current_keycode;
  int current_kbstate;

  while (!feof (pf))
  {
    fscanf (pf, " %[^\n]%*[\n]", buffer_line);
    if (buffer_line[0] != '#')
    {
      if (buffer_line[0] == 'L')
	strcpy(letters, buffer_line + 1);
      sscanf (buffer_line, " %[^#]", buffer_line);
      if (buffer_line[0] == '(')
	sscanf (buffer_line + 1, " %i", &current_keycode);
      else
      {
	sscanf (buffer_line, " %i%*[:]", &current_kbstate);
	composition[current_keycode-1][current_kbstate] = create_comp (current_keycode, current_kbstate, buffer_line + 2);	// Never more than 1 digit kb_states
      }
    }
  }
}				/* -----  end of function parseCompositions  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  load_map
 *  Description:  Loads and parses the keymap. Library entry point.
 * =====================================================================================
 */
  void
load_map (const char *map_file)
{
  FILE *pm;			// Pointer to map file.
  char buffer_line[LINESIZEMAX];
  int keycodes_size;		// Number of keycodes defined.

  if (!(pm = fopen (map_file, "r")))
    exit_because_error ("Couldn't open map file.", EXIT_FAILURE);

  keycodes_size = count_keycodes (pm);
  init_kc (&keyset, composition);

  for (int line = 0; line < keycodes_size; line++)
  {
    fscanf (pm, " %[^\n]", buffer_line);
    add_kc (buffer_line);
  }

  parseCompositions (pm);
  fclose (pm);
}				/* -----  end of function load_map  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  unload_map
 *  Description:  Removes keycodes map from memory and its compositions.
 * =====================================================================================
 */
  void
unload_map ()
{
  /* Free words for each keycode */
  for (int kc = 0; kc < keyset.size; kc++)
  {
    for (int word = 0; word < DEF_KC_STATES; word++)
    {
      char *same_word = keyset.keycode[kc][word];
      free (keyset.keycode[kc][word]);
      for (int next_similars = word + 1; next_similars < DEF_KC_STATES;	// We accept corrupted double linked
	  next_similars++)	// lists, despite glibc doesn't.
	if (keyset.keycode[kc][next_similars] == same_word)
	  word++;
    }
    free (keyset.keycode[kc]);
  }
  free (keyset.keycode);
  keyset.size = 0;

  /* Free the compositions */
  for (int code = 0; code < MAX_KEYCODES; code++)
    for (int state = 0; state < DEF_KC_STATES; state++)
      if (composition[code] != NULL)
	if (composition[code][state] != NULL)
	  free (composition[code][state]);
}				/* -----  end of function destroy_map  ----- */
