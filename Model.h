#pragma once
#include"ObjFileModel.h"


struct MODEL_CONSTANT_BUFFER
{

	XMMATRIX WVPMatrix; //64 bytes
						//float red_fraction;  //4bytes	
						//float scale;//4bytes
						//	float2 packing; // 8 bytes

	XMVECTOR directional_light_vector; //16bytes
	XMVECTOR directional_light_colour; //16bytes
	XMVECTOR ambient_light_colour; //16bytes

								   // total 112 bytes
};

class  Model {
public:
	Model(ID3D11Device* Device, ID3D11DeviceContext* Context);
	int LoadObjModel(char* filename);
	void Draw(XMMATRIX* world, XMMATRIX* view, XMMATRIX* projection);
	~Model();

	void SetXPos(float x);
	void SetYPos(float y);
	void SetZPos(float z);

	void SetXRot(float xRotation);
	void SetYRot(float yRotation);
	void SetZRot(float zRotation);

	void SetScale(float scale);


	float GetXPos();
	float GetYPos();
	float GetZPos();

	float GetXRot();
	float GetYRot();
	float GetZRot();

	float GetScale();

	void LookAt_XZ(float x, float z);
	void MoveForward(float distance);
	XMVECTOR GetBoundingSphereWorldSpacePosition();
	float GetBoundingSphereRadius();
	float GetBoundingSphere_x();
	float GetBoundingSphere_y();
	float GetBoundingSphere_z();
	bool CheckCollision(Model* model);

	void SetTexture(ID3D11ShaderResourceView* texture);
	void SetSampler(ID3D11SamplerState* sampler);

	void CalculateModelCentrePoint();
	void CalculateBoundingSphereRadius();

private:
	ID3D11Device * m_pD3DDevice;
	ID3D11DeviceContext* m_pImmediateContext;

	ObjFileModel* m_pObject;
	ID3D11VertexShader* m_pVShader;
	ID3D11PixelShader* m_pPShader;
	ID3D11InputLayout* m_pInputLayout;
	ID3D11Buffer* m_pConstantBuffer;

	ID3D11ShaderResourceView*	m_pTexture0;
	ID3D11SamplerState*	m_pSampler0;



	float m_x, m_y, m_z;
	float m_xangle, m_zangle, m_yangle;
	float m_scale;

	float m_bounding_sphere_centre_x;
	float m_bounding_sphere_centre_y;
	float m_bounding_sphere_centre_z;
	float m_bounding_sphere_radius;





};
