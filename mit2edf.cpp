/*
***************************************************************************
*
* Author: Teunis van Beelen
*
* Copyright (C) 2014 - 2019 Teunis van Beelen
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




#include "mit2edf.h"


#define NOTQRS  0 /* not-QRS (not a getann/putann code) */
#define NORMAL  1 /* normal beat */
#define LBBB  2 /* left bundle branch block beat */
#define RBBB  3 /* right bundle branch block beat */
#define ABERR 4 /* aberrated atrial premature beat */
#define PVC 5 /* premature ventricular contraction */
#define FUSION  6 /* fusion of ventricular and normal beat */
#define NPC 7 /* nodal (junctional) premature beat */
#define APC 8 /* atrial premature contraction */
#define SVPB  9 /* premature or ectopic supraventricular beat */
#define VESC  10  /* ventricular escape beat */
#define NESC  11  /* nodal (junctional) escape beat */
#define PACE  12  /* paced beat */
#define UNKNOWN 13  /* unclassifiable beat */
#define NOISE 14  /* signal quality change */
#define ARFCT 16  /* isolated QRS-like artifact */
#define STCH  18  /* ST change */
#define TCH 19  /* T-wave change */
#define SYSTOLE 20  /* systole */
#define DIASTOLE 21 /* diastole */
#define NOTE  22  /* comment annotation */
#define MEASURE 23  /* measurement annotation */
#define PWAVE 24  /* P-wave peak */
#define BBB 25  /* left or right bundle branch block */
#define PACESP  26  /* non-conducted pacer spike */
#define TWAVE 27  /* T-wave peak */
#define RHYTHM  28  /* rhythm change */
#define UWAVE 29  /* U-wave peak */
#define LEARN 30  /* learning */
#define FLWAV 31  /* ventricular flutter wave */
#define VFON  32  /* start of ventricular flutter/fibrillation */
#define VFOFF 33  /* end of ventricular flutter/fibrillation */
#define AESC  34  /* atrial escape beat */
#define SVESC 35  /* supraventricular escape beat */
#define LINK    36  /* link to external data (aux contains URL) */
#define NAPC  37  /* non-conducted P-wave (blocked APB) */
#define PFUS  38  /* fusion of paced and normal beat */
#define WFON  39  /* waveform onset */
#define PQ  WFON  /* PQ junction (beginning of QRS) */
#define WFOFF 40  /* waveform end */
#define JPT WFOFF /* J point (end of QRS) */
#define RONT  41  /* R-on-T premature ventricular contraction */

/* ... annotation codes between RONT+1 and ACMAX inclusive are user-defined */

#define ACMAX 49  /* value of largest valid annot code (must be < 50) */


static char annotdescrlist[42][48]=
  {"not-QRS","normal beat",
  "left bundle branch block beat", "right bundle branch block beat",
  "aberrated atrial premature beat", "premature ventricular contraction",
  "fusion of ventricular and normal beat", "nodal (junctional) premature beat",
  "atrial premature contraction", "premature or ectopic supraventricular beat",
  "ventricular escape beat", "nodal (junctional) escape beat",
  "paced beat", "unclassifiable beat",
  "signal quality change", "isolated QRS-like artifact",
  "ST change", "T-wave change",
  "systole", "diastole",
  "comment annotation", "measurement annotation",
  "P-wave peak", "left or right bundle branch block",
  "non-conducted pacer spike", "T-wave peak",
  "rhythm change", "U-wave peak",
  "learning", "ventricular flutter wave",
  "start of ventricular flutter/fibrillation", "end of ventricular flutter/fibrillation",
  "atrial escape beat", "supraventricular escape beat",
  "link to external data (aux contains URL)", "non-conducted P-wave (blocked APB)",
  "fusion of paced and normal beat", "waveform onset",
  "waveform end", "R-on-T premature ventricular contraction"};


#define ANNOT_EXT_CNT   (9)


static char annotextlist[ANNOT_EXT_CNT][16]=
  {
    ".ari",
    ".ecg",
    ".trigger",
    ".qrs",
    ".atr",
    ".apn",
    ".st",
    ".pwave",
    ".marker"
  };


UI_MIT2EDFwindow::UI_MIT2EDFwindow(char *recent_dir, char *save_dir)
{
  char txt_string[2048];

  recent_opendir = recent_dir;
  recent_savedir = save_dir;

  myobjectDialog = new QDialog;

  myobjectDialog->setMinimumSize(600, 480);
  myobjectDialog->setMaximumSize(600, 480);
  myobjectDialog->setWindowTitle("MIT to EDF+ converter");
  myobjectDialog->setModal(true);
  myobjectDialog->setAttribute(Qt::WA_DeleteOnClose, true);

  pushButton1 = new QPushButton(myobjectDialog);
  pushButton1->setGeometry(20, 430, 100, 25);
  pushButton1->setText("Select File");

  pushButton2 = new QPushButton(myobjectDialog);
  pushButton2->setGeometry(480, 430, 100, 25);
  pushButton2->setText("Close");

  textEdit1 = new QTextEdit(myobjectDialog);
  textEdit1->setGeometry(20, 20, 560, 380);
  textEdit1->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  textEdit1->setReadOnly(true);
  textEdit1->setLineWrapMode(QTextEdit::NoWrap);
  sprintf(txt_string, "MIT (PhysioBank) to EDF+ converter.\n");
  textEdit1->append(txt_string);

  QObject::connect(pushButton1, SIGNAL(clicked()), this, SLOT(SelectFileButton()));
  QObject::connect(pushButton2, SIGNAL(clicked()), myobjectDialog, SLOT(close()));

  myobjectDialog->exec();
}

void UI_MIT2EDFwindow::showOpen()
{
    isConverted = false;
    SelectFileButton();
}

