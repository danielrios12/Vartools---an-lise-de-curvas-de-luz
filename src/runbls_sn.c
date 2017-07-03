/*     This file is part of VARTOOLS version 1.31                      */
/*                                                                           */
/*     VARTOOLS is free software: you can redistribute it and/or modify      */
/*     it under the terms of the GNU General Public License as published by  */
/*     the Free Software Foundation, either version 3 of the License, or     */
/*     (at your option) any later version.                                   */
/*                                                                           */
/*     This program is distributed in the hope that it will be useful,       */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         */
/*     GNU General Public License for more details.                          */
/*                                                                           */
/*     You should have received a copy of the GNU General Public License     */
/*     along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/*                                                                           */
/*     Copyright 2007, 2008, 2009  Joel Hartman                              */
/*                                                                           */
/*     This file is part of VARTOOLS version 1.152                      */
/*                                                                           */
/*     VARTOOLS is free software: you can redistribute it and/or modify      */
/*     it under the terms of the GNU General Public License as published by  */
/*     the Free Software Foundation, either version 3 of the License, or     */
/*     (at your option) any later version.                                   */
/*                                                                           */
/*     This program is distributed in the hope that it will be useful,       */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         */
/*     GNU General Public License for more details.                          */
/*                                                                           */
/*     You should have received a copy of the GNU General Public License     */
/*     along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/*                                                                           */
/*     Copyright 2007, 2008, 2009  Joel Hartman                              */
/*                                                                           */
#include "commands.h"
#include "programdata.h"
#include "functions.h"

/* Default parameters */
#define DEFAULT_MINPER 1.0
#define DEFAULT_MAXPER 10.0
#define DEFAULT_NFREQ 10000
#define DEFAULT_NBINS 400
#define DEFAULT_QMIN 0.01
#define DEFAULT_QMAX 0.10
#define DEFAULT_TIMEZONE -7.
#define MAXSTRINGLENGTH 512
#define SRVALSSIZE 400000

#ifdef MAX_
#undef MAX_
#endif
#define MAX_(A,B) ((A) > (B) ? (A) : (B))

#ifdef MIN_
#undef MIN_
#endif
#define MIN_(A,B) ((A) < (B) ? (A) : (B))

#ifdef ABS_
#undef ABS_
#endif
#define ABS_(A) ((A) > 0 ? (A) : (-(A)))

#define CLIP_FACTOR 3.0
#define BIN_FACTOR 100

double getfrac_onenight(int n,double *t,double *u, double *v,double *err,double bper,double depth,double qtran,double bt0,double timezone)
{
  int i, j, k, nnights;
  long night, night0, night1;
  double ph, f0, maxfrac;
  double *chisqrnights, chisqrtot, val;
  f0 = 1./bper;

  /* We have to add 0.5 to go from HJD (where days begin at midnight) to roughly JD (where days begin at noon) */
  timezone = timezone/24.0 + 0.5;
  night0 = (long) (t[0] + timezone);
  night1 = (long) (t[n-1] + timezone);
  nnights = (int) ((night1 - night0) + 1);
  if(nnights < 2) nnights = 2;

  chisqrnights = (double *) malloc(nnights * sizeof(double));
  chisqrtot = 0.;
  for(i=0;i<nnights;i++)
    chisqrnights[i] = 0.;

  for(i=0;i<n;i++)
    {
      ph = (t[i]-bt0)*f0;
      ph -= floor(ph);
      if(ph <= qtran)
	{
	  night = (long) (t[i] + timezone);
	  k = (int) (night - night0);
	  if(k < 0) k = 0;
	  if(k >= nnights) k = nnights - 1;
	  val = v[i]*v[i]/(err[i]*err[i]);
	  chisqrnights[k] += val;
	  chisqrtot += val;
	}
    }
  maxfrac = 0.;
  for(k=0;k<nnights;k++)
    {
      val = chisqrnights[k] / chisqrtot;
      if(val > maxfrac) maxfrac = val;
    }
  free(chisqrnights);
  return(maxfrac);
}


double getclippedsrave(int n, double *sr)
{
  int i, n2;
  double ave1, ave2, ave3;
  ave1 = 0.; ave2 = 0.;
  for(i=0;i<n;i++)
    {
      ave1 += sr[i];
      ave2 += sr[i]*sr[i];
    }
  ave3 = 0.;
  n2 = 0;
  ave1 /= n;
  ave2 = sqrt((double)((ave2 / (double) n) - ave1*ave1));
  for(i=0;i<n;i++)
    {
      if((sr[i] - ave1) < CLIP_FACTOR*ave2)
	{
	  ave3 += sr[i];
	  n2++;
	}
    }
  return (ave3 / (double) n2);
}


double getclippedstddev(int n, double *pow)
{
  int i, n2;
  double ave1, ave2, ave3, ave4;
  ave1 = 0.; ave2 = 0.;
  for(i=0;i<n;i++)
    {
      ave1 += pow[i];
      ave2 += pow[i]*pow[i];
    }
  ave1 /= n;
  ave2 = sqrt((ave2 / n) - (ave1*ave1));
  ave3 = 0.;
  ave4 = 0.;
  n2 = 0;
  for(i=0;i<n;i++)
    {
      if((pow[i] - ave1) < CLIP_FACTOR*ave2)
	{
	  ave3 += pow[i];
	  ave4 += pow[i]*pow[i];
	  n2++;
	}
    }
  ave3 /= n;
  ave4 = sqrt((ave4 / n) - (ave3*ave3));
  return(ave4);
}

void getclippedavestddev(int n, double *pow, double *ave_out, double *stddev_out)
{
  int i, n2;
  double ave1, ave2, ave3, ave4;
  ave1 = 0.; ave2 = 0.;
  for(i=0;i<n;i++)
    {
      ave1 += pow[i];
      ave2 += pow[i]*pow[i];
    }
  ave1 /= n;
  ave2 = sqrt((ave2 / n) - (ave1*ave1));
  ave3 = 0.;
  ave4 = 0.;
  n2 = 0;
  for(i=0;i<n;i++)
    {
      if((pow[i] - ave1) < CLIP_FACTOR*ave2)
	{
	  ave3 += pow[i];
	  ave4 += pow[i]*pow[i];
	  n2++;
	}
    }
  ave3 /= n;
  ave4 = sqrt((ave4 / n) - (ave3*ave3));
  *ave_out = ave3;
  *stddev_out = ave4;
}


double subtract_binnedrms(int N, double *mag, double bintime, double *aveval, int *ngood, double *binmag, double *binsig)
{
  int i, n, jmin, jmax, *ngoodpoints, nclippedlast, nclippedthis;
  double avesum1, avesum2, avesum3, rmsval, v;
  double *sumval1, *sumval2, *sumval3, ave;

  *aveval = -1.;
  if(N > 0)
    {
      if((sumval1 = (double *) malloc(N * sizeof(double))) == NULL ||
	 (sumval2 = (double *) malloc(N * sizeof(double))) == NULL ||
	 (sumval3 = (double *) malloc(N * sizeof(double))) == NULL ||
	 (ngoodpoints = (int *) malloc(N * sizeof(int))) == NULL)
	{
	  fprintf(stderr,"Memory Allocation Error\n");
	  exit(2);
	}

      /* First get the clipped average magnitude and rms*/
      nclippedlast = 0;
      nclippedthis = 0;
      *aveval = 0.;
      rmsval = 1000000.;
      do
	{
	  nclippedlast = nclippedthis;
	  nclippedthis = 0;
	  if(!nclippedlast)
	    {
	      avesum1 = mag[0];
	      avesum2 = mag[0]*mag[0];
	      nclippedthis = 1;
	    }
	  else if(ABS_(mag[0] - *aveval) < CLIP_FACTOR*rmsval)
	    {
	      avesum1 = mag[0];
	      avesum2 = mag[0]*mag[0];
	      nclippedthis = 1;
	    }
	  else
	    {
	      avesum1 = 0.;
	      avesum2 = 0.*0.;
	    }
	  for(i=1;i<N;i++)
	    {
	      if(!nclippedlast || ABS_(mag[i] - *aveval) < CLIP_FACTOR*rmsval)
		{
		  avesum1 += mag[i];
		  avesum2 += mag[i]*mag[i];
		  nclippedthis++;
		}
	    }
	  *aveval = avesum1 / nclippedthis;
	  rmsval = sqrt((avesum2 / nclippedthis) - ((*aveval)*(*aveval)));
	} while (nclippedthis > nclippedlast);

      if(ABS_(mag[0] - *aveval) < CLIP_FACTOR*rmsval)
	{
	  ngoodpoints[0] = 1;
	  sumval1[0] = mag[0];
	  sumval2[0] = mag[0]*mag[0];
	}
      else
	{
	  ngoodpoints[0] = 0;
	  sumval1[0] = 0.;
	  sumval2[0] = 0.;
	}
      for(i=1;i<N;i++)
	{
	  if(ABS_(mag[i] - *aveval) < CLIP_FACTOR*rmsval)
	    {
	      sumval1[i] = sumval1[i-1] + mag[i];
	      sumval2[i] = sumval2[i-1] + mag[i]*mag[i];
	      ngoodpoints[i] = ngoodpoints[i-1] + 1;
	    }
	  else
	    {
	      sumval1[i] = sumval1[i-1];
	      sumval2[i] = sumval2[i-1];
	      ngoodpoints[i] = ngoodpoints[i-1];
	    }
	}

      /* Go through the list find the minimum and maximum times to include via bisection and compute the binned average magnitude and error */
      for(i=0;i<N;i++)
	{
	  jmin = MAX_(0,i - bintime);
	  jmax = MIN_(N-1,i + bintime);
	  if(jmin > 0)
	    {
	      if((v = ngoodpoints[jmax] - ngoodpoints[jmin-1]) > 0)
		{
		  binmag[i] = (sumval1[jmax] - sumval1[jmin-1]) / v;
		  binsig[i] = sqrt((sumval2[jmax] - sumval2[jmin-1]) / v - (binmag[i]*binmag[i]));
		}
	      else
		{
		  binmag[i] = 0.;
		  binsig[i] = 0.;
		}
	    }
	  else
	    {
	      if((v = ngoodpoints[jmax]) > 0)
		{
		  binmag[i] = (sumval1[jmax]) / v;
		  binsig[i] = sqrt((sumval2[jmax])/v - (binmag[i]*binmag[i]));
		}
	      else
		{
		  binmag[i] = 0.;
		  binsig[i] = 0.;
		}
	    }
	}
      avesum1 = 0.;
      avesum2 = 0.;
      avesum3 = 0.;
      n = 0;
      for(i=0;i<N;i++)
	{
	  if(binsig[i] > 0.)
	    {
	      avesum1 += binmag[i];
	      avesum2 += (binmag[i] * binmag[i]);
	      n++;
	    }
	}
      if(n > 0)
	{
	  ave = avesum1 / (double) n;
	  *aveval = ave;
	  *ngood = n;
	  rmsval = sqrt((avesum2 / (double) n) - (ave * ave));
	}
      else
	{
	  *ngood = 0;
	  rmsval = -1.;
	}
      free(sumval1);
      free(sumval2);
      free(sumval3);
      free(ngoodpoints);
    }
  else
    {
      *ngood = 0;
      rmsval = -1.;
    }

  return(rmsval);
}


