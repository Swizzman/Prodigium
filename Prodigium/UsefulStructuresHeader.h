#pragma once
#include "Texture.h"
#include <SimpleMath.h>

constexpr int BUFFER_COUNT = 3;
/*
	Struct for usage with Vertices.
	Try to have it as general as possible for possible reusage.
*/
struct Vertex
{
	DirectX::SimpleMath::Vector3 position = {};
	DirectX::SimpleMath::Vector2 uv = {};
	DirectX::SimpleMath::Vector3 normal = {};
};

/*
	Struct for usage with Materialized objects.
	Contains the mostly used ambient, diffuse and specular.
*/
struct Material
{
	DirectX::SimpleMath::Vector4 ambient;
	DirectX::SimpleMath::Vector4 diffuse;
	DirectX::SimpleMath::Vector4 specular;
};

/*
	Struct for usage with different Matrices
	Contains the matrices for view and projection.
*/
struct ViewProjMatrix
{
	DirectX::SimpleMath::Matrix view;
	DirectX::SimpleMath::Matrix projection;
};

/*
	Compact version of the 'Matrix' struct.
	Contains one element that can be used as world, view and projection in one matrix.
*/
struct MatrixCompact
{
	DirectX::SimpleMath::Matrix transform;
};

struct TextureRenderTargets
{
	ID3D11RenderTargetView* renderTargets[BUFFER_COUNT];
	ID3D11ShaderResourceView* shaderResourceViews[BUFFER_COUNT];
	Texture* textures[BUFFER_COUNT];
};

/*
* Use this structure when making a new light. It containts three 
* XMFLOAT4 used for position, direction and attenuation.
* --LightTypes--
* 0 = Directional Light.
* 1 = Point Light.
* 2 = Spot Light.
*/
struct LightStruct
{
	DirectX::SimpleMath::Vector4 position; // position.w is range.
	DirectX::SimpleMath::Vector4 direction; // direction.w is Spot.
	DirectX::SimpleMath::Vector4 attentuate; // attentuate.w is LightType.
};

/*
	Structure for vertices that will be used in an animation
	bonesID is 3 integers which represents 3 bones that can affect the vertex
	weights is 3 floats which is how much each bone will affect the vertex
*/
struct AnimationVertex
{
	DirectX::SimpleMath::Vector3 position = {};
	DirectX::SimpleMath::Vector2 uv = {};
	DirectX::SimpleMath::Vector3 normal = {};
	DirectX::SimpleMath::Vector3 boneIDs = {};
	DirectX::SimpleMath::Vector3 boneWeights = {};
};