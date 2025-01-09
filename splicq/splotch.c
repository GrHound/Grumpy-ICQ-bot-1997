
/*****************************************************************************
   splotch.c a robot type thing Version 2.0 alpha

   7/17/1992 Duane Fields (dkf8117@tamsun.tamu.edu) Splotch
   august 1997  Lambert Schomaker (schomaker@nici.kun.nl) Grumpy incarnation
******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h> 
#include <unistd.h>
#include "splicq.h"

#define tolow(A) (isupper(A)?(tolower(A)):(A))
#define LOGDIR "logs"


#define NAME      "Grumpy"    /* name of robot */
#define TOLD_ENOUGH_CHARS 80

#define HUMAN    0
#define COMPUTER 1
#define NAGENTS  2
#define NOAGENT -1

#define MAXSTR    4096
#define MAXFILNAM  256

#define HISTORY   20           /* number of slots in old key queue */
#define NOTOPIC        -1
#define SERIOUS_TOPIC   5
#define MAXPRIOR_TOPIC  9

#define NTEMPL_MAX    5000            /* maximum number of templates */
#define NRESPONSE_MAX  NTEMPL_MAX     /* maximum number of responses */
#define DICTFILE  "main.dict"  /* name of dictionary file */

/* #define SILENT */
#ifndef SILENT
#define DEBUG   1                /* debug flag */
#define VERBOSE 1              /* verbose errors */
#define PDEBUG
#else
#define DEBUG   0                /* debug flag */
#define VERBOSE 0              /* verbose errors */
#endif


#define dprintf if(Debug)printf

#define BLANK 32



/* Lambert's STM RC time constants and activation stuff */

#define ALPHA_OTHERS_TOPICS 0.4

/* own topics */

#define ALPHA_WORDS         0.4
#define ALPHA_PHRASES       0.2
#define ALPHA_EPISODES      0.1

#define ACT_INITIAL         0.5
#define INHIBIT_WORDS       -2.
#define INHIBIT_PHRASES     -4.
#define INHIBIT_EPISODES   -10.

#define NREACT_MAX        4  /* number of 'keys' to react on maximally */
#define REACT_THRESHOLD 1.1  /* scaled as act * priority */
#define REACT_NOISE     0.4  /* noise in reacting on the keys */

/* ................................................................. */

/* technical globals */

int    Debug = DEBUG;
FILE   *dfile;       /* file pointer to main dictionary file */

/* types */

/* The following structure is for the detection of emotionality
   in the human's input, merely on the basis of surface text
   features */

typedef struct {
	int nexclama;       /* number of exclamation signs ! */
	int nquestion;      /* number of question marks ? */
	int nperiods;       /* number of periods . */
	int nperiod_last;   /* number of periods at the end . */
	int ncommas;        /* number of commas */
	int nwords;         /* number of words */
	int ngood_words;    /* number of friendly words */
	int nbad_words;     /* number of bad words */
	int nletters;       /* number of letters */
	int nupper;         /* number of upper-case letters */
	int nupper_first;   /* number of upper-case first letters */
	int ndigits;        /* number of digits */
	int ntacit;         /* number of silent (<Enter>) lines */
	int nanswers;       /* number of answers */
	int nanswers_since; /* number of answers since previous meta reaction */
	int nidentical_row; /* number of identical questions in a row */
	
	int nadd;           /* number of measurements (to calc avgs) */
	
	double dt;          /* time between responses */
	
        char previous_str[MAXSTR];
} Text_affect;

/* The following struct contains the information for all topic
   templates in the file main.dict. Note that the responses to a 
   keyword are not stored in memory: only the keys (lemmas). Instead,
   an offset for fseek is kept to the first response of a topic in
   the dict file. This should be improved with malloced strings later. */

typedef struct {
  long    toffset;           /* start of responses in file */
  char    tplate[MAXSTR];       /* the template itself (keywords) */
  char    last_question[MAXSTR];  /* the last question which triggered */
  char    last_words[MAXSTR];     /* and the % parts matched there  */
  int     priority;          /* how important the key is 1=worst, 9=most */
  int     ifreq;             /* how often used already                   */
  double  act;               /* current activation level                 */
  int     talts;             /* number of alternate replies (>0)         */
  int     tnext;             /* next reply (1 <= tnext <= talts)         */
} Template;

/* The following struct contains the internal state. This is not
   the same as the external state (the act value of responses which
   is kept in the .Act_* files). Ideally, for each human dialog
   partner, there is a state vector (intern/extern). This is not
   the case yet. The State struct is reset to initial values as 
   soon as a new human_id is detected, but the external act values
   are kept on disk, unchanged. Thus there may be a topic overflow
   between conversations with different humans.
   
   Summarizing:
   
   A  topic templates (keys) have an activation value 
      determining p(fire_template). The activation fluctuates 
      between 0. and 1.: Towards 1. if a topic has been activated,
      towards 0. if not used.
      
   B  --->  responses within topic templates have an act. 
      determining p(fire_response). The activation of a response is strongly 
      inhibited after firing, to prevent a response from popping up too 
      quickly again.
   
   This introduces two processes: topics are activated such that they can
   be referred to in (additional) ramblings after the main reply by the
   chatbot. On the other hand, individual responses and phrases just being
   uttered must be suppressed to prevent boredom in the human dialog partner.
   
   C The last mechanism concerning topic bookkeeping is a formal and
   sequential list containing the last #HISTORY topics. If no template
   has been found, either a random rambling or a random selection from
   the topics[] list is taken. For each topic, both the last original
   question and the derived '%' words string is kept for correct
   reference. This list _is_ reset between human_ID switches.
   
  */

typedef struct {
	char     my_nick[20];           /* my own nickname */
        char     current_human[MAXSTR]; /* the mortal's nickname or id */
	
	Template templ[NTEMPL_MAX];
	int      maxtempl;     /* templ[maxtempl] is last entry */

	int topics[HISTORY];   /* queue of indices of most recent keys */
	
        Text_affect cumul[NAGENTS];
        Text_affect now[NAGENTS];  
       
        time_t t_bot_response;
        time_t t_hum_question;
        int ncycles; 
        
        int ntaboo_words;
} State;


/* ..................................................................... */

#define UIN_IGN_FILE 	"ignorable_uin.dat"
#define UIN_ADD_IGN 	1
#define UIN_TEST_IGN 	0

int ignorable_rand_user(int iuin, int add1_or_test0) 
{
	FILE *fp;
	int i, iret;
	int had_it;
	
	fp = fopen(UIN_IGN_FILE,"r");
	if(fp == NULL) {
            if(add1_or_test0 == 1) {
		fp = fopen(UIN_IGN_FILE,"w");
		fprintf(fp,"%d\n", iuin);
		fclose(fp);
            }
	    iret = 0;
		
	} else {
		had_it = 0;
		while(fscanf(fp, "%d", &i) != EOF) {
			if(i == iuin) {
				had_it = 1;
				break;
			}
		}
		fclose(fp);
		if(! had_it) {
                   if(add1_or_test0 == 1) {
		      fp = fopen(UIN_IGN_FILE,"a");
		      fprintf(fp,"%d\n", iuin);
		      fclose(fp);
                      iret = 1;
                   } else {
		      iret = 0;
                   }
		} else {
		   iret = 1;
		}
	}
	return(iret);
}


char *simple_date(void)
{
        time_t tnow;

        time(&tnow);
        return(ctime(&tnow));
}

char *simple_time(void)
{
        time_t tnow;
        char str[100];
        static char out[100];

        time(&tnow);
        strcpy(str, ctime(&tnow));
        str[19] = '\0';
        strcpy(out, &str[11]);
        return(out);
}


char *full_time(void)
{
        time_t tnow;
        static char out[100];

        time(&tnow);
        strcpy(out, ctime(&tnow));
        return(out);
}

void str_tr_controls(char *str, char tr)
{
  int i;
  i = 0;
  while(str[i] != (char) 0) {
  	if(str[i] <= 32) {
  		str[i] = tr;
  	}
  	++i;
  }
}

void strlower(char *s)
{
   register int i;
   
   for (i=0; s[i]; ++i)
       s[i] = tolower (s[i]);
     
}
 

char *lower(char  *s)
{
  int i;
  static char tmp[MAXSTR];
  
  for (i=0; s[i]; ++i)
    tmp[i] = tolower(s[i]);
  tmp[i]='\0';
  return(tmp);
    
}

void remove_controlchars(char os_line[])
{
	int i;
	
	i = 0;
	while(i < MAXSTR && os_line[i] != (char) 0) {
		if((int) os_line[i] < 32) {
			 os_line[i] = (char) 32;
		}
		++i;
	}
}

void remove_controlchars_and_backquote(char os_line[])
{
	int i;
	
	remove_controlchars(os_line);
	
	i = 0;
	while(i < MAXSTR && os_line[i] != (char) 0) {
		if(os_line[i] == '`') {
			 os_line[i] = '\'';
		}
		++i;
	}
}


void cleanup_os_args(char os_line[])
{
	int i;
	
	remove_controlchars(os_line);

	i = 0;
	while(i < MAXSTR && os_line[i] != (char) 0) {
		
#define PUNCTUATIONS "!@#$%^&*()_+-={}[]|\\:\";'?/<>,.~`"

		if(strchr(PUNCTUATIONS, os_line[i]) != NULL) {
			os_line[i] = (char) 32;
		}
		++i;
	}
}


