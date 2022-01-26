#ifndef DLGIMGPROCESADA_H
#define DLGIMGPROCESADA_H
#include "ui_dlgimgprocesada.h"

class DlgImgProcesada : public QDialog, private Ui::DlgImgProcesada
{
    Q_OBJECT

public:
    explicit DlgImgProcesada(QWidget *parent = nullptr);
    QImage* m_image;

private:
    void paintEvent(QPaintEvent*);
};

#endif // DLGIMGPROCESADA_H
