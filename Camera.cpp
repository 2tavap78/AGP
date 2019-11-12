#include "camera.h"
//#include <d3d11.h>
//#define _XM_NO_INTRINSICS_
//#define XM_NO_ALIGNMENT
//#include <math.h>
//#include <xnamath.h>
//int (WINAPIV * __vsnprintf_s)(char *, size_t, const char*, va_list) = _vsnprintf;

Camera::Camera(float x, float y, float z, float camera_rotation)
{
	m_x = x;
	m_y = y;
	m_z = z;
	m_camera_rotation = camera_rotation;

	m_dx = sin(m_camera_rotation *(XM_PI / 180.0));
	m_dz = cos(m_camera_rotation *(XM_PI / 180.0));
}

void Camera::Rotate(float degree_number)
{

	m_camera_rotation += degree_number;

	m_dx = sin(m_camera_rotation *(XM_PI / 180.0));
	m_dz = cos(m_camera_rotation *(XM_PI / 180.0));

}

void Camera::Rotation(float degree_number)
{

	m_camera_rotation = degree_number;

	m_dx = sin(m_camera_rotation *(XM_PI / 180.0));
	m_dz = cos(m_camera_rotation *(XM_PI / 180.0));

}

void Camera::Forward(float distance)
{
	m_x += m_dx * distance;
	m_z += m_dz * distance;

}

void Camera::Right(float distance)
{
	Rotate(90);
	m_x += m_dx * distance;
	m_z += m_dz * distance;
	Rotate(270);

}

void Camera::Up(float heigh)
{
	m_y += heigh;
}

XMMATRIX Camera::GetViewMatrix()
{
	m_position = XMVectorSet(m_x, m_y, m_z, 0.0);
	m_lookat = XMVectorSet(m_x + m_dx, m_y, m_z + m_dz, 0.0);
	m_up = XMVectorSet(0.0, 1.0, 0.0, 0.0);

	XMMATRIX view = XMMatrixLookAtLH(m_position, m_lookat, m_up);

	return view;
}

float Camera::GetX()

{

	return m_x;

}

float Camera::GetY()

{

	return m_y;

}

float Camera::GetZ()

{

	return m_z;

}

float Camera::GetRotation()
{
	return m_camera_rotation;
}

void Camera::LookAt_XZ(float x, float z)
{
	//calculate dx and dz between the object and the look at position passed in.
	float DX = x - m_x;
	float DZ = z - m_z;

	//update m_yangle using the arctangent calculation and converting to degrees.
	m_camera_rotation = atan2(DX, DZ) * (180.0 / XM_PI);

}