void UI_MIT2EDFwindow::SelectFileButton()
{
  FILE *header_inputfile=NULL,
       *data_inputfile=NULL,
       *annot_inputfile=NULL;

  int i, j, p, len, hdl, *buf;

  char header_filename[MAX_PATH_LENGTH],
       //record_filename[MAX_PATH_LENGTH],
       //edf_result_filename[MAX_PATH_LENGTH],
       txt_string[2048],
       edf_filename[MAX_PATH_LENGTH],
       data_filename[MAX_PATH_LENGTH],
       annot_filename[MAX_PATH_LENGTH],
       filename_x[MAX_PATH_LENGTH],
       scratchpad[4096],
       *charpntr;

  unsigned char a_buf[128],
                ch_tmp;

  long long filesize;

  union {
    int one;
    unsigned char four[4];
  } var;

  pushButton1->setEnabled(false);

  strcpy(header_filename, QFileDialog::getOpenFileName(0, "Select inputfile", QString::fromLocal8Bit(recent_opendir), "MIT header files (*.hea *.HEA)").toLocal8Bit().data());

  if(!strcmp(header_filename, ""))
  {
    pushButton1->setEnabled(true);
    return;
  }

  get_directory_from_path(recent_opendir, header_filename, MAX_PATH_LENGTH);

  header_inputfile = fopeno(header_filename, "rb");
  if(header_inputfile==NULL)
  {
    snprintf(txt_string, 2048, "Can not open file %s for reading.\n", header_filename);
    textEdit1->append(QString::fromLocal8Bit(txt_string));
    pushButton1->setEnabled(true);
    return;
  }

  get_filename_from_path(filename_x, header_filename, MAX_PATH_LENGTH);

  snprintf(txt_string, 2048, "Read file: %s", filename_x);
  textEdit1->append(QString::fromLocal8Bit(txt_string));

  remove_extension_from_filename(filename_x);

  charpntr = fgets(scratchpad, 4095, header_inputfile);
  if(charpntr == NULL)
  {
    textEdit1->append("Can not read header file. (error 1)\n");
    fclose(header_inputfile);
    pushButton1->setEnabled(true);
    return;
  }

  len = strlen(charpntr);
  if(len < 6)
  {
    textEdit1->append("Can not read header file. (error 2)\n");
    fclose(header_inputfile);
    pushButton1->setEnabled(true);
    return;
  }

  for(i=0; i<len; i++)
  {
    if(charpntr[i] == ' ')
    {
      charpntr[i] = 0;

      break;
    }
  }

  if(i == len)
  {
    textEdit1->append("Can not read header file. (error 3)\n");
    fclose(header_inputfile);
    pushButton1->setEnabled(true);
    return;
  }

  if(strcmp(charpntr, filename_x))
  {
    snprintf(txt_string, 2048, "Can not read header file. (error 4), %s, %s\n", charpntr, filename_x);
    textEdit1->append(QString::fromLocal8Bit(txt_string));
    fclose(header_inputfile);
    pushButton1->setEnabled(true);
    return;
  }

  p = ++i;

  for(; i<len; i++)
  {
    if(charpntr[i] == ' ')
    {
      charpntr[i] = 0;

      break;
    }
  }

  if(i == p)
  {
    textEdit1->append("Can not read header file. (error 5)\n");
    fclose(header_inputfile);
    pushButton1->setEnabled(true);
    return;
  }

  mit_hdr.chns = atoi(charpntr + p);

  if(mit_hdr.chns < 1)
  {
    textEdit1->append("Error, number of signals is less than one. (error 6)\n");
    fclose(header_inputfile);
    pushButton1->setEnabled(true);
    return;
  }

  if(mit_hdr.chns > MAXSIGNALS)
  {
    textEdit1->append("Error, Too many signals in header. (error 7)\n");
    fclose(header_inputfile);
    pushButton1->setEnabled(true);
    return;
  }

  p = ++i;

  for(; i<len; i++)
  {
    if(charpntr[i] == ' ')
    {
      charpntr[i] = 0;

      break;
    }
  }

  if(i == p)
  {
    textEdit1->append("Can not read header file. (error 8)\n");
    fclose(header_inputfile);
    pushButton1->setEnabled(true);
    return;
  }

  mit_hdr.sf = atoi(charpntr + p);
  snprintf(txt_string, 2048, "mit_hdr.sf = %d\n", mit_hdr.sf);
  textEdit1->append(txt_string);

  if(mit_hdr.sf < 1)
  {
    textEdit1->append("Error, samplefrequency is less than 1 Hz. (error 9)\n");
    fclose(header_inputfile);
    pushButton1->setEnabled(true);
    return;
  }

  if(mit_hdr.sf > 100000)
  {
    textEdit1->append("Error, samplefrequency is more than 100000 Hz. (error 10)\n");
    fclose(header_inputfile);
    pushButton1->setEnabled(true);
    return;
  }

  mit_hdr.smp_period = 1000000000LL / mit_hdr.sf;

  strcat(filename_x, ".dat");

  for(j=0; j<mit_hdr.chns; j++)
  {
    mit_hdr.adc_gain[j] = 200.0;

    mit_hdr.adc_resolution[j] = 12;

    mit_hdr.adc_zero[j] = 0;

    mit_hdr.init_val[j] = 0;

    mit_hdr.baseline[j] = 0;
    snprintf(txt_string, 2048, "baseline = %f",mit_hdr.baseline[j]);
    mit_hdr.unit_multiplier[j] = 1;  /* default 1 milliVolt */

    strcpy(mit_hdr.unit[j], "mV");

    sprintf(mit_hdr.label[j], "chan. %i", j + 1);

    charpntr = fgets(scratchpad, 4095, header_inputfile);
    if(charpntr == NULL)
    {
      textEdit1->append("Can not read header file. (error 11)\n");
      fclose(header_inputfile);
      pushButton1->setEnabled(true);
      return;
    }

    len = strlen(charpntr);
    if(len < 6)
    {
      textEdit1->append("Can not read header file. (error 12)\n");
      fclose(header_inputfile);
      pushButton1->setEnabled(true);
      return;
    }

    for(i=0; i<len; i++)
    {
      if(charpntr[i] == ' ')
      {
        charpntr[i] = 0;

        break;
      }
    }

    if(i == len)
    {
      textEdit1->append("Can not read header file. (error 13)\n");
      fclose(header_inputfile);
      pushButton1->setEnabled(true);
      return;
    }

    if(strcmp(charpntr, filename_x))
    {
      textEdit1->append("Error, filenames are different. (error 14)\n");
      fclose(header_inputfile);
      pushButton1->setEnabled(true);
      return;
    }

    p = ++i;

    for(; i<len; i++)
    {
      if(charpntr[i] == ' ')
      {
        charpntr[i] = 0;

        break;
      }
    }

    if(i == len)
    {
      textEdit1->append("Can not read header file. (error 15)\n");
      fclose(header_inputfile);
      pushButton1->setEnabled(true);
      return;
    }

    mit_hdr.format[j] = atoi(charpntr + p);

    if((mit_hdr.format[j] != 212) &&
      (mit_hdr.format[j] != 16) &&
      (mit_hdr.format[j] != 32) &&
      (mit_hdr.format[j] != 61))
    {
      snprintf(txt_string, 2048, "Error, unsupported format: %i  (error 16)\n", mit_hdr.format[j]);
      textEdit1->append(txt_string);
      fclose(header_inputfile);
      pushButton1->setEnabled(true);
      return;
    }

    if(j>0)
    {
      if(mit_hdr.format[j] != mit_hdr.format[0])
      {
        textEdit1->append("Error, different formats in the same file. (error 17)\n");
        fclose(header_inputfile);
        pushButton1->setEnabled(true);
        return;
      }
    }

    p = ++i;

    for(ch_tmp=0; i<len; i++)
    {
      if((charpntr[i] == ' ') || (charpntr[i] == '(') || (charpntr[i] == '/'))
      {
        ch_tmp = charpntr[i];

        charpntr[i] = 0;

        break;
      }
    }

    if(i == p)
    {
      continue;
    }

    if(atoi(charpntr + p) != 0)
    {
      mit_hdr.adc_gain[j] = atof(charpntr + p);
    }
    else if(charpntr[p + 1] == '.')
      {
        mit_hdr.adc_gain[j] = atof(charpntr + p);
      }

    if(mit_hdr.adc_gain[j] < 1e-9)  mit_hdr.adc_gain[j] = 200.0;

    p = ++i;
    if(ch_tmp == '(')
    {
      for(; i<len; i++)
      {
        if(charpntr[i] == ')')
        {
          charpntr[i] = 0;

          break;
        }
      }

      if(i == len)
      {
        textEdit1->append("Can not read header file. (error 18)\n");
        fclose(header_inputfile);
        pushButton1->setEnabled(true);
        return;
      }

      p++;

      mit_hdr.baseline[j] = atoi(charpntr + p);
      p = ++i;
    }

    if((ch_tmp == '/') || (charpntr[i] == '/'))
    {
      for(; i<len; i++)
      {
        if(charpntr[i] == ' ')
        {
          charpntr[i] = 0;

          break;
        }
      }

      if(i == len)
      {
        textEdit1->append("Can not read header file. (error 19)\n");
        fclose(header_inputfile);
        pushButton1->setEnabled(true);
        return;
      }

      p++;

      strncpy(mit_hdr.unit[j], charpntr + p, 8);
      mit_hdr.unit[j][8] = 0;

      p = ++i;
    }
//     else
//     {
//       strcpy(mit_hdr.unit[j], "uV");
//
//       mit_hdr.unit_multiplier[j] = 1000;
//     }

    for(; i<len; i++)
    {
      if(charpntr[i] == ' ')
      {
        charpntr[i] = 0;

        break;
      }
    }

    if(i == p)
    {
      continue;
    }

    mit_hdr.adc_resolution[j] = atoi(charpntr + p);

    p = ++i;

    for(; i<len; i++)
    {
      if(charpntr[i] == ' ')
      {
        charpntr[i] = 0;

        break;
      }
    }

    if(i == p)
    {
      continue;
    }

    mit_hdr.adc_zero[j] = atoi(charpntr + p);
    p = ++i;

    for(; i<len; i++)
    {
      if(charpntr[i] == ' ')
      {
        charpntr[i] = 0;

        break;
      }
    }

    if(i == p)
    {
      continue;
    }

    mit_hdr.init_val[j] = atoi(charpntr + p);

    p = ++i;

    for(; i<len; i++)
    {
      if(charpntr[i] == ' ')
      {
        charpntr[i] = 0;

        break;
      }
    }

    if(i == p)
    {
      continue;
    }

    // skip

    p = ++i;

    for(; i<len; i++)
    {
      if(charpntr[i] == ' ')
      {
        charpntr[i] = 0;

        break;
      }
    }

    if(i == p)
    {
      continue;
    }

    // skip

    p = ++i;

    for(; i<len; i++)
    {
      if((charpntr[i] == '\n') || (charpntr[i] == '\r'))
      {
        charpntr[i] = 0;

        break;
      }
    }

    if(i == p)
    {
      continue;
    }

    strncpy(mit_hdr.label[j], charpntr + p, 16);

    mit_hdr.label[j][16] = 0;
  }

  fclose(header_inputfile);

  strcpy(data_filename, header_filename);

  remove_extension_from_filename(data_filename);

  strcpy(edf_filename, data_filename);

  strcpy(annot_filename, data_filename);

  strcat(data_filename, ".dat");

  strcat(edf_filename, ".edf");

  strcat(annot_filename, ".atr");

  data_inputfile = fopeno(data_filename, "rb");
  if(data_inputfile==NULL)
  {
    snprintf(txt_string, 2048, "Can not open file %s for reading.\n", data_filename);
    textEdit1->append(QString::fromLocal8Bit(txt_string));
    pushButton1->setEnabled(true);
    return;
  }

  get_filename_from_path(filename_x, data_filename, MAX_PATH_LENGTH);

  snprintf(txt_string, 2048, "Read file: %s (format: %i)", filename_x, mit_hdr.format[0]);
  textEdit1->append(QString::fromLocal8Bit(txt_string));

  remove_extension_from_filename(filename_x);

  fseeko(data_inputfile, 0LL, SEEK_END);
  filesize = ftello(data_inputfile);
  if(filesize < (mit_hdr.chns * mit_hdr.sf * 45 / 10))
  {
    textEdit1->append("Error, .dat filesize is too small.\n");
    fclose(data_inputfile);
    pushButton1->setEnabled(true);
    return;
  }

  for(mit_hdr.sf_div=10; mit_hdr.sf_div>0; mit_hdr.sf_div--)
  {
    if(mit_hdr.sf_div == 9)  continue;
    if(mit_hdr.sf_div == 7)  continue;
    if(mit_hdr.sf_div == 6)  continue;
    if(mit_hdr.sf_div == 3)  continue;

    if(!(mit_hdr.sf % mit_hdr.sf_div))  break;
  }

  if(mit_hdr.sf_div < 1)  mit_hdr.sf_div = 1;

  mit_hdr.sf_block = mit_hdr.sf / mit_hdr.sf_div;

  hdl = edfopen_file_writeonly(edf_filename, EDFLIB_FILETYPE_EDFPLUS, mit_hdr.chns);

  if(hdl<0)
  {
    snprintf(txt_string, 2048, "Can not open file %s for writing.\n", edf_filename);
    textEdit1->append(QString::fromLocal8Bit(txt_string));
    fclose(data_inputfile);
    pushButton1->setEnabled(true);
    return;
  }
  //*/

  if(mit_hdr.sf_div == 1)
  {
    if(edf_set_number_of_annotation_signals(hdl, 2))
    {
      textEdit1->append("Error: edf_set_number_of_annotation_signals()\n");
      fclose(data_inputfile);
      edfclose_file(hdl);
      pushButton1->setEnabled(true);
      return;
    }
  }

  for(i=0; i<mit_hdr.chns; i++)
  {
    if(edf_set_samplefrequency(hdl, i, mit_hdr.sf_block))
    {
      textEdit1->append("Error: edf_set_samplefrequency()\n");
      fclose(data_inputfile);
      edfclose_file(hdl);
      pushButton1->setEnabled(true);
      return;
    }
  }

  for(i=0; i<mit_hdr.chns; i++)
  {
    if(edf_set_digital_minimum(hdl, i, -32768))
    {
      textEdit1->append("Error: edf_set_digital_minimum()\n");
      fclose(data_inputfile);
      edfclose_file(hdl);
      pushButton1->setEnabled(true);
      return;
    }
  }

  for(i=0; i<mit_hdr.chns; i++)
  {
    if(edf_set_digital_maximum(hdl, i, 32767))
    {
      textEdit1->append("Error: edf_set_digital_maximum()\n");
      fclose(data_inputfile);
      edfclose_file(hdl);
      pushButton1->setEnabled(true);
      return;
    }
  }

  for(i=0; i<mit_hdr.chns; i++)
  {
    if(edf_set_label(hdl, i, mit_hdr.label[i]))
    {
      textEdit1->append("Error: edf_set_label()\n");
      fclose(data_inputfile);
      edfclose_file(hdl);
      pushButton1->setEnabled(true);
      return;
    }
  }


  for(i=0; i<mit_hdr.chns; i++)
  {
    if(!strcmp(mit_hdr.unit[i], "mV"))
    {
      if(((double)((32767 - mit_hdr.adc_zero[i]) * mit_hdr.unit_multiplier[i]) / mit_hdr.adc_gain[i]) <= 100)
      {
        if(((double)((-32768 - mit_hdr.adc_zero[i]) * mit_hdr.unit_multiplier[i]) / mit_hdr.adc_gain[i]) >= -100)
        {
          strcpy(mit_hdr.unit[i], "uV");

          mit_hdr.unit_multiplier[i] *= 1000;
        }
      }
    }


//     printf("physmax: %f    physmin: %f  adcgain: %f\n",
//       (double)((32767 - mit_hdr.adc_zero[i]) * mit_hdr.unit_multiplier[i]) / mit_hdr.adc_gain[i],
//       (double)((-32768 - mit_hdr.adc_zero[i]) * mit_hdr.unit_multiplier[i]) / mit_hdr.adc_gain[i],
//       mit_hdr.adc_gain[i]);

    if(edf_set_physical_maximum(hdl, i, (double)((32767 - mit_hdr.adc_zero[i]) * mit_hdr.unit_multiplier[i]) / mit_hdr.adc_gain[i]))
    {
      textEdit1->append("Error: edf_set_physical_maximum()\n");
      fclose(data_inputfile);
      edfclose_file(hdl);
      pushButton1->setEnabled(true);
      return;
    }

    if(edf_set_physical_minimum(hdl, i, (double)((-32768 - mit_hdr.adc_zero[i]) * mit_hdr.unit_multiplier[i]) / mit_hdr.adc_gain[i]))
    {
      textEdit1->append("Error: edf_set_physical_minimum()\n");
      fclose(data_inputfile);
      edfclose_file(hdl);
      pushButton1->setEnabled(true);
      return;
    }

    if(edf_set_physical_dimension(hdl, i, mit_hdr.unit[i]))
    {
      textEdit1->append("Error: edf_set_physical_dimension()\n");
      fclose(data_inputfile);
      edfclose_file(hdl);
      pushButton1->setEnabled(true);
      return;
    }
  }

  if(edf_set_datarecord_duration(hdl, 100000 / mit_hdr.sf_div))
  {
    textEdit1->append("Error: edf_set_datarecord_duration()\n");
    fclose(data_inputfile);
    edfclose_file(hdl);
    pushButton1->setEnabled(true);
    return;
  }

  buf = (int *)malloc(mit_hdr.sf_block * mit_hdr.chns * sizeof(int));
  if(buf == NULL)
  {
    textEdit1->append("Malloc() error (buf)\n");
    fclose(data_inputfile);
    edfclose_file(hdl);
    pushButton1->setEnabled(true);
    return;
  }
  //*/
/////////////////// Start conversion //////////////////////////////////////////

  int k, blocks, tmp1, tmp2, l_end=0;


  fseeko(data_inputfile, 0LL, SEEK_SET);

  blocks = filesize / (mit_hdr.sf_block * mit_hdr.chns);

  QProgressDialog progress("Converting digitized signals ...", "Abort", 0, blocks);
  progress.setWindowModality(Qt::WindowModal);
  progress.setMinimumDuration(200);

  if(mit_hdr.format[0] == 212)
  {
    blocks *= 10;
    blocks /= 15;

    progress.setMaximum(blocks);

    int oddEven = 0;

    for(k=0; k<blocks; k++)
    {
      if(!(k % 100))
      {
        progress.setValue(k);

        qApp->processEvents();

        if(progress.wasCanceled() == true)
        {
          textEdit1->append("Conversion aborted by user.\n");
          fclose(data_inputfile);
          edfclose_file(hdl);
          free(buf);
          pushButton1->setEnabled(true);
          return;
        }
      }

      for(i=0; i<mit_hdr.sf_block; i++)
      {
        for(j=0; j<mit_hdr.chns; j++)
        {
          if((oddEven++) % 2)
          {
            tmp1 = fgetc(data_inputfile);
            tmp2 = fgetc(data_inputfile);

            if(tmp2 == EOF)
            {
              goto OUT;
            }

            buf[j * mit_hdr.sf_block + i] = (tmp1 & 0xf0) << 4;
            buf[j * mit_hdr.sf_block + i] += tmp2;
            if(buf[j * mit_hdr.sf_block + i] & 0x800)
            {
              buf[j * mit_hdr.sf_block + i] |= 0xfffff000;
            }
          }
          else
          {
            tmp1 = fgetc(data_inputfile);
            tmp2 = fgetc(data_inputfile);

            buf[j * mit_hdr.sf_block + i] = (tmp2 & 0x0f) << 8;
            buf[j * mit_hdr.sf_block + i] += tmp1;
            if(buf[j * mit_hdr.sf_block + i] & 0x800)
            {
              buf[j * mit_hdr.sf_block + i] |= 0xfffff000;
            }

            fseeko(data_inputfile, -1LL, SEEK_CUR);
          }
        }// mit_hdr.chns j (1)
      } // mit_hdr.sf_block i (25)

      if(edf_blockwrite_digital_samples(hdl, buf))
      {
        progress.reset();
        textEdit1->append("A write error occurred during conversion.\n");
        fclose(data_inputfile);
        edfclose_file(hdl);
        free(buf);
        pushButton1->setEnabled(true);
        return;
      }
    }//blocks - k
  }

  if((mit_hdr.format[0] == 16) || (mit_hdr.format[0] == 61))
  {
    if(mit_hdr.format[0] == 16)  l_end = 1;

    blocks /= 2;

    progress.setMaximum(blocks);

    for(k=0; k<blocks; k++)
    {
      if(!(k % 100))
      {
        progress.setValue(k);

        qApp->processEvents();

        if(progress.wasCanceled() == true)
        {
          textEdit1->append("Conversion aborted by user.\n");
          fclose(data_inputfile);
          edfclose_file(hdl);
          free(buf);
          pushButton1->setEnabled(true);
          return;
        }
      }

      for(i=0; i<mit_hdr.sf_block; i++)
      {
        for(j=0; j<mit_hdr.chns; j++)
        {
          tmp1 = fgetc(data_inputfile);
          if(tmp1 == EOF)
          {
            goto OUT;
          }

          if(l_end)
          {
            tmp1 += (fgetc(data_inputfile) << 8);
          }
          else
          {
            tmp1 <<= 8;

            tmp1 += fgetc(data_inputfile);
          }

          if(tmp1 & 0x8000)
          {
            tmp1 |= 0xffff0000;
          }

          buf[j * mit_hdr.sf_block + i] = tmp1;
        }
      }

      if(edf_blockwrite_digital_samples(hdl, buf))
      {
        progress.reset();
        textEdit1->append("A write error occurred during conversion.\n");
        fclose(data_inputfile);
        edfclose_file(hdl);
        free(buf);
        pushButton1->setEnabled(true);
        return;
      }
    }
  }

  if(mit_hdr.format[0] == 32)
  {
    blocks /= 4;

    progress.setMaximum(blocks);

    for(k=0; k<blocks; k++)
    {
      if(!(k % 100))
      {
        progress.setValue(k);

        qApp->processEvents();

        if(progress.wasCanceled() == true)
        {
          textEdit1->append("Conversion aborted by user.\n");
          fclose(data_inputfile);
          edfclose_file(hdl);
          free(buf);
          pushButton1->setEnabled(true);
          return;
        }
      }

      for(i=0; i<mit_hdr.sf_block; i++)
      {
        for(j=0; j<mit_hdr.chns; j++)
        {
          tmp1 = fgetc(data_inputfile);
          if(tmp1 == EOF)
          {
            goto OUT;
          }

          var.four[0] = tmp1;
          var.four[1] = fgetc(data_inputfile);
          var.four[2] = fgetc(data_inputfile);
          var.four[3] = fgetc(data_inputfile);

          buf[j * mit_hdr.sf_block + i] = var.one;
        }
      }

      if(edf_blockwrite_digital_samples(hdl, buf))
      {
        progress.reset();
        textEdit1->append("A write error occurred during conversion.\n");
        fclose(data_inputfile);
        edfclose_file(hdl);
        free(buf);
        pushButton1->setEnabled(true);
        return;
      }
    }
  }

OUT:

  progress.reset();

  qApp->processEvents();

/////////////////// End conversion //////////////////////////////////////////

  fclose(data_inputfile);

  free(buf);

  int annot_code, tc, skip, total_annots=0;

  long long bytes_read;

  get_filename_from_path(filename_x, annot_filename, MAX_PATH_LENGTH);

  for(k=0; k<ANNOT_EXT_CNT; k++)
  {
    tc = 0;

    remove_extension_from_filename(annot_filename);

    strcat(annot_filename, annotextlist[k]);

    annot_inputfile = fopeno(annot_filename, "rb");
    if(annot_inputfile==NULL)
    {
      continue;
    }

    get_filename_from_path(filename_x, annot_filename, MAX_PATH_LENGTH);

    if(annot_inputfile==NULL)
    {
      snprintf(txt_string, 2048, "Can not open file %s for reading.\n"
                                 "Annotations can not be included.", filename_x);
      textEdit1->append(QString::fromLocal8Bit(txt_string));
    }
    else
    {
      snprintf(txt_string, 2048, "Read file: %s", filename_x);
      textEdit1->append(QString::fromLocal8Bit(txt_string));

      fseeko(annot_inputfile, 0LL, SEEK_END);
      filesize = ftello(annot_inputfile);

      progress.setLabelText("Converting annotations ...");
      progress.setMinimum(0);
      progress.setMaximum(filesize);

      fseeko(annot_inputfile, 0LL, SEEK_SET);

      for(bytes_read=0LL; bytes_read < filesize; bytes_read += 2LL)
      {
        if(!(bytes_read % 100))
        {
          progress.setValue(bytes_read);

          qApp->processEvents();

          if(progress.wasCanceled() == true)
          {
            textEdit1->append("Conversion aborted by user.\n");

            break;
          }
        }

        skip = 0;

        if(fread(a_buf, 2, 1, annot_inputfile) != 1)
        {
          break;
        }

  #pragma GCC diagnostic ignored "-Wstrict-aliasing"

        if(*((unsigned short *)a_buf) == 0)  // end of file
        {
          break;
        }

        annot_code = a_buf[1] >> 2;

        if(annot_code == 59)
        {
          if(fread(a_buf, 4, 1, annot_inputfile) != 1)
          {
            break;
          }

          tc += (*((unsigned short *)a_buf) << 16);

          tc += *((unsigned short *)(a_buf + 2));
        }
        else if(annot_code == 63)
          {
            skip = *((unsigned short *)a_buf) & 0x3ff;

            if(skip % 2) skip++;
          }
          else if((annot_code >= 0) && (annot_code <= ACMAX))
            {
              tc += *((unsigned short *)a_buf) & 0x3ff;

  #pragma GCC diagnostic warning "-Wstrict-aliasing"

              if(annot_code < 42)
              {
                edfwrite_annotation_latin1(hdl, ((long long)tc * mit_hdr.smp_period) / 100000LL, -1, annotdescrlist[annot_code]);
              }
              else
              {
                edfwrite_annotation_latin1(hdl, ((long long)tc * mit_hdr.smp_period) / 100000LL, -1, "user-defined");
              }

              total_annots++;
            }

        if(skip)
        {
          if(fseek(annot_inputfile, skip, SEEK_CUR) < 0)
          {
            break;
          }

          bytes_read += skip;
        }
      }

      fclose(annot_inputfile);
    }
  }

  if(total_annots)
  {
    snprintf(txt_string, 2048, "Read %i annotations.", total_annots);
    textEdit1->append(txt_string);
  }


  progress.reset();

  edfclose_file(hdl);

  textEdit1->append("Ready.\n");

  pushButton1->setEnabled(true);

  strcpy(convertedEdfFilePath, edf_filename);
  isConverted = true;
  myobjectDialog->close();
}
















