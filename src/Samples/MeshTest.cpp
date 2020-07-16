#include"MeshTest.h"
#include"../Utility/MeshReader.h"
#include "Utility/input.h"
#include "Utility/logger.h"

namespace Samples {

	void MeshTest::Init(const HWND m_appHwnd)
	{
		Engine::InitializeEngine();
		Engine::InitializeInputManager(m_appHwnd);

		/*m_rootSignature = std::make_shared<RootSignature>();
		m_rootSignature->Initialize();

		m_GraphicPSO = std::make_shared<MeshTestPSO>();
		m_GraphicPSO->SetRootSigature(m_rootSignature->GetRootSignature());
		m_GraphicPSO->Initialize();*/

		// m_Camera.CreateCBV();
		// Create Scene
		
		m_Scene.Initialize();

		m_Render = new RenderEngine(m_width, m_height);
		m_Render->Initialize(m_appHwnd);

		m_Scene.ConfigLight(1, 4, 4);
		// Load light
		m_Light = new DirectionLight();
		m_Scene.AddLight(*m_Light);
		m_Light->SetLightData({ {1.0, 1.0, 1.0}, {0.0 , 10.0, 0.0}, {0.0, 3.0, 0.0} });
		this->LoadAssert();

	}

	void MeshTest::LoadAssert() 
	{
		MeshReader reader;
		reader.ReadOBJ("D:\\work\\tEngine\\bunny.obj");

		std::vector<DefaultVertex>& vertex = reader.m_vertex;
		std::vector<UINT>& index = reader.m_index;

		m_Mesh = std::make_shared<TriangleMesh>(vertex, index);
		m_Material = std::make_shared<SimpleMaterial>(Vector3{0.75f, 0.75f, 0.9f});
		m_Material->Initialize();

		// TODO!! error C2338 aligin?
		obj0 = new Game::GObject();
		obj0->SetMesh(m_Mesh);
		 //obj0->SetMesh(plane);
		obj0->SetMaterial(m_Material);
		obj0->Initialize();
		obj0->SetTransform(Transform({ 1.0, 0, 0 }, { 0, 1.0, 0 }, { 0, 0, 1.0 }, {0.0, 0.0, 0.0}));

		m_Scene.AddGameObj(obj0);
		
	}

	void MeshTest::Render()
	{
		// m_Scene->Render();
		m_Render->Render(m_Scene);
	}

	void MeshTest::Update() {
		InputManager& input = Engine::GetInputManager();

		auto kb = input.GetKeyboardState();
		auto mouse = input.GetMouseState();
		auto tracker = input.GetTracker();
		
		// TODO move debug view into render engine later 
		// Press f1 to show depth texture
		if (tracker.IsKeyPressed(Keyboard::F1)) {
			RenderEngine::Config& renderSetting = m_Render->GetRenderSetting();
			renderSetting.mixpass = !renderSetting.mixpass;
		}

		// press f2 to show normal
		if (tracker.IsKeyPressed(Keyboard::F2)) {

			RenderEngine::Config& renderSetting = m_Render->GetRenderSetting();

			DefaultRenderPass& pass = m_Render->GetDefaultPass();
			DefaultRenderPass::ConstBufferData& data = pass.GetCBVData();
			
			renderSetting.mixpass = false;
			data.debugnormal = true;
			data.debugpos = false;
			
		}

		// press f3 to show normal
		if (tracker.IsKeyPressed(Keyboard::F3)) {

			RenderEngine::Config& renderSetting = m_Render->GetRenderSetting();

			DefaultRenderPass& pass = m_Render->GetDefaultPass();
			DefaultRenderPass::ConstBufferData& data = pass.GetCBVData();
			
			
			renderSetting.mixpass = false;
			data.debugnormal = false;
			data.debugpos = true;
		}

		frame ++;
		if (kb.E) {
			Matrix4 t = obj0->GetTransform();
			Matrix4 r = Matrix4(XMMatrixRotationAxis({ 0, 1, 0 }, 0.01));
			t = t * r;
			obj0->SetTransform(Transform(t));
		}

		if (kb.Q) {
			Matrix4 t = obj0->GetTransform();
			Matrix4 r = Matrix4(XMMatrixRotationAxis({ 0, 1, 0 }, -0.01));
			t = t * r;
			obj0->SetTransform(Transform(t));
		}

		if (kb.W) {
			Matrix4 t = obj0->GetTransform();
			Matrix4 r = Matrix4(XMMatrixRotationAxis({ 1, 0, 0 }, 0.01));
			t = t * r;
			obj0->SetTransform(Transform(t));
		}

		if (kb.S) {
			Matrix4 t = obj0->GetTransform();
			Matrix4 r = Matrix4(XMMatrixRotationAxis({ 1, 0, 0 }, -0.01));
			t = t * r;
			obj0->SetTransform(Transform(t));
		}

		if (tracker.IsKeyPressed(Keyboard::L)) {
			Camera& camera = m_Scene.GetMainCamera();
			const Transform& T = obj0->GetTransform();
			const Transform& view = camera.GetView();
			const Transform& proj = camera.GetToScreen();
			Vector3 test(-1.0, -1.0, 0.0);
			test = T * test;
			Logger::Log(test, "World Space");
			test = view * test;
			Logger::Log(test, " Camera Space:");
			test = proj * test;
			Logger::Log(test, " Screen Space:");
			Logger::Log("");
		}
	
		input.UpdateTracker();
	}

}