void System(char os_line[]) 
{
       remove_controlchars_and_backquote(os_line);

       if(VERBOSE) printf("system(%s)\n", os_line);

       system(os_line);
}

int strlen_eff(char str[])
{
	int i, n;
	
	n = strlen(str);
	
	if(n < 1) {
		n = 0;
	} else {
		i = n - 1;
		while(i >= 0 && str[i] <= (char) 32) {
		   --i;
		}
		n = i + 1;
	}
	return(n);
}


int has_taboo_word(char str[])
{
	static char *words[] = {"piss"
	                       ,"shit"
	                       ,"fuck"
	                       ,"fucking"
	                       ,"stupid"
	                       ,"ass"
	                       ,"asshole"
	                       ,"cunt"
	                       ,"licker"
	                       ,"gay"
	                       ,"you are a homo"
	                       ,"homosexual"
	                       ,"lesbian"
	                       ,"pervert"
	                       ,""};
	register int i, ok;
	char tmp[MAXSTR];
	
        strcpy(tmp,str);
        strlower(tmp);
	
	ok = 0;
	i = 0;
	while(words[i][0] != (char) 0) {
		if(strstr(tmp,words[i]) != NULL) {
			ok = 1;
			break;
		}
		++i;
	}
	return(ok);
}

void store_question(char question[])
{
  FILE *fp;
  char edited_question[MAXSTR];
  int nc;
  
  if(!has_taboo_word(question)) {
   
    strcpy(edited_question, question);
    str_tr_controls(edited_question,' ');
    
    nc = strlen_eff(edited_question);
    if(nc > 0) {
	
	if(strchr(edited_question,'?') != NULL) {
		fp = fopen("words/human_questions.w","a");
		if(fp != NULL) {
			fprintf(fp,"%s\n", edited_question);
			fclose(fp);
		}
		
	} else if(strchr(edited_question,'!') != NULL) {
		fp = fopen("words/human_claims.w","a");
		if(fp != NULL) {
			fprintf(fp,"%s\n", edited_question);
			fclose(fp);
		}
			
	} else {
	    if(nc > 1 && 
	        (int) edited_question[0] > 32) {
	       
		fp = fopen("words/human_remarks.w","a");
		if(fp != NULL) {
			fprintf(fp,"%s\n", edited_question);
			fclose(fp);
		}
	    }
	}
    }
  }
}

void shell_sortr(double reals[], int iarsiz, int ipoint[], int itype)
{
        register int more, i, j, igap, imax, kk;

        if(itype != 1 && itype != -1) {
              fprintf(stderr,"%%shell_sortr, itype not 1 or -1\n");
              exit(1);
        }
              
/* Initialize index array IPOINT. */
  
        for(i = 0; i < iarsiz; i++) {
                ipoint[i] = i;
        }

        igap = iarsiz;
        while(igap > 1) {
                igap = igap/2;
                imax = iarsiz-igap;
                more = 1;
                while(more) {
                        more = 0;
                        for (i = 0; i < imax; i++) {
                            j = i+igap;
                            if((itype == 1 &&   
                                     reals[ipoint[j]] < reals[ipoint[i]]) ||
                               (itype == -1 &&
                                     reals[ipoint[j]] > reals[ipoint[i]])) {
              
                                        kk = ipoint[j];
                                        ipoint[j] = ipoint[i];
                                        ipoint[i] = kk;
                                        more = 1;
                            }
                        }
                }
        }
} /* end shell_sortr() */


int pick_winner(double activity[], int nentries)
{
  int i, try, nused, iret;
  int idx[NTEMPL_MAX];
  double norm, pcrit, p;
  
  /* The activity vector is sorted and treated as a 
     discrete probability distribution. From this
     distribution a random draw is taken */

  shell_sortr(activity,nentries,idx,-1);

  norm = 0.0;
  for(i = 0; i < nentries; ++i) {
    if(activity[idx[i]] > 0.0) {
       norm += activity[i];
    } else {
       break;
    }
  }
  nused = i;

  try = 5;
  while(try > 0) {
     pcrit = drand48();

     i = 0;
     p = 0.0;
     while(i < nused && p < pcrit) {
        p += activity[idx[i]] / norm;
        ++i;
     }
     if(i < nused) {
        break;
     } else {
     	--try;
     }
  }
  if(try <= 0) {
  	iret = -1;
  } else {
        iret = idx[i];

        dprintf("pcrit=%f p=%f norm=%f, took %d of %d\n"
                   ,pcrit,p,norm,idx[i],nentries);
  }
  return(iret);
}


int get_entry(FILE *fp,char type[],char str[],int maxstr)
{
   char buf[MAXSTR];	

   if(type[0] != (char) 0) {

      /* Multi-line records with separator keywords: Get next item */

      strcpy(str,"");
      if(fgets (str, maxstr, fp) == NULL) {
   	 return(0);
      }
   
      /* It is the first word of an entry, or a .type lemma */

      if(strstr(str,type) == str) { /* If it is a lemma, get the next word */
    	 if(fgets (str, maxstr, fp) == NULL) {
   		return(0);
   	 } 
      }
   
      /* Just after .type lemma now */

      while(fgets (buf, MAXSTR, fp) != NULL) {
         if(strstr(buf,type) != buf) {
   	   strcat(str,buf);
   	 } else {
   	   break;
   	 }
      }
      return(1);
      
   } else {

   	/* Single-line records */

        if((fgets(str, maxstr, fp) != NULL) && (! feof(fp))) {
        	return(1);
        } else {
        	return(0);
        }
   }
}


int number_of_entries(char infile[MAXFILNAM])
{
   int n;
   char s1[MAXSTR];
   char fname[MAXFILNAM];
   char dir[MAXFILNAM];
   char sep[MAXFILNAM];
   FILE *fp;
   
   if(strstr(infile,".w") != NULL) {
   	strcpy(dir,"words");
   	strcpy(sep,"");

   } else if(strstr(infile,".e") != NULL) {
   	strcpy(dir,"episodes");
   	strcpy(sep,".episode");

   } else if(strstr(infile,".p") != NULL) {
   	strcpy(dir,"phrases");
   	strcpy(sep,".phrase");

   } else {
        fprintf(stderr, "\n ERROR! could not recognize :%s: \n",infile);
   	return(0);
   	
   }
   
   sprintf(fname,"%s/%s", dir,infile);
   fp = fopen(fname,"r");
   if(fp == NULL) {
   	fprintf(stderr, "\n ERROR! could not read :%s: \n",infile);
   	return(0);
   }

   n = 0;
   
   while(get_entry(fp,sep,s1,MAXSTR) == 1) {
      ++n;
   }
   fclose(fp);
   
   if(n > NRESPONSE_MAX) {
   	fprintf(stderr,"Error too many responses %d (max=%d) in file %s\n"
   	              , n , NRESPONSE_MAX, fname);
    	n = NRESPONSE_MAX;
   }

   return(n);
}


void read_activities(char type[],char infile[]
                                  ,double activity[],int *nentries)
{
  int i, nact;
  FILE *fpc;
  char cname[MAXFILNAM];
  
  
 /* Read the current activations of the responses in the *.w infile
    which are in the .Act-<infile>. If that file does not exist,
    initialize to activations of ACT_INITIAL */
  
  sprintf(cname,"%s/.Act-%s",type,infile);
    
  *nentries = number_of_entries(infile);

  if ((fpc=fopen(cname, "r")) == NULL)
  {
       for(i = 0; i < NRESPONSE_MAX; ++i) {
       	   activity[i] = ACT_INITIAL;
       }
       
  } else {
       fscanf(fpc,"%d", &nact);
       if(nact == *nentries) {
          for(i = 0; i < *nentries; ++i) {
       	     fscanf(fpc,"%lf\n", &activity[i]);
       	  }
       } else {
          for(i = 0; i < NRESPONSE_MAX; ++i) {
       	     activity[i] = ACT_INITIAL;
          }
       }
       fclose(fpc);
  }
}


void update_activities(
char type[],
char infile[MAXFILNAM],
double activity[],
int nentries,
int just_picked,  
double alpha,
double inhibit)
{
  int i;
  FILE *fpc;
  char cname[MAXFILNAM];
  
/* Update activities: a used response (just_picked) is inhibited,
   the others gets a higher activation according to a RC filter 
   with 1. as asymptote */

  for(i = 0; i < nentries; ++i) {
      if(i != just_picked) {
      	  activity[i] = alpha * 1. + (1. - alpha) * activity[i]; /* Excite */
      } else {
      	  activity[i] = inhibit; /* Inhibit */
      }
  }
  
/* Write to file */

  
  sprintf(cname,"%s/.Act-%s",type,infile);

  if ((fpc=fopen(cname, "w")) != NULL)
  {
       dprintf("Writing %d updated activities to %s\n", nentries, cname);
       
       fprintf(fpc,"%d\n", nentries);
       for(i = 0; i < nentries; ++i) {
       	   fprintf(fpc,"%5.2f\n", activity[i]);
       }
       fclose(fpc);
       
  } else { 
       fprintf(stderr,"Error writing activities to %s\n\n", cname);
  }
}       



