#include "RenderPass.h"

RenderPass::RenderPass()
{
}

RenderPass::~RenderPass()
{
}

GeometryPass::GeometryPass()
{
	this->depthTexture = nullptr;
	this->depthStencilView = nullptr;
	this->inputLayout = nullptr;
	this->pShader = nullptr;
	this->sampler = nullptr;
	this->vShader = nullptr;
	for (int i = 0; i < BUFFER_COUNT; i++)
	{
		this->gBuffer.renderTargets[i] = nullptr;
		this->gBuffer.shaderResourceViews[i] = nullptr;
		this->gBuffer.textures[i];
	}
}

GeometryPass::~GeometryPass()
{
	
}

void GeometryPass::ClearScreen()
{
	for (int i = 0; i < BUFFER_COUNT; i++)
	{
		float color[4] = {};

		// Red
		color[0] = 0.0f;

		// Green
		color[1] = 0.0f;

		// Blue
		color[2] = 0.0f;

		// Alpha
		color[3] = 0.0f;

		Graphics::GetContext()->ClearRenderTargetView(gBuffer.renderTargets[i], color);
	}
	Graphics::GetContext()->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1, 0);
}

void GeometryPass::BindSSAO()
{
	Graphics::GetContext()->PSSetShaderResources(0, 1, &this->gBuffer.shaderResourceViews[3]);
}

bool GeometryPass::CreateGBuffer()
{
	HRESULT hr;

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetDesc = {};

	renderTargetDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	renderTargetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetDesc.Texture2D.MipSlice = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceDesc = {};

	shaderResourceDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	shaderResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceDesc.Texture2D.MipLevels = 1;
	shaderResourceDesc.Texture2D.MostDetailedMip = 0;


	for (int i = 0; i < BUFFER_COUNT; i++)
	{
		std::string key = std::to_string(i);
		gBuffer.textures[i] = ResourceManager::GetTexture(key, true);

		hr = Graphics::GetDevice()->CreateRenderTargetView(gBuffer.textures[i], &renderTargetDesc, &gBuffer.renderTargets[i]);

		if (FAILED(hr))
		{
			return false;
		}

		hr = Graphics::GetDevice()->CreateShaderResourceView(gBuffer.textures[i], &shaderResourceDesc, &gBuffer.shaderResourceViews[i]);

		if (FAILED(hr))
		{
			return false;
		}
	}

	return true;
}

