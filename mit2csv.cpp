/*
***************************************************************************
*
* Author: Wang Liaong
*
* Copyright (C) 2020 - 2021 Wang Liaong
*
* Email: wbit85@gmail.com
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



#include "mit2csv.h"



UI_MIT2CSVwindow::UI_MIT2CSVwindow(char *recent_dir, char *save_dir, bool onlyShowFileOpenDialog, char *w_path)
{
  qDebug()<<"0";
  this->wfdb_path = w_path;
  qDebug()<<"1"<<this->wfdb_path<<"w"<<w_path;
  char txt_string[2048];

  recent_opendir = recent_dir;
  recent_savedir = save_dir;

  myobjectDialog = new QDialog;

  myobjectDialog->setMinimumSize(600, 480);
  myobjectDialog->setMaximumSize(600, 480);
  myobjectDialog->setWindowTitle(tr("MIT to CSV converter"));
  myobjectDialog->setModal(true);
  myobjectDialog->setAttribute(Qt::WA_DeleteOnClose, true);

  pushButton1 = new QPushButton(myobjectDialog);
  pushButton1->setGeometry(20, 430, 100, 25);
  pushButton1->setText(tr("Select File"));

  pushButton2 = new QPushButton(myobjectDialog);
  pushButton2->setGeometry(480, 430, 100, 25);
  pushButton2->setText(tr("Close"));

  textEdit1 = new QTextEdit(myobjectDialog);
  textEdit1->setGeometry(20, 20, 560, 380);
  textEdit1->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  textEdit1->setReadOnly(true);
  textEdit1->setLineWrapMode(QTextEdit::NoWrap);
  sprintf(txt_string, "MIT (PhysioBank) to CSV converter.\n");
  textEdit1->append(txt_string);

  QObject::connect(pushButton1, SIGNAL(clicked()), this, SLOT(SelectFileButton()));
  QObject::connect(pushButton2, SIGNAL(clicked()), myobjectDialog, SLOT(close()));

  if(!onlyShowFileOpenDialog){
      myobjectDialog->exec();
  }
  else {
      SelectFileButton();
  }
}

void UI_MIT2CSVwindow::showOpen()
{
    isConverted = false;
    SelectFileButton();
}

void UI_MIT2CSVwindow::SelectFileButton()
{
    FILE *header_inputfile=NULL;

    char header_filename[MAX_PATH_LENGTH],
            txt_string[2048],
            record_path[MAX_PATH_LENGTH],
            csv_path[MAX_PATH_LENGTH],
            filename_x[MAX_PATH_LENGTH];

    pushButton1->setEnabled(false);
    if(this->wfdb_path == NULL){
        strcpy(header_filename, QFileDialog::getOpenFileName(0, "Select inputfile", QString::fromLocal8Bit(recent_opendir), "MIT header files (*.hea *.HEA)").toLocal8Bit().data());
    } else {
        strcpy(header_filename,  this->wfdb_path);
    }


    if(!strcmp(header_filename, ""))
    {
        pushButton1->setEnabled(true);
        qDebug()<<"header_filename is empty";
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
    remove_extension_from_filename(filename_x);

    snprintf(txt_string, 2048, "Read file: %s", filename_x);
    snprintf(record_path, 2048, "%s/%s", recent_opendir, filename_x);
    snprintf(csv_path, 2048, "%s/%s.csv", recent_opendir, filename_x);

    snprintf(txt_string, 2048, "Read file: %s", filename_x);
    textEdit1->append(QString::fromLocal8Bit(txt_string));


    /////////////////// Start conversion //////////////////////////////////////////
    qDebug()<<"converting"<<record_path<<csv_path;

    convertWFDB2CSV(2, record_path, csv_path);
    textEdit1->append("Ready.\n");

    pushButton1->setEnabled(true);

    isConverted = true;
//    myobjectDialog->close();
}
