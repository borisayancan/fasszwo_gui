#ifndef WCUT_H
#define WCUT_H

#include <QWidget>

class WCut : public QWidget
{
    Q_OBJECT
public:
    explicit WCut(QWidget *parent = nullptr);
    void setData(int* d, int sz, int ymin, int ymax);

private:
    QPolygon m_poly;
    int m_min, m_max;
    QRect m_szText;
    QFont m_font;
    void paintEvent(QPaintEvent*);
};

#endif // WCUT_H
