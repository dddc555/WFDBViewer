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



#include "signalonecurve.h"


SignalOneCurve::SignalOneCurve(QWidget *w_parent) : QWidget(w_parent)
{
  setAttribute(Qt::WA_OpaquePaintEvent);

  recent_savedir[0] = 0;

  SignalColor = Qt::blue;
  tracewidth = 0;
  BackgroundColor = Qt::gray;
  RasterColor = Qt::darkGray;
  BorderColor = Qt::lightGray;
  RulerColor = Qt::black;
  TextColor = Qt::black;
  crosshair_1_color = Qt::red;

  Marker1Pen.setStyle(Qt::DashLine);
  Marker1Pen.setColor(Qt::yellow);
  Marker2Pen.setStyle(Qt::DashLine);
  Marker2Pen.setColor(Qt::yellow);

  sigcurve_font = new QFont;
#ifdef Q_OS_WIN32
  sigcurve_font->setFamily("Tahoma");
  sigcurve_font->setPixelSize(11);
#else
  sigcurve_font->setFamily("Arial");
  sigcurve_font->setPixelSize(12);
#endif

  bufsize = 0;
  dbuf = NULL;
  fbuf = NULL;
  ibuf = NULL;
  bordersize = 60;
  drawHruler = 1;
  drawVruler = 1;
  h_ruler_startvalue = 0.0;
  h_ruler_endvalue = 100.0;
  drawcurve_before_raster = 0;
  h_label[0] = 0;
  v_label[0] = 0;
  upperlabel1[0] = 0;
  upperlabel2[0] = 0;
  lowerlabel[0] = 0;
  max_value = 100.0;
  min_value = -100.0;
  extra_button = 0;
  extra_button_txt[0] = 0;
  use_move_events = 0;
  crosshair_1_active = 1;
  crosshair_1_moving = 0;
  crosshair_1_value = 0.0;
  crosshair_1_value_2 = 0.0;
  crosshair_1_x_position = 0;
  marker_1_position = 0.25;
  marker_1_moving = 0;
  marker_2_position = 0.75;
  marker_2_moving = 0;
  fillsurface = 0;

  cursorEnabled = false;
  printEnabled = false;
  dashBoardEnabled = false;


  updates_enabled = true;
  Marker1Enabled = false;
  Marker1MovableEnabled = false;
  Marker2Enabled = false;
  Marker2MovableEnabled = false;
  curveUpSideDown = false;
  line1Enabled = false;

  spectrum_color = NULL;

  old_w = 10000;

}


SignalOneCurve::~SignalOneCurve()
{
  delete sigcurve_font;
}


void SignalOneCurve::clear()
{
  dbuf = NULL;
  fbuf = NULL;
  ibuf = NULL;

  bufsize = 0;

  use_move_events = 0;
  crosshair_1_active = 0;
  crosshair_1_moving = 0;

  crosshair_1_value = 0.0;
  crosshair_1_value_2 = 0.0;
  crosshair_1_x_position = 0;
  marker_1_position = 0.25;
  marker_2_position = 0.75;
  Marker1Enabled = false;
  Marker1MovableEnabled = false;
  Marker2Enabled = false;
  Marker2MovableEnabled = false;
  line1Enabled = false;

  w = width();

  crosshair_1_active = 1;


//*/
}


void SignalOneCurve::mousePressEvent(QMouseEvent *press_event)
{
  int m_x,
      m_y;

  setFocus(Qt::MouseFocusReason);

  w = width();
  h = height();

  m_x = press_event->x() - bordersize;
  m_y = press_event->y() - bordersize;

    qDebug()<<m_x<<m_y<<"bordersize"<<bordersize<<"w"<<w;

  if(press_event->button()==Qt::LeftButton)
  {
    if(printEnabled == true)
    {
      if((m_y<-15)&&(m_y>-40)&&(m_x>((w - (bordersize * 2)) - 280))&&(m_x<((w - (bordersize * 2)) - 200)))
      {
        return;
      }
    }

    if(dashBoardEnabled == true)
    {
      if((m_y<-15)&&(m_y>-40)&&(m_x>((w - (bordersize * 2)) - 80))&&(m_x<((w - (bordersize * 2)) - 0)))
      {
        emit dashBoardClicked();

        return;
      }
    }

    if(cursorEnabled == true)
    {

      if((m_y<-15)&&(m_y>-40)&&(m_x>((w - (bordersize * 2)) - 185))&&(m_x<((w - (bordersize * 2)) - 100)))
      {
        if(crosshair_1_active)
        {
          crosshair_1_active = 0;
          crosshair_1_moving = 0;
          use_move_events = 0;

          setMouseTracking(false);
        }
        else
        {
          crosshair_1_active = 1;
          if(!crosshair_1_x_position)
          {
            crosshair_1_value = 0.0;
            crosshair_1_x_position = (w - (bordersize * 2)) / 2;
            crosshair_1_y_position = (h - (bordersize * 2)) / 2;
            mouse_old_x = crosshair_1_x_position;
            mouse_old_y = crosshair_1_y_position;
          }
        }

        update();

        return;
      }
    }

    if(crosshair_1_active)
    {
      if((m_y<(crosshair_1_y_position + 15))&&(m_y>(crosshair_1_y_position - 25))&&(m_x>crosshair_1_x_position)&&(m_x<(crosshair_1_x_position + 65)))
      {
        crosshair_1_moving = 1;
        use_move_events = 1;
        setMouseTracking(true);
        mouse_old_x = m_x;
        mouse_old_y = m_y;
      }

      if(m_x>(crosshair_1_x_position-10)&&(m_x<(crosshair_1_x_position + 10)))
      {
        crosshair_1_moving = 1;
        use_move_events = 1;
        setMouseTracking(true);
        mouse_old_x = m_x;
        mouse_old_y = m_y;
      }
    }

//    if(m_x < 0 ||
//       m_x > (w - (bordersize * 2)) ||
//       m_y < 0 ||
//       m_y > (h - (bordersize * 2)))
//    {
//      return;
//    }

    if((Marker1MovableEnabled == true) && (Marker1Enabled == true))
    {
      marker_1_x_position = (w - (bordersize * 2)) * marker_1_position;

      if(m_x > (marker_1_x_position - 5) && (m_x < (marker_1_x_position + 5)))
      {
        marker_1_moving = 1;
        use_move_events = 1;
        setMouseTracking(true);
        mouse_old_x = m_x;
        mouse_old_y = m_y;
      }
    }

    if((Marker2MovableEnabled == true) && (Marker2Enabled == true))
    {
      marker_2_x_position = (w - (bordersize * 2)) * marker_2_position;

      if(m_x > (marker_2_x_position - 5) && (m_x < (marker_2_x_position + 5)))
      {
        marker_2_moving = 1;
        use_move_events = 1;
        setMouseTracking(true);
        mouse_old_x = m_x;
        mouse_old_y = m_y;
      }
    }
  }
}


void SignalOneCurve::mouseReleaseEvent(QMouseEvent *)
{
  crosshair_1_moving = 0;
  marker_1_moving = 0;
  marker_2_moving = 0;
  use_move_events = 0;
  setMouseTracking(false);
}


void SignalOneCurve::mouseMoveEvent(QMouseEvent *move_event)
{
  if(!use_move_events)
  {
    return;
  }

  mouse_x = move_event->x() - bordersize;
  mouse_y = move_event->y() - bordersize;

  if(crosshair_1_moving)
  {
    crosshair_1_x_position += (mouse_x - mouse_old_x);
    mouse_old_x = mouse_x;
    if(crosshair_1_x_position<2)
    {
      crosshair_1_x_position = 2;
    }
    if(crosshair_1_x_position>(w-(bordersize * 2) -2))
    {
      crosshair_1_x_position = w-(bordersize * 2) -2;
    }

    crosshair_1_y_position += (mouse_y - mouse_old_y);
    mouse_old_y = mouse_y;
    if(crosshair_1_y_position<25)
    {
      crosshair_1_y_position = 25;
    }
    if(crosshair_1_y_position>(h-(bordersize * 2) -25))
    {
      crosshair_1_y_position = h-(bordersize * 2) -25;
    }
  }

  if(marker_1_moving)
  {
    marker_1_x_position += (mouse_x - mouse_old_x);
    mouse_old_x = mouse_x;
    if(marker_1_x_position<2)
    {
      marker_1_x_position = 2;
    }
    if(marker_1_x_position>(w-(bordersize * 2) - 2))
    {
      marker_1_x_position = w-(bordersize * 2) - 2;
    }

    marker_1_position = (double)marker_1_x_position / (double)(w-(bordersize * 2));

    emit markerHasMoved();
  }

  if(marker_2_moving)
  {
    marker_2_x_position += (mouse_x - mouse_old_x);
    mouse_old_x = mouse_x;
    if(marker_2_x_position<2)
    {
      marker_2_x_position = 2;
    }
    if(marker_2_x_position>(w-(bordersize * 2) - 2))
    {
      marker_2_x_position = w-(bordersize * 2) - 2;
    }

    marker_2_position = (double)marker_2_x_position / (double)(w-(bordersize * 2));

    emit markerHasMoved();
  }

  update();
}


