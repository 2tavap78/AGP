#include"Model.h"


Model::Model(ID3D11Device * Device, ID3D11DeviceContext * Context)
{
	m_pD3DDevice = Device;
	m_pImmediateContext = Context;

	m_x = 0.0f;
	m_y = 0.0f;
	m_z = 15.0f;
	m_xangle = 0.0f;
	m_zangle = 0.0f;
	m_yangle = 0.0f;
	m_scale = 1.0f;

	m_pTexture0 = NULL;
	m_pSampler0 = NULL;
}


int Model::LoadObjModel(char * filename)
{
	HRESULT hr = S_OK;

	m_pObject = new ObjFileModel(filename, m_pD3DDevice, m_pImmediateContext);
	if (m_pObject->filename == "FILE NOT LOADED") return S_FALSE;

	//create constant buffer 
	D3D11_BUFFER_DESC constant_buffer_desc;
	ZeroMemory(&constant_buffer_desc, sizeof(constant_buffer_desc));
	constant_buffer_desc.Usage = D3D11_USAGE_DEFAULT;  //can use updateSubresource() to update
	constant_buffer_desc.ByteWidth = 64;  //must be a multiple of 16, calculate from CB struct
	constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;  //use as a constant buffer

	hr = m_pD3DDevice->CreateBuffer(&constant_buffer_desc, NULL, &m_pConstantBuffer);

	if (FAILED(hr))
		return hr;


	//load and compile pixel and vertex shaders -use vs_5_0 to target DX11 hardware only
	ID3DBlob *VS, *PS, *error;

	hr = D3DX11CompileFromFile("model_shaders.hlsl", 0, 0, "ModelVS", "vs_4_0", 0, 0, 0, &VS, &error, 0);
	if (error != 0)//check for shader compilation error
	{
		OutputDebugStringA((char*)error->GetBufferPointer());
		error->Release();
		if (FAILED(hr)) //dont fail if error is just a warning
		{
			return hr;
		};
	}

	hr = D3DX11CompileFromFile("model_shaders.hlsl", 0, 0, "ModelPS", "ps_4_0", 0, 0, 0, &PS, &error, 0);
	if (error != 0)//check for shader compilation error
	{
		OutputDebugStringA((char*)error->GetBufferPointer());
		error->Release();
		if (FAILED(hr)) //dont fail if error is just a warning
		{
			return hr;
		};
	}



	//create shader objects
	hr = m_pD3DDevice->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &m_pVShader);

	if (FAILED(hr))
	{
		return hr;
	}


	hr = m_pD3DDevice->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_pPShader);

	if (FAILED(hr))
	{
		return hr;
	}



	//create and set the input layout object
	D3D11_INPUT_ELEMENT_DESC iedesc[] =
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
	{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0 },
	{ "NORMAL", 0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 }
	};

	hr = m_pD3DDevice->CreateInputLayout(iedesc, ARRAYSIZE(iedesc), VS->GetBufferPointer(), VS->GetBufferSize(), &m_pInputLayout);

	if (FAILED(hr))
	{
		return hr;
	}

	return 0;
}
void Model::Draw(XMMATRIX* identity, XMMATRIX* view, XMMATRIX* projection)
{





	MODEL_CONSTANT_BUFFER model_cb_values;
	model_cb_values.WVPMatrix = (*identity)*(*view)*(*projection);

	XMVECTOR vc;
	vc = XMVectorSet(0.5f, 0.5f, 0.5f, 0.5f);
	model_cb_values.ambient_light_colour = vc;

	//upload the new values for the constant buffer
	m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, 0, &model_cb_values, 0, 0);
	m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	//set the shader objects as active
	m_pImmediateContext->VSSetShader(m_pVShader, 0, 0);
	m_pImmediateContext->PSSetShader(m_pPShader, 0, 0);
	m_pImmediateContext->PSSetSamplers(0, 0, &m_pSampler0);
	m_pImmediateContext->PSSetShaderResources(0, 0, &m_pTexture0);
	m_pImmediateContext->IASetInputLayout(m_pInputLayout);

	m_pObject->Draw();
}

void Model::LookAt_XZ(float x, float z)
{
	//calculate dx and dz between the object and the look at position passed in.
	float DX = x - m_x;
	float DZ = z - m_z;

	//update m_yangle using the arctangent calculation and converting to degrees.
	m_yangle = atan2(DX, DZ) * (180.0 / XM_PI);

}

void Model::MoveForward(float distance)
{

	//update the model position by using this distance to update the model m_x and m_z positions based on m_yangle
	m_x += sin(m_yangle * (XM_PI / 180.0)) * distance;
	m_z += cos(m_yangle * (XM_PI / 180.0)) * distance;

}

