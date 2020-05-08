/*
    jTDCviewer

    Main class.

    Copyright (C) 2015 Maxim Singer

    License: GNU GPLv3 (https://www.gnu.org/licenses/gpl-3.0.html)
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QCPColorGradient colorMapGradient;
    colorMapGradient.clearColorStops();
    colorMapGradient.setColorInterpolation(QCPColorGradient::ciHSV);
    colorMapGradient.setColorStopAt(0, QColor(255, 0, 0, 0));
    colorMapGradient.setColorStopAt(0.0001, QColor(255, 0, 0));
    colorMapGradient.setColorStopAt(1.0/3.0, QColor(0, 0, 255));
    colorMapGradient.setColorStopAt(2.0/3.0, QColor(0, 255, 0));
    colorMapGradient.setColorStopAt(1, QColor(255, 0, 0));
    colorMapGradient.setLevelCount(10);

    QFont font;
    font.setFamily("Sans");
    font.setPixelSize(14);

    QCustomPlotZoom *customPlot = ui->plot1;
    customPlot->addGraph();
    customPlot->addGraph();
    customPlot->graph(0)->setPen(QColor(0, 0, 0, 255));
    customPlot->graph(0)->setLineStyle(QCPGraph::lsStepCenter);
    customPlot->graph(1)->setPen(QColor(255, 50, 50, 255));
    customPlot->xAxis->setLabel("channel time difference in ns");
    customPlot->yAxis->setLabel("counts");
    customPlot->yAxis->setLabelFont(font);
    customPlot->xAxis->setLabelFont(font);
    customPlot->setZoomMode(true);

    customPlot = ui->plot_entries;
    customPlot->addGraph();
    customPlot->graph(0)->setPen(QColor(0, 0, 0, 255));
    customPlot->graph(0)->setLineStyle(QCPGraph::lsStepCenter);
    customPlot->xAxis->setLabel("channel");
    customPlot->yAxis->setLabel("hits");
    customPlot->yAxis->setLabelFont(font);
    customPlot->xAxis->setLabelFont(font);
    customPlot->setZoomMode(true);


    customPlot = ui->plot_timeSpacing;
    customPlot->addGraph();
    customPlot->addGraph();
    customPlot->graph(0)->setPen(QColor(0, 0, 0, 255));
    customPlot->graph(0)->setLineStyle(QCPGraph::lsStepCenter);
    customPlot->graph(1)->setPen(QColor(255, 50, 50, 255));
    customPlot->xAxis->setLabel("channel time difference in ns");
    customPlot->yAxis->setLabel("counts");
    customPlot->yAxis->setLabelFont(font);
    customPlot->xAxis->setLabelFont(font);
    customPlot->setZoomMode(true);

    customPlot = ui->plot_2d;
    customPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom); // this will also allow rescaling the color scale by dragging/zooming
    customPlot->axisRect()->setupFullAxesBox(true);
    customPlot->xAxis->setLabel("time ch1");
    customPlot->yAxis->setLabel("time ch2");
    QCPColorScale *colorScale = new QCPColorScale(customPlot);
    customPlot->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
    colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
    colorMap = new QCPColorMap(customPlot->xAxis, customPlot->yAxis);
    colorMap->setColorScale(colorScale); // associate the color map with the color scale
    colorMap->setGradient(colorMapGradient);
    colorMap->setInterpolate(false);
    // make sure the axis rect and color scale synchronize their bottom and top margins (so they line up):
    QCPMarginGroup *marginGroup = new QCPMarginGroup(customPlot);
    customPlot->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

    customPlot = ui->plot2;
    customPlot->addGraph();
    customPlot->graph(0)->setPen(QColor(0, 0, 0, 255));
    customPlot->graph(0)->setLineStyle(QCPGraph::lsStepCenter);
    customPlot->xAxis->setLabel("bin");
    customPlot->yAxis->setLabel("counts");
    customPlot->yAxis->setLabelFont(font);
    customPlot->xAxis->setLabelFont(font);

    customPlot = ui->plot3;
    customPlot->addGraph();
    customPlot->graph(0)->setPen(QColor(0, 0, 0, 255));
    customPlot->xAxis->setLabel("bin");
    customPlot->yAxis->setLabel("clock offset / ns");
    customPlot->yAxis->setLabelFont(font);

    customPlot->xAxis->setLabelFont(font);

    noHitInChannelError[0] = 0;
    noHitInChannelError[1] = 0;

    interventclockHist.init(0,1,1000);
    last_intereventclock.fill(-1);

    for(size_t i = 0; i < maxNumOfTDC; i++)
    {
        channelEntries[i].fill(0);
        event_list.at(i).reserve(100);
    }

    ui->sB_TDC->setMaximum(maxNumOfTDC-1);

    dataBufferSize = 2000;
    dataBuffer = new uint32_t[dataBufferSize];

    for(size_t i = 0; i < unpacker.size(); ++i)
    {
        unpacker[i] = new ElbVfb6Unpacker;
    }

    hist.init(0.0, 0.1, 100);
    hist2d.init(0.0, 0.0, 0.1, 100, 100);

    generator.seed(static_cast<unsigned long>(clock()));

    analysisTimer= new QTimer(this);
    connect( analysisTimer, SIGNAL( timeout() ), this, SLOT(  analyseIncommingData()  ) );
    analysisTimer->start(50); // ms
}

MainWindow::~MainWindow()
{
    delete[] dataBuffer;
    for (size_t i = 0; i < unpacker.size(); ++i)
    {
        delete unpacker[i];
    }

    delete analysisTimer;
    delete ui;
}

void MainWindow::on_pB_loadJTDCFile_clicked()
{
    // Kalibrierungstabellen neu initialisieren
    /*    for(size_t t = 0; t < maxNumOfTDC; t++)
    {
        unpacker.at(t)->reset();
        event_list.at(t).clear();
    }*/

    if(mbsclient.isConnected())
        mbsclient.disconnect();
    /*
    std::string path = QFileDialog::getOpenFileName(this, tr("Open File"), "../", tr("MBS Files (*.lmd)")).toStdString();

    if(path != "")
        mbsclient.connect(path, MBSClient::connectionOption::file);
*/
    //  mbsclient.connect("R4-2", MBSClient::connectionOption::stream);

    drawPlots();
}