//int UI_MIT2EDFwindow::wfdb_mit2edf(char *record, char *ofname)

//{
//    char txt_string[2048];

//    char buf[100];
//    char *header, *p, *block, **blockp;
//    char *pname = "wfdb_mit2edf";
//    double *pmax, *pmin, frames_per_second, seconds_per_block;
//    FILE *ofile = NULL;
//    int *dmax, *dmin, i, j, k, nsig, samples_per_frame, start_date_recorded,
//    edfplusflag = 0, vflag = 0, day, month, year, hour, minute, second;
//    long bytes_per_block, frames_per_block, n, nblocks, blocks_per_minute,
//    blocks_per_hour;
//    WFDB_Sample *v, *vp;
//    WFDB_Siginfo *si;

//    static char *month_name[] = {  "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
//                   "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };

//    /* Quit if no record was specified. */

//    if (record == NULL) {
//        snprintf(txt_string, 2048, "Please specify record\n");
//        textEdit1->append(QString::fromLocal8Bit(txt_string));
//        pushButton1->setEnabled(true);
//        return;
//    }

//    /* Construct or validate the name of the output file. */
//    if (ofname == NULL) {
//    for (p = record + strlen(record); p > record; p--)
//        if (*(p-1) == '/') break;
//    strncpy(buf, p, WFDB_MAXRNL);
//    strcat(buf, ".edf");
//    ofname = buf;
//    }
//    else if (edfplusflag) {
//    p = ofname + strlen(ofname) - 4;
//    if (strcmp(p, ".edf") && strcmp(p, ".EDF")) {
//        (void)fprintf(stderr, "%s: '%s' is not a valid EDF+ file name\n",
//              pname, ofname);
//        (void)fprintf(stderr,
//                 " EDF+ file names must end with '.edf' or '.EDF')\n");

