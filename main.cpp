#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <dxerr.h>
#include <stdio.h>
#include <xnamath.h>
#include "Camera.h"
#include"Model.h"
#include "text2D.h"
#include "Scene_Node.h"
#include <dinput.h>
#include <xinput.h>
#define _XM_NO_INTRINSICS_
#define XM_NO_ALIGNMENT

int (WINAPIV * __vsnprintf_s)(char *, size_t, const char*, va_list) = _vsnprintf;


D3D_DRIVER_TYPE         m_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       m_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           m_pD3DDevice = NULL;
ID3D11DeviceContext*    m_pImmediateContext = NULL;
IDXGISwapChain*         m_pSwapChain = NULL;
ID3D11RenderTargetView* m_pBackBufferRTView = NULL;

ID3D11Buffer* m_pVertexBuffer;
ID3D11VertexShader* m_pVertexShader;
ID3D11PixelShader* m_pPixelShader;
ID3D11InputLayout* m_pInputLayout;

ID3D11Buffer*  m_pConstantBuffer0;

ID3D11DepthStencilView* m_pZBuffer;

ID3D11ShaderResourceView* m_pTexture0;   //Declared with other global variables at start of main()
ID3D11SamplerState*   m_pSampler0;      // Declared with other global varriables at start of main()

IDirectInput8* m_direct_input;
IDirectInputDevice8* m_keyboard_device;

IDirectInputDevice8*	m_pMouseDevice;
DIMOUSESTATE			m_MouseState, m_LastMouseState;

unsigned char m_keyboard_keys_state[256];

int controllerNum = 0;//just one controller for now
XINPUT_STATE controllerState;
bool controllerConnected = false;
bool isGameRunning = false;
bool g_cube_follow = false;


XMVECTOR g_directional_light_shines_from;
XMVECTOR g_directional_light_colour;
XMVECTOR g_ambient_light_colour;

Model* g_tent_model;
Model* g_sphere_model;
Model* g_cube_model;
Model* g_cube2_model;
Model* g_tree_model;
Model* g_dog_model;




Text2D* g_2DText;


Scene_Node* g_camera_node;
Scene_Node* g_root_node;
Scene_Node* g_sphere_node;
Scene_Node* g_cube_node;
Scene_Node* g_cube2_node;
Scene_Node* g_tent_node;

Scene_Node* g_tree1_node;
Scene_Node* g_tree2_node;
Scene_Node* g_tree3_node;
Scene_Node* g_tree4_node;
Scene_Node* g_tree5_node;
Scene_Node* g_tree6_node;
Scene_Node* g_tree7_node;

Scene_Node* g_dog_node;
Scene_Node* g_wall_node;
Scene_Node* g_bullet_node;

// add the header as a pointer to be able to use it and hen add the stuff in InitialiseGraphics()
Camera* g_pCamera;


float g_camera_speed = 0.01;
float rotationValueX = 0;
float rotationValueY = 0;
float rotationValueZ = 0;

float rotationValueX2 = 0;
float rotationValueY2 = 0;
float rotationValueZ2 = 0;

bool HasMouseMoved();
void UpdateMouse();

float GetMouseX();
void SetPositions();
bool HasMouseMoved()
{
	if (m_MouseState.lX != m_LastMouseState.lX
		|| m_MouseState.lY != m_LastMouseState.lY)
	{
		return true;
	}
	else
		return false;
}

float GetMouseX()
{
	return m_MouseState.lX - m_LastMouseState.lX;
}

void UpdateMouse()
{
	m_LastMouseState = m_MouseState;
}

//define vertex structure
struct POS_COL_TEX_NORM_VERTEX
{
	XMFLOAT3 Pos;
	XMFLOAT4 Col;
	XMFLOAT2 Texture0;
	XMFLOAT3 Normal;
};

//const buffer structs. pack to 16 bytes. don't let any single element cross a 16 byte boundary 
struct CONSTANT_BUFFER0
{
	XMMATRIX WorldViewProjection; //64 bytes ( 4x4 = 16 floats x 4 bytes)
								  //float RedAmount; // 4 bytes	
								  //float scale;    //4 bytes
								  //XMFLOAT2 packing_bytes;  //8 bytes
	XMVECTOR directional_light_vector; //16 bytes
	XMVECTOR directional_light_colour; //16 bytes
	XMVECTOR ambient_light_colour; //16 bytes
								   // TOTAL SIZE = 112 BYTES
};

HINSTANCE	g_hInst = NULL;
HWND		g_hWnd = NULL;

// Rename for each tutorial
char		g_TutorialName[100] = "Paulo Tavares Resit AGP AE2\0";

//	Forward declarations
HRESULT InitialiseGraphics(void);

