/*
***************************************************************************
*
* Author: Teunis van Beelen
*
* Copyright (C) 2018 - 2019 Teunis van Beelen
*
* Email: teuniz@gmail.com
*
***************************************************************************
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
***************************************************************************
*/


#include "fft_wrap.h"


static void window_func(const double *, double *, double *, int, int, int);
static void set_gain_unity(double *, int);


struct fft_wrap_settings_struct * fft_wrap_create(double *buf, int buf_size, int dft_size, int window_type)
{
  struct fft_wrap_settings_struct *st;

  if(buf == NULL)  return NULL;
  if(buf_size < 4)  return NULL;
  if(dft_size < 4)  return NULL;
  if(dft_size & 1)  dft_size--;
  if((window_type < 0) || (window_type > 7))  return NULL;

  st = (struct fft_wrap_settings_struct *)calloc(1, sizeof(struct fft_wrap_settings_struct));
  if(st == NULL)  return NULL;
  st->sz_in = buf_size;
  st->dft_sz = dft_size;
  st->wndw_type = window_type;

  st->blocks = 1;

  if(st->dft_sz < st->sz_in)
  {
    st->blocks = st->sz_in / st->dft_sz;
  }
  else
  {
    st->dft_sz = st->sz_in;
  }

  if(st->dft_sz & 1)  st->dft_sz--;

  st->smpls_left = st->sz_in % st->dft_sz;
  if(st->smpls_left & 1)  st->smpls_left--;

  st->sz_out = st->dft_sz / 2;
  st->buf_in = buf;
  if(st->wndw_type)
  {
    st->buf_wndw = (double *)malloc(sizeof(double) * (st->dft_sz + 2));
    if(st->buf_wndw == NULL)
    {
      free(st);
      return NULL;
    }
    st->buf_wndw_coef = (double *)malloc(sizeof(double) * (st->dft_sz / 2 + 2));
    if(st->buf_wndw_coef == NULL)
    {
      free(st->buf_wndw);
      free(st);
      return NULL;
    }
  }
  st->buf_out = (double *)malloc(sizeof(double) * (st->sz_out + 2));
  if(st->buf_out == NULL)
  {
    free(st->buf_wndw_coef);
    free(st->buf_wndw);
    free(st);
    return NULL;
  }
  st->kiss_fftbuf = (kiss_fft_cpx *)malloc((st->sz_out + 1) * sizeof(kiss_fft_cpx));
  if(st->kiss_fftbuf == NULL)
  {
    free(st->buf_out);
    free(st->buf_wndw_coef);
    free(st->buf_wndw);
    free(st);
    return NULL;
  }
  st->cfg = kiss_fftr_alloc(st->dft_sz, 0, NULL, NULL);

  return st;
}


