#include "stdafx.h"
#include "FFTOcean.h"


void OceanPSO::Initialize() 
{
	assert(m_rootSignature != nullptr);
	m_psoDesc.pRootSignature = m_rootSignature;

	ComPtr<ID3DBlob> VS;
	ComPtr<ID3DBlob> PS;

	const std::wstring path = L"D:\\work\\tEngine\\Shaders\\Ocean.hlsl";

	ThrowIfFailed(D3DCompileFromFile(path.c_str(), nullptr, nullptr, "VSMain", "vs_5_1", CompileFlags, 0, &VS, nullptr));
	ThrowIfFailed(D3DCompileFromFile(path.c_str(), nullptr, nullptr, "PSMain", "ps_5_1", CompileFlags, 0, &PS, nullptr));

	D3D12_INPUT_ELEMENT_DESC  inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_UINT,     0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT,    0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	this->SetVertexShader(CD3DX12_SHADER_BYTECODE(VS.Get()));
	this->SetPixelShader(CD3DX12_SHADER_BYTECODE(PS.Get()));
	this->SetTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	this->SetInoutLayout(_countof(inputElementDescs), inputElementDescs);

	// configrations
	CD3DX12_DEPTH_STENCIL_DESC depthStencilDesc(D3D12_DEFAULT);
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	this->SetDepthStencilState(depthStencilDesc);

	this->SetBlendState();

	CD3DX12_RASTERIZER_DESC raseterDesc(D3D12_DEFAULT);
	raseterDesc.CullMode = D3D12_CULL_MODE_NONE;
	this->SetRasterState(raseterDesc);

	this->SetStuffThatIdontKnowYet();
	this->CreatePSO();
}

void FFTOcean::InitOceanMesh() 
{
	struct VertexData 
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMUINT2 texcoor;
		DirectX::XMFLOAT2 fuv;
	};

	UINT vertexX = m_ResX ;
	UINT vertexY = m_ResY ;
	UINT vertexSize = vertexX * vertexY;

	std::vector<VertexData> vertices(vertexSize);
	std::vector<UINT> index;

	const float dx = 2.0f / vertexX;

	for (UINT x = 0; x < vertexX; ++x) {
		for (UINT y = 0; y < vertexY; ++y) {
			int i = x * vertexY + y;
			VertexData& v = vertices[i];
			v.position = DirectX::XMFLOAT3(dx*x - 1.0f, dx*y - 1.0f, 0.0f);
			v.texcoor = DirectX::XMUINT2(x, y);
			v.fuv = DirectX::XMFLOAT2(float(x) / (vertexX - 1), float(y) / (vertexY - 1));
		}
	}

	for (UINT x = 0; x < (vertexX - 1); ++x) {
		for (UINT y = 0; y < (vertexY - 1); ++y) {
			UINT a = x * vertexY + y;
			UINT b = (x+1) * vertexY + y;
			UINT c = x * vertexY + (y+1);
			UINT d = (x+1) * vertexY + (y+1);
			index.push_back(a);
			index.push_back(b);
			index.push_back(c);

			index.push_back(c);
			index.push_back(d);
			index.push_back(b);
		}
	}

	m_Mesh = std::make_shared<TriangleMesh>(vertices, index);
}

void FFTOcean::AmplitedeUpdate() {
	// Calculate amplitede for every wave
	for (UINT n = 0; n < m_ResX; ++n) {
		for (UINT m = 0; m < m_ResY; ++m) {
			Complex h = Amplitede(n, m);
			m_coeff[n][m] = h;

			// Test
			int index = n * m_ResY + m;
			m_Displacement[index] = Vector3(10.0 * h.real(), 10.0 * h.imag(), 0.0);
		}
	}
}


std::vector<Complex> StupidFT(const std::vector<Complex>& coeff) {
	std::vector<Complex> ans;
	Complex w0 = std::exp(Complex());
	

	for (size_t n = 0; n < coeff.size(); ++n) {
		Complex wn = std::exp(Complex(0, 2 * PI * n/ coeff.size()));
		Complex a = 0.0;
		Complex w = Complex(1.0, 0.0);
		for (int u = 0; u <coeff.size(); ++u) {
			double sign = 1 ? -1 : (n + u) % 2 == 0;
			a += sign * w * coeff[u];
			w *= wn;
		}
		ans.push_back(a);
	}
	return ans;
}

