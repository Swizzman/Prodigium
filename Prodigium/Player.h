#pragma once
#include "GameObject.h"
#include "CameraObject.h"
#include "Graphics.h"

class Player:public GameObject
{
private:
	float speed;
	CameraObject playerCam;

public:
	Player();
	virtual ~Player();

	void Update(const float& deltaTime);
	void Move(float x, float z);
	void Rotate(const float& pitch, const float& yaw);
};