#include <QFileDialog>
#include <QtDebug>

#include "tileSources/OSMTileSource.h"
#include "tileSources/CompositeTileSource.h"
#include "CircleObject.h"
#include "Position.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "raster.h"
#include "RasterObject.h"


extern gis::Crit3DGeoMap *geoMap;
extern gis::Crit3DRasterGrid *DTM;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /*!
     * Setup the MapGraphics scene and view
     */
    this->scene = new MapGraphicsScene(this);
    this->view = new MapGraphicsView(scene,this);
    Position* startCenter = new Position (11.35, 44.5, 0.0);


    this->setCentralWidget(this->view); /**< The view will be our central widget */

    /*!
     * Setup some tile sources
     */
    QSharedPointer<OSMTileSource> osmTiles(new OSMTileSource(OSMTileSource::OSMTiles), &QObject::deleteLater);
    QSharedPointer<CompositeTileSource> composite(new CompositeTileSource(), &QObject::deleteLater);
    composite->addSourceBottom(osmTiles);
    this->view->setTileSource(composite);

    /*!
     * marker example
     * CircleObject* marker1 = new CircleObject(5.0, true, QColor(255,0,0,255), 0);
     * marker1->setFlag(MapGraphicsObject::ObjectIsMovable, false);
     * marker1->setFlag(MapGraphicsObject::ObjectIsSelectable, false);
     * marker1->setLatitude(startCenter->latitude());
     * marker1->setLongitude(startCenter->longitude());
     * this->view->scene()->addObject(marker1);
    */

    geoMap->referencePoint.latitude = startCenter->latitude();
    geoMap->referencePoint.longitude = startCenter->longitude();

    this->rasterMap = NULL;
    this->view->setZoomLevel(10);
    this->view->centerOn(startCenter->lonLat());
}


MainWindow::~MainWindow()
{
    delete view;
    delete scene;
    delete ui;
}

void MainWindow::on_actionLoad_Raster_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Raster"), "",
                                           tr("ESRI grid files (*.flt)"));
    if (fileName == "") return;

    qDebug() << "loading raster";
    this->rasterMap->deleteLater();

    loadRaster(fileName, DTM);

    this->rasterMap = new RasterObject(this->view);
    this->rasterMap->setOpacity(0.5);
    this->rasterMap->moveCenter();
    this->view->scene()->addObject(this->rasterMap);
}


void MainWindow::mouseReleaseEvent(QMouseEvent *event){
    if (this->rasterMap != NULL)
        this->rasterMap->moveCenter();
}


void MainWindow::mouseDoubleClickEvent(QMouseEvent * event)
{
    Position newCenter = view->mapToScene(QPoint(event->pos().x(), event->pos().y()));
    this->view->centerOn(newCenter.lonLat());

    if (event->button() == Qt::LeftButton)
        this->view->zoomIn();
    else
        this->view->zoomOut();

    if (this->rasterMap != NULL)
        this->rasterMap->moveCenter();
}