void fft_wrap_run(struct fft_wrap_settings_struct *st)
{
  int i, j;

  if(st == NULL)  return;
  if(st->sz_in < 4)  return;
  if(st->dft_sz < 4) return;
  if(st->sz_out < 1) return;
  if(st->buf_in == NULL)  return;
  if(st->buf_out == NULL)  return;
  if(st->kiss_fftbuf == NULL)  return;

  if(st->wndw_type)
  {
    if(st->buf_wndw == NULL)  return;

    if(st->wndw_type)
    {
      window_func(st->buf_in, st->buf_wndw, st->buf_wndw_coef, st->dft_sz, st->wndw_type, 0);
    }

    kiss_fftr(st->cfg, st->buf_wndw, st->kiss_fftbuf);
  }
  else
  {
    kiss_fftr(st->cfg, st->buf_in, st->kiss_fftbuf);
  }

  for(i=0; i<st->sz_out; i++)
  {
    st->buf_out[i] = ((st->kiss_fftbuf[i].r * st->kiss_fftbuf[i].r) + (st->kiss_fftbuf[i].i * st->kiss_fftbuf[i].i)) / st->sz_out;
  }

  for(j=1; j<st->blocks; j++)
  {
    if(st->wndw_type)
    {
      window_func(st->buf_in + (j * st->dft_sz), st->buf_wndw, st->buf_wndw_coef, st->dft_sz, st->wndw_type, j);

      kiss_fftr(st->cfg, st->buf_wndw, st->kiss_fftbuf);
    }
    else
    {
      kiss_fftr(st->cfg, st->buf_in + (j * st->dft_sz), st->kiss_fftbuf);
    }

    for(i=0; i<st->sz_out; i++)
    {
      st->buf_out[i] += ((st->kiss_fftbuf[i].r * st->kiss_fftbuf[i].r) + (st->kiss_fftbuf[i].i * st->kiss_fftbuf[i].i)) / st->sz_out;
    }
  }

  if(st->smpls_left)
  {
    if(st->wndw_type)
    {
      window_func(st->buf_in + ((j-1) * st->dft_sz) + st->smpls_left, st->buf_wndw, st->buf_wndw_coef, st->dft_sz, st->wndw_type, j);

      kiss_fftr(st->cfg, st->buf_wndw, st->kiss_fftbuf);
    }
    else
    {
      kiss_fftr(st->cfg, st->buf_in + ((j-1) * st->dft_sz) + st->smpls_left, st->kiss_fftbuf);
    }

    for(i=0; i<st->sz_out; i++)
    {
      st->buf_out[i] += ((st->kiss_fftbuf[i].r * st->kiss_fftbuf[i].r) + (st->kiss_fftbuf[i].i * st->kiss_fftbuf[i].i)) / st->sz_out;

      st->buf_out[i] /= (st->blocks + 1);
    }
  }
  else
  {
    if(st->blocks > 1)
    {
      for(i=0; i<st->sz_out; i++)
      {
        st->buf_out[i] /= st->blocks;
      }
    }
  }
}


void free_fft_wrap(struct fft_wrap_settings_struct *st)
{
  if(st == NULL)  return;
  free(st->cfg);
  free(st->kiss_fftbuf);
  free(st->buf_out);
  free(st->buf_wndw);
  free(st->buf_wndw_coef);
  memset(st, 0, sizeof(struct fft_wrap_settings_struct));
  free(st);
}


