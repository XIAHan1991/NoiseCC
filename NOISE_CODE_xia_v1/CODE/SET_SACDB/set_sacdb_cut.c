#define MAIN
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mysac.h"
#include "sac_db.h"

SAC_HD *read_sac (char *fname, float *sig, SAC_HD *SHD, int nmax);
/*c/////////////////////////////////////////////////////////////////////////*/
/*--------------------------------------------------------------------------*/
        int isign(double f)
/*--------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/
{
/*..........................................................................*/
        if (f < 0.)     return -1;
        else            return 1;
}
/*c/////////////////////////////////////////////////////////////////////////*/
/*--------------------------------------------------------------------------*/
        int nint(double f)
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
{
        int i;
        double df;
/*..........................................................................*/
                           i=(int)f;
        df=f-(double)i;
        if (fabs(df) > .5) i=i+isign(df);

        return i;
}

/*c/////////////////////////////////////////////////////////////////////////*/
/*--------------------------------------------------------------------------*/
	SAC_HD *read_sac (char *fname, float *sig, SAC_HD *SHD, int nmax)
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/
{
 FILE *fsac;
/*..........................................................................*/
	fsac = fopen(fname, "rb");
	if ( !fsac )
	{
	 fclose (fsac);
	 return NULL;
	}

	if ( !SHD ) SHD = &SAC_HEADER;

	 fread(SHD,sizeof(SAC_HD),1,fsac);

	 if ( SHD->npts > nmax )
	 {
	   /*fprintf(stderr,
	     "ATTENTION !!! dans le fichier %s npts est limite a %d",fname,nmax);*/

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
	fsac = fopen(fname, "wb");

	if ( !SHD ) SHD = &SAC_HEADER;


        SHD->iftype = (int)ITIME;
        SHD->leven = (int)TRUE;

        SHD->lovrok = (int)TRUE;
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


/*c/////////////////////////////////////////////////////////////////////////*/
/*--------------------------------------------------------------------------*/
	void sac_db_write_to_asc ( SAC_DB *sdb, char *fname )
/*--------------------------------------------------------------------------*/
{
  int ie, is;
  FILE *fi, *ff;
  static SAC_HD shd;

  ff = fopen(fname,"w");

  for ( ie = 0; ie < sdb->nev; ie++ ) for ( is = 0; is < sdb->nst; is++ )
    {
      fprintf(ff,"%s  %s  ", sdb->ev[ie].name, sdb->st[is].name );
      if ( sdb->rec[ie][is].n <= 0 ) fprintf(ff,"NO DATA\n");

      else 
	{
	  fi = fopen(sdb->rec[ie][is].fname,"rb");
	  fread(&shd, sizeof(SAC_HD), 1, fi );
	  fclose(fi);

	  fprintf(ff,"%s  t0: %d/%d:%d:%d:%g  %g s of record\n", sdb->rec[ie][is].fname, 
		   shd.nzyear, shd.nzjday, shd.nzhour, shd.nzmin, 
		   (shd.nzsec + 0.001*shd.nzmsec), shd.delta*(shd.npts-1) );
	}
    } 

  fclose(ff);
}


/*////////////////////////////////////////////////////////////////////////*/
/*------------------------------------------------------------------------*/
 int jday ( int y, int m, int d )
/*------------------------------------------------------------------------*/
{
  int jd = 0;
  int i;
 
  for ( i = 1; i < m; i++ )
    {
      if ( (i==1) || (i==3) || (i==5) || (i==7) || (i==8) || (i==10) ) jd += 31;
      else if (i==2)
        {
          if ( y == 4*(y/4) ) jd += 29;
          else jd += 28;
        }
      else jd += 30;
    }
 
  return jd + d;
}


/*////////////////////////////////////////////////////////////////////////*/
/*------------------------------------------------------------------------*/
 double abs_time ( int yy, int jday, int hh, int mm, int ss, int ms )
/*--------------------------------------------------------------------------
     computes time in s relative to 1900
--------------------------------------------------------------------------*/
{
  int nyday = 0, i;
  double abssec;
  for ( i = 1901; i < yy; i++ )
    {
      if ( 4*(i/4) == i ) nyday += 366;
      else nyday += 365;
    }
  abssec =  24.*3600.*(nyday+jday) + 3600.*hh + 60.*mm + 1.*ss + 0.001*ms;
  return abssec;
}

/*////////////////////////////////////////////////////////////////////////*/
/*------------------------------------------------------------------------*/
float av_sig (float *sig, int i, int N, int nwin )
/*------------------------------------------------------------------------*/
{
  int n1, n2, j, nav = 0;
  float av = 0.;

  if ( nwin > N ) nwin = N;

  n1 = i - nwin/2;

  if ( n1 < 0 ) n1 = 0;

  n2 = n1 + nwin - 1;

  if ( n2 > N-1 ) n2 = N-1;

  n1 = n2 - nwin + 1;

  for ( j = n1; j <= n2; j++ ) if ( sig[j] < 1.e29 )
    {
      av += sig[j];
      nav++;
    }

  if ( nav < 1 ) av = 1.e30;

  else av = av/(float)nav;

  return av;
}

/*/////////////////////////////////////////////////////////////////////////*/

char str[300];
char fname[300][300];
double t1[300], t2[300], T1, T2;
SAC_HD sd, s0;
int nf;


#define NPTSMAX 10000000
float sig0[NPTSMAX], sig1[NPTSMAX];

/*////////////////////////////////////////////////////////////////////////*/
/*------------------------------------------------------------------------*/
 void mk_one_rec (SAC_DB *sdb, int ne, int ns, char *nseed, char *ch)
/*------------------------------------------------------------------------*/
{
  FILE *ff;
  static char resp_name[150];
  char seisname;
  char filename[300]; 
  double t1;
  
  if ( sdb->rec[ne][ns].n > 0 ) return;

  sprintf(filename, "%s/%s.%s.SAC\0", sdb->ev[ne].name, sdb->st[ns].name, ch);

  if ( access(filename,F_OK) != 0) return;

  printf("filename %s iev %d ista %d\n",filename, ne,ns);

         if ( !read_sac (filename, sig1, &sd, NPTSMAX) )
	{
	  fprintf(stderr,"file %s not found\n", filename );
	}
  printf(" sd.nzyear %d,sd.nzjday %d,sd.nzhour %d ,sd.nzmin %d,sd.nzsec %d,sd.nzmsec %d\n%d\n",
	   sd.nzyear,   sd.nzjday,   sd.nzhour,    sd.nzmin,   sd.nzsec,   sd.nzmsec,sd.npts);


 
  t1 = abs_time(sd.nzyear,sd.nzjday,sd.nzhour,sd.nzmin,sd.nzsec,sd.nzmsec);
//  printf("t1= %f \n",t1);
  sdb->rec[ne][ns].t0 = t1;
  sdb->rec[ne][ns].dt = sd.delta;
  sdb->rec[ne][ns].n =  sd.npts;
 
  /*---------- response file -----------*/

  sprintf(str,"ls %s/RESP*%s*%s* > list_resp\0", sdb->ev[ne].name, sdb->st[ns].name, ch);

  system(str);
  ff = fopen("list_resp","r");
   if ( fscanf(ff,"%s", resp_name ) == EOF )
    {
      sdb->rec[ne][ns].n = 0;
      return;
    }
  fclose(ff);

  sprintf(sdb->rec[ne][ns].resp_fname,"%s\0",  resp_name);

  sprintf(sdb->rec[ne][ns].fname,"%s/%s.%s.SAC\0", sdb->ev[ne].name, sdb->st[ns].name, ch);
  sprintf(sdb->rec[ne][ns].ft_fname,"%s/ft_%s.%s.sac\0", sdb->ev[ne].name, sdb->st[ns].name, ch);
  sprintf(sdb->rec[ne][ns].chan,"%s\0", ch );
  
  printf("make_one_rec  sdb->rec[ne][ns].t0 %f\n",sdb->rec[ne][ns].t0);
  
}

/*////////////////////////////////////////////////////////////////////////*/
/*------------------------------------------------------------------------*/
 void  fill_one_sta (STATION *st, char *buff )
/*------------------------------------------------------------------------*/
{
  int i;
//  for ( i = 0; i < 6; i++ )
//    {
//      if ( buff[i] == ' ' ) break;
//      else st->name[i] = buff[i];
//    } 

//  st->name[i] = '\0';
//  sscanf(&(buff[7]),"%g%g", &(st->lon), &(st->lat) );
 
 	sscanf(buff, "%s %g %g", st->name, &(st->lon), &(st->lat) ); 
}

/*////////////////////////////////////////////////////////////////////////*/
/*------------------------------------------------------------------------*/
  void fill_one_event (EVENT *ev, char *buff )
/*------------------------------------------------------------------------*/
{
  sscanf(&(buff[0]),"%d", &(ev->yy) );
  sscanf(&(buff[7]),"%d", &(ev->mm) );
  sscanf(&(buff[10]),"%d", &(ev->dd) );
 sscanf(&(buff[13]),"%2d", &(ev->h) );
  sscanf(&(buff[15]),"%2d", &(ev->m) );
  sscanf(&(buff[17]),"%2d", &(ev->s) );
/*  sscanf(&(buff[20]),"%2d", &(ev->ms) );
  ev->h = 16;
  ev->m = 0;
  ev->s = 0; */
  ev->ms = 0; 

  printf("ev->yy %d, ev->mm %d,ev->dd %d, ev->h %f, ev->m %f, ev->s %f, ev->ms %f\n",
	  ev->yy,    ev->mm,   ev->dd,    ev->h,    ev->m,    ev->s,     ev->ms);

  ev->ms = 10.*ev->ms;

  ev->jday = jday( ev->yy, ev->mm, ev->dd );

  ev->t0 = abs_time (ev->yy, ev->jday, ev->h, ev->m, ev->s, ev->ms );
  sprintf(ev->name,"%d_%d_%d_%d_%d_%d\0",ev->yy, ev->mm, ev->dd, ev->h, ev->m, ev->s );
  printf(" ev->t0 %f\n",ev->t0);
  system(str);
}


/*========================================================================*/


SAC_DB sdb;
char buff[300];

/*////////////////////////////////////////////////////////////////////////*/
/*------------------------------------------------------------------------*/
 int main (void)
/*------------------------------------------------------------------------*/
{
  int ist, iev;
  FILE *ff;

  for ( iev = 0; iev < NEVENTS; iev++ ) for ( ist = 0; ist < NSTATION; ist++ ) sdb.rec[iev][ist].n = 0;

  fprintf(stderr,"initializing DB ok\n");


  ff = fopen("station.lst","r");

  for ( ist = 0; ; ist++ )
    {
      if ( !fgets(buff,300,ff) ) break;

      puts(buff);

      fill_one_sta (&(sdb.st[ist]), buff );

      fprintf(stderr,"filling station %s\n", sdb.st[ist].name );
    }

  sdb.nst = ist;

  fclose(ff);


  ff = fopen("event.dat","r");

  for ( iev = 0;;iev++ )
    {
      if ( !fgets(buff,300,ff) ) break;

	  fill_one_event (&(sdb.ev[iev]), buff );

	  for ( ist = 0; ist < sdb.nst; ist++ )
	    {
	      mk_one_rec (&sdb, iev, ist, buff, "LHZ");
	    }
    }



  sdb.nev = iev;
  fclose(ff);

  printf(" events %d  station %d\n", iev,ist);

  ff = fopen("sac_db.out","wb");
  fwrite(&sdb, sizeof(SAC_DB), 1, ff );
  fclose(ff);

  sac_db_write_to_asc ( &sdb, "event_station.tbl" );
}
