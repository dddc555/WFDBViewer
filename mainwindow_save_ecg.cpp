/*
***************************************************************************
*
* Author: Teunis van Beelen
*
* Copyright (C) 2007 - 2019 Teunis van Beelen
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


#include "mainwindow.h"


void UI_Mainwindow::save_ecg()
{
  qDebug()<<"save_ecg";
  int i, j, k, r, s,
      temp=0,
      skip,
      totalsize,
      hasprefilter=0,
      readsize=0,
      dif;

  double pre_time=0.0,
         d_temp=0.0,
         dig_value;

  long long l_temp,
            datarecords;

  union {
          unsigned int one;
          signed int one_signed;
          unsigned short two[2];
          signed short two_signed[2];
          unsigned char four[4];
        } var;

  struct date_time_struct date_time_str;


  for(i=0; i<files_open; i++) edfheaderlist[i]->prefiltertime = 0;

  // init pre_time - with filter
  for(i=0; i<signalcomps; i++)
  {
    if(signalcomp[i]->filter_cnt)
    {
      hasprefilter = 1;

      for(k=0; k<signalcomp[i]->filter_cnt; k++)
      {
        if(pre_time < (1.0 / signalcomp[i]->filter[k]->cutoff_frequency))
        {
          pre_time = (1.0 / signalcomp[i]->filter[k]->cutoff_frequency);
        }
      }
    }

    if(signalcomp[i]->spike_filter)
    {
      hasprefilter = 1;

      if(pre_time < 5.0)
      {
        pre_time = 5.0;
      }
    }

    if(signalcomp[i]->plif_ecg_filter)
    {
      hasprefilter = 1;

      if(pre_time < 2.0)
      {
        pre_time = 2.0;
      }
    }

    if(signalcomp[i]->ravg_filter_cnt)
    {
      hasprefilter = 1;

      for(k=0; k<signalcomp[i]->ravg_filter_cnt; k++)
      {
        if(pre_time < ((double)(signalcomp[i]->ravg_filter[k]->size + 3) / signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[0]].sf_f))
        {
          pre_time = (double)(signalcomp[i]->ravg_filter[k]->size + 3) / signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[0]].sf_f;
        }
      }
    }

    if(signalcomp[i]->fir_filter != NULL)
    {
      hasprefilter = 1;

      if(pre_time < ((double)(fir_filter_size(signalcomp[i]->fir_filter) + 3) / signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[0]].sf_f))
      {
        pre_time = (double)(fir_filter_size(signalcomp[i]->fir_filter) + 3) / signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[0]].sf_f;
      }
    }

    if(signalcomp[i]->ecg_filter != NULL)
    {
      hasprefilter = 1;

      if(pre_time < 10.0)
      {
        pre_time = 10.0;
      }
    }

    if(signalcomp[i]->zratio_filter != NULL)
    {
      hasprefilter = 1;

      if(pre_time < 4.0)
      {
        pre_time = 4.0;
      }
    }

    if(signalcomp[i]->fidfilter_cnt)
    {
      hasprefilter = 1;

      for(k=0; k<signalcomp[i]->fidfilter_cnt; k++)
      {
        if(pre_time < ((2.0 * signalcomp[i]->fidfilter_order[k]) / signalcomp[i]->fidfilter_freq[k]))
        {
          pre_time = (2.0 * signalcomp[i]->fidfilter_order[k]) / signalcomp[i]->fidfilter_freq[k];
        }
      }
    }
  } // end for - init pre_time - with filter

  // if has prefilter
  if(hasprefilter)
  {
    for(i=0; i<signalcomps; i++)
    {
      if((signalcomp[i]->filter_cnt) || (signalcomp[i]->spike_filter) || (signalcomp[i]->ravg_filter_cnt) || (signalcomp[i]->fidfilter_cnt) || (signalcomp[i]->fir_filter != NULL) || (signalcomp[i]->plif_ecg_filter != NULL) || (signalcomp[i]->ecg_filter != NULL) || (signalcomp[i]->zratio_filter != NULL))
      {
        signalcomp[i]->edfhdr->prefiltertime = (long long)(pre_time * ((double)TIME_DIMENSION));
        if(signalcomp[i]->edfhdr->prefiltertime>signalcomp[i]->edfhdr->viewtime)
        {
          signalcomp[i]->edfhdr->prefiltertime = signalcomp[i]->edfhdr->viewtime;
          if(signalcomp[i]->edfhdr->prefiltertime<0) signalcomp[i]->edfhdr->prefiltertime = 0;
        }
      }
    }

    totalsize = 0;

    for(i=0; i<signalcomps; i++)
    {
      if(signalcomp[i]->edfhdr->prefiltertime)  signalcomp[i]->records_in_viewbuf = (signalcomp[i]->edfhdr->viewtime / signalcomp[i]->edfhdr->long_data_record_duration) - ((signalcomp[i]->edfhdr->viewtime - signalcomp[i]->edfhdr->prefiltertime) / signalcomp[i]->edfhdr->long_data_record_duration) + 1;
      else signalcomp[i]->records_in_viewbuf = 0;

      signalcomp[i]->viewbufsize = signalcomp[i]->records_in_viewbuf * signalcomp[i]->edfhdr->recordsize;

      if(signalcomp[i]->edfhdr->prefiltertime)
      {
        signalcomp[i]->samples_in_prefilterbuf = (signalcomp[i]->records_in_viewbuf - 1) * signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[0]].smp_per_record;

        signalcomp[i]->samples_in_prefilterbuf
        += (int)(((double)(signalcomp[i]->edfhdr->viewtime % signalcomp[i]->edfhdr->long_data_record_duration)
        / (double)signalcomp[i]->edfhdr->long_data_record_duration)
        * (double)signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[0]].smp_per_record);
      }
      else
      {
        signalcomp[i]->samples_in_prefilterbuf = 0;
      }

      if(!i)
      {
        signalcomp[i]->viewbufoffset = 0;
        totalsize = signalcomp[i]->viewbufsize;
      }
      else
      {
        skip = 0;

        for(j=0; j<i; j++)
        {
          if(signalcomp[i]->edfhdr->file_hdl==signalcomp[j]->edfhdr->file_hdl)
          {
            skip = 1;
            signalcomp[i]->viewbufoffset = signalcomp[j]->viewbufoffset;
            signalcomp[i]->records_in_viewbuf = signalcomp[j]->records_in_viewbuf;
            signalcomp[i]->viewbufsize = signalcomp[j]->viewbufsize;
            break;
          }
        }

        if(!skip)
        {
          signalcomp[i]->viewbufoffset = totalsize;
          totalsize += signalcomp[i]->viewbufsize;
        }
      }
    }

    if(viewbuf!=NULL)
    {
      free(viewbuf);
      viewbuf = NULL;
    }

    for(i=0, l_temp=0; i<signalcomps; i++)
    {
      l_temp += (long long)signalcomp[i]->records_in_viewbuf * (long long)signalcomp[i]->edfhdr->recordsize;
    }

    if(l_temp >= 0x80000000)
    {
      live_stream_active = 0;
      QMessageBox messagewindow(QMessageBox::Critical, "Error", "Internal error: Memory limit protection:\n\"prefilterbuf\"");
      messagewindow.exec();

      remove_all_signals();
      return;
    }

    viewbuf = (char *)malloc(totalsize);
    if(viewbuf==NULL)
    {
      live_stream_active = 0;
      QMessageBox messagewindow(QMessageBox::Critical, "Error", "Internal error: Memory allocation error:\n\"prefilterbuf\"");
      messagewindow.exec();
      remove_all_signals();
      return;
    }

    for(i=0; i<signalcomps; i++)
    {
      if(!i)
      {
        for(int mmRecord = 0;mmRecord<edfheaderlist[sel_viewtime]->datarecords;mmRecord++) {
            datarecords = mmRecord;

            signalcomp[i]->prefilter_starttime = datarecords * signalcomp[i]->edfhdr->long_data_record_duration;

            if((signalcomp[i]->viewbufsize>0)&&(datarecords<signalcomp[i]->edfhdr->datarecords))
            {
              fseeko(signalcomp[i]->edfhdr->file_hdl, (long long)(signalcomp[i]->edfhdr->hdrsize + (datarecords * signalcomp[i]->edfhdr->recordsize)), SEEK_SET);

              if(signalcomp[i]->viewbufsize>((signalcomp[i]->edfhdr->datarecords - datarecords) * signalcomp[i]->edfhdr->recordsize))
              {
                signalcomp[i]->viewbufsize = (signalcomp[i]->edfhdr->datarecords - datarecords) * signalcomp[i]->edfhdr->recordsize;
              }
              qDebug()<<"fread" << __LINE__;
              if(fread(viewbuf + signalcomp[i]->viewbufoffset, signalcomp[i]->viewbufsize, 1, signalcomp[i]->edfhdr->file_hdl)!=1)
              {
                live_stream_active = 0;
                QMessageBox messagewindow(QMessageBox::Critical, "Error", "A read error occurred. 2");
                messagewindow.exec();
                remove_all_signals();
                return;
              }
            }
        }//end mmRecord for
      }
      else
      {
        skip = 0;

        for(j=0; j<i; j++)
        {
          if(signalcomp[i]->edfhdr->file_hdl==signalcomp[j]->edfhdr->file_hdl)
          {
            skip = 1;
            break;
          }
        }
        for(int mmRecord = 0;mmRecord<edfheaderlist[sel_viewtime]->datarecords;mmRecord++) {

            if(!skip)
            {
              datarecords = mmRecord;

              signalcomp[i]->prefilter_starttime = datarecords * signalcomp[i]->edfhdr->long_data_record_duration;

              if((signalcomp[i]->viewbufsize>0)&&(datarecords<signalcomp[i]->edfhdr->datarecords))
              {
                fseeko(signalcomp[i]->edfhdr->file_hdl, (long long)(signalcomp[i]->edfhdr->hdrsize + (datarecords * signalcomp[i]->edfhdr->recordsize)), SEEK_SET);

                if(signalcomp[i]->viewbufsize>((signalcomp[i]->edfhdr->datarecords - datarecords) * signalcomp[i]->edfhdr->recordsize))
                {
                  signalcomp[i]->viewbufsize = (signalcomp[i]->edfhdr->datarecords - datarecords) * signalcomp[i]->edfhdr->recordsize;
                }
                qDebug()<<"fread" << __LINE__;
                if(fread(viewbuf + signalcomp[i]->viewbufoffset, signalcomp[i]->viewbufsize, 1, signalcomp[i]->edfhdr->file_hdl)!=1)
                {
                  live_stream_active = 0;
                  QMessageBox messagewindow(QMessageBox::Critical, "Error", "A read error occurred. 3");
                  messagewindow.exec();
                  remove_all_signals();
                  return;
                }
              }
            }
        }// end mmRecord
      }
    } // end for(i=0; i<signalcomps; i++)

    for(i=0; i<signalcomps; i++)
    {
      if(signalcomp[i]->zratio_filter != NULL)
      {
        l_temp = signalcomp[i]->prefilter_starttime % (TIME_DIMENSION * 2LL); // necessary for the Z-ratio filter
        if(l_temp != 0L)
        {
          temp = (TIME_DIMENSION * 2LL) - l_temp;

          l_temp = temp;

          signalcomp[i]->prefilter_reset_sample = (l_temp / signalcomp[i]->edfhdr->long_data_record_duration)
          * signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[0]].smp_per_record;

          signalcomp[i]->prefilter_reset_sample
          += (int)(((double)(l_temp % signalcomp[i]->edfhdr->long_data_record_duration)
          / (double)signalcomp[i]->edfhdr->long_data_record_duration)
          * (double)signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[0]].smp_per_record);
        }
        else
        {
          signalcomp[i]->prefilter_reset_sample = 0;
        }

// printf("records_in_viewbuf is %lli\n"
//       "samples_in_prefilterbuf is %i\n"
//       "l_temp is %lli\n"
//       "temp is %i\n"
//       "prefilter_reset_sample is %i\n\n",
//       signalcomp[i]->records_in_viewbuf,
//       signalcomp[i]->samples_in_prefilterbuf,
//       l_temp,
//       temp,
//       signalcomp[i]->prefilter_reset_sample);

      }
    }

    for(i=0; i<signalcomps; i++)
    {
      if((!signalcomp[i]->filter_cnt) && (!signalcomp[i]->spike_filter) && (!signalcomp[i]->ravg_filter_cnt) && (!signalcomp[i]->fidfilter_cnt) && (!signalcomp[i]->fir_filter) && (!signalcomp[i]->plif_ecg_filter) && (signalcomp[i]->ecg_filter == NULL) && (signalcomp[i]->zratio_filter == NULL)) continue;

      for(s=0; s<signalcomp[i]->samples_in_prefilterbuf; s++)
      {
        dig_value = 0.0;

        for(k=0; k<signalcomp[i]->num_of_signals; k++)
        {
          if(signalcomp[i]->edfhdr->bdf)
          {
            var.two[0] = *((unsigned short *)(
              viewbuf
              + signalcomp[i]->viewbufoffset
              + (signalcomp[i]->edfhdr->recordsize * (s / signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[k]].smp_per_record))
              + signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[k]].buf_offset
              + ((s % signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[k]].smp_per_record) * 3)));

            var.four[2] = *((unsigned char *)(
              viewbuf
              + signalcomp[i]->viewbufoffset
              + (signalcomp[i]->edfhdr->recordsize * (s / signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[k]].smp_per_record))
              + signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[k]].buf_offset
              + ((s % signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[k]].smp_per_record) * 3)
              + 2));

            if(var.four[2]&0x80)
            {
              var.four[3] = 0xff;
            }
            else
            {
              var.four[3] = 0x00;
            }

            d_temp = var.one_signed;
          }

          if(signalcomp[i]->edfhdr->edf)
          {
            d_temp = *(((short *)(
            viewbuf
            + signalcomp[i]->viewbufoffset
            + (signalcomp[i]->edfhdr->recordsize * (s / signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[k]].smp_per_record))
            + signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[k]].buf_offset))
            + (s % signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[k]].smp_per_record));
          }

          d_temp += signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[k]].offset;
          d_temp *= signalcomp[i]->factor[k];

          dig_value += d_temp;
        }

        if(signalcomp[i]->spike_filter)
        {
          dig_value = run_spike_filter(dig_value, signalcomp[i]->spike_filter);
        }

        for(j=0; j<signalcomp[i]->filter_cnt; j++)
        {
          dig_value = first_order_filter(dig_value, signalcomp[i]->filter[j]);
        }

        for(j=0; j<signalcomp[i]->ravg_filter_cnt; j++)
        {
          dig_value = run_ravg_filter(dig_value, signalcomp[i]->ravg_filter[j]);
        }

        for(j=0; j<signalcomp[i]->fidfilter_cnt; j++)
        {
          dig_value = signalcomp[i]->fidfuncp[j](signalcomp[i]->fidbuf[j], dig_value);
        }

        if(signalcomp[i]->fir_filter != NULL)
        {
          dig_value = run_fir_filter(dig_value, signalcomp[i]->fir_filter);
        }

        if(signalcomp[i]->plif_ecg_filter)
        {
          dig_value = plif_run_subtract_filter(dig_value, signalcomp[i]->plif_ecg_filter);
        }

        if(signalcomp[i]->ecg_filter != NULL)
        {
          if(s == 0)
          {
            reset_ecg_filter(signalcomp[i]->ecg_filter);
          }

          dig_value = run_ecg_filter(dig_value, signalcomp[i]->ecg_filter);
        }

        if(signalcomp[i]->zratio_filter != NULL)
        {
          if(s == signalcomp[i]->prefilter_reset_sample)
          {
            reset_zratio_filter(signalcomp[i]->zratio_filter);
          }

          dig_value = run_zratio_filter(dig_value, signalcomp[i]->zratio_filter);
        }
      }
    }

    for(i=0; i<signalcomps; i++)
    {
      if(signalcomp[i]->samples_in_prefilterbuf > 0)
      {
        if(signalcomp[i]->spike_filter)
        {
          spike_filter_save_buf(signalcomp[i]->spike_filter);
        }

        for(j=0; j<signalcomp[i]->filter_cnt; j++)
        {
          signalcomp[i]->filterpreset_a[j] = signalcomp[i]->filter[j]->old_input;
          signalcomp[i]->filterpreset_b[j] = signalcomp[i]->filter[j]->old_output;
        }

        for(j=0; j<signalcomp[i]->ravg_filter_cnt; j++)
        {
          ravg_filter_save_buf(signalcomp[i]->ravg_filter[j]);
        }

        for(j=0; j<signalcomp[i]->fidfilter_cnt; j++)
        {
          memcpy(signalcomp[i]->fidbuf2[j], signalcomp[i]->fidbuf[j], fid_run_bufsize(signalcomp[i]->fid_run[j]));
        }

        if(signalcomp[i]->fir_filter != NULL)
        {
          fir_filter_save_buf(signalcomp[i]->fir_filter);
        }

        if(signalcomp[i]->plif_ecg_filter)
        {
          plif_subtract_filter_state_copy(signalcomp[i]->plif_ecg_filter_sav, signalcomp[i]->plif_ecg_filter);
        }

        if(signalcomp[i]->ecg_filter != NULL)
        {
          ecg_filter_save_buf(signalcomp[i]->ecg_filter);
        }

        if(signalcomp[i]->zratio_filter != NULL)
        {
          zratio_filter_save_buf(signalcomp[i]->zratio_filter);
        }
      }
    }
  } // end if has prefilter

  totalsize = 0;

  for(i=0; i<signalcomps; i++)
  {
    if(signalcomp[i]->edfhdr->viewtime>=0)  signalcomp[i]->records_in_viewbuf = ((pagetime + (signalcomp[i]->edfhdr->viewtime % signalcomp[i]->edfhdr->long_data_record_duration)) / signalcomp[i]->edfhdr->long_data_record_duration) + 1;
    else  signalcomp[i]->records_in_viewbuf = ((pagetime + ((-(signalcomp[i]->edfhdr->viewtime)) % signalcomp[i]->edfhdr->long_data_record_duration)) / signalcomp[i]->edfhdr->long_data_record_duration) + 1;

    signalcomp[i]->viewbufsize = signalcomp[i]->records_in_viewbuf * signalcomp[i]->edfhdr->recordsize;

//     printf("viewbuf test: signalcomp: %i  records_in_viewbuf: %lli  recordsize: %i  viewbufsize: %i\n",
//            i, signalcomp[i]->records_in_viewbuf, signalcomp[i]->edfhdr->recordsize, signalcomp[i]->viewbufsize);

    signalcomp[i]->samples_on_screen = (int)(((double)pagetime / (double)signalcomp[i]->edfhdr->long_data_record_duration) * (double)signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[0]].smp_per_record);

    if(signalcomp[i]->edfhdr->viewtime<0)
    {
      d_temp =
        (((double)(-(signalcomp[i]->edfhdr->viewtime)))
        / (double)signalcomp[i]->edfhdr->long_data_record_duration)
        * (double)signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[0]].smp_per_record;

      if(d_temp>2147483648.0)
      {
        signalcomp[i]->sample_start = 2147483647LL;
      }
      else
      {
        signalcomp[i]->sample_start = (int)d_temp;
      }
    }
    else
    {
      signalcomp[i]->sample_start = 0;
    }

    if(signalcomp[i]->edfhdr->viewtime>=0)
    {
      signalcomp[i]->sample_timeoffset_part = ((double)(signalcomp[i]->edfhdr->viewtime % signalcomp[i]->edfhdr->long_data_record_duration) / (double)signalcomp[i]->edfhdr->long_data_record_duration) * (double)signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[0]].smp_per_record;
      signalcomp[i]->sample_timeoffset = (int)(signalcomp[i]->sample_timeoffset_part);
      signalcomp[i]->sample_timeoffset_part -= signalcomp[i]->sample_timeoffset;
    }
    else
    {
      signalcomp[i]->sample_timeoffset_part = 0.0;
      signalcomp[i]->sample_timeoffset = 0;
    }

    if(!i)
    {
      signalcomp[i]->viewbufoffset = 0;
      totalsize = signalcomp[i]->viewbufsize;
    }
    else
    {
      skip = 0;

      for(j=0; j<i; j++)
      {
        if(signalcomp[i]->edfhdr->file_hdl==signalcomp[j]->edfhdr->file_hdl)
        {
          skip = 1;
          signalcomp[i]->viewbufoffset = signalcomp[j]->viewbufoffset;
          signalcomp[i]->records_in_viewbuf = signalcomp[j]->records_in_viewbuf;
          signalcomp[i]->viewbufsize = signalcomp[j]->viewbufsize;
          break;
        }
      }

      if(!skip)
      {
        signalcomp[i]->viewbufoffset = totalsize;
        totalsize += signalcomp[i]->viewbufsize;
      }
    }
  } // end for

  if(viewbuf!=NULL)
  {
    free(viewbuf);
    viewbuf = NULL;
  }

  for(i=0, l_temp=0; i<signalcomps; i++)
  {
    l_temp += (long long)signalcomp[i]->records_in_viewbuf * (long long)signalcomp[i]->edfhdr->recordsize;
  }

  if(l_temp >= 0x80000000)
  {
    live_stream_active = 0;
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "The system was not able to provide enough resources (memory) to perform the requested action.\n"
                                                              "Decrease the timescale and try again.");
    messagewindow.exec();

    remove_all_signals();
    return;
  }

  if(totalsize)
  {
    viewbuf = (char *)malloc(totalsize);
    if(viewbuf==NULL)
    {
      live_stream_active = 0;
      QMessageBox messagewindow(QMessageBox::Critical, "Error", "The system was not able to provide enough resources (memory) to perform the requested action.\n"
                                                                "Decrease the timescale and try again.");
      messagewindow.exec();

      remove_all_signals();
      return;
    }
  }

  for(i=0; i<signalcomps; i++)
  {
    if(!i)
    {
      for(int mmRecord = 0;mmRecord<edfheaderlist[sel_viewtime]->datarecords;mmRecord++) {
          datarecords = mmRecord;
          qDebug()<<"datarecords (mmRecord)"<<__LINE__<<"va "<<datarecords;

          dif = signalcomp[i]->edfhdr->datarecords - datarecords;

          if(dif<=0)
          {
            memset(viewbuf + signalcomp[i]->viewbufoffset, 0, signalcomp[i]->records_in_viewbuf * signalcomp[i]->edfhdr->recordsize);

            signalcomp[i]->sample_stop = 0;
          }
          else
          {
            if(dif<signalcomp[i]->records_in_viewbuf)
            {
              readsize = dif * signalcomp[i]->edfhdr->recordsize;

              memset(viewbuf + signalcomp[i]->viewbufoffset + readsize, 0, (signalcomp[i]->records_in_viewbuf * signalcomp[i]->edfhdr->recordsize) - readsize);

              signalcomp[i]->sample_stop = (dif * signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[0]].smp_per_record) - signalcomp[i]->sample_timeoffset;
            }
            else
            {
              readsize = signalcomp[i]->records_in_viewbuf * signalcomp[i]->edfhdr->recordsize;

              signalcomp[i]->sample_stop = signalcomp[i]->samples_on_screen;
            }

            l_temp = signalcomp[i]->edfhdr->hdrsize;
            l_temp += (datarecords * signalcomp[i]->edfhdr->recordsize);

            fseeko(signalcomp[i]->edfhdr->file_hdl, l_temp, SEEK_SET);
            qDebug()<<"fread" << __LINE__<<"signalcomps = "<<signalcomps<<"datarecords"<<datarecords<<"readsize"<<readsize;
            if(fread(viewbuf + signalcomp[i]->viewbufoffset, readsize, 1, signalcomp[i]->edfhdr->file_hdl)!=1)
            {
              live_stream_active = 0;
              QMessageBox messagewindow(QMessageBox::Critical, "Error", "A read error occurred. 5");
              messagewindow.exec();
              remove_all_signals();
              return;
            }
          }
      }// end mmRecords for
    }

    else
    {
      skip = 0;

      for(j=0; j<i; j++)
      {
        if(signalcomp[i]->edfhdr->file_hdl==signalcomp[j]->edfhdr->file_hdl)
        {
          skip = 1;
          break;
        }
      }
      qDebug()<<"edfheaderlist[sel_viewtime]->datarecords" <<edfheaderlist[sel_viewtime]->datarecords;

      for(int mmRecord = 0;mmRecord<edfheaderlist[sel_viewtime]->datarecords - 1;mmRecord++) {

          datarecords = mmRecord;
          qDebug()<<"datarecords (line)"<<__LINE__<<"datarecords "<<datarecords << "skip"<<skip;

          dif = signalcomp[i]->edfhdr->datarecords - datarecords;

          if(dif<=0)
          {
            if(!skip)
            {
              memset(viewbuf + signalcomp[i]->viewbufoffset, 0, signalcomp[i]->records_in_viewbuf * signalcomp[i]->edfhdr->recordsize);
            }

            signalcomp[i]->sample_stop = 0;
          }
          else
          {
            if(dif<signalcomp[i]->records_in_viewbuf)
            {
              if(!skip)
              {
                readsize = dif * signalcomp[i]->edfhdr->recordsize;

    //             printf("viewbuf test: signalcomp: %i  viewbufoffset: %i  readsize: %i  records_in_viewbuf: %lli  recordsize: %i\n"
    //                    "viewtime: %lli  datarecords: %lli  dif: %i  readsize: %i  (records_in_viewbuf * recordsize): %lli\n"
    //                    "viewbufsize: %i\n",
    //                    i, signalcomp[i]->viewbufoffset, readsize, signalcomp[i]->records_in_viewbuf, signalcomp[i]->edfhdr->recordsize,
    //                    signalcomp[i]->edfhdr->viewtime, signalcomp[i]->edfhdr->datarecords, dif, readsize,
    //                    signalcomp[i]->records_in_viewbuf * signalcomp[i]->edfhdr->recordsize, signalcomp[i]->viewbufsize);

                memset(viewbuf + signalcomp[i]->viewbufoffset + readsize, 0, (signalcomp[i]->records_in_viewbuf * signalcomp[i]->edfhdr->recordsize) - readsize);
              }

              signalcomp[i]->sample_stop = (dif * signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[0]].smp_per_record) - signalcomp[i]->sample_timeoffset;
            }
            else
            {
              if(!skip)
              {
                readsize = signalcomp[i]->records_in_viewbuf * signalcomp[i]->edfhdr->recordsize;
              }

              signalcomp[i]->sample_stop = signalcomp[i]->samples_on_screen;
            }

            if(!skip)
            {
              l_temp = signalcomp[i]->edfhdr->hdrsize;
              l_temp += (datarecords * signalcomp[i]->edfhdr->recordsize);

              fseeko(signalcomp[i]->edfhdr->file_hdl, l_temp, SEEK_SET);
              qDebug()<<"fread" << __LINE__;
              if(fread(viewbuf + signalcomp[i]->viewbufoffset, readsize, 1, signalcomp[i]->edfhdr->file_hdl)!=1)
              {
                live_stream_active = 0;
                QMessageBox messagewindow(QMessageBox::Critical, "Error", "A read error occurred. 6");
                messagewindow.exec();
                remove_all_signals();
                return;
              }
            }
          }

      }// end mmRecord
      //*/
    }


    signalcomp[i]->sample_stop += signalcomp[i]->sample_start;
  } // end for
  qDebug()<<"end loop";

  if(signalcomps && (!signal_averaging_active))
  {
    viewtime_string[0] = 0;

    if(viewtime_indicator_type == 2)
    {
      l_temp = (edfheaderlist[sel_viewtime]->viewtime + edfheaderlist[sel_viewtime]->starttime_offset) / TIME_DIMENSION;
      l_temp += edfheaderlist[sel_viewtime]->utc_starttime;
      utc_to_date_time(l_temp, &date_time_str);

      snprintf(viewtime_string, 32, "%2i-%s ", date_time_str.day, date_time_str.month_str);
    }

    if((edfheaderlist[sel_viewtime]->viewtime + edfheaderlist[sel_viewtime]->starttime_offset)>=0LL)
    {
      if(viewtime_indicator_type > 0)
      {
        snprintf(viewtime_string + strlen(viewtime_string), 32, "%2i:%02i:%02i.%04i (",
                (int)((((edfheaderlist[sel_viewtime]->l_starttime + edfheaderlist[sel_viewtime]->viewtime + edfheaderlist[sel_viewtime]->starttime_offset) / TIME_DIMENSION)/ 3600LL) % 24LL),
                (int)((((edfheaderlist[sel_viewtime]->l_starttime + edfheaderlist[sel_viewtime]->viewtime + edfheaderlist[sel_viewtime]->starttime_offset) / TIME_DIMENSION) % 3600LL) / 60LL),
                (int)(((edfheaderlist[sel_viewtime]->l_starttime + edfheaderlist[sel_viewtime]->viewtime + edfheaderlist[sel_viewtime]->starttime_offset) / TIME_DIMENSION) % 60LL),
                (int)(((edfheaderlist[sel_viewtime]->l_starttime + edfheaderlist[sel_viewtime]->viewtime + edfheaderlist[sel_viewtime]->starttime_offset) % TIME_DIMENSION) / 1000LL));
      }

      snprintf(viewtime_string + strlen(viewtime_string), 32, "%i:%02i:%02i.%04i",
              (int)((edfheaderlist[sel_viewtime]->viewtime / TIME_DIMENSION)/ 3600LL),
              (int)(((edfheaderlist[sel_viewtime]->viewtime / TIME_DIMENSION) % 3600LL) / 60LL),
              (int)((edfheaderlist[sel_viewtime]->viewtime / TIME_DIMENSION) % 60LL),
              (int)((edfheaderlist[sel_viewtime]->viewtime % TIME_DIMENSION) / 1000LL));

      if(viewtime_indicator_type > 0)
      {
        sprintf(viewtime_string + strlen(viewtime_string), ")");
      }
    }
    else
    {
      l_temp = edfheaderlist[sel_viewtime]->l_starttime + ((edfheaderlist[sel_viewtime]->viewtime + edfheaderlist[sel_viewtime]->starttime_offset) % (86400LL * TIME_DIMENSION));
      if(l_temp<=0)
      {
        l_temp += (86400LL * TIME_DIMENSION);
      }

      if(viewtime_indicator_type > 0)
      {
        snprintf(viewtime_string + strlen(viewtime_string), 32, "%2i:%02i:%02i.%04i (",
                (int)((((l_temp) / TIME_DIMENSION)/ 3600LL) % 24LL),
                (int)((((l_temp) / TIME_DIMENSION) % 3600LL) / 60LL),
                (int)(((l_temp) / TIME_DIMENSION) % 60LL),
                (int)(((l_temp) % TIME_DIMENSION) / 1000LL));
      }

      l_temp = -edfheaderlist[sel_viewtime]->viewtime;

      snprintf(viewtime_string + strlen(viewtime_string), 32, "-%i:%02i:%02i.%04i",
              (int)((l_temp / TIME_DIMENSION)/ 3600LL),
              (int)(((l_temp / TIME_DIMENSION) % 3600LL) / 60LL),
              (int)((l_temp / TIME_DIMENSION) % 60LL),
              (int)((l_temp % TIME_DIMENSION) / 1000LL));

      if(viewtime_indicator_type > 0)
      {
        sprintf(viewtime_string + strlen(viewtime_string), ")");
      }
    }

    if(pagetime >= (3600LL * TIME_DIMENSION))
    {
      snprintf(pagetime_string, 32, "%i:%02i:%02i.%04i",
              ((int)(pagetime / TIME_DIMENSION)) / 3600,
              (((int)(pagetime / TIME_DIMENSION)) % 3600) / 60,
              ((int)(pagetime / TIME_DIMENSION)) % 60,
              (int)((pagetime % TIME_DIMENSION) / 1000LL));
    }
    else if(pagetime >= (60LL * TIME_DIMENSION))
      {
        snprintf(pagetime_string, 32, "%i:%02i.%04i",
                ((int)(pagetime / TIME_DIMENSION)) / 60,
                ((int)(pagetime / TIME_DIMENSION)) % 60,
                (int)((pagetime % TIME_DIMENSION) / 1000LL));
      }
      else if(pagetime >= TIME_DIMENSION)
      {
        snprintf(pagetime_string, 32, "%i.%04i sec",
                (int)(pagetime / TIME_DIMENSION),
                (int)((pagetime % TIME_DIMENSION) / 1000LL));
      }
      else
      {
        convert_to_metric_suffix(pagetime_string, (double)pagetime / TIME_DIMENSION, 3);

        strcat(pagetime_string, "S");
      }

    remove_trailing_zeros(viewtime_string);
    remove_trailing_zeros(pagetime_string);
  } // end if(signalcomps && (!signal_averaging_active))

  if(!signal_averaging_active)
  {
    if(print_to_edf_active)
    {
      print_to_edf_active = 0;
    }
    else
    {
      if(signalcomps && (!live_stream_active))
      {
        positionslider->blockSignals(true);

        long long record_duration = edfheaderlist[sel_viewtime]->long_data_record_duration * edfheaderlist[sel_viewtime]->datarecords;

        record_duration -= pagetime;

        if(edfheaderlist[sel_viewtime]->viewtime<=0)
        {
          positionslider->setValue(0);
        }
        else
        {
          if(edfheaderlist[sel_viewtime]->viewtime>=record_duration)
          {
            positionslider->setValue(1000000);
          }
          else
          {
            if(record_duration<pagetime)
            {
              positionslider->setValue(1000000);
            }
            else
            {
              positionslider->setValue(edfheaderlist[sel_viewtime]->viewtime * 1000000LL / record_duration);
            }
          }
        }

        slidertoolbar->setEnabled(true);
      }
      else
      {
        slidertoolbar->setEnabled(false);

        positionslider->blockSignals(true);
      }

      maincurve->drawCurve_stage_1();

      if(signalcomps && (!live_stream_active))
      {
        positionslider->blockSignals(false);
      }
    }

    for(r=0; r<MAXSPECTRUMDOCKS; r++)
    {
      if(spectrumdock[r]->dock->isVisible())
      {
        spectrumdock[r]->rescan();
      }
    }
  } // end if(!signal_averaging_active)

   printf("\n");

   for(int n=0; n<signalcomps; n++)
   {
     printf("signalcomp: %i  filenum: %i  signal: %i  viewbufoffset: %i  buf_offset: %i\n",
            n,
            signalcomp[n]->filenum, signalcomp[n]->edfsignal[0],
            signalcomp[n]->viewbufoffset,
            signalcomp[n]->edfhdr->edfparam[signalcomp[n]->edfsignal[0]].buf_offset);
   }

   // ---------------------------------------------- wang draw code -----------------------------------------------------
   drawSimulate();
}
void UI_Mainwindow::drawSimulate(){
    int i, j, k, n, x1, y1, x2, y2,
        signalcomps,
        baseline,
        value,
        minimum,
        maximum,
        runin_samples,
        stat_zero_crossing=0, h, w;
    int printsize_y_factor;
    struct graphicBufStruct *graphicBuf;

    int* screensamples = (int *)calloc(1, sizeof(int[MAXSIGNALS]));

    int printing = 1;
    long long s, s2;
    double dig_value=0.0,
             f_tmp=0.0;

    union {
          unsigned int one;
          signed int one_signed;
          unsigned short two[2];
          signed short two_signed[2];
          unsigned char four[4];
        } var;
    // else use_threads viewcurve.cpp: line 2681 ~ line 3105

    for(i=0; i<signalcomps; i++)
    {
      signalcomp[i]->max_dig_value = -2147483647;
      signalcomp[i]->min_dig_value = 2147483647;
      signalcomp[i]->stat_cnt = 0;
      signalcomp[i]->stat_zero_crossing_cnt = 0;
      signalcomp[i]->stat_sum = 0.0;
      signalcomp[i]->stat_sum_sqr = 0.0;
      signalcomp[i]->stat_sum_rectified = 0.0;

      baseline = h / (signalcomps + 1);
      baseline *= (i + 1);

      signalcomp[i]->pixels_shift = signalcomp[i]->sample_timeoffset_part / signalcomp[i]->sample_pixel_ratio;

      for(s=signalcomp[i]->sample_start; s<signalcomp[i]->samples_on_screen; s++)
      {
        if(s>=signalcomp[i]->sample_stop)  break;

        dig_value = 0.0;
        s2 = s + signalcomp[i]->sample_timeoffset - signalcomp[i]->sample_start;

        for(j=0; j<signalcomp[i]->num_of_signals; j++)
        {
          if(signalcomp[i]->edfhdr->bdf)
          {
            var.two[0] = *((unsigned short *)(
              viewbuf
              + signalcomp[i]->viewbufoffset
              + (signalcomp[i]->edfhdr->recordsize * (s2 / signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[j]].smp_per_record))
              + signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[j]].buf_offset
              + ((s2 % signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[j]].smp_per_record) * 3)));

            var.four[2] = *((unsigned char *)(
              viewbuf
              + signalcomp[i]->viewbufoffset
              + (signalcomp[i]->edfhdr->recordsize * (s2 / signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[j]].smp_per_record))
              + signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[j]].buf_offset
              + ((s2 % signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[j]].smp_per_record) * 3)
              + 2));

            if(var.four[2]&0x80)
            {
              var.four[3] = 0xff;
            }
            else
            {
              var.four[3] = 0x00;
            }

            f_tmp = var.one_signed;
          }

          if(signalcomp[i]->edfhdr->edf)
          {
            f_tmp = *(((short *)(
              viewbuf
              + signalcomp[i]->viewbufoffset
              + (signalcomp[i]->edfhdr->recordsize * (s2 / signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[j]].smp_per_record))
              + signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[j]].buf_offset))
              + (s2 % signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[j]].smp_per_record));
          }

          f_tmp += signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[j]].offset;
          f_tmp *= signalcomp[i]->factor[j];

          dig_value += f_tmp;
        }

        if(signalcomp[i]->spike_filter)
        {
          if(s==signalcomp[i]->sample_start)
          {
            if(this->edfheaderlist[signalcomp[i]->filenum]->viewtime<=0)
            {
              reset_spike_filter(signalcomp[i]->spike_filter);
            }
            else
            {
              spike_filter_restore_buf(signalcomp[i]->spike_filter);
            }
          }

          dig_value = run_spike_filter(dig_value, signalcomp[i]->spike_filter);
        }

        for(k=0; k<signalcomp[i]->filter_cnt; k++)
        {
          if(s==signalcomp[i]->sample_start)
          {
            if(this->edfheaderlist[signalcomp[i]->filenum]->viewtime==0)
            {
              reset_filter(dig_value, signalcomp[i]->filter[k]);
            }
            else
            {
              signalcomp[i]->filter[k]->old_input = signalcomp[i]->filterpreset_a[k];
              signalcomp[i]->filter[k]->old_output = signalcomp[i]->filterpreset_b[k];
            }
          }

          dig_value = first_order_filter(dig_value, signalcomp[i]->filter[k]);
        }

        for(k=0; k<signalcomp[i]->ravg_filter_cnt; k++)
        {
          if(s==signalcomp[i]->sample_start)
          {
            if((this->edfheaderlist[signalcomp[i]->filenum]->viewtime <= 0) && signalcomp[i]->ravg_filter_setup[k])
            {
              reset_ravg_filter(dig_value, signalcomp[i]->ravg_filter[k]);
            }
            else
            {
              ravg_filter_restore_buf(signalcomp[i]->ravg_filter[k]);
            }

            signalcomp[i]->ravg_filter_setup[k] = 0;
          }

          dig_value = run_ravg_filter(dig_value, signalcomp[i]->ravg_filter[k]);
        }

        for(k=0; k<signalcomp[i]->fidfilter_cnt; k++)
        {
          if(s==signalcomp[i]->sample_start)
          {
            if((this->edfheaderlist[signalcomp[i]->filenum]->viewtime <= 0) && signalcomp[i]->fidfilter_setup[k])
            {
              runin_samples = signalcomp[i]->edfhdr->edfparam[signalcomp[i]->edfsignal[0]].sf_f / signalcomp[i]->fidfilter_freq[k];

              runin_samples *= 26;

              if(runin_samples < 10)
              {
                runin_samples = 10;
              }

              for(n=0; n<runin_samples; n++)
              {
                signalcomp[i]->fidfuncp[k](signalcomp[i]->fidbuf[k], dig_value);
              }

              memcpy(signalcomp[i]->fidbuf2[k], signalcomp[i]->fidbuf[k], fid_run_bufsize(signalcomp[i]->fid_run[k]));
            }
            else
            {
              memcpy(signalcomp[i]->fidbuf[k], signalcomp[i]->fidbuf2[k], fid_run_bufsize(signalcomp[i]->fid_run[k]));
            }

            signalcomp[i]->fidfilter_setup[k] = 0;
          }

          dig_value = signalcomp[i]->fidfuncp[k](signalcomp[i]->fidbuf[k], dig_value);
        }

        if(signalcomp[i]->fir_filter != NULL)
        {
          if(s==signalcomp[i]->sample_start)
          {
            if(this->edfheaderlist[signalcomp[i]->filenum]->viewtime<=0)
            {
              reset_fir_filter(0, signalcomp[i]->fir_filter);
            }
            else
            {
              fir_filter_restore_buf(signalcomp[i]->fir_filter);
            }
          }

          dig_value = run_fir_filter(dig_value, signalcomp[i]->fir_filter);
        }

        if(signalcomp[i]->plif_ecg_filter)
        {
          if(s==signalcomp[i]->sample_start)
          {
            if(this->edfheaderlist[signalcomp[i]->filenum]->viewtime<=0)
            {
              plif_reset_subtract_filter(signalcomp[i]->plif_ecg_filter, 0);
            }
            else
            {
              plif_subtract_filter_state_copy(signalcomp[i]->plif_ecg_filter, signalcomp[i]->plif_ecg_filter_sav);
            }
          }

          dig_value = plif_run_subtract_filter(dig_value, signalcomp[i]->plif_ecg_filter);
        }

        if(signalcomp[i]->ecg_filter != NULL)
        {
          if(s==signalcomp[i]->sample_start)
          {
            if(this->edfheaderlist[signalcomp[i]->filenum]->viewtime <= 0LL)
            {
              reset_ecg_filter(signalcomp[i]->ecg_filter);
            }
            else
            {
              ecg_filter_restore_buf(signalcomp[i]->ecg_filter);
            }
          }

          dig_value = run_ecg_filter(dig_value, signalcomp[i]->ecg_filter);
        }

        if(signalcomp[i]->zratio_filter != NULL)
        {
          if(s==signalcomp[i]->sample_start)
          {
            if(this->edfheaderlist[signalcomp[i]->filenum]->viewtime <= 0LL)
            {
              reset_zratio_filter(signalcomp[i]->zratio_filter);
            }
            else
            {
              zratio_filter_restore_buf(signalcomp[i]->zratio_filter);
            }
          }

          dig_value = run_zratio_filter(dig_value, signalcomp[i]->zratio_filter);
        }

        if(printing)
        {
          value = (int)(dig_value * signalcomp[i]->sensitivity[0] * printsize_y_factor) * signalcomp[i]->polarity;
        }
        else
        {
          value = (int)(dig_value * signalcomp[i]->sensitivity[0]) * signalcomp[i]->polarity;

          signalcomp[i]->stat_cnt++;
          signalcomp[i]->stat_sum += dig_value;
          signalcomp[i]->stat_sum_sqr += ((dig_value) * (dig_value));
          if(dig_value < 0)
          {
            signalcomp[i]->stat_sum_rectified += (dig_value * -1.0);
          }
          else
          {
            signalcomp[i]->stat_sum_rectified += dig_value;
          }

          if(s==signalcomp[i]->sample_start)
          {
            if(dig_value < 0.0)
            {
              stat_zero_crossing = 0;
            }
            else
            {
              stat_zero_crossing = 1;
            }
          }
          else
          {
            if(dig_value < 0.0)
            {
              if(stat_zero_crossing)
              {
                stat_zero_crossing = 0;

                signalcomp[i]->stat_zero_crossing_cnt++;
              }
            }
            else
            {
              if(!stat_zero_crossing)
              {
                stat_zero_crossing = 1;

                signalcomp[i]->stat_zero_crossing_cnt++;
              }
            }
          }
        }

        if(((int)dig_value)>signalcomp[i]->max_dig_value)  signalcomp[i]->max_dig_value = dig_value;
        if(((int)dig_value)<signalcomp[i]->min_dig_value)  signalcomp[i]->min_dig_value = dig_value;

        if(printing)
        {
          value = baseline - value + (int)(signalcomp[i]->screen_offset * printsize_y_factor);
        }
        else
        {
          value = baseline - value + signalcomp[i]->screen_offset;
        }

        if(s>=signalcomp[i]->sample_start)
        {
          x1 = (int)((double)s / signalcomp[i]->sample_pixel_ratio);
          y1 = signalcomp[i]->oldvalue;
          x2 = (int)(((double)(s + 1)) / signalcomp[i]->sample_pixel_ratio);
          y2 = value;

          if(signalcomp[i]->samples_on_screen < w)
          {
            if(linear_interpol)
            {
              x1 = (int)(((double)(s - 1)) / signalcomp[i]->sample_pixel_ratio);
              x2 = (int)((double)s / signalcomp[i]->sample_pixel_ratio);
              qDebug()<<"viewCurve"<<__LINE__;
              graphicBuf[screensamples[i]].graphicLine[i].x1 = x1 - signalcomp[i]->pixels_shift;
              graphicBuf[screensamples[i]].graphicLine[i].y1 = y1;
              graphicBuf[screensamples[i]].graphicLine[i].x2 = x2 - signalcomp[i]->pixels_shift;
              graphicBuf[screensamples[i]].graphicLine[i].y2 = y2;
            }
            else
            {
              graphicBuf[screensamples[i]].graphicLine[i].x1 = x1 - signalcomp[i]->pixels_shift;
              graphicBuf[screensamples[i]].graphicLine[i].y1 = y2;
              graphicBuf[screensamples[i]].graphicLine[i].x2 = x2 - signalcomp[i]->pixels_shift;
              graphicBuf[screensamples[i]].graphicLine[i].y2 = y2;

              if(screensamples[i])
              {
                screensamples[i]++;

                graphicBuf[screensamples[i]].graphicLine[i].x1 = x1 - signalcomp[i]->pixels_shift;
                graphicBuf[screensamples[i]].graphicLine[i].y1 = y1;
                graphicBuf[screensamples[i]].graphicLine[i].x2 = x1 - signalcomp[i]->pixels_shift;
                graphicBuf[screensamples[i]].graphicLine[i].y2 = y2;
              }
            }

            screensamples[i]++;
          }
          else
          {
            if(!screensamples[i])
            {
              graphicBuf[screensamples[i]].graphicLine[i].x1 = x1;
              graphicBuf[screensamples[i]].graphicLine[i].y1 = y1;
              graphicBuf[screensamples[i]].graphicLine[i].x2 = x2;
              graphicBuf[screensamples[i]].graphicLine[i].y2 = y2;

              screensamples[i]++;
            }
            else
            {
              if((x1==x2)&&(x1==graphicBuf[screensamples[i]-1].graphicLine[i].x1)&&
                (graphicBuf[screensamples[i]-1].graphicLine[i].x1==graphicBuf[screensamples[i]-1].graphicLine[i].x2))
              {
                maximum = y1;
                if(y2>maximum)  maximum = y2;
                if(graphicBuf[screensamples[i]-1].graphicLine[i].y1>maximum)  maximum = graphicBuf[screensamples[i]-1].graphicLine[i].y1;
                if(graphicBuf[screensamples[i]-1].graphicLine[i].y2>maximum)  maximum = graphicBuf[screensamples[i]-1].graphicLine[i].y2;

                minimum = y1;
                if(y2<minimum)  minimum = y2;
                if(graphicBuf[screensamples[i]-1].graphicLine[i].y1<minimum)  minimum = graphicBuf[screensamples[i]-1].graphicLine[i].y1;
                if(graphicBuf[screensamples[i]-1].graphicLine[i].y2<minimum)  minimum = graphicBuf[screensamples[i]-1].graphicLine[i].y2;

                graphicBuf[screensamples[i]-1].graphicLine[i].y1 = maximum;
                graphicBuf[screensamples[i]-1].graphicLine[i].y2 = minimum;
              }
              else
              {
                graphicBuf[screensamples[i]].graphicLine[i].x1 = x1;
                graphicBuf[screensamples[i]].graphicLine[i].y1 = y1;
                graphicBuf[screensamples[i]].graphicLine[i].x2 = x2;
                graphicBuf[screensamples[i]].graphicLine[i].y2 = y2;

                if(screensamples[i]<(w * 2))  screensamples[i]++;
              }
            }
          }
        }

        signalcomp[i]->oldvalue = value;


      }
    } // end for(i=0; i<signalcomps; i++)

}