static void window_func(const double *src, double *dest, double *coef, int sz, int type, int block)
{
  int i, sz2;

  sz2 = sz / 2;

  if(!block)
  {
    if(type == FFT_WNDW_TYPE_HAMMING)
    {
      for(i=0; i<sz2; i++)
      {  /* Spectrum and spectral density estimation by the Discrete Fourier transform (DFT), including a comprehensive list of window functions and some new at-top windows Max Planck Institute */
        coef[i] = 0.54 - (0.46 * cos((2.0 * M_PI * i) / (sz - 1)));  /* Hamming (original) */
//        coef[i] = 0.53836 - (0.46164 * cos((2.0 * M_PI * i) / (sz - 1)));  /* Hamming marginally optimized */
      }
    }
    else if(type == FFT_WNDW_TYPE_NUTTALL3B)
      {  /* Spectrum and spectral density estimation by the Discrete Fourier transform (DFT), including a comprehensive list of window functions and some new at-top windows Max Planck Institute */
        for(i=0; i<sz2; i++)
        {
//          coef[i] = 0.42 - (0.5 * cos((2.0 * M_PI * i) / (sz - 1))) + (0.08 * cos((4.0 * M_PI * i) / (sz - 1)));  /* Blackman */
          coef[i] = 0.4243801 - (0.4973406 * cos((2.0 * M_PI * i) / (sz - 1))) + (0.0782793 * cos((4.0 * M_PI * i) / (sz - 1)));  /* Nuttall3b */
        }
      }
      else if(type == FFT_WNDW_TYPE_4TERM_BLACKMANHARRIS)
        {  /* Spectrum and spectral density estimation by the Discrete Fourier transform (DFT), including a comprehensive list of window functions and some new at-top windows Max Planck Institute */
          for(i=0; i<sz2; i++)
          {
            coef[i] = 0.35875 - (0.48829 * cos((2.0 * M_PI * i) / (sz - 1))) + (0.14128 * cos((4.0 * M_PI * i) / (sz - 1))) - (0.01168 * cos((6.0 * M_PI * i) / (sz - 1)));  /* 4-term Blackman-Harris */
          }
        }
        else if(type == FFT_WNDW_TYPE_7TERM_BLACKMANHARRIS)
          {  /* The use of DFT windows in signal-to-noise ratio and harmonic distortion computations IEEE */
            for(i=0; i<sz2; i++)
            {
              coef[i] = 0.271051400693424 - (0.433297939234485 * cos((2.0 * M_PI * i) / (sz - 1))) + (0.218122999543110 * cos((4.0 * M_PI * i) / (sz - 1))) - (0.65925446388031e-1 * cos((6.0 * M_PI * i) / (sz - 1)))
              + (0.10811742098371e-1 * cos((8.0 * M_PI * i) / (sz - 1))) - (0.776584825226e-3 * cos((10.0 * M_PI * i) / (sz - 1))) + (0.13887217352e-4 * cos((12.0 * M_PI * i) / (sz - 1)));  /* 7-term Blackman-Harris */
            }
          }
          else if(type == FFT_WNDW_TYPE_NUTTALL4C)
            {
              for(i=0; i<sz2; i++)
              {  /* Spectrum and spectral density estimation by the Discrete Fourier transform (DFT), including a comprehensive list of window functions and some new at-top windows Max Planck Institute */
                coef[i] =  0.3635819 - (0.4891775 * cos((2.0 * M_PI * i) / (sz - 1))) + ( 0.1365995 * cos((4.0 * M_PI * i) / (sz - 1))) - (0.0106411 * cos((6.0 * M_PI * i) / (sz - 1)));  /* Nuttall4c */
              }
            }
            else if(type == FFT_WNDW_TYPE_HANN)
              {  /* Spectrum and spectral density estimation by the Discrete Fourier transform (DFT), including a comprehensive list of window functions and some new at-top windows Max Planck Institute */
                for(i=0; i<sz2; i++)
                {                           /* both are the same */
                  coef[i] = (1.0 - cos((2.0 * M_PI * i) / (sz - 1))) / 2.0;  /* Hann */
//                  coef[i] = 0.5 - (0.5 * cos((2.0 * M_PI * i) / (sz - 1)));  /* Hann */
                }
              }
              else if(type == FFT_WNDW_TYPE_HFT223D)
                {  /* Spectrum and spectral density estimation by the Discrete Fourier transform (DFT), including a comprehensive list of window functions and some new at-top windows Max Planck Institute */
                  for(i=0; i<sz2; i++)
                  {
                    coef[i] = 1.0 - (1.98298997309 * cos((2.0 * M_PI * i) / (sz - 1))) + (1.75556083063 * cos((4.0 * M_PI * i) / (sz - 1))) - (1.19037717712 * cos((6.0 * M_PI * i) / (sz - 1)))
                    + (0.56155440797 * cos((8.0 * M_PI * i) / (sz - 1))) - (0.17296769663 * cos((10.0 * M_PI * i) / (sz - 1))) + (0.3233247087e-1 * cos((12.0 * M_PI * i) / (sz - 1)))
                    - (0.324954578e-2 * cos((14.0 * M_PI * i) / (sz - 1))) + (0.13801040e-3 * cos((16.0 * M_PI * i) / (sz - 1))) - (0.132725e-5 * cos((18.0 * M_PI * i) / (sz - 1)));  /* 9-term HFT223D */
                  }
                }
                else
                {
                  for(i=0; i<sz2; i++)
                  {
                    coef[i] = 0;
                  }
                }

    set_gain_unity(coef, sz2);
  }

  for(i=0; i<sz2; i++)
  {
    dest[i] = coef[i] * src[i];

    dest[(sz - 1) - i] = coef[i] * src[(sz - 1) - i];
  }
}


static void set_gain_unity(double *arr, int sz)
{
  int i;

  double total = 0.0;

  if(sz < 4)  return;

  for(i=0; i<sz; i++)
  {
    total += arr[i];
  }

  total /= sz;

  if(total != 0.0)
  {
    for(i=0; i<sz; i++)
    {
      arr[i] /= total;
    }
  }
}











