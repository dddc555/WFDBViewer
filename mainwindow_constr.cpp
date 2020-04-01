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



UI_Mainwindow::UI_Mainwindow()
{
  int i, j, k;

  setMinimumSize(1024, 480);
  setWindowTitle(PROGRAM_NAME);
  setWindowIcon(QIcon(":/images/edf.png"));

  myfont = new QFont;

  monofont = new QFont;

#ifdef Q_OS_WIN32
  myfont->setFamily("Tahoma");
  myfont->setPixelSize(11);

  monofont->setFamily("courier");
  monofont->setPixelSize(12);
#else
  myfont->setFamily("Arial");
  myfont->setPixelSize(12);

  monofont->setFamily("andale mono");
  monofont->setPixelSize(12);
#endif

  QApplication::setFont(*myfont);

  setlocale(LC_NUMERIC, "C");

  pixelsizefactor = 0.0294382;

  x_pixelsizefactor = 0.0294382;

  viewtime_indicator_type = 1;

  mainwindow_title_type = 1;

  check_for_updates = 1;

  use_threads = 0;

  auto_dpi = 1;

  show_annot_markers = 1;

  show_baselines = 1;

  clip_to_pane = 0;

  annotations_onset_relative = 1;

  annotations_show_duration = 1;

  annotations_duration_background_type = 0;

  auto_reload_mtg = 1;

  read_biosemi_status_signal = 1;

  read_nk_trigger_signal = 1;

  maxfilesize_to_readin_annotations = 10485760000LL;

  exit_in_progress = 0;

  live_stream_active = 0;

  playback_realtime_active = 0;

  signal_averaging_active = 0;

  live_stream_update_interval = 500;

  powerlinefreq = 50;

  mousewheelsens = 10;

  amplitude_doubler = 10;

  timescale_doubler = 10;

  default_amplitude = 100;

  linear_interpol = 0;

  toolbar_stats.sz = 0;
  toolbar_stats.active = 0;
  toolbar_stats.annot_label[0] = 0;
  toolbar_stats.annot_list = NULL;

  recent_montagedir[0] = 0;
  recent_savedir[0] = 0;
  recent_opendir[0] = 0;
  recent_colordir[0] = 0;
  cfg_app_version[0] = 0;

  for(i=0; i<MAXSPECTRUMDIALOGS; i++)
  {
    spectrumdialog[i] = NULL;
  }

  for(i=0; i<MAXAVERAGECURVEDIALOGS; i++)
  {
    averagecurvedialog[i] = NULL;
  }

  for(i=0; i<MAXZSCOREDIALOGS; i++)
  {
    zscoredialog[i] = NULL;
  }

  spectrum_colorbar = (struct spectrum_markersblock *)calloc(1, sizeof(struct spectrum_markersblock));
  for(i=0; i < MAXSPECTRUMMARKERS; i++)
  {
    spectrum_colorbar->freq[i] = 1.0;
    spectrum_colorbar->color[i] = Qt::white;
  }
  spectrum_colorbar->items = 5;
  spectrum_colorbar->freq[0] = 4.0;
  spectrum_colorbar->freq[1] = 8.0;
  spectrum_colorbar->freq[2] = 12.0;
  spectrum_colorbar->freq[3] = 30.0;
  spectrum_colorbar->freq[4] = 100.0;
  spectrum_colorbar->color[0] = Qt::darkRed;
  spectrum_colorbar->color[1] = Qt::darkGreen;
  spectrum_colorbar->color[2] = Qt::darkBlue;
  spectrum_colorbar->color[3] = Qt::darkCyan;
  spectrum_colorbar->color[4] = Qt::darkMagenta;
  spectrum_colorbar->method = 0;
  spectrum_colorbar->auto_adjust = 1;
  spectrum_colorbar->max_colorbar_value = 10.0;

  maxdftblocksize = 1000;

  import_annotations_var = (import_annotations_var_block *)calloc(1, sizeof(struct import_annotations_var_block));
  strcpy(import_annotations_var->separator, "tab");
  import_annotations_var->format = 1;
  import_annotations_var->onsettimeformat = 0;
  import_annotations_var->onsetcolumn = 1;
  import_annotations_var->descriptioncolumn = 2;
  import_annotations_var->useduration = 0;
  import_annotations_var->durationcolumn = 3;
  import_annotations_var->datastartline = 1;
  import_annotations_var->dceventbittime = 10;
  import_annotations_var->triggerlevel = 500.0;
  import_annotations_var->manualdescription = 0;
  import_annotations_var->description[0] = 0;
  import_annotations_var->ignoreconsecutive = 0;

  export_annotations_var = (export_annotations_var_block *)calloc(1, sizeof(struct export_annotations_var_block));
  export_annotations_var->separator = 0;
  export_annotations_var->format = 1;
  export_annotations_var->duration = 0;

  average_period = 300;

  average_ratio = 0;

  average_upsidedown = 0;

  average_bw = 0;

  spectrum_bw = 0;

  spectrum_sqrt = 0;

  spectrum_vlog = 0;

  spectrumdock_sqrt = 0;

  spectrumdock_vlog = 0;

  spectrumdock_colorbars = 0;

  z_score_var.crossoverfreq = 7.5;
  z_score_var.z_threshold = 0.0;
  z_score_var.zscore_page_len = 30;
  z_score_var.zscore_error_detection = 80;
  z_score_var.z_hysteresis = 0.0;

  raw2edf_var.sf = 256;
  raw2edf_var.chns = 1;
  raw2edf_var.phys_max = 3000;
  raw2edf_var.straightbinary = 0;
  raw2edf_var.endianness = 0;
  raw2edf_var.samplesize = 2;
  raw2edf_var.offset = 0;
  raw2edf_var.skipblocksize = 0;
  raw2edf_var.skipbytes = 1;
  strcpy(raw2edf_var.phys_dim, "uV");

  annot_filter = (struct annot_filter_struct *)calloc(1, sizeof(struct annot_filter_struct));
  annot_filter->tmin = 1000;
  annot_filter->tmax = 2000;
  annot_filter->invert = 0;
  annot_filter->hide_other = 1;
  annot_filter->hide_in_list_only = 1;

  vlc_sock = NULL;

  read_general_settings();

  maincurve = new ViewCurve(this);

  dpix = maincurve->logicalDpiX();

  dpiy = maincurve->logicalDpiY();

  if(auto_dpi)
  {
    pixelsizefactor = 1.0 / ((double)dpiy / 2.54);

    x_pixelsizefactor = 1.0 / ((double)dpix / 2.54);
  }

  read_color_settings();

  video_player = (struct video_player_struct *)calloc(1, sizeof(struct video_player_struct));
  video_player->poll_timer = 100;

  live_stream_timer = new QTimer;
  live_stream_timer->setSingleShot(true);
  QObject::connect(live_stream_timer, SIGNAL(timeout()), this, SLOT(live_stream_timer_func()));

  video_poll_timer = new QTimer;
  video_poll_timer->setSingleShot(true);
  QObject::connect(video_poll_timer, SIGNAL(timeout()), this, SLOT(video_poll_timer_func()));

  playback_realtime_time = new QTime();

  playback_realtime_timer = new QTimer;
  playback_realtime_timer->setInterval(20);
  QObject::connect(playback_realtime_timer, SIGNAL(timeout()), this, SLOT(playback_realtime_timer_func()));

#if QT_VERSION >= 0x050000
  live_stream_timer->setTimerType(Qt::PreciseTimer);
  video_poll_timer->setTimerType(Qt::PreciseTimer);
  playback_realtime_timer->setTimerType(Qt::PreciseTimer);
#endif

  setCentralWidget(maincurve);

  menubar = menuBar();



  recent_filesmenu = new QMenu(this);
  recent_filesmenu->setTitle(tr("Recent files"));
  connect(recent_filesmenu, SIGNAL(triggered(QAction *)), this, SLOT(recent_file_action_func(QAction *)));

  close_filemenu = new QMenu(this);
  close_filemenu->setTitle(tr("Close"));
  connect(close_filemenu, SIGNAL(triggered(QAction *)), this, SLOT(close_file_action_func(QAction *)));

  print_img_menu = new QMenu(this);
  print_img_menu->setTitle(tr("to Image"));
  print_img_menu->addAction("640 x 480",   this, SLOT(print_to_img_640x480()));
  print_img_menu->addAction("800 x 600",   this, SLOT(print_to_img_800x600()));
  print_img_menu->addAction("1024 x 768",  this, SLOT(print_to_img_1024x768()));
  print_img_menu->addAction("1280 x 1024", this, SLOT(print_to_img_1280x1024()));
  print_img_menu->addAction("1600 x 1200", this, SLOT(print_to_img_1600x1200()));

  printmenu = new QMenu(this);
  printmenu->setTitle(tr("Export"));
  to_Printer_act = new QAction(tr("to Printer"));

  printmenu->addAction(to_Printer_act);
  to_Printer_act->setShortcut(QKeySequence::Print);
  connect(to_Printer_act, SIGNAL(triggered()), maincurve, SLOT(print_to_printer()));

#if QT_VERSION < 0x050000
  printmenu->addAction("to PostScript", maincurve, SLOT(print_to_postscript()));
#endif

  printmenu->addMenu(print_img_menu);

  to_EDF_act = new QAction(tr("to EDF"));
  printmenu->addAction(to_EDF_act);
  connect(to_EDF_act, SIGNAL(triggered()), this, SLOT(print_to_edf()));

  to_BDF_act = new QAction(tr("to BDF"));
  printmenu->addAction(to_BDF_act);
  connect(to_BDF_act, SIGNAL(triggered()), this, SLOT(print_to_bdf()));

  to_CSV_act = new QAction(tr("to CSV"));
  printmenu->addAction(to_CSV_act);
  connect(to_CSV_act, SIGNAL(triggered()), this, SLOT(print_to_csv()));

  save_act = new QAction(tr("Save as"), this);
  save_act->setShortcut(QKeySequence::Save);
  save_act->setEnabled(false);
  connect(save_act, SIGNAL(triggered()), this, SLOT(save_file()));

  video_act = new QAction("Start video", this);
  connect(video_act, SIGNAL(triggered()), this, SLOT(start_stop_video()));
  video_act->setShortcut(QKeySequence("Ctrl+Shift+V"));

  filemenu = new QMenu(this);


  filemenu->setTitle("&"+tr("File"));

  Open_WFDB_act = new QAction(tr("Open WFDB"));
  filemenu->addAction(Open_WFDB_act);
  Open_WFDB_act->setShortcut(QKeySequence::Open);
  connect(Open_WFDB_act, SIGNAL(triggered()), this, SLOT(open_wfdb_file()));

  filemenu->addSeparator();
  filemenu->addAction(save_act);
  filemenu->addMenu(recent_filesmenu);
  filemenu->addMenu(printmenu);
  exit_act = new QAction(tr("Exit"));
  filemenu->addAction(exit_act);
  exit_act->setShortcut( QKeySequence::Quit);
  connect(exit_act, SIGNAL(triggered()), this, SLOT(exit_program()));

  menubar->addMenu(filemenu);

  signalmenu = new QMenu(this);
  signalmenu->setTitle("&" + tr("Signals"));
  signal_Properties_act = new QAction(tr("Properties"));
  signalmenu->addAction(signal_Properties_act);
  connect(signal_Properties_act, SIGNAL(triggered()), this, SLOT(signalproperties_dialog()));

  signal_add_act = new QAction(tr("Add"));
  signalmenu->addAction(signal_add_act);
  connect(signal_add_act, SIGNAL(triggered()), this, SLOT(add_signals_dialog()));

  signal_organize_act = new QAction(tr("Organize"));
  signalmenu->addAction(signal_organize_act);
  connect(signal_organize_act, SIGNAL(triggered()), this, SLOT(organize_signals()));

  signal_remove_all_act = new QAction(tr("Remove all"));
  signalmenu->addAction(signal_remove_all_act);
  connect(signal_remove_all_act, SIGNAL(triggered()), this, SLOT(remove_all_signals()));

//  menubar->addMenu(signalmenu);

  displaymenu = new QMenu(this);
  displaymenu->setTitle(tr("Timescale"));

  displaymenu->addAction(tr("3 cm/sec"),  this, SLOT(page_3cmsec()));
  displaymenu->addAction(tr("25 mm/sec"), this, SLOT(page_25mmsec()));
  displaymenu->addAction(tr("50 mm/sec"), this, SLOT(page_50mmsec()));

  displaymenu->addSeparator();

  page_10u = new QAction(tr("10 uSec/page"), this);
  displaymenu->addAction(page_10u);

  page_20u = new QAction(tr("20 uSec/page"), this);
  displaymenu->addAction(page_20u);

  page_50u = new QAction(tr("50 uSec/page"), this);
  displaymenu->addAction(page_50u);

  page_100u = new QAction(tr("100 uSec/page"), this);
  displaymenu->addAction(page_100u);

  page_200u = new QAction(tr("200 uSec/page"), this);
  displaymenu->addAction(page_200u);

  page_500u = new QAction(tr("500 uSec/page"), this);
  displaymenu->addAction(page_500u);

  page_1m = new QAction(tr("1 mSec/page"), this);
  displaymenu->addAction(page_1m);

  page_2m = new QAction(tr("2 mSec/page"), this);
  displaymenu->addAction(page_2m);

  page_5m = new QAction(tr("5 mSec/page"), this);
  displaymenu->addAction(page_5m);

  page_10m = new QAction(tr("10 mSec/page"), this);
  displaymenu->addAction(page_10m);

  page_20m = new QAction(tr("20 mSec/page"), this);
  displaymenu->addAction(page_20m);

  page_50m = new QAction(tr("50 mSec/page"), this);
  displaymenu->addAction(page_50m);

  page_100m = new QAction(tr("100 mSec/page"), this);
  displaymenu->addAction(page_100m);

  page_200m = new QAction(tr("200 mSec/page"), this);
  displaymenu->addAction(page_200m);

  page_500m = new QAction(tr("500 mSec/page"), this);
  displaymenu->addAction(page_500m);

  page_1 = new QAction(tr("1 Sec/page"), this);
  displaymenu->addAction(page_1);

  page_2 = new QAction(tr("2 Sec/page"), this);
  displaymenu->addAction(page_2);

  page_5 = new QAction(tr("5 Sec/page"), this);
  displaymenu->addAction(page_5);

  page_10 = new QAction(tr("10 Sec/page"), this);
  displaymenu->addAction(page_10);

  page_15 = new QAction(tr("15 Sec/page"), this);
  displaymenu->addAction(page_15);

  page_20 = new QAction(tr("20 Sec/page"), this);
  displaymenu->addAction(page_20);

  page_30 = new QAction(tr("30 Sec/page"), this);
  page_30->setChecked(true);
  displaymenu->addAction(page_30);

  page_60 = new QAction(tr("60 Sec/page"), this);
  displaymenu->addAction(page_60);

  page_300 = new QAction(tr("5 min/page"), this);
  displaymenu->addAction(page_300);

  page_1200 = new QAction(tr("20 min/page"), this);
  displaymenu->addAction(page_1200);

  page_3600 = new QAction(tr("1 hour/page"), this);
  displaymenu->addAction(page_3600);

  displaymenu->addSeparator();

  page_div2 = new QAction(tr("Timescale / 2"), this);
  page_div2->setShortcut(QKeySequence::ZoomIn);
  connect(page_div2, SIGNAL(triggered()), this, SLOT(set_page_div2()));
  displaymenu->addAction(page_div2);

  page_mult2 = new QAction(tr("Timescale x 2"), this);
  page_mult2->setShortcut(QKeySequence::ZoomOut);
  connect(page_mult2, SIGNAL(triggered()), this, SLOT(set_page_mult2()));
  displaymenu->addAction(page_mult2);

  displaymenu->addSeparator();

  page_user_defined = new QAction(tr("user defined"), this);
  connect(page_user_defined, SIGNAL(triggered()), this, SLOT(set_user_defined_display_time()));
  displaymenu->addAction(page_user_defined);

  displaymenu->addSeparator();

  page_whole_rec = new QAction(tr("whole recording"), this);
  connect(page_whole_rec, SIGNAL(triggered()), this, SLOT(set_display_time_whole_rec()));
  displaymenu->addAction(page_whole_rec);

  menubar->addMenu(displaymenu);

  DisplayGroup = new QActionGroup(this);
  DisplayGroup->addAction(page_10u);
  DisplayGroup->addAction(page_20u);
  DisplayGroup->addAction(page_50u);
  DisplayGroup->addAction(page_100u);
  DisplayGroup->addAction(page_200u);
  DisplayGroup->addAction(page_500u);
  DisplayGroup->addAction(page_1m);
  DisplayGroup->addAction(page_2m);
  DisplayGroup->addAction(page_5m);
  DisplayGroup->addAction(page_10m);
  DisplayGroup->addAction(page_20m);
  DisplayGroup->addAction(page_50m);
  DisplayGroup->addAction(page_100m);
  DisplayGroup->addAction(page_200m);
  DisplayGroup->addAction(page_500m);
  DisplayGroup->addAction(page_1);
  DisplayGroup->addAction(page_2);
  DisplayGroup->addAction(page_5);
  DisplayGroup->addAction(page_10);
  DisplayGroup->addAction(page_15);
  DisplayGroup->addAction(page_20);
  DisplayGroup->addAction(page_30);
  DisplayGroup->addAction(page_60);
  DisplayGroup->addAction(page_300);
  DisplayGroup->addAction(page_1200);
  DisplayGroup->addAction(page_3600);
  connect(DisplayGroup, SIGNAL(triggered(QAction *)), this, SLOT(set_display_time(QAction *)));

  amplitudemenu = new QMenu(this);
  amplitudemenu->setTitle("&"+tr("Amplitude"));

  fit_to_pane = new QAction(tr("Fit to pane"), this);
  connect(fit_to_pane, SIGNAL(triggered()), this, SLOT(fit_signals_to_pane()));
  amplitudemenu->addAction(fit_to_pane);

  fit_to_dc = new QAction(tr("Adjust offset"), this);
  connect(fit_to_dc, SIGNAL(triggered()), this, SLOT(fit_signals_dc_offset()));
  amplitudemenu->addAction(fit_to_dc);

  amplitudemenu->addAction(tr("Offset -> 0"), this, SLOT(set_dc_offset_to_zero()));

  amplitudemenu->addSeparator();

  amp_50000 = new QAction("50000", this);
  amplitudemenu->addAction(amp_50000);

  amp_20000 = new QAction("20000", this);
  amplitudemenu->addAction(amp_20000);

  amp_10000 = new QAction("10000", this);
  amplitudemenu->addAction(amp_10000);

  amp_5000 = new QAction("5000", this);
  amplitudemenu->addAction(amp_5000);

  amp_2000 = new QAction("2000", this);
  amplitudemenu->addAction(amp_2000);

  amp_1000 = new QAction("1000", this);
  amplitudemenu->addAction(amp_1000);

  amp_500 = new QAction("500", this);
  amplitudemenu->addAction(amp_500);

  amp_200 = new QAction("200", this);
  amplitudemenu->addAction(amp_200);

  amp_100 = new QAction("100 (50uV/5mm)", this);
  amplitudemenu->addAction(amp_100);

  amp_50 = new QAction("50", this);
  amplitudemenu->addAction(amp_50);

  amp_20 = new QAction("20", this);
  amplitudemenu->addAction(amp_20);

  amp_10 = new QAction("10", this);
  amplitudemenu->addAction(amp_10);

  amp_5 = new QAction("5", this);
  amplitudemenu->addAction(amp_5);

  amp_2 = new QAction("2", this);
  amplitudemenu->addAction(amp_2);

  amp_1 = new QAction("1", this);
  amplitudemenu->addAction(amp_1);

  amp_05 = new QAction("0.5", this);
  amplitudemenu->addAction(amp_05);

  amp_02 = new QAction("0.2", this);
  amplitudemenu->addAction(amp_02);

  amp_01 = new QAction("0.1", this);
  amplitudemenu->addAction(amp_01);

  amp_005 = new QAction("0.05", this);
  amplitudemenu->addAction(amp_005);

  amp_002 = new QAction("0.02", this);
  amplitudemenu->addAction(amp_002);

  amp_001 = new QAction("0.01", this);
  amplitudemenu->addAction(amp_001);

  amp_0005 = new QAction("0.005", this);
  amplitudemenu->addAction(amp_0005);

  amp_0002 = new QAction("0.002", this);
  amplitudemenu->addAction(amp_0002);

  amp_0001 = new QAction("0.001", this);
  amplitudemenu->addAction(amp_0001);

  amp_00005 = new QAction("0.0005", this);
  amplitudemenu->addAction(amp_00005);

  amp_00002 = new QAction("0.0002", this);
  amplitudemenu->addAction(amp_00002);

  amp_00001 = new QAction("0.0001", this);
  amplitudemenu->addAction(amp_00001);

  amplitudemenu->addSeparator();

  amp_plus = new QAction(tr("Amplitude x 2"), this);
  amp_plus->setShortcut(Qt::Key_Minus);
  connect(amp_plus, SIGNAL(triggered()), this, SLOT(set_amplitude_mult2()));
  amplitudemenu->addAction(amp_plus);

  amp_minus = new QAction(tr("Amplitude / 2"), this);
  amp_minus->setShortcut(Qt::Key_Plus);
  connect(amp_minus, SIGNAL(triggered()), this, SLOT(set_amplitude_div2()));
  amplitudemenu->addAction(amp_minus);

  menubar->addMenu(amplitudemenu);

  AmplitudeGroup = new QActionGroup(this);
  AmplitudeGroup->addAction(amp_00001);
  AmplitudeGroup->addAction(amp_00002);
  AmplitudeGroup->addAction(amp_00005);
  AmplitudeGroup->addAction(amp_0001);
  AmplitudeGroup->addAction(amp_0002);
  AmplitudeGroup->addAction(amp_0005);
  AmplitudeGroup->addAction(amp_001);
  AmplitudeGroup->addAction(amp_002);
  AmplitudeGroup->addAction(amp_005);
  AmplitudeGroup->addAction(amp_01);
  AmplitudeGroup->addAction(amp_02);
  AmplitudeGroup->addAction(amp_05);
  AmplitudeGroup->addAction(amp_1);
  AmplitudeGroup->addAction(amp_2);
  AmplitudeGroup->addAction(amp_5);
  AmplitudeGroup->addAction(amp_10);
  AmplitudeGroup->addAction(amp_20);
  AmplitudeGroup->addAction(amp_50);
  AmplitudeGroup->addAction(amp_100);
  AmplitudeGroup->addAction(amp_200);
  AmplitudeGroup->addAction(amp_500);
  AmplitudeGroup->addAction(amp_1000);
  AmplitudeGroup->addAction(amp_2000);
  AmplitudeGroup->addAction(amp_5000);
  AmplitudeGroup->addAction(amp_10000);
  AmplitudeGroup->addAction(amp_20000);
  AmplitudeGroup->addAction(amp_50000);
  connect(AmplitudeGroup, SIGNAL(triggered(QAction *)), this, SLOT(set_amplitude(QAction *)));

  filtermenu = new QMenu(this);
  filtermenu->setTitle("&"+tr("Filter"));

  filter_new_act = new QAction(tr("New"));

  filtermenu->addAction(filter_new_act);
  connect(filter_new_act, SIGNAL(triggered()), this, SLOT(add_new_filter()));

  filter_Adjust_act = new QAction(tr("Adjust"));
  filtermenu->addAction(filter_Adjust_act);
  connect(filter_Adjust_act, SIGNAL(triggered()), this, SLOT(filterproperties_dialog()));

  filter_remove_all_act = new QAction(tr("Remove all"));
  filtermenu->addAction(filter_remove_all_act);
  connect(filter_remove_all_act, SIGNAL(triggered()), this, SLOT(remove_all_filters()));

  filtermenu->addSeparator();
  filter_Powerline_act = new QAction(tr("Powerline interference removal for ECG"));
  filtermenu->addAction(filter_Powerline_act);
  connect(filter_Powerline_act, SIGNAL(triggered()), this, SLOT(add_plif_ecg_filter()));

  filter_remove_all_Powerline_act = new QAction(tr("Remove all Powerline interference filters"));
  filtermenu->addAction(filter_remove_all_Powerline_act);
  connect(filter_remove_all_Powerline_act, SIGNAL(triggered()), this, SLOT(remove_all_plif_ecg_filters()));

  filtermenu->addSeparator();

  filter_customize_fir_act = new QAction(tr("Customizable FIR filter"));
  filtermenu->addAction(filter_customize_fir_act);
  connect(filter_customize_fir_act, SIGNAL(triggered()), this, SLOT(add_fir_filter()));

  filter_remove_all_fir_act = new QAction(tr("Remove all FIR filters"));
  filtermenu->addAction(filter_remove_all_fir_act);
  connect(filter_remove_all_fir_act, SIGNAL(triggered()), this, SLOT(remove_all_fir_filters()));

  filtermenu->addSeparator();

  filter_spike_act = new QAction(tr("Spike"));
  filtermenu->addAction(filter_spike_act);
  connect(filter_spike_act, SIGNAL(triggered()), this, SLOT(add_spike_filter()));

  filter_remove_all_spike_act = new QAction(tr("Remove all spike filters"));
  filtermenu->addAction(filter_remove_all_spike_act);
  connect(filter_remove_all_spike_act, SIGNAL(triggered()), this, SLOT(remove_all_spike_filters()));
  
  menubar->addMenu(filtermenu);

//   math_func_menu = new QMenu(this);
//   math_func_menu->setTitle("&Math");
//   math_func_menu->addAction("New", this, SLOT(add_new_math_func()));
//   math_func_menu->addAction("Remove all", this, SLOT(remove_all_math_funcs()));
//   menubar->addMenu(math_func_menu);

  load_predefined_mtg_act[0] = new QAction("Empty", this);
  load_predefined_mtg_act[0]->setShortcut(Qt::Key_F1);
  load_predefined_mtg_act[1] = new QAction("Empty", this);
  load_predefined_mtg_act[1]->setShortcut(Qt::Key_F2);
  load_predefined_mtg_act[2] = new QAction("Empty", this);
  load_predefined_mtg_act[2]->setShortcut(Qt::Key_F3);
  load_predefined_mtg_act[3] = new QAction("Empty", this);
  load_predefined_mtg_act[3]->setShortcut(Qt::Key_F4);
  load_predefined_mtg_act[4] = new QAction("Empty", this);
  load_predefined_mtg_act[4]->setShortcut(Qt::Key_F5);
  load_predefined_mtg_act[5] = new QAction("Empty", this);
  load_predefined_mtg_act[5]->setShortcut(Qt::Key_F6);
  load_predefined_mtg_act[6] = new QAction("Empty", this);
  load_predefined_mtg_act[6]->setShortcut(Qt::Key_F7);
  load_predefined_mtg_act[7] = new QAction("Empty", this);
  load_predefined_mtg_act[7]->setShortcut(Qt::Key_F8);
  load_predefined_mtg_act[8] = new QAction("Empty", this);
  load_predefined_mtg_act[8]->setShortcut(Qt::Key_F9);
  load_predefined_mtg_act[9] = new QAction("Empty", this);
  load_predefined_mtg_act[9]->setShortcut(Qt::Key_F10);
  load_predefined_mtg_act[10] = new QAction("Empty", this);
  load_predefined_mtg_act[10]->setShortcut(Qt::Key_F11);
  load_predefined_mtg_act[11] = new QAction("Empty", this);
  load_predefined_mtg_act[11]->setShortcut(Qt::Key_F12);
  load_predefined_mtg_group = new QActionGroup(this);
  for(i=0; i < MAXPREDEFINEDMONTAGES; i++)
  {
    load_predefined_mtg_group->addAction(load_predefined_mtg_act[i]);
  }
  connect(load_predefined_mtg_group, SIGNAL(triggered(QAction *)), this, SLOT(load_predefined_mtg(QAction *)));

  montagemenu = new QMenu(this);
  montagemenu->setTitle("&Montage");
  montagemenu->addAction("View this montage", this, SLOT(show_this_montage()));
  montagemenu->addAction("View saved montages", this, SLOT(view_montage()));
  montagemenu->addAction("Save", this, SLOT(save_montage()));
  montagemenu->addAction("Load", this, SLOT(load_montage()));
  montagemenu->addSeparator();
  montagemenu->addAction("Edit key-bindings for montages", this, SLOT(edit_predefined_montages()));
  montagemenu->addSeparator();
  for(i=0; i < MAXPREDEFINEDMONTAGES; i++)
  {
    montagemenu->addAction(load_predefined_mtg_act[i]);
  }
//  menubar->addMenu(montagemenu);

//   patternmenu = new QMenu(this);
//   patternmenu->setTitle("&Pattern");
//   patternmenu->addAction("Search", this, SLOT(search_pattern()));
//   menubar->addMenu(patternmenu);

  toolsmenu = new QMenu(this);
  toolsmenu->setTitle(tr("Tools"));



  tools_detect_qrs_act = new QAction(tr("Detect QRS and generate annotations"));
  toolsmenu->addAction(tools_detect_qrs_act);

  connect(tools_detect_qrs_act, SIGNAL(triggered()), this, SLOT(detect_qrs()));

  toolsmenu->addSeparator();

  tools_option_act = new QAction(("Settings"));
  toolsmenu->addAction(tools_option_act);
  connect(tools_option_act, SIGNAL(triggered()), this, SLOT(show_options_dialog()));

  menubar->addMenu(toolsmenu);

  settingsmenu = new QMenu(this);
  settingsmenu->setTitle("Settings");
  settingsmenu->addAction("Settings", this, SLOT(show_options_dialog()));
//  menubar->addMenu(settingsmenu);

  no_timesync_act = new QAction("no timelock", this);
  no_timesync_act->setCheckable(true);

  offset_timesync_act = new QAction("synchronize start of files (offset)", this);
  offset_timesync_act->setCheckable(true);

  absolut_timesync_act = new QAction("synchronize absolute time", this);
  absolut_timesync_act->setCheckable(true);

  user_def_sync_act = new QAction("user defined synchronizing", this);
  user_def_sync_act->setCheckable(true);

  timelock_act_group = new QActionGroup(this);
  timelock_act_group->addAction(no_timesync_act);
  timelock_act_group->addAction(offset_timesync_act);
  timelock_act_group->addAction(absolut_timesync_act);
  timelock_act_group->addAction(user_def_sync_act);
  absolut_timesync_act->setChecked(true);
  connect(timelock_act_group, SIGNAL(triggered(QAction *)), this, SLOT(set_timesync(QAction *)));

  sel_viewtime_act_group = new QActionGroup(this);
  connect(sel_viewtime_act_group, SIGNAL(triggered(QAction *)), this, SLOT(set_timesync_reference(QAction *)));

  timemenu = new QMenu(this);
  timemenu->setTitle("T&imesync");
  timemenu->addAction("Go to start of file", this, SLOT(jump_to_start()), QKeySequence::MoveToStartOfDocument);
  timemenu->addAction("Go to end of file", this, SLOT(jump_to_end()), QKeySequence::MoveToEndOfDocument);
  timemenu->addAction("Jump to", this, SLOT(jump_to_dialog()));
  timemenu->addSeparator()->setText("Timelock");
  timemenu->addAction(no_timesync_act);
  timemenu->addAction(offset_timesync_act);
  timemenu->addAction(absolut_timesync_act);
  timemenu->addAction(user_def_sync_act);
  timemenu->addSeparator();
  timemenu->addAction("synchronize by crosshairs", this, SLOT(sync_by_crosshairs()));
  timemenu->addSeparator()->setText("Time reference");
//  menubar->addMenu(timemenu);

  windowmenu = new QMenu(this);
  windowmenu->setTitle("&Window");
  windowmenu->addAction("Annotations", this, SLOT(show_annotations()));
  windowmenu->addAction("Annotation editor", this, SLOT(annotation_editor()));
  windowmenu->addAction("Power Spectrum", this, SLOT(show_spectrum_dock()));
//  menubar->addMenu(windowmenu);

  helpmenu = new QMenu(this);
  helpmenu->setTitle("&Help");
//#ifdef Q_OS_LINUX
//  helpmenu->addAction("Manual",  this, SLOT(show_help()));
//#endif
//#ifdef Q_OS_WIN32
//  helpmenu->addAction("Manual",  this, SLOT(show_help()));
//#endif
  helpmenu->addAction("Keyboard shortcuts", this, SLOT(show_kb_shortcuts()));
  helpmenu->addAction("About WFDBbrowser", this, SLOT(show_about_dialog()));
  helpmenu->addAction("Show splashscreen", this, SLOT(show_splashscreen()));
//  menubar->addMenu(helpmenu);
  languagemenu =  new QMenu(this);
  languagemenu ->setTitle("&"+tr("Language"));

  language_en_act = new QAction(tr("English"));
  languagemenu ->addAction(language_en_act);
  connect(language_en_act, SIGNAL(triggered()), this, SLOT(language_en()));

  language_jp_act = new QAction(tr("Japanese"));
  languagemenu ->addAction(language_jp_act);
  connect(language_jp_act, SIGNAL(triggered()), this, SLOT(language_jp()));

  menubar->addMenu(languagemenu);
  navtoolbarGroupWidget = new QWidget(maincurve);
  int wid = 600;
  navtoolbarGroupWidget->setGeometry(100, 0, wid, 60);

  navtoolbar = new QHBoxLayout(navtoolbarGroupWidget);

//  navtoolbar->setGeometry(QRect(0,0,50,500));

//  navtoolbar->setFloatable(false);
//  navtoolbar->setAllowedAreas(Qt::AllToolBarAreas);

//  addToolBar(Qt::BottomToolBarArea, navtoolbar);


//  former_page_Act = new QAction(QIcon(":/images/go-first-symbolic.symbolic.png"), "<<", this);
//  former_page_Act->setToolTip("Former Page");
//  former_page_Act->setShortcut(QKeySequence::MoveToPreviousPage);
//  connect(former_page_Act, SIGNAL(triggered()), this, SLOT(former_page()));
//  navtoolbar->addAction(former_page_Act);

  former_page_Act = new QPushButton(QIcon(":/images/go-first-symbolic.symbolic.png"), "", this);
  QSize *iconSize = new QSize(200,200);
  former_page_Act->setToolTip("Former Page");

  former_page_Act->setIconSize(*iconSize);
  former_page_Act->setShortcut(QKeySequence::MoveToPreviousPage);
  connect(former_page_Act, SIGNAL(clicked()), this, SLOT(former_page()));
  navtoolbar->addWidget(former_page_Act);
  //navtoolbar->addAction(former_page_Act);

  shift_page_left_Act = new QPushButton(QIcon(":/images/go-next-symbolic-rtl.symbolic.png"), "");
  shift_page_left_Act->setToolTip("Shift Left");
  shift_page_left_Act->setIconSize(*iconSize);
  shift_page_left_Act->setShortcut(QKeySequence::MoveToPreviousChar);
  connect(shift_page_left_Act, SIGNAL(clicked()), this, SLOT(shift_page_left()));
  navtoolbar->addWidget(shift_page_left_Act);

  stop_playback_realtime_Act = new QPushButton(QIcon(":/images/media-playback-stop-symbolic.symbolic.png"), "", this);
  stop_playback_realtime_Act->setIconSize(*iconSize);
  connect(stop_playback_realtime_Act, SIGNAL(clicked()), this, SLOT(stop_playback()));
//  navtoolbar->addAction(stop_playback_realtime_Act);
  navtoolbar->addWidget(stop_playback_realtime_Act);

  playback_file_Act = new QPushButton(QIcon(":/images/media-playback-start-symbolic.symbolic.png"), "", this);
  playback_file_Act->setIconSize(*iconSize);
  connect(playback_file_Act, SIGNAL(clicked()), this, SLOT(playback_file()));
//  navtoolbar->addAction(playback_file_Act);
  navtoolbar->addWidget(playback_file_Act);

  shift_page_right_Act = new QPushButton(QIcon(":/images/go-next-symbolic.symbolic.png"), "", this);
  shift_page_right_Act->setToolTip("Shift Right");
  shift_page_right_Act->setShortcut(QKeySequence::MoveToNextChar);
  shift_page_right_Act->setIconSize(*iconSize);
  connect(shift_page_right_Act, SIGNAL(clicked()), this, SLOT(shift_page_right()));
//  navtoolbar->addAction(shift_page_right_Act);
  navtoolbar->addWidget(shift_page_right_Act);

  next_page_Act = new QPushButton(QIcon(":/images/go-first-symbolic-rtl.symbolic.png"), "", this);
  next_page_Act->setToolTip("Next Page");
  next_page_Act->setShortcut(QKeySequence::MoveToNextPage);
  next_page_Act->setIconSize(*iconSize);
  connect(next_page_Act, SIGNAL(clicked()), this, SLOT(next_page()));
//  navtoolbar->addAction(next_page_Act);
  navtoolbar->addWidget(next_page_Act);

  slower_Act = new QAction(QIcon(":/images/media-seek-backward.png"), "<<", this);
  slower_Act->setToolTip("Slower");
//  slower_Act->setShortcut(QKeySequence::MoveToNextPage);
  next_page_Act->setIconSize(*iconSize);
  connect(slower_Act, SIGNAL(triggered()), this, SLOT(video_player_slower()));
//  navtoolbar->addAction(slower_Act);
  slower_Act->setVisible(false);

  faster_Act = new QAction(QIcon(":/images/media-seek-forward.png"), ">>", this);
  faster_Act->setToolTip("Faster");
//  faster_Act->setShortcut(QKeySequence::MoveToNextPage);
  connect(faster_Act, SIGNAL(triggered()), this, SLOT(video_player_faster()));
//  navtoolbar->addAction(faster_Act);
  faster_Act->setVisible(false);

  shift_page_up_Act = new QPushButton(QIcon(":/images/go-up-symbolic.symbolic.png"), "", this);
  shift_page_up_Act->setToolTip("Shift Up");
  shift_page_up_Act->setShortcut(QKeySequence::MoveToPreviousLine);
  shift_page_up_Act->setIconSize(*iconSize);
  connect(shift_page_up_Act, SIGNAL(clicked()), this, SLOT(shift_page_up()));
//  navtoolbar->addAction(shift_page_up_Act);
  navtoolbar->addWidget(shift_page_up_Act);

  shift_page_down_Act = new QPushButton(QIcon(":/images/go-down-symbolic.symbolic.png"), "", this);
  shift_page_down_Act->setToolTip("Shift Down");
  shift_page_down_Act->setShortcut(QKeySequence::MoveToNextLine);
  shift_page_down_Act->setIconSize(*iconSize);
  connect(shift_page_down_Act, SIGNAL(clicked()), this, SLOT(shift_page_down()));
//  navtoolbar->addAction(shift_page_down_Act);
  navtoolbar->addWidget(shift_page_down_Act);

  zoomback_Act = new QPushButton(QIcon(":/images/zoom-out-symbolic.symbolic.png"), "", this);
  zoomback_Act->setShortcut(Qt::Key_Backspace);
  zoomback_Act->setIconSize(*iconSize);
  connect(zoomback_Act, SIGNAL(clicked()), this, SLOT(zoomback()));
//  navtoolbar->addAction(zoomback_Act);
  navtoolbar->addWidget(zoomback_Act);

  zoomforward_Act = new QPushButton(QIcon(":/images/zoom-in-symbolic.symbolic.png"), "", this);
  zoomforward_Act->setShortcut(Qt::Key_Insert);
  zoomforward_Act->setIconSize(*iconSize);
  connect(zoomforward_Act, SIGNAL(clicked()), this, SLOT(forward()));
//  navtoolbar->addAction(zoomforward_Act);
  navtoolbar->addWidget(zoomforward_Act);

  nav_toolbar_label = new QLabel;
  navtoolbar->addWidget(nav_toolbar_label);
//  navtoolbar->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  Escape_act = new QAction(this);
  Escape_act->setShortcut(Qt::Key_Escape);
  connect(Escape_act, SIGNAL(triggered()), this, SLOT(Escape_fun()));
  maincurve->addAction(Escape_act);

  positionslider = new QSlider(Qt::Horizontal);
  positionslider->setRange(0, 1000000);
  positionslider->setSingleStep(10000);
  positionslider->setPageStep(100000);

  slidertoolbar = new QToolBar("Position Slider");
  slidertoolbar->setFloatable(false);
  slidertoolbar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
  slidertoolbar->addWidget(positionslider);
  addToolBar(Qt::BottomToolBarArea, slidertoolbar);
  QObject::connect(positionslider, SIGNAL(valueChanged(int)), this, SLOT(slider_moved(int)));
  slidertoolbar->setEnabled(false);
  positionslider->blockSignals(true);

  insertToolBarBreak(slidertoolbar);

  files_open = 0;
  signalcomps = 0;
  sel_viewtime = 0;
  viewtime_sync = VIEWTIME_SYNCED_ABSOLUT;
  pagetime = 10 * TIME_DIMENSION;
  viewtime_string[0] = 0;
  pagetime_string[0] = 0;
  totalviewbufsize = 0;
  print_to_edf_active = 0;
  annot_editor_active = 0;
  annotations_edited = 0;

  viewbuf = NULL;

  for(i=0; i<MAXFILES; i++)
  {
    annotations_dock[i] = NULL;
  }

  annotationlist_backup = NULL;

  zoomhistory = (struct zoomhistoryblock *)calloc(1, sizeof(struct zoomhistoryblock));

  zoomhistory->history_size_tail = 0;
  zoomhistory->history_size_front = 0;
  for(i=0; i<MAXZOOMHISTORY; i++)
  {
    zoomhistory->pntr = 0;
    zoomhistory->pagetime[i] = 10 * TIME_DIMENSION;
    for(j=0; j<MAXFILES; j++)
    {
      zoomhistory->viewtime[i][j] = 0;
    }
    for(j=0; j<MAXSIGNALS; j++)
    {
      zoomhistory->voltpercm[i][j] = 70.0;
      zoomhistory->screen_offset[i][j] = 0.0;
      for(k=0; k<MAXSIGNALS; k++)
      {
        zoomhistory->sensitivity[i][j][k] = 0.0475;
      }
    }
  }

  path[0] = 0;
  recent_montagedir[0] = 0;
  recent_savedir[0] = 0;
  recent_opendir[0] = 0;
  montagepath[0] = 0;
  option_str[0] = 0;

  for(i=0; i<MAX_RECENTFILES; i++)
  {
    recent_file_path[i][0] = 0;
    recent_file_mtg_path[i][0] = 0;
  }

  for(i=0; i<MAXPREDEFINEDMONTAGES; i++)
  {
    predefined_mtg_path[i][0] = 0;
  }

  read_recent_file_settings();

  for(i=0; i<MAXPREDEFINEDMONTAGES; i++)
  {
    if(predefined_mtg_path[i][0] != 0)
    {
      load_predefined_mtg_act[i]->setText(predefined_mtg_path[i]);
    }
  }

  annotationEditDock = new UI_AnnotationEditwindow(this);

  addDockWidget(Qt::BottomDockWidgetArea, annotationEditDock->dockedit, Qt::Horizontal);

  annotationEditDock->dockedit->hide();

  for(i=0; i<MAXSPECTRUMDOCKS; i++)
  {
    spectrumdock[i] = new UI_SpectrumDockWindow(this);

    addDockWidget(Qt::TopDockWidgetArea, spectrumdock[i]->dock, Qt::Horizontal);

    spectrumdock[i]->dock->hide();
  }

  setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
  setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
  setCorner(Qt::BottomLeftCorner, Qt::BottomDockWidgetArea);
  setCorner(Qt::BottomRightCorner, Qt::BottomDockWidgetArea);

  char tmp_str[MAX_PATH_LENGTH];

  cmdlineargument = 0;

  cmdlineoption = 0;

  if(QCoreApplication::arguments().size()>1)
  {
    strcpy(tmp_str, QCoreApplication::arguments().at(1).toLocal8Bit().data());

    if((strlen(tmp_str) > 2) && (!strncmp(tmp_str, "--", 2)))
    {
      strcpy(option_str, tmp_str);

      cmdlineoption++;
    }
    else
    {
      strcpy(path, tmp_str);

      cmdlineargument++;
    }

    if(QCoreApplication::arguments().size()>2)
    {
      strcpy(tmp_str, QCoreApplication::arguments().at(2).toLocal8Bit().data());

      if(!cmdlineargument)
      {
        strcpy(path, tmp_str);
      }
      else
      {
        strcpy(montagepath, tmp_str);
      }

      cmdlineargument++;

      if(cmdlineargument == 1)
      {
        if(QCoreApplication::arguments().size()>3)
        {
          strcpy(montagepath, QCoreApplication::arguments().at(3).toLocal8Bit().data());

          cmdlineargument++;
        }
      }
    }
  }

  showMaximized();

  oldwindowheight = height();

  if(cmdlineargument)
  {
    if(cmdlineoption)
    {
      if(!strcmp(option_str, "--stream"))
      {
        open_stream();
      }
    }
    else
    {
      open_new_file();
    }
  }

  if((QT_VERSION < MINIMUM_QT4_VERSION) || ((QT_VERSION >= 0x050000) && (QT_VERSION < MINIMUM_QT5_VERSION)))
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "WFDBbrowser has been compiled with a version of Qt\n"
                                                              "which is too old and will likely cause problems!");
    messagewindow.exec();
  }

  int v_nr;

  char v_str[32];

  strncpy(v_str, qVersion(), 32);
  v_str[31] = 0;

  v_nr = 0x10000 * atoi(v_str);
  v_nr += 0x100 * atoi(v_str + 2);
  v_nr += atoi(v_str + 4);

  if((v_nr < MINIMUM_QT4_VERSION) || ((v_nr >= 0x050000) && (v_nr < MINIMUM_QT5_VERSION)))
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "Your version of Qt is too old\n"
                                                              "and will likely cause problems!");
    messagewindow.exec();
  }

  if(edflib_version() != 115)
  {
    QMessageBox messagewindow(QMessageBox::Critical, "Error", "There's a version problem with EDFlib.\n"
                                                              "Can not continue.");
    messagewindow.exec();

    menubar->setEnabled(false);
    navtoolbar->setEnabled(false);
  }

  pixmap = new QPixmap(":/images/splash.png");
  splash = new QSplashScreen(this, *pixmap, Qt::WindowStaysOnTopHint);

  update_checker = NULL;

  if(check_for_updates)
  {
    update_checker = new Check_for_updates;
  }
  loadLanguage();
}

