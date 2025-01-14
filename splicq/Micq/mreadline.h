/*****************************************************
 * mreadline - small line editing and history code
 * Copyright (C) 1998 Sergey Shkonda (serg@bcs.zp.ua)
 *
 * This software is provided AS IS to be used in
 * whatever way you see fit and is placed in the
 * public domain.
 *
 * Author : Sergey Shkonda Nov 27, 1998
 *****************************************************/

#ifdef USE_MREADLINE

void R_init (void);			/* init mreadline lib */
void R_setprompt (char *prompt);	/* set prompt */
void R_prompt (void);			/* type prompt */
void R_doprompt (char *prompt);		/* = {R_setprompt(p);R_prompt() */
int R_process_input (void);		/* parse input, returns 1 if CR typed */
void R_getline (char *buf, int len);	/* returns line */
void R_undraw (void);			/* hide input */
void R_redraw (void);			/* unhide (redraw) input line */
void R_pause (void);			/* pause mreadline befor system () */
void R_resume (void);			/* resume ... */

#else /* USE_MREADLINE */

#define R_init() {}
#define R_setprompt(a) {}
#define R_prompt Prompt
#define R_doprompt M_print
#define R_process_input() 1
#define R_getline(buf,len) M_fdnreadln(STDIN,buf,len)
#define R_undraw() {M_print("\n");}
//#define R_undraw() {}
#define R_redraw() { Prompt(); }
#define R_pause() {}
#define R_resume() {}

#endif /* USE_MREADLINE */
