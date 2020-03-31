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




#ifndef HRV_DIALOGFORM1_H
#define HRV_DIALOGFORM1_H



#include <QtGlobal>
#include <QApplication>
#include <QObject>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSlider>
#include <QProgressDialog>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "mainwindow.h"
#include "ecg_filter.h"
#include "signalonecurve.h"
#include "signaltwocurve.h"
#include "edf_annot_list.h"
#include "ecg_statistics.h"


class UI_Mainwindow;


class UI_HrvWindow : public QObject
{
  Q_OBJECT

public:

UI_HrvWindow(struct signalcompblock *,
                   long long,
                   QWidget *,
                   struct annotation_list *annot_list,
                   struct annotationblock *annot, int second = 0);
~UI_HrvWindow();

UI_Mainwindow *mainwindow;
static QString secondToFormat(int second);
void changedRange();

private:

QDialog     *StatDialog;

QVBoxLayout *vlayoutTotal;

QHBoxLayout *hlayoutTotal;
QHBoxLayout *hlayoutButton;

QHBoxLayout *hlayoutRight,
            *hlayout1_1_1,
            *hlayoutMin,*hlayoutMax, *hlayoutStartTime,*hlayoutEndTime;
QLabel *labelMin, *labelMax, *labelStartTime, *labelEndTime, *labelStartTimeValue, *labelEndTimeValue, *heartrateMinValue, *heartrateMaxValue;

QVBoxLayout *vlayout1_1,
            *vlayout2_1;

QVBoxLayout *vlayoutLeft;
QLabel *labelSelectRange, *labelAnalysisResult;
QVBoxLayout *vLayoutRight;

QSlider     *startSlider,
            *stopSlider,
            *startTimeSlider,
            *endTimeSlider;

QLabel      *Label1,
            *startLabel,
            *stopLabel;



QPushButton  *pushButtonClose, *pushButtonExport;

SignalOneCurve    *curveRight;
SignalTwoCurve *curveLeft;

int beat_cnt, second,
    bpm_distribution[300],


    max_val,
    start_ruler,
    end_ruler;
double *beat_interval_list = NULL;
float *heartRateList;
struct ecg_hr_statistics_struct hr_stat;

int beat_from, beat_to;
 QDialog *sidemenu;

private slots:
void exec_sidemenu();
void print_to_ascii();
void print_to_printer();
void print_to_pdf();
void print_to_image();


void startSliderMoved(int);


void stopSliderMoved(int);

void startTimeSliderMoved(int);
void _startTimeSliderMouseRelase();
void endTimeSliderMoved(int);
void _endTimeSliderMouseRelase();

void exportButtonClicked();

};



#endif // HRV_DIALOGFORM1_H


