#pragma once
#include "Descriptor.h"
#include "CommandList.h"

#define ptrTexture std::shared_ptr<Graphic::Texture>

namespace Graphic {
	enum TextureType 
	{
		TEXTURE_SRV = 1,	// Create a SRV texture
		TEXTURE_UAV = 2,	// Create a UAV texture
		TEXTURE_CBV = 4,		
		TEXTURE_DSV = 8,	// Create a DSV texture
		TEXTURE_RTV = 16
	};


	void LoadChessBoard(const UINT width, const UINT height, const UINT pixelSize, std::vector<UINT8>& data);
	

	// Texture
	// Texture class implement this base class should 
	//		1. Allocate memory from GPU mem manager
	//		2. initialize views (multiple views using the same gpu memory)
	//		3. Load data from file and copy(upload) it to the GPU memory
	class Texture {
	public:
		Texture(UINT type) : m_Type(type) {}

		// Copy (one)view to in use descriptor heap
		inline void BindTexture(UINT index, TextureType type=TEXTURE_SRV) const
		{
			assert(type & m_Type == 1 && "Binding view type not created");
			switch (type)
			{
			case Graphic::TEXTURE_SRV:
				m_SRV->BindDescriptor(index);
				break;
			case Graphic::TEXTURE_UAV:
				m_UAV->BindDescriptor(index);
				break;
			case Graphic::TEXTURE_CBV:
				
				break;
			case Graphic::TEXTURE_DSV:

				break;
			case Graphic::TEXTURE_RTV:

				break;
			default:
				break;
			}
		}

		inline ShaderResource* GetShaderResourceView() const 
		{ 
			assert(m_Type & TEXTURE_SRV && "SRV not created for this texture");
			return m_SRV; 
		}
		
		inline UnorderedAccess* GetUnorderedAccessView() const 
		{ 
			assert(m_Type & TEXTURE_UAV && "DSV not created for this texture");
			return m_UAV; 
		}

		inline DepthStencil* GetDepthStencilView() const 
		{
			assert(m_Type & TEXTURE_DSV && "DSV not created for this texture");
			return m_DSV; 
		}

		inline RenderTarget* GetRenderTargetView() const
		{
			assert(m_Type & TEXTURE_RTV && "RTV not created for this texture");
			return m_RTV; 
		}

		// Write texture data to gpu memory
		// Only need to upload once since SRV, UAV will point to the same memory!
		inline void UploadTexture(D3D12_SUBRESOURCE_DATA& data) {
			assert(m_Type & TEXTURE_SRV  && "The Texture type doesn't contain SRV");
			m_SRV->CopyTexture(&data); 
		}

		// Create 1d texture data
		template <class T>
		static D3D12_SUBRESOURCE_DATA CreateTextureData(const std::vector<T>&  data) {
			D3D12_SUBRESOURCE_DATA textureData = {};
			textureData.pData = &data[0];
			textureData.RowPitch = data.size() * sizeof(T);
			textureData.SlicePitch = data.size() * sizeof(T);
			return textureData;
		}
		
		// TODO Template
		// Create 2d texture data
		static D3D12_SUBRESOURCE_DATA CreateTextureData(const UINT width, const UINT height, const UINT pixelSize, 
												 const std::vector<UINT8>& data) 
		{
			D3D12_SUBRESOURCE_DATA textureData = {};
			// Set infomation to m_textureData
			textureData.pData = &data[0];
			textureData.RowPitch = width * pixelSize;
			textureData.SlicePitch = textureData.RowPitch * height;
			return textureData;
		}

	protected:
		virtual void CreateSRV() = 0;
		virtual void CreateUAV() = 0;
		virtual void CreateDSV() = 0;
		virtual void CreateRTV() = 0;
		// TODO RTV CBV

		inline  void CreateViews()
		{
			if (m_Type & TEXTURE_SRV) {
				CreateSRV();
			} 
			if (m_Type & TEXTURE_UAV) {
				CreateUAV();
			}
			if (m_Type & TEXTURE_CBV) {

			}
			if (m_Type & TEXTURE_DSV) {
				CreateDSV();
			}
			if (m_Type & TEXTURE_RTV) {
				CreateRTV();
			}
		}

		ptrGPUMem m_gpuMem;

		// Store or value for texture_type
		// EX: m_Type = TEXUTRE_SRV | TEXTURE_UAV | TEXTURE_DSV
		const UINT m_Type;

		ShaderResource* m_SRV;
		UnorderedAccess* m_UAV;
		DepthStencil* m_DSV;
		RenderTarget* m_RTV;

		D3D12_RESOURCE_DESC m_textureDesc;
	};


	// Texture 
	class TextureBuffer : public Texture {
	public:
		TextureBuffer(UINT elementSize, UINT stride, TextureType type=TEXTURE_SRV);
	private:
		void CreateSRV() override;
		void CreateUAV() override;
		void CreateDSV() override;
		void CreateRTV() override;
		UINT m_size;
		UINT m_stride;
	};


	class Texture2D : public Texture {
	public:
		Texture2D(UINT width, UINT height, TextureType type=TEXTURE_SRV, const std::wstring& textureFile=L"");
	private:
		void CreateSRV() override;
		void CreateUAV() override;
		void CreateDSV() override;
		void CreateRTV() override;
	};


}