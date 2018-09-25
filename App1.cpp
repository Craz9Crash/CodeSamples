//This code was taken from my graphics project that at carried out while at University. 
//The purpose of this code was to try different graphics programming techniques using shaders with DirectX.
#include "App1.h"

App1::App1()
{
	//BaseApplication::BaseApplication();
	m_CubeMesh = nullptr;
	m_SphereMesh = nullptr;
	m_PlaneMesh = nullptr;
	m_LightShader = nullptr;
	m_ShadowShader = nullptr;
	m_SoftShader = nullptr;
	m_TessShader = nullptr;
	m_GeometryShader = nullptr;
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in)
{
	// Call super init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in);

	tessFactor = 5;

	// Create Mesh object
	m_CubeMesh = new CubeMesh(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), L"../res/DefaultDiffuse.png");
	m_SphereMesh = new SphereMesh(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), L"../res/DefaultDiffuse.png");
	m_PlaneMesh = new PlaneMesh(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), L"../res/DefaultDiffuse.png");
	m_OrthoMesh = new OrthoMesh(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), screenWidth, screenHeight, 0, 0);
	m_smallOrthoMesh = new OrthoMesh(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), screenWidth, screenHeight, 0, 0);
	m_TessMesh = new PlaneTessMesh(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), L"../res/Heightmap.png");
	m_PointMesh = new PointMesh(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), L"../res/DefaultDiffuse.png");

	m_RenderTexture = new RenderTexture(m_Direct3D->GetDevice(), SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, SCREEN_NEAR, SCREEN_DEPTH);
	m_RenderTexture2 = new RenderTexture(m_Direct3D->GetDevice(), SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, SCREEN_NEAR, SCREEN_DEPTH);
	m_RenderTexture3 = new RenderTexture(m_Direct3D->GetDevice(), SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, SCREEN_NEAR, SCREEN_DEPTH);
	m_RenderTexture4 = new RenderTexture(m_Direct3D->GetDevice(), SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, SCREEN_NEAR, SCREEN_DEPTH);
	m_ShadowRenderTex = new RenderTexture(m_Direct3D->GetDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);
	m_DownSampleTexture = new RenderTexture(m_Direct3D->GetDevice(), screenWidth / 2, screenHeight / 2, SCREEN_NEAR, SCREEN_DEPTH);
	m_HorTexture = new RenderTexture(m_Direct3D->GetDevice(), screenWidth / 2, screenHeight / 2, SCREEN_NEAR, SCREEN_DEPTH);
	m_VerTexture = new RenderTexture(m_Direct3D->GetDevice(), screenWidth / 2, screenHeight / 2, SCREEN_NEAR, SCREEN_DEPTH);
	m_UpSampleTexture = new RenderTexture(m_Direct3D->GetDevice(), screenWidth, screenHeight, SCREEN_NEAR, SCREEN_DEPTH);

	m_LightShader = new LightShader(m_Direct3D->GetDevice(), hwnd);
	m_DepthShader = new DepthShader(m_Direct3D->GetDevice(), hwnd);
	m_ShadowShader = new ShadowShader(m_Direct3D->GetDevice(), hwnd);
	m_TessShader = new TessellationShader(m_Direct3D->GetDevice(), hwnd);
	m_GeometryShader = new GeometryShader(m_Direct3D->GetDevice(), hwnd);
	m_SoftShader = new SoftShadowShader(m_Direct3D->GetDevice(), hwnd);
	m_HorizontalShader = new HorizontalBlurShader(m_Direct3D->GetDevice(), hwnd);
	m_VerticalShader = new VerticalBlurShader(m_Direct3D->GetDevice(), hwnd);
	m_TextureShader = new TextureShader(m_Direct3D->GetDevice(), hwnd);

	m_Light = new Light;
	m_Light->SetPosition(0.0, 20.0, 0.0);
	m_Light->SetLookAt(50.0, 0.0, 50.0);
	m_Light->SetDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light->SetAmbientColour(0.3f, 0.3f, 0.3f, 1.0f);
	m_Light->SetDirection(0.5f, -0.5f, 0.0f);
	m_Light->GenerateViewMatrix();
	m_Light->GenerateProjectionMatrix(SCREEN_NEAR, SCREEN_DEPTH);

	m_Light2 = new Light;
	m_Light2->SetPosition(100.0, 20.0, 0.0);
	m_Light2->SetLookAt(50.0, 0.0, 50.0);
	m_Light2->SetDiffuseColour(1.0f, 0.0f, 0.0f, 1.0f);
	m_Light2->SetDirection(0.5f, -0.5f, 0.0f);
	m_Light2->GenerateViewMatrix();
	m_Light2->GenerateProjectionMatrix(SCREEN_NEAR, SCREEN_DEPTH);

	m_Light3 = new Light;
	m_Light3->SetPosition(100.0, 20.0, 100.0);
	m_Light3->SetLookAt(50.0, 0.0, 50.0);
	m_Light3->SetDiffuseColour(1.0f, 0.0f, 0.0f, 1.0f);
	m_Light3->SetDirection(0.5f, -0.5f, 0.0f);
	m_Light3->GenerateViewMatrix();
	m_Light3->GenerateProjectionMatrix(SCREEN_NEAR, SCREEN_DEPTH);

	m_Light4 = new Light;
	m_Light4->SetPosition(0.0, 20.0, 100.0);
	m_Light4->SetLookAt(50.0, 0.0, 50.0);
	m_Light4->SetDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light4->SetDirection(0.5f, -0.5f, 0.0f);
	m_Light4->GenerateViewMatrix();
	m_Light4->GenerateProjectionMatrix(SCREEN_NEAR, SCREEN_DEPTH);

	screenw = screenWidth;
	screenh = screenHeight;
	Wireframe = false;
	Shadowmap = false;
	//clear_col = ImColor(100, 0, 100);
}


