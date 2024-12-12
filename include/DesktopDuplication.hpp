#include <windows.h>
#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <string>
#include <SetupAPI.h>
#include <devguid.h>

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

    class DuplMan {
        public:
        DuplMan();
        ~DuplMan();
        bool InitDuplication();
        bool InitDuplication(_In_ ID3D11Device* device, UINT output);
        void SetOutput(UINT adapterIndex, UINT outputIndex);
        ID3D11Device* GetDevice() { return m_Device.Get(); }
        bool IsOutputSet() { return m_Output != -1; }

        private:
        ComPtr<ID3D11Device5> m_Device;
        ComPtr<IDXGIOutputDuplication> m_DesktopDupl;
        ComPtr<ID3D11Texture2D> m_AcquiredDesktopImage;
        UINT m_Output;
    };

    void ChooseOutput();
    bool ChooseOutput(_Out_ UINT& adapterIndex, _Out_ UINT& outputIndex);
    UINT enumOutputs(IDXGIAdapter* adapter);
    std::wstring GetMonitorFriendlyName(const DXGI_OUTPUT_DESC1& desc);
    std::wstring GetMonitorNameFromEDID(const std::wstring& deviceName);
}