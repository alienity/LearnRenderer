// LearnViewer.cpp : Defines the entry point for the application.
//

#include "Display.h"
#include "GameCore.h"
#include "VectorMath.h"
#include "Camera.h"
#include "CameraController.h"
#include "ShadowCamera.h"
#include "CommandContext.h"

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

void LearnViewer::Startup(void) {

}

void LearnViewer::Cleanup(void) {

}

void LearnViewer::Update(float deltaT) {

}

void LearnViewer::RenderScene(void) {
    GraphicsContext& gfxContext = GraphicsContext::Begin(L"Scene Render");

    gfxContext.Finish();
}
