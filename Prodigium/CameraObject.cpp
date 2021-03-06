#include "CameraObject.h"
using namespace DirectX::SimpleMath;
void CameraObject::UpdateViewMatrix()
{
	this->viewProjMatrix.viewMatrix = XMMatrixTranspose(XMMatrixLookToLH(this->eyePos, this->camForward, this->defaultUp));
}

CameraObject::CameraObject()
{
	this->aspectRatio = 0.f;
	this->nearPlane = 0.f;
	this->farPlane = 0.f;
	this->fieldOfView = 0.f;
	this->viewProjMatrix.projectionMatrix = {};
	this->upDir = {};
	this->viewProjMatrix.viewMatrix = {};
	this->eyePos = { 0.f, 0.f, 0.f };
	this->pitch = 0.f;
	this->yaw = 0.f;
	this->roll = 0.f;
	this->eyePosGPU = {};
	this->matrixBuffer = nullptr;
	this->camPosBuffer = nullptr;
	this->frustum = new Frustum();
}

CameraObject::~CameraObject()
{
	if (this->matrixBuffer)
		this->matrixBuffer->Release();
	if (this->camPosBuffer)
		this->camPosBuffer->Release();
	if (frustum)
		delete this->frustum;
}

bool CameraObject::Initialize(const int& windowWidth, const int& windowHeight, 
	const float& nearPlane, const float& farPlane, const float& fov, const Vector3& 
	eyePosition, const Vector3& lookTo)
{
	this->defaultForward = lookTo;
	this->defaultPosition = eyePosition;
	this->eyePos = eyePosition;
	this->position = eyePos;
	this->nearPlane = nearPlane;
	this->farPlane = farPlane;
	this->fieldOfView = fov;
	this->aspectRatio = float(windowWidth) / float(windowHeight);
	this->upDir = { 0.f,1.f,0.f };
	this->camForward = lookTo;
	this->viewProjMatrixCPU.viewMatrix = DirectX::XMMatrixLookToLH(eyePos, camForward, upDir);
	this->viewProjMatrix.viewMatrix = this->viewProjMatrixCPU.viewMatrix;
	this->viewProjMatrixCPU.projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(this->fieldOfView, aspectRatio, this->nearPlane, this->farPlane);
	this->viewProjMatrix.projectionMatrix = this->viewProjMatrixCPU.projectionMatrix.Transpose();
	this->scale = { 1.f, 1.f, 1.f };
	this->yaw = DirectX::XM_PIDIV2;
	this->rotation = { pitch, yaw, roll };
	this->qRotation = Quaternion::CreateFromYawPitchRoll(this->yaw, this->pitch, this->roll);
	this->position = eyePos;
	this->UpdateMatrixCPU();
	if (!this->frustum->Initialize())
	{
		return false;
	}

	D3D11_BUFFER_DESC buffDesc = {};
	buffDesc.ByteWidth = sizeof(ViewProjectionMatrix);
	buffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	buffDesc.Usage = D3D11_USAGE_DYNAMIC;

	D3D11_SUBRESOURCE_DATA data = {};
	data.pSysMem = &this->viewProjMatrix;

	HRESULT hr = Graphics::GetDevice()->CreateBuffer(&buffDesc, &data, &matrixBuffer);

	if (FAILED(hr))
	{
		std::cout << "Failed to create camera constant buffer" << std::endl;
		return false;
	}

	buffDesc.ByteWidth = sizeof(LightPassCameraBuffer);
	buffDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	buffDesc.Usage = D3D11_USAGE_DYNAMIC;


	hr = Graphics::GetDevice()->CreateBuffer(&buffDesc, nullptr, &camPosBuffer);

	if (FAILED(hr))
	{
		std::cout << "Failed to create camera constant buffer" << std::endl;
		return false;
	}

	return true;
}

void CameraObject::Rotate(const float& pitchAmount, const float& yawAmount, const float& rollAmount)
{
	this->pitch = fmod(this->pitch + pitchAmount, FULL_CIRCLE);
	this->yaw = fmod(this->yaw + yawAmount, FULL_CIRCLE);
	this->roll = fmod(this->roll + rollAmount, FULL_CIRCLE);

	if (this->pitch > 1.0f)
	{
		this->pitch = 1.0f;
	}
	if (this->pitch < -0.32f)
	{
		this->pitch = -0.32f;
	}

	this->qRotation = Quaternion::CreateFromYawPitchRoll(this->yaw, this->pitch, this->roll);
	this->rotation = { this->pitch, this->yaw, this->roll };
}

Vector3 CameraObject::GetPos() const
{
	return this->eyePos;
}

void CameraObject::Update()
{
	this->viewProjMatrixCPU.viewMatrix = XMMatrixLookToLH(this->eyePos, this->camForward, this->defaultUp);
	this->viewProjMatrix.viewMatrix = this->viewProjMatrixCPU.viewMatrix.Transpose();
	this->UpdateMatrixCPU();

	if (this->frustum)
	{
		this->frustum->Update();
	}

	D3D11_MAPPED_SUBRESOURCE mappedResource = {};

	Graphics::GetContext()->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &viewProjMatrix, sizeof(viewProjMatrix));
	Graphics::GetContext()->Unmap(matrixBuffer, 0);

	LightPassCameraBuffer camBuffer;
	this->eyePosGPU = Vector4(this->eyePos.x, this->eyePos.y, this->eyePos.z, 0.0f);
	camBuffer.cameraPos = eyePosGPU;
	camBuffer.viewMatrix = viewProjMatrix.viewMatrix;
	camBuffer.fogColour = { 0.2f,0.2f,0.2f,0.f };
	camBuffer.fogStart = 10.f;
	camBuffer.fogRange = 200;
	Graphics::GetContext()->Map(camPosBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &camBuffer, sizeof(camBuffer));
	Graphics::GetContext()->Unmap(camPosBuffer, 0);

	Graphics::GetContext()->VSSetConstantBuffers(0, 1, &matrixBuffer);
	Graphics::GetContext()->PSSetConstantBuffers(1, 1, &camPosBuffer);
	Graphics::GetContext()->PSSetConstantBuffers(10, 1, &matrixBuffer);

	// For particles geometry shader.
	Graphics::GetContext()->GSSetConstantBuffers(0, 1, &matrixBuffer);
	Graphics::GetContext()->GSSetConstantBuffers(1, 1, &camPosBuffer);

	// For particles compute shader.
	//Graphics::GetContext()->CSSetConstantBuffers(0, 1, &camPosBuffer);
}

void CameraObject::ChangeOffset(const Vector3& offset)
{
}

DirectX::SimpleMath::Vector3 CameraObject::GetForward()
{
	return this->camForward;
}

const DirectX::SimpleMath::Matrix& CameraObject::GetViewMatrixCPU() const
{
	return this->viewProjMatrixCPU.viewMatrix;
}

const DirectX::SimpleMath::Matrix& CameraObject::GetProjMatrixCPU() const
{
	return this->viewProjMatrixCPU.projectionMatrix;
}

Frustum* CameraObject::GetFrustum()
{
	return this->frustum;
}

void CameraObject::MoveCameraTowards(const Vector3& playerPos)
{
	Matrix transformed = Matrix::CreateFromQuaternion(this->qRotation) * Matrix::CreateTranslation(playerPos);

	this->eyePos = Vector3::Transform(this->defaultPosition, transformed);
	this->position = eyePos;
	this->camForward = (playerPos - this->position);
	this->camForward.y += 9.f;
	this->camForward.Normalize();
}