void shift(int base, int delta, char words[])
{
	int     i, k;
	if (delta == 0)
		return;
	if (delta > 0) {
		k = base;
		while (words[k] != 0)
			k++;
		for (i = k; i >= base; i--)
			words[i + delta] = words[i];
	} else	/* delta <0 */
		for (i = 0; i == 0 || words[base + i - 1] != 0; i++)
			words[base + i] = words[base + i - delta];
}


/*
scan the array "words" for the string OLD.  if it occurs,
replace it by NEW.  also set the parity bits on in the
replacement text to  mark them as already modified
*/
void gswap(char    old[], char new[], char wordos[])
{     
	int     i, nlen, olen, flag, base, delim;
	olen = 0;
	while (old[olen] != 0)
		olen++;
	nlen = 0;
	while (new[nlen] != 0)
		nlen++;

	for (base = 0; wordos[base] != 0; base++) {
		flag = 1;
		for (i = 0; i < olen; i++)
			if (old[i] != wordos[base + i]) {
				flag = 0;
				break;
			}
		delim = wordos[base + olen];
		if (flag == 1 && (base == 0 || wordos[base - 1] == BLANK)
		    && (delim == BLANK || delim == '\n' || delim == 0)) {
			shift(base, nlen - olen, wordos);
			for (i = 0; i < nlen; i++) {
			   wordos[base + i] = new[i] + 128; /* mark subst */
	                }                
		}
	}
}

void clear_substitution_flags(char wordos[])
{
  int i;
  
  for (i=0; wordos[i] != '\0'; i++) {
    wordos[i] = wordos[i] & 127;
  }
}



void reverse_pronouns(char wordos[])
{
  
  gswap("that you", "that I", wordos);
  gswap("you say", "i say", wordos);
  gswap("i say", "you say", wordos);
  gswap("my", "your", wordos);
  gswap("your", "my", wordos);
  if(drand48() > 0.5) {
     gswap("you", "me", wordos);
  } else {
     gswap("you", "i", wordos);
  } 
  gswap("me", "you", wordos);
  gswap("mine", "yours", wordos);
  gswap("am", "are", wordos);
  gswap("yours", "mine", wordos);
  gswap("yourself", "myself", wordos);
  gswap("myself", "yourself", wordos);
  gswap("are", "am", wordos);
  gswap("i", "you", wordos);

  clear_substitution_flags(wordos);
 
}

void secondary_correction_reversed_pronouns(char wordos[])
{
  
  gswap("me had", "I had", wordos);
  gswap("me was", "i was", wordos);

  clear_substitution_flags(wordos);
 
}


/*********************************************************************
   substitute_pattern() takes a string pointer and two words.  
   All occurrences of the first word are replaced by the second word.
 *********************************************************************/
 
void substitute_pattern(char *s, char *old, char *new)
{
   char *n;
   char *n0;
   char *i;
   char *s0;
   char *new0;
  
   i=NULL;
   
   n=(char *) malloc(MAXSTR);   
   n0=n;
   s0=s;
   new0=new;
   
   while ((i = strstr(s,old)) != NULL)
         {
            while (s != i)
            {
               *n=*s;
               s++;
               n++;
            }
            
            if ( (isspace(i[strlen(old)]) || ispunct(i[strlen(old)]) ||
                  i[strlen(old)] == '\0')  && 
               (s==s0 || isspace(s[-1]) || ispunct(s[-1])) )
               
               {
                  new = new0;
                  while (*new)
                        {
                           *n=*new;
                           new++;
                           n++;
                        }
                  s += strlen(old);       
               }      
            else
               {
                  *n=*s;
                  s++;
                  n++;
               }   
         } 
         

   strcpy(n,s);
   strcpy(s0,n0);
   free(n0);
}


/*****************************/
/* gets random line from file*/
/*****************************/ 
int get_random_entry_words(char infile[], char s1[MAXSTR])
  
{
  int i, just_pick, nentries, iret;
  FILE *fp;
  char fname[MAXFILNAM];
  double activity[NRESPONSE_MAX];
  
  read_activities("words",infile,activity,&nentries);

  sprintf(fname, "words/%s", infile);
  
  just_pick = pick_winner(activity, nentries);
  if(just_pick >= 0) {
     if ((fp=fopen(fname, "r")) == NULL)
     {
        fprintf(stderr, "\n ERROR! could not open :%s: \n",infile);
        return(0);
     }

     for (i=0; i<= just_pick && (! feof(fp)); i++) {
        get_entry(fp, "", s1, MAXSTR);
     }
     fclose(fp);
      
     s1[strlen(s1)-1]='\0';
  
     iret = 1;
  } else {
     iret = 0;
  }  
  
  update_activities("words",infile,activity,nentries, just_pick
                           ,ALPHA_WORDS,INHIBIT_WORDS);
  return(iret);
  
}
 

/*****************************/
/* gets random episode from file*/
/*****************************/ 
int get_random_entry_episodes(char infile[MAXFILNAM], char s1[MAXSTR])
  
{
  int i,just_pick,nentries,iret;
  FILE *fp;
  char fname[MAXFILNAM];
  double activity[NRESPONSE_MAX];
  
  read_activities("episodes",infile,activity,&nentries);
  
  just_pick = pick_winner(activity, nentries);
  if(just_pick >= 0) {
     sprintf(fname, "episodes/%s", infile);
  
     if ((fp=fopen(fname, "r")) == NULL)
     {
        fprintf(stderr, "\n ERROR! could not open :%s: \n",infile);
        return(0);
     }

     for (i=0; i <= just_pick && (! feof(fp)); i++) {
        get_entry(fp,".episode",s1, MAXSTR);
     }
     fclose(fp);
        
     s1[strlen(s1)-1]='\0';

     iret = 1;
  } else {
     iret = 0;
  }
  update_activities("episodes",infile,activity,nentries, just_pick
                              ,ALPHA_EPISODES,INHIBIT_EPISODES);
                               
  return(iret);
  
}
 
/*****************************/
/* gets random phrase from file*/
/*****************************/ 
int get_random_entry_phrases(char infile[MAXFILNAM], char s1[MAXSTR])
  
{
  int i,just_pick,nentries,iret;
  FILE *fp;
  char fname[MAXFILNAM];
  double activity[NRESPONSE_MAX];
  
  read_activities("phrases",infile,activity,&nentries);
  
  just_pick = pick_winner(activity, nentries);
  if(just_pick >= 0) {
     sprintf(fname, "phrases/%s", infile);
  
     if ((fp=fopen(fname, "r")) == NULL)
     {
        fprintf(stderr, "\n ERROR! could not open :%s: \n",infile);
        return(0);
     }

     for (i=0; i <= just_pick  && (! feof(fp)); i++)
        get_entry(fp,".phrase",s1, MAXSTR);
     fclose(fp);
        
     s1[strlen(s1)-1]='\0';

     iret = 1;
   } else {
     iret = 0;
  }
  update_activities("phrases",infile,activity,nentries, just_pick
                             ,ALPHA_PHRASES,INHIBIT_PHRASES);
                             
  return(iret);
  
}


/******************************/
/* gets random entry from file*/
/******************************/ 

int get_random_entry(char infile[MAXFILNAM], char s1[MAXSTR])
{
   if(strstr(infile,".w") != NULL) {
   	return(get_random_entry_words(infile,s1));
   } else if(strstr(infile,".e") != NULL) {
   	return(get_random_entry_episodes(infile,s1));
   } else if(strstr(infile,".p") != NULL) {
   	return(get_random_entry_phrases(infile,s1));
   } else {
        fprintf(stderr, "\n ERROR! could not recognize :%s: \n",infile);
   	return(0);
   }
}

 
void gfile_truncate(char filename[], char str[])
{
	FILE *fp;
	int nout;
	
	fp = fopen(filename,"r");
	if(fp != NULL) {
		nout = fread(str, sizeof(char), MAXSTR,fp);
		fclose(fp);
		if(nout >= MAXSTR) {
			str[MAXSTR-4] = '\0';
			strcat(str,"...");
		} else {
			if(nout < 0) nout = 0;
			str[nout] = '\0';
		}
		
	} else {
		strcpy(str,"??");
	}
}

void chop_off_trailing_clauses(char words[])
{
    char *p;

    p=strstr(words, ",");          /* chop off trailing clauses */
    if (p != NULL)
       p[0]='\0';
    
    p=strstr(words, ". ");
    if (p != NULL)
      p[0]='\0';

    p=strstr(words, "...");
    if (p != NULL)
      p[0]='\0';

    p=strstr(words, "?");
    if (p != NULL)
      p[0]='\0';


    p=strstr(words, ";");                      
    if (p != NULL)
       p[0]='\0';

    p=strstr(words, "!");
    if (p != NULL)
       p[0]='\0';
    
    p=strstr(words, ":");
    if (p != NULL)
      p[0]='\0';

}

void fix_problems_with_grammar(char wordos[])
{  
  clear_substitution_flags(wordos);
  
  gswap("i are", "I am", wordos);
  gswap("me have", "I have", wordos);
  gswap("me don't", "I don't", wordos);
  gswap("you am", "you are", wordos);
  gswap("me am", "I am", wordos);
  gswap("me is", "I am", wordos);
  gswap("i", "I", wordos);
  gswap("r u ", "are you ", wordos);
  gswap("wher ", "where ", wordos);
  gswap("did not be", "weren't", wordos);

/* bad words but they need to be detected and interpreted */

  gswap("phuk", "fuck", wordos);
  
  clear_substitution_flags(wordos);
  
}

