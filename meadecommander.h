#ifndef MEADECOMMANDER_H
#define MEADECOMMANDER_H
#include "ui_meadecommander.h"
#include <QElapsedTimer>


class MeadeCommander : public QDialog, private Ui::MeadeCommander
{
    Q_OBJECT

public:
    explicit MeadeCommander(QWidget *parent = nullptr);

private slots:
    void on_btnUp_clicked();
    void on_btnRight_clicked();
    void on_btnDown_clicked();
    void on_btnLeft_clicked();
    void slot_readyRead();
    void on_btnAbrir_clicked();
    void slot_detener();
    void on_btnGet_clicked();
    void on_btnDetener_clicked();
    void slot_timer();

    void on_btnRateMLento_clicked();

    void on_btnRateLento_clicked();

private:
    class QSerialPort* m_port;
    QElapsedTimer m_tpoRx;
    bool m_busy;
    void update_ui();
    void send_mov(const char* cmd);
};

#endif // MEADECOMMANDER_H