void FFTOcean::HeightUpdate() {
	// Solve 2d fft for height
	for (UINT n = 0; n < m_ResX; ++n) {
		std::vector<Complex> An = FFT(m_coeff[n]);
		//std::vector<Complex> test = StupidFT(m_coeff[n]);

		for (int i = 0; i < An.size(); ++i) {
			//std::cout<<An[i]-test[i]<<std::endl;
			m_A[i][n] = An[i];
		}
	}

	for (UINT m = 0; m < m_ResY; ++m) {
		std::vector<Complex> hm = FFT(m_A[m]);
		for (int n = 0; n < hm.size(); ++n) {
			float h = hm[n].real();
			int index = n * m_ResY + m;
			m_Displacement[index].SetZ(h);
		}
	}
}


void FFTOcean::ShiftUpdate() {
	for (UINT n = 0; n < m_ResX; ++n) {
		for (int m = 0; m < m_ResY; ++m) {
			Complex h = Amplitede(n, m);
			Vector2 k = Normalize(WaveK(n, m));
			Complex ck ((double)k.GetY(), (double)-k.GetX());
			m_coeff[n][m] = ck * h ;
		}
	}

	for (UINT n = 0; n < m_ResX; ++n) {
		std::vector<Complex> An = FFT(m_coeff[n]);
		for (int i = 0; i < An.size(); ++i) {
			m_A[i][n] = An[i];
		}
	}

	for (UINT m = 0; m < m_ResY; ++m) {
		std::vector<Complex> Dm = FFT(m_A[m]);
		for (int n = 0; n < m_ResX; ++n) {
			float x = Dm[n].real() * m_VerticalShift;
			float y = Dm[n].imag() * m_VerticalShift;
			int index = n * m_ResY + m;
			m_Displacement[index].SetX(x);
			m_Displacement[index].SetY(y);
		}
	}	
}


void FFTOcean::NormalUpdate() {

	const float normalScale = 9.0;
	for (UINT n = 0; n < m_ResX; ++n) {
		for (int m = 0; m < m_ResY; ++m) {
			Vector2 k = WaveK(n, m);
			double lenK = Length(k);
			m_coeff[n][m] *= lenK * normalScale;				// After calculating the shift m_coeff[n,m] = ik/|k|
		}
	}

	// Calculate gradient and use that as normal
	for (UINT n = 0; n < m_ResX; ++n) {
		std::vector<Complex> An = FFT(m_coeff[n]);
		for (size_t i = 0; i < An.size(); ++i) {
			m_A[i][n] = An[i];
		}
	}
	
	for (UINT m = 0; m < m_ResY; ++m) {
		std::vector<Complex> gradm = FFT(m_A[m]);
		for (size_t n = 0; n < gradm.size(); ++n) {
			float x = gradm[n].real();
			float y = gradm[n].imag();
			int index = n * m_ResY + m;
			m_Normals[index] = Normalize(Vector3(-x, -y, 1.0));
		}
	}
}

void FFTOcean::Update(double dt) {
	m_Time += 3.0 * dt;

	AmplitedeUpdate();
	HeightUpdate();
	ShiftUpdate();
	NormalUpdate();

	D3D12_SUBRESOURCE_DATA textureData = 
	Graphic::Texture::CreateTextureData({ m_ResX, m_ResY, 4, 4}, (const unsigned char*)&m_Displacement[0]);
	m_DisplacementTexture->UploadTexture(&textureData);

	D3D12_SUBRESOURCE_DATA nData = 
	Graphic::Texture::CreateTextureData({ m_ResX, m_ResY, 4, 4}, (const unsigned char*)&m_Normals[0]);
	m_NormalTexture->UploadTexture(&nData);
}


std::vector<Complex> BitReverseCopy(const std::vector<Complex>& v) {
	int n = v.size();
	std::vector<Complex> A(n);
	for (int i = 0; i < n; i++) {
		double sign = (i % 2 == 0) ? 1.0 : -1.0;
		A[bitReverse256(i)] =  sign * v[i];
	}
	return A;
}


std::vector<Complex> FFT(const std::vector<Complex>& coeff) {
	int n = coeff.size();
	int lgN = logOceanResolution;

	// Bit reverse copy
	std::vector<Complex> A = BitReverseCopy(coeff);

	for (int i = 1; i <= lgN; ++i) {
		int m = 1<<i;
		Complex Wm = std::exp(Complex{0, 2.0 * PI / m});
		for (int k = 0; k <= n - 1; k += m) {
			Complex W{1.0, 0.0};
			for (int j = 0; j <= m / 2 - 1; ++j) {
				Complex t = W * A[k + j + m/2];
				Complex u = A[k + j];
				A[k + j] = u + t;
				A[k + j + m/2] = u - t;
				W = W * Wm;
			}
		}
	}


	for (int i = 0; i < n; i++) {
		double sign = (i % 2 == 0) ? 1.0 : -1.0;
		A[i] =  sign * A[i];
	}

	return A;
}