/* This version of BLS calculates the average magnitude, delta-chi^2 for the best positive and negative dips (as described in Burke et al. 2006), SDE, SR, the transit period, the depth the phases of transit start and end, assuming phase zero occurs at the first observation */

/* C port of the eebls.f routine -------
c
c------------------------------------------------------------------------
c     >>>>>>>>>>>> This routine computes BLS spectrum <<<<<<<<<<<<<<
c
c         [ see Kovacs, Zucker & Mazeh 2002, A&A, Vol. 391, 369 ]
c
c     This is the slightly modified version of the original BLS routine
c     by considering Edge Effect (EE) as suggested by
c     Peter R. McCullough [ pmcc@stsci.edu ].
c
c     This modification was motivated by considering the cases when
c     the low state (the transit event) happened to be devided between
c     the first and last bins. In these rare cases the original BLS
c     yields lower detection efficiency because of the lower number of
c     data points in the bin(s) covering the low state.
c
c     For further comments/tests see  www.konkoly.hu/staff/kovacs.html
c------------------------------------------------------------------------
c
c     Input parameters:
c     ~~~~~~~~~~~~~~~~~
c
c     n    = number of data points
c     t    = array {t(i)}, containing the time values of the time series
c     x    = array {x(i)}, containing the data values of the time series
c     e    = array {e(i)}, containing the uncertainty values of the time series
c     u    = temporal/work/dummy array, must be dimensioned in the
c            calling program in the same way as  {t(i)}
c     v    = the same as  {u(i)}
c     nf   = number of frequency points in which the spectrum is computed
c     fmin = minimum frequency (MUST be > 0)
c     df   = frequency step
c     nb   = number of bins in the folded time series at any test period
c     qmi  = minimum fractional transit length to be tested
c     qma  = maximum fractional transit length to be tested
c
c     Output parameters:
c     ~~~~~~~~~~~~~~~~~~
c
c     p    = array {p(i)}, containing the values of the BLS spectrum
c            at the i-th frequency value -- the frequency values are
c            computed as  f = fmin + (i-1)*df
c     bper = period at the highest peak in the frequency spectrum
c     bpow = value of {p(i)} at the highest peak
c     depth= depth of the transit at   *bper*
c     qtran= fractional transit length  [ T_transit/bper ]
c     in1  = bin index at the start of the transit [ 0 < in1 < nb+1 ]
c     in2  = bin index at the end   of the transit [ 0 < in2 < nb+1 ]
      chisqrplus = delta_chisqr for the best transit like signal
      chisqrminus = delta_chisqr for the best inverse transit like signal

c -- added sde - the signal detection efficiency
c
c
c     Remarks:
c     ~~~~~~~~
c
c     -- *fmin* MUST be greater than  *1/total time span*
c     -- *nb*   MUST be lower than  *nbmax*
c     -- Dimensions of arrays {y(i)} and {ibi(i)} MUST be greater than
c        or equal to  *nbmax*.
c     -- The lowest number of points allowed in a single bin is equal
c        to   MAX(minbin,qmi*N),  where   *qmi*  is the minimum transit
c        length/trial period,   *N*  is the total number of data points,
c        *minbin*  is the preset minimum number of the data points per
c        bin.
c
c========================================================================
c
*/

