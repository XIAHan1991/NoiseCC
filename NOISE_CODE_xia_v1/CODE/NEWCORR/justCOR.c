#define MAIN

#include <stdio.h>
#include "mysac.h"
#include <unistd.h>
#include "sac_db.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Finction prorotypes */

void dcommon_(int *len, float *amp,float *phase);
void dmultifft_(int *len,float *amp,float *phase, int *lag,float *seis_out, int *ns);

//void read_sac(char *name,char *stnam,float *stlat,float *stlon,int *n,float *sei);
void swapn(unsigned char *b, int N, int n);
//void write_cor(char *nam_f1,char *nam_corr,int *lag,float *corr,
//     char *stnam,float *stlat,float *stlon);


/*c/////////////////////////////////////////////////////////////////////////*/
/*--------------------------------------------------------------------------*/
        SAC_HD *read_sac (char *fname, float *sig, SAC_HD *SHD, int nmax)
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/
{
 FILE *fsac;
/*..........................................................................*/
        if((fsac=fopen(fname, "rb")) == NULL) return NULL;

        if ( !SHD ) SHD = &SAC_HEADER;

         fread(SHD,sizeof(SAC_HD),1,fsac);

         if ( SHD->npts > nmax ) {
           fprintf(stderr,
           "ATTENTION !!! in the file %s npts exceeds limit  %d",fname,nmax);
           SHD->npts = nmax;
         }

         fread(sig,sizeof(float),(int)(SHD->npts),fsac);

         fclose (fsac);

   /*-------------  calculate from t0  ----------------*/
   {
        int eh, em ,i;
        float fes;
        char koo[9];

        for ( i = 0; i < 8; i++ ) koo[i] = SHD->ko[i];
        koo[8] = '\0';

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
        fsac = fopen(fname, "wb");

        if ( !SHD ) SHD = &SAC_HEADER;


        SHD->iftype = (long)ITIME;
        SHD->leven = (long)TRUE;

        SHD->lovrok = (long)TRUE;
        SHD->internal4 = 6L;



  /*+++++++++++++++++++++++++++++++++++++++++*/
     SHD->depmin = sig[0];
     SHD->depmax = sig[0];
 
   for ( i = 0; i < SHD->npts ; i++ ) {
    if ( SHD->depmin > sig[i] ) SHD->depmin = sig[i];
    if ( SHD->depmax < sig[i] ) SHD->depmax = sig[i];
   }

   fwrite(SHD,sizeof(SAC_HD),1,fsac);

   fwrite(sig,sizeof(float),(int)(SHD->npts),fsac);


   fclose (fsac);
}

/*************************************************************************/
 int check_info ( SAC_DB *sdb, int ne, int ns1, int ns2 )
/*************************************************************************/
{
  //fprintf(stderr, "ne %d ns1 %d ns2 %d\n", ne, ns1, ns2);
  if ( ne >= sdb->nev ) {
    fprintf(stderr,"cannot make correlation: too large event number\n");
    return 0;
  }
  if ( (ns1>=sdb->nst) ||(ns2>=sdb->nst)  ) {
    fprintf(stderr,"cannot make correlation: too large station number\n");
    return 0;
  }
  if ( sdb->rec[ne][ns1].n <= 0 ) {
    fprintf(stderr,"no data for station %s and event %s\n", sdb->st[ns1].name, sdb->ev[ne].name );
    return 0;
  }
  if ( sdb->rec[ne][ns2].n <= 0 ) {
    fprintf(stderr,"no data for station %s and event %s\n", sdb->st[ns2].name, sdb->ev[ne].name );
    return 0;
  }
  if ( fabs(sdb->rec[ne][ns1].dt-sdb->rec[ne][ns2].dt) > .0001 ) {
    fprintf(stderr,"incompatible DT\n");
    return 0;
  }
  return 1;
}

/*c/////////////////////////////////////////////////////////////////////////*/
 float sig[10000000];
 float amp[5000000], phase[5000000], cor[10000000];
 float seis_out[10000000];
 SAC_HD shdamp1, shdph1, shdamp2, shdph2, shd_cor;

/*--------------------------------------------------------------------------*/
 int do_cor( SAC_DB *sdb, int lag )
/*--------------------------------------------------------------------------*/
{
  int ine, jsta1, jsta2, k;

  int len,ns,i; 


  char filename[200], amp_sac[200], phase_sac[200];
  FILE *ff;

  // outermost loop over day number, then station number

    for( ine = 0; ine < sdb->nev; ine++ ) {

      fprintf(stderr,"sdb->nev %d\n",ine);

    // loop over "base" station number, this will be stored into common memory

    for( jsta1 = 0; jsta1 < sdb->nst; jsta1++ ) {  

      if(sdb->rec[ine][jsta1].n > 0){
        sprintf( amp_sac, "%s.am", sdb->rec[ine][jsta1].ft_fname );
        sprintf( phase_sac, "%s.ph", sdb->rec[ine][jsta1].ft_fname );

// read amp and phase files and read into common memory
        if ( read_sac(amp_sac, amp, &shdamp1, 5000000 )==NULL ) {
  	  fprintf( stderr,"file %s did not found\n", amp_sac );
   	  goto loop1;
        }
        if ( read_sac(phase_sac, phase, &shdph1, 5000000)== NULL ) {
          fprintf( stderr,"file %s did not found\n", phase_sac );
          goto loop1;
        }

	len = shdamp1.npts;
        printf("len1=%d\n",len);

        dcommon_( &len, amp, phase ); // reads amp and phase files into common memory

     for( jsta2 = (jsta1 +1); jsta2 <= sdb->nst; jsta2++ ) {

	    sprintf(filename, "COR/COR_%s_%s.SAC",
	      sdb->st[jsta1].name, sdb->st[jsta2].name);
       if(access(filename, F_OK) != 0) {  

  	  if(sdb->rec[ine][jsta2].n > 0){

	    // compute correlation
	    sprintf(amp_sac, "%s.am", sdb->rec[ine][jsta2].ft_fname);
            sprintf(phase_sac, "%s.ph", sdb->rec[ine][jsta2].ft_fname);
	  fprintf(stderr,"file %s  %s\n", sdb->rec[ine][jsta1].ft_fname,sdb->rec[ine][jsta2].ft_fname );
            // get array of floats for amp and phase of first signal

            if ( read_sac(amp_sac, amp, &shdamp2, 5000000) ==NULL ) {
              fprintf(stderr,"file %s did not found\n", amp_sac );
              goto loop2;
            }

            if ( read_sac(phase_sac, phase, &shdph2, 5000000)==NULL ) {
              fprintf(stderr,"file %s did not found\n", phase_sac );
              goto loop2;
            }
     
	      len = shdamp2.npts;
            printf("len2=%d\n",len);

            if(!check_info(sdb, ine, jsta1, jsta2 )) {
              fprintf(stderr,"files incompatible\n");
              return 0;
            }
            else
            {

                 dmultifft_(&len, amp, phase, &lag, seis_out,&ns);

                   cor[lag] = seis_out[0];
                   for( i = 1; i< (lag+1); i++)
                 { 
     	           cor[lag-i] =  seis_out[i];
	           cor[lag+i] =  seis_out[ns-i];
	         }


  	    // move and rename cor file accordingly 
	    sprintf(filename, "COR/COR_%s_%s.SAC.prelim",
	      sdb->st[jsta1].name, sdb->st[jsta2].name);

	    if(access(filename, F_OK) == 0) { // if file alread present, do this
	      if ( !read_sac (filename, sig, &shd_cor, 10000000 ) ) {
	        fprintf(stderr,"file %s did not found\n", filename );
	        return 0;
	      }
	      // add new correlation to previous one
 	      for(k = 0; k < (2*lag+1); k++) sig[k] += cor[k];
             
	      write_sac (filename, sig, &shd_cor );
	    }
	    // if file doesn't already exist, use one of the current headers
	    // and change a few values. more may need to be added
	     else {
	      shdamp1.delta = sdb->rec[ine][jsta1].dt;
	      shdamp1.evla =  sdb->st[jsta1].lat;
	      shdamp1.evlo =  sdb->st[jsta1].lon;
	      shdamp1.stla =  sdb->st[jsta2].lat;
	      shdamp1.stlo =  sdb->st[jsta2].lon;
	      shdamp1.npts =  2*lag+1;
              shdamp1.b    = -(lag)*shdamp1.delta;
	      write_sac (filename, cor, &shdamp1);
	     }
 	    }   //loop over check
	  }    // loop over if filename exists
	 }    //loop over if jsta2

      else if (access(filename, F_OK) == 0)
	    { 
              printf("COR_%s_%s.SAC has existed \n",sdb->st[jsta1].name, sdb->st[jsta2].name);
            }
      loop2: ;
       }   //loop over jsta2
      }  //loop over if jsta1
    loop1: ;
     }  //loop over jsta1
    }  //loop over events
  return 0;
}


SAC_DB sdb;

/*c/////////////////////////////////////////////////////////////////////////*/
/*--------------------------------------------------------------------------*/
int main (int na, char *arg[])
/*--------------------------------------------------------------------------*/
{
  FILE *ff;
  int ns1 = 0, ns2 = 0,lag;
  char str[600], filename[200];

  if ( na < 2 )
    {
      fprintf(stderr,"usage: corr_filtered_5000 lag\n");
      exit(1);
    }

  sscanf(arg[1],"%d", &lag );


  ff = fopen("sac_db.out","rb");
  fread(&sdb, sizeof(SAC_DB), 1, ff );
  fclose(ff);

fprintf(stderr, "read info fine\n");

  // do all the work of correlations here

  do_cor(&sdb,lag);  


  fprintf(stderr, "finished correlations\n");

  // move COR/COR_STA1_STA2.SAC.prelim to COR/COR_STA1_STA2.SAC

  for ( ns2 = 1; ns2 < sdb.nst; ns2++ ) for ( ns1 =0 ; ns1 < ns2; ns1++ ) {
    sprintf(filename, "COR/COR_%s_%s.SAC.prelim", sdb.st[ns1].name, sdb.st[ns2].name);
    sprintf(str, "mv COR/COR_%s_%s.SAC.prelim COR/COR_%s_%s.SAC",
      sdb.st[ns1].name, sdb.st[ns2].name, sdb.st[ns1].name, sdb.st[ns2].name);
    if(access(filename, F_OK) == 0) system(str);
  }

  return 0;
}
