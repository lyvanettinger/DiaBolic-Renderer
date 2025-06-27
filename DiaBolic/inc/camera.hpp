#pragma once

using namespace DirectX;

struct Camera
{
	XMVECTOR position = XMVectorSet(0, 0, -10, 1);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMVECTOR front = XMVectorSet(0, 0, 1, 0);

	XMMATRIX model;
	XMMATRIX view;
	XMMATRIX projection;

	FLOAT yaw = 30.0f;
	FLOAT pitch = 45.0f;

	const FLOAT nearZ = 0.1f;
	const FLOAT farZ = 1000.0f;

	const FLOAT fov = 45.0f;
};