App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.


	if (m_TextureShader)
	{
		delete m_TextureShader;
		m_TextureShader = 0;
	}

	if (m_LightShader)
	{
		delete m_LightShader;
		m_LightShader = 0;
	}

	if (m_DepthShader)
	{
		delete m_DepthShader;
		m_DepthShader = 0;
	}

	if (m_ShadowShader)
	{
		delete m_ShadowShader;
		m_ShadowShader = 0;
	}

	if (m_TessShader)
	{
		delete m_TessShader;
		m_TessShader = 0;
	}

	if (m_GeometryShader)
	{
		delete m_GeometryShader;
		m_GeometryShader = 0;
	}

	if (m_SoftShader)
	{
		delete m_SoftShader;
		m_SoftShader = 0;
	}

	if (m_HorizontalShader)
	{
		delete m_HorizontalShader;
		m_HorizontalShader = 0;
	}

	if (m_VerticalShader)
	{
		delete m_VerticalShader;
		m_VerticalShader = 0;
	}

}


bool App1::Frame()
{
	bool result;

	result = BaseApplication::Frame();
	if (!result)
	{
		return false;
	}

	// Render the graphics.
	result = Render();
	if (!result)
	{
		return false;
	}

	//increase or decrease tessellation factor
	if (m_Input->isKeyDown('P'))
	{
		tessFactor += 0.5;
	}

	if (m_Input->isKeyDown('O'))
	{
		tessFactor -= 0.5;
	}

	if (tessFactor < 1)
	{
		tessFactor = 1;
	}

	if (tessFactor > 10)
	{
		tessFactor = 10;
	}

	//Wireframe mode
	if (Wireframe == true)
	{
		m_Direct3D->TurnOnWireframe();
	}
	else
	{
		m_Direct3D->TurnOffWireframe();
	}

	return true;
}

