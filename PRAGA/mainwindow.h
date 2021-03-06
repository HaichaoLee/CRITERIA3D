#ifndef MAINWINDOW_H
#define MAINWINDOW_H

    #include <QMainWindow>
    #include <QList>
    #include <QCheckBox>

    #include "rubberBand.h"
    #include "MapGraphicsView.h"
    #include "MapGraphicsScene.h"
    #include "tileSources/OSMTileSource.h"
    #include "rasterObject.h"
    #include "stationMarker.h"
    #include "colorlegend.h"
    #include "guiConfiguration.h"
    #include "dbArkimet.h"


    namespace Ui
    {
        class MainWindow;
    }

    /*!
     * \brief The MainWindow class
     */
    class MainWindow : public QMainWindow
    {
        Q_OBJECT

    public:

        explicit MainWindow(environment menu, QWidget *parent = 0);
        ~MainWindow();

    private slots:

        void on_actionLoadRaster_triggered();

        void on_actionNewMeteoPointsArkimet_triggered();

        void on_actionOpen_meteo_points_DB_triggered();

        void on_actionDownload_meteo_data_triggered();

        void on_rasterOpacitySlider_sliderMoved(int position);

        void on_actionMapToner_triggered();

        void on_actionMapOpenStreetMap_triggered();

        void on_actionMapESRISatellite_triggered();

        void on_actionMapTerrain_triggered();

        void on_actionSetUTMzone_triggered();

        void on_actionRectangle_Selection_triggered();

        void on_actionVariableChoose_triggered();

        void on_dateTimeEdit_dateTimeChanged(const QDateTime &dateTime);

        void on_actionVariableNone_triggered();

        void on_rasterScaleButton_clicked();

        void on_variableButton_clicked();

        void on_frequencyButton_clicked();

        void enableAllDataset(bool toggled);

        void on_actionVariableQualitySpatial_triggered();

        void on_actionInterpolation_triggered();

        void on_actionPointsVisible_triggered();

        void on_action_Open_NetCDF_data_triggered();

        void on_action_Extract_NetCDF_series_triggered();

    protected:
        /*!
         * \brief mouseReleaseEvent call moveCenter
         * \param event
         */
        void mouseReleaseEvent(QMouseEvent *event);

        /*!
         * \brief mouseDoubleClickEvent implements zoom In and zoom Out
         * \param event
         */
        void mouseDoubleClickEvent(QMouseEvent * event);

        void mouseMoveEvent(QMouseEvent * event);

        void mousePressEvent(QMouseEvent *event);

        void resizeEvent(QResizeEvent * event);


    private:
        Ui::MainWindow* ui;

        MapGraphicsScene* mapScene;
        MapGraphicsView* mapView;
        RasterObject* rasterObj;
        RasterObject* gridObj;
        ColorLegend *rasterLegend;
        ColorLegend *pointsLegend;
        QList<StationMarker*> pointList;
        RubberBand *myRubberBand;
        bool showPoints;

        environment menu;
        QList<QCheckBox*> datasetCheckbox;

        void setMapSource(OSMTileSource::OSMTileType mySource);
        QString selectArkimetDataset(QDialog* datasetDialog);

        QPoint getMapPoint(QPoint* point) const;

        void updateVariable();
        void updateDateTime();
        void resetMeteoPoints();
        void addMeteoPoints();
        void redrawMeteoPoints();

        bool loadMeteoPointsDB(QString dbName);

    };

    void exportNetCDFDataSeries(gis::Crit3DGeoPoint geoPoint);


#endif // MAINWINDOW_H
