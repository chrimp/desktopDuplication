#include <windows.h>
#include <d3d11_4.h>
#include <dxgi1_5.h>
#include <wrl/client.h>
#include <string>
#include <SetupAPI.h>
#include <devguid.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "SetupAPI.lib")

using Microsoft::WRL::ComPtr;

namespace DesktopDuplication {
    class DuplMan {
        public:
        DuplMan();
        ~DuplMan();
        bool InitDuplication();
        bool InitDuplication(_In_ ID3D11Device* device, UINT output);
        void ChooseOutput();
        ID3D11Device* GetDevice() { return m_Device.Get(); }
        bool IsOutputSet() { return m_Output != -1; }

        private:
        ComPtr<ID3D11Device5> m_Device;
        ComPtr<IDXGIOutputDuplication> m_DesktopDupl;
        ComPtr<ID3D11Texture2D> m_AcquiredDesktopImage;
        UINT m_Output;

        UINT enumOutputs(IDXGIAdapter* adapter);
    };

    std::wstring GetMonitorFriendlyName(const DXGI_OUTPUT_DESC& desc);
}