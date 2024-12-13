#include <windows.h>
#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <string>
#include <SetupAPI.h>
#include <devguid.h>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <mutex>
#include <filesystem>
#include <vector>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "SetupAPI.lib")

using Microsoft::WRL::ComPtr;

namespace DesktopDuplication {
    template <typename T>
    class Singleton {
        public:
        static T& Instance() {
            static T instance;
            return instance;
        }

        Singleton(const Singleton&) = delete;
        Singleton& operator=(const Singleton&) = delete;

        protected:
        Singleton() = default;
        ~Singleton() = default;
    };

    class Duplication {
        public:
        Duplication();
        ~Duplication();

        ID3D11Device5* GetDevice() { return m_Device.Get(); }

        void SetOutput(UINT adapterIndex, UINT outputIndex);
        void GetTelemetry(_Out_ unsigned long long& frameCount, _Out_ unsigned int& framePerUnit);

        bool InitDuplication();
        bool IsOutputSet() { return m_Output != -1; }
        bool SaveFrame(const std::filesystem::path& path);
        int GetFrame(_Out_ ID3D11Texture2D*& frame, _In_ unsigned long timemout = 16);

        std::atomic<bool> ShowPreview;

        private:
        void duplicationThread();
        void releaseFrame();

        bool stageFrame(_Out_ std::vector<uint8_t>& dst, _Out_ D3D11_TEXTURE2D_DESC& descDst);

        ComPtr<ID3D11Device5> m_Device;
        ComPtr<ID3D11DeviceContext4> m_Context;
        ComPtr<IDXGIOutputDuplication> m_DesktopDupl;
        ComPtr<ID3D11Texture2D> m_AcquiredDesktopImage;

        UINT m_Output;

        bool m_IsDuplRunning;

        // Telemetry
        std::atomic<unsigned long long> m_FrameCount;
        std::atomic<unsigned int> m_FramePerUnit;
    };

    void ChooseOutput();
    bool ChooseOutput(_Out_ UINT& adapterIndex, _Out_ UINT& outputIndex);
    UINT enumOutputs(IDXGIAdapter* adapter);
    std::wstring GetMonitorFriendlyName(const DXGI_OUTPUT_DESC1& desc);
    std::wstring GetMonitorNameFromEDID(const std::wstring& deviceName);
}