bool App1::Render()
{
	//Generates depth maps
	RenderToTexture();
	RenderToTexture2();
	RenderToTexture3();
	RenderToTexture4();

	//Generates normal shadows
	ShadowRender();

	//post processing functions
	DownSample();
	HorizontalBlur();
	VerticalBlur();
	UpSample();
	
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, baseViewMatrix, orthoMatrix;

	//// Clear the scene. (default blue colour)
	m_Direct3D->BeginScene(0.39f, 0.58f, 0.92f, 1.0f);

	//// Generate the view matrix based on the camera's position.
	m_Camera->Update();

	//// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

	//Generate soft shadows
	//Plane mesh
	m_PlaneMesh->SendData(m_Direct3D->GetDeviceContext());
	m_SoftShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, m_PlaneMesh->GetTexture(), 
	m_UpSampleTexture->GetShaderResourceView(), m_Light, m_Light2, m_Light3, m_Light4);
	m_SoftShader->Render(m_Direct3D->GetDeviceContext(), m_PlaneMesh->GetIndexCount());

	//translate world for Cube
	worldMatrix = XMMatrixTranslation(50.0, 3.0, 50.0);

	//Cube
	m_CubeMesh->SendData(m_Direct3D->GetDeviceContext());
	m_SoftShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, m_CubeMesh->GetTexture(), 
	m_UpSampleTexture->GetShaderResourceView(),	m_Light, m_Light2, m_Light3, m_Light4);
	m_SoftShader->Render(m_Direct3D->GetDeviceContext(), m_CubeMesh->GetIndexCount());

	//translate world for Sphere
	worldMatrix = XMMatrixTranslation(15.0, 2.0, 15.0);

	//Sphere
	m_SphereMesh->SendData(m_Direct3D->GetDeviceContext());
	m_SoftShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, m_SphereMesh->GetTexture(),
	m_UpSampleTexture->GetShaderResourceView(),	m_Light, m_Light2, m_Light3, m_Light4);
	m_SoftShader->Render(m_Direct3D->GetDeviceContext(), m_SphereMesh->GetIndexCount());

	//Move world for tess plane
	worldMatrix = XMMatrixTranslation(0.0, 0.0, 100.0);

	m_TessMesh->SendData(m_Direct3D->GetDeviceContext());
	m_TessShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, m_TessMesh->GetTexture(), tessFactor);
	m_TessShader->Render(m_Direct3D->GetDeviceContext(), m_TessMesh->GetIndexCount());

	//translate world for geometry shader mesh
	worldMatrix = XMMatrixTranslation(0.0, 10.0, 0.0);

	m_PointMesh->SendData(m_Direct3D->GetDeviceContext());
	m_GeometryShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, m_PointMesh->GetTexture());
	m_GeometryShader->Render(m_Direct3D->GetDeviceContext(), m_PointMesh->GetIndexCount());


	//This will render the 
	if (Shadowmap == true)
	{
		m_Direct3D->GetOrthoMatrix(orthoMatrix);
		m_Camera->GetBaseViewMatrix(baseViewMatrix);

		m_Direct3D->TurnZBufferOff();

		m_OrthoMesh->SendData(m_Direct3D->GetDeviceContext());
		m_TextureShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, m_ShadowRenderTex->GetShaderResourceView());
		m_TextureShader->Render(m_Direct3D->GetDeviceContext(), m_OrthoMesh->GetIndexCount());

		m_Direct3D->TurnZBufferOn();
	}

	gui();

	//// Present the rendered scene to the screen.
	m_Direct3D->EndScene();

	return true;
}

