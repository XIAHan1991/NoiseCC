#define MAIN
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mysac.h"

/* a program to take the list files, for up to 30 files, 
   and stack all the files that are present in all 
   directories.  The list files must have the absolute 
   path as the first line.  The other lines should be 
   a file name followed by a tab then the number of days 
   of raw data that lead to this correlation.  		*/


  /* information about all files in all directories are 
     read into this af[] structure.   */

  struct onefile { /* file name and number */
    char name[40]; /* complete file name */
    int  ndays; /* number of days for the given correlation */
    int  onoff; /* tells if file has been stacked (1) or not (0) */
    float depmin; /* min signal value, from sac header depmin value */
    float depmax; /* like depmin, max signal value */
  };

  struct allfiles { /*for whole filelist with dir and len*/
    char dir[120]; /* absolute path, ending in a "/" */
    struct onefile fname[60000]; /* structure for each file */
    int len; /* total number of files in a given directory */
  } af[480]; /* up to 30 month.lst files can be used, this can be changed */


/*c/////////////////////////////////////////////////////////////////////////*/
/*--------------------------------------------------------------------------*/
        SAC_HD *read_sac (char *fname, float *sig, SAC_HD *SHD, int nmax)
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/
{
 FILE *fsac;
/*..........................................................................*/
        if((fsac = fopen(fname, "rb")) == NULL) {
          printf("could not open sac file to read%s \n", fname);
          exit(1);
        }

        if ( !fsac )
        {
          /*fprintf(stderr,"file %s not find\n", fname);*/
         return NULL;
        }

        if ( !SHD ) SHD = &SAC_HEADER;

         fread(SHD,sizeof(SAC_HD),1,fsac);

         if ( SHD->npts > nmax )
         {
          fprintf(stderr,
           "ATTENTION !!! dans le fichier %s npts est limite a %d",fname,nmax);

          SHD->npts = nmax;
         }

         fread(sig,sizeof(float),(int)(SHD->npts),fsac);

        fclose (fsac);

   /*-------------  calcule de t0  ----------------*/
   {
        int eh, em ,i;
        float fes;
        char koo[9];

        for ( i = 0; i < 8; i++ ) koo[i] = SHD->ko[i];
        koo[8] = NULL;

        SHD->o = SHD->b + SHD->nzhour*3600. + SHD->nzmin*60 +
         SHD->nzsec + SHD->nzmsec*.001;

        sscanf(koo,"%d%*[^0123456789]%d%*[^.0123456789]%g",&eh,&em,&fes);

        SHD->o  -= (eh*3600. + em*60. + fes);
   /*-------------------------------------------*/}

        return SHD;
}

#define SLEN 10000000
float sig0[SLEN];

