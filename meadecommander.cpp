#include "meadecommander.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <Windows.h>
#include <QDebug>


MeadeCommander::MeadeCommander(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);
    setWindowFlag(Qt::WindowMaximizeButtonHint, true);
    m_busy=false;

    m_port = new QSerialPort(this);
    connect(m_port,SIGNAL(readyRead()), this, SLOT(slot_readyRead()));

    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    for(int i=0; i<ports.count(); i++) cmbPort->addItem(ports.at(i).portName());
    update_ui();

    QTimer* t = new QTimer();
    connect(t,&QTimer::timeout, this, &MeadeCommander::slot_timer);
    t->start(1000);
}


void MeadeCommander::on_btnUp_clicked()
{
    send_mov(":Mn#");
}


void MeadeCommander::on_btnRight_clicked()
{
    send_mov(":Me#");
}


void MeadeCommander::on_btnDown_clicked()
{
    send_mov(":Ms#");
}


void MeadeCommander::on_btnLeft_clicked()
{
    send_mov(":Mw#");
}


void MeadeCommander::slot_readyRead()
{
    bool nuevo = !m_tpoRx.isValid() || m_tpoRx.elapsed()>500;
    QString rx = m_port->readAll();
    qDebug() << rx;

    if(nuevo) edtLog->appendPlainText("-> "+rx);
    else      edtLog->appendPlainText(rx);
}


void MeadeCommander::on_btnAbrir_clicked()
{
    m_port->setPortName(cmbPort->currentText());
    m_port->setBaudRate(9600);
    m_port->setDataBits(QSerialPort::Data8);
    m_port->setParity(QSerialPort::NoParity);
    m_port->setStopBits(QSerialPort::OneStop);
    m_port->setFlowControl(QSerialPort::NoFlowControl);
    m_port->open(QIODevice::ReadWrite);
    update_ui();
}

void MeadeCommander::slot_detener()
{
    m_port->write(":Q#");
    Sleep(50);
    m_busy = false;
    update_ui();
}


void MeadeCommander::update_ui()
{
    bool port = m_port->isOpen();
    btnAbrir     ->setEnabled(!port && cmbPort->count()>0);
    btnGet       ->setEnabled(port);
    edtTpo       ->setEnabled(port);
    btnRateMLento->setEnabled(port && !m_busy);
    btnRateLento ->setEnabled(port && !m_busy);
    btnUp        ->setEnabled(port && !m_busy);
    btnRight     ->setEnabled(port && !m_busy);
    btnDown      ->setEnabled(port && !m_busy);
    btnLeft      ->setEnabled(port && !m_busy);
    btnDetener   ->setEnabled(port);
}

void MeadeCommander::send_mov(const char *cmd)
{
    m_port->write(cmd);
    m_busy = true;
    QTimer::singleShot(edtTpo->value(), this, &MeadeCommander::slot_detener);
    update_ui();
}


void MeadeCommander::on_btnGet_clicked()
{
    int wr = m_port->write(":GVP#", 5);
    m_port->flush();
}

void MeadeCommander::on_btnDetener_clicked()
{
    slot_detener();
}

void MeadeCommander::slot_timer()
{
}

void MeadeCommander::on_btnRateMLento_clicked()
{
    m_port->write(":RC#", 4);
}

void MeadeCommander::on_btnRateLento_clicked()
{
    m_port->write(":RM#", 4);

}