void make_words_suitable_for_substitution(char *words, State *mystate)
{
    char *p, *pp;	
    char text2[MAXSTR];
    char text[MAXSTR];

    chop_off_trailing_clauses(words);

    sprintf(text, " %s", lower(mystate->my_nick));
    sprintf(text2,"than %s", lower(mystate->my_nick));

    pp = strstr(words, text2);
    p = strstr(words, text);                        
    if ((p != NULL) && (pp == NULL))
       p[0]='\0';

    p = strstr(words, " please");
    if(p != NULL)
      p[0]='\0';

    p = strstr(words, "--");
    if(p != NULL && p != words)
      p[0]='\0';

    if (ispunct(words[strlen(words)-1]))
      words[strlen(words)-1]='\0';
}

/**************************************************************************
 usetempl() a template has been sucessfully matched.  generate output 
**************************************************************************/
void usetempl(
     int     i,
     char question[],
     char words[],
     char response[],
     State *mystate)

{
  int     k, n,p1, p2;
  char    text[MAXSTR];
  char    args[MAXSTR];
  char    add[MAXSTR];
  char    filename[MAXSTR];
  char    os_line[MAXSTR];
  char    tempname[MAXSTR];
  char    inwords[MAXSTR];
  char    new[MAXSTR];
  char    *eop;
  double  active;
  
  strcpy(inwords, words);

  dprintf("<<entering usetempl, i=%d>>\n", i);
    
/*+Update activity of this and the other templates */
  
  ++(mystate->templ[i].ifreq);
     
  for(k = 0; k < mystate->maxtempl; ++k) {
  	if(k == i) {
  		active = 1.0;
  	} else {
  		active = 0.0;
  	}
  	mystate->templ[k].act = active * ALPHA_OTHERS_TOPICS 
  	               + (1. - ALPHA_OTHERS_TOPICS) * mystate->templ[k].act;
  }
  
/*-Update activity*/

  fseek(dfile, mystate->templ[i].toffset, 0);	/* seek the template */
  fgets(text, sizeof(text), dfile);
       
  if (mystate->templ[i].tnext > mystate->templ[i].talts)
    mystate->templ[i].tnext = 1;
    
  n = mystate->templ[i].tnext;
  ++(mystate->templ[i].tnext);
  
  /* skip to the proper alternative */

  dprintf("<<usetempl, n=%d from main.dict>>\n", n);

  for (k = 1; k < n; k++) {
    fgets(text, sizeof(text), dfile);
  }
  
  /* make reply using template */
  
  if(text[0] != '*') {
  	fprintf(stderr,"Error: template %d alt %d has no * in 1st pos: [%s]\n"
  	              , i, n, text);
  }
  
  strcpy(response, text+1); /* skip '*' in first position */
  if (response[strlen(response)-1] == '\n') {
    response[strlen(response)-1]='\0';
  }

  dprintf("<<usetempl, raw response=[%s]>>\n", response);

  if (words[0] != '\0') {
  	
    make_words_suitable_for_substitution(words, mystate);

    dprintf("<<usetempl, fix grammar>>\n");

    reverse_pronouns(words);                           /* fix grammar */
    
    secondary_correction_reversed_pronouns(words);     /* fix the fixes */
    
    dprintf("<<usetempl, swap words>>\n");

    substitute_pattern (response, "%", words);
  }

  dprintf("<<usetempl, swapped response=[%s]>>\n", response);

  substitute_pattern(response, lower(mystate->my_nick), "I");          /* should use NAME */
  
  substitute_pattern(response, "%", " "); /* remove trailing % (this should not happen) */
  
  dprintf("<<usetempl, substituted response=[%s]>>\n", response);

  fix_problems_with_grammar(words);

  dprintf("<<usetempl, grammar response=[%s]>>\n", response);

  eop = strstr(question,": ");
  if(eop == NULL) {
     strcpy(args,question);
  } else {
     strcpy(args,eop+1); 
         /* assume string of the form: "Agent: questword questword..." */
  }
         
  cleanup_os_args(args);

  /* do file insertions */

  dprintf("<<usetempl, start file and live insertions>>\n");

  while (strstr(response, "@") != NULL) {
    p1=0;
    p2=0;
    strcpy (filename, "\0");
    strcpy (new, "\0");
    strcpy (add, "\0");
    
    while (response[p1] != '@') {
      new[p1] = response[p1++];  
    } 

    p1++;
    new[p1-1]='\0';
    
    if(strchr("<=", response[p1]) == NULL) { /* regular file */
    
       while (! (response[p1] == '.') && response[p1] != '\0') {
          filename[p2++]=response[p1++];
       }
      
       filename[p2++]=response[p1++];
       filename[p2++]=response[p1++];
       filename[p2]='\0';
    
       if (! get_random_entry(filename, add)) {    /* problem reading file */
          strcpy (response, "");                  /* error msg printed by */
          return;                                  /* grline()             */
       }
       if (words[0] != '\0') {
            substitute_pattern(add, "%", words);
       }

    } else if(response[p1] == '=') { /* command sidekick */

       dprintf("Detected '@=' construct: %s question was: %s\n", response, question);
       
//       printf("...\n");

       ++p1;
       while (response[p1] != '~' && response[p1] != '\0') {
          filename[p2] = response[p1++];
          if(filename[p2] == '$') {
          	filename[p2] = '\0';
          	strcat(filename,"'");
          	strcat(filename, args);
          	strcat(filename,"'");
          	p2 = strlen(filename) - 1;
          }
          ++p2;
       }
       filename[p2]='\0';
       if(response[p1] == '~') ++p1;

       dprintf("Command=%s\n",filename);

       System(filename);
#ifdef INTERJECT
       strcpy(add,"ok, ehm (where were we?) ... ");
#else
       sprintf(add,"(%s ready at %s)\n", filename, simple_time());
#endif
       
    } else if(response[p1] == '<') { /* command output eater */

       dprintf("Detected @< construct: %s question was: %s\n", response,question);

//       printf("......\n");
       ++p1;
       while (! (response[p1] == '~') && response[p1] != '\0') {
          filename[p2] = response[p1++];
          if(filename[p2] == '$') {
              
              if(strlen(question) > 2) {
          	filename[p2] = '\0';
          	strcat(filename,"'");
          	strcat(filename, args);
          	strcat(filename,"'");
          	p2 = strlen(filename) - 1;
              }
          }
          ++p2;
       }
       filename[p2]='\0';
       if(response[p1] == '~') ++p1;

       sprintf(tempname,"/tmp/splo-%x", rand());
       sprintf(os_line,"%s >%s", filename, tempname);
       System(os_line);
       gfile_truncate(tempname, add);
       sprintf(os_line,"rm -f %s", tempname);
       System(os_line);
       
    } /* endwhile indirector @ in response */
    
    dprintf("<<usetempl, insertions ready>>\n");

    if (response[p1] != '\0') {
      sprintf(text, "%s%s%s", new, add, &response[p1]);
    } else {
      sprintf(text, "%s%s",new,add);
    }
    strcpy(response, text); 
  }
  
  remove_controlchars_and_backquote(response);
  strcpy(mystate->templ[i].last_question, question); 
  strcpy(mystate->templ[i].last_words, inwords); 
}




/***************************************************************************
   normalized_synonyms() takes a string pointer.  
   Using the syn.dict file (format is
   given in the syn.dict file itself) it expands synonyms.
****************************************************************************/ 
void normalized_synonyms (char *s)
{
   char *old;
   char *new;
   char line[255];
   FILE *fp;

   strcat(line, "#");
   strlower(s);
   fp = fopen("syn.dict","r");
   if (fp == NULL)
     {
        fprintf(stderr, "ERROR: Could not open the file syn.dict\n");
        return;
     }
   else
     {
       while(!feof(fp))
         {
           fgets(line,255,fp);
           while ((line[0] == '#') || (isspace(line[0])))
             fgets(line,255,fp);
           
           strlower(line);
           
           new = strtok (line, ":\n");
           old = strtok (NULL, ":\n");
           while (old != NULL)
             {
               substitute_pattern(s,old,new);        
               old = strtok (NULL, ":\n");
             }
         }
       fclose(fp);
     }
}              



/**************************************************************************
 phrasefind()
**************************************************************************/
char *phrasefind(char *string, char *searchstring)
{

        while (!isalnum(*string) && (*string != '\0')) string++;
        while (isalnum(*string))
        {
                if (!strncasecmp(string, searchstring, strlen(searchstring)) &&
                        !isalnum(*(string + strlen(searchstring))))
                        return string;
                while (isalnum(*string)) string++;
                while ((*string != '\0') && !isalnum(*string)) string++;
        }
        return NULL;
}


/***************************************************************************
 trytempl() will try to match the current line to a template.  In turn, try
 all templates to see if if they find a match.
***************************************************************************/

