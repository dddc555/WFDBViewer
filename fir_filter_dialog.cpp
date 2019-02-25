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



#include "fir_filter_dialog.h"





UI_FIRFilterDialog::UI_FIRFilterDialog(char *recent_dir, char *save_dir, QWidget *w_parent)
{
  int i, n;

  QListWidgetItem *item;

  QList<QListWidgetItem *> selectedlist;

  n_taps = 0;

  mainwindow = (UI_Mainwindow *)w_parent;

  recent_opendir = recent_dir;
  recent_savedir = save_dir;

  firfilterdialog = new QDialog;

  firfilterdialog->setMinimumSize(620, 415);
  firfilterdialog->setMaximumSize(620, 415);
  firfilterdialog->setWindowTitle("Add a Custom FIR filter");
  firfilterdialog->setModal(true);
  firfilterdialog->setAttribute(Qt::WA_DeleteOnClose, true);

  varsLabel = new QLabel(firfilterdialog);
  varsLabel->setGeometry(20, 20, 100, 25);
  varsLabel->setText("Filter taps: 0");

  listlabel = new QLabel(firfilterdialog);
  listlabel->setGeometry(440, 20, 100, 25);
  listlabel->setText("Select signals:");

  textEdit = new QPlainTextEdit(firfilterdialog);
  textEdit->setGeometry(20, 45, 250, 300);
  textEdit->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  textEdit->setReadOnly(false);

  list = new QListWidget(firfilterdialog);
  list->setGeometry(440, 45, 160, 300);
  list->setSelectionBehavior(QAbstractItemView::SelectRows);
  list->setSelectionMode(QAbstractItemView::ExtendedSelection);

  CancelButton = new QPushButton(firfilterdialog);
  CancelButton->setGeometry(20, 370, 100, 25);
  CancelButton->setText("&Close");

  fileButton = new QPushButton(firfilterdialog);
  fileButton->setGeometry(160, 370, 100, 25);
  fileButton->setText("&File");
  fileButton->setToolTip("Load values from text file");

  ApplyButton = new QPushButton(firfilterdialog);
  ApplyButton->setGeometry(300, 370, 100, 25);
  ApplyButton->setText("&Apply");

  helpButton = new QPushButton(firfilterdialog);
  helpButton->setGeometry(440, 370, 100, 25);
  helpButton->setText("Help");

  for(i=0; i<mainwindow->signalcomps; i++)
  {
    item = new QListWidgetItem;
    if(mainwindow->signalcomp[i]->alias[0] != 0)
    {
      item->setText(mainwindow->signalcomp[i]->alias);
    }
    else
    {
      item->setText(mainwindow->signalcomp[i]->signallabel);
    }
    item->setData(Qt::UserRole, QVariant(i));
    list->addItem(item);
  }

  n = list->count();

  for(i=0; i<n; i++)
  {
    item = list->item(i);

    item->setSelected(true);
  }

  QObject::connect(ApplyButton,  SIGNAL(clicked()),     this,            SLOT(ApplyButtonClicked()));
  QObject::connect(CancelButton, SIGNAL(clicked()),     firfilterdialog, SLOT(close()));
  QObject::connect(textEdit,     SIGNAL(textChanged()), this,            SLOT(check_text()));
  QObject::connect(helpButton,   SIGNAL(clicked()),     this,            SLOT(helpbuttonpressed()));
  QObject::connect(fileButton,   SIGNAL(clicked()),     this,            SLOT(filebuttonpressed()));

  firfilterdialog->exec();
}


void UI_FIRFilterDialog::check_text()
{
  int i, len=0;

  char *str=NULL,
       str2[512]="";

  n_taps = 0;

  strncpy(textbuf, textEdit->toPlainText().toLatin1().data(), FIR_FILTER_MAX_BUFSZ);

  len = strlen(textbuf);

  for(i=0; i<len; i++)
  {
    if(textbuf[i] == ',')
    {
      textbuf[i] = '.';
    }
  }

  textbuf[FIR_FILTER_MAX_BUFSZ] = 0;

  str = strtok(textbuf, "\r\n");

  while(str != NULL)
  {
    taps[n_taps++] = atof(str);

    if(n_taps >= FIR_FILTER_MAX_TAPS)  break;

    str = strtok(NULL, "\r\n");
  }

  snprintf(str2, 256, "Filter taps: %i", n_taps);

  varsLabel->setText(str2);
}


