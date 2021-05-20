#include "Enemy.h"
using namespace DirectX;
using namespace DirectX::SimpleMath;
Enemy::Enemy()
{
	this->model = new MeshObject();
	this->model->Initialize("LowPoly_Character.obj");
	this->attackRange = 20.f;
	this->speed = 40.f;
	this->reachedTarget = false;
}

Enemy::~Enemy()
{
}

void Enemy::SetNewTarget(const DirectX::SimpleMath::Vector3& newPos)
{
	this->targetPos = newPos;
}

void Enemy::MoveToTarget(const float& deltaTime)
{
	this->model->position = Vector3::Lerp(this->model->position, targetPos, 0.2f * deltaTime);
	if ((this->model->position - targetPos).Length() < 10.f)
	{
		reachedTarget = true;
	}
}