HRESULT InitialiseWindow(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

CONSTANT_BUFFER0 cb0_values;
CONSTANT_BUFFER0 cb0_values2;


//////////////////////////////////////////////////////////////////////////////////////
// Create D3D device and swap chain
///////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseD3D()
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;

#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE, // comment out this line if you need to test D3D 11.0 functionality on hardware that doesn't support it
		D3D_DRIVER_TYPE_WARP, // comment this out also to use reference device
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = true;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		m_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, m_driverType, NULL,
			createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &m_pSwapChain,
			&m_pD3DDevice, &m_featureLevel, &m_pImmediateContext);
		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
		return hr;




	// Get pointer to back buffer texture
	ID3D11Texture2D *pBackBufferTexture;
	hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
		(LPVOID*)&pBackBufferTexture);

	if (FAILED(hr)) return hr;

	// Use the back buffer texture pointer to create the render target view
	hr = m_pD3DDevice->CreateRenderTargetView(pBackBufferTexture, NULL,
		&m_pBackBufferRTView);
	pBackBufferTexture->Release();

	if (FAILED(hr)) return hr;

	//create a z buffer texture
	D3D11_TEXTURE2D_DESC tex2dDesc;
	ZeroMemory(&tex2dDesc, sizeof(tex2dDesc));

	tex2dDesc.Width = width;
	tex2dDesc.Height = height;
	tex2dDesc.ArraySize = 1;
	tex2dDesc.MipLevels = 1;
	tex2dDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	tex2dDesc.SampleDesc.Count = sd.SampleDesc.Count;
	tex2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	tex2dDesc.Usage = D3D11_USAGE_DEFAULT;


	ID3D11Texture2D *pZBufferTexture;
	hr = m_pD3DDevice->CreateTexture2D(&tex2dDesc, NULL, &pZBufferTexture);

	if (FAILED(hr)) return hr;

	//create the Z buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));

	dsvDesc.Format = tex2dDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;


	m_pD3DDevice->CreateDepthStencilView(pZBufferTexture, &dsvDesc, &m_pZBuffer);
	pZBufferTexture->Release();



	// Set the render target view
	m_pImmediateContext->OMSetRenderTargets(1, &m_pBackBufferRTView, m_pZBuffer);

	// Set the viewport
	D3D11_VIEWPORT viewport;

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	m_pImmediateContext->RSSetViewports(1, &viewport);

	g_2DText = new Text2D("assets/font1.bmp", m_pD3DDevice, m_pImmediateContext);
	return S_OK;
}

void RenderFrame(void);

HRESULT InitialiseInput()
{
	HRESULT hr;

	ZeroMemory(m_keyboard_keys_state, sizeof(m_keyboard_keys_state));

	// Initialize direct input
	hr = DirectInput8Create(g_hInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_direct_input, NULL);
	if (FAILED(hr)) return hr;

	// Initialize keyboard device
	hr = m_direct_input->CreateDevice(GUID_SysKeyboard, &m_keyboard_device, NULL);
	if (FAILED(hr)) return hr;

	hr = m_keyboard_device->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(hr)) return hr;

	hr = m_keyboard_device->SetCooperativeLevel(g_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr)) return hr;

	//Mouse initialization
	hr = m_direct_input->CreateDevice(GUID_SysMouse, &m_pMouseDevice, NULL);
	if (FAILED(hr))
		return hr;

	//Sets Mouse data format
	hr = m_pMouseDevice->SetDataFormat(&c_dfDIMouse);
	if (FAILED(hr))
		return hr;

	//Sets the flags for the mouse
	hr = m_pMouseDevice->SetCooperativeLevel(g_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr))
		return hr;

	m_pMouseDevice->Acquire();
	if (FAILED(hr))
		return hr;

	return S_OK;
}

void ReadInputStates(void)
{
	HRESULT hr;

	// Read the keyboard state
	hr = m_keyboard_device->GetDeviceState(
		sizeof(m_keyboard_keys_state), (LPVOID)&m_keyboard_keys_state);

	if (FAILED(hr))
	{
		// reacquire keyboard access if lost
		if ((hr == DIERR_INPUTLOST) ||
			(hr == DIERR_NOTACQUIRED))
		{
			m_keyboard_device->Acquire();
		}
	}

	hr = m_pMouseDevice->GetDeviceState(sizeof(m_MouseState), (LPVOID)&m_MouseState);

	if (FAILED(hr))
	{
		if ((hr == DIERR_INPUTLOST) ||
			(hr == DIERR_NOTACQUIRED))
		{
			m_pMouseDevice->Acquire();
		}
	}

	//now the controller
	ZeroMemory(&controllerState, sizeof(XINPUT_STATE));

	// Get the state
	controllerConnected = (XInputGetState(controllerNum, &controllerState) == ERROR_SUCCESS);

}

bool IsKeyPressed(unsigned char DI_keycode)
{
	return m_keyboard_keys_state[DI_keycode] & 0x80;
}

//////////////////////////////////////////////////////////////////////////////////////
// Clean up D3D objects
//////////////////////////////////////////////////////////////////////////////////////
void ShutdownD3D()
{

	if (m_pVertexBuffer) m_pVertexBuffer->Release(); //03-01
	if (m_pInputLayout)m_pInputLayout->Release(); //03-01
	if (m_pVertexShader)m_pVertexShader->Release(); //03-01
	if (m_pPixelShader)m_pPixelShader->Release(); //03-01


	if (m_pBackBufferRTView) m_pBackBufferRTView->Release();

	if (m_pSwapChain) m_pSwapChain->Release();
	if (m_pConstantBuffer0) m_pConstantBuffer0->Release();
	if (m_pImmediateContext) m_pImmediateContext->Release();

	delete g_pCamera;
	delete g_tent_model;
	delete g_sphere_model;
	delete g_cube_model;

	if (m_pTexture0)  m_pTexture0->Release();
	if (m_pSampler0)  m_pSampler0->Release();

	if (m_pD3DDevice) m_pD3DDevice->Release();

	if (m_keyboard_device)
	{
		m_keyboard_device->Unacquire();
		m_keyboard_device->Release();
	}

	if (m_direct_input) m_direct_input->Release();

}


HRESULT InitialiseD3D();
void ShutdownD3D();




