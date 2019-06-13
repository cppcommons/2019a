#include "form.h"
#include "ui_form.h"

#include <QtGui>
#include <QtWidgets>

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags()|Qt::MSWindowsFixedSizeDialogHint);
    ui->plainTextEdit->setReadOnly(true);
    QObject::connect(&timer1, SIGNAL(timeout()), this, SLOT(on_timer1_timeout()));
    QObject::connect(&spotify_elapse_timer, SIGNAL(timeout()), this, SLOT(on_spotify_elapse_timer_timeout()));
}

Form::~Form()
{
    delete ui;
}

void Form::on_pushButton_clicked()
{
#if 0x0
    QMessageBox::information(this, tr(u8"タイトル"), tr(u8"メッセージ"));
    this->ui->plainTextEdit->appendPlainText(u8"ABCハロー");
    this->ui->plainTextEdit->verticalScrollBar()->setValue(this->ui->plainTextEdit->verticalScrollBar()->maximum());
#endif
    this->spotify_text = "";
    this->timer1.setInterval(1);
    //this->timer1.setInterval(10);
    this->timer1.start();
}

void Form::on_timer1_timeout()
{
    //qDebug() << "Form::on_timer1_timeout()" << get_spotify_text();
    QString text = get_spotify_text();
    if(text == spotify_text) return;
    spotify_text = text;
    spotify_start_time = QDateTime::currentDateTime();
    spotify_elapse_timer.stop();
    spotify_elapse_timer.setInterval(5000);
    spotify_elapse_timer.start();
    qDebug() << "Form::on_timer1_timeout()" << text;
}

void Form::on_spotify_elapse_timer_timeout()
{
    qDebug() << "Form::on_spotify_elapse_timer_timeout()" << spotify_start_time.secsTo(QDateTime::currentDateTime());
}

#include <Windows.h>
#include <stdio.h>
#include <psapi.h>
#include <wtsapi32.h>
#include <string>

#if 0x0
std::wstring GetProcessPath(DWORD processId)
{
    wchar_t filename[MAX_PATH];
    HANDLE processHandle = NULL;
    processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    GetModuleFileNameEx(processHandle, NULL, filename, MAX_PATH);
    CloseHandle(processHandle);
    QString s = QString::fromWCharArray(filename);
    QFileInfo fi(s);
    return fi.fileName().toStdWString();
}
#else
QString GetProcessPath(DWORD processId)
{
    wchar_t filename[MAX_PATH];
    HANDLE processHandle = NULL;
    processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    GetModuleFileNameEx(processHandle, NULL, filename, MAX_PATH);
    CloseHandle(processHandle);
    QString s = QString::fromWCharArray(filename);
    QFileInfo fi(s);
    return fi.fileName();
}
#endif

//
// プロセスIDからウィンドウハンドルを取得する。
// トップレベルウィンドウを列挙して、プロセスIDが一致するやつのHWNDを返す。
// 同じプロセスIDでトップレベルウィンドウが複数個あった場合は
// どうなっても知らない。
//
HWND GetWindowHandle(	// 戻り値: 成功 望みのHWND / 失敗 NULL
    const DWORD TargetID)	// プロセスID
{
    HWND hWnd = GetTopWindow(NULL);
    do {
        if(GetWindowLong( hWnd, GWL_HWNDPARENT) != 0 || !IsWindowVisible( hWnd))
            continue;
        DWORD ProcessID;
        GetWindowThreadProcessId( hWnd, &ProcessID);
        if(TargetID == ProcessID)
            return hWnd;
    } while((hWnd = GetNextWindow( hWnd, GW_HWNDNEXT)) != NULL);

    return NULL;
}