int trytempl(char question[], char words[], State *mystate)
{
  int     i;
  char    t[MAXSTR];
  int     found,done;
  char    *key;
  char    key1[MAXSTR];                 /* first half of a % template */
  char    key2[MAXSTR];                 /* second half of a % template */
  char    winwords[MAXSTR];
  int     firstime;
  int     p,j;
  char    *p1,*p2;
  int     score;                     /* current highest priority */
  int     winner;                    /* current high scorer */

  winner=-1;
  found=0;
  done=0;
  score=0;
  words[0] = '\0';                    /* the % words */
  firstime=1;
  for (i=0; i <= mystate->maxtempl; i++) {     /* loop through all templates */
    done=0;
    strcpy(t, mystate->templ[i].tplate);
    key = mystate->templ[i].tplate;
    firstime=1;
    while (done == 0) {
      if (firstime) {
	key = strtok(t, ":\n");
	firstime=0;
      } else {
	 key = strtok(&key[strlen(key)+1], ":\n");
      }
	 
      if (key == NULL) {
	done=1;
	break;
      }
      switch (key[0]) {
      case '!': if (phrasefind(question, key+1) != NULL) {
	           found=0;
		   done=1;
		 }
	        break;
      case '&': if (phrasefind(question, key+1) == NULL) {
	           found=0;
		   done=1;
		 }
	        break;
      case '+': if (phrasefind(question, mystate->my_nick) == NULL) {  /* name */
                   found=0;
                   done=1;
                 }
                break;
      default:  if (strstr(key, " %") == NULL) {    /* regular template, no % */
	           if (phrasefind(question, key))
		     found=1;
		   break;
		 
	        } else {     		  /* there is a " %" in template */
		  key1[0]=key[0];
		  p=0;
		  j=0;
		  while ((key1[p++]=key[j++]) != '%');    
		  if(p >= 2) {
		  	key1[p-2]='\0'; /* LEFT PART */
		  }
		  
                  if(p >= 2) {
		   if ((key[p] != '\0') && (key[p] != '\n') )  { /* xxx % xxx */
		    strcpy(key2, &key[p+1]); /* RIGHT PART */
		    j=0;
		    p1 = phrasefind(question, key1);
		    if (p1 != NULL) {
		      if (!ispunct(p1[strlen(p1)-1])) {
			p2 = phrasefind(p1+1, key2);
			if (p1 != NULL && p2 != NULL) {  /* both keys found */
			  found=1;
			  strcpy(words, p1+strlen(key1)+1);
			  words[p2-p1-strlen(key1)-2]='\0'; /* -2 for spaces */
			}
		      }
		    }
		    
		   } else {		    /* xxxx % */
		    p1=phrasefind(question, key1);
		    if ((p1 != NULL)&& p1[strlen(key1)] != '\0') {
		      
		      /* if (!ispunct(p1[strlen(p1)-1]))  */
			found=1;
			strcpy(words, p1+strlen(key1)+1);
		    }
		   }
		  } else {     /* % xxxx */
		    strcpy(key2, &key[p+1]); /* RIGHT PART */
		    j=0;
		    p2 = phrasefind(question, key2);
		    if (p2 != NULL) {  /* key found */
			  found=1;
			  strcpy(words, p2+strlen(key2)+1);
		    }
		  }
		} /* endif find % */
		if(strlen_eff(words) < 1) {
			found = 0;
		}
      }
    }
   
    if (found && done)
      {
        if (mystate->templ[i].priority >score) {
  	  winner=i; 
	  score = mystate->templ[i].priority;
	  strcpy(winwords, words);
/*	  printf ("the new template is %s\n",mystate->templ[i].tplate); */
	}
	  
	if (mystate->templ[i].priority == MAXPRIOR_TOPIC) {
	  strcpy(words, winwords);
	  return(winner);
	}
      }
    found=0;
    done=0;
  }
  strcpy(words, winwords);
  return(winner);
}


void pick_templ_winner(char words[], char response[], State *mystate)
{
  int i, j, nout, nout_target, nout_max, isorted;
  double activity[NTEMPL_MAX];
  int idx[NTEMPL_MAX];
  double norm, act_noise;
  char buf[MAXSTR];

  for(i = 0; i < mystate->maxtempl; ++i) {
  	if(mystate->templ[i].priority >= SERIOUS_TOPIC) {
  	   activity[i] = mystate->templ[i].act * mystate->templ[i].priority;
  	} else {
  	   activity[i] = (double) NOTOPIC;
  	}
  }
  
  shell_sortr(activity, mystate->maxtempl,idx,-1);

  nout_max = NREACT_MAX;
  norm = 0.0;
  for(i = 0; i < mystate->maxtempl; ++i) {
    if(activity[idx[i]] > 0.0) {
       norm += activity[i];
    } else {
       break;
    }
  }
  if(i < nout_max) {
    nout_max = i;
  }

#ifndef NORMALIZE
  norm = 1.0;
#endif

  strcpy(buf, response);
  
  nout_target = 3. * drand48();;
  nout = 0;
  
  if(nout_target > 0) {
     for(i = 0; i < nout_max; ++i) {
        isorted = idx[i];
        
        act_noise = REACT_NOISE * drand48();

        if(activity[isorted] / norm > (REACT_THRESHOLD 
                                      + act_noise)) {

  	dprintf("%d %d %d %6.2f %6.2f !ACT! %s (earlier question was: %s)"
  	                        , i
  	                        , isorted
  	                        , mystate->templ[isorted].priority
  	                        , activity[isorted] / norm
  	                        , act_noise
  	                        , mystate->templ[isorted].tplate
  	                        , mystate->templ[isorted].last_question
  	                        );


        	j = strlen_eff(buf);
        	if(j < TOLD_ENOUGH_CHARS) {
        	   if(j > 0 && isalpha(buf[j-1])) {
        		strcat(buf,",");
        	   }
        	   if(j > 0) {
        	   	strcat(buf," ");
                   }
        	   usetempl(isorted,mystate->templ[isorted].last_question
        	                   ,mystate->templ[isorted].last_words
        	                   ,response,mystate);
        	   strcat(buf, response);
        	   ++nout;
        	   if(nout >= nout_target) break;
        	} else {
                   dprintf("Already told %d characters\n", j);
        	}
        } else {
  	        dprintf("%d %d %d %6.2f %6.2f (INH) %s", i
  	                        , isorted
  	                        , mystate->templ[isorted].priority
  	                        , activity[isorted] / norm
  	                        , act_noise
  	                        , mystate->templ[isorted].tplate
  	                        );

        	
        }
     }
  }

  strcpy(response, buf);
  
#ifdef PDEBUG
  printf("Reponse would be [%s] nchr=%d\n", response, strlen_eff(response));
#endif
}

int process_expression(char *question, char *response);

#define NUMERICS "-+*/0123456789()"

int call_process_expression(char *question, char *response)
{
  char xpr[MAXSTR];
  char *eop;
  int ist, iend, i;
  
  eop = strstr(question, ":");
  if(eop == NULL) {
     strcpy(xpr, question);
  } else {
     strcpy(xpr, eop+1);
  }
  i = 0;
  ist = 0;
  while(i < MAXSTR && xpr[i] != (char) 0 && strchr(NUMERICS, xpr[i]) == NULL) {
        ++i;
  }
  ist = i;                        	
  iend = strlen(xpr) -1;
  i = iend;
  while(i >= 0 && (strchr(NUMERICS, xpr[i]) == NULL)) {
        --i;                        	
  }         
  iend = i;                       	
  	
  if(iend >= ist) {
     xpr[iend+1] = (char) 0;
     return(process_expression (&xpr[ist],response));
  } else {
     return(0);
  }
}

int high_bits(char str[]) /* Return true if non-ASCII (>127) chars present */
{
	int i;
	
	i = 0;
	while(str[i] != (char) 0) {
		if((unsigned int) str[i] > 128) {
			return(1);
		}
		++i;
	}
	return(0);
}


/***************************************************************************
  ask(person, question) takes two nul-terminated strings, one is the 
  question and the other is the person who asked the question.  The function 
  returns the int value which represents the template used (0 or greater).
  if no match was found, a -1 is returned.  ask always sets the global 
  variable "response" to contain an appropriate response, or if no match was
  found, an reply from the last entry in the dictionary. (a default reply)
 ***************************************************************************/

int ask(
     char    person[],            /* person who asked the question */
     char    question[MAXSTR],      /* input line from user */
     char    words[],
     char    response[],
     State   *mystate
     )         