void SignalOneCurve::shiftCursorIndexLeft(int idxs)
{
  int idx;

  double ppi;

  if(!crosshair_1_active)
  {
    return;
  }

  if(bufsize < 2)
  {
    return;
  }

  w = width();

  ppi = (double)(w-(bordersize * 2)) / (double)bufsize;

  idx = (double)crosshair_1_x_position / ppi;

  idx -= idxs;

  if(idx < 0)
  {
    idx = 0;
  }

  if(idx >= bufsize)
  {
    idx = bufsize - 1;
  }

  crosshair_1_x_position = (double)idx * ppi + (0.5 * ppi);

  update();
}


void SignalOneCurve::shiftCursorIndexRight(int idxs)
{
  int idx;

  double ppi;

  if(!crosshair_1_active)
  {
    return;
  }

  if(bufsize < 2)
  {
    return;
  }

  w = width();

  ppi = (double)(w-(bordersize * 2)) / (double)bufsize;

  idx = (double)crosshair_1_x_position / ppi;

  idx += idxs;

  if(idx < 0)
  {
    idx = 0;
  }

  if(idx >= bufsize)
  {
    idx = bufsize - 1;
  }

  crosshair_1_x_position = (double)idx * ppi + (0.5 * ppi);

  update();
}


void SignalOneCurve::shiftCursorPixelsLeft(int pixels)
{
  if(!crosshair_1_active)
  {
    return;
  }

  crosshair_1_x_position -= pixels;
  if(crosshair_1_x_position<2)
  {
    crosshair_1_x_position = 2;
  }

  update();
}


void SignalOneCurve::shiftCursorPixelsRight(int pixels)
{
  if(!crosshair_1_active)
  {
    return;
  }

  w = width();

  crosshair_1_x_position += pixels;
  if(crosshair_1_x_position>(w-(bordersize * 2) -2))
  {
    crosshair_1_x_position = w-(bordersize * 2) -2;
  }

  update();
}


void SignalOneCurve::resizeEvent(QResizeEvent *resize_event)
{
  if(crosshair_1_active)
  {
    crosshair_1_x_position *= ((double)(width() - (bordersize * 2)) / (double)(old_w - (bordersize * 2)));
  }
  QWidget::resizeEvent(resize_event);
}


void SignalOneCurve::send_button_event()
{
  emit extra_button_clicked();
}
void SignalOneCurve::send_button2_event()
{
  emit extra_button2_clicked();
}


void SignalOneCurve::create_button(const char *txt)
{
  extra_button = 1;
  strncpy(extra_button_txt, txt, 16);
  extra_button_txt[15] = 0;
}


void SignalOneCurve::backup_colors_for_printing(void)
{
  backup_color_1 = SignalColor;
  SignalColor = Qt::black;
  backup_color_2 = RasterColor;
  RasterColor = Qt::black;
  backup_color_3 = BorderColor;
  BorderColor = Qt::black;
  backup_color_4 = RulerColor;
  RulerColor = Qt::black;
  backup_color_5 = TextColor;
  TextColor = Qt::black;
  backup_color_6 = Marker1Pen.color();
  Marker1Pen.setColor(Qt::black);
}


void SignalOneCurve::restore_colors_after_printing(void)
{
  SignalColor = backup_color_1;
  RasterColor = backup_color_2;
  BorderColor = backup_color_3;
  RulerColor = backup_color_4;
  TextColor = backup_color_5;
  Marker1Pen.setColor(backup_color_6);
}


void SignalOneCurve::print_to_ascii()
{
  int i;

  char path[SC_MAX_PATH_LEN];

  FILE *outputfile;

  path[0] = 0;
  if(recent_savedir[0]!=0)
  {
    strcpy(path, recent_savedir);
    strcat(path, "/");
  }
  strcat(path, "curve.csv");

  strcpy(path, QFileDialog::getSaveFileName(0, "Print to ASCII / CSV", QString::fromLocal8Bit(path), "ASCII / CSV files (*.csv *.CSV *.txt *.TXT)").toLocal8Bit().data());

  if(!strcmp(path, ""))
  {
    return;
  }

  get_directory_from_path(recent_savedir, path, SC_MAX_PATH_LEN);

  outputfile = fopen(path, "wb");
  if(outputfile == NULL)
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "Can not open outputfile for writing.");
    messagewindow.exec();
    return;
  }

  if(ibuf != NULL)
  {
    for(i=0; i<bufsize; i++)
    {
      fprintf(outputfile, "%i\n", ibuf[i]);
    }
  }

  if(dbuf != NULL)
  {
    for(i=0; i<bufsize; i++)
    {
      fprintf(outputfile, "%.8f\n", dbuf[i]);
    }
  }

  if(fbuf != NULL)
  {
    for(i=0; i<bufsize; i++)
    {
      fprintf(outputfile, "%.8f\n", fbuf[i]);
    }
  }

  fclose(outputfile);
}


#if QT_VERSION < 0x050000
void SignalOneCurve::print_to_postscript()
{
  char path[SC_MAX_PATH_LEN];

  path[0] = 0;
  if(recent_savedir[0]!=0)
  {
    strcpy(path, recent_savedir);
    strcat(path, "/");
  }
  strcat(path, "curve.ps");

  strcpy(path, QFileDialog::getSaveFileName(0, "Print to PostScript", QString::fromLocal8Bit(path), "PostScript files (*.ps *.PS)").toLocal8Bit().data());

  if(!strcmp(path, ""))
  {
    return;
  }

  get_directory_from_path(recent_savedir, path, SC_MAX_PATH_LEN);

  QPrinter curve_printer(QPrinter::HighResolution);

  curve_printer.setOutputFormat(QPrinter::PostScriptFormat);
  curve_printer.setOutputFileName(path);
  curve_printer.setPageSize(QPrinter::A4);
  curve_printer.setOrientation(QPrinter::Landscape);

  backup_colors_for_printing();

  QPainter paint(&curve_printer);

  drawWidget_to_printer(&paint, curve_printer.pageRect().width(), curve_printer.pageRect().height());

  restore_colors_after_printing();

}
#endif


void SignalOneCurve::print_to_pdf(QPrinter *curve_printer)
{
  backup_colors_for_printing();

  QPainter paint(&curve_printer);
#if QT_VERSION >= 0x050000
  paint.setRenderHint(QPainter::Qt4CompatiblePainting, true);
#endif

  drawWidget_to_printer(&paint, curve_printer.pageRect().width(), curve_printer.pageRect().height());

  restore_colors_after_printing();
}


QPixmap SignalOneCurve::print_to_image()
{
  QPixmap pixmap(width(), height());

  QPainter paint(&pixmap);
#if QT_VERSION >= 0x050000
  paint.setRenderHint(QPainter::Qt4CompatiblePainting, true);
#endif

  drawWidget(&paint, width(), height());

//  pixmap.save(path, "PNG", 90);
  return pixmap;
}


void SignalOneCurve::print_to_printer()
{
  QPrinter curve_printer(QPrinter::HighResolution);

  curve_printer.setOutputFormat(QPrinter::NativeFormat);
  curve_printer.setPageSize(QPrinter::A4);
  curve_printer.setOrientation(QPrinter::Landscape);

  QPrintDialog printerdialog(&curve_printer, this);
  printerdialog.setWindowTitle("Print");

  if(!(printerdialog.exec()==QDialog::Accepted))
  {
    return;
  }

  backup_colors_for_printing();

  QPainter paint(&curve_printer);
#if QT_VERSION >= 0x050000
  paint.setRenderHint(QPainter::Qt4CompatiblePainting, true);
#endif

  drawWidget_to_printer(&paint, curve_printer.pageRect().width(), curve_printer.pageRect().height());

  restore_colors_after_printing();
}


