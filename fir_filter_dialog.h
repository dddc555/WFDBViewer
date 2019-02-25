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



#ifndef ADD_FIR_FILTERFORM1_H
#define ADD_FIR_FILTERFORM1_H



#include <QtGlobal>
#include <QApplication>
#include <QObject>
#include <QListWidget>
#include <QListWidgetItem>
#include <QList>
#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QMessageBox>
#include <QVariant>
#include <QPlainTextEdit>
#include <QTextCursor>
#include <QDesktopServices>
#include <QUrl>
#include <QFileDialog>
#include <QToolTip>

#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "mainwindow.h"
#include "fir_filter.h"
#include "utils.h"


#define FIR_FILTER_MAX_TAPS       5000
#define FIR_FILTER_MAX_BUFSZ    100000


class UI_Mainwindow;



class UI_FIRFilterDialog : public QObject
{
  Q_OBJECT

public:
  UI_FIRFilterDialog(char *recent_dir=NULL, char *save_dir=NULL, QWidget *parent=0);

  UI_Mainwindow *mainwindow;


private:

QDialog        *firfilterdialog;

QPushButton    *CancelButton,
               *ApplyButton,
               *helpButton,
               *fileButton;

QListWidget    *list;

QPlainTextEdit *textEdit;

QLabel         *listlabel,
               *varsLabel;

int n_taps;

double taps[FIR_FILTER_MAX_TAPS];

char textbuf[FIR_FILTER_MAX_BUFSZ + 64],
     *recent_opendir,
     *recent_savedir;

private slots:

void ApplyButtonClicked();
void check_text();
void helpbuttonpressed();
void filebuttonpressed();

};



#endif // ADD_FIR_FILTERFORM1_H








