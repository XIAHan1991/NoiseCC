#define MAIN

#include "koftan.h"
/*#include "../SAC_FROM_SEED/sac_db.h"*/
#include "sac_db.h"


/*c/////////////////////////////////////////////////////////////////////////*/
/*--------------------------------------------------------------------------*/
        SAC_HD *read_sac (char *fname, float *sig, SAC_HD *SHD, long nmax)
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/
{
 FILE *fsac;
/*..........................................................................*/
        if((fsac = fopen(fname, "rb")) == NULL) {
          printf("could not open sac file to write\n");
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

/*c/////////////////////////////////////////////////////////////////////////*/
/*--------------------------------------------------------------------------*/
        void write_sac (char *fname, float *sig, SAC_HD *SHD)
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/
{
 FILE *fsac;
 int i;
/*..........................................................................*/
        if((fsac = fopen(fname, "wb"))==NULL) {
           printf("could not open sac file to write\n");
           exit(1);
        }

        if ( !SHD ) SHD = &SAC_HEADER;


        SHD->iftype = (long)ITIME;
        SHD->leven = (long)TRUE;

        SHD->lovrok = (long)TRUE;
        SHD->internal4 = 6L;



  /*+++++++++++++++++++++++++++++++++++++++++*/
     SHD->depmin = sig[0];
     SHD->depmax = sig[0];
 
   for ( i = 0; i < SHD->npts ; i++ )
   {
    if ( SHD->depmin > sig[i] ) SHD->depmin = sig[i];
    if ( SHD->depmax < sig[i] ) SHD->depmax = sig[i];
   }

         fwrite(SHD,sizeof(SAC_HD),1,fsac);

         fwrite(sig,sizeof(float),(int)(SHD->npts),fsac);


        fclose (fsac);
}


/*c/////////////////////////////////////////////////////////////*/
/* one_pair is the main routine to run the other subroutines.  */
#define SLEN 200000
float sig0[SLEN];
char fname[300];
/*--------------------------------------------------------------*/
int one_pair(char *name1, char *staname1, char *staname2 )
/*--------------------------------------------------------------*/
{
  float b, dist, dt, tmax, Umin = 1.;
  float Evla, Evlo, Stla, Stlo;
  char Statname[9], Evname[9];
  char ch1;
  FILE *fp1;

  sprintf(fname, "%s\0", name1);

  printf("fname is %s\n\n", fname);

  /*---------------- reading sac file  -------------------*/
  if ( read_sac (fname, sig0, &SAC_HEADER, SLEN) == NULL )
    {
      fprintf(stderr,"file %s not found\n", fname);
      return 0;
    }

  b = SAC_HEADER.b;
  dt = SAC_HEADER.delta;
  dist = SAC_HEADER.dist;
  Evla = SAC_HEADER.evla;
  Evlo = SAC_HEADER.evlo;
  Stla = SAC_HEADER.stla;
  Stlo = SAC_HEADER.stlo;

  if((fp1 = fopen("change.csh", "w"))==NULL) {
    printf("cannot open change.csh.\n");
    exit(1);
  }
  sprintf(Statname, staname1);
  strcpy(SAC_HEADER.kstnm, Statname);

  printf("sta1 %s sta2 %s\n", staname1, staname2);
  printf("sta1len %d sta2len %d\n", strlen(staname1), strlen(staname2));

  fprintf(fp1, "sac << END\n");
  fprintf(fp1, "r %s\n", fname);
  fprintf(fp1, "ch kevnm %s\n", staname1);
  fprintf(fp1, "w %s\n", fname);
  fprintf(fp1, "cut 0 5000\n");
  fprintf(fp1, "r %s\n", fname);
  fprintf(fp1, "ch o 0\n");
  fprintf(fp1, "ch kevnm %s\n", staname1);
  fprintf(fp1, "w %s_p\n", fname);

  fprintf(fp1, "cut -5000 0\n");
  fprintf(fp1, "r %s\n", fname);
  fprintf(fp1, "reverse\n");
  fprintf(fp1, "ch b 0\n");
  fprintf(fp1, "ch e 5000\n");
  fprintf(fp1, "ch o 0\n");
  fprintf(fp1, "w %s_n\n", fname);

  fprintf(fp1, "cut off\n");
  fprintf(fp1, "addf %s_p\n", fname);
  fprintf(fp1, "ch o 0\n");
  fprintf(fp1, "div 2\nw %s_s\n", fname);
  fprintf(fp1, "END\n\n");

  fclose(fp1);
  system("csh change.csh");

  return 1;
}


/*c/////////////////////////////////////////////////////////////*/
char fname[300], str[300];
SAC_DB sdb;
/*--------------------------------------------------------------*/
int main (void)
/*--------------------------------------------------------------*/
{
  float stla, stlo, evla, evlo, dist;
  FILE *ff;
  int ns1 = 0, ns2 = 4, n = 0, len = 0, i;
  char filename[29], fname[29], shortname[26];
  char staname1[8], staname2[8], dump[500];
  char ch, ch1;

  if((ff = fopen("filelist", "r"))==NULL) {
    printf("cannot open filelist.\n");
    exit(1);
  }
 
  ch = fgetc(ff);
  do
   {
    fgets(dump, 4, ff);
    i = 0;
    ch = fgetc(ff);
    do{
      staname1[i] = ch;
      ch = fgetc(ff);
      i++;
    }while(ch!='_');
    staname1[i] = '\0';
    i = 0;
    ch = fgetc(ff);
    do{
      staname2[i] = ch;
      ch = fgetc(ff);
      i++;
    }while(ch!='.');
    staname2[i] = '\0';
    fgets(dump, 200, ff);

    sprintf(filename, "COR_%s_%s.SAC_TT", staname1, staname2);

    if ( !one_pair(filename, staname1, staname2) ) continue;

    n++;
    ch = fgetc(ff);
   } while(!feof(ff));

   fclose(ff);

}