void SignalOneCurve::drawWidget_to_printer(QPainter *painter, int curve_w, int curve_h)
{
  int i, j,
      precision,
      bordersize_backup=0,
      p_w,
      p_divisor,
      p_range,
      p_tmp,
      p_h,
      p2_divisor,
      p2_range,
      p2_multiplier,
      p2_ruler_startvalue,
      p2_ruler_endvalue,
      p2_tmp;

  long long p_multiplier,
            p_ruler_startvalue,
            p_ruler_endvalue;

  char str[128];

  double v_sens,
         offset,
         h_step,
         value,
         p_factor,
         p_pixels_per_unit,
         p2_pixels_per_unit;

  QString q_str;

  QFont curve_font;

  QPen printer_pen;


  curve_w -= 30;
  curve_h -= 30;

  p_factor = (double)curve_w / width();

  bordersize_backup = bordersize;
  bordersize *= p_factor;

  curve_font.setFamily("Arial");
  curve_font.setPixelSize((int)((double)curve_w / 104.0));
  painter->setFont(curve_font);

  if((curve_w < ((bordersize * 2) + 5)) || (curve_h < ((bordersize * 2) + 5)))
  {
    bordersize = bordersize_backup;

    return;
  }

  printer_pen = QPen(Qt::SolidPattern, 2, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin);
  printer_pen.setColor(Qt::black);

  painter->setPen(printer_pen);

/////////////////////////////////// draw the window ///////////////////////////////////////////

  painter->drawLine(0, 0, 0, curve_h);
  painter->drawLine(0, 0, curve_w, 0);
  painter->drawLine(curve_w, curve_h, curve_w, 0);
  painter->drawLine(curve_w, curve_h, 0, curve_h);

/////////////////////////////////// draw the rulers ///////////////////////////////////////////

  p_w = curve_w - bordersize - bordersize;

  p_multiplier = 1;

  while((h_ruler_endvalue * p_multiplier) < 10000.0)
  {
    p_multiplier *= 10;

    if(p_multiplier > 100000000000000LL)
    {
      break;
    }
  }

  p_ruler_startvalue = h_ruler_startvalue * p_multiplier;

  p_ruler_endvalue = h_ruler_endvalue * p_multiplier;

  p_range = p_ruler_endvalue - p_ruler_startvalue;

  p_pixels_per_unit = (double)p_w / (double)p_range;

  p_divisor = 1;

  while((p_range / p_divisor) > 10)
  {
    p_divisor *= 2;

    if((p_range / p_divisor) <= 10)
    {
      break;
    }

    p_divisor /= 2;

    p_divisor *= 5;

    if((p_range / p_divisor) <= 10)
    {
      break;
    }

    p_divisor *= 2;
  }

  if(drawHruler && (bordersize > (19 * p_factor)))
  {
    painter->drawLine(bordersize, curve_h - bordersize + (5 * p_factor), curve_w - bordersize, curve_h - bordersize + (5 * p_factor));

    for(i = (p_ruler_startvalue / p_divisor) * p_divisor; i <= p_ruler_endvalue; i += p_divisor)
    {
      if(i < p_ruler_startvalue)
      {
        continue;
      }

      convert_to_metric_suffix(str, (double)i / (double)p_multiplier, 4);

      remove_trailing_zeros(str);

      p_tmp = (double)(i - p_ruler_startvalue) * p_pixels_per_unit;

      painter->drawText(bordersize + p_tmp - (30 * p_factor),  curve_h - bordersize + (18 * p_factor), 60 * p_factor, 16 * p_factor, Qt::AlignCenter | Qt::TextSingleLine, str);

      painter->drawLine(bordersize + p_tmp, curve_h - bordersize + (5 * p_factor), bordersize + p_tmp, curve_h - bordersize + ((5 + 10) * p_factor));
    }

    painter->drawText(curve_w - bordersize + (20 * p_factor),  curve_h - bordersize + (18 * p_factor), 40 * p_factor, 16 * p_factor, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, h_label);
  }

/////////////////////////////////// draw the vertical ruler ///////////////////////////////////////////

  p_h = curve_h - bordersize - bordersize;

  p2_multiplier = 1;

  while(((max_value - min_value) * p2_multiplier) < 10000.0)
  {
    p2_multiplier *= 10;

    if(p2_multiplier > 10000000)
    {
      break;
    }
  }

  p2_ruler_startvalue = min_value * p2_multiplier;

  p2_ruler_endvalue = max_value * p2_multiplier;

  p2_range = p2_ruler_endvalue - p2_ruler_startvalue;

  if(p2_range < 0)  p2_range *= -1;

  p2_pixels_per_unit = (double)p_h / (double)p2_range;

  p2_divisor = 1;

  while((p2_range / p2_divisor) > 10)
  {
    p2_divisor *= 2;

    if((p2_range / p2_divisor) <= 10)
    {
      break;
    }

    p2_divisor /= 2;

    p2_divisor *= 5;

    if((p2_range / p2_divisor) <= 10)
    {
      break;
    }

    p2_divisor *= 2;
  }

  if(drawVruler && (bordersize > (29 * p_factor)))
  {
    painter->drawLine(bordersize - (5 * p_factor), bordersize, bordersize - (5 * p_factor), curve_h - bordersize);

    precision = 0;

    if((max_value < 10.0) && (max_value > -10.0) && (min_value < 10.0) && (min_value > -10.0))
    {
      precision = 1;

      if((max_value < 1.0) && (max_value > -1.0) && (min_value < 1.0) && (min_value > -1.0))
      {
        precision = 2;

        if((max_value < 0.1) && (max_value > -0.1) && (min_value < 0.1) && (min_value > -0.1))
        {
          precision = 3;

          if((max_value < 0.01) && (max_value > -0.01) && (min_value < 0.01) && (min_value > -0.01))
          {
            precision = 4;
          }
        }
      }
    }

    for(i = (p2_ruler_startvalue / p2_divisor) * p2_divisor; i <= p2_ruler_endvalue; i += p2_divisor)
    {
      if(i < p2_ruler_startvalue)
      {
        continue;
      }

      q_str.setNum((double)i / (double)p2_multiplier, 'f', precision);

      p2_tmp = (double)(i - p2_ruler_startvalue) * p2_pixels_per_unit;

      if(curveUpSideDown == false)
      {
        painter->drawText((3 * p_factor), curve_h - bordersize - p2_tmp - (8 * p_factor), (40 * p_factor), (16 * p_factor), Qt::AlignRight | Qt::AlignVCenter | Qt::TextSingleLine, q_str);

        painter->drawLine(bordersize - (5 * p_factor), curve_h - bordersize - p2_tmp, bordersize - (15 * p_factor), curve_h - bordersize - p2_tmp);
      }
      else
      {
        painter->drawText((3 * p_factor), bordersize + p2_tmp - (8 * p_factor), (40 * p_factor), (16 * p_factor), Qt::AlignRight | Qt::AlignVCenter | Qt::TextSingleLine, q_str);

        painter->drawLine(bordersize - (5 * p_factor), bordersize + p2_tmp, bordersize - (15 * p_factor), bordersize + p2_tmp);
      }
    }
  }

/////////////////////////////////// draw the labels ///////////////////////////////////////////

  painter->setFont(curve_font);

  if(v_label[0] != 0)
  {
    painter->drawText(8 * p_factor, 30 * p_factor, 100 * p_factor, 16 * p_factor, Qt::AlignCenter | Qt::TextSingleLine, v_label);
  }

  if(upperlabel1[0] != 0)
  {
    painter->drawText(curve_w / 2 - (200 * p_factor), 10 * p_factor, 400 * p_factor, 16 * p_factor, Qt::AlignCenter | Qt::TextSingleLine, upperlabel1);
  }

  if(upperlabel2[0] != 0)
  {
    painter->drawText(curve_w / 2 - (200 * p_factor), 30 * p_factor, 400 * p_factor, 16 * p_factor, Qt::AlignCenter | Qt::TextSingleLine, upperlabel2);
  }

  if(lowerlabel[0] != 0)
  {
    painter->drawText(curve_w / 2 - (80 * p_factor), curve_h - (20 * p_factor), 160 * p_factor, 16 * p_factor, Qt::AlignCenter | Qt::TextSingleLine, lowerlabel);
  }

/////////////////////////////////// translate coordinates, draw and fill a rectangle ///////////////////////////////////////////

  painter->translate(QPoint(bordersize, bordersize));

  curve_w -= (bordersize * 2);

  curve_h -= (bordersize * 2);

  painter->setClipping(true);
  painter->setClipRegion(QRegion(0, 0, curve_w, curve_h), Qt::ReplaceClip);

/////////////////////////////////// draw the rasters ///////////////////////////////////////////

  painter->drawRect (0, 0, curve_w - 1, curve_h - 1);

  for(i = (p_ruler_startvalue / p_divisor) * p_divisor; i <= p_ruler_endvalue; i += p_divisor)
  {
    if(i < p_ruler_startvalue)
    {
      continue;
    }

    p_tmp = (double)(i - p_ruler_startvalue) * p_pixels_per_unit;

    painter->drawLine(p_tmp, 0, p_tmp, curve_h);
  }

  for(i = (p2_ruler_startvalue / p2_divisor) * p2_divisor; i <= p2_ruler_endvalue; i += p2_divisor)
  {
    if(i < p2_ruler_startvalue)
    {
      continue;
    }

    p2_tmp = (double)(i - p2_ruler_startvalue) * p2_pixels_per_unit;

    if(curveUpSideDown == false)
    {
      painter->drawLine(0, curve_h - p2_tmp, curve_w, curve_h - p2_tmp);
    }
    else
    {
      painter->drawLine(0, p2_tmp, curve_w, p2_tmp);
    }
  }

/////////////////////////////////// draw the curve ///////////////////////////////////////////

  if((dbuf == NULL) && (ibuf == NULL) && (fbuf == NULL)) return;

  if(max_value <= min_value)  return;

  if(bufsize < 2)  return;

  if(curveUpSideDown == true)
  {
    offset = (-(min_value));

    v_sens = curve_h / (max_value - min_value);
  }
  else
  {
    offset = (-(max_value));

    v_sens = (-(curve_h / (max_value - min_value)));
  }

  h_step = (double)curve_w / (double)bufsize;

  if(dbuf)
  {
    for(i = 0; i < bufsize; i++)
    {
      if(fillsurface)
      {
        if(bufsize < curve_w)
        {
          for(j = 0; j < h_step; j++)
          {
            painter->drawLine((i * h_step) + j, (dbuf[i] + offset) * v_sens, (i * h_step) + j, curve_h);
          }
        }
        else
        {
          painter->drawLine(i * h_step, (dbuf[i] + offset) * v_sens, i * h_step, curve_h);
        }
      }
      else
      {
        if(bufsize < (curve_w / 2))
        {
          painter->drawLine(i * h_step, (dbuf[i] + offset) * v_sens, (i + 1) * h_step, (dbuf[i] + offset) * v_sens);
          if(i)
          {
            painter->drawLine(i * h_step, (dbuf[i - 1] + offset) * v_sens, i * h_step, (dbuf[i] + offset) * v_sens);
          }
        }
        else
        {
          if(i < (bufsize - 1))
          {
            {
              painter->drawLine(i * h_step, (dbuf[i] + offset) * v_sens, (i + 1) * h_step, (dbuf[i + 1] + offset) * v_sens);
            }
          }
        }
      }

      if(crosshair_1_active)
      {
        if(i==((int)(((double)crosshair_1_x_position * p_factor) / h_step)))
        {
          crosshair_1_y_value = (dbuf[i] + offset) * v_sens;
          crosshair_1_value = dbuf[i];
          value = (h_ruler_endvalue - h_ruler_startvalue) / bufsize;
          crosshair_1_value_2 = (i * value) + (0.5 * value) + h_ruler_startvalue;
        }
      }
    }
  }

  if(ibuf)
  {
    for(i = 0; i < bufsize; i++)
    {
      if(fillsurface)
      {
        if(bufsize < curve_w)
        {
          for(j = 0; j < h_step; j++)
          {
            painter->drawLine((i * h_step) + j, ((double)(ibuf[i]) + offset) * v_sens, (i * h_step) + j, curve_h);
          }
        }
        else
        {
          painter->drawLine(i * h_step, ((double)(ibuf[i]) + offset) * v_sens, i * h_step, curve_h);
        }
      }
      else
      {
        if(bufsize < (curve_w / 2))
        {
          painter->drawLine(i * h_step, ((double)(ibuf[i]) + offset) * v_sens, (i + 1) * h_step, ((double)(ibuf[i]) + offset) * v_sens);
          if(i)
          {
            painter->drawLine(i * h_step, ((double)(ibuf[i - 1]) + offset) * v_sens, i * h_step, ((double)(ibuf[i]) + offset) * v_sens);
          }
        }
        else
        {
          if(i < (bufsize - 1))
          {
            {
              painter->drawLine(i * h_step, ((double)(ibuf[i]) + offset) * v_sens, (i + 1) * h_step, ((double)(ibuf[i + 1]) + offset) * v_sens);
            }
          }
        }
      }

      if(crosshair_1_active)
      {
        if(i==((int)(((double)crosshair_1_x_position * p_factor) / h_step)))
        {
          crosshair_1_y_value = ((double)(ibuf[i]) + offset) * v_sens;
          crosshair_1_value = (double)(ibuf[i]);
          value = (h_ruler_endvalue - h_ruler_startvalue) / bufsize;
          crosshair_1_value_2 = (i * value) + (0.5 * value) + h_ruler_startvalue;
        }
      }
    }
  }

  if(fbuf)
  {
    for(i = 0; i < bufsize; i++)
    {
      if(fillsurface)
      {
        if(bufsize < curve_w)
        {
          for(j = 0; j < h_step; j++)
          {
            painter->drawLine((i * h_step) + j, (fbuf[i] + offset) * v_sens, (i * h_step) + j, curve_h);
          }
        }
        else
        {
          painter->drawLine(i * h_step, (fbuf[i] + offset) * v_sens, i * h_step, curve_h);
        }
      }
      else
      {
        if(bufsize < (curve_w / 2))
        {
          painter->drawLine(i * h_step, (fbuf[i] + offset) * v_sens, (i + 1) * h_step, (fbuf[i] + offset) * v_sens);
          if(i)
          {
            painter->drawLine(i * h_step, (fbuf[i - 1] + offset) * v_sens, i * h_step, (fbuf[i] + offset) * v_sens);
          }
        }
        else
        {
          if(i < (bufsize - 1))
          {
            {
              painter->drawLine(i * h_step, (fbuf[i] + offset) * v_sens, (i + 1) * h_step, (fbuf[i + 1] + offset) * v_sens);
            }
          }
        }
      }

      if(crosshair_1_active)
      {
        if(i==((int)(((double)crosshair_1_x_position * p_factor) / h_step)))
        {
          crosshair_1_y_value = (fbuf[i] + offset) * v_sens;
          crosshair_1_value = fbuf[i];
          value = (h_ruler_endvalue - h_ruler_startvalue) / bufsize;
          crosshair_1_value_2 = (i * value) + (0.5 * value) + h_ruler_startvalue;
        }
      }
    }
  }

/////////////////////////////////// draw the line ///////////////////////////////////////////

  if(line1Enabled == true)
  {
    painter->drawLine(line1_start_x * h_step, (line1_start_y + offset) * v_sens, line1_end_x * h_step, (line1_end_y + offset) * v_sens);
  }

/////////////////////////////////// draw the markers ///////////////////////////////////////////

  if(Marker1Enabled == true)
  {
    painter->drawLine(curve_w * marker_1_position, 0, curve_w * marker_1_position, curve_h);
  }

  if(Marker2Enabled == true)
  {
    painter->drawLine(curve_w * marker_2_position, 0, curve_w * marker_2_position, curve_h);
  }

/////////////////////////////////// draw the cursor ///////////////////////////////////////////

  if(crosshair_1_active)
  {
    QPainterPath path;
    path.moveTo(crosshair_1_x_position * p_factor, crosshair_1_y_value);
    path.lineTo((crosshair_1_x_position - 4) * p_factor, crosshair_1_y_value - (9 * p_factor));
    path.lineTo((crosshair_1_x_position + 4) * p_factor, crosshair_1_y_value - (9 * p_factor));
    path.lineTo(crosshair_1_x_position * p_factor, crosshair_1_y_value);
    painter->fillPath(path, QBrush(Qt::black));

    snprintf(str, 128, "%f", crosshair_1_value);
    painter->drawText((crosshair_1_x_position + 8) * p_factor, (crosshair_1_y_position - 10) * p_factor, str);
    convert_to_metric_suffix(str, crosshair_1_value_2, 3);
    strcat(str, h_label);
    painter->drawText((crosshair_1_x_position + 8) * p_factor, (crosshair_1_y_position + 10) * p_factor, str);
  }

  bordersize = bordersize_backup;
}


