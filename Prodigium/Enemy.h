#pragma once
#include "MeshObject.h"
#include "Player.h"
#include <omp.h>
class Enemy
{
private:
	MeshObject* model;
	float defaultSpeedFactor;
	float speedFactor;
	float attackRange;
	float speedDegradeCooldown;
	DirectX::SimpleMath::Vector3 targetPos;
	const DirectX::SimpleMath::Vector3 defaultForward = { 0.f,0.f,1.f };
	double lastAttack;
	DirectX::SimpleMath::Vector3 targetDir;
	bool reachedTarget;
	float angle;

public:
	Enemy();
	virtual ~Enemy();

	float speed;
	// Sets forward vector to point to the new targets position
	void SetNewTarget(const DirectX::SimpleMath::Vector3& newPos);
	// Moves the enemy towards the forward vector
	void Move();
	const float& GetAttackRange() const;
	void PlayAttackAnimation();
	const bool CanAttack() const;
	void Attack(Player* player);
	const DirectX::SimpleMath::Vector3& GetPosition() const;
	const bool IsCloseToPlayer(const DirectX::SimpleMath::Vector3& playerPos);
	MeshObject* GetMeshObject() const;
	const bool& HasReachedTarget() const;
	void SetSpeedFactor(float factor);
	void StopMovement();
	void ResetSpeed();
	const float& GetSpeedFactor() const;
	// Rotates the enemy towards the forward vector
	void RotateTo();
	// Updates the enemy
	void Update();
};