int eebls(int n, double *t, double *x, double *e, double *u, double *v, int nf, double fmin, double df, int nb, double qmi, double qma, double *p, int Npeak, double *bper, double *bt0, double *bpow, double *sde, double *snval, double *depth, double *qtran, int *in1, int *in2, double *in1_ph, double *in2_ph, double *chisqrplus, double *chisqrminus, double *bperpos, double *meanmagval, double timezone, double *fraconenight, int operiodogram, char *outname, int omodel, char *modelname, int correctlc,int ascii,int *nt, int *Nt, int *Nbefore, int *Nafter, double *rednoise, double *whitenoise, double *sigtopink, int fittrap, double *qingress, double *OOTmag, int ophcurve, char *ophcurvename, double phmin, double phmax, double phstep, int ojdcurve, char *ojdcurvename, double jdstep, int nobinnedrms, int freq_step_type, int adjust_qmin_mindt, int reduce_nb)
{
  double *y;
  double *ibi;
#ifdef PARALLEL
  int firsttime = 0;
  double *srvals = NULL, *srvals_minus = NULL;
#else
  static int firsttime = 0;
  static double *srvals, *srvals_minus;
#endif
  int minbin = 5;
  int nbmax, nbtot;
  int nsrvals, nsrvals_minus, test, foundsofar, dumint1;
  double powerplus, powerminus, bpowminus, dumdbl1, dumdbl2, jdtmp;
  double sumweights, phb1, phb2;
  double tot, rnbtot, *weight, sr_minus;
  double rn, s,t1,f0,p0,ph,ph2,pow,rn1,rn3,s3,rn4,rn5;
  double kkmi, kk, allave, allstddev, allave_minus, allstddev_minus, *qtran_array, *depth_array, minbest;
  double *sr_ave, *binned_sr_ave, *binned_sr_sig;
  int kmi, kma,nb1,nbkma,i,jf,j,k,jn1,jn2,jnb,nb2,nsr,nclippedfreq, *in1_array, *in2_array, *best_id, nbsave;
  double *p_minus, *bper_array, *sr_ave_minus, *binned_sr_ave_minus, *binned_sr_sig_minus, global_best_sr_ave, global_best_sr_stddev;
  double global_best_sr_ave_inv, global_best_sr_stddev_inv;
  double kmisave, kkmisave, kmasave, mindt, qmi_test;
  long double sde_sr_ave, sde_srsqr_ave;
  FILE *outfile, *outfile2;

  nbmax = 2*nb;

  if(firsttime == 0 && !nobinnedrms)
    {
      if((srvals = (double *) malloc(SRVALSSIZE * sizeof(double))) == NULL ||
	 (srvals_minus = (double *) malloc(SRVALSSIZE * sizeof(double))) == NULL)
	error(ERR_MEMALLOC);
      firsttime = 1;
    }

  /***********************************************************/

  if((sr_ave = (double *) malloc(nf * sizeof(double))) == NULL ||
     (y = (double *) malloc(nbmax * sizeof(double))) == NULL ||
     (ibi = (double *) malloc(nbmax * sizeof(double))) == NULL ||
     (binned_sr_ave = (double *) malloc(nf * sizeof(double))) == NULL ||
     (binned_sr_sig = (double *) malloc(nf * sizeof(double))) == NULL ||
     (in1_array = (int *) malloc(nf * sizeof(int))) == NULL ||
     (in2_array = (int *) malloc(nf * sizeof(int))) == NULL ||
     (qtran_array = (double *) malloc(nf * sizeof(double))) == NULL ||
     (depth_array = (double *) malloc(nf * sizeof(double))) == NULL ||
     (bper_array = (double *) malloc(nf * sizeof(double))) == NULL ||
     (sr_ave_minus = (double *) malloc(nf * sizeof(double))) == NULL ||
     (binned_sr_ave_minus = (double *) malloc(nf * sizeof(double))) == NULL ||
     (binned_sr_sig_minus = (double *) malloc(nf * sizeof(double))) == NULL ||
     (p_minus = (double *) malloc(nf * sizeof(double))) == NULL ||
     (best_id = (int *) malloc(Npeak * sizeof(int))) == NULL)

    {
      fprintf(stderr,"Memory Allocation Error\n");
      exit(3);
    }

  if(nb > nbmax) {
    error(ERR_BLSNBMAX);
  }
  tot = t[n-1] - t[0];
  if(fmin < 1./tot) {
    error(ERR_BLSFMINTOOSMALL);
  }

  /**********************************************************/

  sumweights = 0.;
  weight = (double *) malloc(n * sizeof(double));
  for(i=0;i<n;i++)
    {
      weight[i] = 1./(e[i]*e[i]);
      sumweights += weight[i];
    }
  for(i=0;i<n;i++)
    weight[i] = weight[i] / sumweights;

  rn = (double) n;
  kmi = (int) (qmi*(double)nb);
  if(kmi < 1) kmi = 1;
  kma = ((int) (qma*(double)nb)) + 1;
  kkmi = qmi;
  if(kkmi < (double) minbin / rn) kkmi = (double) minbin / rn;
  //*bpow = 0.;

  if(adjust_qmin_mindt) {
    kmisave = kmi;
    kkmisave = kkmi;
    kmasave = kma;
    mindt = 0;
    if(n > 1) {mindt = t[1] - t[0];}
    for(i=2; i < n; i++) {
      if(t[i]-t[i-1] < mindt) mindt = t[i] - t[i-1];
    }
    if(reduce_nb) {
      nbsave = nb;
    }
  }
    

  bpowminus = 0.;
  sde_sr_ave = 0.;
  sde_srsqr_ave = 0.;

  /**************The following variables are defined for the extension
		 c     of arrays  ibi()  and  y()  [ see below ] ***************/

  nb1 = nb;
  nbkma = nb+kma;

  /*
    c
    c=================================
    c     Set temporal time series
    c=================================
    c
  */

  //sr_ave = 0.;
  //srsqr_ave = 0.;
  nsr = 0;

  s = 0.;
  t1 = t[0];
  for(i=0;i<n;i++)
    {
      u[i]=t[i]-t1;
      s += x[i]*weight[i];
    }
  (*meanmagval) = s;
  //s /= sumweights;
  for(i=0;i<n;i++)
    v[i]=x[i]-s;

  /*
    c
    c******************************
    c     Start period search - we modify this slightly to first compute
the periodogram, and then search it for peaks    *
    c******************************
    c
  */

  for(jf=0;jf<nf;jf++)
    {
      if(!freq_step_type) {
	f0=fmin+df*((double)jf);
	p0=1./f0;
      } else if(freq_step_type == VARTOOLS_FREQSTEPTYPE_PERIOD) {
	p0 = (1./fmin) - df*((double)jf);
	f0 = 1./p0;
      } else if(freq_step_type == VARTOOLS_FREQSTEPTYPE_LOGPERIOD) {
	f0 = exp(log(fmin) + df*((double)jf));
	p0=1./f0;
      }

      if(adjust_qmin_mindt) {
	qmi_test = mindt*f0;
	if(qmi_test > 1) qmi_test = 1.0;
	if(qmi_test > qmi) {
	  if(reduce_nb) {
	    nb = MIN_(nbsave, ceil(1./(0.5*qmi_test)));
	  }
	  kmi = (int) (qmi_test*(double)nb);
	  if(kmi < 1) kmi = 1;
	  if(qmi_test > qma)
	    kma = ((int) (qmi_test*(double)nb)) + 1;
	  else if(reduce_nb)
	    kma = ((int) (qma*(double)nb)) + 1;
	  nb1 = nb;
	  nbkma = nb+kma;
	  kkmi = qmi;
	  if(kkmi < (double) minbin / rn) kkmi = (double) minbin / rn;
	} else {
	  kmi = kmisave; kkmi = kkmisave;
	  kma = kmasave;
	  if(reduce_nb)
	    nb = nbsave;
	  nb1 = nb;
	  nbkma = nb+kma;
	}
      }


      /*
	c
	c======================================================
	c     Compute folded time series with  *p0*  period
	c======================================================
	c
      */


      for(j=0;j<nb;j++)
	{
	  y[j] = 0.;
	  ibi[j] = 0.;
	}
      nbtot = 0;
      for(i=0;i<n;i++)
	{
	  ph = u[i]*f0;
	  ph -= (int) ph;
	  j = (int) (nb*ph);
	  ibi[j] += weight[i];
	  nbtot++;
	  y[j] += v[i]*weight[i];
	}
      /*      for(i=0;i<nb;i++)
        y[i] = ibi[i] * y[i] / rn;
      */
      /*
	c
	c-----------------------------------------------
	c     Extend the arrays  ibi()  and  y() beyond
	c     nb   by  wrapping
	c
      */

      for(j=nb1;j<nbkma;j++)
	{
	  jnb = j - nb;
	  ibi[j] = ibi[jnb];
	  nbtot += ibi[j];
	  y[j] = y[jnb];
	}
      rnbtot = (double) nbtot;
      /*
	c-----------------------------------------------
	c
	c===============================================
	c     Compute BLS statistics for this period
	c===============================================
	c
      */

      powerplus = 0.;
      powerminus = 0.;
      nsrvals = 0;
      nsrvals_minus = 0;
      for(i=0;i<nb;i++)
	{
	  s = 0.;
	  k = 0;
	  kk = 0.;
	  nb2 = i+kma;
	  for(j=i;j<nb2;j++)
	    {
	      k++;
	      kk += ibi[j];
	      s += y[j];
	      if(k >= kmi && kk >= kkmi)
		{
		  rn1 = (double) kk;
		  rn4 = (double) k;
		  pow = s*s/(rn1*(1. - rn1));
		  if(s > 0.)
		    {
		      if(!nobinnedrms) {
			srvals[nsrvals] = sqrt(pow);
			nsrvals++;
		      }
		      if(pow >= powerplus)
			{
			  powerplus = pow;
			  jn1 = i;
			  jn2 = j;
			  rn3 = rn1;
			  rn5 = rn4;
			  s3 = s;
			}
		    }
		  else if(s < 0.)
		    {
		      if(!nobinnedrms) {
			srvals_minus[nsrvals_minus] = sqrt(pow);
			nsrvals_minus++;
		      }
		      if(pow >= powerminus)
			{
			  powerminus = pow;
			}
		    }
		}
	    }
	}
      // Find the average value of the srvals
      if(!nobinnedrms) {
	sr_ave[jf] = getclippedsrave(nsrvals,srvals);
	sr_ave_minus[jf] = getclippedsrave(nsrvals_minus,srvals_minus);
      }
      powerplus = sqrt(powerplus);
      sde_sr_ave += powerplus;
      sde_srsqr_ave += powerplus*powerplus;
      p[jf] = powerplus;
      powerminus = sqrt(powerminus);
      p_minus[jf] = powerminus;
      //sr_ave += powerplus;
      //srsqr_ave += powerplus*powerplus;
      nsr++;
      in1_array[jf] = jn1;
      in2_array[jf] = jn2;
      qtran_array[jf] = (double)(jn2 - jn1 + 1)/(double)nb;
      depth_array[jf] = powerplus/sqrt(rn3*(1.-rn3));
      bper_array[jf] = p0;
      /*      if(powerplus >= *bpow)
	{
	  *bpow = powerplus;
	  *in1 = jn1;
	  *in2 = jn2;
	  *qtran = (double)(jn2 - jn1 + 1)/(double)nb;
	  *depth = powerplus/sqrt(rn3*(1.-rn3));
	  *bper = p0;
	}
      if(powerminus >= bpowminus)
	{
	  bpowminus = powerminus;
	  *bperpos = p0;
	  }*/
    }
  if(!nobinnedrms) {
    allstddev = subtract_binnedrms(nf, sr_ave, BIN_FACTOR, &allave, &nclippedfreq, binned_sr_ave, binned_sr_sig);
  }
  else {
    getclippedavestddev(nf,p,&global_best_sr_ave,&global_best_sr_stddev);
    nclippedfreq = nf;
  }

  /* Now let's find the peaks in the periodogram, first convert the periodogram from SR to SN ratio */

  if(nclippedfreq > Npeak)
    {
      if(!nobinnedrms) {
	for(i=0;i<nf;i++)
	  {
	    if(binned_sr_ave[i] > 0.)
	      {
		p[i] = (p[i] - binned_sr_ave[i]) / allstddev;
	      /*	      if(p[i] > *bpow)
	        {
		  *bpow = p[i];
		  sr_plus = p[i]*allstddev + binned_sr_ave[i];
		  *in1 = in1_array[i];
		  *in2 = in2_array[i];
		  *qtran = qtran_array[i];
		  *depth = depth_array[i];
		  *bper = bper_array[i];
		  }*/
	      }
	    else
	      p[i] = 0.;
	  }
      }
      else {
	for(i=0; i<nf; i++) {
	  p[i] = (p[i] - global_best_sr_ave)/global_best_sr_stddev;
	}
      }
    }
  else
    {
      /* We have no peaks, just put -1. for the values and return to the calling function */
      for(j=0;j<Npeak;j++)
	{
	  bper[j] = -1.;
          bt0[j] = -1.;
	  snval[j] = -1.;
	  bpow[j] = -1.;
	  in1[j] = -1;
	  in2[j] = -1;
	  in1_ph[j] = -1.;
	  in2_ph[j] = -1.;
	  qtran[j] = -1.;
	  depth[j] = -1.;
	  sde[j] = -1.;
	  chisqrplus[j] = 999999.;
      	  fraconenight[j] = -1.;
	}
      *bperpos = -1.;
      *chisqrminus = 999999.;
      *meanmagval = -1.;

      free(weight);
      free(y);
      free(ibi);
      free(best_id);
      free(sr_ave);
      free(binned_sr_ave);
      free(binned_sr_sig);
      free(in1_array);
      free(in2_array);
      free(qtran_array);
      free(depth_array);
      free(bper_array);
      free(sr_ave_minus);
      free(binned_sr_ave_minus);
      free(binned_sr_sig_minus);
      free(p_minus);
#ifdef PARALLEL
      if(srvals != NULL) free(srvals);
      if(srvals_minus != NULL) free(srvals_minus);
#endif
      return 1;
    }

  foundsofar = 0;
  i = 0;
  while(foundsofar < Npeak && i < nf)
    {
      if(p[i] > 0)
	{
	  test = 1;
	  for(j=0;j<foundsofar;j++)
	    {
	      if(!isDifferentPeriods(MIN_(bper[j],bper_array[i]),MAX_(bper[j],bper_array[i]),tot))
		{
		  if(p[i] > snval[j])
		    {
		      bper[j] = bper_array[i];
		      snval[j] = p[i];
		      best_id[j] = i;
		    }
		  test = 0;
		  break;
		}
	    }
	  if(test)
	    {
	      snval[foundsofar] = p[i];
	      bper[foundsofar] = bper_array[i];
	      best_id[foundsofar] = i;
	      foundsofar++;
	    }
	}
      i++;
    }

  if(i < nf)
    {
      mysort3_int(Npeak,snval,bper,best_id);
      minbest = snval[0];
      for(;i<nf;i++)
	{
	  if(p[i] > minbest)
	    {
	      test = 1;
	      for(j=0;j<Npeak;j++)
		{
		  if(!isDifferentPeriods(MIN_(bper[j],bper_array[i]),MAX_(bper[j],bper_array[i]),tot))
		    {
		      if(p[i] > snval[j])
			{
			  snval[j] = p[i];
			  bper[j] = bper_array[i];
			  best_id[j] = i;
			  mysort3_int(Npeak,snval,bper,best_id);
			  minbest = snval[0];
			}
		      test = 0;
		      break;
		    }
		}
	      if(test)
		{
		  snval[0] = p[i];
		  bper[0] = bper_array[i];
		  best_id[0] = i;
		  mysort3_int(Npeak,snval,bper,best_id);
		  minbest = snval[0];
		}
	    }
	}
    }
  else if(foundsofar >= 1)
    {
      /* We have a few peaks, but Npeak of them */
      mysort3_int(foundsofar,snval,bper,best_id);
      for(j=foundsofar;j<Npeak;j++)
	{
	  /* Put -1 for the remaining peaks */
	  bper[j] = -1.;
	  snval[j] = -1.;
	  bpow[j] = -1.;
	  bt0[j] = -1.;
	  in1[j] = -1;
	  in2[j] = -1;
	  in1_ph[j] = -1.;
	  in2_ph[j] = -1.;
	  qtran[j] = -1.;
	  depth[j] = -1.;
	  sde[j] = -1.;
	  chisqrplus[j] = 999999.;
      	  fraconenight[j] = -1.;
	}
    }
  else
    {
      /* We have no peaks, just put -1. for the values and return to the calling function */
      for(j=0;j<Npeak;j++)
	{
	  bper[j] = -1.;
	  snval[j] = -1.;
	  bpow[j] = -1.;
	  bt0[j] = -1.;
	  in1[j] = -1;
	  in2[j] = -1;
	  in1_ph[j] = -1.;
	  in2_ph[j] = -1.;
	  qtran[j] = -1.;
	  depth[j] = -1.;
	  sde[j] = -1.;
	  chisqrplus[j] = 999999.;
      	  fraconenight[j] = -1.;
	}
      *bperpos = -1.;
      *chisqrminus = 999999.;
      *meanmagval = -1.;
      free(weight);
      free(y);
      free(ibi);
      free(best_id);
      free(sr_ave);
      free(binned_sr_ave);
      free(binned_sr_sig);
      free(in1_array);
      free(in2_array);
      free(qtran_array);
      free(depth_array);
      free(bper_array);
      free(sr_ave_minus);
      free(binned_sr_ave_minus);
      free(binned_sr_sig_minus);
      free(p_minus);
#ifdef PARALLEL
      if(srvals != NULL) free(srvals);
      if(srvals_minus != NULL) free(srvals_minus);
#endif
      return 1;
    }
  //fprintf(stderr,"Error Running BLS - no frequencies survive clipping!\n");

  /* invert the snval, bper and best_id vectors */
  for(i = 0, j = foundsofar - 1; i < foundsofar/2 + 1; i++)
    {
      if(i < j)
	{
	  dumdbl1 = snval[j];
	  dumdbl2 = bper[j];
	  dumint1 = best_id[j];
	  snval[j] = snval[i];
	  bper[j] = bper[i];
	  best_id[j] = best_id[i];
	  snval[i] = dumdbl1;
	  bper[i] = dumdbl2;
	  best_id[i] = dumint1;
	}
      j--;
    }

  /* Collect all the output bls parameters for the peaks */
  for(i=0;i<Npeak;i++)
    {
      if(bper[i] > -1)
	{
	  if(!nobinnedrms)
	    bpow[i] = snval[i]*allstddev + binned_sr_ave[best_id[i]];
	  else
	    bpow[i] = snval[i]*global_best_sr_stddev + global_best_sr_ave;
	  if(adjust_qmin_mindt && reduce_nb) {
	    qmi_test = mindt/bper[i];
	    if(qmi_test > 1.0) qmi_test = 1.0;
	    if(qmi_test > qmi) {
	      nb = MIN_(nbsave, ceil(1./(0.5*qmi_test)));
	    } else {
	      nb = nbsave;
	    }
	    if(nb != nbsave) {
	      in1[i] = rint(((double) nbsave*in1_array[best_id[i]])/(double) nb);
	      in2[i] = rint(((double) nbsave*in2_array[best_id[i]])/(double) nb);
	    }
	    else {
	      in1[i] = in1_array[best_id[i]];
	      in2[i] = in2_array[best_id[i]];
	    }
	  } else {
	    in1[i] = in1_array[best_id[i]];
	    in2[i] = in2_array[best_id[i]];
	  }
	  in1_ph[i] = ((double) in1_array[best_id[i]]) / ((double) nb);
	  in2_ph[i] = ((double) in2_array[best_id[i]]) / ((double) nb);
	  if(fittrap) {
	    qingress[i]=0.25;
	    OOTmag[i]=*meanmagval;
	    dofittrap_amoeba(n, t, x, e, bper[i], &(qtran_array[best_id[i]]), &(qingress[i]), &(in1_ph[i]), &(in2_ph[i]), &(depth_array[best_id[i]]), &(OOTmag[i]));
	  } else {
	    qingress[i] = 0.;
	    OOTmag[i] = *meanmagval;
	  }
	  // Be sure to correct for transits past the edge
	  if(in2[i] >= nb) in2[i] = in2[i] - nb;
	  qtran[i] = qtran_array[best_id[i]];
	  bt0[i] = t[0] + (0.5*qtran[i]+in1_ph[i])*bper[i];
	  depth[i] = depth_array[best_id[i]];
	  sde[i] = (bpow[i] - ((double)sde_sr_ave / (double)nsr))/sqrt((double)((sde_srsqr_ave / (long double) nsr) - (sde_sr_ave*sde_sr_ave/((long double)nsr*(long double)nsr))));
	  chisqrplus[i] = -bpow[i]*bpow[i]*sumweights;

	  fraconenight[i] = getfrac_onenight(n, t, u, v, e, bper[i], depth[i], qtran[i], (t[0] + in1_ph[i]*bper[i]), timezone);
	  /* Get the signal to pink noise for the peak */
	  getsignaltopinknoiseforgivenblsmodel(n, t, x, e, bper[i], qtran[i], depth[i], in1_ph[i], &nt[i], &Nt[i], &Nbefore[i], &Nafter[i], &rednoise[i], &whitenoise[i], &sigtopink[i], qingress[i], OOTmag[i]);
	}
    }

  /* Now find the maximum inverse transit */
  if(!nobinnedrms)
    allstddev_minus = subtract_binnedrms(nf, sr_ave_minus, BIN_FACTOR, &allave_minus, &nclippedfreq, binned_sr_ave_minus, binned_sr_sig_minus);
  else {
    getclippedavestddev(nf,p_minus,&global_best_sr_ave_inv,&global_best_sr_stddev_inv);
    nclippedfreq = nf;
  }

  if(nclippedfreq > 0.)
    {
      if(!nobinnedrms) {
	for(i=0;i<nf;i++)
	  {
	    if(binned_sr_ave[i] > 0.)
	      {
		p_minus[i] = (p_minus[i] - binned_sr_ave_minus[i]) / allstddev_minus;
		if(p_minus[i] > bpowminus)
		  {
		    bpowminus = p_minus[i];
		    sr_minus = p_minus[i]*allstddev_minus + binned_sr_ave_minus[i];
		    *bperpos = bper_array[i];
		    *chisqrminus = -sr_minus*sr_minus*sumweights;
		  }
	      }
	    else
	      p_minus[i] = 0.;
	  }
      }
      else {
	for(i=0;i<nf;i++)
	  {
	    p_minus[i] = (p_minus[i] - global_best_sr_ave_inv) / global_best_sr_stddev_inv;
	    if(p_minus[i] > bpowminus)
	      {
		bpowminus = p_minus[i];
		sr_minus = p_minus[i]*global_best_sr_stddev_inv + global_best_sr_ave_inv;
		*bperpos = bper_array[i];
		*chisqrminus = -sr_minus*sr_minus*sumweights;
	      }
	  }
      }
    }
  else
    {
      /* We have no peaks, just put -1. for the values and return to the calling function */
      /*for(j=0;j<Npeak;j++)
	{
	  bper[j] = -1.;
	  snval[j] = -1.;
	  bpow[j] = -1.;
	  in1[j] = -1;
	  in2[j] = -1;
	  qtran[j] = -1.;
	  depth[j] = -1.;
	  sde[j] = -1.;
	  chisqrplus[j] = 999999.;
      	  fraconenight[j] = -1.;
	  }*/
      *bperpos = -1.;
      *chisqrminus = 999999.;
      *meanmagval = -1.;
      free(weight);
      free(y);
      free(ibi);
      free(best_id);
      free(sr_ave);
      free(binned_sr_ave);
      free(binned_sr_sig);
      free(in1_array);
      free(in2_array);
      free(qtran_array);
      free(depth_array);
      free(bper_array);
      free(sr_ave_minus);
      free(binned_sr_ave_minus);
      free(binned_sr_sig_minus);
      free(p_minus);
#ifdef PARALLEL
      if(srvals != NULL) free(srvals);
      if(srvals_minus != NULL) free(srvals_minus);
#endif
      return 1;
    }

  //sde = (*bpow - ((double)sr_ave / (double)nsr))/sqrt((double)((srsqr_ave / (long double) nsr) - (sr_ave*sr_ave/(long double)(nsr*nsr))));

  /*
    c
    c     Edge correction of transit end index
    c
  */

  //output the periodogram if asked to
  if(operiodogram)
    {
      if((outfile = fopen(outname,"w")) == NULL)
	error2(ERR_CANNOTWRITE,outname);
      if(ascii)
	{
	  fprintf(outfile,"#Period  S/N   SR\n");
	  if(!nobinnedrms) {
	    for(i=0;i<nf;i++) {
	      fprintf(outfile,"%.17g %.17g %.17g\n",bper_array[i],p[i],(p[i]*allstddev+binned_sr_ave[i]));
	    }
	  }
	  else {
	    for(i=0;i<nf;i++) {
	      fprintf(outfile,"%.17g %.17g %.17g\n",bper_array[i],p[i],(p[i]*global_best_sr_stddev + global_best_sr_ave));
	    }
	  }
	}
      else
	{
	  fwrite(&nf,4,1,outfile);
	  fwrite(bper_array,8,nf,outfile);
	  fwrite(p,8,nf,outfile);
	}
      fclose(outfile);
    }

  //output the model light curve if asked to
  if(omodel)
    {
      if((outfile2 = fopen(modelname,"w")) == NULL)
	error2(ERR_CANNOTWRITE,modelname);

      f0 = 1./bper[0];
      phb1 = qingress[0]*qtran[0];
      phb2 = qtran[0] - phb1;

      fprintf(outfile2,"#Time  Mag_obs   Mag_model   Error   Phase\n");
      for(i=0;i<n;i++)
	{
	  ph = (u[i] - in1_ph[0]*bper[0])*f0;
	  ph -= floor(ph);
	  if(ph >= qtran[0]) {
	    ph2 = ph - 0.5*qtran[0];
	    if(ph2 < 0)
	      ph2 += 1.;
	    fprintf(outfile2,"%f %f %f %f %f\n",t[i], x[i], OOTmag[0], e[i], ph2);
	  }
	  else {
	    if(ph >= phb1 && ph <= phb2) {
	      ph2 = ph - 0.5*qtran[0];
	      if(ph2 < 0)
		ph2 += 1.;
	      fprintf(outfile2,"%f %f %f %f %f\n", t[i], x[i], OOTmag[0]+depth[0], e[i], ph2);
	    }
	    else if(ph < phb1) {
	      ph2 = ph - 0.5*qtran[0];
	      if(ph2 < 0)
		ph2 += 1.;
	      fprintf(outfile2,"%f %f %f %f %f\n", t[i], x[i], OOTmag[0]+depth[0]*ph/phb1, e[i], ph2);
	    }
	    else {
	      ph2 = ph - 0.5*qtran[0];
	      if(ph2 < 0)
		ph2 += 1.;
	      fprintf(outfile2,"%f %f %f %f %f\n", t[i], x[i], OOTmag[0]+depth[0]*(qtran[0] - ph)/phb1, e[i], ph2);
	    }
	  }
	}
      fclose(outfile2);
    }
  // Output the phase curve if asked to.
  if(ophcurve)
    {
      if((outfile2 = fopen(ophcurvename,"w")) == NULL)
	error2(ERR_CANNOTWRITE,ophcurvename);

      fprintf(outfile2,"#Phase Mag_model\n");
      ph2 = phmin;
      phb1 = qingress[0]*qtran[0];
      phb2 = qtran[0] - phb1;
      while(ph2 <= phmax) {
	ph = ph2 + 0.5*qtran[0];
	ph -= floor(ph);
	if(ph >= qtran[0]) {
	  fprintf(outfile2,"%f %f\n",ph2,OOTmag[0]);
	}
	else {
	  if(ph >= phb1 && ph <= phb2) {
	    fprintf(outfile2,"%f %f\n",ph2,OOTmag[0]+depth[0]);
	  }
	  else if(ph < phb1) {
	    fprintf(outfile2,"%f %f\n",ph2,OOTmag[0]+depth[0]*ph/phb1);
	  }
	  else {
	    fprintf(outfile2,"%f %f\n",ph2,OOTmag[0]+depth[0]*(qtran[0] - ph)/phb1);
	  }
	}
	ph2 += phstep;
      }
      fclose(outfile2);
    }

  // Output the JD curve if asked to.
  if(ojdcurve)
    {
      if((outfile2 = fopen(ojdcurvename,"w")) == NULL)
	error2(ERR_CANNOTWRITE,ojdcurvename);

      fprintf(outfile2,"#Time Mag_model Phase\n");
      jdtmp = t[0];
      f0 = 1./bper[0];
      phb1 = qingress[0]*qtran[0];
      phb2 = qtran[0] - phb1;
      while(jdtmp <= t[n-1])
	{
	  ph = (jdtmp - t[0] - in1_ph[0]*bper[0])*f0;
	  ph -= floor(ph);
	  if(ph >= qtran[0]) {
	    ph2 = ph - 0.5*qtran[0];
	    if(ph2 < 0)
	      ph2 += 1.;
	    fprintf(outfile2,"%f %f %f\n", jdtmp, OOTmag[0], ph2);
	  }
	  else {
	    if(ph >= phb1 && ph <= phb2) {
	      ph2 = ph - 0.5*qtran[0];
	      if(ph2 < 0)
		ph2 += 1.;
	      fprintf(outfile2,"%f %f %f\n", jdtmp, OOTmag[0]+depth[0], ph2);
	    }
	    else if(ph < phb1) {
	      ph2 = ph - 0.5*qtran[0];
	      if(ph2 < 0)
		ph2 += 1.;
	      fprintf(outfile2,"%f %f %f\n", jdtmp, OOTmag[0]+depth[0]*ph/phb1, ph2);
	    }
	    else {
	      ph2 = ph - 0.5*qtran[0];
	      if(ph2 < 0)
		ph2 += 1.;
	      fprintf(outfile2,"%f %f %f\n", jdtmp, OOTmag[0]+depth[0]*(qtran[0] - ph)/phb1, ph2);
	    }
	  }
	  jdtmp += jdstep;
	}
      fclose(outfile2);
    }
  if(correctlc)
    {
      f0 = 1./bper[0];
      phb1 = qingress[0]*qtran[0];
      phb2 = qtran[0] - phb1;
      for(i=0;i<n;i++)
	{
	  ph = (u[i] - in1_ph[0]*bper[0])*f0;
	  ph -= floor(ph);
	  if(ph < qtran[0]) {
	    if(ph >= phb1 && ph <= phb2) {
	      x[i] -= depth[0];
	    }
	    else if(ph < phb1) {
	      x[i] -= depth[0]*ph/phb1;
	    }
	    else {
	      x[i] -= depth[0]*(qtran[0] - ph)/phb1;
	    }
	  }
	}

    }

  free(weight);
  free(y);
  free(ibi);
  free(best_id);
  free(sr_ave);
  free(binned_sr_ave);
  free(binned_sr_sig);
  free(in1_array);
  free(in2_array);
  free(qtran_array);
  free(depth_array);
  free(bper_array);
  free(sr_ave_minus);
  free(binned_sr_ave_minus);
  free(binned_sr_sig_minus);
  free(p_minus);

#ifdef PARALLEL
  if(srvals != NULL) free(srvals);
  if(srvals_minus != NULL) free(srvals_minus);
#endif
  return(0);
}

