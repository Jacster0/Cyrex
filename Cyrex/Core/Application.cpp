#include "Application.h"
#include "Platform/Windows/Window.h"
#include "Logger.h"
#include "Console.h"
#include "Graphics/Graphics.h"
#include "Graphics/API/DX12/DXException.h"
#include "Graphics/API/DX12/CommandQueue.h"

using namespace Cyrex;
namespace wrl = Microsoft::WRL;

uint64_t Cyrex::Application::ms_frameCount = 0;

Cyrex::Application::Application() {
	if (Console::Instance()) {
		Console::Show();
	}
}

void Cyrex::Application::Create() {
	Get().Initialize();
}

int Cyrex::Application::Run() {
	if (m_isInitialized) {
		m_gfx->Initialize(m_wnd->GetWidth(), m_wnd->GetHeight());

		if (!m_gfx->IsInitialized()) {
			crxlog::err("Cannot render to screen because no Graphics object have been intialized");
		}

		m_wnd->Show();
	}
	else {
		crxlog::err("Application is not intialized");
	}

	while (true) {
		if (const auto ecode = MessagePump()) {
			//return exit code
			return *ecode;
		}
		else {
			//Render/Graphics stuff
			if (m_gfx && m_gfx->IsInitialized()) {
				Application::FrameCount()++;
				HandleInput();
				m_gfx->Update();
				m_gfx->Render();
			}
		}
	}
}

void Cyrex::Application::Initialize() {
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	EnableDebugLayer();

	m_wnd = std::make_unique<Window>();
	m_gfx = std::make_shared<Graphics>();

	m_wnd->Gfx = m_gfx;
	m_gfx->SetHwnd(m_wnd->GetHWnd());

	auto dxgiAdapter = GetAdapter(false);
	if (dxgiAdapter) {
		m_device = CreateDevice(dxgiAdapter);
	}

	CreateCommandQueue();
	m_tearingSupported = CheckTearingSupport();
	m_isInitialized = true;
}

void Cyrex::Application::Flush() const {
	m_directCommandQueue->Flush();
	m_computeCommandQueue->Flush();
	m_copyCommandQueue->Flush();
}

Application& Cyrex::Application::Get() noexcept {
	static Application instance;
	return instance;
}

wrl::ComPtr<ID3D12Device2> Cyrex::Application::GetDevice() const noexcept {
	return m_device;
}

wrl::ComPtr<ID3D12Device2> Cyrex::Application::CreateDevice(wrl::ComPtr<IDXGIAdapter4> adapter) {
	wrl::ComPtr<ID3D12Device2> device;
	ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));

	//enable debug messages in debug mode
#if defined(_DEBUG)
	wrl::ComPtr<ID3D12InfoQueue> p_infoQueue;

	if (SUCCEEDED(device.As(&p_infoQueue))) {
		p_infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		p_infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		p_infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		std::array severities = { D3D12_MESSAGE_SEVERITY_INFO };
		std::array denyIds = {
			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
		};

		D3D12_INFO_QUEUE_FILTER newFilter = {};
		newFilter.DenyList.NumSeverities = severities.size();
		newFilter.DenyList.pSeverityList = severities.data();
		newFilter.DenyList.NumIDs        = denyIds.size();
		newFilter.DenyList.pIDList       = denyIds.data();

		ThrowIfFailed(p_infoQueue->PushStorageFilter(&newFilter));
	}
#endif

	return device;
}

wrl::ComPtr<IDXGIAdapter4> Cyrex::Application::GetAdapter(bool useWarp) {
	wrl::ComPtr<IDXGIFactory4> dxgiFactory;
	uint32_t createFactoryFlags = 0;

#if defined(_DEBUG)
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

	wrl::ComPtr<IDXGIAdapter1> dxgiAdapter1;
	wrl::ComPtr<IDXGIAdapter4> dxgiAdapter4;

	if (useWarp) {
		ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)));
		ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
	}
	else {
		size_t maxDedicatedVideoMemory = 0;

		for (uint32_t i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i) {
			DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
			dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

			if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
				SUCCEEDED(D3D12CreateDevice(
					dxgiAdapter1.Get(),
					D3D_FEATURE_LEVEL_11_0,
					__uuidof(ID3D12Device),
					nullptr)) &&
				dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
			{
				maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
				ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
			}
		}
	}

	return dxgiAdapter4;
}

void Cyrex::Application::CreateCommandQueue() {
	m_directCommandQueue  = std::make_shared<CommandQueue>(D3D12_COMMAND_LIST_TYPE_DIRECT);
	m_computeCommandQueue = std::make_shared<CommandQueue>(D3D12_COMMAND_LIST_TYPE_COMPUTE);
	m_copyCommandQueue    = std::make_shared<CommandQueue>(D3D12_COMMAND_LIST_TYPE_COPY);
}

std::shared_ptr<CommandQueue> Cyrex::Application::GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) const noexcept {
	std::shared_ptr<CommandQueue> commandQueue;

	switch (type) {
	case D3D12_COMMAND_LIST_TYPE_DIRECT:
		commandQueue = m_directCommandQueue;
		break;
	case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		commandQueue = m_computeCommandQueue;
		break;
	case D3D12_COMMAND_LIST_TYPE_COPY:
		commandQueue = m_copyCommandQueue;
		break;
	default:
		assert(false && "Invalid command queue type.");
	}

	return commandQueue;
}

bool Cyrex::Application::CheckTearingSupport() {
	bool allowTearing = false;

	wrl::ComPtr<IDXGIFactory4> factory4;

	if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4)))) {
		wrl::ComPtr<IDXGIFactory5> factory5;

		if (SUCCEEDED(factory4.As(&factory5))) {
			factory5->CheckFeatureSupport(
				DXGI_FEATURE_PRESENT_ALLOW_TEARING,
				&allowTearing,
				sizeof(allowTearing));
		}
	}

	return allowTearing;
}

void Cyrex::Application::EnableDebugLayer() const {
#if defined(_DEBUG)
	wrl::ComPtr<ID3D12Debug> debugInterface;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
	debugInterface->EnableDebugLayer();
#endif
}

void Cyrex::Application::HandleInput() noexcept {
	KeyboardInput();
}

void Cyrex::Application::KeyboardInput() noexcept {
	while (const auto e = m_wnd->Kbd.ReadKey()) {
		if (!e->IsPress()) {
			continue;
		}

		switch (e->GetCode()) {
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		case VK_F11:
			crxlog::info("Toggled fullscreen mode");
			m_wnd->ToggleFullScreen(!m_wnd->FullScreen());
			break;
		}
	}

	while (const auto e = m_wnd->Kbd.ReadChar()) {
		if (e.value() == 'v') {
			crxlog::info("Toggled VSync");
			m_gfx->ToggleVsync();
		}
	}
}

std::optional<int> Cyrex::Application::MessagePump() {
	MSG msg{ 0 };

	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT)
			return msg.wParam;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return {};
}
