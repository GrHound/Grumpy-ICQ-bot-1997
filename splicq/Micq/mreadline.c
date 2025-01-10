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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <termios.h>
#include "micq.h"

#define HISTORY_LINES 10
#define HISTORY_LINE_LEN 1024


void R_init (void);
void R_setprompt (char *prompt);
void R_prompt (void);
void R_doprompt (char *prompt);
int R_process_input (void);
void R_getline (char *buf, int len);
void R_undraw (void);
void R_redraw (void);

static struct termios t_attr;
static void tty_prepare(void);
static void tty_restore(void);


static char *history[HISTORY_LINES+1];
static int history_cur = 0;
static char s[HISTORY_LINE_LEN];
static int cpos = 0;
static int clen = 0;
static int istat = 0;

void R_init (void)
{
	int	k;
	static int inited = 0;

	if (inited)
		return;
	for (k = 0; k < HISTORY_LINES+1; k++)
	{
		history[k] = (char *) malloc (HISTORY_LINE_LEN);
		history[k][0] = 0;
	}
	s[0] = 0;
	inited = 1;
	tty_prepare ();
	atexit (tty_restore);
}

void R_pause (void)
{
	tty_restore ();
}

void R_resume (void)
{
	tty_prepare ();
}

int R_process_input (void)
{
	char	ch;
	int	k;
	char s1[HISTORY_LINE_LEN];

	if (!read (STDIN_FILENO, &ch, 1))
		return 0;
	if (!istat)
	{
		if (ch == t_attr.c_cc[VERASE] &&
				t_attr.c_cc[VERASE] != _POSIX_VDISABLE)
		{
			if (cpos)
			{
#ifndef ANSI_COLOR
				s[--cpos] = 0;
				clen--;
				printf ("\b \b");
#else
				cpos--;
				clen--;
				strcpy (s + cpos, s + cpos+1);
				printf ("\b\033[K%s",s+cpos);
				if (cpos < clen)
					printf ("\033[%dD", clen-cpos);
#endif
			}
			return 0;
		}
		if (ch == t_attr.c_cc[VEOF] &&
				t_attr.c_cc[VERASE] != _POSIX_VDISABLE)
		{
			if (clen)
				return 0;
			strcpy (s,"q");
			printf ("\n");
			return 1;
		}
#ifdef	VREPRINT
		if (ch == t_attr.c_cc[VREPRINT] &&
				t_attr.c_cc[VERASE] != _POSIX_VDISABLE)
		{
			R_undraw ();
			R_redraw ();
			return 0;
		}
#endif	/* VREPRINT */
		if (ch >= 0 && ch < ' ')
		{
			switch (ch) {
				case '\n':
				case '\r':
					printf ("\n");
					history_cur = 0;
					strcpy (history[0], s);
					if (!s[0])
						return 1;
					if (strcmp (s, history[1]))
						for (k = HISTORY_LINES; k; k--)
							strcpy (history[k], history[k-1]);
					return 1;
				case 12: /* ^L */
					system ("clear");
					R_redraw ();
					break;
#ifdef ANSI_COLOR
				case 0x1b: /* ESC */
					istat = 1;
					break;
#endif
			}
		}
		else if (clen + 1 < HISTORY_LINE_LEN)
		{
			printf ("%c", ch);
#ifdef ANSI_COLOR
			printf ("%s", s + cpos);
			strcpy (s1, s + cpos);
			strcpy (s+cpos+1, s1);
			if (cpos < clen)
				printf ("\033[%dD", clen-cpos);
#endif
			s[cpos++] = ch;
			clen++;
			s[clen] = 0;
		}
		return 0;
	}
#ifdef ANSI_COLOR
	switch (istat)
	{
		case 1: /* ESC */
			if (ch == '[')
				istat = 2;
			else
				istat = 0;
			break;
		case 2: /* CSI */
			istat = 0;
			switch (ch)
			{
				case 'A': /* Up key */
				case 'B': /* Down key */
					if (	(ch == 'A' && history_cur >= HISTORY_LINES) ||
						(ch == 'B' && history_cur == 0))
					{
						printf ("\007");
						break;
					}
					k = history_cur;
					strcpy (history[history_cur], s);
					if (ch == 'A')
						history_cur++;
					else
						history_cur--;
					if (history[history_cur][0] || history_cur == 0)
					{
						strcpy(s, history[history_cur]);
						cpos = clen = strlen (s);
						R_undraw ();
						R_redraw ();
					}
					else
					{
						history_cur = k;
						printf ("\007");
					}
					break;
				case 'C': /* Right key */
					if (cpos == clen)
					{
						printf ("\007");
						break;
					}
					cpos++;
					printf ("\033[C");
					break;
				case 'D': /* Left key */
					if (!cpos)
					{
						printf ("\007");
						break;
					}
					cpos--;
					printf ("\033[D");
					break;
				default:
					printf ("\007");
			}
			break;
	}
#endif
	return 0;
}

void R_redraw (void)
{
	R_prompt ();
	printf ("%s", s);
#ifdef ANSI_COLOR
	if (cpos != clen)
		printf ("\033[%dD", clen-cpos);
#endif
}

void R_getline (char *buf, int len)
{
	strncpy (buf, s, len);
	cpos = 0;
	clen = 0;
	s[0] = 0;
}

static char *curprompt = NULL;
void R_setprompt (char *prompt)
{
	curprompt = prompt;
}

void R_prompt (void)
{
        
	if (curprompt)
		M_print ( curprompt);
}

void R_doprompt (char *prompt)
{
	R_setprompt (prompt);
/*	M_print( "\n\a" );*/
	R_prompt ();
}

void
R_undraw ()
{
	printf ("\r\033[K");
}

static struct termios saved_attr;
static attrs_saved = 0;

static void tty_restore (void)
{
	if (!attrs_saved)
		return;
	if (tcsetattr(STDIN_FILENO,TCSAFLUSH,&saved_attr) != 0)
		perror ("can't restore tty modes");
	else
		attrs_saved = 0;
}

static void tty_prepare (void)
{
	istat = 0;
	if (tcgetattr(STDIN_FILENO, &t_attr) != 0)
		return;
	saved_attr = t_attr;
	attrs_saved = 1;

	t_attr.c_lflag &= ~(ECHO|ICANON);
	t_attr.c_cc[VMIN] = 1;
	if (tcsetattr(STDIN_FILENO,TCSAFLUSH,&t_attr) != 0)
		perror ("can't change tty modes");
}

#endif /* USE_MREADLINE */
