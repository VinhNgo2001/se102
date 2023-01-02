#include<windows.h>
#include<d3d10.h>
#include<d3dx10.h>

#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>

#define WINDOW_CLASS_NAME L"SampleWindow"
#define WINDOW_TITLE L"00 - Intro"
#define WINDOW_ICON_PATH L"brick.ico" 

HWND hWnd = 0;
#define BACKGROUND_COLOR D3DXCOLOR(0.2f, 0.2f, 0.2f, 0.2f)

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define MAX_FRAME_RATE 100

ID3D10Device* pD3DDevice = NULL;
IDXGISwapChain* pSwapChain = NULL;
ID3D10RenderTargetView* pRenderTargetView = NULL;

int BackBufferWidth = 0;
int BackBufferHeight = 0;



#define TEXTURE_PATH_BRICK L"brick.png"
#define BRICK_START_X 8.0f
#define BRICK_START_Y 200.0f

#define BRICK_START_VX 0.2f

#define BRICK_WIDTH 16.0f
#define BRICK_HEIGHT 16.0f

ID3D10Texture2D* texBrick = NULL;
ID3DX10Sprite* spriteObject = NULL;

D3DX10_SPRITE spriteBrick;

float brick_x = BRICK_START_X;
float brick_vx = BRICK_START_VX; 
float brick_y = BRICK_START_Y;

LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;

}

#define _W(x)  __W(x)
#define __W(x)  L##x

#define VA_PRINTS(s) {				\
		va_list argp;				\
		va_start(argp, fmt);		\
		vswprintf_s(s, fmt, argp);	\
		va_end(argp);				\
}

void DebugOut(const wchar_t* fmt, ...)
{
	wchar_t s[4096];
	VA_PRINTS(s);
	OutputDebugString(s);
}

void DebugOutTitle(const wchar_t* fmt, ...)
{
	wchar_t s[1024];
	VA_PRINTS(s);
	SetWindowText(hWnd, s);
}

void InitDirectX(HWND hWnd) {
	RECT r;
	GetClientRect(hWnd, &r);

	BackBufferWidth = r.right + 1;
	BackBufferHeight = r.bottom + 1;

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = BackBufferWidth;
	swapChainDesc.BufferDesc.Height = BackBufferHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = TRUE;

	HRESULT hr = D3D10CreateDeviceAndSwapChain(NULL,
		D3D10_DRIVER_TYPE_REFERENCE,
		NULL,
		0,
		D3D10_SDK_VERSION,
		&swapChainDesc,
		&pSwapChain,
		&pD3DDevice);
	
	if (hr != S_OK)
	{
		DebugOut((wchar_t*)L"[ERROR] D3D10CreateDeviceAndSwapChain has failed %s %d", _W(__FILE__), __LINE__);
		return;
	}

	ID3D10Texture2D* pBackBuffer;
	hr = pSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID*)&pBackBuffer);
	
	if (hr != S_OK)
	{
		DebugOut((wchar_t*)L"[ERROR] pSwapChain->GetBuffer has failed %s %d", _W(__FILE__), __LINE__);
		return;
	}
	hr = pD3DDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView);

	pBackBuffer->Release();

	if (hr != S_OK)
	{
		DebugOut((wchar_t*)L"[ERROR] CreateRenderTargetView has failed %s %d", _W(__FILE__), __LINE__);
		return;
	}
	pD3DDevice->OMSetRenderTargets(1, &pRenderTargetView, NULL);
	
	D3D10_VIEWPORT viewPort;
	viewPort.Width = BackBufferWidth;
	viewPort.Height = BackBufferHeight;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	pD3DDevice->RSSetViewports(1, &viewPort);

	// create the sprite oj to handle sprite drawing
	hr = D3DX10CreateSprite(pD3DDevice, 0, &spriteObject);

	if (hr != S_OK)
	{
		DebugOut((wchar_t*)L"[ERROR] D3DX10CreateSprite has failed %s %d", _W(__FILE__), __LINE__);
		return;
	}

	D3DXMATRIX matProjection;

	D3DXMatrixOrthoOffCenterLH(
		&matProjection,
		(float)viewPort.TopLeftX,
		(float)viewPort.Width,
		(float)viewPort.TopLeftY,
		(float)viewPort.Height,
		0.1f,
		10
	);

	hr = spriteObject->SetProjectionTransform(&matProjection);

	DebugOut((wchar_t*)L"[INFO] InitDirectX has been successful\n");

	return;
}

