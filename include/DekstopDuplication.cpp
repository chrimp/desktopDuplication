#include "./DesktopDuplication.hpp"

#include <iomanip>
#include <iostream> 
#include <conio.h>

using namespace DesktopDuplication;

DuplMan::DuplMan() {
    m_Device = nullptr;
    m_DesktopDupl = nullptr;
    m_AcquiredDesktopImage = nullptr;
    m_Output = -1;
}

DuplMan::~DuplMan() {
    if (m_Device) {
        m_Device.Reset();
        m_Device = nullptr;
    }

    if (m_DesktopDupl) {
        m_DesktopDupl->ReleaseFrame();
        m_DesktopDupl.Reset();
        m_DesktopDupl = nullptr;
    }

    if (m_AcquiredDesktopImage) {
        m_AcquiredDesktopImage.Reset();
        m_AcquiredDesktopImage = nullptr;
    }
}

bool DuplMan::InitDuplication() {
    if (m_Output == -1) {
        std::cout << "Output is not set. Call InitDuplication(ID3D11Device*, UINT) or ChooseOutput() to set the output." << std::endl;
        return false;
    }

    UINT flag = 0;
    #ifdef _DEBUG
    flag |= D3D11_CREATE_DEVICE_DEBUG;
    #endif

    ID3D11Device* device = nullptr;

    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flag,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &device,
        nullptr,
        nullptr
    );

    if (FAILED(hr)) {
        std::cerr << "Failed to create D3D11 device. Reason: 0x" << std::hex << hr << std::endl;
        return false;
    }

    // QI for ID3D11Device5
    hr = device->QueryInterface(IID_PPV_ARGS(m_Device.GetAddressOf()));
    if (FAILED(hr)) {
        std::cerr << "Failed to query D3D11Device5 interface. Reason: 0x" << std::hex << hr << std::endl;
        return false;
    }

    // QI for IDXGIDevice
    IDXGIDevice* dxgiDevice = nullptr;
    hr = m_Device->QueryInterface(IID_PPV_ARGS(&dxgiDevice));
    if (FAILED(hr)) {
        std::cerr << "Failed to query IDXGIDevice interface. Reason: 0x" << std::hex << hr << std::endl;
        return false;
    }

    // Get DXGI adapter
    IDXGIAdapter* dxgiAdapter = nullptr;
    hr = dxgiDevice->GetParent(IID_PPV_ARGS(&dxgiAdapter));
    dxgiDevice->Release();
    dxgiDevice = nullptr;
    if (FAILED(hr)) {
        std::cerr << "Failed to get DXGI adapter. Reason: 0x" << std::hex << hr << std::endl;
        return false;
    }

    // Get Output (Implement enum n list for user to choose)
    IDXGIOutput* dxgiOutput = nullptr;
    hr = dxgiAdapter->EnumOutputs(0, &dxgiOutput); // Replace 0 with user choice (WIP)
    dxgiAdapter->Release();
    dxgiAdapter = nullptr;
    if (FAILED(hr)) {
        std::cerr << "Failed to get DXGI output. Reason: 0x" << std::hex << hr << std::endl;
        return false;
    }

    // QI for IDXGIOutput1
    IDXGIOutput1* dxgiOutput1 = nullptr;
    hr = dxgiOutput->QueryInterface(IID_PPV_ARGS(&dxgiOutput1));
    dxgiOutput->Release();
    dxgiOutput = nullptr;
    if (FAILED(hr)) {
        std::cerr << "Failed to query IDXGIOutput1 interface. Reason: 0x" << std::hex << hr << std::endl;
        return false;
    }

    // Create Desktop Duplication
    hr = dxgiOutput1->DuplicateOutput(m_Device.Get(), m_DesktopDupl.GetAddressOf());
    dxgiOutput1->Release();
    dxgiOutput1 = nullptr;
    if (FAILED(hr)) {
        if (hr == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE) {
            std::cerr << "There are already maximum number of applications using Desktop Duplication API." << std::endl;
        } else {
            std::cerr << "Failed to create Desktop Duplication. Reason: 0x" << std::hex << hr << std::endl;
        }
        return false;
    }

    return true;
}