//        exit(1);
//    }
//    }
//  int test = 90;

//    /* Open the input record. */
//    if ((nsig = isigopen(record, NULL, 0)) <= 0) {
//        snprintf(txt_string, 2048, "Can not open file %s for reading.\n", record);
//        textEdit1->append(QString::fromLocal8Bit(txt_string));
//        pushButton1->setEnabled(true);
//        return;
//    }
//    if ((si = malloc(nsig * sizeof(WFDB_Siginfo))) == NULL) {
//    (void)fprintf(stderr, "%s: insufficient memory\n", pname);
//        snprintf(txt_string, 2048, "%s insufficient memory\n", pname);
//        textEdit1->append(QString::fromLocal8Bit(txt_string));
//        pushButton1->setEnabled(true);
//        return;
//    // exit(2);
//    }
//    if ((nsig = isigopen(record, si, nsig)) <= 0){
//        snprintf(txt_string, 2048, "Can not open file %s for reading 3.\n", record);
//        textEdit1->append(QString::fromLocal8Bit(txt_string));
//        return;
//        //exit(3);
//    }



//    /* Open the output (EDF) file. */
//    if ((ofile = fopen(ofname, "wb")) == NULL) {
//    (void)fprintf(stderr, "%s: can't create %s\n", pname, ofname);
//        snprintf(txt_string, 2048, "Can create file %s for reading 3.\n", ofname);
//        textEdit1->append(QString::fromLocal8Bit(txt_string));
//        return;
//    //exit(4);
//    }