{
  int     i,j, itopic, used_topic;
  
  if(high_bits(question)) {
  	ask("Joe", "meta_foreign_language", words, response, mystate);
  	return(NOTOPIC);
  }         	                                	                 

  if(call_process_expression (question,response) == 1) {
  	return(NOTOPIC);
  }
  
                                     /* normalized_synonyms also strlowers() */
  normalized_synonyms(question);                  /* swap word according to syn.dict file */

  dprintf("<<normalized synonyms>>\n");

  i = trytempl(question,words,mystate);    /* look for a matching template */

  dprintf("<<trytempl done, i=%i>>\n",i);


  if (i >=0)   {                     /* found a match, index=i */
  
    if (mystate->templ[i].priority > SERIOUS_TOPIC)  {    
            /* e.g. key word priorities of value 5 and up */
                                                                   
      for (j=0; j < HISTORY-1; j++) {  /* update history queue */
         mystate->topics[j] = mystate->topics[j+1];
      }
      mystate->topics[HISTORY-1] = i; /* just fired this one */
    }
    
    usetempl(i, question,words,response,mystate);      /* build response */
    used_topic = i;
    
  } else {                              /* no match found */

    if (drand48() > 0.33) {                   /* Sometimes, */
       usetempl(mystate->maxtempl, question, words, response, mystate); 
                                          /* use a neutral response */
       used_topic = NOTOPIC;
       
    } else {                            /* sometimes, use an old key */ 
      i = (random() % HISTORY);
      itopic = mystate->topics[i];

      if (itopic == NOTOPIC) {          /* no history yet, take a neutral one */

	dprintf("<<No history yet, itopic=%d>>\n",itopic);
	  
	usetempl(mystate->maxtempl, question,words,response,mystate);
	used_topic = NOTOPIC;
	
      } else {
	dprintf("<<History used, itopic=%d>>\n",itopic);
        usetempl(itopic,mystate->templ[itopic].last_question
                       ,mystate->templ[itopic].last_words
                       ,response,mystate);

      	used_topic = itopic;
      }
    }
  }
  
 
  /* Additional ramblings */
  
  pick_templ_winner(words,response,mystate);

  return(used_topic);
}



/**************************************************************************
  buildtempl() reads the MAINDICT file and fills in the template table.
  Each entry in the template table refers to a single template and all of 
  its replies.
 ***************************************************************************/

void buildtempl(State *mystate)
{
  char    line[MAXSTR];
  char    temp[MAXSTR];
  int     i;
 
  
  i=0;   /* first template starts at zero */
  
  /* loop, one pass per template (including all replies) */
  while (!feof(dfile)) {
    fgets(line, sizeof(line), dfile);
    
    while (((line[0] == '#') || (isspace(line[0]))) && (!feof(dfile)))
      fgets(line, sizeof(line), dfile);
    
    if (feof(dfile))
      break;
    
    /* read in template */

    strcpy(mystate->templ[i].tplate, line);
    strcpy(mystate->templ[i].last_question, "yes?");
    strcpy(mystate->templ[i].last_words, "");

    /* read priority */
    fgets(line, sizeof(line), dfile);

    if (atoi(line) == 0)  /* no entry */
      mystate->templ[i].priority = MAXPRIOR_TOPIC;     /* default priority */
    else
      mystate->templ[i].priority = atoi(line);
      
    mystate->templ[i].ifreq = 0;
    mystate->templ[i].act   = 0.0;
    
    /* set number of alternate replies to 0 */
    mystate->templ[i].talts = 0;
    
    /* count number or responses, start with asterisk */
    if (line[0] != '*')
      fgets(line, sizeof(line), dfile);

    /* where to find first response within dictionary database file */
    
    mystate->templ[i].toffset = ftell(dfile) - strlen(line);

    while ((line[0] == '*') && !feof(dfile)) {
      mystate->templ[i].talts++;
      fgets(line, sizeof(line), dfile);
    }
    
    /* pick a random starting point for the responses */
    
    mystate->templ[i].tnext = 1 + (random() % (mystate->templ[i].talts));
    if (mystate->templ[i].tnext > mystate->templ[i].talts)
      mystate->templ[i].tnext = 1;

    strcpy(temp, mystate->templ[i].tplate);
    temp[strlen(temp)-1]='\0';

    if (VERBOSE)
      fprintf(stderr, "<<Template[%i]=:%s:>>\n",i,temp);
    

    i++;   /* next template */

    if (i >= NTEMPL_MAX) {
      fprintf(stderr, "ERROR: template array too small\n");
      exit(1);
    }
    
  }
  
  /* all templates have been read and stored */
  mystate->maxtempl = i - 1;
}




/*********************************
strcasestr()
**********************************/

char *strcasestr (char *s1, char *s2)
{

        char n1[MAXSTR], n2[MAXSTR];
        int j;

        for (j=0;s1[j] != '\0';j++)
                n1[j] = toupper (s1[j]);
        n1[j] = '\0';
        for (j=0;s2[j] != '\0';j++)
                n2[j] = toupper (s2[j]);
        n2[j] = '\0';
       
        return (strstr (n1, n2));
}



/*
int strncasecmp (a, b, n)
char *a;
char *b;
int   n;
{

        for (; (*a != '\0') && (*b != '\0') &&
                (tolow (*a) == tolow (*b)) && (n  > 0); a++, b++, n--);
        if (n == 0)
                return 0;
        return (tolow (*a) - tolow (*b));
}

*/



/* ============================ */

double aff_accuracy(Text_affect *aff)
{
      return (double) (100*(aff->nexclama 
                   + aff->nquestion 
                   + aff->nperiod_last 
                   + aff->nupper_first 
                   + aff->ncommas))
                   / (double) aff->nwords;
}

double aff_questioning(Text_affect *aff)
{
      return (double) (100*(aff->nquestion))
                  / (double) aff->nanswers;
}

double aff_stating(Text_affect *aff)
{

      return (double) (100*(aff->nexclama))
                  / (double) aff->nanswers;
}

double aff_anger(Text_affect *aff)
{
   double anger;
                   
   if(aff->nupper > 4) {
	anger = (double) (100 * aff->nupper) / (double) aff->nletters;   	
   } else {
        anger = (double) (100 * aff->nbad_words)
                   / (double) aff->nwords;
   }
   return(anger);
}


double aff_emotionality(Text_affect *aff)
{
   double anger;
   double emo;
   
   emo = (double) (100*(aff->nexclama 
                   + aff->nquestion
                   + aff->ngood_words
                   + aff->nbad_words
                   ))
                   / (double) aff->nwords;
                   
   anger = aff_anger(aff);
   
   if(anger > emo) {
   	emo = anger;
   }
   return(emo);
}
                   
double aff_valence (Text_affect *aff)
{

      return (double) (100*(aff->ngood_words-aff->nbad_words))
                  / (double) aff->nwords;
}

double aff_productivity (Text_affect *aff)
{

      return (double) (100*(aff->nwords - aff->nanswers)) 
                         / (double) aff->nanswers;
}

double aff_pomposity (Text_affect *aff)
{
	return ((double) aff->nletters / (double) aff->nwords - 5.)/5. 
                        * 100.
                        * (double) aff->nwords / (double) aff->nanswers; 
}                        

/* ============================ */

int is_good_smiley(char str[])
{
	static char *smileys[] = { ":-)"
	                          ,";-)"
	                          ,"8-)"
	                          ,":)"
	                          ,""};
	register int i, ok;
	
	ok = 0;
	i = 0;
	while(smileys[i][0] != (char) 0) {
		if(strcmp(smileys[i], str) == 0) {
			ok = 1;
			break;
		}
		++i;
	}
	return(ok);
}

int is_bad_smiley(char str[])
{
	static char *smileys[] = { ":-("
	                          ,";-("
	                          ,"8-("
	                          ,":("
	                          ,""};
	register int i, ok;
	
	ok = 0;
	i = 0;
	while(smileys[i][0] != (char) 0) {
		if(strcmp(smileys[i], str) == 0) {
			ok = 1;
			break;
		}
		++i;
	}
	return(ok);
}

int is_good_word(char str[])
{
	static char *words[] = { "yes"
	                        ,"yo"
	                        ,"good"
	                        ,"nice"
	                        ,"sweet"
	                        ,"honey"
	                        ,"excellent"
	                        ,"well"
	                        ,""};
	register int i, ok;
	
	ok = 0;
	i = 0;
	while(words[i][0] != (char) 0) {
		if(strncasecmp(words[i], str,strlen(str)) == 0) {
			ok = 1;
			break;
		}
		++i;
	}
	return(ok);
}

int is_bad_word(char str[])
{
	static char *words[] = {"no"
	                       ,"bad"
	                       ,"shit"
	                       ,"fuck"
	                       ,"fucking"
	                       ,"stupid"
	                       ,"ass"
	                       ,"asshole"
	                       ,"cunt"
	                       ,"gay"
	                       ,"pervert"
	                       ,""};
	register int i, ok;
	
	ok = 0;
	i = 0;
	while(words[i][0] != (char) 0) {
		if(strncasecmp(words[i], str, strlen(str)) == 0) {
			ok = 1;
			break;
		}
		++i;
	}
	return(ok);
}

void reset_text_affect(Text_affect *aff)
{
	aff->nexclama = 0;
	aff->nquestion = 0;
	aff->nperiods = 0;
	aff->nperiod_last = 0;
	aff->ncommas = 0;
	aff->nwords = 0;
	aff->nbad_words = 0;
	aff->ngood_words = 0;
	aff->nletters = 0;
	aff->nupper = 0;
	aff->nupper_first = 0;
	aff->ndigits = 0;
	aff->ntacit = 0;
	aff->nanswers = 0;
	aff->nanswers_since = 0;

	aff->nadd = 0;

	aff->dt = 0.0;
}

