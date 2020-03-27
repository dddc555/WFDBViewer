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

void UI_Mainwindow::detect_qrs()
{
    if(!initQRS())return;

    struct {
        int chns;
        int sf;
        int sf_div;
        int sf_block;
        long long smp_period;
        int format[MAXSIGNALS];
        double adc_gain[MAXSIGNALS];
        int baseline[MAXSIGNALS];
        int adc_resolution[MAXSIGNALS];
        int adc_zero[MAXSIGNALS];
        int init_val[MAXSIGNALS];
        char label[MAXSIGNALS][17];
        int unit_multiplier[MAXSIGNALS];
        char unit[MAXSIGNALS][9];
    } mit_hdr;

    FILE *header_inputfile=NULL,
            *data_inputfile=NULL,
            *annot_inputfile=NULL;

    int i, j, p, len, *buf;

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

    if(this->selectedWFDBHeaderFilePath == NULL){
        strcpy(header_filename, QFileDialog::getOpenFileName(0, tr("Select input file"), QString::fromLocal8Bit(recent_opendir), "MIT header files (*.hea *.HEA)").toLocal8Bit().data());
    } else{
        strcpy(header_filename, this->selectedWFDBHeaderFilePath);
    }


    if(!strcmp(header_filename, ""))
    {
        return;
    }

    //

    //

    get_directory_from_path(recent_opendir, header_filename, MAX_PATH_LENGTH);

    header_inputfile = fopeno(header_filename, "rb");
    if(header_inputfile==NULL)
    {
        snprintf(txt_string, 2048, "Can not open file %s for reading.\n", header_filename);

        return;
    }

    get_filename_from_path(filename_x, header_filename, MAX_PATH_LENGTH);

    snprintf(txt_string, 2048, "Read file: %s", filename_x);


    remove_extension_from_filename(filename_x);

    charpntr = fgets(scratchpad, 4095, header_inputfile);
    if(charpntr == NULL)
    {
        fclose(header_inputfile);
        return;
    }

    len = strlen(charpntr);
    if(len < 6)
    {
        fclose(header_inputfile);
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
        fclose(header_inputfile);
        return;
    }

    if(strcmp(charpntr, filename_x))
    {
        snprintf(txt_string, 2048, "Can not read header file. (error 4), %s, %s\n", charpntr, filename_x);
        fclose(header_inputfile);
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
        fclose(header_inputfile);
        return;
    }

    mit_hdr.chns = atoi(charpntr + p);

    if(mit_hdr.chns < 1)
    {
        fclose(header_inputfile);
        return;
    }

    if(mit_hdr.chns > MAXSIGNALS)
    {
        fclose(header_inputfile);
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
        //textEdit1->append("Can not read header file. (error 8)\n");
        fclose(header_inputfile);
        //pushButton1->setEnabled(true);
        return;
    }

    mit_hdr.sf = atoi(charpntr + p);
    snprintf(txt_string, 2048, "mit_hdr.sf = %d\n", mit_hdr.sf);
    //textEdit1->append(txt_string);

    if(mit_hdr.sf < 1)
    {
        //textEdit1->append("Error, samplefrequency is less than 1 Hz. (error 9)\n");
        fclose(header_inputfile);
        //pushButton1->setEnabled(true);
        return;
    }

    if(mit_hdr.sf > 100000)
    {
        //textEdit1->append("Error, samplefrequency is more than 100000 Hz. (error 10)\n");
        fclose(header_inputfile);
        //pushButton1->setEnabled(true);
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
            //textEdit1->append("Can not read header file. (error 11)\n");
            fclose(header_inputfile);
            //pushButton1->setEnabled(true);
            return;
        }

        len = strlen(charpntr);
        if(len < 6)
        {
            //textEdit1->append("Can not read header file. (error 12)\n");
            fclose(header_inputfile);
            //pushButton1->setEnabled(true);
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
            //textEdit1->append("Can not read header file. (error 13)\n");
            fclose(header_inputfile);
            //pushButton1->setEnabled(true);
            return;
        }

        if(strcmp(charpntr, filename_x))
        {
            //textEdit1->append("Error, filenames are different. (error 14)\n");
            fclose(header_inputfile);
            //pushButton1->setEnabled(true);
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
            //textEdit1->append("Can not read header file. (error 15)\n");
            fclose(header_inputfile);
            //pushButton1->setEnabled(true);
            return;
        }

        mit_hdr.format[j] = atoi(charpntr + p);

        if((mit_hdr.format[j] != 212) &&
                (mit_hdr.format[j] != 16) &&
                (mit_hdr.format[j] != 32) &&
                (mit_hdr.format[j] != 61))
        {
            snprintf(txt_string, 2048, "Error, unsupported format: %i  (error 16)\n", mit_hdr.format[j]);
            //textEdit1->append(txt_string);
            fclose(header_inputfile);
            //pushButton1->setEnabled(true);
            return;
        }

        if(j>0)
        {
            if(mit_hdr.format[j] != mit_hdr.format[0])
            {
                //textEdit1->append("Error, different formats in the same file. (error 17)\n");
                fclose(header_inputfile);
                //pushButton1->setEnabled(true);
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
                //textEdit1->append("Can not read header file. (error 18)\n");
                fclose(header_inputfile);
                //pushButton1->setEnabled(true);
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
                //textEdit1->append("Can not read header file. (error 19)\n");
                fclose(header_inputfile);
                //pushButton1->setEnabled(true);
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
    QTemporaryFile tempEdfFile;




    strcpy(data_filename, header_filename);

    remove_extension_from_filename(data_filename);

    strcpy(edf_filename, data_filename);

    strcpy(annot_filename, data_filename);

    strcat(data_filename, ".dat");

    strcat(edf_filename, ".edf");
    if(tempEdfFile.open()){
        strcpy(edf_filename, tempEdfFile.fileName().toLocal8Bit().data());
        strcat(edf_filename, ".edf");
        qDebug()<<"edf_filename"<<edf_filename;
    }
    strcat(annot_filename, ".atr");

    data_inputfile = fopeno(data_filename, "rb");
    if(data_inputfile==NULL)
    {
        snprintf(txt_string, 2048, "Can not open file %s for reading.\n", data_filename);
        //textEdit1->append(QString::fromLocal8Bit(txt_string));
        //pushButton1->setEnabled(true);
        return;
    }

    get_filename_from_path(filename_x, data_filename, MAX_PATH_LENGTH);

    snprintf(txt_string, 2048, "Read file: %s (format: %i)", filename_x, mit_hdr.format[0]);
    //textEdit1->append(QString::fromLocal8Bit(txt_string));

    remove_extension_from_filename(filename_x);

    fseeko(data_inputfile, 0LL, SEEK_END);
    filesize = ftello(data_inputfile);
    if(filesize < (mit_hdr.chns * mit_hdr.sf * 45 / 10))
    {
        //textEdit1->append("Error, .dat filesize is too small.\n");
        fclose(data_inputfile);
        //pushButton1->setEnabled(true);
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

    }

    buf = (int *)malloc(mit_hdr.sf_block * mit_hdr.chns * sizeof(int));
    if(buf == NULL)
    {
        //textEdit1->append("Malloc() error (buf)\n");
        fclose(data_inputfile);

        //pushButton1->setEnabled(true);
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
                    //textEdit1->append("Conversion aborted by user.\n");
                    fclose(data_inputfile);

                    free(buf);
                    //pushButton1->setEnabled(true);
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
                            goto OUT2;
                        }

                        buf[j * mit_hdr.sf_block + i] = (tmp1 & 0xf0) << 4;
                        buf[j * mit_hdr.sf_block + i] += tmp2;
                        if(buf[j * mit_hdr.sf_block + i] & 0x800)
                        {
                            buf[j * mit_hdr.sf_block + i] |= 0xfffff000;
                        }

                        if(j == 0) onECGReceived(buf[j * mit_hdr.sf_block + i]);
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
                        if(j == 0) onECGReceived(buf[j * mit_hdr.sf_block + i]);
                        fseeko(data_inputfile, -1LL, SEEK_CUR);
                    }
                }// mit_hdr.chns j (1)
            } // mit_hdr.sf_block i (25)


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
                    //textEdit1->append("Conversion aborted by user.\n");
                    fclose(data_inputfile);

                    free(buf);
                    //pushButton1->setEnabled(true);
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
                        goto OUT2;
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
                    if(j == 0)onECGReceived(buf[j * mit_hdr.sf_block + i]);
                }
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
                    //textEdit1->append("Conversion aborted by user.\n");
                    fclose(data_inputfile);

                    free(buf);
                    //pushButton1->setEnabled(true);
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
                        goto OUT2;
                    }

                    var.four[0] = tmp1;
                    var.four[1] = fgetc(data_inputfile);
                    var.four[2] = fgetc(data_inputfile);
                    var.four[3] = fgetc(data_inputfile);

                    buf[j * mit_hdr.sf_block + i] = var.one;
                }
            }
        }
    }

