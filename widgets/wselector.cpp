#include "wselector.h"
#include <QDebug>
#include <QEvent>
#include <QPainter>

WSelector::WSelector(QWidget *parent) :
    QWidget(parent)
{
    m_xZoom = 1;
    m_selection=SEL_CANCEL;
    parent->installEventFilter(this);
    m_ptStart = QPoint(0,0);
}


void WSelector::redraw(QPainter *p)
{
    if(m_selection==SEL_CANCEL)
        return;

    QPen pen(Qt::yellow);
    pen.setWidth(2);
    p->setPen(pen);
    QPoint current = mapFromGlobal(QCursor::pos());
    QRect rct = QRect(m_ptStart.x(), m_ptStart.y(),
                      current.x()-m_ptStart.x() , current.y()-m_ptStart.y());

    if(!m_ptStart.isNull() && m_selection==SEL_ROI)
        p->drawRect(rct.normalized());

    else if(!m_ptStart.isNull() && m_selection==SEL_CUT_X)
        p->drawLine(rct.x(), rct.y(), rct.x()+rct.width(), rct.y());

    else if(!m_ptStart.isNull() && m_selection==SEL_CUT_Y)
        p->drawLine(rct.x(), rct.y(), rct.x(), rct.y()+rct.height());

    p->setPen(Qt::yellow);
    p->setBrush(Qt::blue);
    p->drawRect(2,2,100,16);
    if(m_selection==SEL_ROI)        p->drawText(2,2,100,16,Qt::AlignLeft | Qt::AlignVCenter,
                                                //QString().sprintf("ROI: %dx%d", int(rct.width()/m_xZoom), int(rct.height()/m_xZoom))
                                                QString("ROI: %1x%2").arg(int(rct.width()/m_xZoom)).arg(int(rct.height()/m_xZoom))
                                                );
    else if(m_selection==SEL_CUT_X) p->drawText(2,2,100,16,Qt::AlignLeft | Qt::AlignVCenter, "CUT X");
    else if(m_selection==SEL_CUT_Y) p->drawText(2,2,100,16,Qt::AlignLeft | Qt::AlignVCenter, "CUT Y");
}

void WSelector::setZoomFactor(double x)
{
    m_xZoom= x;
}

void WSelector::setSelection(T_Selection s)
{
    m_ptStart = QPoint(0,0);
    m_selection = s;
}


bool WSelector::eventFilter(QObject *, QEvent *e)
{
    if(e->type()==QEvent::MouseButtonPress && m_selection!=SEL_CANCEL)
    {
        m_ptStart = mapFromGlobal(QCursor::pos());
    }

    else if(e->type()==QEvent::MouseButtonRelease && !m_ptStart.isNull())
    {
        // Area de seleccion ROI
        QPoint current = mapFromGlobal(QCursor::pos());
        QRect rct = QRect(m_ptStart.x()/m_xZoom,
                          m_ptStart.y()/m_xZoom,
                          (current.x()-m_ptStart.x()) / m_xZoom ,
                          (current.y()-m_ptStart.y()) / m_xZoom);
        m_ptStart = QPoint(0,0);
        emit selection(m_selection, rct);
        m_selection = SEL_CANCEL;
    }
    return false;
}

