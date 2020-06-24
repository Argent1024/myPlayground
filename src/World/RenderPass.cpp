#include "RenderPass.h"
#include "../Utility/MeshReader.h"

namespace Game {

	void DefaultRenderPass::Initialize() 
	{
		// TODO Use only one root signature
		m_rootSignature = std::make_shared<Graphic::RootSignature>();
		m_rootSignature->Initialize();

		m_PSO = std::make_shared<Graphic::DefaultPSO>();
		m_PSO->SetRootSigature(m_rootSignature->GetRootSignature());
		m_PSO->Initialize();

		
		// Create Descriptor Table
		// TODO determine how large the table should be
		const UINT tableSize = 4;
		m_DescriptorTable = new Graphic::DescriptorTable(tableSize);
		m_DescriptorTable->Initialize(Engine::GetInitHeap());

		// Creata CBV 
		const UINT cbvSize = CalculateConstantBufferByteSize(sizeof(CameraBufferData));
		ptrGPUMem gpumem = Engine::MemoryAllocator.CreateCommittedBuffer(cbvSize);
		m_CBV = new Graphic::ConstantBuffer(gpumem, cbvSize, *m_DescriptorTable, 0); // Bind at slot 0
		
		// The other textures needed is binded outside by render engine
	}

	void DefaultRenderPass::PrepareData(Scene& scene)
	{
		Camera& camera = scene.GetMainCamera();
		const Transform& view = camera.GetView();
		const Transform& proj = camera.GetToScreen();
		
		// Need to transpose
		XMStoreFloat4x4(&m_CBVData.projection, XMMatrixTranspose((XMMATRIX)proj));
		XMStoreFloat4x4(&m_CBVData.view, XMMatrixTranspose((XMMATRIX)view));
		
		m_CBV->copyData(&m_CBVData, sizeof(CameraBufferData));
	}

	void DefaultRenderPass::Render(Graphic::CommandList& commandList, Scene& scene) {
		
		commandList.SetDescriptorHeap(*Engine::GetInUseHeap());
		commandList.SetPipelineState(m_PSO);
		commandList.SetGraphicsRootSignature(m_rootSignature);
		

		CD3DX12_GPU_DESCRIPTOR_HANDLE sceneTableHandle = m_DescriptorTable->BindDescriptorTable();
		commandList.SetGraphicsRootDescriptorTable(0, sceneTableHandle);
		
		Camera& camera = scene.GetMainCamera();
		camera.UseCamera(commandList);

		const std::vector<GObject*> objList = scene.GetGameObjects(m_ObjRenderType);
		for (auto const& g_obj : objList)
		{
			g_obj->RecordCommand(commandList);
			g_obj->Draw(commandList);
		}
		
	}

	MixtureRenderPass::MixtureRenderPass(UINT num_texture, const UINT width, const UINT height)
		: m_Viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
		  m_ScissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height))
		 { m_DescriptorTable = new Graphic::DescriptorTable(num_texture); }

	void MixtureRenderPass::Initialize() 
	{
		//m_DescriptorTable = new Graphic::DescriptorTable();
		m_DescriptorTable->Initialize(Engine::GetInitHeap());

		// TODO Use only one root signature
		m_rootSignature = std::make_shared<Graphic::MixRootSignature>(m_DescriptorTable->size());
		m_rootSignature->Initialize();

		m_PSO = std::make_shared<Graphic::MixturePSO>();
		m_PSO->SetRootSigature(m_rootSignature->GetRootSignature());
		m_PSO->Initialize();

		// Create Mesh
		// Normal are useless here
		// Tex Coor map Top left to (0, 0), Top right to (1, 0), Bottom left to (0, 1)
		std::vector<DefaultVertex> triangleVertices =
		{
			{ { 1.0f,  1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f  } },
			{ { 1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
			{ { -1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
			{ { -1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f}, { 0.0f, 1.0f } }
		};
		std::vector<UINT> index_list = { 0, 1, 2, 3, 2, 1 };
		ptrMesh screenMesh = std::make_shared<TriangleMesh>(triangleVertices, index_list);

		m_RenderScreen = new GObject();
		// m_RenderScreen->SetMaterial(m_MixtureTextures);
		m_RenderScreen->SetMesh(screenMesh);
		m_RenderScreen->Initialize();
	}

	void MixtureRenderPass::Render(Graphic::CommandList& commandList, Scene& scene) 
	{
		commandList.SetViewPorts(&m_Viewport);
		commandList.SetScissorRects(&m_ScissorRect);
		commandList.SetPipelineState(m_PSO);
		commandList.SetGraphicsRootSignature(m_rootSignature);
		commandList.SetDescriptorHeap(*Engine::GetInUseHeap());

		CD3DX12_GPU_DESCRIPTOR_HANDLE handle = m_DescriptorTable->BindDescriptorTable();
		commandList.SetGraphicsRootDescriptorTable(1, handle);

		m_RenderScreen->RecordCommand(commandList);
		m_RenderScreen->Draw(commandList);
	}
}