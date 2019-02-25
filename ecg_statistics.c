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




#include "ecg_statistics.h"


int ecg_get_hr_statistics(double *beat_interval_list, int beat_cnt, struct ecg_hr_statistics_struct *ecg_stats)
{
  int i,
      NN20=0,
      NN50=0;

  double d_tmp,
         average_bpm=0,
         average_rr=0,
         sdnn_bpm=0,
         sdnn_rr=0,
         rmssd_rr=0,
         *buf_bpm=NULL;

  if(beat_interval_list == NULL)  return -1;

  if(ecg_stats == NULL)  return -2;

  if(beat_cnt < 3)  return -3;

  buf_bpm = (double *)malloc(sizeof(double) * beat_cnt);
  if(buf_bpm == NULL)
  {
    return -4;
  }

  for(i=0; i<beat_cnt; i++)
  {
    if(beat_interval_list[i] < 1e-15)
    {
      free(buf_bpm);
      buf_bpm = NULL;
      return -5;
    }

    buf_bpm[i] = 60.0 / beat_interval_list[i];

    average_bpm += buf_bpm[i];
    average_rr += beat_interval_list[i];

    if(i < (beat_cnt - 1))
    {
      d_tmp = (beat_interval_list[i] - beat_interval_list[i + 1]) * 1000.0;

      rmssd_rr += (d_tmp * d_tmp);

      if(((beat_interval_list[i] - beat_interval_list[i + 1]) > 0.02 ) || ((beat_interval_list[i + 1] - beat_interval_list[i]) > 0.02 ))
      {
        NN20++;
      }

      if(((beat_interval_list[i] - beat_interval_list[i + 1]) > 0.05 ) || ((beat_interval_list[i + 1] - beat_interval_list[i]) > 0.05 ))
      {
        NN50++;
      }
    }
  }

  average_bpm /= beat_cnt;
  average_rr /= beat_cnt;
  rmssd_rr /= beat_cnt;
  rmssd_rr = sqrt(rmssd_rr);

  for(i=0; i<beat_cnt; i++)
  {
    sdnn_bpm += (buf_bpm[i] - average_bpm) * (buf_bpm[i] - average_bpm);
    sdnn_rr += (beat_interval_list[i] - average_rr) * (beat_interval_list[i] - average_rr);
  }

  sdnn_bpm = sqrt(sdnn_bpm / beat_cnt);
  sdnn_rr = sqrt(sdnn_rr / beat_cnt);

  ecg_stats->beat_cnt = beat_cnt;
  ecg_stats->mean_rr = average_rr * 1000.0;
  ecg_stats->sdnn_rr = sdnn_rr * 1000.0;
  ecg_stats->rmssd_rr = rmssd_rr;
  ecg_stats->mean_hr = average_bpm;
  ecg_stats->sdnn_hr = sdnn_bpm;
  ecg_stats->NN20 = NN20;
  ecg_stats->NN50 = NN50;
  ecg_stats->pNN20 = (NN20 * 100.0) / (beat_cnt - 1);
  ecg_stats->pNN50 = (NN50 * 100.0) / (beat_cnt - 1);

  free(buf_bpm);
  buf_bpm = NULL;

  return 0;
}

















