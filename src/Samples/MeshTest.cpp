#include"MeshTest.h"
#include"../Utility/MeshReader.h"

namespace Samples {

	void MeshTest::Init(const HWND m_appHwnd)
	{
		Engine::EnableDebug();
		Engine::CreateDevice();

		CopyHelper.Initialize();
		GraphicsCommandManager.Initialize();

		DescriptorHeap* initHeap = Engine::GetInitHeap();
		initHeap->Initialize();
		DescriptorHeap* useHeap = Engine::GetInUseHeap();
		useHeap->Initialize();

		m_rootSignature = std::make_shared<RootSignature>();

		m_GraphicPSO = std::make_shared<MeshTestPSO>();
		m_GraphicPSO->SetRootSigature(m_rootSignature->GetRootSignature());
		m_GraphicPSO->Initialize();

		// m_Camera.CreateCBV();
		// Create Scene
		m_Scene = new Scene(m_appHwnd, m_width, m_height);

		this->LoadAssert();

	}

	void MeshTest::LoadAssert() 
	{
		MeshReader reader;
		reader.ReadOBJ("D:\\work\\tEngine\\bunny.obj");

		std::vector<DefaultVertex>& vertex = reader.m_vertex;
		std::vector<UINT>& index = reader.m_index;

		m_Mesh = std::make_shared<TriangleMesh>(vertex, index);
		m_Material = std::make_shared<NoMaterial>(m_GraphicPSO, m_rootSignature);
		
		// TODO!! error C2338 aligin?
		GObject* obj0 = new Game::GObject();
		obj0->SetMesh(m_Mesh);
		obj0->SetMaterial(m_Material);

		m_Scene->AddGameObj(obj0);
	}

	void MeshTest::Render()
	{
		m_Scene->Render();
	}
}