void SignalOneCurve::setUpdatesEnabled(bool enabled)
{
  updates_enabled = enabled;
}


void SignalOneCurve::paintEvent(QPaintEvent *)
{
  if(updates_enabled == true)
  {
    QPainter paint(this);
#if QT_VERSION >= 0x050000
    paint.setRenderHint(QPainter::Qt4CompatiblePainting, true);
#endif
    if(!isIninitedCursor){
        isIninitedCursor = true;
        if(crosshair_1_active && !crosshair_1_x_position)
        {
          w = width();
          h = height();
          crosshair_1_value = 0.0;
          crosshair_1_x_position = (w - (bordersize * 2)) / 2;
          crosshair_1_y_position = (h - (bordersize * 2)) / 2;
          mouse_old_x = crosshair_1_x_position;
          mouse_old_y = crosshair_1_y_position;
        }
    }
    drawWidget(&paint, width(), height());

    old_w = width();

  }
}


void SignalOneCurve::drawWidget(QPainter *painter, int curve_w, int curve_h)
{
  int i, j,
      precision,
      p_w,
      p_h;

  long long lk,
            p_multiplier,
            p_ruler_startvalue,
            p_ruler_endvalue,
            p_divisor,
            p_range,
            p_tmp,
            p2_divisor,
            p2_range,
            p2_multiplier,
            p2_ruler_startvalue,
            p2_ruler_endvalue,
            p2_tmp;

  double v_sens=0.0,
         offset=0.0,
         h_step=0.0,
         value,
         pixelsPerUnit,
         sum_colorbar_value,
         p_pixels_per_unit,
         p2_pixels_per_unit;

  char str[128];

  QString q_str;


  painter->setFont(*sigcurve_font);

  painter->fillRect(0, 0, curve_w, curve_h, BorderColor);

  if((curve_w < ((bordersize * 2) + 5)) || (curve_h < ((bordersize * 2) + 5)))
  {
    return;
  }

/////////////////////////////////// draw the horizontal ruler ///////////////////////////////////////////

  p_w = curve_w - bordersize - bordersize;

  p_multiplier = 1;

  while((h_ruler_endvalue * p_multiplier) < 10000.0)
  {
    p_multiplier *= 10;

    if(p_multiplier > 100000000000000LL)
    {
      break;
    }
  }

  p_ruler_startvalue = h_ruler_startvalue * p_multiplier;

  p_ruler_endvalue = h_ruler_endvalue * p_multiplier;

  p_range = p_ruler_endvalue - p_ruler_startvalue;

  p_pixels_per_unit = (double)p_w / (double)p_range;

  p_divisor = 1;

  while((p_range / p_divisor) > 10)
  {
    p_divisor *= 2;

    if((p_range / p_divisor) <= 10)
    {
      break;
    }

    p_divisor /= 2;

    p_divisor *= 5;

    if((p_range / p_divisor) <= 10)
    {
      break;
    }

    p_divisor *= 2;
  }

//   printf("p_multiplier is %lli\n"
//         "p_ruler_startvalue is %lli\n"
//         "p_ruler_endvalue is %lli\n"
//         "p_range is %lli\n"
//         "p_divisor is %lli\n"
//         "p_pixels_per_unit is %.12f\n\n",
//         p_multiplier,
//         p_ruler_startvalue,
//         p_ruler_endvalue,
//         p_range,
//         p_divisor,
//         p_pixels_per_unit);

  if(drawHruler && (bordersize > 19))
  {
    painter->setPen(RulerColor);

    painter->drawLine(bordersize, curve_h - bordersize + 5, curve_w - bordersize, curve_h - bordersize + 5);

    for(lk = (p_ruler_startvalue / p_divisor) * p_divisor; lk <= p_ruler_endvalue; lk += p_divisor)
    {
      if(lk < p_ruler_startvalue)
      {
        continue;
      }

      convert_to_metric_suffix(str, (double)lk / (double)p_multiplier, 4);

      remove_trailing_zeros(str);

      p_tmp = (double)(lk - p_ruler_startvalue) * p_pixels_per_unit;

      painter->drawText(bordersize + p_tmp - 30,  curve_h - bordersize + 18, 60, 16, Qt::AlignCenter | Qt::TextSingleLine, str);

      painter->drawLine(bordersize + p_tmp, curve_h - bordersize + 5, bordersize + p_tmp, curve_h - bordersize + 5 + 10);
    }

    painter->drawText(curve_w - bordersize + 20,  curve_h - bordersize + 18, 40, 16, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, h_label);
  }

/////////////////////////////////// draw the vertical ruler ///////////////////////////////////////////

  p_h = curve_h - bordersize - bordersize;

  p2_multiplier = 1;

  while(((max_value - min_value) * p2_multiplier) < 10000.0)
  {
    p2_multiplier *= 10;

    if(p2_multiplier > 10000000)
    {
      break;
    }
  }

  p2_ruler_startvalue = min_value * p2_multiplier;

  p2_ruler_endvalue = max_value * p2_multiplier;

  p2_range = p2_ruler_endvalue - p2_ruler_startvalue;

  if(p2_range < 0)  p2_range *= -1;

  p2_pixels_per_unit = (double)p_h / (double)p2_range;

  p2_divisor = 1;

  while((p2_range / p2_divisor) > 10)
  {
    p2_divisor *= 2;

    if((p2_range / p2_divisor) <= 10)
    {
      break;
    }

    p2_divisor /= 2;

    p2_divisor *= 5;

    if((p2_range / p2_divisor) <= 10)
    {
      break;
    }

    p2_divisor *= 2;
  }

//   printf("p2_multiplier is %lli\n"
//         "p2_ruler_startvalue is %lli\n"
//         "p2_ruler_endvalue is %lli\n"
//         "p2_range is %lli\n"
//         "p2_divisor is %lli\n"
//         "p2_pixels_per_unit is %.12f\n"
//         "max_value is %f\n"
//         "min_value is %f\n\n",
//         p2_multiplier,
//         p2_ruler_startvalue,
//         p2_ruler_endvalue,
//         p2_range,
//         p2_divisor,
//         p2_pixels_per_unit,
//         max_value,
//         min_value);

  if(drawVruler && (bordersize > 29))
  {
    painter->setPen(RulerColor);

    painter->drawLine(bordersize - 5, bordersize, bordersize - 5, curve_h - bordersize);

    precision = 0;

    if((max_value < 10.0) && (max_value > -10.0) && (min_value < 10.0) && (min_value > -10.0))
    {
      precision = 1;

      if((max_value < 1.0) && (max_value > -1.0) && (min_value < 1.0) && (min_value > -1.0))
      {
        precision = 2;

        if((max_value < 0.1) && (max_value > -0.1) && (min_value < 0.1) && (min_value > -0.1))
        {
          precision = 3;

          if((max_value < 0.01) && (max_value > -0.01) && (min_value < 0.01) && (min_value > -0.01))
          {
            precision = 4;
          }
        }
      }
    }

    for(lk = (p2_ruler_startvalue / p2_divisor) * p2_divisor; lk <= p2_ruler_endvalue; lk += p2_divisor)
    {
      if(lk < p2_ruler_startvalue)
      {
        continue;
      }

      q_str.setNum((double)lk / (double)p2_multiplier, 'f', precision);

      p2_tmp = (double)(lk - p2_ruler_startvalue) * p2_pixels_per_unit;

      if(curveUpSideDown == false)
      {
        painter->drawText(3, curve_h - bordersize - p2_tmp - 8, 40, 16, Qt::AlignRight | Qt::AlignVCenter | Qt::TextSingleLine, q_str);

        painter->drawLine(bordersize - 5, curve_h - bordersize - p2_tmp, bordersize - 5 - 10, curve_h - bordersize - p2_tmp);
      }
      else
      {
        painter->drawText(3, bordersize + p2_tmp - 8, 40, 16, Qt::AlignRight | Qt::AlignVCenter | Qt::TextSingleLine, q_str);

        painter->drawLine(bordersize - 5, bordersize + p2_tmp, bordersize - 5 - 10, bordersize + p2_tmp);
      }
    }
  }

/////////////////////////////////// draw the labels ///////////////////////////////////////////

  painter->setPen(TextColor);
  painter->setFont(*sigcurve_font);

  if(v_label[0] != 0)
  {
    painter->drawText(8, 30, 100, 16, Qt::AlignCenter | Qt::TextSingleLine, v_label);
  }

  if(upperlabel1[0] != 0)
  {
      QString upperLabelString(upperlabel1);
      upperLabelString.replace("FFT resolution", tr("FFT resolution"));
      upperLabelString.replace("Distribution", tr("Distribution"));
      if(upperLabelString.indexOf("blocks of") != -1){
        //3ブロック　ｘ　５サンプル
        if(upperLabelString.indexOf("FFT resolution") == -1){
            upperLabelString.replace("blocks of", tr("blocks of"));
            upperLabelString.replace("samples", tr("samples"));
        }
      }
      painter->drawText(curve_w / 2 - 200, 20, 400, 16, Qt::AlignCenter | Qt::TextSingleLine, upperLabelString);
  }

  if(lowerlabel[0] != 0)
  {
    QString lowerLabelString(lowerlabel);
    lowerLabelString.replace("Frequency", tr("Frequency"));
    painter->drawText(curve_w / 2 - 80, curve_h - 20, 160, 16, Qt::AlignCenter | Qt::TextSingleLine, lowerLabelString);
  }

  /////////////////////////////////// draw the buttons ///////////////////////////////////////////

  painter->setPen(Qt::black);
  painter->setFont(*sigcurve_font);
  if(printEnabled == true)
  {
    painter->fillRect(curve_w - 345, 20, 85, 24, Qt::gray);
    if(tr("Export") == "Export") {
        painter->drawText(curve_w - 343 + 25, 36, tr("Export"));
    }else {
        painter->drawText(curve_w - 343 + 17, 36, tr("Export"));
    }

  }
  if(cursorEnabled == true)
  {
    painter->fillRect(curve_w - 245, 20, 85, 24, Qt::gray);
    if(tr("Export") == "Export") {
      painter->drawText(curve_w - 228 - 4, 36, tr("Show Cursor"));
    } else {
      painter->drawText(curve_w - 228 + 12, 36, tr("Show Cursor"));
    }
  }
  if(dashBoardEnabled == true)
  {
    painter->fillRect(curve_w - 145, 20, 85, 24, Qt::gray);

    if(tr("Export") == "Export") {
        painter->drawText(curve_w - 128 - 6, 36, tr("Show Controls"));
    }else{
        painter->drawText(curve_w - 128 + 4, 36, tr("Show Controls"));
    }
  }

/////////////////////////////////// translate coordinates, draw and fill a rectangle ///////////////////////////////////////////

  painter->translate(QPoint(bordersize, bordersize));

  curve_w -= (bordersize * 2);

  curve_h -= (bordersize * 2);

  painter->fillRect(0, 0, curve_w, curve_h, BackgroundColor);

  painter->setClipping(true);
  painter->setClipRegion(QRegion(0, 0, curve_w, curve_h), Qt::ReplaceClip);

/////////////////////////////////// draw the colorbars /////////////////////////////////////////

  int t;

  if(spectrum_color != NULL)
  {
    if(spectrum_color->auto_adjust)
    {
      spectrum_color->max_colorbar_value = 0.0;
    }
    sum_colorbar_value = 0.0;

    for(i=0; i < spectrum_color->items; i++)
    {
      sum_colorbar_value += spectrum_color->value[i];

      if(spectrum_color->auto_adjust)
      {
        if(spectrum_color->value[i] > spectrum_color->max_colorbar_value)
        {
          spectrum_color->max_colorbar_value = spectrum_color->value[i];
        }
      }
    }

    if(spectrum_color->auto_adjust)
    {
      spectrum_color->max_colorbar_value *= 1.05;
    }

    pixelsPerUnit = (double)curve_w / (h_ruler_endvalue - h_ruler_startvalue);

    if((spectrum_color->freq[0] > h_ruler_startvalue) && (spectrum_color->items > 1))
    {
      t = (spectrum_color->max_colorbar_value - spectrum_color->value[0]) * ((double)curve_h / spectrum_color->max_colorbar_value);
      if(t < 0)  t = 0;

      if(t <= curve_h)
      {
        painter->fillRect(0,
                          t,
                          (spectrum_color->freq[0] - h_ruler_startvalue) * pixelsPerUnit,
                          curve_h - t,
                          (Qt::GlobalColor)spectrum_color->color[0]);
      }

      strcpy(str, spectrum_color->label[0]);
      if(spectrum_color->method == 0)
      {
        sprintf(str + strlen(str), " (%.1f%%)", (spectrum_color->value[0] * 100.0) / sum_colorbar_value);
      }
      painter->setPen(Qt::lightGray);
      painter->drawText(10, 20, str);
    }

    for(i=1; i < spectrum_color->items; i++)
    {
      if(spectrum_color->freq[i] > h_ruler_startvalue)
      {
        t = (spectrum_color->max_colorbar_value - spectrum_color->value[i]) * ((double)curve_h / spectrum_color->max_colorbar_value);
        if(t < 0)  t = 0;

        if(t <= curve_h)
        {
          painter->fillRect((spectrum_color->freq[i-1] - h_ruler_startvalue) * pixelsPerUnit,
                            t,
                            (spectrum_color->freq[i] - spectrum_color->freq[i-1]) * pixelsPerUnit,
                            curve_h - t,
                            (Qt::GlobalColor)spectrum_color->color[i]);
        }

        strcpy(str, spectrum_color->label[i]);
        if(spectrum_color->method == 0)
        {
          sprintf(str + strlen(str), " (%.1f%%)", (spectrum_color->value[i] * 100.0) / sum_colorbar_value);
        }
        painter->setPen(Qt::lightGray);
        painter->drawText((spectrum_color->freq[i-1] - h_ruler_startvalue) * pixelsPerUnit + 10, 20 + ((i%2) * 20), str);
      }
    }

    if(spectrum_color->method == 0)
    {
      sprintf(str, "Total Power: %.1f", sum_colorbar_value);
      painter->drawText(curve_w - 200, 20 + ((i%2) * 20), str);
    }
  }

/////////////////////////////////// draw the curve ///////////////////////////////////////////

  if(drawcurve_before_raster)
  {
    if((dbuf == NULL) && (ibuf == NULL) && (fbuf == NULL)) return;

    if(max_value <= min_value)  return;

    if(bufsize < 2)  return;

    if(curveUpSideDown == true)
    {
      offset = (-(min_value));

      v_sens = curve_h / (max_value - min_value);
    }
    else
    {
      offset = (-(max_value));

      v_sens = (-(curve_h / (max_value - min_value)));
    }

    h_step = (double)curve_w / (double)bufsize;

    painter->setPen(QPen(QBrush(SignalColor, Qt::SolidPattern), tracewidth, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin));

    if(dbuf)
    {
      for(i = 0; i < bufsize; i++)
      {
        if(fillsurface)
        {
          if(bufsize < curve_w)
          {
            for(j = 0; j < h_step; j++)
            {
              painter->drawLine((i * h_step) + j, (dbuf[i] + offset) * v_sens, (i * h_step) + j, curve_h);
            }
          }
          else
          {
            painter->drawLine(i * h_step, (dbuf[i] + offset) * v_sens, i * h_step, curve_h);
          }
        }
        else
        {
          if(bufsize < (curve_w / 2))
          {
            painter->drawLine(i * h_step, (dbuf[i] + offset) * v_sens, (i + 1) * h_step, (dbuf[i] + offset) * v_sens);
            if(i)
            {
              painter->drawLine(i * h_step, (dbuf[i - 1] + offset) * v_sens, i * h_step, (dbuf[i] + offset) * v_sens);
            }
          }
          else
          {
            if(i < (bufsize - 1))
            {
              {
                painter->drawLine(i * h_step, (dbuf[i] + offset) * v_sens, (i + 1) * h_step, (dbuf[i + 1] + offset) * v_sens);
              }
            }
          }
        }

        if(crosshair_1_active)
        {
          if(i==((int)(((double)crosshair_1_x_position) / h_step)))
          {
            crosshair_1_y_value = (dbuf[i] + offset) * v_sens;
            crosshair_1_value = dbuf[i];
            value = (h_ruler_endvalue - h_ruler_startvalue) / bufsize;
            crosshair_1_value_2 = (i * value) + h_ruler_startvalue;
          }
        }
      }
    }

    if(ibuf)
    {
      for(i = 0; i < bufsize; i++)
      {
        if(fillsurface)
        {
          if(bufsize < curve_w)
          {
            for(j = 0; j < h_step; j++)
            {
              painter->drawLine((i * h_step) + j, ((double)(ibuf[i]) + offset) * v_sens, (i * h_step) + j, curve_h);
            }
          }
          else
          {
            painter->drawLine(i * h_step, ((double)(ibuf[i]) + offset) * v_sens, i * h_step, curve_h);
          }
        }
        else
        {
          if(bufsize < (curve_w / 2))
          {
            painter->drawLine(i * h_step, ((double)(ibuf[i]) + offset) * v_sens, (i + 1) * h_step, ((double)(ibuf[i]) + offset) * v_sens);
            if(i)
            {
              painter->drawLine(i * h_step, ((double)(ibuf[i - 1]) + offset) * v_sens, i * h_step, ((double)(ibuf[i]) + offset) * v_sens);
            }
          }
          else
          {
            if(i < (bufsize - 1))
            {
              {
                painter->drawLine(i * h_step, ((double)(ibuf[i]) + offset) * v_sens, (i + 1) * h_step, ((double)(ibuf[i + 1]) + offset) * v_sens);
              }
            }
          }
        }

        if(crosshair_1_active)
        {
          if(i==((int)(((double)crosshair_1_x_position) / h_step)))
          {
            crosshair_1_y_value = ((double)(ibuf[i]) + offset) * v_sens;
            crosshair_1_value = (double)(ibuf[i]);
            value = (h_ruler_endvalue - h_ruler_startvalue) / bufsize;
            crosshair_1_value_2 = (i * value) + h_ruler_startvalue;
          }
        }
      }
    }

    if(fbuf)
    {
      for(i = 0; i < bufsize; i++)
      {
        if(fillsurface)
        {
          if(bufsize < curve_w)
          {
            for(j = 0; j < h_step; j++)
            {
              painter->drawLine((i * h_step) + j, (fbuf[i] + offset) * v_sens, (i * h_step) + j, curve_h);
            }
          }
          else
          {
            painter->drawLine(i * h_step, (fbuf[i] + offset) * v_sens, i * h_step, curve_h);
          }
        }
        else
        {
          if(bufsize < (curve_w / 2))
          {
            painter->drawLine(i * h_step, (fbuf[i] + offset) * v_sens, (i + 1) * h_step, (fbuf[i] + offset) * v_sens);
            if(i)
            {
              painter->drawLine(i * h_step, (fbuf[i - 1] + offset) * v_sens, i * h_step, (fbuf[i] + offset) * v_sens);
            }
          }
          else
          {
            if(i < (bufsize - 1))
            {
              {
                painter->drawLine(i * h_step, (fbuf[i] + offset) * v_sens, (i + 1) * h_step, (fbuf[i + 1] + offset) * v_sens);
              }
            }
          }
        }

        if(crosshair_1_active)
        {
          if(i==((int)(((double)crosshair_1_x_position) / h_step)))
          {
            crosshair_1_y_value = (fbuf[i] + offset) * v_sens;
            crosshair_1_value = fbuf[i];
            value = (h_ruler_endvalue - h_ruler_startvalue) / bufsize;
            crosshair_1_value_2 = (i * value) + h_ruler_startvalue;
          }
        }
      }
    }
  }

/////////////////////////////////// draw the rasters ///////////////////////////////////////////

  painter->setPen(RasterColor);

  painter->drawRect (0, 0, curve_w - 1, curve_h - 1);

  for(lk = (p_ruler_startvalue / p_divisor) * p_divisor; lk <= p_ruler_endvalue; lk += p_divisor)
  {
    if(lk < p_ruler_startvalue)
    {
      continue;
    }

    p_tmp = (double)(lk - p_ruler_startvalue) * p_pixels_per_unit;

    painter->drawLine(p_tmp, 0, p_tmp, curve_h);
  }

  for(lk = (p2_ruler_startvalue / p2_divisor) * p2_divisor; lk <= p2_ruler_endvalue; lk += p2_divisor)
  {
    if(lk < p2_ruler_startvalue)
    {
      continue;
    }

    p2_tmp = (double)(lk - p2_ruler_startvalue) * p2_pixels_per_unit;

    if(curveUpSideDown == false)
    {
      painter->drawLine(0, curve_h - p2_tmp, curve_w, curve_h - p2_tmp);
    }
    else
    {
      painter->drawLine(0, p2_tmp, curve_w, p2_tmp);
    }
  }

/////////////////////////////////// draw the curve ///////////////////////////////////////////

  if(!drawcurve_before_raster)
  {
    if((dbuf == NULL) && (ibuf == NULL) && (fbuf == NULL)) return;

    if(max_value <= min_value)  return;

    if(bufsize < 2)  return;

    if(curveUpSideDown == true)
    {
      offset = (-(min_value));

      v_sens = curve_h / (max_value - min_value);
    }
    else
    {
      offset = (-(max_value));

      v_sens = (-(curve_h / (max_value - min_value)));
    }

    h_step = (double)curve_w / (double)bufsize;

    painter->setPen(QPen(QBrush(SignalColor, Qt::SolidPattern), tracewidth, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin));

    if(dbuf)
    {
      for(i = 0; i < bufsize; i++)
      {
        if(fillsurface)
        {
          if(bufsize < curve_w)
          {
            for(j = 0; j < h_step; j++)
            {
              painter->drawLine((i * h_step) + j, (dbuf[i] + offset) * v_sens, (i * h_step) + j, curve_h);
            }
          }
          else
          {
            painter->drawLine(i * h_step, (dbuf[i] + offset) * v_sens, i * h_step, curve_h);
          }
        }
        else
        {
          if(bufsize < (curve_w / 2))
          {
            painter->drawLine(i * h_step, (dbuf[i] + offset) * v_sens, (i + 1) * h_step, (dbuf[i] + offset) * v_sens);
            if(i)
            {
              painter->drawLine(i * h_step, (dbuf[i - 1] + offset) * v_sens, i * h_step, (dbuf[i] + offset) * v_sens);
            }
          }
          else
          {
            if(i < (bufsize - 1))
            {
              painter->drawLine(i * h_step, (dbuf[i] + offset) * v_sens, (i + 1) * h_step, (dbuf[i + 1] + offset) * v_sens);
            }
          }
        }

        if(crosshair_1_active)
        {
          if(i==((int)(((double)crosshair_1_x_position) / h_step)))
          {
            crosshair_1_y_value = (dbuf[i] + offset) * v_sens;
            crosshair_1_value = dbuf[i];
            value = (h_ruler_endvalue - h_ruler_startvalue) / bufsize;
            crosshair_1_value_2 = (i * value) + h_ruler_startvalue;
          }
        }
      }
    }

    if(ibuf)
    {
      for(i = 0; i < bufsize; i++)
      {
        if(fillsurface)
        {
          if(bufsize < curve_w)
          {
            for(j = 0; j < h_step; j++)
            {
              painter->drawLine((i * h_step) + j, ((double)(ibuf[i]) + offset) * v_sens, (i * h_step) + j, curve_h);
            }
          }
          else
          {
            painter->drawLine(i * h_step, ((double)(ibuf[i]) + offset) * v_sens, i * h_step, curve_h);
          }
        }
        else
        {
          if(bufsize < (curve_w / 2))
          {
            painter->drawLine(i * h_step, ((double)(ibuf[i]) + offset) * v_sens, (i + 1) * h_step, ((double)(ibuf[i]) + offset) * v_sens);
            if(i)
            {
              painter->drawLine(i * h_step, ((double)(ibuf[i - 1]) + offset) * v_sens, i * h_step, ((double)(ibuf[i]) + offset) * v_sens);
            }
          }
          else
          {
            if(i < (bufsize - 1))
            {
              {
                painter->drawLine(i * h_step, ((double)(ibuf[i]) + offset) * v_sens, (i + 1) * h_step, ((double)(ibuf[i + 1]) + offset) * v_sens);
              }
            }
          }
        }

        if(crosshair_1_active)
        {
          if(i==((int)(((double)crosshair_1_x_position) / h_step)))
          {
            crosshair_1_y_value = ((double)(ibuf[i]) + offset) * v_sens;
            crosshair_1_value = (double)(ibuf[i]);
            value = (h_ruler_endvalue - h_ruler_startvalue) / bufsize;
            crosshair_1_value_2 = (i * value) + h_ruler_startvalue;
          }
        }
      }
    }

    if(fbuf)
    {
      for(i = 0; i < bufsize; i++)
      {
        if(fillsurface)
        {
          if(bufsize < curve_w)
          {
            for(j = 0; j < h_step; j++)
            {
              painter->drawLine((i * h_step) + j, (fbuf[i] + offset) * v_sens, (i * h_step) + j, curve_h);
            }
          }
          else
          {
            painter->drawLine(i * h_step, (fbuf[i] + offset) * v_sens, i * h_step, curve_h);
          }
        }
        else
        {
          if(bufsize < (curve_w / 2))
          {
            painter->drawLine(i * h_step, (fbuf[i] + offset) * v_sens, (i + 1) * h_step, (fbuf[i] + offset) * v_sens);
            if(i)
            {
              painter->drawLine(i * h_step, (fbuf[i - 1] + offset) * v_sens, i * h_step, (fbuf[i] + offset) * v_sens);
            }
          }
          else
          {
            if(i < (bufsize - 1))
            {
              {
                painter->drawLine(i * h_step, (fbuf[i] + offset) * v_sens, (i + 1) * h_step, (fbuf[i + 1] + offset) * v_sens);
              }
            }
          }
        }

        if(crosshair_1_active)
        {
          if(i==((int)(((double)crosshair_1_x_position) / h_step)))
          {
            crosshair_1_y_value = (fbuf[i] + offset) * v_sens;
            crosshair_1_value = fbuf[i];
            value = (h_ruler_endvalue - h_ruler_startvalue) / bufsize;
            crosshair_1_value_2 = (i * value) + h_ruler_startvalue;
          }
        }
      }
    }
  }

/////////////////////////////////// draw the line ///////////////////////////////////////////

  if(line1Enabled == true)
  {
    painter->setPen(QPen(QBrush(line1Color, Qt::SolidPattern), tracewidth, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin));

    painter->drawLine(line1_start_x * h_step, (line1_start_y + offset) * v_sens, line1_end_x * h_step, (line1_end_y + offset) * v_sens);
  }

/////////////////////////////////// draw the markers ///////////////////////////////////////////

  if(Marker1Enabled == true)
  {
    painter->setPen(Marker1Pen);

    painter->drawLine(curve_w * marker_1_position, 0, curve_w * marker_1_position, curve_h);
  }

  if(Marker2Enabled == true)
  {
    painter->setPen(Marker2Pen);

    painter->drawLine(curve_w * marker_2_position, 0, curve_w * marker_2_position, curve_h);
  }

/////////////////////////////////// draw the cursor ///////////////////////////////////////////

  if(crosshair_1_active)
  {
    painter->setPen(crosshair_1_color);

    QPainterPath path;
    path.moveTo(crosshair_1_x_position, crosshair_1_y_value);
    path.lineTo(crosshair_1_x_position - 4, crosshair_1_y_value - 10);
    path.lineTo(crosshair_1_x_position + 5, crosshair_1_y_value - 10);
    path.lineTo(crosshair_1_x_position, crosshair_1_y_value);
    painter->fillPath(path, QBrush(crosshair_1_color));

    painter->setFont(*sigcurve_font);
    painter->fillRect(crosshair_1_x_position + 6, crosshair_1_y_position - 23, 60, 16, BackgroundColor);
    snprintf(str, 128, "%f", crosshair_1_value);
    painter->drawText(crosshair_1_x_position + 8, crosshair_1_y_position - 10, str);
    painter->fillRect(crosshair_1_x_position + 6, crosshair_1_y_position - 3, 60, 16, BackgroundColor);
    convert_to_metric_suffix(str, crosshair_1_value_2, 3);
    strcat(str, h_label);
    painter->drawText(crosshair_1_x_position + 8, crosshair_1_y_position + 10, str);
  }

}