OUT2:

    progress.reset();

    qApp->processEvents();

    /////////////////// End conversion //////////////////////////////////////////

    fclose(data_inputfile);

    free(buf);
    progress.reset();
    closeQRSFile();
    QMessageBox messagewindow(QMessageBox::Information, tr("Message"), tr("Annotation file is generated with QRS detector"));
    messagewindow.exec();
}

bool UI_Mainwindow::initQRS(){

    char f_path[MAX_PATH_LENGTH], txt_string[MAX_PATH_LENGTH];
    char header_filename[MAX_PATH_LENGTH];
    qDebug()<<"this->selectedWFDBHeaderFilePath == NULL"<<(this->selectedWFDBHeaderFilePath == NULL);
    if(!files_open) {
        QMessageBox messagewindow(QMessageBox::Critical, tr("Error"), tr("Please open WFDB file to anaysis with QRS detector."));
        messagewindow.exec();
        return false;
    }
    if(this->selectedWFDBHeaderFilePath == NULL || !strcmp(selectedWFDBHeaderFilePath, "")){
        strcpy(header_filename, QFileDialog::getOpenFileName(0, tr("Select input file"), QString::fromLocal8Bit(recent_opendir), "MIT header files (*.hea *.HEA)").toLocal8Bit().data());
        strcpy(this->selectedWFDBHeaderFilePath, header_filename);
    } else{
        strcpy(header_filename, this->selectedWFDBHeaderFilePath);
    }

    if(!strcmp(header_filename, ""))
    {
        return false;
    }

    mHeartRate.QRSDetect(0, 1);

    qrs_delay = 0;
    prev_delay = 0;
    sample_count = 0;
    total_sample_count = 0;


    strcpy(f_path, recent_savedir);
    char directory_path[MAX_PATH_LENGTH];

    get_directory_from_path(directory_path, selectedWFDBHeaderFilePath, MAX_PATH_LENGTH);
    if(strlen(directory_path) == 0)
    {
        return false;
    }
    strcat(directory_path, "/");


    char file_name[MAX_PATH_LENGTH];

    get_filename_from_path(file_name, selectedWFDBHeaderFilePath, MAX_PATH_LENGTH);


    strcpy(targetAnnotation_path, directory_path);

    remove_extension_from_filename(file_name);

    strcat(targetAnnotation_path, file_name);
    strcat(targetAnnotation_path, ".atr");
    qDebug()<<"targetAnnotation_path"<<targetAnnotation_path;
    if(QFileInfo(targetAnnotation_path).exists()){
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Warning"),tr("Annotation file exist, are you replace it"),  QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            qDebug() << "Yes was clicked";

        } else {
            return false;
        }
    }
    annotationBeforeTimeIndex = 0;

    annotationOutputfile = fopeno(targetAnnotation_path, "wb");
    return annotationOutputfile != NULL;
}

