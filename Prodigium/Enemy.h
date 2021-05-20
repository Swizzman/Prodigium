#pragma once
#include "MeshObject.h"
class Enemy
{
private:
	MeshObject* model;
	float speed;
	float attackRange;
	DirectX::SimpleMath::Vector3 targetPos;
	bool reachedTarget;
public:
	Enemy();
	virtual ~Enemy();
	void SetNewTarget(const DirectX::SimpleMath::Vector3& newPos);
	void Move(const DirectX::SimpleMath::Vector2& direction, const float& deltaTime);
	void MoveToTarget(const float& deltaTime);
	const bool& HasReachedTarget() const;


};

