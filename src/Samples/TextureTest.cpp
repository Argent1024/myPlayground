#include "TextureTest.h"

namespace Samples {

	void TextureTestSample::Init(const HWND m_appHwnd) 
	{
		Engine::EnableDebug();
		Engine::CreateDevice();
		
		CopyHelper.Initialize();
		GraphicsCommandManager.Initialize();

		DescriptorHeap* initHeap = Engine::GetInitHeap();
		initHeap->Initialize();
		DescriptorHeap* useHeap = Engine::GetInUseHeap();
		useHeap->Initialize();

		// Swap Chain
		m_swapChain = new SwapChain(m_appHwnd, m_width, m_height);
		m_swapChain->Initialize(GraphicsCommandManager.GetCommadnQueue());

		m_rootSignature = std::make_shared<RootSignature>();
		m_rootSignature->Initialize();
	
		m_GraphicPSO = std::make_shared<TextureTestPSO>();
		m_GraphicPSO->SetRootSigature(m_rootSignature->GetRootSignature());
		m_GraphicPSO->Initialize();	
		
		// Create Assert
		
		struct TextureVertex
		{
			XMFLOAT3 position;
			XMFLOAT2 texcoor;
		};
		// Define the geometry for a triangle.
		std::vector<TextureVertex> triangleVertices =
		{
			{ {  1.0f,  1.0f, -1.0f }, { 1.0f, 1.0f } },
			{ {  1.0f, -1.0f, -1.0f }, { 1.0f, 0.0f } },
			{ { -1.0f,  1.0f, -1.0f }, { 0.0f, 1.0f } },
			{ { -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f } }
		};
		std::vector<UINT> index_list = { 0, 1, 2, 3, 2, 1 };
		
		m_Mesh =std::make_shared<TriangleMesh>(triangleVertices, index_list);
		
		// Create Texture
		m_texture = std::make_shared<Texture2D>(256, 256);

		m_Material = std::make_shared<TextureMaterial>(m_GraphicPSO, m_rootSignature, m_texture);

		m_textureObject.SetMesh(m_Mesh);
		m_textureObject.SetMaterial(m_Material);
	}

	void TextureTestSample::Render() 
	{	

		//m_Camera.LookAt(Vector3(0.,0.,0.), Vector3(0., 0., 0.), Vector3(0., 1., 0.));
		
		GraphicsCommandManager.Start();
		DescriptorHeap* UseHeap = Engine::GetInUseHeap();
		UseHeap->Reset();

		CommandList mainCommandList;
		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		GraphicsCommandManager.InitCommandList(&mainCommandList);
		mainCommandList.SetSwapChain(*m_swapChain);
		mainCommandList.ResourceBarrier(*m_swapChain, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		mainCommandList.ClearSwapChain(*m_swapChain, clearColor);
		mainCommandList.ResourceBarrier(*m_swapChain, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		GraphicsCommandManager.ExecuteCommandList(&mainCommandList);

		// Record commands for each object, only have one object here
		{
			CommandList ThreadCommandList;
			GraphicsCommandManager.InitCommandList(&ThreadCommandList);
			
			m_textureObject.RecordCommand(ThreadCommandList);
			m_Camera.UseCamera(ThreadCommandList);
			

			// Barrier Draw
			ThreadCommandList.ResourceBarrier(*m_swapChain, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			
			ThreadCommandList.SetSwapChain(*m_swapChain);
			m_textureObject.Draw(ThreadCommandList);

			ThreadCommandList.ResourceBarrier(*m_swapChain, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

			GraphicsCommandManager.ExecuteCommandList(&ThreadCommandList);
		}
		
		// Wait Finish
		GraphicsCommandManager.End();
		m_swapChain->Present();

	}

}