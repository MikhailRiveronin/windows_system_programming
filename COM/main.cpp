#include <windows.h>
#include <wrl.h>
#include <stdio.h>
#include <Bits.h>

using namespace Microsoft::WRL;

int main()
{
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    ComPtr<IBackgroundCopyManager> mgr;
    HRESULT hr = CoCreateInstance(__uuidof(BackgroundCopyManager), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&mgr));
    if (FAILED(hr))
    {
        return -1;
    }

    GUID jobId;
    ComPtr<IBackgroundCopyJob> pJob;
    hr = mgr->CreateJob(L"My job", BG_JOB_TYPE_DOWNLOAD, &jobId, &pJob);
    if (FAILED(hr))
    {
        return -1;
    }

    hr = pJob->AddFile(L"https://upload.wikimedia.org/wikipedia/commons/4/47/PNG_transparency_demonstration_1.png", L"D:\\Downloads\\tmp\\image.png");
    if (FAILED(hr))
    {
        return -1;
    }

    hr = pJob->Resume();
    if (SUCCEEDED(hr))
    {
        printf("Downloading... ");
        BG_JOB_STATE state;
        for (;;)
        {
            pJob->GetState(&state);
            if (state == BG_JOB_STATE_ERROR || state == BG_JOB_STATE_TRANSFERRED)
                break;
            printf(".");
            ::Sleep(300);
        }
        if (state == BG_JOB_STATE_ERROR)
        {
            printf("\nError in transfer!\n");
        }
        else
        {
            pJob->Complete();
            printf("\nTransfer successful!\n");
        }
    }

    return 0;
}