int main(int argc, char *argv[])
{

  FILE *fp1, *fp0;
  char ch;
  int i = 0, j = 0, k = 0, quit = 0, nfiles = 0;
  int inotbase = 0, jbase = 0, jnotbs = 0;
  int div_by = 0, mindays = 0, ibase = 0;
  char dir[120], filename[60], nombre[40];
  char dump[60], outdir[80], fname[100];

  float minv = 1000000;

  /* check for comand line arg */
  if(argc!=4) {
    printf("Usage: mklist filelist outfile_directory mindays\n");
    exit(1);
  }
 
  strcpy(outdir, argv[2]); 

  /* open file for reading */
  if((fp0 = fopen(argv[1], "r"))==NULL) {
    printf("cannot open infile.\n");
    exit(1);
  }
  fgets(filename, 60, fp0); /*name of month.lst */

  /* fill the structure with all the file information for up to 
	30 files */
  do{
    i = strlen(filename);
    filename[i-1] ='\0';
    printf("file name %d is %s\n", k, filename);

    /* open file for reading */
    if((fp1 = fopen(filename, "r"))==NULL) {
      printf("cannot open infile number %d.\n", k);
      exit(1);
    }

    /* first line of file must be the absolute path*/
    fgets(dir, 120, fp1);
    i = strlen(dir);
    dir[i-1] = '\0';
    strcpy(af[k].dir, dir);

    ch = fgetc(fp1);
    do {
      i = 0;
      /* get file name */
      do {
        nombre[i] = ch;
        i++;
        ch = fgetc(fp1);
      } while((ch != '	') && (i != 39));
      /* copies string until tab encountered */
      nombre[i] = '\0';

      sprintf(fname, "%s%s", af[k].dir, nombre);
 
      /*---------------- reading sac file  -------------------*/
      if ( read_sac (fname, sig0, &SAC_HEADER, SLEN) == NULL ){
          fprintf(stderr,"file %s not found\n", fname);
          return 0;
        }

      /* judge if the sac file is a null file with only headfile no signal */     
     if( SAC_HEADER.depmin < minv ) {
       strcpy(af[k].fname[j].name, nombre);
      /* get the number of days */
      fgets(dump, 30, fp1);
      af[k].fname[j].ndays = atoi(dump);
      af[k].fname[j].onoff = 0;
      af[k].fname[j].depmin = ((-1)*SAC_HEADER.depmin);
      af[k].fname[j].depmax = SAC_HEADER.depmax;
      j++;
     } 
     else {
      fgets(dump, 30, fp1);
     }   
      ch = fgetc(fp1);
    } while(!feof(fp1));

    /* j is number of files for a given dir */
    af[k].len = j; 
    fprintf(stderr, "length of file %d is %d\n", k, af[k].len);
    j = 0;
    k++;
  
    fgets(filename, 60, fp0);
  } while(!feof(fp0));
  fprintf(stderr, "all info read fine\n\n");
  nfiles = k; /* number of files as determined from loop*/

  fclose(fp1);
  fclose(fp0);
 
  /*next section checks for file existence in all directories
    and writes script to add the files together.  It will only be 
    executed if the file is in all directories.    		*/
 
  /* inotbase is file num.  inotbase=0 is reference file*/
  jbase = 0;/* j is line of first (ibase) file	*/
  jnotbs = 0;/* jnotbs is line of second file 		*/
  
  /* increment over nfiles so if a file is in the second, third
	etc but not the first it will not be missed */
  for(ibase = 0; ibase < nfiles; ibase++){
    do{  /* outer do while */
      inotbase = ibase + 1;
      if(af[ibase].fname[jbase].onoff==0){
        /* write sac script header */
        if((fp1 = fopen("dostack.csh", "w"))==NULL) {
          fprintf(stderr, "cannot open dostack.csh \n");
          exit(1);
        }
        fprintf(stderr, "filename is %s\n", af[ibase].fname);
        af[ibase].fname[jbase].onoff=1;
        fprintf(fp1, "#!/bin/csh\n");
        fprintf(fp1, "sac << END\n");
 
        fprintf(fp1, "r %s%s\n", af[ibase].dir, af[ibase].fname[jbase].name);

	//        if(af[ibase].fname[jbase].depmin > af[ibase].fname[jbase].depmax){
	//   fprintf(fp1, "div %f\n", af[ibase].fname[jbase].depmin);
	// }
	// else fprintf(fp1, "div %f\n", af[ibase].fname[jbase].depmax);
	//fprintf(fp1, "mul %d\n", af[ibase].fname[jbase].ndays);

        fprintf(fp1, "w a\n\n");
        /* div_by will sum up total number of days from each month	*/
        div_by = af[ibase].fname[jbase].ndays; 
  
        do{ /* inner do while */
          /* if strings are different, increment jnotbs, quit if jnotbs
     	  gets to reach the end of the file			*/
          if(strcmp(af[ibase].fname[jbase].name, af[inotbase].fname[jnotbs].name)) {
            if(jnotbs == af[inotbase].len) {
	      fprintf(stderr, "match wrong\n");
	      if(inotbase != nfiles) inotbase++;
	      jnotbs = 0;
  	    }
            else jnotbs++;
          }

          /* if file names are the same, write this part of the script*/
          else {
    	    fprintf(fp1, "r %s%s\n", af[inotbase].dir, af[ibase].fname[jbase].name);
	    af[inotbase].fname[jnotbs].onoff = 1;

	    //          if(af[inotbase].fname[jnotbs].depmin > af[inotbase].fname[jnotbs].depmax){
	    //	 fprintf(fp1, "div %f\n", af[inotbase].fname[jnotbs].depmin);
	    // }
            //else fprintf(fp1, "div %f\n", af[inotbase].fname[jnotbs].depmax);
	    //fprintf(fp1, "mul %d\n", af[inotbase].fname[jnotbs].ndays);

            fprintf(fp1, "w aa\n");
	    fprintf(fp1, "r a\n");
	    fprintf(fp1, "addf aa\n");
	    fprintf(fp1, "w a\n\n");
            div_by += af[inotbase].fname[jnotbs].ndays;
	 
            inotbase++;
    	    jnotbs = 0;
          }
        } while(inotbase != nfiles); /* inner do while */
        fprintf(fp1, "r a\n");
        fprintf(fp1, "ch user0 %d\n", div_by);
        fprintf(fp1, "w %s%s\n", outdir, af[ibase].fname[jbase].name); 
        fprintf(fp1, "END\n\n");
        fclose(fp1);

        /* if inotbase = nfiles, file present in all directories */
        /* this could be made into something less than nfiles
    	  for example, if nfiles = 12, this could be executed
  	  for inotbase > 10 or something. 			*/
        if(inotbase == nfiles){  
          fprintf(stderr, "file %s present in all dirs \n", af[ibase].fname[jbase].name);
        }
        strcpy(dump, argv[3]);
        mindays=atoi(dump);
        if(div_by > mindays){
          system("csh dostack.csh"); 
	  sprintf(dump, "mv dostack.csh scripts/%d%s.csh", ibase, af[ibase].fname[jbase].name);
	  system(dump);
        }
    } /* end if loop */

    /* reset the indices */
    quit = 0;
    jnotbs = 0;
    jbase++;
    } while(jbase != af[ibase].len); /* outer do while */
    jbase = 0;
  } /* end for loop */

  return 0;
}