//////////////////////////////////////////////////////////////////////////////////////
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//////////////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	ShowCursor(FALSE);//disable cursor when creating window

	if (FAILED(InitialiseWindow(hInstance, nCmdShow)))
	{
		DXTRACE_MSG("Failed to create Window");
		return 0;
	}

	if (FAILED(InitialiseInput()))
	{
		DXTRACE_MSG("Failed to create Window");
		return 0;
	}

	if (FAILED(InitialiseD3D()))
	{
		DXTRACE_MSG("Failed to create Device");
		ShutdownD3D();
		return 0;
	}
	//call initialiserGraphics
	if (FAILED(InitialiseGraphics())) //03-01
	{
		DXTRACE_MSG("Failed to initialise graphics");
		return 0;
	}
	// Main message loop
	MSG msg = { 0 };

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// do something
			RenderFrame();
		}
	}

	ShutdownD3D();
	return (int)msg.wParam;
}


//////////////////////////////////////////////////////////////////////////////////////
// Register class and create window
//////////////////////////////////////////////////////////////////////////////////////
HRESULT InitialiseWindow(HINSTANCE hInstance, int nCmdShow)
{
	// Give your app window your own name
	char Name[100] = "Assignment 2 Resit\0";

	// Register class
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	//   wcex.hbrBackground = (HBRUSH )( COLOR_WINDOW + 1); // Needed for non-D3D apps
	wcex.lpszClassName = Name;

	if (!RegisterClassEx(&wcex)) return E_FAIL;

	// Create window
	g_hInst = hInstance;
	RECT rc = { 0, 0, 1920, 1080 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWnd = CreateWindow(Name, g_TutorialName, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left,
		rc.bottom - rc.top, NULL, NULL, hInstance, NULL);
	if (!g_hWnd)
		return E_FAIL;

	ShowWindow(g_hWnd, nCmdShow);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////////////////
// Called every time the application receives a message
//////////////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
		//case WM_MOUSEMOVE:
		//	POINT cursorPos;
		//	GetCursorPos(&cursorPos);
		//	if (cursorPos.x > 1920 / 2)
		//	{
		//		//g_camera_node->Rotate(GetCursorPos(&cursorPos));
		//		g_camera_node->Rotate(GetCursorPos(&cursorPos));
		//		g_pCamera->Rotate(GetCursorPos(&cursorPos));
		//	}
		//	else
		//	{
		//		//g_camera_node->Rotate(-GetCursorPos(&cursorPos));
		//		g_camera_node->Rotate(-GetCursorPos(&cursorPos));
		//		g_pCamera->Rotate(-GetCursorPos(&cursorPos));
		//	}
		//	break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//init graphics
////////////////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT InitialiseGraphics()//03-01
{
	HRESULT hr = S_OK;

	g_tent_model = new Model(m_pD3DDevice, m_pImmediateContext);
	g_tent_model->LoadObjModel("assets/tent.obj");

	g_sphere_model = new Model(m_pD3DDevice, m_pImmediateContext);
	g_sphere_model->LoadObjModel("assets/sphere.obj");

	g_cube_model = new Model(m_pD3DDevice, m_pImmediateContext);
	g_cube_model->LoadObjModel("assets/cube.obj");

	g_cube2_model = new Model(m_pD3DDevice, m_pImmediateContext);
	g_cube2_model->LoadObjModel("assets/cube.obj");

	g_tree_model = new Model(m_pD3DDevice, m_pImmediateContext);
	g_tree_model->LoadObjModel("assets/tree.obj");

	g_dog_model = new Model(m_pD3DDevice, m_pImmediateContext);
	g_dog_model->LoadObjModel("assets/bouncer.obj");


	g_camera_node = new Scene_Node();
	g_root_node = new Scene_Node();
	g_tent_node = new Scene_Node();
	g_sphere_node = new Scene_Node();
	g_cube_node = new Scene_Node();
	g_cube2_node = new Scene_Node();



	g_tree1_node = new Scene_Node();
	g_tree2_node = new Scene_Node();
	g_tree3_node = new Scene_Node();
	g_tree4_node = new Scene_Node();
	g_tree5_node = new Scene_Node();
	g_tree6_node = new Scene_Node();
	g_tree7_node = new Scene_Node();
	g_dog_node = new Scene_Node();
	g_wall_node = new Scene_Node();

	g_camera_node->SetModel(g_sphere_model);
	g_sphere_node->SetModel(g_sphere_model);
	g_cube_node->SetModel(g_cube_model);
	g_cube2_node->SetModel(g_cube2_model);
	g_tent_node->SetModel(g_tent_model);
	g_dog_node->SetModel(g_dog_model);



	g_tree1_node->SetModel(g_tree_model);
	g_tree2_node->SetModel(g_tree_model);
	g_tree3_node->SetModel(g_tree_model);
	g_tree4_node->SetModel(g_tree_model);
	g_tree5_node->SetModel(g_tree_model);
	g_tree6_node->SetModel(g_tree_model);
	g_tree7_node->SetModel(g_tree_model);
	g_dog_node->SetModel(g_dog_model);

	g_root_node->addChildNode(g_camera_node);
	g_root_node->addChildNode(g_sphere_node);
	g_root_node->addChildNode(g_cube_node);
	g_root_node->addChildNode(g_cube2_node);
	g_root_node->addChildNode(g_tent_node);

	g_root_node->addChildNode(g_tree1_node);
	g_root_node->addChildNode(g_tree2_node);
	g_root_node->addChildNode(g_tree3_node);
	g_root_node->addChildNode(g_tree4_node);
	g_root_node->addChildNode(g_tree5_node);
	g_root_node->addChildNode(g_tree6_node);
	g_root_node->addChildNode(g_tree7_node);
	g_root_node->addChildNode(g_dog_node);
	g_root_node->addChildNode(g_wall_node);

	//create constant buffer //04-1
	D3D11_BUFFER_DESC constant_buffer_desc;
	ZeroMemory(&constant_buffer_desc, sizeof(constant_buffer_desc));
	constant_buffer_desc.Usage = D3D11_USAGE_DEFAULT;  //can use updateSubresource() to update
	constant_buffer_desc.ByteWidth = 112;  //must be a multiple of 16, calculate from CB struct
	constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;  //use as a constant buffer

	hr = m_pD3DDevice->CreateBuffer(&constant_buffer_desc, NULL, &m_pConstantBuffer0);

	if (FAILED(hr))
		return hr;



	//define vertices of a triangle - screen coordinates -1.0 to +1.0
	POS_COL_TEX_NORM_VERTEX vertices[] =
	{
		//2D triangle
		//xmfloat 3 the position of the triangle   // xmfloat 4 the colorof the triangle
		//{ XMFLOAT3(0.9f, 0.9f, 0.0f),XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		//{ XMFLOAT3(0.9f, -0.9f, 0.0f),XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		//{ XMFLOAT3(-0.9f, -0.9f, 0.0f),XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }

		//3Dcube
		// back face
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f),XMFLOAT4(1.0f, -1.0f, 1.0f, 1.0f)  ,XMFLOAT2(0.0f,0.0f),XMFLOAT3(0.0f, 0.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, -1.0f, 1.0f),XMFLOAT4(1.0f, -1.0f, 1.0f, 1.0f) ,XMFLOAT2(0.0f,1.0f),XMFLOAT3(0.0f, 0.0f, 1.0f) },
	{ XMFLOAT3(1.0f, 1.0f, 1.0f),XMFLOAT4(1.0f, -1.0f, 1.0f, 1.0f)   ,XMFLOAT2(1.0f,0.0f),XMFLOAT3(0.0f, 0.0f, 1.0f) },

	{ XMFLOAT3(1.0f, 1.0f, 1.0f),XMFLOAT4(1.0f, -1.0f, 1.0f, 1.0f)  ,XMFLOAT2(1.0f,0.0f),XMFLOAT3(0.0f, 0.0f, 1.0f) },
	{ XMFLOAT3(-1.0f, -1.0f, 1.0f),XMFLOAT4(1.0f, -1.0f, 1.0f, 1.0f),XMFLOAT2(0.0f,1.0f),XMFLOAT3(0.0f, 0.0f, 1.0f) },
	{ XMFLOAT3(1.0f, -1.0f, 1.0f),XMFLOAT4(1.0f, -1.0f, 1.0f, 1.0f) ,XMFLOAT2(1.0f,1.0f),XMFLOAT3(0.0f, 0.0f, 1.0f) },

	// front face
	{ XMFLOAT3(-1.0f, -1.0f, -1.0f),XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) ,XMFLOAT2(0.0f,1.0f),XMFLOAT3(0.0f, 0.0f, -1.0f) },
	{ XMFLOAT3(-1.0f, 1.0f, -1.0f),XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)  ,XMFLOAT2(0.0f,0.0f),XMFLOAT3(0.0f, 0.0f, -1.0f) },
	{ XMFLOAT3(1.0f, 1.0f, -1.0f) ,XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)  ,XMFLOAT2(1.0f,0.0f),XMFLOAT3(0.0f, 0.0f, -1.0f) },

	{ XMFLOAT3(-1.0f, -1.0f, -1.0f),XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) ,XMFLOAT2(0.0f,1.0f),XMFLOAT3(0.0f, 0.0f, -1.0f) },
	{ XMFLOAT3(1.0f, 1.0f, -1.0f),XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)   ,XMFLOAT2(1.0f,0.0f),XMFLOAT3(0.0f, 0.0f, -1.0f) },
	{ XMFLOAT3(1.0f, -1.0f, -1.0f),XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f)  ,XMFLOAT2(1.0f,1.0f),XMFLOAT3(0.0f, 0.0f, -1.0f) },

	// left face
	{ XMFLOAT3(-1.0f, -1.0f, -1.0f),XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) ,XMFLOAT2(0.0f,1.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f) },
	{ XMFLOAT3(-1.0f, -1.0f, 1.0f),XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)  ,XMFLOAT2(0.0f,0.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f) },
	{ XMFLOAT3(-1.0f, 1.0f, -1.0f),XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)  ,XMFLOAT2(1.0f,1.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f) },

	{ XMFLOAT3(-1.0f, -1.0f, 1.0f),XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)  ,XMFLOAT2(0.0f,0.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f) },
	{ XMFLOAT3(-1.0f, 1.0f, 1.0f),XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)   ,XMFLOAT2(1.0f,0.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f) },
	{ XMFLOAT3(-1.0f, 1.0f, -1.0f),XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)  ,XMFLOAT2(1.0f,1.0f),XMFLOAT3(-1.0f, 0.0f, 0.0f) },
	// right face
	{ XMFLOAT3(1.0f, -1.0f, 1.0f),XMFLOAT4(1.0f, 0.70f, 0.40f, 1.0f)   ,XMFLOAT2(0.0f,0.0f),XMFLOAT3(1.0f, 0.0f, 0.0f) },
	{ XMFLOAT3(1.0f, -1.0f, -1.0f),XMFLOAT4(1.0f, 0.70f, 0.40f, 1.0f) ,XMFLOAT2(0.0f,1.0f),XMFLOAT3(1.0f, 0.0f, 0.0f) },
	{ XMFLOAT3(1.0f, 1.0f, -1.0f),XMFLOAT4(1.0f, 0.70f, 0.40f, 1.0f)  ,XMFLOAT2(1.0f,1.0f),XMFLOAT3(1.0f, 0.0f, 0.0f) },

	{ XMFLOAT3(1.0f, 1.0f, 1.0f),XMFLOAT4(1.0f, 0.70f, 0.40f, 1.0f)   ,XMFLOAT2(1.0f,0.0f),XMFLOAT3(1.0f, 0.0f, 0.0f) },
	{ XMFLOAT3(1.0f, -1.0f, 1.0f),XMFLOAT4(1.0f, 0.70f, 0.40f, 1.0f)  ,XMFLOAT2(0.0f,0.0f),XMFLOAT3(1.0f, 0.0f, 0.0f) },
	{ XMFLOAT3(1.0f, 1.0f, -1.0f),XMFLOAT4(1.0f, 0.70f, 0.40f, 1.0f)  ,XMFLOAT2(1.0f,1.0f),XMFLOAT3(1.0f, 0.0f, 0.0f) },

	// bottom face
	{ XMFLOAT3(1.0f, -1.0f, -1.0f),XMFLOAT4(0.20f, 0.30f, 0.40f, 1.0f)   ,XMFLOAT2(1.0f,1.0f),XMFLOAT3(0.0f, -1.0f, 0.0f) },
	{ XMFLOAT3(1.0f, -1.0f, 1.0f),XMFLOAT4(0.20f, 0.30f, 0.40f, 1.0f)   ,XMFLOAT2(1.0f,0.0f),XMFLOAT3(0.0f, -1.0f, 0.0f) },
	{ XMFLOAT3(-1.0f, -1.0f, -1.0f),XMFLOAT4(0.20f, 0.30f, 0.40f, 1.0f) ,XMFLOAT2(0.0f,1.0f),XMFLOAT3(0.0f, -1.0f, 0.0f) },

	{ XMFLOAT3(1.0f, -1.0f, 1.0f),XMFLOAT4(0.20f, 0.30f, 0.40f, 1.0f)   ,XMFLOAT2(1.0f,0.0f),XMFLOAT3(0.0f, -1.0f, 0.0f) },
	{ XMFLOAT3(-1.0f, -1.0f, 1.0f),XMFLOAT4(0.20f, 0.30f, 0.40f, 1.0f)  ,XMFLOAT2(0.0f,0.0f),XMFLOAT3(0.0f, -1.0f, 0.0f) },
	{ XMFLOAT3(-1.0f, -1.0f, -1.0f),XMFLOAT4(0.20f, 0.30f, 0.40f, 1.0f) ,XMFLOAT2(0.0f,1.0f),XMFLOAT3(0.0f, -1.0f, 0.0f) },

	// top face
	{ XMFLOAT3(1.0f, 1.0f, 1.0f),XMFLOAT4(-1.0f, -1.0f,0.30f, 1.0f)    ,XMFLOAT2(1.0f,0.0f),XMFLOAT3(0.0f, 1.0f, 0.0f) },
	{ XMFLOAT3(1.0f, 1.0f, -1.0f),XMFLOAT4(-1.0f, -1.0f,0.30f, 1.0f)  ,XMFLOAT2(1.0f,1.0f),XMFLOAT3(0.0f, 1.0f, 0.0f) },
	{ XMFLOAT3(-1.0f, 1.0f, -1.0f),XMFLOAT4(-1.0f, -1.0f,0.30f, 1.0f) ,XMFLOAT2(0.0f,1.0f),XMFLOAT3(0.0f, 1.0f, 0.0f) },

	{ XMFLOAT3(-1.0f, 1.0f, 1.0f),XMFLOAT4(-1.0f, -1.0f,0.30f, 1.0f)  ,XMFLOAT2(0.0f,0.0f),XMFLOAT3(0.0f, 1.0f, 0.0f) },
	{ XMFLOAT3(1.0f, 1.0f, 1.0f),XMFLOAT4(-1.0f, -1.0f,0.30f, 1.0f)   ,XMFLOAT2(1.0f,0.0f),XMFLOAT3(0.0f, 1.0f, 0.0f) },
	{ XMFLOAT3(-1.0f, 1.0f, -1.0f),XMFLOAT4(-1.0f, -1.0f,0.30f, 1.0f) ,XMFLOAT2(0.0f,1.0f),XMFLOAT3(0.0f, 1.0f, 0.0f) },


	};

	UINT stride = sizeof(POS_COL_TEX_NORM_VERTEX);
	UINT offset = 0;
	m_pImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//set up and create vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;   //used by CPU and GPU
	bufferDesc.ByteWidth = sizeof(vertices);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; //use as vertex buffer
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; //allow CPU access
	hr = m_pD3DDevice->CreateBuffer(&bufferDesc, NULL, &m_pVertexBuffer);//create the buffer

	if (FAILED(hr))// return error code on failure
	{
		return hr;
	}

	// adding the sampler 
	D3D11_SAMPLER_DESC sampler_desc;
	ZeroMemory(&sampler_desc, sizeof(sampler_desc));
	sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;

	m_pD3DDevice->CreateSamplerState(&sampler_desc, &m_pSampler0);

	//adding the texture from the assets file
	D3DX11CreateShaderResourceViewFromFile(m_pD3DDevice, "assets/what.png", NULL, NULL, &m_pTexture0, NULL);

	//copy the vertices into the buffer
	D3D11_MAPPED_SUBRESOURCE ms;

	//lock the buffer to allow writting
	m_pImmediateContext->Map(m_pVertexBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);

	//copy the data
	memcpy(ms.pData, vertices, sizeof(vertices));

	//unlock the buffer
	m_pImmediateContext->Unmap(m_pVertexBuffer, NULL);

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
	hr = m_pD3DDevice->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &m_pVertexShader);

	if (FAILED(hr))
	{
		return hr;
	}


	hr = m_pD3DDevice->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &m_pPixelShader);

	if (FAILED(hr))
	{
		return hr;
	}



	//set the shader objects as active
	m_pImmediateContext->VSSetShader(m_pVertexShader, 0, 0);
	m_pImmediateContext->PSSetShader(m_pPixelShader, 0, 0);

	//create and set the input layout object
	D3D11_INPUT_ELEMENT_DESC iedesc[] =
	{
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
	{ "COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0 },
	{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,0 },
	{ "NORMAL", 0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0 }
	};

	hr = m_pD3DDevice->CreateInputLayout(iedesc, ARRAYSIZE(iedesc), VS->GetBufferPointer(), VS->GetBufferSize(), &m_pInputLayout);

	if (FAILED(hr))
	{
		return hr;
	}


	m_pImmediateContext->IASetInputLayout(m_pInputLayout);

	// add the camera and add values instead of parameters in the same order
	g_pCamera = new Camera(0.0f, 0.0f, -0.5f, 0.0f);


	g_tent_model->SetSampler(m_pSampler0);
	g_tent_model->SetTexture(m_pTexture0);
	g_sphere_model->SetSampler(m_pSampler0);
	g_sphere_model->SetTexture(m_pTexture0);

	SetPositions();

	return S_OK;
}


