/*
***************************************************************************
*
* Author: Teunis van Beelen
*
* Copyright (C) 2010 - 2019 Teunis van Beelen
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



#ifndef SIGNALTWOCURVE_H
#define SIGNALTWOCURVE_H


#include <QtGlobal>
#include <QWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#if QT_VERSION < 0x050000
#include <QPrinter>
#else
#include <QtPrintSupport>
#endif
#include <QPrintDialog>
#include <QFileDialog>
#include <QPixmap>
#include <QPen>
#include <QString>
#include <QStringList>
#include <QFont>
#include <QMessageBox>

#include <string.h>

#include "utils.h"
#include "signalcurve.h"

#define MAXSPECTRUMMARKERS 16
#define SC_MAX_PATH_LEN 1024




class SignalTwoCurve: public QWidget
{
  Q_OBJECT

public:
  SignalTwoCurve(QWidget *parent=0, int second=0);
  ~SignalTwoCurve();

  QSize sizeHint() const {return minimumSizeHint(); }
  QSize minimumSizeHint() const {return QSize(30,10); }

  void setSignalColor(QColor);
  void setSignal2Color(QColor);
  void setTraceWidth(int);
  static QString secondToFormat(int second);
  void setBackgroundColor(QColor);
  void setRasterColor(QColor);
  void setBorderColor(QColor);
  void setTextColor(QColor);
  void setBorderSize(int);
  void setH_RulerValues(double, double);
  void setH_label(const char *);
  void setV_label(const char *);
  void setUpperLabel1(const char *);
  void setUpperLabel2(const char *);
  void setLowerLabel(const char *);

  void drawCurve( float *,  float *, int , double , double, double , double );
  void drawLine(int, double, int, double, QColor);
  void setLineEnabled(bool);
  void create_button(const char *);

  void setMarker1Enabled(bool);
  void setMarker1MovableEnabled(bool);
  void setMarker1Position(double);
  void setMarker1Color(QColor);
  double getMarker1Position(void);
  void setMarker2Enabled(bool);
  void setMarker2MovableEnabled(bool);
  void setMarker2Position(double);
  void setMarker2Color(QColor);
  double getMarker2Position(void);
  void setCrosshairColor(QColor);
  void clear();
  void setUpdatesEnabled(bool);
  void enableSpectrumColors(struct spectrum_markersblock *);
  void disableSpectrumColors();
  void setFillSurfaceEnabled(bool);
  void setV_rulerEnabled(bool);
  void setUpsidedownEnabled(bool);
  int getCursorPosition(void);
  void shiftCursorPixelsLeft(int);
  void shiftCursorPixelsRight(int);
  void shiftCursorIndexLeft(int);
  void shiftCursorIndexRight(int);
  void drawRegion(int _begin, int to);

signals:
  void extra_button_clicked();
  void extra_button2_clicked();
  void dashBoardClicked();
  void markerHasMoved();

public slots:
#if QT_VERSION < 0x050000
  void print_to_postscript();
#endif
  void print_to_pdf(QPrinter *curve_printer);
  QPixmap print_to_image();
  void print_to_printer();
  void print_to_ascii();
  void send_button_event();
  void send_button2_event();


private:

  QColor SignalColor,
         Signal2Color,
         BackgroundColor,
         RasterColor,
         BorderColor,
         RulerColor,
         TextColor,

         line1Color,
         backup_color_1,
         backup_color_1_1,
         backup_color_2,
         backup_color_3,
         backup_color_4,
         backup_color_5,
         backup_color_6;

  QPrinter *printer;

 public: QFont *sigcurve_font;
 private:
  QPen Marker1Pen,
       Marker2Pen;

  int second;
  double max_value,
         min_value,
         max2_value,
         min2_value,

         h_ruler_startvalue,
         h_ruler_endvalue,
         printsize_x_factor,
         printsize_y_factor,

         line1_start_y,
         line1_end_y,
         marker_1_position,
         marker_2_position;

  float *fbuf, *fbuf2;

  int bufsize,
      bordersize,
      drawHruler,
      drawVruler,
      drawcurve_before_raster,
      tracewidth,
      extra_button,
      use_move_events,
      mouse_x,
      mouse_y,
      mouse_old_x,
      mouse_old_y,

      marker_1_moving,
      marker_1_x_position,
      marker_2_moving,
      marker_2_x_position,
      line1_start_x,
      line1_end_x,
      w,
      h,
      old_w,
      updates_enabled,
      fillsurface;
      int begin, to;
  bool isIninitedCursor;
  char h_label[32],
       v_label[21],
       upperlabel1[64],
       upperlabel2[64],
       lowerlabel[64],
       extra_button_txt[16],
       recent_savedir[SC_MAX_PATH_LEN];

  bool Marker1Enabled,
       Marker1MovableEnabled,
       Marker2Enabled,
       Marker2MovableEnabled,
       curveUpSideDown,
       line1Enabled;


  void backup_colors_for_printing();
  void restore_colors_after_printing();
  void drawWidget(QPainter *, int, int);
  void drawWidget_to_printer(QPainter *, int, int);
  int get_directory_from_path(char *, const char *, int);

protected:
  void paintEvent(QPaintEvent *);
  void mousePressEvent(QMouseEvent *);
  void mouseReleaseEvent(QMouseEvent *);
  void mouseMoveEvent(QMouseEvent *);
  void resizeEvent(QResizeEvent *);

};


#endif


