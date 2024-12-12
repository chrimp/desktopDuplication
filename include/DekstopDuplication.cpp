#include "./DesktopDuplication.hpp"

#include <iostream> 
#include <conio.h>
#include <vector>
#include <initguid.h>
#include <Ntddvdeo.h>
#include <SetupAPI.h>

#pragma comment(lib, "SetupAPI.lib")

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

void DuplMan::SetOutput(UINT adapterIndex, UINT outputIndex) {
    m_Output = outputIndex;
    // adapterIndex is not used for now
}

bool DesktopDuplication::ChooseOutput(_Out_ UINT& adapterIndex, _Out_ UINT& outputIndex) {
    // Currently assuming there is only one adapter
    IDXGIFactory* factory = nullptr;

    HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&factory));
    if (FAILED(hr)) {
        std::cerr << "Failed to create DXGI Factory. Reason: 0x" << std::hex << hr << std::endl;
        return false;
    }

    adapterIndex = 0;
    IDXGIAdapter* adapter = nullptr;

    hr = factory->EnumAdapters(adapterIndex, &adapter);
    factory->Release();
    factory = nullptr;
    if (FAILED(hr)) {
        std::cerr << "Failed to enumerate adapters. Reason: 0x" << std::hex << hr << std::endl;
        return false;
    }

    UINT lastOutputNum = enumOutputs(adapter);
    adapter->Release();
    adapter = nullptr;

    bool validOutput = false;
    while (!validOutput) {
        std::cout << std::endl;
        std::cout << "Choose an output to capture: ";
        char input = _getch();
        UINT output = input - '0';

        if (output >= lastOutputNum || output < 0) {
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
            outputIndex = output;
            system("cls");
        }
    }

    return true;
}

void DesktopDuplication::ChooseOutput() {
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
        UINT output = input - '0';

        if (output >= lastOutputNum || output < 0) {
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
            Singleton<DuplMan>::Instance().SetOutput(0, output);
            system("cls");
        }
    }

    return;
}

UINT DesktopDuplication::enumOutputs(IDXGIAdapter* adapter) {
    UINT outputIndex = 0;
    IDXGIOutput* output = nullptr;

    while (adapter->EnumOutputs(outputIndex, &output) != DXGI_ERROR_NOT_FOUND) {
        // QI for IDXGIOutput6
        IDXGIOutput6* output6 = nullptr;
        HRESULT hr = output->QueryInterface(IID_PPV_ARGS(&output6));
        if (FAILED(hr)) {
            std::cerr << "Failed to query IDXGIOutput6 interface. Reason: 0x" << std::hex << hr << std::endl;
            throw std::exception();
        }

        DXGI_OUTPUT_DESC1 desc;

        if (SUCCEEDED(output6->GetDesc1(&desc))) {
            std::wstring monitorName = GetMonitorFriendlyName(desc);
            std::wcout << L"Output " << outputIndex << L": " << monitorName << L"\n";
            
            DEVMODE devMode = {};
            devMode.dmSize = sizeof(DEVMODE);

            if (EnumDisplaySettings(desc.DeviceName, ENUM_CURRENT_SETTINGS, &devMode)) {
                std::cout << "Current Mode: " << devMode.dmPelsWidth << "x" << devMode.dmPelsHeight << "@" << devMode.dmDisplayFrequency << "Hz" << std::endl;
            } else {
                std::wcout << "Unable to get current display mode for output " << monitorName << "." << std::endl;
                std::cout << "Reason: 0x" << std::hex << GetLastError() << std::endl;
                throw std::exception();
            }
        } else {
            std::cout << "Unable to get display information for output #" << outputIndex << "." << std::endl;
        }

        output->Release();
        output = nullptr;
        outputIndex++;
    }
    return outputIndex;
}

std::wstring DesktopDuplication::GetMonitorNameFromEDID(const std::wstring& deviceName) {
    std::vector<BYTE> edid;

    DISPLAY_DEVICE dd;
    dd.cb = sizeof(DISPLAY_DEVICE);

    if (EnumDisplayDevices(deviceName.c_str(), 0, &dd, 0)) {
        HDEVINFO devInfo = SetupDiGetClassDevsEx(&GUID_DEVINTERFACE_MONITOR, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE, NULL, NULL, NULL);
    
        if (devInfo == INVALID_HANDLE_VALUE) {
            return L"";
        }

        SP_DEVINFO_DATA devInfoData;
        devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

        if (SetupDiEnumDeviceInfo(devInfo, 0, &devInfoData)) {
            HKEY hDevRegKey = SetupDiOpenDevRegKey(devInfo, &devInfoData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);

            if (hDevRegKey == INVALID_HANDLE_VALUE) {
                SetupDiDestroyDeviceInfoList(devInfo);
                return L"";
            }

            DWORD edidSize = 0;
            RegQueryValueEx(hDevRegKey, L"EDID", NULL, NULL, NULL, &edidSize);

            if (edidSize != 0) {
                edid.resize(edidSize);
                RegQueryValueEx(hDevRegKey, L"EDID", NULL, NULL, edid.data(), &edidSize);
            }

            RegCloseKey(hDevRegKey);
        }

        SetupDiDestroyDeviceInfoList(devInfo);
    }

    if (edid.size() == 0) {
        return L"";
    }

    // Parse EDID (Range: 54-71, 72-89, 90-107, 108-125) (Look for: 0x00 0x00 0x00 0xFC)
    std::wstring monitorName = L"";
    std::vector<std::pair<size_t, size_t>> ranges = {{54, 71}, {72, 89}, {90, 107}, {108, 125}};
    
    for (const auto& range : ranges) {
        for (size_t i = range.first; i <= range.second && i + 3 < edid.size(); i++) {
            if (edid[i] == 0x00 && edid[i + 1] == 0x00 && edid[i + 2] == 0x00 && edid[i + 3] == 0xFC) {
                i += 5;
                while (i < edid.size() && edid[i] != 0x0A) {
                    monitorName += edid[i];
                    i++;
                }
                break;
            }
        }
        if (!monitorName.empty()) {
            break;
        }
    }

    return monitorName;
}

std::wstring DesktopDuplication::GetMonitorFriendlyName(const DXGI_OUTPUT_DESC1& desc) {
    // Generic PnP Monitor

    std::wstring monitorName = L"Unknown";

    DISPLAY_DEVICE displayDevice;
    displayDevice.cb = sizeof(DISPLAY_DEVICE);

    if (EnumDisplayDevices(desc.DeviceName, 0, &displayDevice, 0)) {
        monitorName = displayDevice.DeviceString;
    }

    if (monitorName == L"Generic PnP Monitor") {
        monitorName = GetMonitorNameFromEDID(desc.DeviceName);
    }
    
    return monitorName;
}