void MainWindow::analyseIncommingData()
{  
    std::vector<MbsClient::MbsEvent> mbs_events_to_analyse;
    mbsclient.getEventData(mbs_events_to_analyse,10000);


    if(mbs_events_to_analyse.size() != 0)
        std::cout << "MBS client  : received "<< mbs_events_to_analyse.size() << " readout events." << std::endl;
    else
        return;

    analysisTimer->stop();

    uint32_t windowsSize = static_cast<uint32_t>(ui->sB_windowSize->value());

    ElbVfb6Unpacker::VFB6_EVENT_LIST eventlist;
    eventlist.reserve(32);


    for(size_t ev = 0; ev < mbs_events_to_analyse.size(); ev++)
    {
        const std::vector<uint32_t> &data = mbs_events_to_analyse.at(ev).data;

        if(data.size() == 0)
            continue;

        size_t i_start = 0, i_end = 0;
        uint32_t module_id = 0, module_nr = 0;
        for(size_t i = 0; i < data.size();++i)
        {
            const uint32_t dword = data.at(i);


            if(dword==ELB_VFB6TDC_ID)
            {
                module_id = dword;
                module_nr = data.at(++i);
                i_start = i+1;
            }

            if(dword == END_ID || dword == END_ID_OLD)
            {
                i_end = i;

                // analyse data
                switch(module_id)
                {
                case ELB_VFB6TDC_ID:
                {
                    if(module_nr>=unpacker.size())
                    {
                        std::cout<< "error: module_nr>=VFB6_TDC_unpacker.size()." << std::endl;
                        return;
                    }

                    unpacker.at(module_nr)->analyseFileFormat(data, i_start, i_end, eventlist, windowsSize, true);

                    for(size_t n = 0; n < eventlist.size() /*&& eventlist.at(n).eventNr > -1*/; n++)
                    {
                        ElbVfb6Unpacker::Vfb6Event &tdcEvent = eventlist.at(n);

                        //std::cout << module_nr << "\t"<< tdcEvent.triggerReguest << "\t" << (int)tdcEvent.triggerDuringBusy << std::endl;


                        for(uint32_t h = 0; h < tdcEvent.nHits; h++)
                        {
                            uint32_t ch = unpacker.at(module_nr)->getChannel(tdcEvent.hits[h]);
                            channelEntries.at(module_nr).at(ch)++;
                        }

                        event_list.at(module_nr).push_back(std::shared_ptr <ElbVfb6Unpacker::Vfb6Event>(new ElbVfb6Unpacker::Vfb6Event(tdcEvent)));
                    }

                    eventlist.clear();
                }
                    break;

                default:
                    break;
                }

                module_id = 0;  // important to exclude other module data
            }
        }
    }

    analysisTimer->start();
    drawPlots();
}

