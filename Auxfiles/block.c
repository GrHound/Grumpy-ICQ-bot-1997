/*by mwelz - subject to the GPL*/

/* #define UCOL         delete this line if you do not want color   */
/* #define NEW          delete this line if you have an old ncurses */

#ifndef CURSES
#include "ncurses.h"
#else
#include <curses.h>
#endif

#include <string.h>
#include <stdlib.h>

/*heck - i started writing the thing using curses - then 
         switched to ncurses, did not seem to make much
         difference
   
         oh - ncurses includes stdio.h
 */

#define NAMEL  40
#define FNAMEL 20
#define CHARL 126

#define VERSION 5

#define FF_ME      1
#define FF_SOLID   2
#define FF_BLOCK   3
#define FF_HOME    4 
#define FF_OVEN    5

#define DEFGAME "/usr/games/lib/default.block"    

/* uncomment if you like lots of diagnostic garbage */
/* #define DEBUG 1                                  */

struct game{
  char name[NAMEL];
  char fname[FNAMEL];
  char xdim;
  char ydim;
  char field[CHARL][CHARL];
  char diff;
};

/* the start of utterly unreadable, incomprehensible,
   nonmodular, inefficient code */

int pyr=1;
int flashi=0;

int main(int argc,char **argv){
  int in;
  int myx,myy,i,j;
  char lname[60],*cptr;
  char tmp[60];
  int xo,yo,mvv;
  char mover=0,alternate=0,jumped=1;
  long score=0,moves=0;
  char done=0;
  int  speed=0;
  int  looper=0;

  /* some important flags */
  char stricti=0;      /* if all sweeties have to be collected */
  char helpi=0;
  char movei=0;
 

  struct game cw;

  i=argc-1;
  /*copy a resonable default to this one*/
  strcpy(lname,DEFGAME); /*say /usr/games/lib/default.block */
  
  while(i){
    cptr=argv[i--];
    j=0;
    if(cptr[0]!='-'){
      strcpy(lname,cptr);
    }
    else{
      while(cptr[j]!='\0'){
      switch(cptr[j]){
          case '-' :
                     break;
          case '?' :
          case 'H' :
          case 'h' : helpi=1;
                     break;
          case 'a' : alternate=cptr[++j];
                     if(alternate=='\0')j--;
                     break;
          case 'f' : flashi=1;
                     break;
          case 'l' : looper=1;
                     break;
          case 's' : stricti=1;   
                     break;
          case 'm' : movei=1;
                     break;
          case 'g' : strcpy(lname,&cptr[j+1]);
                     j=strlen(cptr)-1; 
                     #ifdef DEBUG
                       printf("We are going to read file %s\n",lname);
                     #endif
                     break;
          default  : printf("Unknown argument -%c\n",cptr[j]);
                     break;
          
        }
        #ifdef DEBUG
          printf("\nLooped with %c",cptr[j]);
        #endif
        j++;
      } 
    }
  }
 
  if(helpi){
    printf("\n\t block -ahgms (ver0.%d)\t a small game for termials (GPL)\n\n-h\t\t this help screen\n-a<character>\t use different character\n-g<filename>\t load game from file <filename> eg -gtest or just test\n-m\t\t game has to be completed within a given number of moves\n-s\t\t all sweeties have to be collected\n\n",VERSION);
    exit(1);
  }
 
  if(stricti){
    printf("You will need to collect all the sweeties \nbefore you may advance to the next level\n");
  }

  do{

  done=0;

  switch(loadnext(&cw,lname)){
    case 0 : 
#ifdef DEBUG
             printf("Loaded game %s\n",cw.name);
#endif
             break;
    case 1 : 
    case 2 : printf("Hmmm... trouble reading file %s\n",lname);
             sprintf(tmp,"/usr/games/lib/%s.block",lname);
             if(loadnext(&cw,tmp)){
               printf("Could not read %s either\n",tmp); 
               if(loadnext(&cw,"default")){
                 printf("Could not even read file default - giving up\n"); 
                 exit(1);
               }
             }
             else{
               printf("Ahhh... managed to load game from file %s\n",tmp);
             }
             break;
  } 

  if(movei){
    printf("You will need to complete the game with in %d00 moves\n",cw.diff);
  }


#ifdef DEBUG  
  printf("\nWorking with game %s found in file %s (%s) of dim (%d, %d) and level %d\n",cw.name,cw.fname,lname,cw.xdim,cw.ydim,cw.diff);
  sleep(2);
#endif

 
  if(looper!=2)leaveok(initscr(),TRUE);   /*start the curses*/
  else         doupdate();

  cbreak();
  noecho();    /*for one char at a time without echo*/

#ifdef UCOL
  start_color();
  if(has_colors()==TRUE){
/* change the colors to your liking */
    init_pair(FF_ME      ,COLOR_RED    ,COLOR_BLACK);
    init_pair(FF_SOLID   ,COLOR_BLUE   ,COLOR_WHITE);
    init_pair(FF_BLOCK   ,COLOR_MAGENTA,COLOR_BLACK);  
    init_pair(FF_HOME    ,COLOR_YELLOW ,COLOR_BLACK);
    init_pair(FF_OVEN    ,COLOR_RED    ,COLOR_BLACK);
  }
#endif

  keypad(stdscr,TRUE);
 
  speed=60000/baudrate();

#ifdef NEW
  typeahead(-1);
#endif

  clear();

  xo=(COLS-(cw.xdim))/2;
  yo=(LINES-(cw.ydim))/2;  /*the offsets onto the screen */

  myx=COLS/2-yo;
  myy=LINES*2/3-xo;  /*default starting position*/

  for(i=0;i<cw.ydim;i++){
    for(j=0;j<cw.xdim;j++){
      if(cw.field[i][j]=='{'){
        myx=j;
        myy=i;
        cw.field[i][j]=' ';
        i=cw.ydim;
      }
    }
  } 
  
  /*perform the checks here*/
  /*ie screen too small etc...*/

  if((COLS<cw.xdim+2)||(LINES<cw.ydim+2)){
    endwin();
    printf("\nSorry, but this game needs a screen of at least %d by %d characters\nand this one is only %d by %d characters\n",cw.xdim+2,cw.ydim+2,COLS,LINES);
    exit(1);
  }
  
  if(movei)mvprintw(0,1,"Game has to be completed in %d00 moves",cw.diff);
  
  drawsolid(&cw);

  while(('q'!=(in=getch()))&&(!done)){
    switch(cw.field[(myy+1)%cw.ydim][myx]){
      case '.' : 
      case ' ' :
      case  0  : mvaddch(myy+yo,myx+xo,' ');
                 myy=(myy+1)%cw.ydim;
                 mvaddch(myy+yo,myx+xo,'+');
                 refresh();
                 break;
    }
    if(in!=ERR){
      moves++;
      mover=++mover%2;
      mvaddch(myy+yo,myx+xo,' ');
      switch(in){
        case 'z' :
        case 'Z' :
        case KEY_SLEFT :
        case KEY_C1 :
        case '4' :
        case 'H' : if(jumped){
                     switch(cw.field[(myy+1)%cw.ydim][myx]){
                       case '|' :
                       case '!' :
                       case '#' :
                       case '@' : 
                       case '>' : 
                       case '<' :
                       case 'v' :
                       case '^' : switch(cw.field[(myy+cw.ydim-1)%cw.ydim][myx]){
                                    case  0  :
                                    case '.' :
                                    case ' ' : myy=(myy+cw.ydim-1)%cw.ydim;
                                               break;
                                  }
                                  break;  
                     }
                     jumped=0;
                     mover=1;
                   }
                   else{
                     jumped=1;
                     break;
                   }
        case KEY_LEFT :
        case '1' :
        case 'h' : switch(cw.field[myy][(myx+cw.xdim-1)%cw.xdim]){
                     case '<' : if(myx--==0)myx=cw.xdim-1; 
                     case '.' :
                     case  0  :
                     case ' ' :
                     case '}' :
                                if(myx--==0)myx=cw.xdim-1;
                                break;
                     case '@' : if(in!='H'){
                                  mvv=(myx+cw.xdim-1)%cw.xdim;
                                  while(cw.field[myy][mvv]=='@')mvv=(mvv+cw.xdim-1)%cw.xdim;
                                  switch(cw.field[myy][mvv]){
                                    case  0  :
                                    case ' ' : cw.field[myy][mvv]='@';
                                               cw.field[myy][(myx+cw.xdim-1)%cw.xdim]=' '; 
                                               mvaddch(myy+yo,(myx+cw.xdim-1)%cw.xdim+xo,' ');
                                               break; /*opti here - just exchange 1st with last*/
                                  }
                                  break;
                                }
                   }
                   break;
        case KEY_DOWN :
        case '2' :
        case 'J' :
        case 'j' : switch(cw.field[(myy+1)%cw.ydim][myx]){
                     case 'v' : if(++myy==cw.ydim)myy=0; 
                     case '}' :
                     case '.' :
                     case  0  :
                     case ' ' : if(++myy==cw.ydim)myy=0;
                                break;
                   }
                   jumped=1;
                   break;
        case KEY_UP :
        case '5' :
        case '8' :
        case 'K' :
        case 'k' : switch(cw.field[(myy+cw.ydim-1)%cw.ydim][myx]){
                     case '^' : if(myy--==0)myy=cw.ydim-1; 
                                if(myy--==0)myy=cw.ydim-1;
                                break;
                     case '}' :
                     case '.' :
                     case  0  :
                     case ' ' : switch(cw.field[(myy+1)%cw.ydim][myx]){
                                  case '|' :
                                  case '#' :
                                  case '!' :
                                  case '@' :
                                  case '^' :
                                  case '<' :
                                  case '>' : if(myy--==0)myy=cw.ydim-1;
                               /*              if(myy--==0)myy=cw.ydim-1;*/
                                             break;
                                } 
                                break;
                   }
                   break;
        case 'x' :
        case 'X' :
        case KEY_SRIGHT :
        case KEY_C3 :
        case '6' :
        case 'L' : if(jumped){
                     switch(cw.field[(myy+1)%cw.ydim][myx]){
                       case '#' :
                       case '|' :
                       case '!' :
                       case '@' : 
                       case '>' : 
                       case '<' :
                       case 'v' :
                       case '^' : switch(cw.field[(myy+cw.ydim-1)%cw.ydim][myx]){
                                    case '.' :
                                    case  0  :
                                    case ' ' : myy=(myy+cw.ydim-1)%cw.ydim;
                                               break;
                                  }
                                  break;

                     }
                     jumped=0; 
                     mover=1;
                   }
                   else{
                     jumped=1;
                     break;
                   }
        case KEY_RIGHT :
        case '3' :
        case 'l' : switch(cw.field[myy][(myx+1)%cw.xdim]){
                     case '>' : if(++myx==cw.xdim)myx=0;
                     case '.' :
                     case  0  :
                     case '}' :
                     case ' ' : if(++myx==cw.xdim)myx=0;
                                break;
                     case '@' : if(in!='L'){
                                  mvv=(myx+1)%cw.xdim;
                                  while(cw.field[myy][mvv]=='@')mvv=(mvv+1)%cw.xdim;
                                  switch(cw.field[myy][mvv]){
                                    case  0  :
                                    case ' ' : cw.field[myy][mvv]='@';
                                               cw.field[myy][(myx+1)%cw.xdim]=' '; 
                                               mvaddch(myy+yo,(myx+1)%cw.xdim+xo,' ');
                                               break; /*opti here - just exchange 1st with last*/
                                  }
                                  break;
                                }
                   }
                   break;
        case ' ' :
        case '0' :
        case 'R' : 
                   drawsolid(&cw);
                   break;
        case KEY_F0 :
        case KEY_HELP :
        case '/' :
        case '?' : clear();
                   move(1,1);
                   printw("\tblocks\t\t\tGPL\n\n");
                   printw("The aim is to reach the exit      \t\t    } \n");
                   printw("while eating all sweeties         \t\t    . \n");
                   printw("by pushing the blocks             \t\t    @ \n");
                   printw("into suitable positions.\n\n");
                   printw("You are                           \t\t   * +\n");
                   printw("and can move through one way doors\t\t < v ^ > \n");
                   printw("while the blocks can not.\n\n");
                   printw("To move around you can use        \t\t h j k l \n");
                   printw("or the numlocked keypad           \t\t 1 2 5 3 \n");
                   printw("while jumps are controlled by     \t\t H L 4 6 or z x\n\n");
                   printw("Redraw and quit respectively are  \t\t   R q   \n");
                   printw("To create your own maze use       \t\t   bed   \n");
                   printw("while the flags are shown by      \t\tblocks -h\n\n"); 
                   printw("\t\tNote : the maze wraps around.\n");
                   refresh();
                   getch();
                   clear();
                   break;
        case 'm' : mvprintw(0,COLS-18," kludged by mwelz");
                   break;
        default  : mvprintw(0,COLS-18,"   Try ? for help");
                   break;
      }
      switch(cw.field[myy][myx]){
        case '.' : score+=10;
                   cw.field[myy][myx]=' ';
                   break;
        case '}' : done=1; 
                   if(stricti){
                     for(i=0;i<cw.xdim;i++){
                       for(j=0;j<cw.ydim;j++){
                         if(cw.field[j][i]=='.'){
                           done=0;
                         }
                       }
                     }
                   } 
                   break;
      }
      mvprintw(LINES-1,1,"Score : %ld0   \tMoves : %ld  ",score,moves);

      if(cw.field[(myy+1)%cw.xdim][myx]=='!'){
        cw.field[(myy+1)%cw.ydim][myx]=':';
        drawsolid(&cw);
      }
      else if(cw.field[(myy+1)%cw.xdim][myx]=='|'){
        cw.field[(myy+1)%cw.ydim][myx]='!';
        drawsolid(&cw);
      }
      else if(cw.field[(myy+1)%cw.xdim][myx]==':'){
        cw.field[(myy+1)%cw.ydim][myx]='.';
        drawsolid(&cw);
      }
      else if(!(moves%speed)){
        drawsolid(&cw);
      }
      drawmob(&cw);

#ifdef NEW
      flushinp();
#endif

      if(alternate){
#ifdef UCOL
        if(mover)mvaddch(myy+yo,myx+xo,'*'|COLOR_PAIR(FF_ME));
        else     mvaddch(myy+yo,myx+xo,alternate|COLOR_PAIR(FF_ME));
#else
        if(mover)mvaddch(myy+yo,myx+xo,'*');
        else     mvaddch(myy+yo,myx+xo,alternate);
#endif
      }
      else{
#ifdef UCOL
        if(mover)mvaddch(myy+yo,myx+xo,'*'|COLOR_PAIR(FF_ME));
        else     mvaddch(myy+yo,myx+xo,'+'|COLOR_PAIR(FF_ME));
#else
        if(mover)mvaddch(myy+yo,myx+xo,'*');
        else     mvaddch(myy+yo,myx+xo,'+');
#endif
      }
      refresh();
      if((moves/100>=cw.diff)&&movei){
        done=2;
      }
    }  
  }
 
  refresh();
  endwin();
  switch(done){
    case 0 : printf("Game not completed.                            \nPity!\n");
             break;
    case 1 : if(moves/100<cw.diff){
               score+=(cw.diff-(moves/100))*10;
             } 
             printf("Game successfully completed in %ld moves.      \nYour score is %ld0 points\n",moves,score);
             break;
    case 2 : printf("Game not completed within %ld moves.           \nHard Luck!\n",moves);
             break;
  }
               /*Hope they come back again*/

  sprintf(lname,"/usr/games/lib/%s.block",cw.fname);
  /* yuck - a messy kludge - the fname stores the name of the next game*/
  moves=0;
  if(looper)looper=2;    
  
 
  }while(looper&&done);

  return 0;
}