#define NORMAL_BEAT 1
#define SKIP_BEAT 59


void UI_Mainwindow::onECGReceived(int ecg){

    if(annotationOutputfile == NULL)return;
    int file_num = 0;
    int frequency =  edfheaderlist[file_num]->edfparam->sf_int;

    sample_count ++;
    total_sample_count ++;
    qrs_delay = mHeartRate.QRSDetect(ecg, 0);
    if(qrs_delay > 0) qDebug()<<"qrs_delay "<<qrs_delay;
    if (qrs_delay > 0) // QRS deteted at this time
    {
        if (((sample_count - qrs_delay + prev_delay)) != 0) {

            mHeartRate.hrm_roll((60 * frequency) / (sample_count - qrs_delay + prev_delay) + 1);
            FLOAT hr = mHeartRate.hrm_calc();

            if (MIN_HRM < hr && hr < MAX_HRM) {
                FLOAT rri = 60 / hr;
                //                qDebug()<<"rr = " <<rri;

                int diff = total_sample_count - annotationBeforeTimeIndex - qrs_delay;
                qDebug()<<"total_sample_count - annotationBeforeTimeIndex"<<diff<<qrs_delay;
                int word;
                unsigned char arrByte[10];

                if(diff > 1023){
                    qDebug()<<"skip"<<diff;
                    word = (SKIP_BEAT << 10) + ((0) & 0xFFF); //skip

                    arrByte[0] =  (byte(word & 0x00FF));
                    arrByte[1] = ((byte) ((word & 0xFF00) >> 8));
                    fwrite(arrByte, 2, 1 , annotationOutputfile);

                    word = diff;
                    arrByte[3] = ((byte) ((word & 0xFF00) >> 8));
                    arrByte[2] = ((byte) ((word & 0xFF)));
                    arrByte[1] = ((byte) ((word & 0xff000000) >> 24));
                    arrByte[0] = ((byte) ((word & 0xFF0000) >> 16));
                    fwrite(arrByte, 4, 1 , annotationOutputfile);

                    diff = 0;
                }

                word = (NORMAL_BEAT << 10) + ((diff) & 0xFFF);

                arrByte[0] =  (byte(word & 0x00FF));
                arrByte[1] = ((byte) ((word & 0xFF00) >> 8));
                fwrite(arrByte, 2, 1 , annotationOutputfile);

                annotationBeforeTimeIndex = total_sample_count - qrs_delay;
            } else{
                qDebug()<<"else 2";
            }
        } else{
            qDebug()<<"else 1";
        }

        prev_delay = qrs_delay;
        sample_count = 0;
    }
    //
}

void UI_Mainwindow::closeQRSFile(){
    if(annotationOutputfile == NULL)return;
    unsigned char arrByte[2];
    arrByte[0] = 0;
    arrByte[1] = 0;
    fwrite(arrByte, 2,1,annotationOutputfile);
    fclose(annotationOutputfile);
    qDebug()<<"total_sample_count"<<total_sample_count;
}