//    /* Get start date and time */
//    p = timstr(0L);
//    start_date_recorded = 1;
//    if (*p != '[') {	/* start date/time not recorded -- use current date */
//    setbasetime(NULL);
//    p = timstr(0L);
//    start_date_recorded = 0;
//    }
//    i = sscanf(p, "[%d:%d:%d %d/%d/%d",
//           &hour, &minute, &second, &day, &month, &year);
//    if (i != 6) {		/* start time, but not date, recorded */
//    start_date_recorded = 0;
//    day = month = 1;
//    year = 1985;		/* beginning of EDF epoch */
//    }

//    /* Calculate block duration.  (In the EDF spec, blocks are called "records"
//       or "data records", but this would be confusing here since "record"
//       refers to the entire recording -- so here we say "blocks".) */
//    for (i = samples_per_frame = 0; i < nsig; i++)
//    samples_per_frame += si[i].spf;
//    frames_per_second = strtim("1:0")/60.0;	/* i.e., the number of frames
//                           per minute, divided by 60 */
//    frames_per_block = 10 * frames_per_second + 0.5;	/* ten seconds */
//    bytes_per_block = 2 * samples_per_frame * frames_per_block;
//                       /* EDF specifies 2 bytes per sample */
//    while (bytes_per_block > EDFMAXBLOCK) {
//    /* blocks would be too long -- reduce their length by a factor of 10 */
//    frames_per_block /= 10;
//    bytes_per_block = samples_per_frame * 2 * frames_per_block;
//    }