void add_text_affect(Text_affect *aff, Text_affect *add)
{
	aff->nexclama     += add->nexclama;
	aff->nquestion    += add->nquestion;
	aff->nperiods     += add->nperiods;
	aff->nperiod_last += add->nperiod_last;
	aff->ncommas      += add->ncommas;
	aff->nwords       += add->nwords;
	aff->nletters     += add->nletters;
	aff->nupper       += add->nupper;
	aff->nupper_first += add->nupper_first;
	aff->ndigits      += add->ndigits;
	aff->ntacit       += add->ntacit;
	aff->nanswers     += add->nanswers;

	aff->dt           += add->dt;
	++aff->nadd;
}

void analyze_text_affect(char str[], Text_affect *aff, int dt_sec)
{
  char buf[MAXSTR];
  register int ist, iend, i;
  
  reset_text_affect(aff);

  ist = 0;
  while(str[ist] <= (char) 32 && str[ist] != (char) 0) {
  	++ist;
  	if(str[ist] == (char) 0) break;
  }
  
  iend = strlen(str) - 1;
  while(iend > ist &&
          str[ist] <= (char) 32) {
          	--iend;
  }	
  strncpy(buf, &str[ist], iend - ist + 1);
  buf[iend-ist+1] = (char) 0;
  
  i = 0;
  while(buf[i] != (char) 0) {
  	if(buf[i] == '!') {
  		++aff->nexclama;
	} else if(buf[i] == '\?') {
  		++aff->nquestion;
	} else if(buf[i] == '.') {
  		++aff->nperiods;
  		if(buf[i+1] == (char) 0) {
  			++aff->nperiod_last;
  		}
	} else if(buf[i] == ',') {
  		++aff->ncommas;
	} else if(isupper(buf[i]) ||
	          islower(buf[i])) {
  		++aff->nletters;
  		if(isupper(buf[i])) {
  		     ++aff->nupper;
  		     if(i == 0) {
  		     	++aff->nupper_first;
  		     }
  		}
	} else if(isdigit(buf[i])) {
  		++aff->ndigits;
  	}
  	++i;
   }
   
   if(is_good_smiley(buf)) {
   	++aff->ngood_words;
   }  	
   
   if(is_bad_smiley(buf)) {
   	++aff->nbad_words;
   }  	
   if(is_good_word(buf)) {
   	++aff->ngood_words;
   }  	
   
   if(is_bad_word(buf)) {
   	++aff->nbad_words;
   }  	
   
   aff->dt += dt_sec;
}

void print_text_affect(FILE *fp, char title[],Text_affect *aff)
{


#ifdef VERTICAL
fprintf(fp,"%s ==================================\n", title);
fprintf(fp,"nexclama =     %d\n", aff->nexclama);
fprintf(fp,"nquestion =    %d\n", aff->nquestion);
fprintf(fp,"nperiods =     %d\n", aff->nperiods);
fprintf(fp,"nperiod_last = %d\n", aff->nperiod_last);
fprintf(fp,"ncommas =      %d\n", aff->ncommas);
fprintf(fp,"nwords =       %d\n", aff->nwords);
fprintf(fp,"nletters =     %d\n", aff->nletters);
fprintf(fp,"nupper =       %d\n", aff->nupper);
fprintf(fp,"nupper_first = %d\n", aff->nupper_first);
fprintf(fp,"ndigits =      %d\n", aff->ndigits);
fprintf(fp,"ntacit =       %d\n", aff->ntacit);
fprintf(fp,"nanswers =     %d\n", aff->nanswers);
fprintf(fp,"\n");
fprintf(fp,"accuracy =     %5.1f\n", aff_accuracy(aff));
fprintf(fp,"questioning =  %5.1f\n", aff_questioning(aff));
fprintf(fp,"stating =      %5.1f\n", aff_stating(aff));
fprintf(fp,"emotionality = %5.1f\n", aff_emotionality(aff));
fprintf(fp,"valence =      %5.1f\n", aff_valence(aff));
fprintf(fp,"productivity = %5.1f\n", aff_productivity(aff));
fprintf(fp,"pomposity =    %5.1f\n", aff_pomposity(aff));

#else
fprintf(fp,"%s Na=%d Nw=%d Acc=%5.1f Qu=%5.1f St=%5.1f Em=%5.1f Val=%5.1f Pr=%5.1f  Pomp=%5.1f dt/n=%5.1f\n"
          , title
          , aff->nanswers
          , aff->nwords
          , aff_accuracy(aff)
          , aff_questioning(aff)
          , aff_stating(aff)
          , aff_emotionality(aff)
          , aff_valence(aff)
          , aff_productivity(aff)
          , aff_pomposity(aff)
          , aff->dt/aff->nadd);
#endif
}


void do_analyze_text_affect(char str[], Text_affect *cumul
                                      , Text_affect *now, int dt_sec)
{
   char *p, *w;
   char tmp[MAXSTR];
   int i;

   if(strlen(str) > 0) {
        if(strcmp(str,cumul->previous_str) == 0) {
           cumul->nidentical_row += 1;
        } else {
           cumul->nidentical_row = 0;
        }
        strcpy(tmp, str);
	p = tmp;
	i = 0;
	while((w = strtok(p," ")) != NULL) {
	  analyze_text_affect(w, now, dt_sec);
	  add_text_affect(cumul, now);
	  ++cumul->nwords;
	  p = NULL;
	  ++i;
	}
	cumul->ntacit = 0;
   } else {
   	++cumul->ntacit;
   }
   ++cumul->nanswers;
   
   strcpy(cumul->previous_str,str);
}

void append_newline(char saved[])
{
#define HTML
#ifndef HTML
   strcat(saved,"\n");
#else 
   strcat(saved,"<br>\n");
#endif
}
void add_meta_remarks(char str[], State *mystate
                                , char words[], char response[]) {
   char saved[MAXSTR];
   char meta[MAXSTR];

#define PROB_COMPLAIN_SOMETHING_ELSE 0.75

   if(strlen_eff(response) < 1) { 
#ifdef PDEBUG
         printf("(reponse of length zero)\n");
#endif
         if(drand48() > (1.-PROB_COMPLAIN_SOMETHING_ELSE)) {
       	    strcpy(meta,"splotch: meta_something_else");
            ask("Joe", meta, words, response, mystate);
         }
   }

   if(mystate->cumul->nanswers_since > 3) {
      strcpy(saved,response);
         
      if(aff_questioning(mystate->cumul) > 60.) {
      	strcpy(meta,"splotch: meta_all_these_questions");
        ask("Joe", meta,words,response, mystate);
        append_newline(saved);
        strcat(saved, response);
        
      }
      if(aff_stating(mystate->cumul) > 60.) {
        strcpy(meta, "splotch: meta_overconfident");
        ask("Joe", meta,words,response, mystate);
        append_newline(saved);
        strcat(saved, response);
      }
      if(mystate->cumul->ntacit > 3) {
        strcpy(meta, "splotch: meta_tacit");
        ask("Joe", meta, words, response, mystate);
        append_newline(saved);
        strcat(saved, response);
        mystate->cumul->ntacit = 0;
      }
      if(aff_emotionality(mystate->cumul) > 50.) {
        strcpy(meta, "splotch: meta_emotional");
        ask("Joe", meta, words, response, mystate);
        append_newline(saved);
        strcat(saved, response);
      }
      if(mystate->cumul->nidentical_row > 4 + drand48() * 2.) {
        strcpy(meta, "splotch: meta_robot_suspicion");
        ask("Joe", meta, words, response, mystate);
        append_newline(saved);
        strcat(saved, response);
      }
      if(mystate->cumul->nidentical_row > 10 + drand48() * 5.) {
        strcpy(meta, "splotch: meta_robot_certainty");
        ask("Joe", meta, words, response, mystate);
        append_newline(saved);
        strcat(saved, response);
      }
      mystate->cumul->nanswers_since = 0;
      
      strcpy(response,saved);
   }
   ++(mystate->cumul->nanswers_since);
}


/***************************************************************************
  init() sets up the template, zeros variables. Must be called by main prg
 ***************************************************************************/


void reset_splotch(State *mystate)
{
  int    i;
  
  /* initialization section, zero array */
  
  for (i=0; i < HISTORY; i++) {
      mystate->topics[i] = NOTOPIC;                /* no previous keywords */
  }

  for(i = 0; i < NAGENTS; ++i) {  
     reset_text_affect(&(mystate->cumul[i]));
     reset_text_affect(&(mystate->now[i]));
  }
  
  /* oops: templ[i].act are not reset (yet) */
  
  time(&mystate->t_bot_response);
  time(&mystate->t_hum_question);

  mystate->ncycles = 0;
  mystate->ntaboo_words = 0;
}

void init_splotch(State *mystate, char *myname)
{
  strcpy(mystate->my_nick, myname);

  reset_splotch(mystate);
  
  dprintf("<<using dict file %s>>>\n", DICTFILE);
  if(dfile != NULL) {
  	fclose(dfile);
  } 
 
  dfile = fopen (DICTFILE, "r");
  if (dfile == NULL) {
    	    fprintf(stderr, "ERROR: unable to read %s\n",DICTFILE);
    	    exit(1);
  }
    	        
  srandom(getpid());   /* randomize seed */

  srand48((long int) getpid());

  buildtempl(mystate);    /* read the templates, make 1 entry per template */

  dprintf("<<templates built, seed chosen>>\n");
}

