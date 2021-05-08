/*
    jTDCviewer

    Main class.

    Copyright (C) 2015 Maxim Singer

    License: GNU GPLv3 (https://www.gnu.org/licenses/gpl-3.0.html)
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "MbsClient/mbsclient.h"
#include "VmeModules/readoutmarkers.h"
#include "VmeModules/elb_vfb6_unpacker.h"
#include "NucMath/src/minimizer.h"
#include "NucMath/src/functions.h"
#include "NucMath/src/hist.h"
#include "NucMath/src/hist2.h"

#include "qcustomplotzoom/qcustomplotzoom.h"


#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <fstream>
#include <ostream>
#include <algorithm>
#include <numeric>
#include <array>
#include <map>
#include <time.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pB_loadJTDCFile_clicked();

    void on_pB_refresh_clicked();

    void on_pB_saveSpectrum_clicked();

    void on_pB_fit_clicked();

    void analyseIncommingData();

    void on_pB_loadJTDCFileAsLMD_clicked();

    void on_sB_TDC_valueChanged(int arg1);

    void on_sB_tdcChannel_valueChanged(int arg1);

    void on_pB_minus_clicked();

    void on_pB_plus_clicked();

    void on_pB_timeSpacing_clicked();

private:
    Ui::MainWindow *ui;


    void drawPlots();

    QTimer *analysisTimer;
    MbsClient mbsclient;
    size_t dataBufferSize;
    uint32_t *dataBuffer;

    std::default_random_engine generator;

    static const size_t maxNumOfTDC = 20;
    std::array< ElbVfb6Unpacker*, maxNumOfTDC> unpacker;

    std::array< std::vector< std::shared_ptr< ElbVfb6Unpacker::Vfb6Event> >, maxNumOfTDC> event_list;

    nucmath::Hist hist;
    std::array< std::array<size_t, ElbVfb6Unpacker::nChannels>, maxNumOfTDC> channelEntries;

    nucmath::Hist2 hist2d;
    QCPColorMap *colorMap;

    void plotCalibrationTables(size_t tdc, size_t channel);

    nucmath::Hist interventclockHist;
    std::array<int64_t, maxNumOfTDC> last_intereventclock;

    size_t noHitInChannelError[2];
};

#endif // MAINWINDOW_H
