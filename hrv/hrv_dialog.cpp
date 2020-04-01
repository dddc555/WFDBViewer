/*
***************************************************************************
*
* Author: Teunis van Beelen
*
* Copyright (C) 2011 - 2019 Teunis van Beelen
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




#include "hrv_dialog.h".h"


/* 200 bpm x 60 min. x 24 hours x 2 days */
#define BEAT_IVAL_LIST_SZ   (576000)




UI_HrvWindow::UI_HrvWindow(struct signalcompblock *signalcomp,
                           long long pagetime,
                           QWidget *w_parent,
                           struct annotation_list *annot_list,
                           struct annotationblock *annot, int _second)
{
    mainwindow = (UI_Mainwindow *)w_parent;
    this->second = _second;

    int i,
            tmp,
            //       NN20,
            //       pNN20,
            //       NN50,
            //       pNN50,
            err;

    char stat_str[2048]={""};



    //   double d_tmp,
    //          average_bpm,
    //          average_rr,
    //          sdnn_bpm,
    //          sdnn_rr,
    //          *buf_bpm=NULL,
    //          rmssd_rr;

    long long l_tmp=0;

    struct annotationblock *tmp_annot=NULL;

    StatDialog = new QDialog;
    StatDialog->setWindowTitle(tr("HRV Analysis"));
    StatDialog->setModal(true);
    StatDialog->setAttribute(Qt::WA_DeleteOnClose, true);
    StatDialog->setWindowFlags(Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    StatDialog->setWindowIcon(QIcon(":/images/edf.png"));

    beat_interval_list = (double *)malloc(sizeof(double) * BEAT_IVAL_LIST_SZ);

    if(beat_interval_list == NULL)
    {
        QMessageBox messagewindow(QMessageBox::Critical, "Error", "The system was not able to provide enough resources (memory) to perform the requested action.");
        messagewindow.exec();
        return;
    }


    StatDialog->setMinimumSize(1200, 400);
    StatDialog->setSizeGripEnabled(true);

    curveLeft = new SignalTwoCurve(StatDialog, second);
    curveLeft->setSignalColor(QColor(0x4a,0x98,0x15));//QColor(0x70,0xAD,0x47)
    curveLeft->setSignal2Color(QColor(0x44, 0x72, 0xC4));

    curveLeft->setBackgroundColor(Qt::black);
    curveLeft->setRasterColor(QColor(0xa9,0xa9,0xa9, 70));//Qt::gray;
    curveLeft->setTraceWidth(0);

    //    curveLeft->setLowerLabel("HR (beats/min)");

    strcpy(stat_str, "Distribution ");
    strcat(stat_str, annot->annotation);
    //    curveLeft->setUpperLabel1(stat_str);

    curveLeft->setFillSurfaceEnabled(false);
    // end curve2

    vlayoutLeft= new QVBoxLayout;
    vlayoutLeft->setSpacing(20);

    labelSelectRange = new QLabel;
    labelSelectRange->setText(tr("Select Range"));
    vlayoutLeft->addWidget(labelSelectRange, 1);
    vlayoutLeft->addWidget(curveLeft, 100);

    startTimeSlider = new QSlider;
    startTimeSlider->setOrientation(Qt::Horizontal);
    startTimeSlider->setMinimum(0);
    //    startTimeSlider->setMaximum(295);
    startTimeSlider->setValue(0);

    endTimeSlider = new QSlider;
    endTimeSlider->setOrientation(Qt::Horizontal);
    //    endTimeSlider->setMinimum(5);

    //    int totalTime = this->mainwindow->
    labelStartTimeValue = new QLabel;
    labelStartTimeValue->setText("00:00");
    labelEndTimeValue = new QLabel;

    QString  str = SignalTwoCurve::secondToFormat(second);
    qDebug()<<"hours forma"<<str;

    labelEndTimeValue->setText(str);

    heartrateMinValue = new QLabel;
    heartrateMinValue->setText("0 bpm");
    heartrateMaxValue = new QLabel;
    heartrateMaxValue->setText("0 bpm");

    hlayoutStartTime = new QHBoxLayout;

    labelStartTime = new QLabel(tr("Start Time"));
    labelStartTime->setFixedWidth(57);
    hlayoutStartTime->setSpacing(10);
    hlayoutStartTime->addWidget(labelStartTime);
    hlayoutStartTime->addWidget(labelStartTimeValue);
    hlayoutStartTime->addWidget(startTimeSlider);

    hlayoutEndTime = new QHBoxLayout;
    labelEndTime = new QLabel(tr("End Time"));
    hlayoutEndTime->setSpacing(10);
    hlayoutEndTime->addWidget(labelEndTime);
    hlayoutEndTime->addWidget(labelEndTimeValue);
    hlayoutEndTime->addWidget(endTimeSlider);

    labelEndTime->setFixedWidth(57);


    startSlider = new QSlider;
    startSlider->setOrientation(Qt::Horizontal);
    startSlider->setMinimum(0);
    startSlider->setMaximum(295);
    startSlider->setValue(0);

    stopSlider = new QSlider;
    stopSlider->setOrientation(Qt::Horizontal);
    stopSlider->setMinimum(5);
    stopSlider->setMaximum(300);
    stopSlider->setValue(300);

    curveRight = new SignalOneCurve(StatDialog);
    curveRight->setSignalColor(Qt::darkGreen);
    curveRight->setBackgroundColor(Qt::black);
    curveRight->setRasterColor(Qt::gray);
    curveRight->setTraceWidth(0);

    curveRight->setLowerLabel("HR (beats/min)");
    curveRight->setDashBoardEnabled(false);


    strcpy(stat_str, "Distribution ");
    strcat(stat_str, annot->annotation);
    curveRight->setUpperLabel1(stat_str);

    curveRight->setFillSurfaceEnabled(true);
    // end curveRight

    labelAnalysisResult = new QLabel;
    labelAnalysisResult->setText(tr("Analysis Result"));


    vlayout2_1 = new QVBoxLayout;
    vlayout2_1->setSpacing(20);
    vlayout2_1->addWidget(curveRight);

    QSizePolicy spOne(QSizePolicy::Preferred, QSizePolicy::Preferred);
    QSizePolicy spTwo(QSizePolicy::Preferred, QSizePolicy::Preferred);
    QSizePolicy spThree(QSizePolicy::Preferred, QSizePolicy::Preferred);
    spOne.setHorizontalStretch(1);
    spTwo.setHorizontalStretch(1);
    spThree.setHorizontalStretch(5);

    hlayoutMin = new QHBoxLayout;
    labelMin = new QLabel(tr("Min"));

    //    labelMin->setSizePolicy(spOne);
    //    heartrateMinValue->setSizePolicy(spTwo);
    //    startSlider->setSizePolicy(spThree);

    hlayoutMin->addWidget(labelMin);
    hlayoutMin->addWidget(heartrateMinValue);
    hlayoutMin->addWidget(startSlider);

    hlayoutMax = new QHBoxLayout;
    labelMax = new QLabel(tr("Max"));
    hlayoutMax->addWidget(labelMax);
    hlayoutMax->addWidget(heartrateMaxValue);
    hlayoutMax->addWidget(stopSlider);

    vlayout2_1->addLayout(hlayoutMin);
    vlayout2_1->addLayout(hlayoutMax);


    Label1 = new QLabel(StatDialog);
    Label1->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    pushButtonClose = new QPushButton(StatDialog);
    pushButtonClose->setMinimumSize(100, 25);
    pushButtonClose->setText(tr("Close"));

    pushButtonExport = new QPushButton(StatDialog);
    pushButtonExport->setMinimumSize(100, 25);
    pushButtonExport->setText(tr("Export"));

    hlayout1_1_1 = new QHBoxLayout;

    hlayout1_1_1->addStretch(100);

    vlayout1_1 = new QVBoxLayout;
    vlayout1_1->setSpacing(20);
    vlayout1_1->addWidget(Label1);
    vlayout1_1->addStretch(100);
    vlayout1_1->addLayout(hlayout1_1_1);

    hlayoutRight = new QHBoxLayout;

    hlayoutRight->addLayout(vlayout1_1, 1);
    hlayoutRight->addLayout(vlayout2_1, 100);

    vLayoutRight = new QVBoxLayout;
    vLayoutRight->setSpacing(20);
    vLayoutRight->addWidget(labelAnalysisResult);
    vLayoutRight->addLayout(hlayoutRight);

    hlayoutTotal = new QHBoxLayout;


    vlayoutLeft->addLayout(hlayoutStartTime);
    vlayoutLeft->addLayout(hlayoutEndTime);

    hlayoutTotal->addLayout(vlayoutLeft, 1);
    hlayoutTotal->addLayout(vLayoutRight, 1);

    vlayoutTotal = new QVBoxLayout;

    StatDialog->setLayout(vlayoutTotal);

    vlayoutTotal->addLayout(hlayoutTotal);
    hlayoutButton = new QHBoxLayout;

    QLabel *labelRemain = new QLabel();
    labelRemain->setText(" ");

    hlayoutButton->setSpacing(20);

    hlayoutButton->addWidget(pushButtonClose, 1);
    hlayoutButton->addWidget(pushButtonExport, 1);

    hlayoutButton->addWidget(labelRemain, 100);

    vlayoutTotal->addLayout(hlayoutButton);

    QObject::connect(pushButtonClose, SIGNAL(clicked()), StatDialog, SLOT(close()));
    QObject::connect(pushButtonExport, SIGNAL(clicked()), this, SLOT(exportButtonClicked()));

    //
    QProgressDialog progress("Processing...", "Abort", 0, 1);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(200);
    progress.reset();

    progress.setRange(0, edfplus_annotation_size(annot_list));
    progress.setValue(0);

    int p_i=0, p_j=0;

    for(i=0, beat_cnt=0; beat_cnt<BEAT_IVAL_LIST_SZ; i+=1)
    {
        tmp_annot = edfplus_annotation_get_item_visible_only_cached(annot_list, i, &p_i, &p_j);

        if(tmp_annot == NULL)  break;

        if(tmp_annot->onset - l_tmp > 0)
        {
            if(beat_cnt)
            {
                beat_interval_list[beat_cnt - 1] = ((double)(tmp_annot->onset - l_tmp)) / (double)TIME_DIMENSION;
            }

            l_tmp = tmp_annot->onset;

            beat_cnt++;
        }

        if(!(i%1000))
        {
            progress.setValue(i);

            qApp->processEvents();

            if(progress.wasCanceled() == true)
            {
                break;
            }
        }
    }

    if(beat_cnt)  beat_cnt--;

    progress.reset();

    if(beat_cnt < 3)
    {
        sprintf(stat_str, "Not enough beats.");
    }
    else
    {
        for(int i = 0;i<beat_cnt;i++) {
            if(beat_interval_list[i] < 0.2f) beat_interval_list[i] = 0.2f;
        }

        heartRateList = (float *)malloc(sizeof(float) * beat_cnt);

        for(int i = 0;i<beat_cnt;i++){
            heartRateList[i] = 60.0f / beat_interval_list[i];
            if(heartRateList[i]>300) {
                heartRateList[i] = 300;
            }
        }
        beat_from = 0;
        beat_to = beat_cnt;

        startTimeSlider->setMaximum(beat_cnt);
        endTimeSlider->setMaximum(beat_cnt);
        endTimeSlider->setValue(beat_cnt);

        //draw rri/heart rate
        float min1 = heartRateList[beat_from];
        float max1 = heartRateList[beat_from];

        float min2 = beat_interval_list[beat_from];
        float max2 = beat_interval_list[beat_from];

        for(i = beat_from; i < beat_to; i++) {
            if(min1 > heartRateList[i]) min1 = heartRateList[i];
            if(max1 < heartRateList[i]) max1 = heartRateList[i];

            if(min2 > beat_interval_list[i]) min2 = beat_interval_list[i];
            if(max2 < beat_interval_list[i]) max2 = beat_interval_list[i];
        }
        max1 = fmax(200, max1);
        min1 = fmin(0, min1);

        max2 = fmax(2, max2);
        min2 = fmin(0.2, min2);


        QString str;

        str.sprintf("%d bpm", (int)min1);
        heartrateMinValue ->setText(str);
        str.sprintf("%d bpm", (int)max1);
        heartrateMaxValue ->setText(str);

        float range1 = max1 - min1;
        float range2 = max2 - min2;
        float multiple = range1 / range2;

        float *rriList = (float *)malloc(sizeof(float) * beat_cnt);
        for(int i = 0;i < beat_cnt; i++) {
            rriList[i] = beat_interval_list[i] * multiple;
        }

        curveLeft->setLineEnabled(true);


        curveLeft->drawCurve(heartRateList, rriList, beat_to - beat_from, (int)(max1 * 1.1) + 1, min1, (int)(max2 * 1.1) + 1, min2);
        // end
        changedRange();

        QObject::connect(startSlider, SIGNAL(valueChanged(int)), this, SLOT(startSliderMoved(int)));
        QObject::connect(stopSlider,  SIGNAL(valueChanged(int)), this, SLOT(stopSliderMoved(int)));

        QObject::connect(startTimeSlider,  SIGNAL(sliderReleased()), this, SLOT(_startTimeSliderMouseRelase()));
        QObject::connect(endTimeSlider,  SIGNAL(sliderReleased()), this, SLOT(_endTimeSliderMouseRelase()));

    }

    strcpy(mainwindow->toolbar_stats.annot_label, annot->annotation);

    mainwindow->toolbar_stats.annot_list = annot_list;

    mainwindow->toolbar_stats.sz = 0;

    mainwindow->toolbar_stats.active = 1;


    StatDialog->exec();
}


void UI_HrvWindow::exec_sidemenu()
{
    sidemenu = new QDialog(StatDialog);
    sidemenu ->setWindowFlags(sidemenu ->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    sidemenu->setMinimumSize(120, 160);
    sidemenu->setMaximumSize(120, 160);

    sidemenu->setWindowTitle(tr("Export"));
    sidemenu->setModal(true);
    sidemenu->setAttribute(Qt::WA_DeleteOnClose, true);

    QPushButton *sidemenuButton1 = new QPushButton(sidemenu);
    sidemenuButton1->setGeometry(10, 10, 100, 20);
    sidemenuButton1->setText(tr("to printer"));

#if QT_VERSION < 0x050000
    QPushButton *sidemenuButton2 = new QPushButton(sidemenu);
    sidemenuButton2->setGeometry(10, 40, 100, 20);
    sidemenuButton2->setText("to Postscript");
#endif

    QPushButton *sidemenuButton3 = new QPushButton(sidemenu);
    sidemenuButton3->setGeometry(10, 70, 100, 20);
    sidemenuButton3->setText(tr("to PDF"));

    QPushButton *sidemenuButton4 = new QPushButton(sidemenu);
    sidemenuButton4->setGeometry(10, 100, 100, 20);
    sidemenuButton4->setText(tr("to Image"));

    QPushButton* sidemenuButton5 = new QPushButton(sidemenu);
    sidemenuButton5->setGeometry(10, 130, 100, 20);
    sidemenuButton5->setText(tr("to CSV"));
    QPushButton *sidemenuButton6;

    QObject::connect(sidemenuButton1, SIGNAL(clicked()), this, SLOT(print_to_printer()));

    QObject::connect(sidemenuButton3, SIGNAL(clicked()), this, SLOT(print_to_pdf()));
    QObject::connect(sidemenuButton4, SIGNAL(clicked()), this, SLOT(print_to_image()));
    QObject::connect(sidemenuButton5, SIGNAL(clicked()), this, SLOT(print_to_ascii()));

    sidemenu->exec();
}

void UI_HrvWindow::print_to_printer(){
    QPrinter curve_printer(QPrinter::HighResolution);

    curve_printer.setOutputFormat(QPrinter::NativeFormat);
    curve_printer.setPageSize(QPrinter::A4);
    curve_printer.setOrientation(QPrinter::Landscape);

    QPrintDialog printerdialog(&curve_printer, StatDialog);
    printerdialog.setWindowTitle(tr("Print"));

    if(!(printerdialog.exec()==QDialog::Accepted))
    {
        return;
    }

}

void UI_HrvWindow::print_to_pdf(){
    char path[SC_MAX_PATH_LEN];

    path[0] = 0;
    if(mainwindow->recent_savedir[0]!=0)
    {
        strcpy(path, mainwindow->recent_savedir);
        strcat(path, "/");
    }
    strcat(path, "hrv.pdf");

    strcpy(path, QFileDialog::getSaveFileName(0, "Print to PDF", QString::fromLocal8Bit(path), "PDF files (*.pdf *.PDF)").toLocal8Bit().data());

    if(!strcmp(path, ""))
    {

        return;
    }
    get_directory_from_path(mainwindow->recent_savedir, path, SC_MAX_PATH_LEN);
    QPrinter curve_printer(QPrinter::HighResolution);

    curve_printer.setOutputFormat(QPrinter::PdfFormat);
    curve_printer.setOutputFileName(path);
    curve_printer.setPageSize(QPrinter::A4);
    curve_printer.setOrientation(QPrinter::Landscape);

    curveLeft->print_to_pdf(&curve_printer);
//    curveRight->print_to_pdf(&curve_printer);
}

void UI_HrvWindow::print_to_image(){

    char path[SC_MAX_PATH_LEN];

    path[0] = 0;
    if(mainwindow->recent_savedir[0]!=0)
    {
        strcpy(path, mainwindow->recent_savedir);
        strcat(path, "/");
    }
    strcat(path, "hrv.png");

    strcpy(path, QFileDialog::getSaveFileName(0, tr("Print to Image"), QString::fromLocal8Bit(path), "PNG files (*.png *.PNG)").toLocal8Bit().data());

    if(!strcmp(path, ""))
    {
        sidemenu->close();

        return;
    }

    get_directory_from_path(mainwindow->recent_savedir, path, SC_MAX_PATH_LEN);

    QPixmap pixmap2 = curveRight->print_to_image();
    QPixmap pixmap = curveLeft->print_to_image();

    int height = pixmap.height();
    if( height < pixmap2.height() ) height = pixmap2.height();

    //    QPixmap pixmapTotal(pixmap.width() + pixmap2.width(), height);
    //    QPainter painter(&pixmapTotal);
    //    painter.drawPixmap(
    //            QRectF(0, 0, pixmap.width(), pixmap.height()), pixmap,
    //            QRectF(pixmap.width(), 0, pixmap2.width(), pixmap2.height()));

    QPixmap pixmapTotal(pixmap.width() + pixmap2.width(), height);
    QPainter painter(&pixmapTotal);
    painter.drawPixmap(
                QRectF(0, 0, pixmap.width(), pixmap.height()), pixmap,
                QRectF(0, 0, pixmap.width(), pixmap.height()));

    painter.drawPixmap(
                QRectF(pixmap.width(), 0, pixmap2.width(), pixmap2.height()), pixmap2,
                QRectF(pixmap2.rect()));


    qDebug()<<"pixmap"<<pixmap.width()<<pixmap.height()<<pixmap2.width()<<pixmap2.height();
    pixmapTotal.save(path, "PNG", 90);

    sidemenu->close();
}

void UI_HrvWindow::print_to_ascii()
{
    int i;

    char path[SC_MAX_PATH_LEN];

    FILE *outputfile;

    path[0] = 0;
    if(mainwindow->recent_savedir[0]!=0)
    {
        strcpy(path, mainwindow->recent_savedir);
        strcat(path, "/");
    }
    strcat(path, "hrv.csv");

    strcpy(path, QFileDialog::getSaveFileName(0, "Print to ASCII / CSV", QString::fromLocal8Bit(path), "ASCII / CSV files (*.csv *.CSV *.txt *.TXT)").toLocal8Bit().data());

    if(!strcmp(path, ""))
    {
        sidemenu->close();
        return;
    }

    get_directory_from_path(mainwindow->recent_savedir, path, SC_MAX_PATH_LEN);

    outputfile = fopen(path, "wb");
    if(outputfile == NULL)
    {
        QMessageBox messagewindow(QMessageBox::Critical, "Error", "Can not open outputfile for writing.");
        messagewindow.exec();
        return;
    }

    /*
     *  "Beats:    %3i\n\n"
            "Mean RR:  %3.1f ms\n\n"
            "SDNN RR:  %3.1f ms\n\n"
            "RMSSD RR: %3.1f ms\n\n"
            "Mean HR:  %3.1f bpm\n\n"
            "SDNN HR:  %3.1f bpm\n\n"
            "NN20:     %3i\n\n"
            "pNN20:    %3.1f %%\n\n"
            "NN50:     %3i\n\n"
            "pNN50:    %3.1f %%\n\n",
     */
    fprintf(outputfile, "HRV Metric, Value\n");
    fprintf(outputfile, "Beats, %d\n", hr_stat.beat_cnt);
    fprintf(outputfile, "Mean RR(ms), %3.1f\n", hr_stat.mean_rr);
    fprintf(outputfile, "SDNN RR(ms), %3.1f\n", hr_stat.sdnn_rr);
    fprintf(outputfile, "RMSSD RR(ms), %3.1f\n", hr_stat.rmssd_rr);
    fprintf(outputfile, "Mean HR (bpm), %3.1f\n", hr_stat.mean_hr);
    fprintf(outputfile, "SDNN HR (bpm), %3.1f\n", hr_stat.sdnn_hr);
    fprintf(outputfile, "NN20, %d\n", hr_stat.NN20);
    fprintf(outputfile, "pNN20(%), %3.1f\n", hr_stat.pNN20);
    fprintf(outputfile, "NN50, %d\n", hr_stat.NN50);
    fprintf(outputfile, "pNN50(%), %3.1f\n", hr_stat.pNN50);
    fprintf(outputfile, "\n");
    fprintf(outputfile, "HR Distribution, Counts\n");

    int tmp;

    for(i=0; i<300; i++)
    {
        bpm_distribution[i] = 0;
    }


    for(i = beat_from; i < beat_to; i++)
    {
        tmp = 60.0 / beat_interval_list[i];

        if((tmp > 0) && (tmp < 301))
        {
            bpm_distribution[tmp-1]++;
        }
    }
    for(int i = 0;i<300;i++){
        if(bpm_distribution[i] > 0)
            fprintf(outputfile, "%d, %d\n", i, bpm_distribution[i]);
    }

    fclose(outputfile);

    sidemenu->close();
}


void UI_HrvWindow::changedRange() {
    char stat_str[2048]={""};

    int tmp;
    int i;
    for(i=0; i<300; i++)
    {
        bpm_distribution[i] = 0;
    }

    double *sub_beat_interval_list = (double *)malloc(sizeof(double) * beat_to - beat_from);

    for(i = beat_from; i < beat_to; i++) {
        sub_beat_interval_list[i - beat_from] = beat_interval_list[i];
    }
    if(beat_to - beat_from < 3) return;
    int err = ecg_get_hr_statistics(sub_beat_interval_list, beat_to - beat_from, &hr_stat);
    if(err)
    {
        sprintf(stat_str, "Error %i occurred at line %i in file %s.", err, __LINE__, __FILE__);
        QMessageBox messagewindow(QMessageBox::Critical, "Error", stat_str);
        messagewindow.exec();
        return;
    }

    sprintf(stat_str,
            "Heart Rate\n\n"
            "Beats:    %3i\n\n"
            "Mean RR:  %3.1f ms\n\n"
            "SDNN RR:  %3.1f ms\n\n"
            "RMSSD RR: %3.1f ms\n\n"
            "Mean HR:  %3.1f bpm\n\n"
            "SDNN HR:  %3.1f bpm\n\n"
            "NN20:     %3i\n\n"
            "pNN20:    %3.1f %%\n\n"
            "NN50:     %3i\n\n"
            "pNN50:    %3.1f %%\n\n",
            hr_stat.beat_cnt,
            hr_stat.mean_rr,
            hr_stat.sdnn_rr,
            hr_stat.rmssd_rr,
            hr_stat.mean_hr,
            hr_stat.sdnn_hr,
            hr_stat.NN20,
            hr_stat.pNN20,
            hr_stat.NN50,
            hr_stat.pNN50);

    for(i = beat_from; i < beat_to; i++)
    {
        tmp = 60.0 / beat_interval_list[i];

        if((tmp > 0) && (tmp < 301))
        {
            bpm_distribution[tmp-1]++;
        }
    }

    max_val = 1;

    for(i=0; i<300; i++)
    {
        if(bpm_distribution[i] > max_val)
        {
            max_val = bpm_distribution[i];
        }
    }

    for(i=0; i<300; i++)
    {
        if(bpm_distribution[i] > (max_val / 70))
        {
            start_ruler = i;

            break;
        }
    }

    for(i=299; i>=0; i--)
    {
        if(bpm_distribution[i] > (max_val / 70))
        {
            end_ruler = i + 1;

            if(end_ruler > 300)
            {
                end_ruler = 300;
            }

            break;
        }
    }

    if(start_ruler >= end_ruler)
    {
        start_ruler = 0;

        end_ruler = 300;
    }

    startSlider->setValue(start_ruler);
    stopSlider->setValue(end_ruler);

    curveRight->setH_RulerValues(start_ruler + 1, end_ruler + 1);

    curveRight->drawCurve(bpm_distribution + start_ruler, end_ruler - start_ruler, (int)(max_val * 1.1) + 1, 0.0);

    float min1 = heartRateList[beat_from];
    float max1 = heartRateList[beat_from];


    for(i = beat_from; i < beat_to; i++) {
        if(min1 > heartRateList[i]) min1 = heartRateList[i];
        if(max1 < heartRateList[i]) max1 = heartRateList[i];
    }

    QString str;

    str.sprintf("%d bpm", (int)min1);
    heartrateMinValue ->setText(str);
    str.sprintf("%d bpm", (int)max1);
    heartrateMaxValue ->setText(str);

    curveLeft->drawRegion(beat_from, beat_to);

    QString str1 (stat_str);
    str1.replace("Not enough beats.", tr("Not enough beats."));
    str1.replace("Heart Rate", tr("Heart Rate"));
    Label1->setText(str1);
}

UI_HrvWindow::~UI_HrvWindow(){
    qDebug()<<"~UI_HrvWindow()";
    free(beat_interval_list);
}


void UI_HrvWindow::startSliderMoved(int)
{
    startSlider->blockSignals(true);
    stopSlider->blockSignals(true);

    start_ruler = startSlider->value();
    end_ruler = stopSlider->value();

    if(end_ruler < (start_ruler + 5))
    {
        end_ruler = start_ruler + 5;

        stopSlider->setValue(end_ruler);
    }

    curveRight->setH_RulerValues(start_ruler + 1, end_ruler + 1);

    curveRight->drawCurve(bpm_distribution + start_ruler, end_ruler - start_ruler, (int)(max_val * 1.1) + 1, 0.0);

    startSlider->blockSignals(false);
    stopSlider->blockSignals(false);
}

void UI_HrvWindow::_startTimeSliderMouseRelase(){
    startTimeSliderMoved(0);
}

void UI_HrvWindow::stopSliderMoved(int)
{
    startSlider->blockSignals(true);
    stopSlider->blockSignals(true);

    start_ruler = startSlider->value();
    end_ruler = stopSlider->value();

    if(start_ruler > (end_ruler - 5))
    {
        start_ruler = end_ruler - 5;

        startSlider->setValue(start_ruler);
    }

    curveRight->setH_RulerValues(start_ruler + 1, end_ruler + 1);

    curveRight->drawCurve(bpm_distribution + start_ruler, end_ruler - start_ruler, (int)(max_val * 1.1) + 1, 0.0);

    startSlider->blockSignals(false);
    stopSlider->blockSignals(false);
}

void UI_HrvWindow::_endTimeSliderMouseRelase(){
    endTimeSliderMoved(0);
}

void UI_HrvWindow::startTimeSliderMoved(int)
{
    startTimeSlider->blockSignals(true);
    endTimeSlider->blockSignals(true);

    beat_from = startTimeSlider->value();
    beat_to = endTimeSlider->value();

    if(beat_to < (beat_from + 5))
    {
        beat_to = beat_from + 5;
        beat_to = fmin(beat_to, beat_cnt);

        endTimeSlider->setValue(beat_to);
    }
    int currentSecond = this->second * beat_from / beat_cnt;
    int endSecond = this->second * beat_to / beat_cnt;

    labelStartTimeValue->setText(SignalTwoCurve::secondToFormat(currentSecond));
    labelEndTimeValue->setText(SignalTwoCurve::secondToFormat(endSecond));

    curveLeft->drawRegion(beat_from, beat_to);

    startTimeSlider->blockSignals(false);
    endTimeSlider->blockSignals(false);
    changedRange();
}


void UI_HrvWindow::endTimeSliderMoved(int)
{
    startTimeSlider->blockSignals(true);
    endTimeSlider->blockSignals(true);

    beat_from = startTimeSlider->value();
    beat_to = endTimeSlider->value();

    if(beat_from > (beat_to - 5))
    {
        beat_from = beat_to - 5;
        beat_from = fmax(beat_from, 0);

        startTimeSlider->setValue(beat_from);
    }

    int currentSecond = this->second * beat_from / beat_cnt;
    int endSecond = this->second * beat_to / beat_cnt;

    labelStartTimeValue->setText(SignalTwoCurve::secondToFormat(currentSecond));
    labelEndTimeValue->setText(SignalTwoCurve::secondToFormat(endSecond));

    curveLeft->drawRegion(beat_from, beat_to);

    startTimeSlider->blockSignals(false);
    endTimeSlider->blockSignals(false);
    changedRange();
}


void UI_HrvWindow::exportButtonClicked()
{
    qDebug()<<endTimeSlider->maximum();
    qDebug()<<endTimeSlider->value();

    exec_sidemenu();
}