void UI_Mainwindow::updateText(){

  recent_filesmenu->setTitle(tr("Recent files"));
  close_filemenu->setTitle(tr("Close"));
  print_img_menu->setTitle(tr("to Image"));
  printmenu->setTitle(tr("Export"));
  to_Printer_act->setText(tr("to Printer"));

  to_EDF_act->setText(tr("to EDF"));
  to_BDF_act->setText(tr("to BDF"));
  to_CSV_act->setText(tr("to CSV"));
  save_act->setText(tr("Save as"));
  
  filemenu->setTitle("&"+tr("File"));

  Open_WFDB_act->setText(tr("Open WFDB"));
  
  exit_act->setText(tr("Exit"));

  displaymenu->setTitle(tr("Timescale"));
  page_div2->setText(tr("Timescale / 2"));

  page_mult2->setText(tr("Timescale x 2"));
  page_user_defined->setText(tr("user defined"));

  page_whole_rec->setText(tr("whole recording"));
  
  amplitudemenu->setTitle("&"+tr("Amplitude"));
  fit_to_pane->setText(tr("Fit to pane"));
  fit_to_dc->setText(tr("Adjust offset"));

  amp_plus->setText(tr("Amplitude x 2"));
  amp_minus->setText(tr("Amplitude / 2"));

  filtermenu->setTitle("&"+tr("Filter"));
  filter_new_act->setText(tr("New"));
  filter_Adjust_act->setText(tr("Adjust"));
  filter_remove_all_act->setText(tr("Remove all"));
  filter_Powerline_act->setText(tr("Powerline interference removal for ECG"));
  filter_remove_all_Powerline_act->setText(tr("Remove all Powerline interference filters"));
  filter_customize_fir_act->setText(tr("Customizable FIR filter"));
  filter_remove_all_fir_act->setText(tr("Remove all FIR filters"));
  filter_spike_act ->setText(tr("Spike"));
  filter_remove_all_spike_act->setText(tr("Remove all spike filters"));
  tools_detect_qrs_act->setText(tr("Detect QRS and generate annotations"));

  toolsmenu->setTitle(tr("Tools"));

  tools_option_act->setText(tr("Settings"));

  language_en_act->setText(tr("English"));
  language_jp_act->setText(tr("Japanese"));
  languagemenu->setTitle(tr("Language"));

  if(spectrumdock[0] != NULL){
      spectrumdock[0]->updateText();
  }

  if(annotations_dock[0]  != NULL){
      annotations_dock[0]->updateText();
  }

  if(annotationEditDock != NULL){
      annotationEditDock->updateText();
  }
}