int loadnext(struct game *s,char *fname){
  static count=0;
  FILE *fp;
  int i,j;
  int retval=0;

#ifdef DEBUG
  printf("\nAttempting to open file %s \n",fname);
#endif

  fp=fopen(fname,"r");
  if(fp!=NULL){
    fseek(fp,(long)(count*(CHARL*CHARL+FNAMEL+NAMEL+3)),SEEK_SET);
    for(i=0;i<NAMEL;i++){
      if(EOF==(s->name[i]=fgetc(fp))){
        retval=1; 
        i=NAMEL;
      }
    } 
    if(!retval){
      for(i=0;i<FNAMEL;i++){
        if(EOF==(s->fname[i]=fgetc(fp))){
          retval=2; 
        }
      }
      if(EOF==(s->xdim=fgetc(fp))){
        retval=2; 
      }
      if(EOF==(s->ydim=fgetc(fp))){
        retval=2; 
      }
      if(EOF==(s->diff=fgetc(fp))){
        retval=2; 
      }
      for(i=0;i<CHARL;i++){
        for(j=0;j<CHARL;j++){
          if(EOF==((s->field[i][j])=fgetc(fp))){
            retval=2; 
          }
#ifdef DEBUG
          if((s->field[i][j])=='#')printf("#");
          else                     printf("-");
#endif
        }
      }
    } 
    fclose(fp);
  }
  else{
    retval=2;  
  }
  return retval;
}    