void App1::RenderToTexture()
{
	XMMATRIX worldMatrix, lightViewMatrix, lightProjectionMatrix;

	// Set the render target to be the render to texture.
	m_RenderTexture->SetRenderTarget(m_Direct3D->GetDeviceContext());

	// Clear the render to texture.
	m_RenderTexture->ClearRenderTarget(m_Direct3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Update();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Direct3D->GetWorldMatrix(worldMatrix);

	m_Light->GenerateViewMatrix();
	m_Light->GenerateProjectionMatrix(SCREEN_NEAR, SCREEN_DEPTH);
	lightViewMatrix = m_Light->GetViewMatrix();
	lightProjectionMatrix = m_Light->GetProjectionMatrix();
	m_Direct3D->GetWorldMatrix(worldMatrix);


	// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
	//Floor
	m_PlaneMesh->SendData(m_Direct3D->GetDeviceContext());
	m_DepthShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	m_DepthShader->Render(m_Direct3D->GetDeviceContext(), m_PlaneMesh->GetIndexCount());

	worldMatrix = XMMatrixTranslation(50.0, 3.0, 50.0);

	//Cube
	m_CubeMesh->SendData(m_Direct3D->GetDeviceContext());
	m_DepthShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	m_DepthShader->Render(m_Direct3D->GetDeviceContext(), m_CubeMesh->GetIndexCount());

	worldMatrix = XMMatrixTranslation(15.0, 2.0, 15.0);

	//Sphere
	m_SphereMesh->SendData(m_Direct3D->GetDeviceContext());
	m_DepthShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	m_DepthShader->Render(m_Direct3D->GetDeviceContext(), m_SphereMesh->GetIndexCount());


	// Reset the render target back to the original back buffer and not the render to texture anymore.
	m_Direct3D->SetBackBufferRenderTarget();
	m_Direct3D->ResetViewport();

}

void App1::RenderToTexture2()
{
	XMMATRIX worldMatrix, lightViewMatrix, lightProjectionMatrix;

	// Set the render target to be the render to texture.
	m_RenderTexture2->SetRenderTarget(m_Direct3D->GetDeviceContext());

	// Clear the render to texture.
	m_RenderTexture2->ClearRenderTarget(m_Direct3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Update();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Direct3D->GetWorldMatrix(worldMatrix);

	m_Light2->GenerateViewMatrix();
	m_Light2->GenerateProjectionMatrix(SCREEN_NEAR, SCREEN_DEPTH);
	lightViewMatrix = m_Light2->GetViewMatrix();
	lightProjectionMatrix = m_Light2->GetProjectionMatrix();
	m_Direct3D->GetWorldMatrix(worldMatrix);


	// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
	//Floor
	m_PlaneMesh->SendData(m_Direct3D->GetDeviceContext());
	m_DepthShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	m_DepthShader->Render(m_Direct3D->GetDeviceContext(), m_PlaneMesh->GetIndexCount());

	worldMatrix = XMMatrixTranslation(50.0, 3.0, 50.0);

	//Cube
	m_CubeMesh->SendData(m_Direct3D->GetDeviceContext());
	m_DepthShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	m_DepthShader->Render(m_Direct3D->GetDeviceContext(), m_CubeMesh->GetIndexCount());

	worldMatrix = XMMatrixTranslation(15.0, 2.0, 15.0);

	//Sphere
	m_SphereMesh->SendData(m_Direct3D->GetDeviceContext());
	m_DepthShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	m_DepthShader->Render(m_Direct3D->GetDeviceContext(), m_SphereMesh->GetIndexCount());


	// Reset the render target back to the original back buffer and not the render to texture anymore.
	m_Direct3D->SetBackBufferRenderTarget();
	m_Direct3D->ResetViewport();

}

void App1::RenderToTexture3()
{
	XMMATRIX worldMatrix, lightViewMatrix, lightProjectionMatrix;

	// Set the render target to be the render to texture.
	m_RenderTexture3->SetRenderTarget(m_Direct3D->GetDeviceContext());

	// Clear the render to texture.
	m_RenderTexture3->ClearRenderTarget(m_Direct3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Update();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Direct3D->GetWorldMatrix(worldMatrix);

	m_Light3->GenerateViewMatrix();
	m_Light3->GenerateProjectionMatrix(SCREEN_NEAR, SCREEN_DEPTH);
	lightViewMatrix = m_Light3->GetViewMatrix();
	lightProjectionMatrix = m_Light3->GetProjectionMatrix();
	m_Direct3D->GetWorldMatrix(worldMatrix);


	// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
	//Floor
	m_PlaneMesh->SendData(m_Direct3D->GetDeviceContext());
	m_DepthShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	m_DepthShader->Render(m_Direct3D->GetDeviceContext(), m_PlaneMesh->GetIndexCount());

	worldMatrix = XMMatrixTranslation(50.0, 3.0, 50.0);

	//Cube
	m_CubeMesh->SendData(m_Direct3D->GetDeviceContext());
	m_DepthShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	m_DepthShader->Render(m_Direct3D->GetDeviceContext(), m_CubeMesh->GetIndexCount());

	worldMatrix = XMMatrixTranslation(15.0, 2.0, 15.0);

	//Sphere
	m_SphereMesh->SendData(m_Direct3D->GetDeviceContext());
	m_DepthShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	m_DepthShader->Render(m_Direct3D->GetDeviceContext(), m_SphereMesh->GetIndexCount());


	// Reset the render target back to the original back buffer and not the render to texture anymore.
	m_Direct3D->SetBackBufferRenderTarget();
	m_Direct3D->ResetViewport();

}

void App1::RenderToTexture4()
{
	XMMATRIX worldMatrix, lightViewMatrix, lightProjectionMatrix;

	// Set the render target to be the render to texture.
	m_RenderTexture4->SetRenderTarget(m_Direct3D->GetDeviceContext());

	// Clear the render to texture.
	m_RenderTexture4->ClearRenderTarget(m_Direct3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Update();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Direct3D->GetWorldMatrix(worldMatrix);

	m_Light4->GenerateViewMatrix();
	m_Light4->GenerateProjectionMatrix(SCREEN_NEAR, SCREEN_DEPTH);
	lightViewMatrix = m_Light4->GetViewMatrix();
	lightProjectionMatrix = m_Light4->GetProjectionMatrix();
	m_Direct3D->GetWorldMatrix(worldMatrix);


	// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
	//Floor
	m_PlaneMesh->SendData(m_Direct3D->GetDeviceContext());
	m_DepthShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	m_DepthShader->Render(m_Direct3D->GetDeviceContext(), m_PlaneMesh->GetIndexCount());

	worldMatrix = XMMatrixTranslation(50.0, 3.0, 50.0);

	//Cube
	m_CubeMesh->SendData(m_Direct3D->GetDeviceContext());
	m_DepthShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	m_DepthShader->Render(m_Direct3D->GetDeviceContext(), m_CubeMesh->GetIndexCount());

	worldMatrix = XMMatrixTranslation(15.0, 2.0, 15.0);

	//Sphere
	m_SphereMesh->SendData(m_Direct3D->GetDeviceContext());
	m_DepthShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	m_DepthShader->Render(m_Direct3D->GetDeviceContext(), m_SphereMesh->GetIndexCount());


	// Reset the render target back to the original back buffer and not the render to texture anymore.
	m_Direct3D->SetBackBufferRenderTarget();
	m_Direct3D->ResetViewport();

}

void App1::ShadowRender()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

	// Set the render target to be the render to texture.
	m_ShadowRenderTex->SetRenderTarget(m_Direct3D->GetDeviceContext());

	// Clear the render to texture.
	m_ShadowRenderTex->ClearRenderTarget(m_Direct3D->GetDeviceContext(), 0.0f, 0.0f, 1.0f, 1.0f);
	
	//// Generate the view matrix based on the camera's position.
	m_Camera->Update();

	//// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

	//Plane mesh
	m_PlaneMesh->SendData(m_Direct3D->GetDeviceContext());
	m_ShadowShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, m_PlaneMesh->GetTexture(), m_RenderTexture->GetShaderResourceView(), m_RenderTexture2->GetShaderResourceView(), m_RenderTexture3->GetShaderResourceView(), m_RenderTexture4->GetShaderResourceView(),
		m_Light, m_Light2, m_Light3, m_Light4);
	m_ShadowShader->Render(m_Direct3D->GetDeviceContext(), m_PlaneMesh->GetIndexCount());

	//translate world for Cube
	worldMatrix = XMMatrixTranslation(50.0, 3.0, 50.0);

	//Cube
	m_CubeMesh->SendData(m_Direct3D->GetDeviceContext());
	m_ShadowShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, m_CubeMesh->GetTexture(), m_RenderTexture->GetShaderResourceView(), m_RenderTexture2->GetShaderResourceView(), m_RenderTexture3->GetShaderResourceView(), m_RenderTexture4->GetShaderResourceView(),
		m_Light, m_Light2, m_Light3, m_Light4);
	m_ShadowShader->Render(m_Direct3D->GetDeviceContext(), m_CubeMesh->GetIndexCount());

	//translate world for Sphere
	worldMatrix = XMMatrixTranslation(15.0, 2.0, 15.0);

	//Sphere
	m_SphereMesh->SendData(m_Direct3D->GetDeviceContext());
	m_ShadowShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, m_SphereMesh->GetTexture(), m_RenderTexture->GetShaderResourceView(), m_RenderTexture2->GetShaderResourceView(), m_RenderTexture3->GetShaderResourceView(), m_RenderTexture4->GetShaderResourceView(),
		m_Light, m_Light2, m_Light3, m_Light4);
	m_ShadowShader->Render(m_Direct3D->GetDeviceContext(), m_SphereMesh->GetIndexCount());


	// Reset the render target back to the original back buffer and not the render to texture anymore.
	m_Direct3D->SetBackBufferRenderTarget();
	m_Direct3D->ResetViewport();
}