void Form::on_pushButton_proc_enum_clicked()
{
    HANDLE hServer = WTS_CURRENT_SERVER_HANDLE;
    PWTS_PROCESS_INFOW ProcessInfo;
    DWORD dwCount = 0;
    //プロセス情報を取得する
    WTSEnumerateProcessesW(hServer,
                           0,
                           1,
                           &ProcessInfo,
                           &dwCount);
    //取得したプロセス情報（プロセス名、プロセスID、セッションID）をコンソールに表示する
    for (DWORD i=0; i < dwCount; i++) {
        HWND hwnd = GetWindowHandle(ProcessInfo[i].ProcessId);
        wchar_t title[1024+1];
        GetWindowTextW(hwnd, title, 1024);
        if(!hwnd) continue;
        qDebug() << "ProcessName:" << GetProcessPath(ProcessInfo[i].ProcessId)
                 << "ProcessId:" << ProcessInfo[i].ProcessId
                 << "HWND:" << hwnd
                 << "Title:" << QString::fromWCharArray(title);
#if 0x0
        wprintf(L"ProcessName:%s, ProcessId:%d, SessionId:%d HWND:0x%08x Title:%s\n",
                //GetProcessPath(ProcessInfo[i].ProcessId).c_str(), //ProcessInfo[i].pProcessName,
                GetProcessPath(ProcessInfo[i].ProcessId).toStdWString().c_str(), //ProcessInfo[i].pProcessName,
                ProcessInfo[i].ProcessId,
                ProcessInfo[i].SessionId,
                hwnd, title);
#endif
    }
    //WTS_PROCESS_INFO構造体配列のメモリを解放する
    WTSFreeMemory(ProcessInfo);
}

QString Form::get_spotify_text()
{
    HANDLE hServer = WTS_CURRENT_SERVER_HANDLE;
    PWTS_PROCESS_INFOW ProcessInfo;
    DWORD dwCount = 0;
    WTSEnumerateProcessesW(hServer,
                           0,
                           1,
                           &ProcessInfo,
                           &dwCount);
    for (DWORD i=0; i < dwCount; i++) {
        QString processName = GetProcessPath(ProcessInfo[i].ProcessId);
        if(processName.toLower() != "spotify.exe") continue;
        HWND hwnd = GetWindowHandle(ProcessInfo[i].ProcessId);
        if(!hwnd) continue;
        wchar_t title[4096+1];
        GetWindowTextW(hwnd, title, 4096);
        WTSFreeMemory(ProcessInfo);
        return QString::fromWCharArray(title);
    }
    WTSFreeMemory(ProcessInfo);
    return "";
}

bool Form::find_spotify(DWORD *procId, HWND *hwnd)
{
    HANDLE hServer = WTS_CURRENT_SERVER_HANDLE;
    PWTS_PROCESS_INFOW ProcessInfo;
    DWORD dwCount = 0;
    WTSEnumerateProcessesW(hServer,
                           0,
                           1,
                           &ProcessInfo,
                           &dwCount);
    for (DWORD i=0; i < dwCount; i++) {
        QString processName = GetProcessPath(ProcessInfo[i].ProcessId);
        if(processName.toLower() != "spotify.exe") continue;
        *procId = ProcessInfo[i].ProcessId;
        *hwnd = GetWindowHandle(ProcessInfo[i].ProcessId);
        if(!*hwnd) continue;
        if(GetWindowLongW(*hwnd, GWL_HWNDPARENT) != 0 || !IsWindowVisible(*hwnd)) continue;
        WTSFreeMemory(ProcessInfo);
        return true;
    }
    WTSFreeMemory(ProcessInfo);
    return false;
}

void Form::EventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD idEventThread, DWORD dwmsEventTime)
{
    if(GetWindowLongW(hwnd, GWL_HWNDPARENT) != 0 || !IsWindowVisible(hwnd)) return;
    qDebug() << "EventProc" << event;
    switch(event) {
    case EVENT_OBJECT_NAMECHANGE:
        wchar_t title[4096+1];
        GetWindowTextW(hwnd, title, 4096);
        qDebug() << "EVENT_OBJECT_NAMECHANGE" << QString::fromWCharArray(title);
        break;
    }
}


void Form::on_pushButton_evant_callback_clicked()
{
    DWORD procId;
    HWND hwnd;
    if(find_spotify(&procId, &hwnd)) {
        HWINEVENTHOOK hHook = SetWinEventHook(
          EVENT_MIN, //DWORD        eventMin,
          EVENT_MAX, //DWORD        eventMax,
          nullptr, //HMODULE      hmodWinEventProc,
          Form::EventProc, //WINEVENTPROC pfnWinEventProc,
          procId, //DWORD        idProcess,
          0, //DWORD        idThread,
          WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS //DWORD        dwFlags
        );
    } else {
        qDebug() << "spotify not found!";
    }
}