void MainWindow::on_pB_refresh_clicked()
{
    const size_t firstCh = static_cast<size_t>(ui->sB_firstChannel->value());
    const size_t secondCh = static_cast<size_t>(ui->sB_secondChannel->value());
    const size_t firstTDC = static_cast<size_t>(ui->sB_firstTDC->value());
    const size_t secondTDC = static_cast<size_t>(ui->sB_secondTDC->value());
    const double binWidth = static_cast<double>(ui->sB_binWidth->value());

//    std::cout << "tdc X: " << firstTDC <<"\t tdc Y: " << secondTDC << std::endl;

    hist2d = nucmath::Hist2(0.0, 0.0, binWidth/1000.0, binWidth/1000.0, 100, 100);
    hist.clear();
    hist.setBinWidth(binWidth/1000.0);
    interventclockHist.clear();

    last_intereventclock.fill(-1);



    ElbVfb6Unpacker::TimeInterpolation statisticalInterpolation = ui->cB_statInterpolation->isChecked()
                                        ? ElbVfb6Unpacker::TimeInterpolation::Random
                                        : ElbVfb6Unpacker::TimeInterpolation::Linear;

    const auto &calib_table1 = unpacker.at(firstTDC)->getCalibrationTable().at(firstCh);
    const auto &calib_table2 = unpacker.at(secondTDC)->getCalibrationTable().at(secondCh);

    double t1min=10e9,t1max=-10e9,t2min=10e9,t2max = -10e9;

    for(size_t k = 0; k < event_list.at(firstTDC).size() && k < event_list.at(secondTDC).size(); k++)
    {
        size_t k2 = k;

        const ElbVfb6Unpacker::Vfb6Event& ev1 = *event_list.at(firstTDC).at(k).get();
        const ElbVfb6Unpacker::Vfb6Event& ev2 = *event_list.at(secondTDC).at(k2).get();

        double t1 = -10000.0, t2 = -10000.0;

        for(size_t i = 0; i < ev1.nHits; i++)
        {
            uint32_t ch = unpacker.at(firstTDC)->getChannel(ev1.hits[i]);
            if(ch == firstCh)
            {
                t1 = unpacker.at(firstTDC)->getTime(ev1.hits[i], calib_table1, statisticalInterpolation);
                break;
            }
        }
        if(t1 < -10)
        {
            noHitInChannelError[0]++;
            continue;
        }

        for(size_t i = 0; i < ev2.nHits; i++)
        {
            uint32_t ch = unpacker.at(secondTDC)->getChannel(ev2.hits[i]);
            if(ch == secondCh)
            {
                t2 = unpacker.at(secondTDC)->getTime(ev2.hits[i], calib_table2, statisticalInterpolation);
                break;
            }
        }
        if(t2 < -10)
        {
            noHitInChannelError[1]++;
            continue;
        }

        t1max = std::max(t1max,t1);
        t1min = std::min(t1min,t1);
        t2max = std::max(t2max,t2);
        t2min = std::min(t2min,t2);

        double time_in_ns = 0.0;
        if(ui->pB_plus->isChecked())
            time_in_ns = (t2+t1)*1.0;
        else
            time_in_ns = t1-t2;

        hist.add(time_in_ns);

        if(-2e3 < t1 && t1 < 2e3
                && -2e3 < t2 && t2 < 2e3)
        {
            hist2d.add(t1, t2, 1.0, true);
        }
        else
            std::cout << "t1=" << t1 << "\tt2=" << t2 << std::endl;

    }


    // finde ersten nicht leeren Eintrag im Histogram. Benutze es als Startpunkt fürs Plotten.
    size_t start_plot_range = 0, end_plot_range = 0;
    hist.getThresholdBorders(1, start_plot_range, end_plot_range);
    QCustomPlotZoom *customPlot = ui->plot1;
    int graphNr = 0;
    QVector<double> x, y;
    for (size_t i=start_plot_range; i< hist.nBins(); ++i)
    {
        const auto& [vx,vy] = hist.data(i);
        x.push_back(vx);
        y.push_back(vy);
    }
    customPlot->graph(graphNr)->setData(x, y);
    customPlot->rescaleAxes();
    customPlot->replot();



    if(t1max > 1e4)
        t1max = 1e4;
    if(t2max > 1e4)
        t2max = 1e4;

    if(t1min < -1e4)
        t1min = -1e4;
    if(t2min < -1e4)
        t2min = -1e4;

    // set up the QCPColorMap:
    customPlot = ui->plot_2d;

    int nx = static_cast<int>(std::min(1.0+(t1max-t1min)/(binWidth/1000), 2000.0));
    int ny = static_cast<int>(std::min(1.0+(t2max-t2min)/(binWidth/1000), 2000.0));
    colorMap->data()->setSize(ny, nx); // we want the color map to have nx * ny data points
    colorMap->data()->setRange(QCPRange(t2min, t2max), QCPRange(t1min, t1max)); // and span the coordinate range -4..4 in both key (x) and value (y) dimensions


    {
        nucmath::Hist2& hist = hist2d;
        //const auto&[rx1, rx2, ry1, ry2] = hist.getRange();
        //colorMap->data()->setRange(QCPRange(ry1, ry2), QCPRange(rx1, rx2));

        if(hist.isChanged(false))
        {
            colorMap->data()->fill(0.0);
            for(size_t j = 0; j < hist.nBins(); ++j)
            {
                const auto&[x, y, z] = hist.data(j);
                if(z>0)
                    colorMap->data()->setData(y, x, z);
            }


            colorMap->rescaleDataRange(true);
            customPlot->rescaleAxes();
            customPlot->replot();
        }
    }

    ui->sB_tdcChannel->setValue(static_cast<int>(firstCh));
    drawPlots();
}

