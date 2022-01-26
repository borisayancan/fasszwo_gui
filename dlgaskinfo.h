#ifndef DLGASKINFO_H
#define DLGASKINFO_H

#include "ui_dlgaskinfo.h"

class DlgAskInfo : public QDialog, private Ui::DlgAskInfo
{
    Q_OBJECT

public:
    explicit DlgAskInfo(float texp, int gain, const char *camara, QWidget *parent = nullptr);
    QString getInfo(int max_len, QString *estrella);

private slots:
    void on_btnAceptar_clicked();
    void on_btnCancerlar_clicked();
};

#endif // DLGASKINFO_H
