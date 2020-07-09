c
c aftan3 function. Provides regular ftan analysis, jumps correction,
c noise RMS computations (SNR) and amplitude map 
c output for input periods.
c
      subroutine aftan3(n,sei,t0,dt,delta,vmin,vmax,tmin,tmax,tresh,ffact,
     *           perc,npoints,taperl,nfin,fsnr,nfout1,arr1,nfout2,arr2,
     *           tamp,nrow,ncol,amp,ierr);
c======================================================================
c Parameters for aftan3 function:
c Input parameters:
c n       - number of input samples, (integer*4)
c sei     - input array length of n, (real*4)
c t0      - time shift of SAC file in seconds, (real*8)
c dt      - sampling rate in seconds, (real*8)
c delta   - distance, km (real*8)
c vmin    - minimal group velocity, km/s (real*8)
c vmax    - maximal value of the group velocity, km/s (real*8)
c tmin    - minimal period, s (real*8)
c tmax    - maximal period, s (real*8)
c tresh   - treshold, usualy = 10, (real*8)
c ffact   - factor to automatic filter parameter, usualy =1, (real*8)
c perc    - minimal length of of output segment vs freq. range, % (real*8)
c npoints - max number points in jump, (integer*4)
c taperl  - factor for the left end seismogram tapering,
c           taper = taperl*tmax,    (real*8)
c nfin    - starting number of frequencies, nfin <= 32,(integer*4)
c fsnr    - NOT USED, for future extensions (real*8)
c ==========================================================
c Output parameters are placed in 2-D arrays arr1 and arr2,
c arr1 contains preliminary results and arr2 - final.
c ==========================================================
c nfout1 - output number of frequencies for arr1, (integer*4)
c arr1   - the first nfout1 raws contain preliminary data,
c          (real*8 arr1 (5 x n), n >= nfin)
c          arr1(1,:) -  central periods, s (real*8)
c          arr1(2,:) -  observed periods, s (real*8)
c          arr1(3,:) -  group velocities, km/s (real*8)
c          arr1(4,:) -  amplitudes, Db (real*8)
c          arr1(5,:) -  discrimination function, (real*8)
c          arr1(6,:) -  signal/noise ratio, Db (real*8)
c nfout2 - output number of frequencies for arr2, (integer*4)
c          If nfout2 == 0, no final result.
c arr2   - the first nfout2 raws contains final data,
c          (real*8 arr2 (4 x n), n >= nfin)
c          arr2(1,:) -  central periods, s (real*8)
c          arr2(2,:) -  observed periods, s (real*8)
c          arr2(3,:) -  group velocities, km/s (real*8)
c          arr2(4,:) -  amplitudes, Db (real*8)
c          arr2(5,:) -  signal/noise ratio, Db (real*8)
c          tamp      -  time to the beginning of ampo table, s (real*8)
c          nrow      -  number of rows in array ampo, (integer*4)
c          ncol      -  number of columns in array ampo, (integer*4)
c          amp       -  Ftan amplitude array, Db, (real*8)
c ierr   - completion status, =0 - O.K.,           (integer*4)
c                             =1 - some problems occures
c                             =2 - no final results
c======================================================================

      implicit none
      include 'fftw3.h'
      integer*4 n,npoints,nf,nfin,nfout1,ierr,nrow,ncol
      real*8    perc,taperl,tamp,arr1(6,100),arr2(5,100)
      real*8    t0,dt,delta,vmin,vmax,tmin,tmax,tresh,ffact,ftrig(100)
      real*8    fsnr
      real*4    sei(6000)
      double complex dczero,s(6000),sf(4096),fils(4096),tmp(4096)
      real*8    grvel(100),tvis(100),ampgr(100),om(100),per(100),tim(100)
      real*8    pha(4096,32),amp(4096,32),ampo(4096,32)
      real*8    time(4096),v(4096),b(4096)
      real*8    alpha,pi,omb,ome,dom,step,amax,t,dph,tm,ph
      integer*4 j,k,m,ntapb,ntape,ne,nb,ntime,ns,ntall,ici,iciflag,ia
      real*8    plan1,plan2
      integer*4 ind(2,4096)
      real*8    ipar(4,4096)
      real*8    grvel1(100),tvis1(100),ampgr1(100),ftrig1(100)
      real*8    trig1(100),grvelt(100),tvist(100),ampgrt(100)
