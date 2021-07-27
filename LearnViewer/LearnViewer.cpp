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

private:

    Camera m_Camera;
    unique_ptr<CameraController> m_CameraController;

    D3D12_VIEWPORT m_MainViewport;
    D3D12_RECT m_MainScissor;

    ShadowCamera m_SunShadowCamera;
};

CREATE_APPLICATION(LearnViewer)

RootSignature m_TestRootSig;
GraphicsPSO m_TestPSO(L"Renderer: Test PSO");
DescriptorHeap s_TextureHeap;
DescriptorHandle m_CommonTextures;

void LearnViewer::Startup(void) {
    m_TestRootSig.Reset(1, 1);
    m_TestRootSig[0].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
    m_TestRootSig.InitStaticSampler(0, SamplerLinearWrapDesc);
    m_TestRootSig.Finalize(L"TestRootSig");

    m_TestPSO.SetRootSignature(m_TestRootSig);
    m_TestPSO.SetRasterizerState(RasterizerDefault);
    m_TestPSO.SetBlendState(BlendDisable);
    m_TestPSO.SetDepthStencilState(DepthStateDisabled);
    m_TestPSO.SetSampleMask(0xFFFFFFFF);
    m_TestPSO.SetInputLayout(0, nullptr);
    m_TestPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    m_TestPSO.SetVertexShader(g_pDefaultVS, sizeof(g_pDefaultVS));
    m_TestPSO.SetPixelShader(g_pDefaultPS, sizeof(g_pDefaultPS));
    m_TestPSO.SetRenderTargetFormat(g_SceneColorBuffer.GetFormat(), DXGI_FORMAT_UNKNOWN);
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

    gfxContext.SetRootSignature(m_TestRootSig);
    gfxContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    gfxContext.SetPipelineState(m_TestPSO);

    gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
    gfxContext.SetRenderTarget(g_SceneColorBuffer.GetRTV());
    gfxContext.SetViewportAndScissor(0, 0, g_DisplayWidth, g_DisplayHeight);
    gfxContext.Draw(3);

    gfxContext.Finish();
}