//    seconds_per_block = frames_per_block / frames_per_second;

//    if (frames_per_block < 1 && bytes_per_block < EDFMAXBLOCK/60) {
//    frames_per_block = strtim("1:0");     /* the number of frames/minute */
//    bytes_per_block = 2* samples_per_frame * frames_per_block;
//    seconds_per_block = 60;
//    }

//    if (bytes_per_block > EDFMAXBLOCK) {
//    fprintf(stderr, "%s: can't convert record %s to EDF\n", pname, record);
//    fprintf(stderr,
// " EDF blocks cannot be larger than %d bytes, but each input frame requires\n",
//        EDFMAXBLOCK);
//    fprintf(stderr,
// " %d bytes.  Use 'snip' to select a subset of the input signals, or use\n"
//        " 'xform' to reduce the sampling frequency.\n",
//        samples_per_frame * 2);
//    //exit(5);
//    snprintf(txt_string, 2048, "Can create file %s for reading 5.\n", ofname);
//    textEdit1->append(QString::fromLocal8Bit(txt_string));
//    return;
//    }


//    /* Calculate the number of blocks to be written.  strtim("e") is the frame
//       number of the last frame in the record, and that of the first frame is
//       0, so the number of frames is strtim("e") + 1.  The calculation rounds
//       up so that we don't lose any frames, even if the number of frames is not
//       an exact multiple of frames_per_block. */
//    nblocks = strtim("e") / frames_per_block + 1;