/* This version adjusts the qmin and qmax according the period using a specified rmin and rmax, it assumes that for P in days and R in solar radii that q is given by:
q = 0.076 * R**(2/3) / P**(2/3)
*/

int eebls_rad(int n, double *t, double *x, double *e, double *u, double *v, int nf, double fmin, double df, int nb, double rmin, double rmax, double *p, int Npeak, double *bper, double *bt0, double *bpow, double *sde, double *snval, double *depth, double *qtran, int *in1, int *in2, double *in1_ph, double *in2_ph, double *chisqrplus, double *chisqrminus, double *bperpos, double *meanmagval, double timezone, double *fraconenight, int operiodogram, char *outname, int omodel, char *modelname, int correctlc, int ascii,int *nt, int *Nt, int *Nbefore, int *Nafter, double *rednoise, double *whitenoise, double *sigtopink, int fittrap, double *qingress, double *OOTmag, int ophcurve, char *ophcurvename, double phmin, double phmax, double phstep, int ojdcurve, char *ojdcurvename, double jdstep, int nobinnedrms, int freq_step_type, int adjust_qmin_mindt, int reduce_nb)
{
  double *y;
  double *ibi;
#ifdef PARALLEL
  int firsttime =0;
  double *srvals = NULL;
  double *srvals_minus = NULL;
#else
  static int firsttime=0;
  static double *srvals;
  static double *srvals_minus;
#endif
  int minbin = 5;
  int nbmax, nbtot;
  int nsrvals, nsrvals_minus, test, foundsofar, dumint1;
  double powerplus, powerminus, bpowminus, dumdbl1, dumdbl2, jdtmp;
  double sumweights, qminP, qmaxP, Ppow,rminpow,rmaxpow;
  double tot, rnbtot, sr_minus, *weight;
  double rn, s,t1,f0,p0,ph,ph2,phb1,phb2,testpow,rn1,rn3,s3,rn4,rn5;
  double kkmi, kk, allave, allstddev, allave_minus, allstddev_minus, *qtran_array, *depth_array, minbest;
  double *sr_ave, *binned_sr_ave, *binned_sr_sig;
  int kmi, kma,nb1,nbkma,i,jf,j,k,jn1,jn2,jnb,nb2,nsr,nclippedfreq, *in1_array, *in2_array, *best_id, nbsave;
  double *p_minus, *bper_array, *sr_ave_minus, *binned_sr_ave_minus, *binned_sr_sig_minus;
  long double sde_sr_ave, sde_srsqr_ave;
  FILE *outfile, *outfile2;
  double global_best_sr_ave, global_best_sr_stddev, global_best_sr_ave_inv, global_best_sr_stddev_inv, qmi_test, mindt;

  if(firsttime == 0 && !nobinnedrms)
    {
      if((srvals = (double *) malloc(SRVALSSIZE * sizeof(double))) == NULL ||
	 (srvals_minus = (double *) malloc(SRVALSSIZE * sizeof(double))) == NULL)
	error(ERR_MEMALLOC);
      firsttime = 1;
    }

  rminpow = pow(rmin,0.6666667);
  rmaxpow = pow(rmax,0.6666667);
  nbmax = 2*nb;

  /***********************************************************/

  if((sr_ave = (double *) malloc(nf * sizeof(double))) == NULL ||
     (y = (double *) malloc(nbmax * sizeof(double))) == NULL ||
     (ibi = (double *) malloc(nbmax * sizeof(double))) == NULL ||
     (binned_sr_ave = (double *) malloc(nf * sizeof(double))) == NULL ||
     (binned_sr_sig = (double *) malloc(nf * sizeof(double))) == NULL ||
     (in1_array = (int *) malloc(nf * sizeof(int))) == NULL ||
     (in2_array = (int *) malloc(nf * sizeof(int))) == NULL ||
     (qtran_array = (double *) malloc(nf * sizeof(double))) == NULL ||
     (depth_array = (double *) malloc(nf * sizeof(double))) == NULL ||
     (bper_array = (double *) malloc(nf * sizeof(double))) == NULL ||
     (sr_ave_minus = (double *) malloc(nf * sizeof(double))) == NULL ||
     (binned_sr_ave_minus = (double *) malloc(nf * sizeof(double))) == NULL ||
     (binned_sr_sig_minus = (double *) malloc(nf * sizeof(double))) == NULL ||
     (p_minus = (double *) malloc(nf * sizeof(double))) == NULL ||
     (best_id = (int *) malloc(Npeak * sizeof(int))) == NULL)

    {
      fprintf(stderr,"Memory Allocation Error\n");
      exit(3);
    }

  if(nb >= nbmax) {
    error(ERR_BLSNBMAX);
  }
  if(nb < 2) nb = 2;
  tot = t[n-1] - t[0];
  if(fmin < 1./tot) {
    error(ERR_BLSFMINTOOSMALL);
  }

  /**********************************************************/

  sumweights = 0.;
  weight = (double *) malloc(n * sizeof(double));
  for(i=0;i<n;i++)
    {
      weight[i] = 1./(e[i]*e[i]);
      sumweights += weight[i];
    }
  for(i=0;i<n;i++)
    weight[i] = weight[i] / sumweights;

  rn = (double) n;
  //*bpow = 0.;

  bpowminus = 0.;
  sde_sr_ave = 0.;
  sde_srsqr_ave = 0.;

  /**************The following variables are defined for the extension
		 c     of arrays  ibi()  and  y()  [ see below ] ***************/

  if(adjust_qmin_mindt) {
    mindt = 0;
    if(n > 1) {mindt = t[1] - t[0];}
    for(i=2; i < n; i++) {
      if(t[i]-t[i-1] < mindt) mindt = t[i] - t[i-1];
    }
  }

  nbsave = nb;
  nb1 = nb;

  /*
    c
    c=================================
    c     Set temporal time series
    c=================================
    c
  */

  //sr_ave = 0.;
  //srsqr_ave = 0.;
  nsr = 0;

  s = 0.;

  /* Get the minimum time */
  t1 = t[0];
  for(i=0;i<n;i++)
    {
      u[i]=t[i]-t1;
      s += x[i]*weight[i];
    }
  (*meanmagval) = s;
  //s /= sumweights;
  for(i=0;i<n;i++)
    v[i]=x[i]-s;

  /*
    c
    c******************************
    c     Start period search - we modify this slightly to first compute
the periodogram, and then search it for peaks    *
    c******************************
    c
  */

  for(jf=0;jf<nf;jf++)
    {
      if(!freq_step_type) {
	f0=fmin+df*((double)jf);
	p0=1./f0;
      } else if(freq_step_type == VARTOOLS_FREQSTEPTYPE_PERIOD) {
	p0 = (1./fmin) - df*((double)jf);
	f0 = 1./p0;
      } else if(freq_step_type == VARTOOLS_FREQSTEPTYPE_LOGPERIOD) {
	f0 = exp(log(fmin) + df*((double)jf));
	p0=1./f0;
      }

      /*
	c
	c======================================================
	c     Compute folded time series with  *p0*  period
	c======================================================
	c
      */

      Ppow = pow(p0,0.6666667);
      qminP = 0.076*rminpow/Ppow;
      qmaxP = 0.076*rmaxpow/Ppow;

      if(qminP > 1.0) qminP = 1.0;
      if(qmaxP > 1.0) qmaxP = 1.0;

      if(adjust_qmin_mindt) {
	qmi_test = mindt*f0;
	if(qmi_test > qminP) {
	  qminP = qmi_test;
	  if(qmi_test > qmaxP)
	    qmaxP = qmi_test;
	}
	if(reduce_nb) {
	  nb = MIN_(nbsave, ceil(1./(0.5*qminP)));
	  if(nb < 2) nb=2;
	  nb1 = nb;
	}
      }

      kmi = (int) (qminP*(double)nb);
      if(kmi < 1) kmi = 1;
      if(kmi > nb-1) kmi = nb-1;
      kma = ((int) (qmaxP*(double)nb)) + 1;
      if(kma > nb-1) kma = nb-1;
      kkmi = qminP;
      if(kkmi < (double) minbin / rn) kkmi = (double) minbin / rn;
      nbkma = nb+kma;


      for(j=0;j<nb;j++)
	{
	  y[j] = 0.;
	  ibi[j] = 0.;
	}
      nbtot = 0;
      for(i=0;i<n;i++)
	{
	  ph = u[i]*f0;
	  ph -= (int) ph;
	  j = (int) (nb*ph);
	  ibi[j] += weight[i];
	  nbtot++;
	  y[j] += v[i]*weight[i];
	}
      /*      for(i=0;i<nb;i++)
        y[i] = ibi[i] * y[i] / rn;
      */
      /*
	c
	c-----------------------------------------------
	c     Extend the arrays  ibi()  and  y() beyond
	c     nb   by  wrapping
	c
      */


      for(j=nb1;j<nbkma;j++)
	{
	  jnb = j - nb;
	  ibi[j] = ibi[jnb];
	  nbtot += ibi[j];
	  y[j] = y[jnb];
	}
      rnbtot = (double) nbtot;
      /*
	c-----------------------------------------------
	c
	c===============================================
	c     Compute BLS statistics for this period
	c===============================================
	c
      */

      powerplus = 0.;
      powerminus = 0.;
      nsrvals = 0;
      nsrvals_minus = 0;
      for(i=0;i<nb;i++)
	{
	  s = 0.;
	  k = 0;
	  kk = 0.;
	  nb2 = i+kma;
	  for(j=i;j<nb2;j++)
	    {
	      k++;
	      kk += ibi[j];
	      s += y[j];
	      if(k >= kmi && kk >= kkmi)
		{
		  rn1 = (double) kk;
		  rn4 = (double) k;
		  testpow = s*s/(rn1*(1. - rn1));
		  if(s > 0.)
		    {
		      if(!nobinnedrms) {
			srvals[nsrvals] = sqrt(testpow);
			nsrvals++;
		      }
		      if(testpow >= powerplus)
			{
			  powerplus = testpow;
			  jn1 = i;
			  jn2 = j;
			  rn3 = rn1;
			  rn5 = rn4;
			  s3 = s;
			}
		    }
		  else if(s < 0.)
		    {
		      if(!nobinnedrms) {
			srvals_minus[nsrvals_minus] = sqrt(testpow);
			nsrvals_minus++;
		      }
		      if(testpow >= powerminus)
			{
			  powerminus = testpow;
			}
		    }
		}
	    }
	}
      // Find the average value of the srvals
      if(!nobinnedrms) {
	sr_ave[jf] = getclippedsrave(nsrvals,srvals);
	sr_ave_minus[jf] = getclippedsrave(nsrvals_minus,srvals_minus);
      }
      powerplus = sqrt(powerplus);
      sde_sr_ave += powerplus;
      sde_srsqr_ave += powerplus*powerplus;
      p[jf] = powerplus;
      powerminus = sqrt(powerminus);
      p_minus[jf] = powerminus;
      //sr_ave += powerplus;
      //srsqr_ave += powerplus*powerplus;
      nsr++;
      in1_array[jf] = jn1;
      in2_array[jf] = jn2;
      qtran_array[jf] = (double)(jn2 - jn1 + 1)/(double)nb;
      depth_array[jf] = powerplus/sqrt(rn3*(1.-rn3));
      bper_array[jf] = p0;
      /*      if(powerplus >= *bpow)
	{
	  *bpow = powerplus;
	  *in1 = jn1;
	  *in2 = jn2;
	  *qtran = (double)(jn2 - jn1 + 1)/(double)nb;
	  *depth = powerplus/sqrt(rn3*(1.-rn3));
	  *bper = p0;
	}
      if(powerminus >= bpowminus)
	{
	  bpowminus = powerminus;
	  *bperpos = p0;
	  }*/
    }
  if(!nobinnedrms)
    allstddev = subtract_binnedrms(nf, sr_ave, BIN_FACTOR, &allave, &nclippedfreq, binned_sr_ave, binned_sr_sig);
  else {
    getclippedavestddev(nf,p,&global_best_sr_ave,&global_best_sr_stddev);
    nclippedfreq = nf;
  }

  /* Now let's find the peaks in the periodogram, first convert the periodogram from SR to SN ratio */

  if(nclippedfreq > Npeak)
    {
      if(!nobinnedrms) {
	for(i=0;i<nf;i++)
	  {
	    if(binned_sr_ave[i] > 0.)
	      {
		p[i] = (p[i] - binned_sr_ave[i]) / allstddev;
		/*	      if(p[i] > *bpow)
			      {
			      *bpow = p[i];
			      sr_plus = p[i]*allstddev + binned_sr_ave[i];
			      *in1 = in1_array[i];
			      *in2 = in2_array[i];
			      *qtran = qtran_array[i];
			      *depth = depth_array[i];
			      *bper = bper_array[i];
			      }*/
	      }
	    else
	      p[i] = 0.;
	  }
      }
      else {
	for(i=0; i<nf; i++) {
	  p[i] = (p[i] - global_best_sr_ave)/global_best_sr_stddev;
	}
      }

    }
  else
    {
      /* We have no peaks, just put -1. for the values and return to the calling function */
      for(j=0;j<Npeak;j++)
	{
	  bper[j] = -1.;
	  snval[j] = -1.;
	  bpow[j] = -1.;
	  bt0[j] = -1.;
	  in1[j] = -1;
	  in2[j] = -1;
	  in1_ph[j] = -1.;
	  in2_ph[j] = -1.;
	  qtran[j] = -1.;
	  depth[j] = -1.;
	  sde[j] = -1.;
	  chisqrplus[j] = 999999.;
      	  fraconenight[j] = -1.;
	}
      *bperpos = -1.;
      *chisqrminus = 999999.;
      *meanmagval = -1.;

      free(weight);
      free(y);
      free(ibi);
      free(best_id);
      free(sr_ave);
      free(binned_sr_ave);
      free(binned_sr_sig);
      free(in1_array);
      free(in2_array);
      free(qtran_array);
      free(depth_array);
      free(bper_array);
      free(sr_ave_minus);
      free(binned_sr_ave_minus);
      free(binned_sr_sig_minus);
      free(p_minus);

#ifdef PARALLEL
      if(srvals != NULL) free(srvals);
      if(srvals_minus != NULL) free(srvals_minus);
#endif
      return 1;
    }

  foundsofar = 0;
  i = 0;
  while(foundsofar < Npeak && i < nf)
    {
      if(p[i] > 0)
	{
	  test = 1;
	  for(j=0;j<foundsofar;j++)
	    {
	      if(!isDifferentPeriods(MIN_(bper[j],bper_array[i]),MAX_(bper[j],bper_array[i]),tot))
		{
		  if(p[i] > snval[j])
		    {
		      bper[j] = bper_array[i];
		      snval[j] = p[i];
		      best_id[j] = i;
		    }
		  test = 0;
		  break;
		}
	    }
	  if(test)
	    {
	      snval[foundsofar] = p[i];
	      bper[foundsofar] = bper_array[i];
	      best_id[foundsofar] = i;
	      foundsofar++;
	    }
	}
      i++;
    }

  if(i < nf)
    {
      mysort3_int(Npeak,snval,bper,best_id);
      minbest = snval[0];
      for(;i<nf;i++)
	{
	  if(p[i] > minbest)
	    {
	      test = 1;
	      for(j=0;j<Npeak;j++)
		{
		  if(!isDifferentPeriods(MIN_(bper[j],bper_array[i]),MAX_(bper[j],bper_array[i]),tot))
		    {
		      if(p[i] > snval[j])
			{
			  snval[j] = p[i];
			  bper[j] = bper_array[i];
			  best_id[j] = i;
			  mysort3_int(Npeak,snval,bper,best_id);
			  minbest = snval[0];
			}
		      test = 0;
		      break;
		    }
		}
	      if(test)
		{
		  snval[0] = p[i];
		  bper[0] = bper_array[i];
		  best_id[0] = i;
		  mysort3_int(Npeak,snval,bper,best_id);
		  minbest = snval[0];
		}
	    }
	}
    }
  else if(foundsofar >= 1)
    {
      /* We have a few peaks, but not Npeak of them */
      mysort3_int(foundsofar,snval,bper,best_id);
      for(j=foundsofar;j<Npeak;j++)
	{
	  /* Put -1 for the remaining peaks */
	  bper[j] = -1.;
	  snval[j] = -1.;
	  bpow[j] = -1.;
	  bt0[j] = -1.;
	  in1[j] = -1;
	  in2[j] = -1;
	  in1_ph[j] = -1.;
	  in2_ph[j] = -1.;
	  qtran[j] = -1.;
	  depth[j] = -1.;
	  sde[j] = -1.;
	  chisqrplus[j] = 999999.;
      	  fraconenight[j] = -1.;
	}
    }
  else
    {
      /* We have no peaks, just put -1. for the values and return to the calling function */
      for(j=0;j<Npeak;j++)
	{
	  bper[j] = -1.;
	  snval[j] = -1.;
	  bpow[j] = -1.;
	  bt0[j] = -1.;
	  in1[j] = -1;
	  in2[j] = -1;
	  in1_ph[j] = -1.;
	  in2_ph[j] = -1.;
	  qtran[j] = -1.;
	  depth[j] = -1.;
	  sde[j] = -1.;
	  chisqrplus[j] = 999999.;
      	  fraconenight[j] = -1.;
	}
      *bperpos = -1.;
      *chisqrminus = 999999.;
      *meanmagval = -1.;
      free(weight);
      free(y);
      free(ibi);
      free(best_id);
      free(sr_ave);
      free(binned_sr_ave);
      free(binned_sr_sig);
      free(in1_array);
      free(in2_array);
      free(qtran_array);
      free(depth_array);
      free(bper_array);
      free(sr_ave_minus);
      free(binned_sr_ave_minus);
      free(binned_sr_sig_minus);
      free(p_minus);
#ifdef PARALLEL
      if(srvals != NULL) free(srvals);
      if(srvals_minus != NULL) free(srvals_minus);
#endif
      return 1;
    }
  //fprintf(stderr,"Error Running BLS - no frequencies survive clipping!\n");

  /* invert the snval, bper and best_id vectors */
  for(i = 0, j = foundsofar - 1; i < foundsofar/2 + 1; i++)
    {
      if(i < j)
	{
	  dumdbl1 = snval[j];
	  dumdbl2 = bper[j];
	  dumint1 = best_id[j];
	  snval[j] = snval[i];
	  bper[j] = bper[i];
	  best_id[j] = best_id[i];
	  snval[i] = dumdbl1;
	  bper[i] = dumdbl2;
	  best_id[i] = dumint1;
	}
      j--;
    }

  /* Collect all the output bls parameters for the peaks */
  for(i=0;i<Npeak;i++)
    {
      if(bper[i] > -1)
	{
	  if(!nobinnedrms)
	    bpow[i] = snval[i]*allstddev + binned_sr_ave[best_id[i]];
	  else
	    bpow[i] = snval[i]*global_best_sr_stddev + global_best_sr_ave;

	  if(adjust_qmin_mindt && reduce_nb) {
	    Ppow = pow(bper[i],0.6666667);
	    qminP = 0.076*rminpow/Ppow;
	    if(qminP > 1.0) qminP = 1.0;
	    qmi_test = mindt/bper[i];
	    if(qmi_test > qminP) {
	      qminP = qmi_test;
	    }
	    nb = MIN_(nbsave, ceil(1./(0.5*qminP)));
	    if(nb != nbsave) {
	      in1[i] = rint(((double) nbsave*in1_array[best_id[i]])/(double) nb);
	      in2[i] = rint(((double) nbsave*in2_array[best_id[i]])/(double) nb);
	    }
	    else {
	      in1[i] = in1_array[best_id[i]];
	      in2[i] = in2_array[best_id[i]];
	    }
	  } else {
	    in1[i] = in1_array[best_id[i]];
	    in2[i] = in2_array[best_id[i]];
	  }
	  in1_ph[i] = ((double) in1_array[best_id[i]]) / ((double) nb);
	  in2_ph[i] = ((double) in2_array[best_id[i]]) / ((double) nb);
	  if(fittrap) {
	    qingress[i]=0.25;
	    OOTmag[i]=*meanmagval;
	    dofittrap_amoeba(n, t, x, e, bper[i], &(qtran_array[best_id[i]]), &(qingress[i]), &(in1_ph[i]), &(in2_ph[i]), &(depth_array[best_id[i]]), &(OOTmag[i]));
	  } else {
	    qingress[i] = 0.;
	    OOTmag[i] = *meanmagval;
	  }
	  // Be sure to correct for transits past the edge
	  if(in2[i] >= nb) in2[i] = in2[i] - nb;
	  qtran[i] = qtran_array[best_id[i]];
	  bt0[i] = t[0] + (0.5*qtran[i]+in1_ph[i])*bper[i];
	  depth[i] = depth_array[best_id[i]];
	  sde[i] = (bpow[i] - ((double)sde_sr_ave / (double)nsr))/sqrt((double)((sde_srsqr_ave / (long double) nsr) - (sde_sr_ave*sde_sr_ave/((long double)nsr*(long double)nsr))));
	  chisqrplus[i] = -bpow[i]*bpow[i]*sumweights;

	  fraconenight[i] = getfrac_onenight(n, t, u, v, e, bper[i], depth[i], qtran[i], (t[0] +in1_ph[i]*bper[i]), timezone);
	  /* Get the signal to pink noise for the peak */
	  getsignaltopinknoiseforgivenblsmodel(n, t, x, e, bper[i], qtran[i], depth[i], in1_ph[i], &nt[i], &Nt[i], &Nbefore[i], &Nafter[i], &rednoise[i], &whitenoise[i], &sigtopink[i], qingress[i], OOTmag[i]);
	}

    }

  /* Now find the maximum inverse transit */
  if(!nobinnedrms)
    allstddev_minus = subtract_binnedrms(nf, sr_ave_minus, BIN_FACTOR, &allave_minus, &nclippedfreq, binned_sr_ave_minus, binned_sr_sig_minus);
  else {
    getclippedavestddev(nf,p_minus,&global_best_sr_ave_inv,&global_best_sr_stddev_inv);
    nclippedfreq = nf;
  }
  if(nclippedfreq > 0.)
    {
      if(!nobinnedrms) {
	for(i=0;i<nf;i++)
	  {
	    if(binned_sr_ave[i] > 0.)
	      {
		p_minus[i] = (p_minus[i] - binned_sr_ave_minus[i]) / allstddev_minus;
		if(p_minus[i] > bpowminus)
		  {
		    bpowminus = p_minus[i];
		    sr_minus = p_minus[i]*allstddev_minus + binned_sr_ave_minus[i];
		    *bperpos = bper_array[i];
		    *chisqrminus = -sr_minus*sr_minus*sumweights;
		  }
	      }
	    else
	      p_minus[i] = 0.;
	  }
      }
      else {
	for(i=0;i<nf;i++)
	  {
	    p_minus[i] = (p_minus[i] - global_best_sr_ave_inv) / global_best_sr_stddev_inv;
	    if(p_minus[i] > bpowminus)
	      {
		bpowminus = p_minus[i];
		sr_minus = p_minus[i]*global_best_sr_stddev_inv + global_best_sr_ave_inv;
		*bperpos = bper_array[i];
		*chisqrminus = -sr_minus*sr_minus*sumweights;
	      }
	  }
      }
    }
  else
    {
      /* We have no peaks, just put -1. for the values and return to the calling function */
      /*for(j=0;j<Npeak;j++)
	{
	  bper[j] = -1.;
	  snval[j] = -1.;
	  bpow[j] = -1.;
	  in1[j] = -1;
	  in2[j] = -1;
	  qtran[j] = -1.;
	  depth[j] = -1.;
	  sde[j] = -1.;
	  chisqrplus[j] = 999999.;
      	  fraconenight[j] = -1.;
	  }*/
      *bperpos = -1.;
      *chisqrminus = 999999.;
      *meanmagval = -1.;
      free(weight);
      free(y);
      free(ibi);
      free(best_id);
      free(sr_ave);
      free(binned_sr_ave);
      free(binned_sr_sig);
      free(in1_array);
      free(in2_array);
      free(qtran_array);
      free(depth_array);
      free(bper_array);
      free(sr_ave_minus);
      free(binned_sr_ave_minus);
      free(binned_sr_sig_minus);
      free(p_minus);
#ifdef PARALLEL
      if(srvals != NULL) free(srvals);
      if(srvals_minus != NULL) free(srvals_minus);
#endif
      return 1;
    }


  // *sde = (*bpow - ((double)sr_ave / (double)nsr))/sqrt((double)((srsqr_ave / (long double) nsr) - (sr_ave*sr_ave/(long double)(nsr*nsr))));

  /*
    c
    c     Edge correction of transit end index
    c
  */

  //output the periodogram if asked to
  if(operiodogram)
    {
      if((outfile = fopen(outname,"w")) == NULL)
	error2(ERR_CANNOTWRITE,outname);

      if(ascii)
	{
	  fprintf(outfile,"#Period  S/N   SR\n");
	  if(!nobinnedrms) {
	    for(i=0;i<nf;i++) {
	      fprintf(outfile,"%.17g %.17g %.17g\n",bper_array[i],p[i],(p[i]*allstddev+binned_sr_ave[i]));
	    }
	  }
	  else {
	    for(i=0;i<nf;i++) {
	      fprintf(outfile,"%.17g %.17g %.17g\n",bper_array[i],p[i],(p[i]*global_best_sr_stddev + global_best_sr_ave));
	    }
	  }
	}
      else
	{
	  fwrite(&nf,4,1,outfile);
	  fwrite(bper_array,8,nf,outfile);
	  fwrite(p,8,nf,outfile);
	}
      fclose(outfile);
    }

  //output the model light curve if asked to
  if(omodel)
    {
      if((outfile2 = fopen(modelname,"w")) == NULL)
	error2(ERR_CANNOTWRITE,modelname);

      f0 = 1./bper[0];
      phb1 = qingress[0]*qtran[0];
      phb2 = qtran[0] - phb1;

      fprintf(outfile2,"#Time  Mag_obs   Mag_model   Error   Phase\n");
      for(i=0;i<n;i++)
	{
	  ph = (u[i] - in1_ph[0]*bper[0])*f0;
	  ph -= floor(ph);
	  if(ph >= qtran[0]) {
	    ph2 = ph - 0.5*qtran[0];
	    if(ph2 < 0)
	      ph2 += 1.;
	    fprintf(outfile2,"%f %f %f %f %f\n",t[i], x[i], OOTmag[0], e[i], ph2);
	  }
	  else {
	    if(ph >= phb1 && ph <= phb2) {
	      ph2 = ph - 0.5*qtran[0];
	      if(ph2 < 0)
		ph2 += 1.;
	      fprintf(outfile2,"%f %f %f %f %f\n", t[i], x[i], OOTmag[0]+depth[0], e[i], ph2);
	    }
	    else if(ph < phb1) {
	      ph2 = ph - 0.5*qtran[0];
	      if(ph2 < 0)
		ph2 += 1.;
	      fprintf(outfile2,"%f %f %f %f %f\n", t[i], x[i], OOTmag[0]+depth[0]*ph/phb1, e[i], ph2);
	    }
	    else {
	      ph2 = ph - 0.5*qtran[0];
	      if(ph2 < 0)
		ph2 += 1.;
	      fprintf(outfile2,"%f %f %f %f %f\n", t[i], x[i], OOTmag[0]+depth[0]*(qtran[0] - ph)/phb1, e[i], ph2);
	    }
	  }
	}
      fclose(outfile2);
    }
  // Output the phase curve if asked to.
  if(ophcurve)
    {
      if((outfile2 = fopen(ophcurvename,"w")) == NULL)
	error2(ERR_CANNOTWRITE,ophcurvename);

      fprintf(outfile2,"#Phase Mag_model\n");
      ph2 = phmin;
      phb1 = qingress[0]*qtran[0];
      phb2 = qtran[0] - phb1;
      while(ph2 <= phmax) {
	ph = ph2 + 0.5*qtran[0];
	ph -= floor(ph);
	if(ph >= qtran[0]) {
	  fprintf(outfile2,"%f %f\n",ph2,OOTmag[0]);
	}
	else {
	  if(ph >= phb1 && ph <= phb2) {
	    fprintf(outfile2,"%f %f\n",ph2,OOTmag[0]+depth[0]);
	  }
	  else if(ph < phb1) {
	    fprintf(outfile2,"%f %f\n",ph2,OOTmag[0]+depth[0]*ph/phb1);
	  }
	  else {
	    fprintf(outfile2,"%f %f\n",ph2,OOTmag[0]+depth[0]*(qtran[0] - ph)/phb1);
	  }
	}
	ph2 += phstep;
      }
      fclose(outfile2);
    }

  // Output the JD curve if asked to.
  if(ojdcurve)
    {
      if((outfile2 = fopen(ojdcurvename,"w")) == NULL)
	error2(ERR_CANNOTWRITE,ojdcurvename);

      fprintf(outfile2,"#Time Mag_model Phase\n");
      jdtmp = t[0];
      f0 = 1./bper[0];
      phb1 = qingress[0]*qtran[0];
      phb2 = qtran[0] - phb1;
      while(jdtmp <= t[n-1])
	{
	  ph = (jdtmp - t[0] - in1_ph[0]*bper[0])*f0;
	  ph -= floor(ph);
	  if(ph >= qtran[0]) {
	    ph2 = ph - 0.5*qtran[0];
	    if(ph2 < 0)
	      ph2 += 1.;
	    fprintf(outfile2,"%f %f %f\n", jdtmp, OOTmag[0], ph2);
	  }
	  else {
	    if(ph >= phb1 && ph <= phb2) {
	      ph2 = ph - 0.5*qtran[0];
	      if(ph2 < 0)
		ph2 += 1.;
	      fprintf(outfile2,"%f %f %f\n", jdtmp, OOTmag[0]+depth[0], ph2);
	    }
	    else if(ph < phb1) {
	      ph2 = ph - 0.5*qtran[0];
	      if(ph2 < 0)
		ph2 += 1.;
	      fprintf(outfile2,"%f %f %f\n", jdtmp, OOTmag[0]+depth[0]*ph/phb1, ph2);
	    }
	    else {
	      ph2 = ph - 0.5*qtran[0];
	      if(ph2 < 0)
		ph2 += 1.;
	      fprintf(outfile2,"%f %f %f\n", jdtmp, OOTmag[0]+depth[0]*(qtran[0] - ph)/phb1, ph2);
	    }
	  }
	  jdtmp += jdstep;
	}
      fclose(outfile2);
    }
  if(correctlc)
    {
      f0 = 1./bper[0];
      phb1 = qingress[0]*qtran[0];
      phb2 = qtran[0] - phb1;
      for(i=0;i<n;i++)
	{
	  ph = (u[i] - in1_ph[0]*bper[0])*f0;
	  ph -= floor(ph);
	  if(ph < qtran[0]) {
	    if(ph >= phb1 && ph <= phb2) {
	      x[i] -= depth[0];
	    }
	    else if(ph < phb1) {
	      x[i] -= depth[0]*ph/phb1;
	    }
	    else {
	      x[i] -= depth[0]*(qtran[0] - ph)/phb1;
	    }
	  }
	}
    }

  free(weight);
  free(y);
  free(ibi);
  free(best_id);
  free(sr_ave);
  free(binned_sr_ave);
  free(binned_sr_sig);
  free(in1_array);
  free(in2_array);
  free(qtran_array);
  free(depth_array);
  free(bper_array);
  free(sr_ave_minus);
  free(binned_sr_ave_minus);
  free(binned_sr_sig_minus);
  free(p_minus);

#ifdef PARALLEL
  if(srvals != NULL) free(srvals);
  if(srvals_minus != NULL) free(srvals_minus);
#endif
  return(0);
}
