#include "BlurFilter.h"

bool BlurFilter::CreateComputeShader()
{
	std::string shaderData;
	std::ifstream reader;

	reader.open("BlurFilterCS.cso", std::ios::binary | std::ios::ate);
	if (!reader.is_open())
	{
		return false;
	}

	shaderData.reserve((unsigned int)reader.tellg());
	reader.seekg(0, std::ios::beg);
	shaderData.assign((std::istreambuf_iterator<char>(reader)), std::istreambuf_iterator<char>());
	reader.close();

	return !FAILED(Graphics::GetDevice()->CreateComputeShader(shaderData.c_str(), shaderData.length(), nullptr, &this->computeShader));
}

bool BlurFilter::CreateUnorderedAccessView()
{
	ID3D11Texture2D* backBuffer = nullptr;
	if (FAILED(Graphics::GetSwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer))))
	{
		return false;
	}
	HRESULT hr = Graphics::GetDevice()->CreateUnorderedAccessView(backBuffer, nullptr, &this->unorderedAccessView);
	backBuffer->Release();
	return !FAILED(hr);
}

bool BlurFilter::CreateSettingsBuffer()
{
	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = sizeof(BlurSettings);
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	this->blurSettings.useVerticalBlur = true;
	this->blurSettings.blurRadius = 0;

	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = &this->blurSettings;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	return !FAILED(Graphics::GetDevice()->CreateBuffer(&desc, &data, &this->settingsBuffer));
}

void BlurFilter::SetupBlurLevels()
{
	this->blurLevels[(UINT)BlurLevel::SUPERLOW] = std::pair<UINT, float>(1, 0.5f);
	this->blurLevels[(UINT)BlurLevel::LOW] = std::pair<UINT, float>(2, 2.0f);
	this->blurLevels[(UINT)BlurLevel::MEDIUM] = std::pair<UINT, float>(3, 5.0f);
	this->blurLevels[(UINT)BlurLevel::HIGH] = std::pair<UINT, float>(3, 10.0f);
	this->blurLevels[(UINT)BlurLevel::HELLAHIGH] = std::pair<UINT, float>(4, 10.0f);
}

void BlurFilter::GenerateGaussFilter(BlurLevel level)
{
	UINT index = UINT(level);
	if (index < BLURLEVELSIZE)
	{
		UINT radius = this->blurLevels[index].first;
		float sigma = this->blurLevels[index].second;

		//Only acceptable to have radiuses in the range of min and max
		if (radius > MAXRADIUS)
			radius = MAXRADIUS;
		else if (radius < MINRADIUS)
			radius = MINRADIUS;

		this->blurSettings.blurRadius = radius;

		//If not sigma is picked we calculate it to fit with the curve
		if (sigma == 0)
			sigma = float(radius / 2.0f);

		for (int x = 0; x <= (int)radius; x++)
		{
			float weight = (float)(1.0f / sqrt(2 * PI * pow(sigma, 2)) * exp(-pow(x, 2) / (2 * pow(sigma, 2))));
			this->blurSettings.weights[x] = weight;
		}

		//Calculate the total sum
		float total = 0.0f;
		for (UINT i = 1; i <= radius; i++)
		{
			total += this->blurSettings.weights[i];
		}
		total *= 2;
		total += this->blurSettings.weights[0];

		//Avoid devide by zero
		if (total >= 0)
		{
			//Normalize each value
			for (UINT i = 0; i <= radius; i++)
			{
				this->blurSettings.weights[i] /= total;
			}
		}
	}
}

void BlurFilter::UpdateBlurSettings()
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	Graphics::GetContext()->Map(this->settingsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &this->blurSettings, sizeof(BlurSettings));
	Graphics::GetContext()->Unmap(settingsBuffer, 0);
}

void BlurFilter::SwapBlurDirection()
{
	if (this->blurSettings.useVerticalBlur)
		this->blurSettings.useVerticalBlur = false;
	else
		this->blurSettings.useVerticalBlur = true;

	UpdateBlurSettings();
}

BlurFilter::BlurFilter()
{
	this->computeShader = nullptr;
	this->unorderedAccessView = nullptr;
	this->settingsBuffer = nullptr;
	this->blurSettings = {};
	this->currentBlurLevel = BlurLevel::NOBLUR;
}

BlurFilter::~BlurFilter()
{
}

bool BlurFilter::Initialize()
{
	if (!CreateComputeShader())
	{
		std::cout << "Failed to create computeShader for blur..." << std::endl;
		return false;
	}
	if (!CreateUnorderedAccessView())
	{
		std::cout << "Failed to create unordered accessview..." << std::endl;
		return false;
	}

	SetupBlurLevels();

	if (!CreateSettingsBuffer())
	{
		std::cout << "Failed to create cbuffer for blur filter..." << std::endl;
		return false;
	}	
	return true;
}

void BlurFilter::SetBlurLevel(BlurLevel level)
{
	if (this->currentBlurLevel != level)
	{
		this->currentBlurLevel = level;
		//Does not have to calc blur if no blur and no need to update cbuffer
		if (this->currentBlurLevel != BlurLevel::NOBLUR)
		{
			GenerateGaussFilter(level);
			UpdateBlurSettings();
		}
	}
}

void BlurFilter::Render(ID3D11UnorderedAccessView* uav)
{
	//Only render blur when needed
	if (this->currentBlurLevel != BlurLevel::NOBLUR)
	{
		//The render target view shall not be changed at this time
		ID3D11RenderTargetView* nullRTV = nullptr;
		Graphics::GetContext()->OMSetRenderTargets(1, &nullRTV, nullptr);
		Graphics::GetContext()->CSSetShader(this->computeShader, nullptr, 0);
		Graphics::GetContext()->CSSetConstantBuffers(5, 1, &this->settingsBuffer);

		//Render to screen if none other has been set
		if (uav == nullptr)
			Graphics::GetContext()->CSSetUnorderedAccessViews(0, 1, &this->unorderedAccessView, nullptr);
		else 
			Graphics::GetContext()->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);

		//Render in two steps. Use vertical first, then swap and do horizontal. 
		//Like a cross, which gives better performance than a square 
		Graphics::GetContext()->Dispatch(Graphics::GetWindowWidth() / 8, Graphics::GetWindowHeight() / 8, 1);
		SwapBlurDirection();
		Graphics::GetContext()->Dispatch(Graphics::GetWindowWidth() / 8, Graphics::GetWindowHeight() / 8, 1);
		SwapBlurDirection();

		//Unbind the unordered access view
		ID3D11UnorderedAccessView* nullUAV = nullptr;
		Graphics::GetContext()->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
	}
}

void BlurFilter::Destroy()
{
	if (this->computeShader)
		this->computeShader->Release();
	if (this->unorderedAccessView)
		this->unorderedAccessView->Release();
	if (this->settingsBuffer)
		this->settingsBuffer->Release();
}

BlurLevel BlurFilter::GetBlurLevel() const
{
	return this->currentBlurLevel;
}