void SignalOneCurve::drawLine(int start_x, double start_y, int end_x, double end_y, QColor lineColor)
{
  line1Color = lineColor;

  line1_start_x = start_x;
  if(line1_start_x < 0)
  {
    line1_start_x = 0;
  }
  if(line1_start_x >= bufsize)
  {
    line1_start_x = bufsize - 1;
  }

  line1_start_y = start_y;

  line1_end_x = end_x;
  if(line1_end_x < 0)
  {
    line1_end_x = 0;
  }
  if(line1_end_x >= bufsize)
  {
    line1_end_x = bufsize - 1;
  }

  line1_end_y = end_y;

  line1Enabled = true;

  update();
}


void SignalOneCurve::setLineEnabled(bool stat)
{
  line1Enabled = stat;

  update();
}


void SignalOneCurve::drawCurve(double *samplebuf, int bsize, double max_val, double min_val)
{
  dbuf = samplebuf;

  ibuf = NULL;

  fbuf = NULL;

  bufsize = bsize;

  max_value = max_val;

  min_value = min_val;

  update();
}


void SignalOneCurve::drawCurve(int *samplebuf, int bsize, double max_val, double min_val)
{
  ibuf = samplebuf;

  dbuf = NULL;

  fbuf = NULL;

  bufsize = bsize;

  max_value = max_val;

  min_value = min_val;

  update();
}


