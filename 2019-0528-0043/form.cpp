#include "form.h"
#include "ui_form.h"

#include <QtGui>
#include <QtWidgets>

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);
    this->setFixedSize(QSize(750, 400));
    ui->plainTextEdit->setReadOnly(true);
}

Form::~Form()
{
    delete ui;
}

void Form::on_pushButton_clicked()
{
    QMessageBox::information(this, tr(u8"タイトル"), tr(u8"メッセージ"));
    this->ui->plainTextEdit->appendPlainText(u8"ABCハロー");
    this->ui->plainTextEdit->verticalScrollBar()->setValue(this->ui->plainTextEdit->verticalScrollBar()->maximum());
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
        wprintf(L"ProcessName:%s, ProcessId:%d, SessionId:%d HWND:0x%08x Title:%s\n",
                //GetProcessPath(ProcessInfo[i].ProcessId).c_str(), //ProcessInfo[i].pProcessName,
                GetProcessPath(ProcessInfo[i].ProcessId).toStdWString().c_str(), //ProcessInfo[i].pProcessName,
                ProcessInfo[i].ProcessId,
                ProcessInfo[i].SessionId,
                hwnd, title);
    }
    //WTS_PROCESS_INFO構造体配列のメモリを解放する
    WTSFreeMemory(ProcessInfo);
}
