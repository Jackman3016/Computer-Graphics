#include <d3d11.h>
#include <wrl.h>
#include <d3dcompiler.h>
#include <xlocnum>

namespace wrl = Microsoft::WRL;
class Graphics {
public:
	HWND hWnd;
public:
	void D3DInitialize() {
		DXGI_SWAP_CHAIN_DESC sd = {};
		sd.BufferDesc.Width = 0;
		sd.BufferDesc.Height = 0;
		sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 0;
		sd.BufferDesc.RefreshRate.Denominator = 0;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//
		sd.BufferCount = 1;// one back buffer
		sd.OutputWindow = hWnd;
		sd.Windowed = TRUE;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		sd.Flags = 0;
		//create device and front/back buffers, and swap chain and rendering context
		D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			0,
			nullptr,
			D3D11_SDK_VERSION,
			0,
			&sd,
			pSwap.GetAddressOf(),
			&pDevice,
			nullptr,
			&pContext);
		//gain access to texture subresource in swap chain (back buffer)
		ID3D11Resource* pBackBuffer = nullptr;
		pSwap->GetBuffer(0, __uuidof(ID3D11Resource), reinterpret_cast<void**>(&pBackBuffer));
		pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pTarget);//利用后缓存创建渲染目标视图后，不再需要
		pBackBuffer->Release();
	}
	void DrawTriangle() {
#pragma region IA(Vertex and Index buffer)
		struct Vertex {
			float x;
			float y;
			float r;
			float g;
			float b;
		};
#pragma region create vertex buffer
		const Vertex vertices[] = {
			{0.0f, 0.5f  , 1.0f,0.0f,0.0f},
			{0.5f, -0.5f , 1.0f,0.0f,0.0f},
			{-0.5f, -0.5f, 1.0f,0.0f,0.0f},

			//{0.5f, 1.0f},//逆序不会显示三角形
			//{0.5f, 0.5f},
			//{1.0f, 0.5f}
		};

		wrl::ComPtr<ID3D11Buffer> pVertexBuffer;

		D3D11_BUFFER_DESC bd = {};
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.CPUAccessFlags = 0u;
		bd.MiscFlags = 0u;
		bd.ByteWidth = sizeof(vertices);//TODO: 大小为vertices的大小
		bd.StructureByteStride = sizeof(Vertex);

		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = vertices;

		pDevice->CreateBuffer(&bd, &sd, &pVertexBuffer);

		const UINT stride = sizeof(Vertex);
		const UINT offset = 0u;
		//bind vertex buffer to pipeline
		pContext->IASetVertexBuffers(0u, 1u, pVertexBuffer.GetAddressOf(), &stride, &offset);
#pragma endregion
#pragma region create index buffer
		const unsigned short indexs[] = {
			0,1,2,
			0,2,3,
			0,4,1,
			2,1,5,
		};

		wrl::ComPtr<ID3D11Buffer> pIndexBuffer;

		D3D11_BUFFER_DESC ibd = {};
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.Usage = D3D11_USAGE_DEFAULT;
		ibd.CPUAccessFlags = 0u;
		ibd.MiscFlags = 0u;
		ibd.ByteWidth = sizeof(indexs);//TODO: 大小为vertices的大小
		ibd.StructureByteStride = sizeof(unsigned short);

		D3D11_SUBRESOURCE_DATA isd = {};
		isd.pSysMem = indexs;

		pDevice->CreateBuffer(&bd, &sd, &pVertexBuffer);

		//bind index buffer
		pContext->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);
#pragma endregion
#pragma endregion

		wrl::ComPtr<ID3DBlob> pBlob;					//blob是二进制, 用来临时存放.cso
#pragma region Pixel Shader
		//create pixel shader
		wrl::ComPtr<ID3D11PixelShader> pPixelShader;
		D3DReadFileToBlob(L"PixelShader.cso", &pBlob);//重用了pBlob
		pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader);
		//bind pixel shader
		pContext->PSSetShader(pPixelShader.Get(), 0, 0);

#pragma endregion
#pragma region Vertex Shader
		//create vertex shader
		wrl::ComPtr<ID3D11VertexShader> pVertexShader;
		D3DReadFileToBlob(L"VertexShader.cso", &pBlob);
		pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader);

		//bind vertex shader
		pContext->VSSetShader(pVertexShader.Get(), 0, 0);
#pragma endregion
#pragma region InputLayout
		//input(vertex) layout
		wrl::ComPtr<ID3D11InputLayout> pInputLayout;
		const D3D11_INPUT_ELEMENT_DESC ied[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
		pDevice->CreateInputLayout(
			ied, (UINT)std::size(ied),
			pBlob->GetBufferPointer(),
			pBlob->GetBufferSize(),
			&pInputLayout);
		pContext->IASetInputLayout(pInputLayout.Get()); 
#pragma endregion
		//bind render target
		pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), nullptr);//需要设置输出合并阶段的输出目标，否则pixelshader不知道输出到哪里

		//set primitive topology
		pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//configure viewport
		D3D11_VIEWPORT vp;//并不会改变应用窗口的大小，但会改变绘制窗口的大小
		vp.Width = 800;
		vp.Height = 600;
		vp.MinDepth = 0;
		vp.MaxDepth = 1;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		pContext->RSSetViewports(1u, &vp);

		//pContext->Draw((UINT)std::size(vertices), 0u);
		pContext->DrawIndexed((UINT)std::size(indexs), 0u, 0u);
	}
	//交换 front 和 back
	void EndFrame() {
		pSwap->Present(0, 0);
	}

	void ClearBuffer(float red, float green, float blue) noexcept {
		const float color[] = { red, green, blue, 1.0f };
		pContext->ClearRenderTargetView(pTarget.Get(), color);
	}
private:
	//设备
	wrl::ComPtr<ID3D11Device> pDevice;
	wrl::ComPtr<ID3D11DeviceContext> pContext;
	wrl::ComPtr<IDXGISwapChain> pSwap;
	//资源
	wrl::ComPtr<ID3D11RenderTargetView> pTarget;
};

int main() {
	Graphics graphics;
	while (1) {
		//const COLOR16 c = sin(time.Peek())//TODO: not finished
		graphics.ClearBuffer();
		graphics.EndFrame();
	}
}
