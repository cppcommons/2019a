#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include <QtCore>

namespace Ui {
class Form;
}

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = nullptr);
    ~Form();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_proc_enum_clicked();

    void on_timer1_timeout();

    void on_spotify_elapse_timer_timeout();

private:
    Ui::Form *ui;
    QTimer timer1;
    QString spotify_text;
    QTimer spotify_elapse_timer;
    QDateTime spotify_start_time;
    QString get_spotify_text();
};

#endif // FORM_H