//    /* Allocate and initialize arrays and buffers. */
//    if ((dmax = malloc(nsig * sizeof(int))) == NULL ||
//    (dmin = malloc(nsig * sizeof(int))) == NULL ||
//    (pmax = malloc(nsig * sizeof(double))) == NULL ||
//    (pmin = malloc(nsig * sizeof(double))) == NULL ||
//    (header = malloc((nsig + 1) * 256)) == NULL ||
//    (v = malloc(samples_per_frame * sizeof(WFDB_Sample))) == NULL ||
//    (block = malloc(bytes_per_block)) == NULL ||
//    (blockp = malloc(nsig * sizeof(char *))) == NULL) {
//    (void)fprintf(stderr, "%s: insufficient memory\n", pname);
//        textEdit1->append(QString::fromLocal8Bit("insufficient memory"));
//        return ;
//    //exit(2);
//    }
//    for (i = 0; i < (nsig + 1)*256; i++)
//        header[i] = ' ';

//    if (vflag)
//    printf("Converting record %s to %s (%s mode)\n", record, ofname,
//           edfplusflag == 1 ? "EDF+" : "EDF");

//    /* Calculate physical and digital extrema. */
//    for (i = 0; i < nsig; i++) {
//    if (si[i].adcres < 1) {	/* invalid ADC resolution in input .hea file */
//        switch (si[i].fmt) { /* guess ADC resolution based on format */
//          case 24: si[i].adcres = 24; break;
//          case 32: si[i].adcres = 32; break;
//          case 80: si[i].adcres = 8; break;
//          case 212: si[i].adcres = 12; break;
//          case 310:
//          case 311: si[i].adcres = 10; break;
//          default: si[i].adcres = 16; break;
//        }
//    }
//    dmax[i] = si[i].adczero + (1 << (si[i].adcres - 1)) - 1;
//    dmin[i] = si[i].adczero - (1 << (si[i].adcres - 1));
//    pmax[i] = aduphys(i, dmax[i]);
//    pmin[i] = aduphys(i, dmin[i]);
//    }

//    /* Start filling in the header.  The first line of comments above each
//       entry is as given in the EDF technical specification
//       (http://www.hsr.nl/edf/edf_spec.htm). */
//    p = header;

//    /* Version of this data format (0). */
//    strncpy(p, "0", 1);
//    p += 8;

//    /* Local patient identification. */
//    if (strlen(record) > 80) record[79] = '\0';
//    strncpy(p, record, strlen(record));
//    p += 80;

//    /* Local recording identification.

//       Bob Kemp recommends using this field to encode the start date including
//       an abbreviated month name in English and a full (4-digit) year, as is
//       done here if this information is available in the input record.  EDF+
//       requires this. */

//    if (start_date_recorded)
//    sprintf(buf, "Startdate %02d-%s-%d", day, month_name[month-1], year);
//    else {
//    sprintf(buf, "Startdate not recorded");
//    if (edfplusflag)
//        fprintf(stderr,
//                "WARNING (%s): EDF+ requires start date (not specified)\n",
//            pname);
//    }
//    strncpy(p, buf, strlen(buf));
//    p += 80;

//    /* Start date of recording (dd.mm.yy). */
//    sprintf(buf, "%02d.%02d.%02d", day, month, year % 100);
//    strncpy(p, buf, 8);
//    p += 8;

//    /* Start time of recording (hh.mm.ss). */
//    sprintf(buf, "%02d.%02d.%02d", hour, minute, second);
//    strncpy(p, buf, 8);
//    p += 8;

//    /* Number of bytes in header. */
//    sprintf(buf, "%ld", (nsig + 1)*256L);
//    strncpy(p, buf, strlen(buf));
//    p += 8;

//    /* Reserved. */
//    if (edfplusflag)
//    strncpy(p, "EDF+C", 5);
//    p += 44;

//    /* Number of blocks (-1 if unknown). */
//    sprintf(buf, "%ld", nblocks);
//    strncpy(p, buf, strlen(buf));
//    p += 8;

//    /* Duration of a block, in seconds. */
//    sprintf(buf, "%g", seconds_per_block);
//    if (strlen(buf) > 8) buf[8] = '\0';
//    strncpy(p, buf, strlen(buf));
//    p += 8;

