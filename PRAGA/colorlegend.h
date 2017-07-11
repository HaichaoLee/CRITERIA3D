#ifndef COLORLEGEND_H
#define COLORLEGEND_H

    #include <QWidget>
    #include "gis.h"

    class ColorLegend : public QWidget
    {
        Q_OBJECT

    public:
        explicit ColorLegend(QWidget *parent = 0);
        ~ColorLegend();

        gis::Crit3DColorScale *colorScale;

    private:
        void paintEvent(QPaintEvent *);
    };

    bool drawColorLegend(gis::Crit3DColorScale* colorScale, QPainter* myPainter);

#endif // COLORLEGEND_H
