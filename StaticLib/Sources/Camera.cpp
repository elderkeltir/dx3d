#include "stdafx.h"
#include "Camera.h"

#include <algorithm>

#include <d3d11.h>

#include "GameTime.h"

using namespace Library;

Camera::Camera(float fov, int width, int height, float nearPlane, float farPlane) :
	m_fov(fov),
	m_width(width),
	m_height(height),
	m_near(nearPlane),
	m_far(farPlane),
	m_viewDirty(true),
	m_viewport(new D3D11_VIEWPORT),
	m_position(DirectX::XMFLOAT3(20.0f, 30.0f, -40.0f)),
	m_look(DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f)),
	m_up(DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f)),
	m_right(DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f))
{
	CalculateVeiw();
	UpdateProjection();
	UpdateViewport();
	m_viewDirty = false;
}

Camera::~Camera()
{
}

const DirectX::XMMATRIX Camera::GetView()
{
	return DirectX::XMLoadFloat4x4(&m_view);
}

const DirectX::XMMATRIX Camera::GetProjection() const
{
	return DirectX::XMLoadFloat4x4(&m_projection);
}

void Camera::UpdateProjection()
{
	float aspectRatio = (float)m_width / m_height;

	DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(m_fov, aspectRatio, m_near, m_far);
	DirectX::XMStoreFloat4x4(&m_projection, P);
}

const D3D11_VIEWPORT* Library::Camera::GetViewport() const
{
	return m_viewport.get();
}

void Library::Camera::UpdateViewport()
{
	assert(g_D3D->deviceCtx);

	m_viewport->Width = (float)m_width;
	m_viewport->Height = (float)m_height;
	m_viewport->TopLeftX = 0.f;
	m_viewport->TopLeftY = 0.f;
	m_viewport->MinDepth = 0.0f;
	m_viewport->MaxDepth = 1.f;

	g_D3D->deviceCtx->RSSetViewports(1, m_viewport.get());
}

float Library::Camera::GetFov() const
{
	return m_fov;
}

float Library::Camera::GetNear() const
{
	return m_near;
}

float Library::Camera::GetFar() const
{
	return m_far;
}

DirectX::XMVECTOR Library::Camera::GetPosition() const
{
	return DirectX::XMLoadFloat3(&m_position);
}

void Library::Camera::Pitch(float angle)
{
	// Rotate up and look vector about the right vector.
	DirectX::XMVECTOR R = DirectX::XMLoadFloat3(&m_right);
	DirectX::XMVECTOR U = DirectX::XMLoadFloat3(&m_up);
	DirectX::XMVECTOR L = DirectX::XMLoadFloat3(&m_look);

	DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationAxis(R, angle);
	DirectX::XMStoreFloat3(&m_up, DirectX::XMVector3TransformNormal(U, rotation));
	DirectX::XMStoreFloat3(&m_look, DirectX::XMVector3TransformNormal(L, rotation));

	m_viewDirty = true;
}

void Library::Camera::RotateY(float angle)
{
	// Rotate the basis vectors about the world y-axis.
	DirectX::XMVECTOR R = DirectX::XMLoadFloat3(&m_right);
	DirectX::XMVECTOR L = DirectX::XMLoadFloat3(&m_look);

	DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationY(angle);

	DirectX::XMStoreFloat3(&m_right, DirectX::XMVector3TransformNormal(R, rotation));
	DirectX::XMStoreFloat3(&m_look, DirectX::XMVector3TransformNormal(L, rotation));

	m_viewDirty = true;
}

void Library::Camera::UpdateViewMatrix()
{
	if (m_viewDirty)
	{
		CalculateVeiw();
		m_viewDirty = false;
	}
}


void Camera::Strafe(float d)
{
	// mPosition += d*mRight
	d *= 2;
	DirectX::XMVECTOR s = DirectX::XMVectorReplicate(d);
	DirectX::XMVECTOR r = DirectX::XMLoadFloat3(&m_right);
	DirectX::XMVECTOR p = DirectX::XMLoadFloat3(&m_position);
	DirectX::XMStoreFloat3(&m_position, DirectX::XMVectorMultiplyAdd(s, r, p));

	m_viewDirty = true;
}

void Camera::Walk(float d)
{
	d *= 5;
	// mPosition += d*mLook
	DirectX::XMVECTOR s = DirectX::XMVectorReplicate(d);
	DirectX::XMVECTOR l = DirectX::XMLoadFloat3(&m_look);
	DirectX::XMVECTOR p = DirectX::XMLoadFloat3(&m_position);
	DirectX::XMStoreFloat3(&m_position, DirectX::XMVectorMultiplyAdd(s, l, p));

	m_viewDirty = true;
}

void Library::Camera::CalculateVeiw()
{
	DirectX::XMVECTOR R = DirectX::XMLoadFloat3(&m_right);
	DirectX::XMVECTOR U = DirectX::XMLoadFloat3(&m_up);
	DirectX::XMVECTOR L = DirectX::XMLoadFloat3(&m_look);
	DirectX::XMVECTOR P = DirectX::XMLoadFloat3(&m_position);	

	// Keep camera's axes orthogonal to each other and of unit length.
	L = DirectX::XMVector3Normalize(L);
	U = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(L, R));

	// U, L already ortho-normal, so no need to normalize cross product.
	R = DirectX::XMVector3Cross(U, L);

	DirectX::XMMATRIX V = DirectX::XMMatrixLookToLH(P, L, U);
	DirectX::XMStoreFloat4x4(&m_view, V);

	DirectX::XMStoreFloat3(&m_position, P);
	DirectX::XMStoreFloat3(&m_up, U);
	DirectX::XMStoreFloat3(&m_look, L);
	DirectX::XMStoreFloat3(&m_right, R);
}
