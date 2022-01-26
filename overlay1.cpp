#include "overlay1.h"
#include <QFileDialog>
#include <QPainter>
#include <QFile>
#include <QMessageBox>
#include <math.h>


Overlay1::Overlay1(QWidget* p) :
    QDialog(p)
{
    setupUi(this);
    setWindowFlag(Qt::WindowMaximizeButtonHint, true);
    m_colores << Qt::yellow << Qt::yellow << Qt::green << Qt::green;
}


void Overlay1::render(QPainter *p, float factor)
{
    if(m_lines.count()==0) return;

    p->save();
    p->translate(edtX->value()*factor, edtY->value()*factor);
    p->scale(factor,factor);
    for(int i=0; i<m_lines.count(); i++)
    {
        QPen pen(Qt::white);
        pen.setCosmetic(true);
        if(m_colores.count()>i)
            pen.setColor(m_colores.at(i));
        p->setPen(pen);
        p->drawPolyline(m_lines.at(i));
    }
    p->restore();
}


void Overlay1::set_size(int w, int h)
{
    int sz = w>h? h:w;
    edtX->setValue(sz/2);
    edtY->setValue(sz/2);
}


void Overlay1::on_btnAbrir_clicked()
{
    QString fname = QFileDialog::getOpenFileName(this,"Puntos XY",QString(), "Puntos (*.pts)" );
    if(fname.isEmpty()) return;

    m_lines.clear();
    QFile f(fname);

    int sz = f.size();
    if(sz<4) return;
    if(!f.open(QIODevice::ReadOnly)) return;


    int npts;
    f.read((char*)&npts, 4);
    int nvect = (sz-4)/4/npts;
    bool valido = npts>0 && npts<32768 &&
                  (4+nvect*npts*4)==sz &&
                  (nvect%2)==0;

    qDebug("valido:%d pts:%d lines:%d", valido, npts, nvect/2);
    if(!valido)
    {
        QMessageBox::critical(this,"Error","Archivo no valido  ");
    }
    else
    {
        int* p = (int*)malloc(4*npts*nvect);
        memset(p,0,4*npts*nvect);
        f.read((char*)p, 4*npts*nvect);

        for(int i=0; i<nvect; i+=2)
        {
            QPolygon poly;
            for(int k=0; k<npts; k++)
            {
                int x = p[i*npts+k];
                int y = p[(i+1)*npts+k];
                poly << QPoint(x,y);
            }
            m_lines << poly;
        }

        free(p);
    }

    f.close();
}


void Overlay1::on_btnBorrar_clicked()
{
    m_lines.clear();
}


void Overlay1::on_btnSetRadio_clicked()
{

    const double pi = 3.14159;
    m_lines.clear();
    QPolygon poly;
    for(int i=0; i<=360; i+=5)
    {
        int x = edtRadio->value()*cos(pi*i/180.);
        int y = edtRadio->value()*sin(pi*i/180.);
        poly << QPoint(x,y);
    }

    m_lines << poly;
}
