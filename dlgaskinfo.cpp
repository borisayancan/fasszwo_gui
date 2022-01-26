#include "dlgaskinfo.h"
#include <QDateTime>
#include <QTimeZone>


DlgAskInfo::DlgAskInfo(float texp, int gain, const char* camara, QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
    QString init;
    init += "Camara  : " + QString(camara) + "\n";
    init += "Fecha   : " + QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss ") +
            QString().sprintf("GMT %+d", QDateTime::currentDateTime().offsetFromUtc()/3600) + "\n";
    init += "Lugar   : \n";
    init += "Exp     : " + QString().sprintf("%.1f ms", texp) + "\n";
    init += "Gain    : " + QString().sprintf("%d ms", gain) + "\n";
    init += "Filtro  : LOW:camara HIGH:camara\n";
    init += "Elevac  : ?? \n";

    edtText->setPlainText(init);
    edtEstrella->setFocus();
}


QString DlgAskInfo::getInfo(int max_len, QString* estrella)
{
    QString ret = "Estrella: " + edtEstrella->text() + "\n";
    ret += edtText->toPlainText();
    ret.truncate(max_len-1);
    if(edtEstrella->text().length()<1)
        edtEstrella->setText("no_estrella");
    *estrella = edtEstrella->text();
    return ret;
}


void DlgAskInfo::on_btnAceptar_clicked()
{
    accept();
}


void DlgAskInfo::on_btnCancerlar_clicked()
{
    reject();
}