void LoadResources()
{
	ID3D10Resource* pD3D10Resource = NULL;
	//load the texture intro a ....
	HRESULT hr = D3DX10CreateTextureFromFile(
		pD3DDevice,
		TEXTURE_PATH_BRICK,
		NULL,
		NULL,
		&pD3D10Resource,
		NULL
	);
	//make sure the texture was loaded successfully
	if (FAILED(hr))
	{
		DebugOut((wchar_t*)L"[ERROR] Failed to load texture file: %s \n", TEXTURE_PATH_BRICK);
		return;
	}
	//
	pD3D10Resource->QueryInterface(__uuidof(ID3D10Texture2D), (LPVOID*)&texBrick);
	pD3D10Resource->Release();

	if (!texBrick) {
		DebugOut((wchar_t*)L"[ERROR] Failed to convert from ID3D10Resource to ID3D10Texture2D \n");
		return;
	}

	// get the texture details
	D3D10_TEXTURE2D_DESC desc;
	texBrick->GetDesc(&desc);

	//
	D3D10_SHADER_RESOURCE_VIEW_DESC SRVDesc;

	//
	ZeroMemory(&SRVDesc, sizeof(SRVDesc));

	//ser the textures format
	SRVDesc.Format = desc.Format;
	//
	SRVDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = desc.MipLevels;

	ID3D10ShaderResourceView* gSpriteTextureRV = NULL;

	pD3DDevice->CreateShaderResourceView(texBrick, &SRVDesc, &gSpriteTextureRV);

	// set the sprite's shader resources view
	spriteBrick.pTexture = gSpriteTextureRV;

	// top-left location in U,V coords
	spriteBrick.TexCoord.x = 0;
	spriteBrick.TexCoord.y = 0;

	// Determine the texture size in U,V coords
	spriteBrick.TexSize.x = 1.0f;
	spriteBrick.TexSize.y = 1.0f;

	// Set the texture index. Single textures will use 0
	spriteBrick.TextureIndex = 0;

	// The color to apply to this sprite, full color applies white.
	spriteBrick.ColorModulate = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);


	DebugOut((wchar_t*)L"[INFO] Texture loaded Ok: %s \n", TEXTURE_PATH_BRICK);	
}

void Update(DWORD dt)
{
	brick_x += brick_vx * dt;
	float right_edge = BackBufferWidth - BRICK_WIDTH;
	if (brick_x < 0 || brick_x > right_edge) {
		brick_vx = -brick_vx;
	}

}

void Render()
{
	if (pD3DDevice != NULL)
	{	
		// clear the target buffet
		pD3DDevice->ClearRenderTargetView(pRenderTargetView, BACKGROUND_COLOR);
		
		// start drawting th sprites

		spriteObject->Begin(D3DX10_SPRITE_SORT_TEXTURE);

		//
		D3DXMATRIX matTranslation;
		//
		D3DXMatrixTranslation(&matTranslation, brick_x, (BackBufferHeight - brick_y), 0.1f);

		//scale sprite to its correct width and height
		D3DXMATRIX matScaling;
		D3DXMatrixScaling(&matScaling, BRICK_WIDTH, BRICK_HEIGHT, 1.0f);

		// setting the sprite's possition and size
		spriteBrick.matWorld = (matScaling * matTranslation);

		spriteObject->DrawSpritesImmediate(&spriteBrick, 1, 0, 0);

		//finish up and ssend the sprite to the hardware
		spriteObject->End();

		pSwapChain->Present(0, 0);
	}

}

HWND CreateGameWindow(HINSTANCE hInstance, int nCmdShow, int ScreenWidth, int ScreenHeight)
{
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hInstance = hInstance;

	wc.lpfnWndProc = (WNDPROC)WinProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hIcon = (HICON)LoadImage(hInstance, WINDOW_ICON_PATH, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WINDOW_CLASS_NAME;

	wc.hIconSm = NULL;

	RegisterClassEx(&wc);

	HWND hWnd =
		CreateWindow(
			WINDOW_CLASS_NAME,
			WINDOW_TITLE,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			ScreenWidth,
			ScreenHeight,
			NULL,
			NULL,
			hInstance,
			NULL
			);
	if (!hWnd)
	{
		DWORD ErrCode = GetLastError();
		DebugOut((wchar_t*)L"[ERROR] CreateWindow failed! ErrCode: %d\nAt: %s %d \n", ErrCode, _W(__FILE__), __LINE__);
		return 0;
	}
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return hWnd;

}
int Run()
{
	MSG msg;
	int done = 0;
	ULONGLONG frameStart = GetTickCount64();
	ULONGLONG tickPerFrame = 1000 / MAX_FRAME_RATE;

	while (!done)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) done = 1;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		ULONGLONG now = GetTickCount64();

		ULONGLONG dt = now - frameStart;

		if (dt >= tickPerFrame)
		{
			frameStart = now;
			Update((DWORD)dt);
			Render();
		}
		else
			Sleep((DWORD)(tickPerFrame - dt));
	}
	return 1;
 }

void Cleanup()
{
	if (pRenderTargetView)
	{
		pRenderTargetView->Release();
	}
	if (pSwapChain)
	{
		pSwapChain->Release();
	}

	if (pD3DDevice)
	{
		pD3DDevice->Release();
	}

	if (spriteObject)
	{
		spriteObject->Release();
		spriteObject = NULL;

	}
	DebugOut((wchar_t*)L"[INFO] Cleanup Ok\n");
}

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nCmdShow
)
{
	hWnd = CreateGameWindow(hInstance, nCmdShow, WINDOW_WIDTH, WINDOW_HEIGHT);
	if (hWnd == 0) return 0;
	InitDirectX(hWnd);
	LoadResources();
	Run();
	Cleanup();

	return 0;
}