//    /* Number of signals. */
//    sprintf(buf, "%d", nsig);
//    strncpy(p, buf, strlen(buf));
//    p += 4;

//    /* Label (e.g., EEG FpzCz or Body temp). */
//    for (i = 0; i < nsig; i++, p += 16) {
//    if (strlen(si[i].desc) > 16) si[i].desc[16] = '\0';
//    strncpy(p, si[i].desc, strlen(si[i].desc));
//    }

//    /* Transducer type (e.g., AgAgCl electrode). */
//    for (i = 0; i < nsig; i++, p += 80) {
//    strncpy(p, "transducer type not recorded",
//        strlen("transducer type not recorded"));
//    }

//    /* Physical dimension (e.g., uV or degreeC). */
//    for (i = 0; i < nsig; i++, p += 8) {
//    if (si[i].units == NULL) si[i].units = "mV";
//    else if (strlen(si[i].units) > 8) si[i].units[8] = '\0';
//    strncpy(p, si[i].units, strlen(si[i].units));
//    }

//    /* Physical minimum (e.g., -500 or 34). */
//    for (i = 0; i < nsig; i++, p += 8) {
//    sprintf(buf, "%g", pmin[i]);
//    strncpy(p, buf, strlen(buf));
//    }

//    /* Physical maximum (e.g., 500 or 40). */
//    for (i = 0; i < nsig; i++, p += 8) {
//    sprintf(buf, "%g", pmax[i]);
//    strncpy(p, buf, strlen(buf));
//    }

//    /* Digital minimum (e.g., -2048). */
//    for (i = 0; i < nsig; i++, p += 8) {
//    sprintf(buf, "%d", dmin[i]);
//    strncpy(p, buf, strlen(buf));
//    }

//    /* Digital maximum (e.g., 2047). */
//    for (i = 0; i < nsig; i++, p += 8) {
//    sprintf(buf, "%d", dmax[i]);
//    strncpy(p, buf, strlen(buf));
//    }

//    /* Prefiltering (e.g., HP:0.1Hz LP:75Hz). */
//    for (i = 0; i < nsig; i++, p += 80) {
//    strncpy(p, "prefiltering not recorded",
//        strlen("prefiltering not recorded"));
//    }

//    /* Number of samples per block. */
//    for (i = 0; i < nsig; i++, p += 8) {
//    sprintf(buf, "%ld", frames_per_block * si[i].spf);
//    strncpy(p, buf, strlen(buf));
//    }
//    /* (The last 32*nsig bytes in the header are unused.) */

//    /* Write the header to the output file. */
//    fwrite(header, 1, (nsig+1) * 256, ofile);
//    /* Check that all characters in the header are valid (printable ASCII
//       between 32 and 126 inclusive).  Note that this test does not prevent
//       generation of files containing invalid characters;  it merely warns
//       the user if this has happened. */
//    for (i = 0; i < (nsig+1) * 256; i++)
//    if (header[i] < 32 || header[i] > 126)
//        fprintf(stderr,
//            "WARNING (%s): output contains an invalid character, %d,"
//            " at byte %d\n", pname, header[i], i);

//    /* In verbose mode, summarize what we've done so far. */
//    if (vflag) {
//    printf(" Header block size: %d bytes\n", (nsig+1) * 256);
//    printf(" Data block size: %g second%s (%ld frame%s or %ld bytes)\n",
//           seconds_per_block, seconds_per_block == 1.0 ? "" : "s",
//           frames_per_block, frames_per_block == 1 ? "" : "s",
//           bytes_per_block);
//    for (p = timstr(strtim("e")); *p == ' '; p++)
//         ;
//    printf(" Recording length: %s"
//           " (%ld data blocks, %ld frames, %ld bytes)\n",
//           p, nblocks, nblocks*frames_per_block, nblocks*bytes_per_block);
//    printf(" Total length of file to be written: %ld bytes\n",
//           (nsig+1)*256 + nblocks*bytes_per_block);

//    blocks_per_minute = (long)(60 / seconds_per_block);
//    blocks_per_hour = (long)60 * blocks_per_minute;
//    }
//    /* Write the data blocks. */
//    for (n = 1; n <= nblocks; n++) {
//    blockp[0] = block;
//    for (j = 1; j < nsig; j++)
//        blockp[j] = blockp[j-1] + 2 * frames_per_block * si[j-1].spf;
//    for (i = 0; i < frames_per_block; i++) {
//        if (nsig != getframe(v)) {
//        /* end of input: pad last block with invalid samples */
//        for (j = 0; j < samples_per_frame; j++)
//            v[j] = WFDB_INVALID_SAMPLE;
//        }
//        vp = v;
//        for (j = 0; j < nsig; j++) {
//        for (k = 0; k < si[j].spf; k++) {
//            *(blockp[j]++) = (*vp) & 0xff;
//            *(blockp[j]++) = ((*vp++ >> 8) & 0xff);
//        }
//        }
//    }
//    fwrite(block, 1, bytes_per_block, ofile);
//    if (vflag) {
//        if (n % blocks_per_minute == 0) { printf("."); fflush(stdout); }
//        if (n % blocks_per_hour == 0) printf("\n");
//    }
//    }
//    (void)fclose(ofile);
//    printf("\n");

//    if (edfplusflag) {
//    fprintf(stderr,
//"\nWARNING (%s): EDF+ requires the subject's gender, birthdate, and name, as\n"
//" well as additional information about the recording that is not usually\n"
//" available.  This information is not saved in the output file even if\n"
//" available.  EDF+ also requires the use of standard names for signals and\n"
//" for physical units;  these requirements are not enforced by this program.\n"
//" To make the output file fully EDF+ compliant, its header must be edited\n"
//" manually.\n",
//        pname);
//    for (i = nsig-1; i >= 0; i--)
//        if (strcmp(si[i].desc, "EDF-Annotations") == 0)
//        break;
//    if (i < 0)
//        fprintf(stderr,
//"\nWARNING:  The output file does not include EDF annotations, which are\n"
//            " required for EDF+.\n");
//    }

//    textEdit1->append(QString::fromLocal8Bit("complete"));

//    wfdbquit();
//    //exit(0);
//}