bool GeometryPass::CreateDepthBuffer()
{
	HRESULT hr;

	D3D11_TEXTURE2D_DESC depthDesc = {};

	depthDesc.Width = Graphics::GetWindowWidth();
	depthDesc.Height = Graphics::GetWindowHeight();
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};

	depthStencilDesc.Format = depthDesc.Format;
	depthStencilDesc.Texture2D.MipSlice = 0;
	depthStencilDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	hr = Graphics::GetDevice()->CreateTexture2D(&depthDesc, nullptr, &depthTexture);

	if (FAILED(hr))
	{
		return false;
	}

	hr = Graphics::GetDevice()->CreateDepthStencilView(depthTexture, &depthStencilDesc, &depthStencilView);

	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool GeometryPass::CreateInputLayout()
{
	HRESULT hr;

	D3D11_INPUT_ELEMENT_DESC geometryLayout[5] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"SPECULAR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	hr = Graphics::GetDevice()->CreateInputLayout(geometryLayout, 5, this->vShaderByteCode.c_str(), this->vShaderByteCode.length(), &inputLayout);

	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool GeometryPass::LoadShaders()
{
	HRESULT hr;

	std::string shaderData;
	std::ifstream reader;

	reader.open("VertexShaderGPass.cso", std::ios::binary | std::ios::ate);
	if (!reader.is_open())
	{
		return false;
	}
	reader.seekg(0, std::ios::end);
	shaderData.reserve(static_cast<unsigned int>(reader.tellg()));
	reader.seekg(0, std::ios::beg);

	shaderData.assign((std::istreambuf_iterator<char>(reader)),
		std::istreambuf_iterator<char>());

	hr = Graphics::GetDevice()->CreateVertexShader(shaderData.c_str(), shaderData.length(), nullptr, &vShader);

	if (FAILED(hr))
	{
		return false;
	}

	this->vShaderByteCode = shaderData;
	shaderData.clear();
	reader.close();

	// PixelShaderDeferred
	reader.open("PixelShaderGPass.cso", std::ios::binary | std::ios::ate);
	if (!reader.is_open())
	{
		return false;
	}
	reader.seekg(0, std::ios::end);
	shaderData.reserve(static_cast<unsigned int>(reader.tellg()));
	reader.seekg(0, std::ios::beg);

	shaderData.assign((std::istreambuf_iterator<char>(reader)),
		std::istreambuf_iterator<char>());

	hr = Graphics::GetDevice()->CreatePixelShader(shaderData.c_str(), shaderData.length(), nullptr, &pShader);

	if (FAILED(hr))
	{
		return false;
	}

	shaderData.clear();
	reader.close();

	return true;
}

bool GeometryPass::CreateSamplerState()
{
	HRESULT hr;

	D3D11_SAMPLER_DESC samplerDesc = {};

	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 4;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = 0;

	// Create the texture sampler state.
	hr = Graphics::GetDevice()->CreateSamplerState(&samplerDesc, &this->sampler);

	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool GeometryPass::Initialize()
{
	if (!LoadShaders())
	{
		return false;
	}

	if (!CreateGBuffer())
	{
		return false;
	}

	if (!CreateInputLayout())
	{
		return false;
	}

	if (!CreateDepthBuffer())
	{
		return false;
	}

	if (!CreateSamplerState())
	{
		return false;
	}

	return true;
}

void GeometryPass::Clear()
{
	ID3D11RenderTargetView* nullRenderTargets[BUFFER_COUNT] = { nullptr };
	ID3D11Buffer* vBufferNull = nullptr;
	ID3D11VertexShader* vShaderNull = nullptr;
	ID3D11PixelShader* pShaderNull = nullptr;
	ID3D11InputLayout* inputLayoutNull = nullptr;
	ID3D11SamplerState* samplerStateNull = nullptr;
	ID3D11DepthStencilView* dsViewNull = nullptr;

	UINT stride = 0;
	UINT offset = 0;

	Graphics::GetContext()->IASetVertexBuffers(0, 1, &vBufferNull, &stride, &offset);
	Graphics::GetContext()->VSSetShader(vShaderNull, NULL, 0);
	Graphics::GetContext()->PSSetShader(pShaderNull, NULL, 0);
	Graphics::GetContext()->IASetInputLayout(inputLayoutNull);
	Graphics::GetContext()->OMSetRenderTargets(BUFFER_COUNT, nullRenderTargets, dsViewNull);
	Graphics::GetContext()->PSSetSamplers(0, 1, &samplerStateNull);
}

void GeometryPass::Prepare()
{
	Graphics::GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Graphics::GetContext()->VSSetShader(vShader, NULL, 0);
	Graphics::GetContext()->PSSetShader(pShader, NULL, 0);
	Graphics::GetContext()->IASetInputLayout(inputLayout);
	Graphics::GetContext()->OMSetRenderTargets(BUFFER_COUNT, gBuffer.renderTargets, depthStencilView);
	
	Graphics::GetContext()->PSSetSamplers(0, 1, &sampler);
}

void GeometryPass::Destroy()
{
	if (this->depthTexture)
		this->depthTexture->Release();
	if (this->depthStencilView)
		this->depthStencilView->Release();
	if (this->inputLayout)
		this->inputLayout->Release();
	if (this->pShader)
		this->pShader->Release();
	if (this->vShader)
		this->vShader->Release();
	if (this->sampler)
		this->sampler->Release();
	for (int i = 0; i < BUFFER_COUNT; i++)
	{
		if (this->gBuffer.renderTargets[i])
			this->gBuffer.renderTargets[i]->Release();
		if (this->gBuffer.shaderResourceViews[i])
			this->gBuffer.shaderResourceViews[i]->Release();
	}
}

ID3D11DepthStencilView*& GeometryPass::GetDepthStencilView()
{
	return this->depthStencilView;
}

bool LightPass::LoadShaders()
{
	HRESULT hr;

	std::string shaderData;
	std::ifstream reader;

	reader.open("VertexShaderLightPass.cso", std::ios::binary | std::ios::ate);
	if (!reader.is_open())
	{
		return false;
	}
	reader.seekg(0, std::ios::end);
	shaderData.reserve(static_cast<unsigned int>(reader.tellg()));
	reader.seekg(0, std::ios::beg);

	shaderData.assign((std::istreambuf_iterator<char>(reader)),
		std::istreambuf_iterator<char>());

	hr = Graphics::GetDevice()->CreateVertexShader(shaderData.c_str(), shaderData.length(), nullptr, &vShader);

	if (FAILED(hr))
	{
		return false;
	}

	this->vShaderByteCode = shaderData;
	shaderData.clear();
	reader.close();

	// PixelShaderDeferred
	reader.open("PixelShaderLightPass.cso", std::ios::binary | std::ios::ate);
	if (!reader.is_open())
	{
		return false;
	}
	reader.seekg(0, std::ios::end);
	shaderData.reserve(static_cast<unsigned int>(reader.tellg()));
	reader.seekg(0, std::ios::beg);

	shaderData.assign((std::istreambuf_iterator<char>(reader)),
		std::istreambuf_iterator<char>());

	hr = Graphics::GetDevice()->CreatePixelShader(shaderData.c_str(), shaderData.length(), nullptr, &pShader);
	if (FAILED(hr))
	{
		return false;
	}

	shaderData.clear();
	reader.close();

	return true;
}

bool LightPass::CreateShaderResources()
{
	HRESULT hr;

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceDesc = {};

	shaderResourceDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	shaderResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceDesc.Texture2D.MipLevels = 1;
	shaderResourceDesc.Texture2D.MostDetailedMip = 0;

	std::string key;

	for (int i = 0; i < BUFFER_COUNT; i++)
	{
		key = std::to_string(i);
		hr = Graphics::GetDevice()->CreateShaderResourceView(ResourceManager::GetTexture(key), &shaderResourceDesc, &shaderResources[i]);

		if (FAILED(hr))
		{
			return false;
		}
	}

	return true;
}

bool LightPass::CreateFullScreenQuad()
{
	HRESULT hr;

	Vertex fullScreenQuad[] =
	{
		{ { -1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } }, // BOTTOM LEFT
		{ { -1.0, 1.0f, 0.0f }, { 0.0f, 0.0f } },   // TOP LEFT
		{ { 1.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },   // TOP RIGHT
		{ { 1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } }    // BOTTOM RIGHT
	};

	DWORD indices[] =
	{
		0, 1, 2,
		0, 2, 3
	};

	D3D11_BUFFER_DESC quadDesc = {};

	quadDesc.ByteWidth = sizeof(fullScreenQuad);
	quadDesc.Usage = D3D11_USAGE_IMMUTABLE;
	quadDesc.CPUAccessFlags = 0;
	quadDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA data = {};

	data.pSysMem = fullScreenQuad;

	hr = Graphics::GetDevice()->CreateBuffer(&quadDesc, &data, &vBuffer);

	if (FAILED(hr))
	{
		return false;
	}

	quadDesc.ByteWidth = sizeof(indices);
	quadDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	data.pSysMem = indices;

	hr = Graphics::GetDevice()->CreateBuffer(&quadDesc, &data, &iBuffer);

	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool LightPass::CreateInputLayout()
{
	HRESULT hr;

	D3D11_INPUT_ELEMENT_DESC layout[2] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	hr = Graphics::GetDevice()->CreateInputLayout(layout, 2, vShaderByteCode.c_str(), vShaderByteCode.length(), &inputLayout);

	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool LightPass::CreateSamplerState()
{
	HRESULT hr;

	D3D11_SAMPLER_DESC samplerDesc = {};

	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 4;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = 0;

	// Create the texture sampler state.
	hr = Graphics::GetDevice()->CreateSamplerState(&samplerDesc, &this->sampler);

	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool LightPass::CreateDepthStencilState()
{
	HRESULT hr;
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};

	dsDesc.DepthEnable = false;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	dsDesc.StencilEnable = false;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	hr = Graphics::GetDevice()->CreateDepthStencilState(&dsDesc, &this->noDepth);

	return !FAILED(hr);
}

bool LightPass::CreateSSAOTurnOffBuffer()
{
	HRESULT hr;

	D3D11_BUFFER_DESC desc;
	desc.ByteWidth = sizeof(Vector4);
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;

	hr = Graphics::GetDevice()->CreateBuffer(&desc, NULL, &this->turnOffSSAO);

	return SUCCEEDED(hr);
}

LightPass::LightPass()
{
	this->iBuffer = nullptr;
	this->inputLayout = nullptr;
	this->renderedImage = nullptr;
	this->renderTarget = nullptr;
	this->turnOffSSAO = nullptr;
	this->vBuffer = nullptr;
	this->vShader = nullptr;
	this->pShader = nullptr;
	this->sampler = nullptr;
	this->noDepth = nullptr;
	for (int i = 0; i < BUFFER_COUNT; i++)
	{
		this->shaderResources[i] = nullptr;
	}
}

LightPass::~LightPass()
{
	
}

void LightPass::ToggleSSAO(bool toggle)
{
	Vector4 package = { 1.0f, 1.0f, 1.0f, 1.0f };

	if (!toggle)
		package = { 0.0f, 0.0f, 0.0f, 0.0f };

	D3D11_MAPPED_SUBRESOURCE submap;
	HRESULT hr = Graphics::GetContext()->Map(this->turnOffSSAO, 0, D3D11_MAP_WRITE_DISCARD, 0, &submap);
	memcpy(submap.pData, &package, sizeof(Vector4));

	Graphics::GetContext()->Unmap(this->turnOffSSAO, 0);
}

bool LightPass::Initialize()
{
	if (!LoadShaders())
	{
		return false;
	}

	if (!CreateShaderResources())
	{
		return false;
	}

	if (!CreateFullScreenQuad())
	{
		return false;
	}

	if (!CreateInputLayout())
	{
		return false;
	}

	if (!CreateSamplerState())
	{
		return false;
	}

	if (!CreateDepthStencilState())
	{
		return false;
	}

	if (!CreateSSAOTurnOffBuffer())
		return false;


	return true;
}

void LightPass::Clear()
{
	ID3D11VertexShader* vShaderNull = nullptr;
	ID3D11PixelShader* pShaderNull = nullptr;
	ID3D11InputLayout* inputLayoutNull = nullptr;
	ID3D11Buffer* vBufferNull = nullptr;
	ID3D11Buffer* iBufferNull = nullptr;
	ID3D11SamplerState* samplerStateNull = nullptr;
	ID3D11DepthStencilState* nullState = nullptr;
	ID3D11ShaderResourceView* shaderResourceNull[BUFFER_COUNT] = { nullptr };
	ID3D11ShaderResourceView* singleSRVNull = nullptr;
	UINT stride = 0;
	UINT offset = 0;

	Graphics::GetContext()->VSSetShader(vShaderNull, nullptr, 0);
	Graphics::GetContext()->PSSetShader(pShaderNull, nullptr, 0);
	Graphics::GetContext()->IASetInputLayout(inputLayoutNull);
	Graphics::GetContext()->IASetVertexBuffers(0, 1, &vBufferNull, &stride, &offset);
	Graphics::GetContext()->IASetIndexBuffer(iBufferNull, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN, 0);
	Graphics::GetContext()->PSSetSamplers(0, 1, &samplerStateNull);
	
	//LATER FIX***
	Graphics::GetContext()->PSSetShaderResources(0, BUFFER_COUNT - 1, shaderResourceNull);
	Graphics::GetContext()->PSSetShaderResources(6, 1, shaderResourceNull);
	
	Graphics::GetContext()->OMSetDepthStencilState(nullState, 0);
	Graphics::GetContext()->PSSetShaderResources(6, 1, &singleSRVNull);

	Graphics::GetContext()->OMSetDepthStencilState(nullState, 0);
	Graphics::GetContext()->PSSetShaderResources(7, 1, &singleSRVNull);
}

void LightPass::Prepare()
{
	Graphics::GetContext()->VSSetShader(vShader, NULL, 0);
	Graphics::GetContext()->PSSetShader(pShader, NULL, 0);
	Graphics::GetContext()->IASetInputLayout(inputLayout);
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	Graphics::GetContext()->IASetVertexBuffers(0, 1, &vBuffer, &stride, &offset);
	Graphics::GetContext()->IASetIndexBuffer(iBuffer, DXGI_FORMAT_R32_UINT, offset);
	Graphics::GetContext()->PSSetSamplers(0, 1, &sampler);
	
	Graphics::GetContext()->PSSetShaderResources(0, BUFFER_COUNT - 2, this->shaderResources);
	Graphics::GetContext()->PSSetShaderResources(7, 1, &this->shaderResources[4]);

	Graphics::GetContext()->OMSetDepthStencilState(this->noDepth, 1);
	Graphics::GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	Graphics::GetContext()->PSSetConstantBuffers(5, 1, &this->turnOffSSAO);

	Graphics::GetContext()->DrawIndexed(6, 0, 0);
}

void LightPass::Destroy()
{
	if (this->iBuffer)
		this->iBuffer->Release();
	if (this->inputLayout)
		this->inputLayout->Release();
	if (this->renderedImage)
		this->renderedImage->Release();
	if (this->renderTarget)
		this->renderTarget->Release();
	if (this->vBuffer)
		this->vBuffer->Release();
	if (this->vShader)
		this->vShader->Release();
	if (this->pShader)
		this->pShader->Release();
	if (this->sampler)
		this->sampler->Release();
	if (this->noDepth)
		this->noDepth->Release();
	if (this->turnOffSSAO)
		this->turnOffSSAO->Release();

	for (int i = 0; i < BUFFER_COUNT; i++)
	{
		if (this->shaderResources[i])
			this->shaderResources[i]->Release();
	}
}