int drawmob(struct game *s){
  int retval=0;
  int i,j,xo,yo,k; 

  xo=(COLS-(s->xdim))/2;
  yo=(LINES-(s->ydim))/2;

  for(i=((s->ydim)-1);(i+1)!=0;i--){
    for(j=0;j<s->xdim;j++){
      switch((s->field[i][j])){
          case '@' : switch(s->field[(i+1)%(s->ydim)][j]){
                       case  0  :
                       case ' ' : s->field[(i+1)%(s->ydim)][j]=s->field[i][j]; 
                                  s->field[i][j]=' ';
                                  mvaddch(yo+i,xo+j,(s->field[i][j]));
                                  mvaddch(yo+((i+1)%s->ydim),xo+j,(s->field[(i+1)%(s->ydim)][j]));
/*                                  if(pyr&&(s->field[(i+1)%(s->ydim)][j]!=' ')){
                                    if(s->field[(i+1)%(s->ydim)][(j+1)%(s->xdim)]==' '){
                                      s->field[(i+1)%(s->ydim)][(j+1)%(s->xdim)]=s->field[i][j];
                                      s->field[i][j]=' ';
                                      mvaddch(yo+i,xo+j,(s->field[i][j]));
                                      mvaddch(yo+((i+1)%s->ydim),xo+((j+1)%s->xdim),'@');

                                    }                   
                                  }*/
                                  break;
                     }             
#ifdef UCOL
                     mvaddch(yo+i,xo+j,(s->field[i][j])|COLOR_PAIR(FF_BLOCK));
#else
                     mvaddch(yo+i,xo+j,(s->field[i][j]));
#endif
                     break;
          case '-' : if(s->field[(i-1)%(s->ydim)][j]=='@'){  
                       s->field[(i-1)%(s->ydim)][j]=' ';  
                     }
          case '=' : if(s->field[(i-1)%(s->ydim)][j]=='@'){
                       s->field[(i-1)%(s->ydim)][j]='.';  
                     }
                     break;
      }
    }
  }    
  return retval;
}