c ---
      integer*4 njump,nijmp,i,ii(100),nii,ijmp(100),ki,kk,istrt,ibeg,iend,ima
      real*8    dmaxt,wor,per2(100),om1(100),snr(100),snr1(100),snrt(100)
      integer*4 iflag,ierr1,nindx,imax,iimax,ipos,ist,ibe,nfout2,indx(100)
      integer*4 mm,m1,mi
      real*8    lm,rm

      ierr = 0
      dczero = (0.0d0,0.0d0)
      lm = 0.0d0
      rm = 0.0d0
      pi = datan(1.0d0)*4.0d0
c number of FTAN filters
      nf = nfin
c automatic width of filters * factor ffact
      alpha = ffact*20.0d0*dsqrt(delta/1000.0d0)
c  number of samples for tapering, left end
      ntapb = taperl*tmax/dt
c  number of samples for tapering, right end
      ntape = tmax/dt
c [omb,ome] - frequency range
      omb = 2.0d0*pi/tmax
      ome = 2.0d0*pi/tmin
c seismgram tapering
      nb = max0(2,nint(delta/vmax/dt))
      tamp = (nb-1)*dt+t0;
      ne = min0(n,nint(delta/vmin/dt))
      nrow = nfin
      ncol = ne-nb+1
c times for FTAN map
      do k = nb,ne
        time(k) = (k-1)*dt
c velocity for FTAN map
        v(k) = delta/(time(k)+t0)
      enddo
      ntime = ne-nb+1
c tapering both ends of seismogram
      call taper(max0(nb,ntapb+1),min0(ne,n-ntape),n,sei,ntapb,ntape,s,ns);
c prepare FTAN filters
      dom = 2*pi/ns/dt
      step =(dlog(omb)-dlog(ome))/(nf -1)
c log scaling for frequency
      do k = 1,nf
        om(k) = dexp(log(ome)+(k-1)*step)
        per(k) = 2*pi/om(k)
      enddo
c make backward FFT for seismogram: s ==> sf
      call dfftw_plan_dft_1d(plan1,ns,s,sf,
     *                         FFTW_BACKWARD, FFTW_ESTIMATE)
      call dfftw_execute(plan1)
      call dfftw_destroy_plan(plan1)
c filtering and FTAN amplitude diagram construction
      call dfftw_plan_dft_1d(plan2,ns,fils,tmp,
     *                         FFTW_FORWARD, FFTW_ESTIMATE)
c main loop by frequency
      do k = 1,nf
c filtering
        call ftfilt(alpha,om(k),dom,ns,sf,fils, b)
c fill with zeros half spectra for Hilbert transformation and
c spectra ends ajastment
        do m = ns/2+2,ns
          fils(m) = dczero
        enddo
        fils(1) = dcmplx(dreal(fils(1))/2.0d0,0.0d0)
        fils(ns/2+1) = dcmplx(dreal(fils(ns/2+1)),0.0d0)
c forward FFT: fils ==> tmp
        call dfftw_execute(plan2)
        do m = 1,ns
          tmp(m) = tmp(m)/ns
        enddo
        j = 1
c extraction from FTAN map area of investigation
        do m = nb-1,ne+1
          pha(j,k) = datan2(dimag(tmp(m)),dreal(tmp(m)))
          wor = cdabs(tmp(m))
          ampo(j,k) = wor
          amp(j,k) = 20.0d0*dlog10(wor)
          j = j+1
        enddo
      enddo
      call dfftw_destroy_plan(plan2)
c normalization amp diagram to 100 Db with three decade cutting
      ntall = ntime+2
      amax = -1.0d10
      do j = 1,ntall
        do k = 1,nf
          if(amp(j,k).gt.amax) amax = amp(j,k)
        enddo
      enddo
      do j = 1,ntall
        do k = 1,nf
          amp(j,k) = amp(j,k)+100.0d0-amax
          if(amp(j,k).lt.40.0d0) amp(j,k) = 40.0d0
        enddo
      enddo