// Render frame
void RenderFrame(void)
{


#pragma region Input

	ReadInputStates();

	if (IsKeyPressed(DIK_ESCAPE)) DestroyWindow(g_hWnd);
	if (IsKeyPressed(DIK_LEFT)) {
		g_pCamera->Right(-g_camera_speed);

		// set camera node to the position of the camera
		g_camera_node->SetXPos(g_pCamera->GetX());
		g_camera_node->SetYPos(g_pCamera->GetY());
		g_camera_node->SetZPos(g_pCamera->GetZ());

		XMMATRIX identity = XMMatrixIdentity();

		// update tree to reflect new camera position
		g_root_node->update_collision_tree(&identity, 1.0);

		if (g_camera_node->check_collision(g_root_node, g_camera_node) == true)
		{
			// if there is a collision, restore camera and camera node positions
			g_pCamera->Forward(g_camera_speed);
			g_camera_node->SetXPos(g_pCamera->GetX()); //15
			g_camera_node->SetYPos(g_pCamera->GetY());//15
			g_camera_node->SetZPos(g_pCamera->GetZ());//15

		}
	}
	if (IsKeyPressed(DIK_RIGHT)) {
		g_pCamera->Right(g_camera_speed);

		// set camera node to the position of the camera
		g_camera_node->SetXPos(g_pCamera->GetX());
		g_camera_node->SetYPos(g_pCamera->GetY());
		g_camera_node->SetZPos(g_pCamera->GetZ());

		XMMATRIX identity = XMMatrixIdentity();

		// update tree to reflect new camera position
		g_root_node->update_collision_tree(&identity, 1.0);

		if (g_camera_node->check_collision(g_root_node, g_camera_node) == true)
		{
			// if there is a collision, restore camera and camera node positions
			g_pCamera->Forward(-g_camera_speed);
			g_camera_node->SetXPos(g_pCamera->GetX()); //15
			g_camera_node->SetYPos(g_pCamera->GetY());//15
			g_camera_node->SetZPos(g_pCamera->GetZ());//15

		}
	}
	if (IsKeyPressed(DIK_UP))
	{
		g_pCamera->Forward(g_camera_speed);

		// set camera node to the position of the camera
		g_camera_node->SetXPos(g_pCamera->GetX());
		g_camera_node->SetYPos(g_pCamera->GetY());
		g_camera_node->SetZPos(g_pCamera->GetZ());

		XMMATRIX identity = XMMatrixIdentity();

		// update tree to reflect new camera position
		g_root_node->update_collision_tree(&identity, 1.0);

		if (g_camera_node->check_collision(g_root_node, g_camera_node) == true)
		{
			// if there is a collision, restore camera and camera node positions
			g_pCamera->Forward(-g_camera_speed);
			g_camera_node->SetXPos(g_pCamera->GetX()); //15
			g_camera_node->SetYPos(g_pCamera->GetY());//15
			g_camera_node->SetZPos(g_pCamera->GetZ());//15

		}
	}
	if (IsKeyPressed(DIK_DOWN))
	{
		g_pCamera->Forward(-g_camera_speed);

		// set camera node to the position of the camera
		g_camera_node->SetXPos(g_pCamera->GetX());
		g_camera_node->SetYPos(g_pCamera->GetY());
		g_camera_node->SetZPos(g_pCamera->GetZ());

		XMMATRIX identity = XMMatrixIdentity();

		// update tree to reflect new camera position
		g_root_node->update_collision_tree(&identity, 1.0);

		if (g_camera_node->check_collision(g_root_node, g_camera_node) == true)
		{
			// if there is a collision, restore camera and camera node positions
			g_pCamera->Forward(g_camera_speed);
			g_camera_node->SetXPos(g_pCamera->GetX()); //15
			g_camera_node->SetYPos(g_pCamera->GetY());//15
			g_camera_node->SetZPos(g_pCamera->GetZ());//15

		}
	}
	if (IsKeyPressed(DIK_A)) {
		g_pCamera->Right(-g_camera_speed);

		// set camera node to the position of the camera
		g_camera_node->SetXPos(g_pCamera->GetX());
		g_camera_node->SetYPos(g_pCamera->GetY());
		g_camera_node->SetZPos(g_pCamera->GetZ());

		XMMATRIX identity = XMMatrixIdentity();

		// update tree to reflect new camera position
		g_root_node->update_collision_tree(&identity, 1.0);

		if (g_camera_node->check_collision(g_root_node, g_camera_node) == true)
		{
			// if there is a collision, restore camera and camera node positions
			g_pCamera->Right(g_camera_speed);
			g_camera_node->SetXPos(g_pCamera->GetX()); //15
			g_camera_node->SetYPos(g_pCamera->GetY());//15
			g_camera_node->SetZPos(g_pCamera->GetZ());//15

		}
	}
	if (IsKeyPressed(DIK_D))
	{
		g_pCamera->Right(g_camera_speed);

		// set camera node to the position of the camera
		g_camera_node->SetXPos(g_pCamera->GetX());
		g_camera_node->SetYPos(g_pCamera->GetY());
		g_camera_node->SetZPos(g_pCamera->GetZ());

		XMMATRIX identity = XMMatrixIdentity();

		// update tree to reflect new camera position
		g_root_node->update_collision_tree(&identity, 1.0);

		if (g_camera_node->check_collision(g_root_node, g_camera_node) == true)
		{
			// if there is a collision, restore camera and camera node positions
			g_pCamera->Right(-g_camera_speed);
			g_camera_node->SetXPos(g_pCamera->GetX()); //15
			g_camera_node->SetYPos(g_pCamera->GetY());//15
			g_camera_node->SetZPos(g_pCamera->GetZ());//15

		}
	}
	if (IsKeyPressed(DIK_W))
	{
		g_pCamera->Forward(g_camera_speed);

		// set camera node to the position of the camera
		g_camera_node->SetXPos(g_pCamera->GetX());
		g_camera_node->SetYPos(g_pCamera->GetY());
		g_camera_node->SetZPos(g_pCamera->GetZ());

		XMMATRIX identity = XMMatrixIdentity();

		// update tree to reflect new camera position
		g_root_node->update_collision_tree(&identity, 1.0);

		if (g_camera_node->check_collision(g_root_node, g_camera_node) == true)
		{
			// if there is a collision, restore camera and camera node positions
			g_pCamera->Forward(-g_camera_speed);
			g_camera_node->SetXPos(g_pCamera->GetX()); //15
			g_camera_node->SetYPos(g_pCamera->GetY());//15
			g_camera_node->SetZPos(g_pCamera->GetZ());//15

		}
	}
	if (IsKeyPressed(DIK_S))
	{
		g_pCamera->Forward(-g_camera_speed);

		// set camera node to the position of the camera
		g_camera_node->SetXPos(g_pCamera->GetX());
		g_camera_node->SetYPos(g_pCamera->GetY());
		g_camera_node->SetZPos(g_pCamera->GetZ());

		XMMATRIX identity = XMMatrixIdentity();

		// update tree to reflect new camera position
		g_root_node->update_collision_tree(&identity, 1.0);

		if (g_camera_node->check_collision(g_root_node, g_camera_node) == true)
		{
			// if there is a collision, restore camera and camera node positions
			g_pCamera->Forward(g_camera_speed);
			g_camera_node->SetXPos(g_pCamera->GetX()); //15
			g_camera_node->SetYPos(g_pCamera->GetY());//15
			g_camera_node->SetZPos(g_pCamera->GetZ());//15

		}
	}

	if (IsKeyPressed(DIK_LSHIFT))
	{
		g_camera_speed += 0.01;
	}
	if (IsKeyPressed(DIK_U))
	{
		g_cube_node->IncX(0.01, g_root_node);
	}
	if (IsKeyPressed(DIK_J))
	{
		g_cube_node->IncX(-0.01, g_root_node);
	}
	if (IsKeyPressed(DIK_H))
	{
		g_cube_node->IncZ(0.01, g_root_node);
	}
	if (IsKeyPressed(DIK_K))
	{
		g_cube_node->IncZ(-0.01, g_root_node);
	}
	else
	{
		g_camera_speed = 0.01;
	}

	if (HasMouseMoved())
	{
		g_pCamera->Rotate(GetMouseX() * 0.1);
	}

#pragma endregion


	//XMVECTOR g_directional_light_colour;
	g_ambient_light_colour = XMVectorSet(1, 1, 1, 1);
	XMMATRIX projection, emptyworld, view, world;
	XMMATRIX transpose;
	//CONSTANT_BUFFER0 cb0_values;

	cb0_values.directional_light_colour = g_directional_light_colour;
	cb0_values.ambient_light_colour = g_ambient_light_colour;
	cb0_values.directional_light_vector = XMVector3Transform(g_directional_light_shines_from, transpose);
	cb0_values.directional_light_vector = XMVector3Normalize(cb0_values.directional_light_vector);


	transpose = XMMatrixTranspose(emptyworld); // model world matrix


											   // Clear the back buffer - choose a color you like
	float rgba_clear_colour[4] = { 0.55f, 0.80f, 0.80f, 1.0f };
	m_pImmediateContext->ClearRenderTargetView(m_pBackBufferRTView, rgba_clear_colour);
	m_pImmediateContext->ClearDepthStencilView(m_pZBuffer, D3D11_CLEAR_DEPTH | D3D10_CLEAR_STENCIL, 1.0f, 0);


	m_pImmediateContext->PSSetSamplers(0, 1, &m_pSampler0);
	m_pImmediateContext->PSSetShaderResources(0, 1, &m_pTexture0);


	SetCursorPos(1920 / 2, 1080 / 2);

	//comment out these lines, run the program, and uncomment again to fix the matrix bug

	//world = XMMatrixRotationX(XMConvertToRadians((float)cursorPos.x));
	//world *= XMMatrixRotationY(XMConvertToRadians((float)cursorPos.y));



#pragma region world definition

	projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0), 640.0 / 480.0, 1.0f, 100.0f);
	view = g_pCamera->GetViewMatrix();
