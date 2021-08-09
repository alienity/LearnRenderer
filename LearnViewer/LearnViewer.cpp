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
    // 准备顶点数据
    struct TextureCoord {
        float u, v;
    public:
        TextureCoord() = default;
        TextureCoord(float _u, float _v) {
            this->u = _u;
            this->v = _v;
        }
    };

    struct VertexBuffer {
        Vector3 pos;
        TextureCoord uv0;
    public:
        VertexBuffer() = default;
        VertexBuffer(Vector3 _pos, TextureCoord _uv) {
            this->pos = _pos;
            this->uv0 = _uv;
        }
    };

    std::vector<VertexBuffer> vertexData;

    vertexData.push_back(VertexBuffer(Vector3(-1, -1, -1), TextureCoord(0, 0)));
    vertexData.push_back(VertexBuffer(Vector3(1, -1, -1), TextureCoord(1, 0)));
    vertexData.push_back(VertexBuffer(Vector3(1, 1, -1), TextureCoord(1, 1)));

    vertexData.push_back(VertexBuffer(Vector3(-1, -1, -1), TextureCoord(0, 0)));
    vertexData.push_back(VertexBuffer(Vector3(1, 1, -1), TextureCoord(1, 1)));
    vertexData.push_back(VertexBuffer(Vector3(-1, 1, -1), TextureCoord(0, 1)));

    vertexData.push_back(VertexBuffer(Vector3(1, -1, -1), TextureCoord(0, 0)));
    vertexData.push_back(VertexBuffer(Vector3(1, -1, 1), TextureCoord(1, 1)));
    vertexData.push_back(VertexBuffer(Vector3(1, 1, 1), TextureCoord(1, 1)));

    vertexData.push_back(VertexBuffer(Vector3(1, -1, -1), TextureCoord(0, 0)));
    vertexData.push_back(VertexBuffer(Vector3(1, 1, 1), TextureCoord(1, 1)));
    vertexData.push_back(VertexBuffer(Vector3(1, 1, -1), TextureCoord(0, 1)));

    vertexData.push_back(VertexBuffer(Vector3(1, -1, 1), TextureCoord(0, 0)));
    vertexData.push_back(VertexBuffer(Vector3(-1, -1, 1), TextureCoord(1, 1)));
    vertexData.push_back(VertexBuffer(Vector3(-1, -1, 1), TextureCoord(1, 1)));

    vertexData.push_back(VertexBuffer(Vector3(1, -1, 1), TextureCoord(0, 0)));
    vertexData.push_back(VertexBuffer(Vector3(-1, -1, 1), TextureCoord(1, 1)));
    vertexData.push_back(VertexBuffer(Vector3(1, 1, 1), TextureCoord(0, 1)));

    vertexData.push_back(VertexBuffer(Vector3(-1, -1, 1), TextureCoord(0, 0)));
    vertexData.push_back(VertexBuffer(Vector3(-1, -1, -1), TextureCoord(1, 1)));
    vertexData.push_back(VertexBuffer(Vector3(-1, 1, -1), TextureCoord(1, 1)));

    vertexData.push_back(VertexBuffer(Vector3(-1, -1, 1), TextureCoord(0, 0)));
    vertexData.push_back(VertexBuffer(Vector3(-1, 1, -1), TextureCoord(1, 1)));
    vertexData.push_back(VertexBuffer(Vector3(-1, -1, 1), TextureCoord(0, 1)));

    vertexData.push_back(VertexBuffer(Vector3(-1, 1, -1), TextureCoord(0, 0)));
    vertexData.push_back(VertexBuffer(Vector3(1, 1, -1), TextureCoord(1, 1)));
    vertexData.push_back(VertexBuffer(Vector3(1, 1, 1), TextureCoord(1, 1)));

    vertexData.push_back(VertexBuffer(Vector3(-1, 1, 1), TextureCoord(0, 0)));
    vertexData.push_back(VertexBuffer(Vector3(1, 1, 1), TextureCoord(1, 1)));
    vertexData.push_back(VertexBuffer(Vector3(-1, -1, 1), TextureCoord(0, 1)));

    vertexData.push_back(VertexBuffer(Vector3(-1, -1, 1), TextureCoord(0, 0)));
    vertexData.push_back(VertexBuffer(Vector3(1, -1, 1), TextureCoord(1, 1)));
    vertexData.push_back(VertexBuffer(Vector3(1, -1, -1), TextureCoord(1, 1)));

    vertexData.push_back(VertexBuffer(Vector3(-1, -1, 1), TextureCoord(0, 0)));
    vertexData.push_back(VertexBuffer(Vector3(1, -1, -1), TextureCoord(1, 1)));
    vertexData.push_back(VertexBuffer(Vector3(-1, -1, -1), TextureCoord(0, 1)));

    m_VertexBuffer.Create(L"VertexBuffer", vertexData.size(), sizeof(VertexBuffer), vertexData.data());



}

