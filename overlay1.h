#ifndef OVERLAY1_H
#define OVERLAY1_H
#include <ui_overlay1.h>
#include <QPolygon>

class Overlay1 : public QDialog, Ui::Overlay1
{
    Q_OBJECT

public:
    Overlay1(QWidget *p);
    void render(QPainter* p, float factor);
    void set_size(int w, int h);

private slots:
    void on_btnAbrir_clicked();
    void on_btnBorrar_clicked();
    void on_btnSetRadio_clicked();

private:
    QList<QPolygon> m_lines;
    QList<QColor>   m_colores;
};

#endif // OVERLAY1_H