#pragma endregion

	world = XMMatrixIdentity();

	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	g_root_node->execute(&world, &view, &projection);



#pragma region interactions


	if (g_camera_node->check_collision(g_cube_node, g_root_node) && (IsKeyPressed(DIK_E)))
	{
		g_cube_node->SetYRot(g_cube_node->GetYRot() + 1);
	}

	if (g_camera_node->check_collision(g_sphere_node, g_root_node) && (IsKeyPressed(DIK_E)))
	{
		g_sphere_node->SetScale(g_sphere_node->GetScale() - 0.0005);
	}

	if (g_camera_node->check_collision(g_tent_node, g_root_node) && (IsKeyPressed(DIK_E)))
	{
		g_root_node->detatchNode(g_tent_node);
	}

	if (g_camera_node->check_collision(g_dog_node, g_root_node) && (IsKeyPressed(DIK_E)))
	{
		g_dog_node->SetScale(g_dog_node->GetScale() + 0.005);
	}


	if (IsKeyPressed(DIK_C))
	{
		g_cube_follow = !g_cube_follow;
	}

	if ((IsKeyPressed(DIK_SPACE)))
	{
		g_bullet_node = new Scene_Node();
		g_bullet_node->SetModel(g_sphere_model);

		g_bullet_node->SetXPos(g_camera_node->GetXPos());
		g_bullet_node->SetYPos(g_camera_node->GetYPos());
		g_bullet_node->SetZPos(g_camera_node->GetZPos());

		g_bullet_node->SetXRot(g_camera_node->GetXRot());
		g_bullet_node->SetYRot(g_camera_node->GetYRot());
		g_bullet_node->SetZRot(g_camera_node->GetZRot());

		g_bullet_node->SetScale(0.1);
		g_bullet_node->LookAt_XZ(g_camera_node->GetXPos(), g_camera_node->GetZPos());
		g_bullet_node->Forward(5);

		g_root_node->addChildNode(g_bullet_node);

	}



	g_cube2_node->LookAt_XZ(g_camera_node->GetXPos(), g_camera_node->GetZPos());


	if (g_cube_follow == true)
	{
		g_cube2_model->CalculateBoundingSphereRadius();
		if (g_camera_node->check_collision(g_cube2_node, g_camera_node) == false)
		{
			g_cube2_node->Forward(0.005);
		}
		if (g_camera_node->check_collision(g_cube2_node, g_camera_node) == true)
		{
			// if there is a collision, restore 
			g_cube2_node->Forward(-0.01);

		}
	}