void LearnViewer::Startup(void) {
    m_Camera.SetEyeAtUp(Vector3(0, 0, 2), Vector3(kZero), Vector3(kYUnitVector));

    m_TestRootSig.Reset(1, 1);
    m_TestRootSig[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
    m_TestRootSig.InitStaticSampler(0, SamplerLinearWrapDesc);
    m_TestRootSig.Finalize(L"TestRootSig");

    std::vector<D3D12_INPUT_ELEMENT_DESC> vertexLayout;
    vertexLayout.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT });
    vertexLayout.push_back({ "TEXCOORD", 0, DXGI_FORMAT_R16G16_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT });

    m_TestPSO.SetRootSignature(m_TestRootSig);
    m_TestPSO.SetRasterizerState(RasterizerDefault);
    m_TestPSO.SetBlendState(BlendDisable);
    m_TestPSO.SetDepthStencilState(DepthStateDisabled);
    m_TestPSO.SetSampleMask(0xFFFFFFFF);
    m_TestPSO.SetInputLayout((uint32_t)vertexLayout.size(), vertexLayout.data());
    m_TestPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    m_TestPSO.SetVertexShader(g_pDefaultVS, sizeof(g_pDefaultVS));
    m_TestPSO.SetPixelShader(g_pDefaultPS, sizeof(g_pDefaultPS));
    m_TestPSO.SetRenderTargetFormat(g_SceneColorBuffer.GetFormat(), g_SceneDepthBuffer.GetFormat());
    m_TestPSO.Finalize();

    s_TextureHeap.Create(L"Scene Texture Descriptors", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4096);
    m_CommonTextures = s_TextureHeap.Alloc(1);

    // 把其他的纹理的handle拷贝到这里
}

void LearnViewer::Cleanup(void) {
    s_TextureHeap.Destroy();
}

void LearnViewer::Update(float deltaT) {

}

void LearnViewer::RenderScene(void) {
    GraphicsContext& gfxContext = GraphicsContext::Begin(L"Scene Render");

    __declspec(align(16)) struct DefaultVSCB
    {
        Matrix4 Proj;
        Matrix4 View;
    } defaultVSCB;
    defaultVSCB.Proj = Invert(m_Camera.GetProjMatrix());
    defaultVSCB.View = Invert(m_Camera.GetViewMatrix());

    gfxContext.SetRootSignature(m_TestRootSig);
    gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    gfxContext.SetPipelineState(m_TestPSO);

    gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
    gfxContext.SetRenderTarget(g_SceneColorBuffer.GetRTV());
    gfxContext.SetViewportAndScissor(0, 0, g_DisplayWidth, g_DisplayHeight);

    gfxContext.SetDynamicConstantBufferView(0, sizeof(DefaultVSCB), &defaultVSCB);


    gfxContext.Draw(3);

    gfxContext.Finish();
}
