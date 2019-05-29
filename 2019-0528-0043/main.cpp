//#include <QCoreApplication>
#include <QtCore>
#include <QtGui>
#include <QtWidgets>

#include <objbase.h>

#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <avrt.h>
#include <functiondiscoverykeys_devpkey.h>

#include "form.h"

class PropVariantClearOnExit {
public:
    PropVariantClearOnExit(PROPVARIANT *p) : m_p(p) {}
    ~PropVariantClearOnExit() {
        HRESULT hr = PropVariantClear(m_p);
        if (FAILED(hr)) {
            ////ERR(L"PropVariantClear failed: hr = 0x%08x", hr);
        }
    }

private:
    PROPVARIANT *m_p;
};

class ReleaseOnExit {
public:
    ReleaseOnExit(IUnknown *p) : m_p(p) {}
    ~ReleaseOnExit() {
        m_p->Release();
    }

private:
    IUnknown *m_p;
};

class App: public QApplication //QCoreApplication
{
    Q_OBJECT
public:
    App(int &argc, char **argv): QApplication (argc, argv) {
        qDebug() << "App() called!";
        CoInitialize(NULL);
    }
    ~App() {
        qDebug() << "~App() called!";
        CoUninitialize();
    }
    HRESULT list_devices() {
        HRESULT hr = S_OK;

        // get an enumerator
        IMMDeviceEnumerator *pMMDeviceEnumerator;

        hr = CoCreateInstance(
            __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
            __uuidof(IMMDeviceEnumerator),
            (void**)&pMMDeviceEnumerator
        );
        if (FAILED(hr)) {
            ////ERR(L"CoCreateInstance(IMMDeviceEnumerator) failed: hr = 0x%08x", hr);
            return hr;
        }
        ReleaseOnExit releaseMMDeviceEnumerator(pMMDeviceEnumerator);

        IMMDeviceCollection *pMMDeviceCollection;

        // get all the active render endpoints
        hr = pMMDeviceEnumerator->EnumAudioEndpoints(
            eRender, DEVICE_STATE_ACTIVE, &pMMDeviceCollection
        );
        if (FAILED(hr)) {
            ////ERR(L"IMMDeviceEnumerator::EnumAudioEndpoints failed: hr = 0x%08x", hr);
            return hr;
        }
        ReleaseOnExit releaseMMDeviceCollection(pMMDeviceCollection);

        UINT count;
        hr = pMMDeviceCollection->GetCount(&count);
        if (FAILED(hr)) {
            ////ERR(L"IMMDeviceCollection::GetCount failed: hr = 0x%08x", hr);
            return hr;
        }
        ////LOG(L"Active render endpoints found: %u", count);

        for (UINT i = 0; i < count; i++) {
            IMMDevice *pMMDevice;

            // get the "n"th device
            hr = pMMDeviceCollection->Item(i, &pMMDevice);
            if (FAILED(hr)) {
                ////ERR(L"IMMDeviceCollection::Item failed: hr = 0x%08x", hr);
                return hr;
            }
            ReleaseOnExit releaseMMDevice(pMMDevice);

            // open the property store on that device
            IPropertyStore *pPropertyStore;
            hr = pMMDevice->OpenPropertyStore(STGM_READ, &pPropertyStore);
            if (FAILED(hr)) {
                ////ERR(L"IMMDevice::OpenPropertyStore failed: hr = 0x%08x", hr);
                return hr;
            }
            ReleaseOnExit releasePropertyStore(pPropertyStore);

            // get the long name property
            PROPVARIANT pv; PropVariantInit(&pv);
            hr = pPropertyStore->GetValue(PKEY_Device_FriendlyName, &pv);
            if (FAILED(hr)) {
                ////ERR(L"IPropertyStore::GetValue failed: hr = 0x%08x", hr);
                return hr;
            }
            PropVariantClearOnExit clearPv(&pv);

            if (VT_LPWSTR != pv.vt) {
                /////ERR(L"PKEY_Device_FriendlyName variant type is %u - expected VT_LPWSTR", pv.vt);
                return E_UNEXPECTED;
            }

            ////LOG(L"    %ls", pv.pwszVal);
            ////printf("    %s\n", wide_to_ansi(std::wstring(pv.pwszVal)).c_str());
            qDebug() << QString::fromWCharArray(pv.pwszVal);
        }

        return S_OK;
    }
public slots:
    void run() {
        qDebug() << "App::run() called!";
        list_devices();
        //this->exit();
    }
};

int main(int argc, char *argv[])
{
    App a(argc, argv);
    QTimer t;
    QObject::connect(&t, SIGNAL(timeout()), &a, SLOT(run()));
    t.setSingleShot(true);
    t.start();
    qDebug() << u8"テストtest";
    qDebug() << QString::fromUtf8(u8"テストtest");
    printf("%s\n", QString::fromUtf8(u8"テストtest").toLocal8Bit().constData());

    Form f;
    f.show();

    return a.exec();
}

#include "main.moc"
