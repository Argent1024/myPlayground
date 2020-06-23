#pragma once

#include "MathCommon.h"
#include "Transform.h"
#include "Descriptor.h"

#include "CommandList.h"

namespace Game {
	using namespace Math;
	// ***IMPORTANT***
	// In DirectX's cpu code, vector's dimension is 1 * 4.
	// So a matrix(transform) M mutiply(apply to) a vector v should be written as v * M
	class Camera {
	public:
		Camera(const UINT width, const UINT height) :
			 m_aspectRatio(float(width) / (float)height),
			 m_Viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
			 m_ScissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height))
		{ }

		// TODO confusing name
		virtual void UseCamera(Graphic::CommandList& commandList)
		{
			commandList.SetViewPorts(&m_Viewport);
			commandList.SetScissorRects(&m_ScissorRect);
		}

		virtual void LookAt(const Vector3& Position, const Vector3& Target, const Vector3& WorldUp) = 0;

		inline const Transform& GetView() const { return m_View; }
		inline const Transform& GetToScreen() const { return m_ToScreen; }
	

		void SetViewPort(CD3DX12_VIEWPORT newViewPort) { m_Viewport = newViewPort; }
		void SetScissorRect(CD3DX12_RECT newScissorRect) { m_ScissorRect = newScissorRect; } 

	protected:
		Transform m_View;
		Transform m_ToScreen; // Projective or Orthnomal

		float m_aspectRatio;
		CD3DX12_VIEWPORT m_Viewport;
		CD3DX12_RECT m_ScissorRect;
	};

	
	class ProjectiveCamera : public Camera { 
	public:
		ProjectiveCamera(const UINT width, const UINT height)
			: Camera(width, height) {}

		ProjectiveCamera(const UINT width, const UINT height, 
			const Vector3& Position, const Vector3& Target, const Vector3& WorldUp)
			: Camera(width, height)
		{ LookAt(Position, Target, WorldUp); }

		inline void LookAt(const Vector3& Position, const Vector3& Target, const Vector3& WorldUp)
		{
			m_View = Transform(Matrix4(DirectX::XMMatrixLookAtRH(Position, Target, WorldUp)));		
			m_ToScreen = Transform(Matrix4(DirectX::XMMatrixPerspectiveFovRH(3.14f / 1.5f, m_aspectRatio, 0.5f, 1000.0f)));			
		}
	};

	class OrthonormalCamera : public Camera {
	public:
		OrthonormalCamera(const UINT width, const UINT height)
			: Camera(width, height) {}

		OrthonormalCamera(const UINT width, const UINT height, 
			const Vector3& Position, const Vector3& Target, const Vector3& WorldUp)
			: Camera(width, height)
		{ LookAt(Position, Target, WorldUp); }

		inline void LookAt(const Vector3& Position, const Vector3& Target, const Vector3& WorldUp)
		{
			m_View = Transform(Matrix4(DirectX::XMMatrixLookAtRH(Position, Target, WorldUp)));		
			m_ToScreen = Transform(Matrix4(DirectX::XMMatrixOrthographicRH(3.14f / 1.5f, m_aspectRatio, 1.0f, 1000.0f)));
		}
	};
}