void Model::CalculateModelCentrePoint()
{


	float MaxX = 0;

	float MaxY = 0;

	float MaxZ = 0;

	float MinX = 0;

	float MinY = 0;

	float MinZ = 0;



	for (int i = 0; i < m_pObject->numverts; i++)

	{

		//see if x is bigger than MaxX -> set MaxX

		if (m_pObject->vertices[i].Pos.x > MaxX)

			MaxX = m_pObject->vertices[i].Pos.x;

		//see if x is smaller than MinX -> set MinX

		if (m_pObject->vertices[i].Pos.x < MinX)

			MinX = m_pObject->vertices[i].Pos.x;



		//see if y is bigger than MaxY -> set MaxY

		if (m_pObject->vertices[i].Pos.x > MaxY)

			MaxY = m_pObject->vertices[i].Pos.x;

		//see if y is smaller than MinY -> set MinY

		if (m_pObject->vertices[i].Pos.x < MinY)

			MinY = m_pObject->vertices[i].Pos.x;



		//see if z is bigger than MaxZ -> set MaxZ

		if (m_pObject->vertices[i].Pos.x > MaxZ)

			MaxZ = m_pObject->vertices[i].Pos.x;

		//see if z is smaller than MinZ -> set MinZ

		if (m_pObject->vertices[i].Pos.x < MinZ)

			MinZ = m_pObject->vertices[i].Pos.x;

	}



	//calculate sphere centre points from min and max values

	m_bounding_sphere_centre_x = MinX + ((MaxX - MinX) / 2);

	m_bounding_sphere_centre_y = MinY + ((MaxY - MinY) / 2);

	m_bounding_sphere_centre_z = MinZ + ((MaxZ - MinZ) / 2);

}

void Model::CalculateBoundingSphereRadius()
{
	float Distance = 0;



	for (int i = 0; i < m_pObject->numverts; i++)

	{

		// see if distance between sphere centre point(X) and vertex point(X) is bigger than Distance -> set Distance

		float distanceX = m_pObject->vertices[i].Pos.x - m_bounding_sphere_centre_x;

		if (distanceX > Distance) Distance = distanceX;



		// see if distance between sphere centre point(Y) and vertex point(Y) is bigger than Distance -> set Distance

		float distanceY = m_pObject->vertices[i].Pos.y - m_bounding_sphere_centre_y;

		if (distanceY > Distance) Distance = distanceY;



		// see if distance between sphere centre point(Z) and vertex point(Z) is bigger than Distance -> set Distance

		float distanceZ = m_pObject->vertices[i].Pos.z - m_bounding_sphere_centre_z;

		if (distanceZ > Distance) Distance = distanceZ;

	}



	// set bounding sphere radius with calculated maximum distance

	m_bounding_sphere_radius = Distance;

}

Model::~Model()
{
	if (m_pObject)delete m_pObject;
	if (m_pInputLayout)m_pInputLayout->Release();
	if (m_pVShader)m_pVShader->Release();
	if (m_pPShader)m_pPShader->Release();

	m_pConstantBuffer->Release();
}

XMVECTOR Model::GetBoundingSphereWorldSpacePosition()
{

	XMMATRIX world;
	world = XMMatrixScaling(m_scale, m_scale, m_scale);
	world *= XMMatrixRotationX(m_xangle);
	world *= XMMatrixRotationY(m_yangle);
	world *= XMMatrixRotationZ(m_zangle);
	world *= XMMatrixTranslation(m_x, m_y, m_z);

	XMVECTOR offset;
	offset = XMVectorSet(m_bounding_sphere_centre_x, m_bounding_sphere_centre_y, m_bounding_sphere_centre_z, 0.0);
	offset = XMVector3Transform(offset, world);

	return offset;
}

float Model::GetBoundingSphere_x()
{
	return m_bounding_sphere_centre_x;
}

float Model::GetBoundingSphere_y()
{
	return m_bounding_sphere_centre_y;
}

float Model::GetBoundingSphere_z()
{
	return m_bounding_sphere_centre_z;
}

float Model::GetBoundingSphereRadius()
{
	return m_bounding_sphere_radius * m_scale;
}

bool Model::CheckCollision(Model* model)
{
	if (model == this)
	{
		return false;
	}

	CalculateBoundingSphereRadius();
	model->CalculateBoundingSphereRadius();
	CalculateModelCentrePoint();
	model->CalculateModelCentrePoint();

	XMVECTOR current_model_position = GetBoundingSphereWorldSpacePosition();
	XMVECTOR model_position = model->GetBoundingSphereWorldSpacePosition();

	float current_model_radius = GetBoundingSphereRadius();
	float model_radius = model->GetBoundingSphereRadius();

	float current_model_x = XMVectorGetX(current_model_position);
	float current_model_y = XMVectorGetY(current_model_position);
	float current_model_z = XMVectorGetZ(current_model_position);

	float model_x = XMVectorGetX(model_position);
	float model_y = XMVectorGetY(model_position);
	float model_z = XMVectorGetZ(model_position);

	float distance_squared = pow(current_model_x - model_x, 2) + pow(current_model_y - model_y, 2) + pow(current_model_z - model_z, 2);

	if (distance_squared < pow(current_model_radius + model_radius, 2))
	{
		return true;
	}
	else
	{
		return false;
	}

}

void Model::SetTexture(ID3D11ShaderResourceView* texture)
{
	m_pTexture0 = texture;
}

void Model::SetSampler(ID3D11SamplerState* sampler)
{
	m_pSampler0 = sampler;
}




void Model::SetXPos(float x) { m_x = x; }
void Model::SetYPos(float y) { m_y = y; }
void Model::SetZPos(float z) { m_z = z; }

void Model::SetXRot(float xRotation) { m_xangle = xRotation; }
void Model::SetYRot(float yRotation) { m_yangle = yRotation; }
void Model::SetZRot(float zRotation) { m_zangle = zRotation; }

void Model::SetScale(float scale) { m_scale = scale; }

float Model::GetXPos() { return m_x; }
float Model::GetYPos() { return m_y; }
float Model::GetZPos() { return m_z; }

float Model::GetXRot() { return m_xangle; }
float Model::GetYRot() { return m_yangle; }
float Model::GetZRot() { return m_zangle; }

float Model::GetScale() { return m_scale; }