void App1::gui()
{
	// Force turn off on Geometry shader
	m_Direct3D->GetDeviceContext()->GSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("Graphics Coursework %d", 123);
	//ImGui::ColorEdit3("Colour", (float*)&clear_col);
	ImGui::Checkbox("Wireframe", &Wireframe);
	ImGui::SliderFloat("Tess", &tessFactor, 1, 10, "%.0f");
	ImGui::Checkbox("Shadow Map", &Shadowmap);

	// Render UI
	ImGui::Render();
}

void App1::DownSample()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix, baseViewMatrix;

	// Set the render target to be the render to texture.
	m_DownSampleTexture->SetRenderTarget(m_Direct3D->GetDeviceContext());

	// Clear the render to texture.
	m_DownSampleTexture->ClearRenderTarget(m_Direct3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Update();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetBaseViewMatrix(baseViewMatrix);
	//m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetOrthoMatrix(orthoMatrix);
	//m_Direct3D->GetProjectionMatrix(projectionMatrix);

	m_Direct3D->TurnZBufferOff();

	m_smallOrthoMesh->SendData(m_Direct3D->GetDeviceContext());
	m_TextureShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, m_ShadowRenderTex->GetShaderResourceView());
	m_TextureShader->Render(m_Direct3D->GetDeviceContext(), m_smallOrthoMesh->GetIndexCount());

	m_Direct3D->TurnZBufferOn();

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	m_Direct3D->SetBackBufferRenderTarget();
	m_Direct3D->ResetViewport();
}

