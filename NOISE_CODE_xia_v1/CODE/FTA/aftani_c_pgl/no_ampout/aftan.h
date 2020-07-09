#ifndef AFTAN_H

/* Finction prorotypes */

void aftanipg_(double *piover4,int *n,float *sei,double *t0,double *dt,
           double *delta,double *vmin,double *vmax,double *tmin,double *tmax,
           double *tresh,double *ffact,double *perc,int *npoints,
           double *taperl,int *nfin,double *snr,double *fmatch,
           int *npred, double pred[][],
           int *nprpv,double prpvper[],double prpvvel[],
           int *nfout1,double arr1[][],int *nfout2,double arr2[][],
           double *tamp, int *nrow,int *ncol, double ampo[][],int *ierr);
void aftanpg_(double *piover4,int *n,float *sei,double *t0,double *dt,
           double *delta,double *vmin,double *vmax,double *tmin,double *tmax,
           double *tresh,double *ffact,double *perc,int *npoints,
           double *taperl,int *nfin,double *snr,
           int *nprpv,double prpvper[],double prpvvel[],
           int *nfout1,double arr1[][],int *nfout2,double arr2[][],
           double *tamp, int *nrow,int *ncol, double ampo[][],int *ierr);
void printres(double dt,int nfout1,double arr1[][],int nfout2,
           double arr2[][],double tamp, int nrow,int ncol,
           double ampo[][],int ierr, char *name,char *pref, double delta);
void readdata(int sac,char *name,int *n,double *dt,double *delta,
              double *t0,float sei[]);
void swapn(unsigned char *b, int N, int nn);

#endif /* !AFTAN_H */
