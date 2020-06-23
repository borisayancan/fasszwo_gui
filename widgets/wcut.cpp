#include "wcut.h"
#include <QPainter>
#include <QDebug>

WCut::WCut(QWidget *parent) :
    QWidget(parent)
{
    m_font = parent->font();
    m_font.setPointSize(10);
    QFontMetrics fm(m_font);
    m_szText = fm.boundingRect("000000");
    m_szText.moveTo(0,0);
}


void WCut::setData(int *d, int sz, int ymin, int ymax)
{
    m_poly.clear();
    float h = height();
    float sx = float(width())/ sz;
    float sy = float(height())/65536.;

    for(int i=0; i<sz; i++)
        m_poly << QPoint( i*sx, h-d[i]*sy);

    m_min=ymin;
    m_max=ymax;
    update();
}


void WCut::paintEvent(QPaintEvent *)
{
    int htxt = m_szText.height();
    int wtxt = m_szText.width();
    float w = width();
    float h = height();
    QPainter p(this);
    p.setFont(m_font);
    p.setPen(Qt::black);
    p.setBrush(Qt::black);
    p.drawRect(0,0,w,h);

    p.setPen(QColor(96,96,96));
    for(int i=1; i<10; i++)
        p.drawLine(0,i*h*0.1, w,i*h*0.1);

    QPen pen(Qt::green);
    pen.setWidth(2);
    p.setPen(pen);
    p.drawPolyline(m_poly);

    p.setPen(Qt::yellow);
    p.setBrush(Qt::blue);
    QRect r1 = m_szText.translated(2,2);
    QRect r2 = m_szText.translated(2,h-m_szText.height()-2);
    p.drawRect(r1);
    p.drawRect(r2);    

    float step = (m_max-m_min)*0.1;
    for(int i=1; i<10; i++)
    {
        p.drawRect(2,i*h*0.1-htxt/2, wtxt, htxt);
        p.drawText(2,i*h*0.1-htxt/2, wtxt, htxt,Qt::AlignCenter,
                   QString::number(int(m_max-step*i)));
    }

    p.drawText(r1,Qt::AlignCenter,
               QString::number(m_max));
    p.drawText(r2,Qt::AlignCenter,
               QString::number(m_min));
}