void save_splotch(State *mystate, char *human_id)
{
	char os_line[255], tmp[MAXSTR];
	int i, j, intopics;
	FILE *fp;
	
	sprintf(os_line,"mkdir -p %s/%s", LOGDIR, human_id);
	system(os_line);
	sprintf(os_line,"%s/%s/main.act", LOGDIR, human_id);
	fp = fopen(os_line,"w");
	for(i = 0; i <= mystate->maxtempl; ++i) {
		intopics = 0;
		for(j = 0; j < HISTORY; ++j) {
			if(mystate->topics[j] == i) {
				intopics = 1;
				break;
			} 
		}

                strcpy(tmp, mystate->templ[i].tplate);
                str_tr_controls(tmp, ' ');

		if(mystate->templ[i].act > 0.00001) {
		   fprintf(fp,"%5.2f %d %d %d %d key=%s que=%s pat=%s\n"
		          , mystate->templ[i].act
		          , i
		          , mystate->templ[i].ifreq
		          , intopics
		          , mystate->templ[i].priority
		          , tmp
		          , mystate->templ[i].last_question
		          , mystate->templ[i].last_words
		          );
		}
	}
	fclose(fp);
}


void add_to_file(char *fname, char *human_id, char *str)
{
	FILE *fp;
	
	fp = fopen(fname,"a");
	if(fp == NULL) {
		fp = fopen(fname,"w");
		if(fp == NULL) {
			fprintf(stderr,"Error writing to file [%s]\n", fname);
			return;
		}
	}
	fprintf(fp,"%s %s %s\n", full_time()
	                       , human_id
	                       , str);
	fclose(fp);
}



#ifdef STANDALONE 

int main(int argc, char *argv[]) 
{

  char   words[MAXSTR];   /* a template has been matched, this is % */
  char   response[MAXSTR];     /* response to be returned */

  static State mystate;

  static char *partner[NAGENTS] = { "HUM:","COM:" };
  char   question[MAXSTR];
  char   qcopy[MAXSTR];
  char   atime[MAXSTR];
  char   qtime[MAXSTR];
  char   rtime[MAXSTR];
  char   tmp[MAXSTR];
  char   os_line[MAXSTR];
  FILE   *fp_biglog;
  char   logfilnam[MAXSTR];

  time_t tprompt;
  time_t tquestion;
  time_t tanswer;
  int    dt_human;
  int    dt_computer;
  int    i;

  sprintf(logfilnam,"%s/%011d", LOGDIR, (int) time(NULL));
  strcat(logfilnam,".log");
  str_tr_controls(logfilnam,'=');
  
  fp_biglog = fopen(logfilnam,"w");
  if(fp_biglog != NULL) fprintf(fp_biglog,"Session starts at %s\n\n", simple_date());

  init_splotch(&mystate, NAME);
  fprintf(stdout, "Hello! Let's chat. My name is %s. ('exit' to quit)\n\n",NAME);
  fflush(stdout);
  strcpy(question, "foo");

  while (strncmp(question, "exit", 4) != 0) {
    strcpy(atime, simple_time());
    time(&tprompt);
    if(fp_biglog != NULL) {
	       fprintf(fp_biglog,"%s >? \n", atime);
    }
    printf("> ");
    if(fgets(question,MAXSTR,stdin) == NULL) {
       fprintf(stderr,"[EOF]\n");
       break;
    }
    strcpy(qtime, simple_time());
    time(&tquestion);
    dt_human = tquestion - tprompt;
    
    if (strncmp(question,"exit", 4) != 0)
	{
		
	  do_analyze_text_affect(question, &mystate.cumul[HUMAN]
	                                 , &mystate.now[HUMAN]
	                                 , dt_human);
	                                      
	  strcpy(qcopy, question);
	  store_question(question);
	  sprintf(tmp, "%s: %s",NAME, question);
	  strcpy(question, tmp);
	
	  printf("\n");
	  if (question[0] != '\0') {
	    ask("Joe", question, words, response, &mystate);

	    add_meta_remarks(response, &mystate,words,response);
	    
	    time(&tanswer);
	    dt_computer = tanswer - tquestion;
            strcpy(rtime, simple_time());
            
	    do_analyze_text_affect(response, &mystate.cumul[COMPUTER]
	                                   , &mystate.now[COMPUTER]
	                                   , dt_computer);
	                                                 
	    fprintf(stdout, "%s\n", response);  
	    fflush(stdout);
	    if(fp_biglog != NULL) {
	       fprintf(fp_biglog,"%s %s %s\n\n", qtime, partner[HUMAN], qcopy);
	       fprintf(fp_biglog,"%s %s %s\n\n", rtime, partner[COMPUTER], response);
	       fflush(fp_biglog);
	    }
#ifdef SOUND
	    sprintf(os_line,"say -f 8 -x 800 \"%s\" 2>/dev/null 1>/dev/null", response);
	    system(os_line);
#endif
            ++mystate.ncycles;
	  }
	}
    else fflush(stdout);
  }
  if(fp_biglog != NULL) {
        for(i = 0; i < NAGENTS; ++i) {
                print_text_affect(fp_biglog, partner[i], &mystate.cumul[i]);
        }
  	
  	fprintf(fp_biglog,"\nSession ends at %s\n%d Q+A cycles were exchanged\n\n"
  	                 , simple_date(),mystate.ncycles);
  }
  fclose(fp_biglog);
  if(mystate.ncycles < 1) {
  	sprintf(os_line,"rm -f %s", logfilnam);
  	system(os_line);
  } else {
  	sprintf(os_line,"chmod a+r %s", logfilnam);
  	system(os_line);
  }
  return(0);
}


#else /* WEBVERSION */

void chatty(char *human_id, char *question, char *response)
{
   static int init = 0;
	
  static char words[MAXSTR];   /* a template has been matched, this is % */

  static State mystate;

  static char *partner[NAGENTS] = { "HUM:","COM:" };

  char   qcopy[MAXSTR];
  char   atime[MAXSTR];
  char   qtime[MAXSTR];
  char   rtime[MAXSTR];
  char   tmp[MAXSTR];
  static FILE   *fp_biglog;
  char   logfilnam[MAXSTR];

  time_t tquestion;
  time_t tanswer;
  int    dt_human;
  int    dt_computer;
  int    i;

  str_tr_controls(question, ' ');
  
  if(init == 0) {
  	 init = 1;
         sprintf(logfilnam,"%s/%011d", LOGDIR, (int) time(NULL));
         strcat(logfilnam,".log");
         str_tr_controls(logfilnam,'=');
  
         fp_biglog = fopen(logfilnam,"w");
         if(fp_biglog != NULL) fprintf(fp_biglog,"Session starts at %s\n\n", simple_date());

         init_splotch(&mystate, NAME);

         strcpy(mystate.current_human, human_id);
    }
    
    if(strcmp(mystate.current_human, human_id) != 0) {
       if(fp_biglog != NULL) {
           for(i = 0; i < NAGENTS; ++i) {
                print_text_affect(fp_biglog, partner[i], &mystate.cumul[i]);
           }
  	
  	   fprintf(fp_biglog
  	     ,"\nSession ends at %s\n%d Q+A cycles were exchanged\n\n"
  	     , simple_date(),mystate.ncycles);
       }
       save_splotch(&mystate, human_id);
       reset_splotch(&mystate);
       strcpy(mystate.current_human, human_id);
    }
    
    if(has_taboo_word(question)) {
    	++mystate.ntaboo_words;
    	if(mystate.ntaboo_words >= 4) {
    		ignorable_rand_user(atoi(mystate.current_human), UIN_ADD_IGN);
    	}
    	add_to_file("taboo.log",mystate.current_human, question);
    	strcpy(response,"");
    	return;
    }

    strcpy(atime, simple_time());

    if(fp_biglog != NULL) {
	       fprintf(fp_biglog,"%s >? \n", atime);
    }

    strcpy(qtime, simple_time());
    time(&tquestion);
    dt_human = mystate.t_hum_question - tquestion;
    mystate.t_hum_question = tquestion;
    
    do_analyze_text_affect(question, &mystate.cumul[HUMAN]
	                                 , &mystate.now[HUMAN]
	                                 , dt_human);
	                                      
    strcpy(qcopy, question);
    store_question(question);
    sprintf(tmp, "%s: %s",NAME, question);
    strcpy(question, tmp);
	
    if (question[0] != '\0') {
	    ask("Joe", question, words, response, &mystate);

	    add_meta_remarks(response, &mystate,words,response);
	    
	    time(&tanswer);
	    dt_computer = tanswer - tquestion;
            strcpy(rtime, simple_time());
            mystate.t_bot_response = tanswer;
            
	    do_analyze_text_affect(response, &mystate.cumul[COMPUTER]
	                                   , &mystate.now[COMPUTER]
	                                   , dt_computer);
	                                                 
	    if(fp_biglog != NULL) {
	       fprintf(fp_biglog,"%s %s (%s) %s\n\n"
	                        , qtime, partner[HUMAN], human_id, qcopy);
	       fprintf(fp_biglog,"%s %s %s\n\n", rtime, partner[COMPUTER], response);
	       fflush(fp_biglog);
    	    }
            ++mystate.ncycles;

   }
   if(strlen_eff(response) < 1) {
  	ask("Joe", "meta_just_a_minute", words, response, &mystate);
   }
}
#endif

