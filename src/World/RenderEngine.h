#pragma once
#include "RenderPass.h"
#include "SimpleUI.h"

namespace Engine {

	using ptrRenderPass = std::shared_ptr<Game::RenderPass>;
	using RenderPassesTable = std::vector<std::pair<ptrRenderPass, ptrGraphicsPSO>>;

	// The main renderer use(called) by the engine
	class RenderEngine {

	public:

		struct RenderConfig
		{
			
		};


		static RenderPassesTable CreateDefaultRenderPasses() {
			ptrRenderPass defaultRP = std::make_shared<Game::DefaultRenderPass>();
			ptrGraphicsPSO pso = std::make_shared<Graphic::DefaultPSO>();
			return { {defaultRP, pso} };
		}

		RenderEngine(const UINT width, const UINT height);

		void Initialize(const HWND appHwnd) {
			RenderPassesTable passes = CreateDefaultRenderPasses();
			Initialize(appHwnd, passes);
		}

		void Initialize(const HWND appHwnd, RenderPassesTable& passes);

		void Render(Game::Scene& scene);

		
		const RenderConfig& GetRenderSetting() const { return m_Setting; }
		RenderConfig& GetRenderSetting() { return m_Setting; }

		const ptrUiEngine& GetUIEngine() const { return m_UiEngine; }
		ptrUiEngine& GetUIEngine() { return m_UiEngine; }

	protected:
		// Bind texture to some render pass maybe
		virtual void PostInitialize() { };

		RenderConfig m_Setting;
		std::vector<ptrRenderPass> m_RenderPasses;
		std::vector<bool> m_EnabledPasses;


		ptrUiEngine m_UiEngine;

	private:

		void BeginRender();
		void EndRender();

		// Main Swap Chain for the Engine
		Graphic::SwapChain swapChain;
		Graphic::DepthBuffer depthBuffer;

	};
}