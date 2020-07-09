#define MAIN

#include "koftan.h"
#include "sac_db.h"
#include "mysac.h"


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
int one_pair(char *name1 )
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

  Evla = SAC_HEADER.evla;
  Evlo = SAC_HEADER.evlo;
  Stla = SAC_HEADER.stla;
  Stlo = SAC_HEADER.stlo;
  


  if((fp1 = fopen("change.csh", "w"))==NULL) {
    printf("cannot open change.csh.\n");
    exit(1);
  }



  fprintf(fp1, "sac << END\n");
  fprintf(fp1, "readhdr %s\n", fname);
  fprintf(fp1, "ch stlo %f\n", Evlo);
  fprintf(fp1, "ch stla %f\n", Evla);
  fprintf(fp1, "ch evlo %f\n", Stlo);
  fprintf(fp1, "ch evla %f\n", Stla);
  fprintf(fp1, "writehdr\n");
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
  int ns1 = 0, ns2 = 4, n = 0, len = 0, i,j;
  char filename[29], fname[29], shortname[26];
  char staname1[8], staname2[8], dump[500];
  char ch, ch1;

  if((ff = fopen("filelist_SAC", "r"))==NULL) {
    printf("cannot open filelist_SAC\n");
    exit(1);
  }
 


  for(;;)
    {
      if(fscanf(ff,"%s",&filename)==EOF)
	break;

// remove quotes from file names
      j = strlen(filename);
     filename[j] = '\0';

    printf("file is %s", filename);

    if ( !one_pair(filename) ) continue;

    }

   fclose(ff);

}
