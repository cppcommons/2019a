#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include <QtCore>
#include <windows.h>

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

    void on_pushButton_evant_callback_clicked();

private:
    Ui::Form *ui;
    QTimer timer1;
    QString spotify_text;
    QTimer spotify_elapse_timer;
    QDateTime spotify_start_time;
    QString get_spotify_text();
    bool find_spotify(DWORD *procId, HWND *hwnd);
    static void __stdcall EventProc(
            HWINEVENTHOOK hWinEventHook,
            DWORD event,
            HWND hwnd,
            LONG idObject,
            LONG idChild,
            DWORD idEventThread,
            DWORD dwmsEventTime
          );
};

#endif // FORM_H