void SignalOneCurve::drawCurve(float *samplebuf, int bsize, double max_val, double min_val)
{
  fbuf = samplebuf;

  dbuf = NULL;

  ibuf = NULL;

  bufsize = bsize;

  max_value = max_val;

  min_value = min_val;

  update();
}


void SignalOneCurve::setFillSurfaceEnabled(bool enabled)
{
  if(enabled == true)
  {
    fillsurface = 1;
  }
  else
  {
    fillsurface = 0;
  }

  update();
}


void SignalOneCurve::setMarker1Position(double mrk_pos)
{
  marker_1_position = mrk_pos;

  if(marker_1_position > 1.01)  marker_1_position = 1.01;

  if(marker_1_position < 0.0001)  marker_1_position = 0.0001;

  update();
}


void SignalOneCurve::setMarker1Enabled(bool on)
{
  Marker1Enabled = on;
  update();
}


void SignalOneCurve::setMarker1MovableEnabled(bool on)
{
  Marker1MovableEnabled = on;
  update();
}


void SignalOneCurve::setMarker1Color(QColor color)
{
  Marker1Pen.setColor(color);
  update();
}


double SignalOneCurve::getMarker1Position(void)
{
  return marker_1_position;
}


void SignalOneCurve::setMarker2Position(double mrk_pos)
{
  marker_2_position = mrk_pos;

  if(marker_2_position > 1.01)  marker_2_position = 1.01;

  if(marker_2_position < 0.0001)  marker_2_position = 0.0001;

  update();
}