void App1::HorizontalBlur()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix, baseViewMatrix;

	// Set the render target to be the render to texture.
	m_HorTexture->SetRenderTarget(m_Direct3D->GetDeviceContext());

	// Clear the render to texture.
	m_HorTexture->ClearRenderTarget(m_Direct3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Update();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetBaseViewMatrix(baseViewMatrix);
	//m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetOrthoMatrix(orthoMatrix);
	//m_Direct3D->GetProjectionMatrix(projectionMatrix);

	m_Direct3D->TurnZBufferOff();

	m_smallOrthoMesh->SendData(m_Direct3D->GetDeviceContext());
	m_HorizontalShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, m_DownSampleTexture->GetShaderResourceView(), screenw);
	m_HorizontalShader->Render(m_Direct3D->GetDeviceContext(), m_smallOrthoMesh->GetIndexCount());

	m_Direct3D->TurnZBufferOn();

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	m_Direct3D->SetBackBufferRenderTarget();
	m_Direct3D->ResetViewport();
}

void App1::VerticalBlur()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix, baseViewMatrix;

	// Set the render target to be the render to texture.
	m_VerTexture->SetRenderTarget(m_Direct3D->GetDeviceContext());

	// Clear the render to texture.
	m_VerTexture->ClearRenderTarget(m_Direct3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Update();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetBaseViewMatrix(baseViewMatrix);
	//m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetOrthoMatrix(orthoMatrix);
	//m_Direct3D->GetProjectionMatrix(projectionMatrix);

	m_Direct3D->TurnZBufferOff();

	m_smallOrthoMesh->SendData(m_Direct3D->GetDeviceContext());
	m_VerticalShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, m_HorTexture->GetShaderResourceView(), screenh);
	m_VerticalShader->Render(m_Direct3D->GetDeviceContext(), m_smallOrthoMesh->GetIndexCount());

	m_Direct3D->TurnZBufferOn();

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	m_Direct3D->SetBackBufferRenderTarget();
	m_Direct3D->ResetViewport();
}

void App1::UpSample()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix, baseViewMatrix;

	// Set the render target to be the render to texture.
	m_UpSampleTexture->SetRenderTarget(m_Direct3D->GetDeviceContext());

	// Clear the render to texture.
	m_UpSampleTexture->ClearRenderTarget(m_Direct3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Update();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetBaseViewMatrix(baseViewMatrix);
	m_Direct3D->GetOrthoMatrix(orthoMatrix);
	//m_Camera->GetViewMatrix(viewMatrix);
	//m_Direct3D->GetProjectionMatrix(projectionMatrix);

	m_Direct3D->TurnZBufferOff();

	m_OrthoMesh->SendData(m_Direct3D->GetDeviceContext());
	m_TextureShader->SetShaderParameters(m_Direct3D->GetDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, m_VerTexture->GetShaderResourceView());
	m_TextureShader->Render(m_Direct3D->GetDeviceContext(), m_OrthoMesh->GetIndexCount());

	m_Direct3D->TurnZBufferOn();


	// Reset the render target back to the original back buffer and not the render to texture anymore.
	m_Direct3D->SetBackBufferRenderTarget();
	m_Direct3D->ResetViewport();
}