int drawsolid(struct game *s){
  int retval=0;
  int i,j,x,y,xo,yo;

  if(COLS<(s->xdim)||LINES<(s->ydim)){
    retval=1;
  }
  else{
    xo=(COLS-(s->xdim))/2;
    yo=(LINES-(s->ydim))/2;
    
    for(i=0;i<s->ydim;i++){
      for(j=0;j<s->xdim;j++){
        switch((s->field[i][j])){
          case '}' : 
#ifdef UCOL
                     mvaddch(yo+i,xo+j,'}'|COLOR_PAIR(FF_HOME));
#else
                     if(flashi)mvaddch(yo+i,xo+j,'}'|A_BLINK);
                     else      mvaddch(yo+i,xo+j,'}'|A_STANDOUT); 
#endif
                     break; 
          case  0  :
          case '.' :
          case ':' :
          case '|' :
          case '!' :
          case ' ' : mvaddch(yo+i,xo+j,(s->field[i][j]));
                     break;
          case '=' :
          case '-' :

#ifdef UCOL
                     if(has_colors())mvaddch(yo+i,xo+j,(s->field[i][j])|A_REVERSE|COLOR_PAIR(FF_OVEN));
                     else            mvaddch(yo+i,xo+j,(s->field[i][j])|A_REVERSE); 
#else
                     mvaddch(yo+i,xo+j,(s->field[i][j])|A_REVERSE);
#endif
                     break;
          case '#' :
          case '>' :
          case '<' :
          case 'v' :
          case '^' : 
#ifdef UCOL
                     if(has_colors())mvaddch(yo+i,xo+j,(s->field[i][j])|A_REVERSE|COLOR_PAIR(FF_SOLID));
                     else            mvaddch(yo+i,xo+j,(s->field[i][j])|A_REVERSE); 
#else
                     mvaddch(yo+i,xo+j,(s->field[i][j])|A_REVERSE);
#endif
                     break;
          default  : if(isalpha(s->field[i][j])){
                       mvaddch(yo+i,xo+j,(s->field[i][j]));
                     }
                     break;
        }
      }
    }    
  }
  return retval;
}
  
  