void MainWindow::on_pB_timeSpacing_clicked()
{
    const size_t firstTDC = static_cast<size_t>(ui->sB_firstTDC->value());

    interventclockHist.clear();
    double last_timestamp = -1;

    for(size_t k = 0; k < event_list.at(firstTDC).size(); k++)
    {
        const ElbVfb6Unpacker::Vfb6Event& ev1 = *event_list.at(firstTDC).at(k).get();

        double timestamp = ev1.timestamp1mu;
        if(k > 0)
        {
            double dt = timestamp - last_timestamp; 

            if(dt <= 0)
            {
                dt = (timestamp + 8192.0 + 1) - last_timestamp;
            }
            else if(dt > 15000)
            {
             /*   std::cout << "dt = " << (double)ev1.timestamp1mu - last_timestamp
                          << "  " << ev1.timestamp1mu
                          << "  " << last_timestamp
                          << std::endl;*/
            }

            interventclockHist.add(dt);
        }

        last_timestamp = ev1.timestamp1mu;
    }


    size_t start_plot_range = 0, end_plot_range = 0;
    interventclockHist.getThresholdBorders(1, start_plot_range, end_plot_range);
    QCustomPlotZoom *customPlot = ui->plot_timeSpacing;
    int graphNr = 0;
    QVector<double> x, y;
    for (size_t i=start_plot_range; i< interventclockHist.nBins(); ++i)
    {
        const auto& [vx,vy] = interventclockHist.data(i);
        x.push_back(vx);
        y.push_back(vy);
    }
    customPlot->graph(graphNr)->setData(x, y);
    customPlot->rescaleAxes();
    customPlot->replot();

}