c construction reference indices table ind. It points to local maxima.
c table ipar contains three parameter for each local maximum:
c tim - group time; tvis - observed period; ampgr - amplitude values in Db
      ici = 0
      do k = 1,nf
c find local maxima on the FTAN amplitude diagram map
        iciflag = 0
        do j =2,ntall-1
          if(amp(j,k).gt.amp(j-1,k).and.amp(j,k).gt.amp(j+1,k)) then
            iciflag = iciflag+1
            ici = ici+1
            ind(1,ici) = k
            ind(2,ici) = j
          endif
        enddo
        if(iciflag.eq.0) then
          ici = ici+1
          ind(1,ici) = k
          ind(2,ici) = ntall-1
          if(ampo(2,k).gt.ampo(ntall-1,k)) ind(2,ici) = 2
          iciflag = 1
        endif
c compute parameters for each maximum
        amax = -1.0d10
        ia = 1
        do j = ici-iciflag+1,ici
          m= ind(2,j)
          call fmax(amp(m-1,k),amp(m,k),amp(m+1,k),pha(m-1,k),
     *             pha(m,k),pha(m+1,k),t,dph,tm,ph)
          ipar(1,j) = (nb+m-1+t)*dt
          ipar(2,j) = -2*pi*dt/dph
          ipar(3,j) = tm
          if(tm.gt.amax) then
            amax = tm
            ia = j
          endif
        enddo
c Compute SNR to noise ratio ------------
        mm = 0
        do j = ici-iciflag+1,ici
          m = ind(2,j)
          mm = mm + 1
c Period has single maximum ------
          if(iciflag.eq.1) then
            ipar(4,j) = 100.0d0+20.0d0*dlog10(ampo(m,k)/
     *                  dsqrt(ampo(1,k)*ampo(ntall,k)))
            goto 10
          endif
c Period has multiple maxima ------
c      compute left minimum -------
          if(mm.ne.1) then
            lm = ampo(m,k)
            m1 = ind(2,j-1)
            do mi = m1,m
              if(ampo(mi,k).le.lm) lm = ampo(mi,k)
            enddo
          endif
c      compute right minimum -------
          if(mm.ne.iciflag) then
            rm = ampo(m,k)
            m1 = ind(2,j+1)
            do mi = m,m1
              if(ampo(mi,k).le.rm) rm = ampo(mi,k)
            enddo
          endif
          if(mm.eq.1) lm = rm
          if(mm.eq.iciflag) rm = lm
          ipar(4,j) = 20.0d0*dlog10(ampo(m,k)/dsqrt(lm*rm))
        enddo
   10   continue
c End of SNR computations
        tim(k)   = ipar(1,ia)
        tvis(k)  = ipar(2,ia)
        ampgr(k) = ipar(3,ia)
        grvel(k) = delta/(tim(k) +t0)
        snr(k) = ipar(4,ia)
      enddo
      nfout1 = nf
c%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
c       Check dispersion curve for jumps
c%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
      call trigger(grvel,om,nf,tresh,trig1, ftrig,ierr)
      if(ierr.ne.0) then
        do k = 1,nf
          grvelt(k) = grvel(k)
          tvist(k)  = tvis(k)
          ampgrt(k) = ampgr(k)
          snrt(k)   = snr(k)
        enddo
        njump = 0
c find all jumps
        nijmp = 0
        do i = 1,nf-1
          if(dabs(trig1(i+1)-trig1(i)).gt.1.5d0) then
            nijmp = nijmp+1
            ijmp(nijmp) = i
          endif
        enddo
        nii = 0
        do i =1,nijmp-1
          if(ijmp(i+1)-ijmp(i).le.npoints) then
             nii = nii +1
             ii(nii) = i
          endif
        enddo
