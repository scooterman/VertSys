#pragma once

#include <QDialog>
#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QMainWindow>

#include "climber.h"

namespace Ui {
class Payment;
}

class Payment : public QDialog
{
    Q_OBJECT

public slots:
    void updateClimberInfo(Climber *&climber);

public:
    explicit Payment(QWidget *parent = 0);
    ~Payment();
    
private slots:
    void on_comboBox_currentIndexChanged(int index);
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

signals:
    void setExpirationDate(QDate date);

private:
    Ui::Payment *ui;
    QString name;
    QDate expirationDate;
};