void SignalOneCurve::setMarker2Enabled(bool on)
{
  Marker2Enabled = on;
  update();
}


void SignalOneCurve::setMarker2MovableEnabled(bool on)
{
  Marker2MovableEnabled = on;
  update();
}


void SignalOneCurve::setMarker2Color(QColor color)
{
  Marker2Pen.setColor(color);
  update();
}


double SignalOneCurve::getMarker2Position(void)
{
  return marker_2_position;
}


void SignalOneCurve::setH_RulerValues(double start, double end)
{
  h_ruler_startvalue = start;
  h_ruler_endvalue = end;

  update();
}


void SignalOneCurve::setSignalColor(QColor newColor)
{
  SignalColor = newColor;
  update();
}


void SignalOneCurve::setCrosshairColor(QColor newColor)
{
  crosshair_1_color = newColor;
  update();
}


void SignalOneCurve::setTraceWidth(int tr_width)
{
  tracewidth = tr_width;
  if(tracewidth < 0)  tracewidth = 0;
  update();
}


void SignalOneCurve::setBackgroundColor(QColor newColor)
{
  BackgroundColor = newColor;
  update();
}


void SignalOneCurve::setRasterColor(QColor newColor)
{
  RasterColor = newColor;
  update();
}


void SignalOneCurve::setBorderColor(QColor newColor)
{
  BorderColor = newColor;
  update();
}


