#include "wimagen.h"
#include "wselector.h"
#include <QDebug>
#include <QPainter>
#include <QWheelEvent>
#include "../overlay1.h"


WImagen::WImagen(int w, int h, Overlay1* ol, QWidget *parent) :
    QWidget(parent)
{
    m_overlay1 = ol;
    m_font1 = parent->font();
    m_font1.setPixelSize(20);
    m_recCurrent = 10000;
    m_recTotal = 10000;

    // Inicia zooms a x1
    m_xZoom << 0.25 << 0.5 << 0.75 << 1 << 1.5 << 2 << 4;
    m_selZoom = 3;
    m_rctDraw = QRect(0,0,w,h);

    m_d_img = (ushort*)malloc(sizeof(ushort)*w*h);
    memset(m_d_img,0,sizeof(sizeof(ushort)* w*h));
    m_img = new QImage((uchar*)m_d_img, w, h, QImage::Format_Grayscale16);

    m_selector = new WSelector(this);
}


void WImagen::setZoom(T_Zoom zoom)
{
    if(zoom==ZOOM_IN && m_selZoom<(m_xZoom.count()-1))
        m_selZoom++;
    else if(zoom==ZOOM_OUT && m_selZoom>0)
        m_selZoom--;

    double factor = m_xZoom.at(m_selZoom);
    double w = m_img->width()*factor;
    double h = m_img->height()*factor;
    parentWidget()->resize(w,h);
    resize(w,h);
    m_selector->setZoomFactor(factor);
    m_overlay1->set_size(m_img->width(), m_img->height());
}


void WImagen::setROI(const QRect &rct)
{
    m_roi = rct;
    memset(m_d_img,0,sizeof(ushort)* m_img->width()*m_img->height());
}


void WImagen::setCut(const QPoint &p0, const QPoint &p1)
{
    m_pt1 = p0;
    m_pt2 = p1;
}

void WImagen::setRecordState(int current, int total)
{
    m_recCurrent=current;
    m_recTotal=total;
}


void WImagen::get_cursor(int *x, int *y)
{
    QPoint pt = mapFromGlobal(QCursor::pos());
    if(pt.x()<0 || pt.y()<0 ||
            pt.x()>width() || pt.y()>height())
    {
        *x=*y=-1;
        return;
    }

    double factor = m_xZoom.at(m_selZoom);
    *x = pt.x()/factor;
    *y = pt.y()/factor;
}



void WImagen::paintEvent(QPaintEvent *)
{
    QPainter p(this);    
    QRect rct(0,0,parentWidget()->width(),parentWidget()->height());
    p.drawImage(rct, *m_img);
    m_selector->redraw(&p);

    double factor = m_xZoom.at(m_selZoom);
    QPen pen(Qt::red);
    pen.setWidth(2);
    if(m_roi.width()!=m_img->width() && m_roi.height()!=m_img->height())
    {
        p.setPen(pen);
        p.setBrush(Qt::transparent);
        p.drawRect(m_roi.x()*factor,
                   m_roi.y()*factor,
                   m_roi.width() *factor,
                   m_roi.height()*factor);
    }

    if(!m_pt1.isNull() || !m_pt2.isNull())
    {
        pen.setColor(Qt::magenta);
        p.setPen(pen);
        p.drawLine(m_pt1.x()*factor,
                   m_pt1.y()*factor,
                   m_pt2.x()*factor,
                   m_pt2.y()*factor);
    }
    m_overlay1->render(&p, factor);

    if(m_recTotal>=0)
    {
        p.setFont(m_font1);
        p.setPen(Qt::white);
        p.setBrush(Qt::red);
        p.drawRect(2,2,230,24);
        p.drawText(5,2,230,24,Qt::AlignLeft,QString().sprintf("REC: %d de %d", m_recCurrent, m_recTotal));
    }
}
