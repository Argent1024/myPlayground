#include "Descriptor.h"

namespace Graphic {
	VertexBuffer::VertexBuffer(ptrGBuffer gpubuffer, const UINT bufferSize, const UINT strideSize) 
		:BufferDescriptor(gpubuffer, bufferSize), m_strideSize(strideSize) 
	{ 
		m_Offset = m_Buffer->MemAlloc(m_BufferSize);
		m_view.BufferLocation = m_Buffer->GetGPUAddr() + m_Offset;
		m_view.StrideInBytes = m_strideSize;
		m_view.SizeInBytes = m_BufferSize;
	}


	IndexBuffer::IndexBuffer(ptrGBuffer gpubuffer, const UINT bufferSize)
		: BufferDescriptor(gpubuffer, bufferSize), m_start(0) 
	{
		m_Offset = m_Buffer->MemAlloc(m_BufferSize);
		m_view.BufferLocation = m_Buffer->GetGPUAddr() + m_Offset;
		m_view.Format = DXGI_FORMAT_R32_UINT;
		m_view.SizeInBytes = m_BufferSize;
	}

	/*// TODO check whether buffersize & start is correct
	IndexBuffer::IndexBuffer(IndexBuffer& buffer, const UINT start, const UINT end)
		: Descriptor(buffer.m_Buffer, (end - start) * sizeof(UINT)), m_start(start) // bufferSize = (end - start) * sizeof(UINT)
	{
		assert(start + m_BufferSize <= buffer.m_BufferSize && "start position plus bufferSize too large");
		m_Offset = buffer.m_Offset + start;
		m_view.BufferLocation = m_Buffer->GetGPUAddr() + m_Offset;
		m_view.Format = DXGI_FORMAT_R32_UINT;
		m_view.SizeInBytes = m_BufferSize;
	}*/

	ConstantBuffer::ConstantBuffer(ptrGBuffer gpubuffer, const UINT bufferSize)
		: BufferDescriptor(gpubuffer, CalculateConstantBufferByteSize(bufferSize)),
		  m_RootHeapIndex(-1)
	{
		assert(m_BufferSize % 256 == 0 && "Constant buffer size not aligned");
		m_Offset = m_Buffer->MemAlloc(m_BufferSize);
		m_cbvDesc.BufferLocation = m_Buffer->GetGPUAddr() + m_Offset;
		m_cbvDesc.SizeInBytes = m_BufferSize;  
	}	

	void ConstantBuffer::CreateView(DescriptorTable& table, UINT slot) 
	{
		ID3D12Device* device = Engine::GetDevice();
		D3D12_CPU_DESCRIPTOR_HANDLE handle = table.GetSlot(slot);
		device->CreateConstantBufferView(&m_cbvDesc, handle);
	}

	void ConstantBuffer::CreateRootView() 
	{
		assert(m_RootHeapIndex==-1 && "Root View should be created only once");
		ID3D12Device* device = Engine::GetDevice();
		DescriptorHeap* initheap = Engine::GetInitHeap();
		m_RootHeapIndex = initheap->MallocHeap();
		device->CreateConstantBufferView(&m_cbvDesc, initheap->GetCPUHandle(m_RootHeapIndex));
	}

	/************************* Shader Resource Begin ******************************/
	ShaderResource::ShaderResource(ptrTBuffer gpubuffer, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc) 
		: m_srvDesc(desc), HeapDescriptor(gpubuffer)
	{ }


	void ShaderResource::CreateView(DescriptorTable& table, UINT slot) 
	{
		ID3D12Device* device = Engine::GetDevice();
		D3D12_CPU_DESCRIPTOR_HANDLE handle = table.GetSlot(slot);
		device->CreateShaderResourceView(m_Buffer->GetResource(), &m_srvDesc, handle);
	}

	void ShaderResource::CreateRootView() 
	{
		assert(m_RootHeapIndex==-1 && "Root View should be created only once");
		ID3D12Device* device = Engine::GetDevice();
		DescriptorHeap* initheap = Engine::GetInitHeap();
		m_RootHeapIndex = initheap->MallocHeap();
		D3D12_CPU_DESCRIPTOR_HANDLE handle = initheap->GetCPUHandle(m_RootHeapIndex);
		device->CreateShaderResourceView(m_Buffer->GetResource(), &m_srvDesc, handle);
	}
	/************************* Shader Resource End ******************************/


	/*************************   UnorderedAccess Resource Begin ******************************/
	UnorderedAccess::UnorderedAccess(ptrTBuffer gpubuffer, const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc)
		: m_uavDesc(desc), HeapDescriptor(gpubuffer) 
	{ }

	void UnorderedAccess::CreateView(DescriptorTable& table, UINT slot) 
	{
		ID3D12Device* device = Engine::GetDevice();
		D3D12_CPU_DESCRIPTOR_HANDLE handle = table.GetSlot(slot);
		device->CreateUnorderedAccessView(m_Buffer->GetResource(), nullptr, &m_uavDesc, handle);
	}

	void UnorderedAccess::CreateRootView()
	{
		assert(m_RootHeapIndex==-1 && "Root View should be created only once");
		ID3D12Device* device = Engine::GetDevice();
		DescriptorHeap* initheap = Engine::GetInitHeap();
		m_RootHeapIndex = initheap->MallocHeap();
		D3D12_CPU_DESCRIPTOR_HANDLE handle = initheap->GetCPUHandle(m_RootHeapIndex);
		device->CreateUnorderedAccessView(m_Buffer->GetResource(), nullptr, &m_uavDesc, handle);
	}

	/************************* UnorderedAccess Resource End ******************************/


	RenderTarget::RenderTarget(ptrTBuffer gpubuffer, const D3D12_RENDER_TARGET_VIEW_DESC& desc, DescriptorHeap* descriptorHeap) 
		: m_rtvDesc(desc), m_DescriptorHeap(descriptorHeap), HeapDescriptor(gpubuffer) 
	{	}


	void RenderTarget::CreateRootView() 
	{
		assert(m_RootHeapIndex==-1 && "Root View should be created only once");
		ID3D12Device* device = Engine::GetDevice();
		m_RootHeapIndex = m_DescriptorHeap->MallocHeap();
		device->CreateRenderTargetView(m_Buffer->GetResource(), &m_rtvDesc, m_DescriptorHeap->GetCPUHandle(m_RootHeapIndex));
	}


	DepthStencil::DepthStencil(ptrTBuffer gpubuffer, const D3D12_DEPTH_STENCIL_VIEW_DESC & desc, DescriptorHeap* descriptorHeap)
		: m_dsvDesc(desc), m_DescriptorHeap(descriptorHeap), HeapDescriptor(gpubuffer)
	{	}

	
	void DepthStencil::CreateRootView() 
	{
		assert(m_RootHeapIndex==-1 && "Root View should be created only once");
		ID3D12Device* device = Engine::GetDevice();
		m_RootHeapIndex = m_DescriptorHeap->MallocHeap();
		device->CreateDepthStencilView(m_Buffer->GetResource(), &m_dsvDesc, m_DescriptorHeap->GetCPUHandle(m_RootHeapIndex));
		m_dsvHandle = m_DescriptorHeap->GetCPUHandle(m_RootHeapIndex);
	}
}