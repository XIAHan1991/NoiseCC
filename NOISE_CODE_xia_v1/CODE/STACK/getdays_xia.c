#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* This program outputs the number of days used in a
   give COR or CS file.
   this program is designed to have 2 input files.
   the first is a list of all COR or CS files obtained
   from a ls CS* > l1.  the second input file is the 
   corresponding event_station.tbl file.  The output
   is a file with the COR filenames followed by the 
   number of days that were stacked to obtain that file. 
   The output is meant to be used with mklist.c which 
   gives a file that can be used for SAC macros */

int main(int argc, char *argv[])
{
  FILE *fp1, *fp2, *fp3;  /* fp1 input file fp2 output file */
  char fname1[31];
  char fname2[31];
  char outname[200];
  int i = 1;

  int gday = 0, j = 0, k = 1;
  char ch;
  char f3a[55], f3b[55], fname4[55], junk[85];
  char isno[3];
  char num[3];
  int good1[32], good2[32];

  /* check for comand line arg */
  if(argc!=3) {
    printf("usage: getdays filelist event_station.tbl\n");
    exit(1);
  }

  /* open file for reading */
  if((fp1 = fopen(argv[1], "r"))==NULL) {
    printf("cannot open infile.\n");
    exit(1);
  }

  /* open file for reading */
  if((fp2 = fopen(argv[2], "r"))==NULL) {
    printf("cannot open second infile.\n");
    exit(1);
  }

  strcpy(outname, argv[1]);
  strcat(outname, ".lst");

  /* open shell file for reading */
  if((fp3 = fopen(outname, "w"))==NULL) {
    printf("cannot open shell file.\n");
    exit(1);
  }


  ch = fgetc(fp1);
  do {

  /* initiate good arrays */
  for(i = 0; i < 32; i++) {
    good1[i] = 0;
    good2[i] = 0;
  }

  /* get two station names from filelist */
    fgets(junk, 4, fp1);
    i = 0;
    ch = fgetc(fp1);
    do {
      f3a[i] = ch;
      i++;
      ch = fgetc(fp1);
    } while(ch != '_');
    f3a[i] = '\0';
    ch = fgetc(fp1);
    i = 0;
    do {
      f3b[i] = ch;
      i++;
      ch = fgetc(fp1);
    } while(ch != '\12');
    f3b[i-4] = '\0';
/*    fgets(junk, 10, fp1); */

   

  /* get name from event station and compare */
    do {
/* get the day */
      do {
        ch = fgetc(fp2);
      } while( ch != '_');
      ch = fgetc(fp2);
      ch = fgetc(fp2);
      if(ch != '_') ch = fgetc(fp2);
      num[0] = fgetc(fp2);
      ch = fgetc(fp2);
      if(ch != '_') {
        num[1] = ch;
        num[2] = '\0';
      }
      else num[1] = '\0'; 
      j = atoi(num);
      do {
        ch = fgetc(fp2);
      } while( ch != ' ');
      ch = fgetc(fp2);
      ch = fgetc(fp2);
      i = 0;

/* get the sta name */
      do {
        fname4[i] = ch;
        ch = fgetc(fp2);
        i++;
      } while( ch != ' ');
      fname4[i] = '\0';
   
      if (!strcmp(f3a, fname4)) {
        ch = fgetc(fp2);
        fgets(isno, 3, fp2);
        if(strcmp(isno, "NO")) good1[j] = 1;
      }
      else if (!strcmp(f3b, fname4)) {
        ch = fgetc(fp2);
        fgets(isno, 3, fp2);
        if(strcmp(isno, "NO")) good2[j] = 1;
      }
      if(!strcmp(f3a,f3b))
      {
        good2[j]=good1[j];
      }
      fgets(junk, 100, fp2);
      /*      printf("%s %s %s %d %d %s\n", f3a, f3b, fname4, gday, j, junk);*/ 
      ch = fgetc(fp2);
    } while(!feof(fp2));

    j = 0;
    fprintf(fp3, "COR_%s_%s.SAC	", f3a, f3b);
    for( i = 0; i< 32; i++) { 

      /*      j += floor((good1[i] + good2[i])/2);*/

      if(good1[i] == 1 && good2[i] == 1) j = j+1;

    }
    fprintf(fp3, "%d\n", j);
  
    gday = 0; 
    fseek(fp2, 0, SEEK_SET);
    ch = fgetc(fp1);
  } while(!feof(fp1)); 

  return 0;
}


