/*
***************************************************************************
*
* Author: Teunis van Beelen
*
* Copyright (C) 2018 - 2019 Teunis van Beelen
*
* Email: teuniz@gmail.com
*
**************************************************************************
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
*
*/


#include "fir_filter.h"


struct fir_filter_settings * create_fir_filter(double *vars, int sz)
{
  struct fir_filter_settings *settings;

  if((vars == NULL) || (sz < 1) || (sz > 1000))  return NULL;

  settings = (struct fir_filter_settings *)calloc(1, sizeof(struct fir_filter_settings));
  if(settings == NULL)
  {
    return NULL;
  }

  settings->vars = (double *)calloc(1, sizeof(double) * sz);
  if(settings == NULL)
  {
    free(settings);
    return NULL;
  }

  settings->buf = (double *)calloc(1, sizeof(double) * sz);
  if(settings == NULL)
  {
    free(settings->vars);
    free(settings);
    return NULL;
  }

  settings->buf_sav = (double *)calloc(1, sizeof(double) * sz);
  if(settings == NULL)
  {
    free(settings->buf);
    free(settings->vars);
    free(settings);
    return NULL;
  }

  settings->sz = sz;

  settings->idx = 0;

  settings->idx_sav = 0;

  memcpy(settings->vars, vars, sizeof(double) * sz);

  return settings;
}


double run_fir_filter(double val, struct fir_filter_settings *settings)
{
  int i, j;

  double result = 0;

  if(settings == NULL)  return 0;

  j = settings->idx;

  settings->buf[settings->idx++] = val;

  settings->idx %= settings->sz;

  for(i=0; i<settings->sz; i++)
  {
    result += settings->buf[j--] * settings->vars[i];

    if(j < 0)  j = settings->sz - 1;
  }

  return result;
}


void free_fir_filter(struct fir_filter_settings *settings)
{
  if(settings == NULL)  return;

  free(settings->vars);
  free(settings->buf);
  free(settings->buf_sav);
  free(settings);
}


void reset_fir_filter(double val, struct fir_filter_settings *settings)
{
  int i;

  if(settings == NULL)  return;

  for(i=0; i<settings->sz; i++)
  {
    settings->buf[i] = val;
  }
}


struct fir_filter_settings * create_fir_filter_copy(struct fir_filter_settings *settings)
{
  int i;

  struct fir_filter_settings *new_settings;

  if(settings == NULL)  return NULL;

  new_settings = create_fir_filter(settings->vars, settings->sz);

  if(new_settings == NULL)  return NULL;

  for(i=0; i<settings->sz; i++)
  {
    new_settings->buf[i] = settings->buf[i];
  }

  return new_settings;
}


int fir_filter_size(struct fir_filter_settings *settings)
{
  if(settings == NULL)  return 0;

  return settings->sz;
}


double fir_filter_tap(int idx, struct fir_filter_settings *settings)
{
  if(settings == NULL)  return 0;

  if((idx < 0) || (idx >= settings->sz))  return 0;

  return settings->vars[idx];
}


void fir_filter_save_buf(struct fir_filter_settings *settings)
{
  int i;

  for(i=0; i<settings->sz; i++)
  {
    settings->buf_sav[i] = settings->buf[i];
  }

  settings->idx_sav = settings->idx;
}


void fir_filter_restore_buf(struct fir_filter_settings *settings)
{
  int i;

  for(i=0; i<settings->sz; i++)
  {
    settings->buf[i] = settings->buf_sav[i];
  }

  settings->idx = settings->idx_sav;
}



