void SignalOneCurve::setTextColor(QColor newColor)
{
  TextColor = newColor;
  update();
}


void SignalOneCurve::setBorderSize(int newsize)
{
  bordersize = newsize;
  if(bordersize < 0)  bordersize = 0;
  update();
}


void SignalOneCurve::setH_label(const char *str)
{
  strncpy(h_label, str, 32);
  h_label[31] = 0;
  update();
}


void SignalOneCurve::setV_label(const char *str)
{
  strncpy(v_label, str, 20);
  v_label[20] = 0;
  update();
}


void SignalOneCurve::setUpperLabel1(const char *str)
{
  strncpy(upperlabel1, str, 64);
  upperlabel1[63] = 0;
  update();
}


void SignalOneCurve::setUpperLabel2(const char *str)
{
  strncpy(upperlabel2, str, 64);
  upperlabel2[63] = 0;
  update();
}


void SignalOneCurve::setLowerLabel(const char *str)
{
  strncpy(lowerlabel, str, 64);
  lowerlabel[63] = 0;
  update();
}


void SignalOneCurve::setCursorEnabled(bool value)
{
  cursorEnabled = value;
}


bool SignalOneCurve::isCursorEnabled(void)
{
  return cursorEnabled;
}


bool SignalOneCurve::isCursorActive(void)
{
  return crosshair_1_active;
}


void SignalOneCurve::setPrintEnabled(bool value)
{
  printEnabled = value;
}


void SignalOneCurve::setDashBoardEnabled(bool value)
{
  dashBoardEnabled = value;
}


void SignalOneCurve::enableSpectrumColors(struct spectrum_markersblock *spectr_col)
{
  spectrum_color = spectr_col;
  update();
}


void SignalOneCurve::disableSpectrumColors()
{
  spectrum_color = NULL;
  update();
}


void SignalOneCurve::setV_rulerEnabled(bool value)
{
  if(value == true)
  {
    drawVruler = 1;
  }
  else
  {
    drawVruler = 0;
  }
}


void SignalOneCurve::setUpsidedownEnabled(bool value)
{
  curveUpSideDown = value;
}


int SignalOneCurve::getCursorPosition(void)
{
  return crosshair_1_value_2;
}


  /* size is size of destination, returns length of directory */
  /* last character of destination is not a slash! */
int SignalOneCurve::get_directory_from_path(char *dest, const char *src, int ssize)
{
  int i, len;

  if(ssize<1)
  {
    return -1;
  }

  if(ssize<2)
  {
    dest[0] = 0;

    return 0;
  }

  len = strlen(src);

  if(len < 1)
  {
    dest[0] = 0;

    return 0;
  }

  for(i=len-1; i>=0; i--)
  {
    if((src[i]=='/') || (src[i]=='\\'))
    {
      break;
    }
  }

  strncpy(dest, src, ssize);

  if(i < ssize)
  {
    dest[i] = 0;
  }
  else
  {
    dest[ssize-1] = 0;
  }

  return strlen(dest);
}

