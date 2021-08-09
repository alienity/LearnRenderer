// LearnViewer.cpp : Defines the entry point for the application.
//

#include "Display.h"
#include "GameCore.h"
#include "VectorMath.h"
#include "Camera.h"
#include "CameraController.h"
#include "ShadowCamera.h"
#include "CommandContext.h"
#include "BufferManager.h"

#include "CompiledShaders/DefaultVS.h"
#include "CompiledShaders/DefaultPS.h"

using namespace GameCore;
using namespace Math;
using namespace Graphics;
using namespace std;

class LearnViewer : public GameCore::IGameApp {
public:
    LearnViewer(void) {}

    virtual void Startup(void) override;
    virtual void Cleanup(void) override;

    virtual void Update(float deltaT) override;

    virtual void RenderScene(void) override;

    void InitGeometry();

private:

    Camera m_Camera;
    unique_ptr<CameraController> m_CameraController;

    D3D12_VIEWPORT m_MainViewport;
    D3D12_RECT m_MainScissor;

    ByteAddressBuffer m_VertexBuffer;
    ByteAddressBuffer m_IndexBuffer;

    ShadowCamera m_SunShadowCamera;
};

CREATE_APPLICATION(LearnViewer)

RootSignature m_TestRootSig;
GraphicsPSO m_TestPSO(L"Renderer: Test PSO");
DescriptorHeap s_TextureHeap;
DescriptorHandle m_CommonTextures;

void LearnViewer::InitGeometry() {
    struct VertexBuffer {
        XMFLOAT3 pos;
        XMFLOAT2 uv0;
    public:
        VertexBuffer() = default;
        VertexBuffer(XMFLOAT3 _pos, XMFLOAT2 _uv) {
            this->pos = _pos;
            this->uv0 = _uv;
        }
    };

    std::vector<VertexBuffer> vertexData;

    vertexData.push_back(VertexBuffer(XMFLOAT3(-1, -1, 1), XMFLOAT2(0, 0)));
    vertexData.push_back(VertexBuffer(XMFLOAT3(1, -1, 1), XMFLOAT2(1, 0)));
    vertexData.push_back(VertexBuffer(XMFLOAT3(1, 1, 1), XMFLOAT2(1, 1)));

    vertexData.push_back(VertexBuffer(XMFLOAT3(-1, -1, 1), XMFLOAT2(0, 0)));
    vertexData.push_back(VertexBuffer(XMFLOAT3(1, 1, 1), XMFLOAT2(1, 1)));
    vertexData.push_back(VertexBuffer(XMFLOAT3(-1, 1, 1), XMFLOAT2(0, 1)));

    vertexData.push_back(VertexBuffer(XMFLOAT3(1, -1, 1), XMFLOAT2(0, 0)));
    vertexData.push_back(VertexBuffer(XMFLOAT3(1, -1, -1), XMFLOAT2(1, 0)));
    vertexData.push_back(VertexBuffer(XMFLOAT3(1, 1, -1), XMFLOAT2(1, 1)));

    vertexData.push_back(VertexBuffer(XMFLOAT3(1, -1, 1), XMFLOAT2(0, 0)));
    vertexData.push_back(VertexBuffer(XMFLOAT3(1, 1, -1), XMFLOAT2(1, 1)));
    vertexData.push_back(VertexBuffer(XMFLOAT3(1, 1, 1), XMFLOAT2(0, 1)));

    vertexData.push_back(VertexBuffer(XMFLOAT3(1, -1, -1), XMFLOAT2(0, 0)));
    vertexData.push_back(VertexBuffer(XMFLOAT3(-1, -1, -1), XMFLOAT2(1, 0)));
    vertexData.push_back(VertexBuffer(XMFLOAT3(-1, 1, -1), XMFLOAT2(1, 1)));

    vertexData.push_back(VertexBuffer(XMFLOAT3(1, -1, -1), XMFLOAT2(0, 0)));
    vertexData.push_back(VertexBuffer(XMFLOAT3(-1, 1, -1), XMFLOAT2(1, 1)));
    vertexData.push_back(VertexBuffer(XMFLOAT3(1, 1, -1), XMFLOAT2(0, 1)));

    vertexData.push_back(VertexBuffer(XMFLOAT3(-1, -1, -1), XMFLOAT2(0, 0)));
    vertexData.push_back(VertexBuffer(XMFLOAT3(-1, -1, 1), XMFLOAT2(1, 0)));
    vertexData.push_back(VertexBuffer(XMFLOAT3(-1, 1, 1), XMFLOAT2(1, 1)));

    vertexData.push_back(VertexBuffer(XMFLOAT3(-1, -1, -1), XMFLOAT2(0, 0)));
    vertexData.push_back(VertexBuffer(XMFLOAT3(-1, 1, 1), XMFLOAT2(1, 1)));
    vertexData.push_back(VertexBuffer(XMFLOAT3(-1, 1, -1), XMFLOAT2(0, 1)));

    vertexData.push_back(VertexBuffer(XMFLOAT3(-1, 1, 1), XMFLOAT2(0, 0)));
    vertexData.push_back(VertexBuffer(XMFLOAT3(1, 1, 1), XMFLOAT2(1, 0)));
    vertexData.push_back(VertexBuffer(XMFLOAT3(1, 1, -1), XMFLOAT2(1, 1)));

    vertexData.push_back(VertexBuffer(XMFLOAT3(-1, 1, 1), XMFLOAT2(0, 0)));
    vertexData.push_back(VertexBuffer(XMFLOAT3(1, 1, -1), XMFLOAT2(1, 1)));
    vertexData.push_back(VertexBuffer(XMFLOAT3(-1, 1, -1), XMFLOAT2(0, 1)));

    vertexData.push_back(VertexBuffer(XMFLOAT3(-1, -1, -1), XMFLOAT2(0, 0)));
    vertexData.push_back(VertexBuffer(XMFLOAT3(1, -1, -1), XMFLOAT2(1, 0)));
    vertexData.push_back(VertexBuffer(XMFLOAT3(1, -1, 1), XMFLOAT2(1, 1)));

    vertexData.push_back(VertexBuffer(XMFLOAT3(-1, -1, -1), XMFLOAT2(0, 0)));
    vertexData.push_back(VertexBuffer(XMFLOAT3(1, -1, 1), XMFLOAT2(1, 1)));
    vertexData.push_back(VertexBuffer(XMFLOAT3(-1, -1, 1), XMFLOAT2(0, 1)));

    m_VertexBuffer.Create(L"VertexBuffer", vertexData.size(), sizeof(VertexBuffer), vertexData.data());

    std::vector<int> indexBuffer;
    for (int i = 0; i < vertexData.size(); ++i) {
        indexBuffer.push_back(i);
    }
    m_IndexBuffer.Create(L"IndexBuffer", indexBuffer.size(), sizeof(int), indexBuffer.data());

}