c main loop by jumps
        if(nii.ne.0) then
          do ki = 1,nii
             kk = ii(ki)
             do i = 1,nf
                grvel1(i) = grvelt(i)
                tvis1(i)  = tvist(i)
                ampgr1(i) = ampgrt(i)
                snr1(i)   = snrt(i)
             enddo
             istrt = ijmp(kk)
             ibeg = istrt+1
             iend = ijmp(kk+1)
             ima = 0
             do k = ibeg,iend
               dmaxt = 1.0e10
               do j = 1,ici
                 if(ind(1,j).eq.k) then
                   wor = dabs(delta/ipar(1,j)-grvel1(k-1))
                   if(wor.lt.dmaxt) then
                     ima = j
                     dmaxt = wor
                   endif
                 endif
               enddo
               grvel1(k) = delta/ipar(1,ima);
               tvis1(k)  = ipar(2,ima);
               ampgr1(k) = ipar(3,ima)
               snr1(k)   = ipar(4,ima)
             enddo
             call trigger(grvel1,om,nf,tresh,trig1, ftrig1,ierr1)
             iflag = 0
             do k=istrt,iend+1
                if(dabs(trig1(k)).ge.0.5d0) iflag = 1
             enddo
             if(iflag.eq.0) then
               do i =1,nf
                 grvelt(i) = grvel1(i)
                 tvist(i)  = tvis1(i)
                 ampgrt(i) = ampgr1(i)
                 snrt(i)   = snr1(i)
                 njump = njump+1
               enddo
             endif
          enddo
        endif
        do i=1,nf
          grvel1(i) = grvelt(i)
          tvis1(i)  = tvist(i)
          ampgr1(i) = ampgrt(i)
          snr1(i)   = snrt(i)
        enddo
c%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
c after removing possible jumps, we cut frequency range to single
c segment with max length
c%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        call trigger(grvel1,om,nf,tresh,trig1, ftrig1,ierr1)
        if(ierr1.ne.0) then
          nindx = 1
          indx(1) = 1
          do i =1,nf
            if(dabs(trig1(i)).ge.0.5d0) then
              nindx = nindx+1
              indx(nindx) = i
            endif
          enddo
          nindx = nindx+1
          indx(nindx) = nf
          imax = 0
          ipos = 0
          do i =1,nindx-1
            iimax = indx(i+1)-indx(i)
            if(iimax.gt.imax) then
               ipos = i
               imax = iimax
            endif
          enddo
          ist = max0(indx(ipos),1);
          ibe = min0(indx(ipos+1),nf);
          nfout2 = ibe -ist+1;
          do i = ist,ibe
            per2(i-ist+1)   = per(i)
            grvel1(i-ist+1) = grvel1(i)
            tvis1(i-ist+1)  = tvis1(i)
            ampgr1(i-ist+1) = ampgr1(i)
            snr1(i-ist+1)   = snr1(i)
            om1(i-ist+1)    = om(i)
          enddo
          call trigger(grvel1,om1,nfout2,tresh,trig1, ftrig1,ierr1)
          if(nfout2 .lt. nf*perc/100.0d0) then
            ierr1 = 1
            nfout2 = 0
          endif
        endif
      else
        ierr = 0
        nfout2 = nf
        do i = 1,nf
          per2(i)   = per(i)
          tvis1(i)  = tvis(i)
          ampgr1(i) = ampgr(i)
          grvel1(i) = grvel(i)
          snr1(i)   = snr(i)
        enddo
      endif
      if(nfout2 .ne.0) then
        call lim(nfout2,per2,grvel1,grvel1);
      endif
c%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
c fill out output data arrays
c%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
      do i = 1,nfout1
        arr1(1,i) = per(i)
        arr1(2,i) = tvis(i)
        arr1(3,i) = grvel(i)
        arr1(4,i) = ampgr(i)
        arr1(5,i) = ftrig(i)
        arr1(6,i) = snr(i)
      enddo
      if(nfout2.ne.0) then
        do i = 1,nfout2
          arr2(1,i) = per2(i)
          arr2(2,i) = tvis1(i)
          arr2(3,i) = grvel1(i)
          arr2(4,i) = ampgr1(i)
          arr2(5,i) = snr1(i)
        enddo
      else
        ierr = 2
      endif
      return
      end