void MainWindow::on_pB_fit_clicked()
{
    // FIND FIT
    if(hist.nBins() > 7)
    {
        std::cout << std::endl << "find fit..." << std::endl;

        // Den Fit-Bereich nur auf den aktuellen im Fenster sichtbaren Datenbereich anpassen
        const QCPAxis* axis  = ui->plot1->graph(0)->valueAxis();
        auto ab = axis->axisRect()->axis(QCPAxis::AxisType::atBottom)->tickVector();
        auto al = axis->axisRect()->axis(QCPAxis::AxisType::atLeft)->tickVector();

        std::vector<double> xv;
        std::vector<double> yv;
        for(size_t i=0; i< hist.nBins(); ++i)
        {
            const auto& [x,y] = hist.data(i);
            xv.push_back(x);
            yv.push_back(y);
        }

        std::vector<std::array<double, 3> > initial_p = { { { al.at(al.size()-1)/2, 5, al.at(al.size()-1)*(ab.at(ab.size()-1)-ab.at(0)) },
                                                            { (ab.at(0)+ab.at(ab.size()-1))/2.0, ab.at(0), ab.at(ab.size()-1) },
                                                            { 0.05, 0.02, std::min(30.0, ab.at(ab.size()-1)-ab.at(0)) } } };
        std::vector<double> result;
        std::vector<double> result_sigma;

        MODELFUNC fitModel = [&](const std::vector<double> &p, const nucmath::Vector<double> &x)
        {
            return nucmath::normal_dist_pdf({p[0],p[1],p[2]}, x(0));
        };

        nucmath::Minimizer minimizer;
        minimizer.setData(xv,yv);
        minimizer.setNumberOfSeedPoints(1000);
        minimizer.setInitialPointsAndConstrains(initial_p);
        minimizer.setParameterNames({"A","mu","s"});
        std::cout << minimizer.getFormatedInitialPoints();
        std::cout << minimizer.getFormatedConstrains();

        minimizer.setModelFunction(fitModel);
        minimizer.findFit(1000, 0.00001);
        result = minimizer.getResult();

        std::cout << minimizer.getFormatedFitResults() << std::endl;

        ui->labelFitOutput->setText(QString(" A=")+QString::number(result[0])
                +"  µ="+QString::number(result[1])
                +" ns   σ="+QString::number(result[2])
                +" ns   fwhm="+QString::number(result[2]*nucmath::SIGMA2FWHM) + " ns");


        QVector<double> x, y;
        x.clear();
        y.clear();
        const double plot_step = (initial_p[1][2]-initial_p[1][1])/100;
        for(double xi=initial_p[1][1]; xi<initial_p[1][2]; xi+=plot_step)
        {
            x.push_back(xi);
            double yi = fitModel(result, std::vector<double>({xi}));
            y.push_back(yi);
        }


        QCustomPlotZoom *customPlot = ui->plot1;
        int graphNr = 1;
        customPlot->graph(graphNr)->setData(x, y);
        customPlot->replot();
    }
    else
    {
        ui->labelFitOutput->setText("zu wenige Daten für eine Funktionsanpassung");
    }
}


void MainWindow::on_pB_saveSpectrum_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Save file"), "", "(*.dat *.txt)");

    std::ofstream fs(filename.toStdString().c_str());

    QCustomPlot *customPlot = ui->plot1;


    // ersten nicht leeren Graph auswählen
    int graphNr = 0;
    for(int i = 0; i < customPlot->graphCount(); i++)
    {
        if(customPlot->graph(i)->data()->size() > 0)
        {
            graphNr = i;
            break;
        }
    }

    auto keys = customPlot->graph(graphNr)->data().data();
    for (auto it = keys->begin(); it != keys->end(); it++)
    {
        double xi = (*it).key;
        double yi = (*it).value;

        fs << xi << " " << yi << std::endl;
    }

    fs.close();
}

