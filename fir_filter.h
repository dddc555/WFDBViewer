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


#ifndef FIR_filter_INCLUDED
#define FIR_filter_INCLUDED


#include <stdlib.h>
#include <string.h>
#include <stdio.h>



#ifdef __cplusplus
extern "C" {
#endif



struct fir_filter_settings{
  double *vars;
  double *buf;
  double *buf_sav;
  int sz;
  int idx;
  int idx_sav;
};


struct fir_filter_settings * create_fir_filter(double *, int);
double run_fir_filter(double, struct fir_filter_settings *);
void free_fir_filter(struct fir_filter_settings *);
void reset_fir_filter(double, struct fir_filter_settings *);
struct fir_filter_settings * create_fir_filter_copy(struct fir_filter_settings *);
int fir_filter_size(struct fir_filter_settings *);
double fir_filter_tap(int, struct fir_filter_settings *);
void fir_filter_restore_buf(struct fir_filter_settings *);
void fir_filter_save_buf(struct fir_filter_settings *);


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif







