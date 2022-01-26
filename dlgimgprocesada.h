#ifndef DLGIMGPROCESADA_H
#define DLGIMGPROCESADA_H
#include "ui_dlgimgprocesada.h"

class DlgImgProcesada : public QDialog, public Ui::DlgImgProcesada
{
    Q_OBJECT

public:
    explicit DlgImgProcesada(QWidget *parent = nullptr);
    QImage* m_image;

private:
    bool eventFilter(QObject *, QEvent *);
};

#endif // DLGIMGPROCESADA_H
