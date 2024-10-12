#include "application.h"
#include "renderer.h"

Chelz::Application::Application(Specification &Spec)
{
    Info = Spec;
    Renderer::Init(Spec);
}

Chelz::Application::~Application()
{
    Renderer::Shutdown();
}

void Chelz::Application::SetPopup(std::function<void()> Func)
{
    Info.PopupFunc = Func;
}

Chelz::CVector2 Chelz::Application::GetWindowSize()
{
    return Info.Size;
}

Chelz::eTheme Chelz::Application::GetTheme()
{
    return Chelz::Renderer::GetThemeMode();
}

void Chelz::Application::SetTheme(Chelz::eTheme theme)
{
    Chelz::Renderer::SetThemeMode(theme);
}

void Chelz::Application::Run()
{
    bool exit = false;

    while (!exit)
    {
        // Handle windows messages
        MSG msg;
        while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
            {
                exit = true;
            }
        }
        if (exit)
        {
            if (Info.CleanupFunc)
            {
                Info.CleanupFunc();
            }
            break;
        }
        

        Renderer::DrawLayer(Info);
    }
}