void MainWindow::on_pB_loadJTDCFileAsLMD_clicked()
{
    // Kalibrierungstabellen neu initialisieren
    for(size_t t = 0; t < maxNumOfTDC; t++)
    {
        unpacker.at(t)->reset();
        event_list.at(t).clear();
    }

    if(mbsclient.isConnected())
        mbsclient.disconnect();

    std::string path = QFileDialog::getOpenFileName(this, tr("Open File"), "../", tr("MBS Files (*.lmd)")).toStdString();

    if(path != "")
        mbsclient.connect(path, MbsClient::ConnectionOption::automatic, false);

    drawPlots();
}

void MainWindow::drawPlots()
{
    size_t tdcNr = static_cast<size_t>(ui->sB_TDC->value());
    size_t channelNr = static_cast<size_t>(ui->sB_tdcChannel->value());

    QCustomPlotZoom *customPlot = ui->plot_entries;
    int graphNr = 0;
    QVector<double> x, y;
    for (size_t i=0; i < channelEntries.at(tdcNr).size(); ++i)
    {
        double yi = channelEntries.at(tdcNr).at(i);
        x.push_back(i);
        y.push_back(yi);
    }
    customPlot->graph(graphNr)->setData(x, y);
    customPlot->rescaleAxes();
    customPlot->replot();

    auto DNL_table = unpacker.at(tdcNr)->getDNLTable();
    auto calib_table = unpacker.at(tdcNr)->getCalibrationTable();

    if(DNL_table.size()!=0 && calib_table.size()!=0)
    {
        customPlot = ui->plot2;
        graphNr = 0;
        x.clear();y.clear();
        for (size_t i=0; i<255; ++i)
        {
            double yi = DNL_table.at(channelNr).at(i);
            x.push_back(i);
            y.push_back(yi);
        }
        customPlot->graph(graphNr)->setData(x, y);
        customPlot->rescaleAxes();
        customPlot->replot();

        customPlot = ui->plot3;
        x.clear();y.clear();
        for (size_t i=0; i<ElbVfb6Unpacker::nTimeBins; ++i)
        {
            double xi = static_cast<double>(i);
            double yi = calib_table.at(channelNr).at(i)*2.5;
            x.push_back(xi);
            y.push_back(yi);
        }
        customPlot->graph(graphNr)->setData(x, y);
        customPlot->rescaleAxes();
        customPlot->replot();
    }

    ui->label_events->setText(std::string("Events: "+std::to_string(unpacker.at(tdcNr)->getEventCounter())).c_str());
    ui->label_trigger_during_busy->setText(std::string("Trigger during busy: "+std::to_string(unpacker.at(tdcNr)->getTriggerDuringBusyCount())).c_str());
    ui->label_fifo_overflow->setText(std::string("FIFO overflow: "+std::to_string(unpacker.at(tdcNr)->getDataFifoOverflowCount())).c_str());
    ui->label_old_data_ignored->setText(std::string("Old data ignored: "+std::to_string(unpacker.at(tdcNr)->getOldTDCDataIgnoreCount())).c_str());
    ui->label_time_over_the_limit->setText(std::string("Time over the limit: "+std::to_string(unpacker.at(tdcNr)->getTimeOverTheLimitCount())).c_str());
    ui->label_hits_per_events_overflow->setText(std::string("Hits per event overflow: "+std::to_string(unpacker.at(tdcNr)->getHitsPerEventOverflow())).c_str());
}

void MainWindow::on_sB_TDC_valueChanged(int )
{
    drawPlots();
}

void MainWindow::on_sB_tdcChannel_valueChanged(int )
{
    drawPlots();
}

void MainWindow::on_pB_minus_clicked()
{
    ui->pB_plus->setChecked(false);
}

void MainWindow::on_pB_plus_clicked()
{
    ui->pB_minus->setChecked(false);
}