void LearnViewer::Startup(void) {
    InitGeometry();

    m_Camera.SetEyeAtUp(Vector3(0, 0, 5), Vector3(kZero), Vector3(kYUnitVector));

    m_TestRootSig.Reset(1, 1);
    m_TestRootSig[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
    m_TestRootSig.InitStaticSampler(0, SamplerLinearWrapDesc);
    m_TestRootSig.Finalize(L"TestRootSig", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    std::vector<D3D12_INPUT_ELEMENT_DESC> vertexLayout;
    vertexLayout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT });
    vertexLayout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT });

    m_TestPSO.SetRootSignature(m_TestRootSig);
    m_TestPSO.SetRasterizerState(RasterizerDefault);
    m_TestPSO.SetBlendState(BlendDisable);
    m_TestPSO.SetDepthStencilState(DepthStateReadWrite);
    //m_TestPSO.SetSampleMask(0xFFFFFFFF);
    m_TestPSO.SetInputLayout((uint32_t)vertexLayout.size(), vertexLayout.data());
    m_TestPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    m_TestPSO.SetVertexShader(g_pDefaultVS, sizeof(g_pDefaultVS));
    m_TestPSO.SetPixelShader(g_pDefaultPS, sizeof(g_pDefaultPS));
    m_TestPSO.SetRenderTargetFormat(g_SceneColorBuffer.GetFormat(), g_SceneDepthBuffer.GetFormat());
    m_TestPSO.Finalize();

    s_TextureHeap.Create(L"Scene Texture Descriptors", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4096);
    m_CommonTextures = s_TextureHeap.Alloc(1);

    // ���������ز���
    m_Camera.SetZRange(1.0f, 10000.0f);
    m_CameraController.reset(new FlyingFPSCamera(m_Camera, Vector3(kYUnitVector)));
}

void LearnViewer::Cleanup(void) {
    s_TextureHeap.Destroy();

    m_VertexBuffer.Destroy();
    m_IndexBuffer.Destroy();
}

void LearnViewer::Update(float deltaT) {
    m_CameraController->Update(deltaT);
}

void LearnViewer::RenderScene(void) {
    GraphicsContext& gfxContext = GraphicsContext::Begin(L"Scene Render");

    __declspec(align(16)) struct DefaultVSCB
    {
        Matrix4 Proj;
        Matrix4 View;
    } defaultVSCB;
    defaultVSCB.Proj = m_Camera.GetProjMatrix();
    defaultVSCB.View = m_Camera.GetViewMatrix();

    gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
    gfxContext.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE);
    gfxContext.ClearColor(g_SceneColorBuffer);
    gfxContext.ClearDepth(g_SceneDepthBuffer);

    gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    gfxContext.SetRootSignature(m_TestRootSig);
    gfxContext.SetPipelineState(m_TestPSO);

    gfxContext.SetRenderTarget(g_SceneColorBuffer.GetRTV(), g_SceneDepthBuffer.GetDSV());
    gfxContext.SetViewportAndScissor(0, 0, g_DisplayWidth, g_DisplayHeight);

    gfxContext.SetDynamicConstantBufferView(0, sizeof(DefaultVSCB), &defaultVSCB);

    gfxContext.SetIndexBuffer(m_IndexBuffer.IndexBufferView());
    gfxContext.SetVertexBuffer(0, m_VertexBuffer.VertexBufferView());

    gfxContext.DrawIndexed(m_IndexBuffer.GetElementCount());

    gfxContext.Finish();
}
