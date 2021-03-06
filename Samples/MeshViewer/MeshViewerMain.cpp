#pragma once

#include "stdafx.h"
#include "WinApp.h"

#include "World/Engine.h"
#include "Utility/MeshReader.h"

#include <iostream>


namespace Engine {
	class MeshViewerEG : public GameEngine {
	public:
		void LoadAssert() override;

	private:
		// TODO Resource Manager
		std::shared_ptr<Game::Mesh> m_Mesh;
		std::shared_ptr<Game::PrincipleMaterial> m_Material;
		std::shared_ptr<Engine::UiEngine> m_UIengine;
	};

	class MeshViewerUI : public UiEngine {
	public:
		MeshViewerUI(std::shared_ptr<Game::PrincipleMaterial> mat) : m_Material(mat) {
			
		}

		void UpdateUIWindows() override {
			UI::PrincipleMaterialUIWindow(m_Material, "Material");
		}

	private:
		std::shared_ptr<Game::PrincipleMaterial> m_Material;

	};


	void MeshViewerEG::LoadAssert() {
		m_Scene = new Scene(m_Setting.Width, m_Setting.Height);
		m_Scene->Initialize();

		// Load asserts
		{
			m_Scene->ConfigLight(1, 4, 4);
			// Load light
			DirectionLight* m_Light = new DirectionLight();
			m_Scene->AddLight(*m_Light);
			m_Light->SetLightData({ {1.0, 1.0, 1.0}, {0.0 , 10.0, 0.0}, {1.0, 1.0, 2.5} });
			m_Material = std::make_shared<PrincipleMaterial>();

			// Create UI for the material
			m_UIengine = std::make_shared<MeshViewerUI>(m_Material);
			m_RenderEngine->GetUIEngine() = m_UIengine;


			// Normal mesh stuff
			PrincipleMaterial::MatData mat;
			DirectX::XMStoreFloat3(&mat.BaseColor, Vector3(1.0, 0.0, 0.0));
			mat.Roughness = 0.15f;
			mat.Specular = 0.0f;
			m_Material->SetData(mat);

			// Load textures
			{
				std::string texpath = "D://work/tEngine/envmap.png";

				auto textype = Graphic::TEXTURE_SRV | Graphic::TEXTURE_UAV;
				ptrTex2D baseColorTex = std::make_shared<Graphic::Texture2D>(texpath, textype, 5);

				Graphic::Texture2D::CreateMipMap(baseColorTex, 0, 4);
				m_Material->SetTexture(PrincipleMaterial::DiffuseTex, baseColorTex);
			}

			m_Material->UploadCBV();

			m_Mesh = TriangleMesh::GetXYPlane();
			//{
			//	MeshReader reader;
			//	reader.ReadOBJ("D://work/tEngine/renderball.obj");

			//	std::vector<DefaultVertex>& vertex = reader.m_vertex;
			//	std::vector<UINT>& index = reader.m_index;

			//	m_Mesh = std::make_shared<TriangleMesh>(vertex, index);
			//}

			// TODO error C2338 aligin
			GObject* obj0 = new Game::GObject();
			const float scale = 5.f;
			obj0->SetMesh(m_Mesh);
			obj0->SetMaterial(m_Material);
			obj0->SetTransform(Transform({ scale, 0, 0 }, { 0, scale, 0 }, { 0, 0, scale }, { 0.0, 0.0, 0.0 }));

			m_Scene->AddGameObj(obj0);
		}
	}

}

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{

	Engine::GameEngine* engine = new Engine::MeshViewerEG();
	return WinApp::RunApp(engine, hInstance, nCmdShow);
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
