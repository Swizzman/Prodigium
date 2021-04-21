#include "Graphics.h"

Graphics* Graphics::instance = nullptr;

Graphics::Graphics()
{
	this->device = nullptr;
	this->context = nullptr;
	this->swapChain = nullptr;
	this->zBufferOn = nullptr;
	this->zBufferOff = nullptr;
}

Graphics::~Graphics()
{
	if (this->device)
		this->device->Release();
	if (this->context)
		this->context->Release();
	if (this->swapChain)
		this->swapChain->Release();
	if (this->zBufferOn)
		this->zBufferOn->Release();
	if (this->zBufferOff)
		this->zBufferOff->Release();
}

bool Graphics::CreateDeviceAndSwapChain(const HWND& windowHandler, const UINT& windowWidth, const UINT& windowHeight)
{
	HRESULT hr;

	// Debug mode
	UINT flags = 0;
	if (_DEBUG)
		flags = D3D11_CREATE_DEVICE_DEBUG;

	// Change this feature level to change DirectX version, Most stable is DirectX11
	D3D_FEATURE_LEVEL directXfeature[] = { D3D_FEATURE_LEVEL_11_0 };
	DXGI_SWAP_CHAIN_DESC swapDesc;

	/*
		What refresh rate do we lock to?
		Numerator - 0 and Denominator - 1 means unlocked framerate.

		for locked to 60 fps Numerator should be 60.
	*/
	swapDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapDesc.BufferDesc.Width = windowWidth;
	swapDesc.BufferDesc.Height = windowHeight;

	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;

	// One front buffer and one Back
	swapDesc.BufferCount = 2;

	// Allow our backbuffer to be manipulated by any compute shader, e.g. allows simple blur effect.
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
	swapDesc.Windowed = true;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapDesc.Flags = 0;
	swapDesc.OutputWindow = windowHandler;

	hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, directXfeature, 1, D3D11_SDK_VERSION,
		&swapDesc, &Graphics::instance->swapChain, &Graphics::instance->device, nullptr, &Graphics::instance->context);

	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool Graphics::CreateZBufferStates()
{
	HRESULT hr;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};

	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	depthStencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	hr = device->CreateDepthStencilState(&depthStencilDesc, &zBufferOn);

	if (FAILED(hr))
	{
		std::cerr << "Failed to DepthStencil State!" << std::endl;
		return false;
	}

	depthStencilDesc.DepthEnable = false;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	hr = device->CreateDepthStencilState(&depthStencilDesc, &zBufferOff);

	if (FAILED(hr))
	{
		std::cerr << "Failed to DepthStencil State!" << std::endl;
		return false;
	}

	return true;
}

ID3D11Device*& Graphics::GetDevice()
{
	return Graphics::instance->device;
}

ID3D11DeviceContext* Graphics::GetContext()
{
	return Graphics::instance->context;
}

IDXGISwapChain*& Graphics::GetSwapChain()
{
	return Graphics::instance->swapChain;
}

bool Graphics::Initialize(const HWND& windowHandler, const UINT& windowWidth, const UINT& windowHeight)
{
	if (Graphics::instance == nullptr)
	{
		Graphics::instance = new Graphics;

		if (!Graphics::instance->CreateDeviceAndSwapChain(windowHandler, windowWidth, windowHeight))
		{
			return false;
		}
	}

	return true;
}

void Graphics::Destroy()
{
	if (Graphics::instance)
	{
		delete Graphics::instance;
	}
}

void Graphics::EnableZBuffer()
{
	Graphics::instance->context->OMSetDepthStencilState(Graphics::instance->zBufferOn, 1);
}

void Graphics::DisableZBuffer()
{
	Graphics::instance->context->OMSetDepthStencilState(Graphics::instance->zBufferOff, 1);
}