#pragma endregion

#pragma region Text
	g_2DText->AddText("Press E to interact with objects", -1.0, +1.0, .03);
	g_2DText->AddText("Press UHJK to move cube", -1.0, +0.97, .03);
	g_2DText->AddText(("Press C to toggle enemy follow: " + std::to_string(g_cube_follow)), -1.0, +0.94, .03);
	g_2DText->RenderText();
#pragma endregion

	// Display what has just been rendered
	m_pSwapChain->Present(0, 0);
}

void SetPositions()
{
#pragma region setting positions

	g_camera_node->LookAt_XZ(g_cube2_node->GetXPos(), g_cube2_node->GetZPos());

	g_tent_node->SetYPos(-1);
	g_tent_node->SetXPos(15);
	g_tent_node->SetZPos(-10);

	g_tree1_node->SetXPos(-15);
	g_tree1_node->SetZPos(-15);

	g_tree2_node->SetXPos(-30);
	g_tree2_node->SetZPos(-30);

	g_tree3_node->SetXPos(-45);
	g_tree3_node->SetZPos(-45);

	g_tree4_node->SetXPos(-15);
	g_tree4_node->SetZPos(-30);

	g_tree5_node->SetXPos(-45);
	g_tree5_node->SetZPos(10);

	g_tree6_node->SetXPos(30);
	g_tree6_node->SetZPos(20);

	g_tree7_node->SetXPos(30);
	g_tree7_node->SetZPos(45);

	g_sphere_node->SetXPos(25);
	g_sphere_node->SetZPos(15);

	g_cube_node->SetXPos(10);
	g_cube_node->SetZPos(10);

	g_cube2_node->SetXPos(20);
	g_cube2_node->SetZPos(-2.5);

	g_dog_node->SetXPos(10);
	g_dog_node->SetYPos(-2.5);
	g_dog_node->SetZPos(-2);
	g_dog_node->SetScale(0.1);



	//g_cube_node->IncX(1, g_root_node);
#pragma endregion

}
