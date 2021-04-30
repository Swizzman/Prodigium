#pragma once
#include <string>
#include "GameObject.h"
#include "Mesh.h"
#include "Texture.h"
#include "ResourceManager.h"

constexpr int MAXNROFTEXTURES = 2;

class MeshObject : public GameObject
{
private:
	Mesh* mesh;

	//Holds all the views of the textures:
	//1. Diffuse texture
	//2. Normal map
	ID3D11ShaderResourceView* shaderResourceViews[MAXNROFTEXTURES];

	bool isPickUp;
	bool isVisible;

private:
	bool BindTextureToSRV(ID3D11Texture2D*& texture, ID3D11ShaderResourceView*& srv);

public:

	MeshObject();
	virtual ~MeshObject();

	bool Initialize(std::string meshObject, std::string diffuseTxt = "", std::string normalTxt = "",
					DirectX::SimpleMath::Vector3 pos = {0.0f,0.0f,0.0f}, DirectX::SimpleMath::Vector3 rot = { 0.0f,0.0f,0.0f }, DirectX::SimpleMath::Vector3 scl= {1.0f,1.0f,1.0f});

	void SetVisible(bool toggle = true);
	void SetPickUp(bool toggle = true);
	void Render();
};