void DuplMan::ChooseOutput() {
    // Currently assuming there is only one adapter
    IDXGIFactory* factory = nullptr;

    HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&factory));
    if (FAILED(hr)) {
        std::cerr << "Failed to create DXGI Factory. Reason: 0x" << std::hex << hr << std::endl;
        return;
    }

    UINT adapterIndex = 0;
    IDXGIAdapter* adapter = nullptr;

    hr = factory->EnumAdapters(adapterIndex, &adapter);
    factory->Release();
    factory = nullptr;
    if (FAILED(hr)) {
        std::cerr << "Failed to enumerate adapters. Reason: 0x" << std::hex << hr << std::endl;
        return;
    }

    UINT lastOutputNum = enumOutputs(adapter);
    adapter->Release();
    adapter = nullptr;

    bool validOutput = false;
    while (!validOutput) {
        std::cout << std::endl;
        std::cout << "Choose an output to capture: ";
        char input = _getch();
        m_Output = input - '0';

        if (m_Output >= lastOutputNum || m_Output < 0) {
            std::cout << "Invalid output number. Please choose a valid output." << std::endl;
            std::cout << "Press any key to continue..." << std::endl;
            _getch();
            std::cout << "\033[2A"
                      << "\033[K"
                      << "\033[1B"
                      << "\033[K"
                      << "\033[1A";
        } else {
            validOutput = true;
            system("cls");
        }
    }

    return;
}

UINT DuplMan::enumOutputs(IDXGIAdapter* adapter) {
    // Create D3D device to get current display mode
    ID3D11Device* device = nullptr;
    HRESULT hr = D3D11CreateDevice(
        adapter,
        D3D_DRIVER_TYPE_UNKNOWN,
        nullptr,
        D3D11_CREATE_DEVICE_DEBUG,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &device,
        nullptr,
        nullptr
    );

    if (FAILED(hr)) {
        std::cerr << "Failed to create D3D11 device. Reason: 0x" << std::hex << hr << std::endl;
        throw std::exception();
    }


    UINT outputIndex = 0;
    IDXGIOutput* output = nullptr;

    DXGI_MODE_DESC unspecifiedMode = {0};

    while (adapter->EnumOutputs(outputIndex, &output) != DXGI_ERROR_NOT_FOUND) {
        DXGI_OUTPUT_DESC desc;

        if (SUCCEEDED(output->GetDesc(&desc))) {
            std::wstring monitorName = GetMonitorFriendlyName(desc);

            std::wcout << L"Output " << outputIndex << L": " << monitorName << L"\n";

            DXGI_MODE_DESC currentMode;
            if (SUCCEEDED(output->FindClosestMatchingMode(&unspecifiedMode, &currentMode, device))) {
                UINT rate = currentMode.RefreshRate.Numerator / currentMode.RefreshRate.Denominator;
                std::wcout << L"Current Mode: " << currentMode.Width << L"x" << currentMode.Height
                           << L"@" << std::fixed << std::setprecision(2) << rate << L"Hz" << std::endl;
            } else {
                std::wcout << L"Unable to get current display mode for output " << monitorName << "." << std::endl;
                HRESULT hr = output->FindClosestMatchingMode(&unspecifiedMode, &currentMode, device);
                std::cout << "Reason: 0x" << std::hex << hr << std::endl;
                throw std::exception();
            }
        } else {
            std::cout << "Unable to get display information for output #" << outputIndex << "." << std::endl;
        }

        output->Release();
        output = nullptr;
        outputIndex++;
    }

    device->Release();
    device = nullptr;

    return outputIndex;
}

std::wstring DesktopDuplication::GetMonitorFriendlyName(const DXGI_OUTPUT_DESC& desc) {
    DISPLAY_DEVICE displayDevice;
    displayDevice.cb = sizeof(DISPLAY_DEVICE);

    if (EnumDisplayDevices(desc.DeviceName, 0, &displayDevice, 0)) {
        std::wstring monitorName = displayDevice.DeviceString;
        return monitorName;
    }

    return L"Unknown";
}