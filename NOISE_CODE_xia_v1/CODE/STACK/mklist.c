#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  FILE *fp1, *fp2, *fp3, *fp4;  /* fp1 input file fp2 output file */
  char fname1[31];
  char ch, ch2;
  char fname2[31], dump[31];
  int i = 0, num1 = 0, num2 = 0, num3 = 0;

  /* check for comand line arg */
  if(argc!=3) {
    printf("Specify file name\n");
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

  /* open shell file for writing*/
  if((fp3 = fopen("filelist", "w"))==NULL) {
    printf("cannot open shell file.\n");
    exit(1);
  }

  /* open shell file for writing*/
  if((fp4 = fopen("filelist.lst", "w"))==NULL) {
    printf("cannot open shell file.\n");
    exit(1);
  }
 

  ch = fgetc(fp1);
  ch2 = fgetc(fp2);
  do {
    do {
      fname1[i] = ch;
      i++;
      ch = fgetc(fp1);
    } while((ch != '	') && (i != 29));
    fname1[i] = '\0';
    i = 0;
    fgets(dump, 30, fp1);
    num1 = atoi(dump); 
    do {
      do {
        fname2[i] = ch2;
        i++;
        ch2 = fgetc(fp2);
      } while((ch2 != '	') && (i != 29));
      fname2[i] = '\0';
      i = 0;
      fgets(dump, 30, fp2);
      num2 = atoi(dump); 
      num3 = num1 + num2;
      if(!strcmp(fname1, fname2))
      {
        fprintf(fp3, "%s %d %d\n", fname1, num1, num2);
        fprintf(fp4, "%s	%d\n", fname1, num3);
        i = 2;
      }  
      ch2 = fgetc(fp2);
    } while((!feof(fp2)) && (i != 2));
    i = 0;
    fseek(fp2, 0, SEEK_SET);
    ch = fgetc(fp1);
  } while(!feof(fp1));

  fclose(fp1);
  fclose(fp2);
  fclose(fp3);
  fclose(fp4);
 
  return 0;
}