void UI_FIRFilterDialog::ApplyButtonClicked()
{
  int i, s, n;

  QListWidgetItem *item;

  QList<QListWidgetItem *> selectedlist;

  selectedlist = list->selectedItems();

  n = selectedlist.size();

  if(n_taps < 2)
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "Put at least two taps into the list.");
    messagewindow.exec();
    return;
  }

  for(i=0; i<n; i++)
  {
    item = selectedlist.at(i);
    s = item->data(Qt::UserRole).toInt();

    if(mainwindow->signalcomp[s]->fir_filter)
    {
      free_fir_filter(mainwindow->signalcomp[s]->fir_filter);

      mainwindow->signalcomp[s]->fir_filter = NULL;
    }

    mainwindow->signalcomp[s]->fir_filter = create_fir_filter(taps, n_taps);
    if(mainwindow->signalcomp[s]->fir_filter == NULL)
    {
      QMessageBox messagewindow(QMessageBox::Critical, "Error", "An error occurred while creating a FIR filter.");
      messagewindow.exec();
    }
  }

  mainwindow->setup_viewbuf();
}


void UI_FIRFilterDialog::filebuttonpressed()
{
  int len=0, i;

  char path[MAX_PATH_LENGTH]="",
  str[4096]="";

  FILE *inputfile;

  strcpy(path, QFileDialog::getOpenFileName(0, "Select inputfile", QString::fromLocal8Bit(recent_opendir), "All files (*)").toLocal8Bit().data());

  if(!strcmp(path, ""))
  {
    return;
  }

  get_directory_from_path(recent_opendir, path, MAX_PATH_LENGTH);

  inputfile = fopeno(path, "rb");
  if(inputfile==NULL)
  {
    snprintf(str, 2048, "Can not open file %s for reading.", path);
    QMessageBox messagewindow(QMessageBox::Critical, "Error", QString::fromLocal8Bit(str));
    messagewindow.exec();
    return;
  }

  fseek(inputfile, 0, SEEK_END);

  len = ftell(inputfile);

  if(len < 2)
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", QString::fromLocal8Bit("File too small."));
    messagewindow.exec();
    fclose(inputfile);
    return;
  }

  if(len > FIR_FILTER_MAX_BUFSZ)  len = FIR_FILTER_MAX_BUFSZ;

  fseek(inputfile, 0, SEEK_SET);

  if(fread(textbuf, len, 1, inputfile) != 1)
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", QString::fromLocal8Bit("Read error."));
    messagewindow.exec();
    fclose(inputfile);
    return;
  }

  fclose(inputfile);

  for(i=0; i<len; i++)
  {
    if(((textbuf[i] < 32) || (textbuf[i] > 126)) && (textbuf[i] != '\n') && (textbuf[i] != '\r'))
    {
      textbuf[i] = '!';
    }

    if(textbuf[i] == ',')
    {
      textbuf[i] = '.';
    }
  }

  textbuf[len] = 0;

  textEdit->setPlainText(textbuf);
}


void UI_FIRFilterDialog::helpbuttonpressed()
{
#ifdef Q_OS_LINUX
  QDesktopServices::openUrl(QUrl("file:///usr/share/doc/edfbrowser/manual.html#Custom_fir_filter"));
#endif

#ifdef Q_OS_WIN32
  char p_path[MAX_PATH_LENGTH];

  strcpy(p_path, "file:///");
  strcat(p_path, mainwindow->specialFolder(CSIDL_PROGRAM_FILES).toLocal8Bit().data());
  strcat(p_path, "\\EDFbrowser\\manual.html#Custom_fir_filter");
  QDesktopServices::openUrl(QUrl(p_path));
#endif
}












