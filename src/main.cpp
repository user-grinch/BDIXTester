#include "Chelz/application.h"
#include "handler.hpp"
#define TESTER_VERSION "1.0"

void TextCentered(const std::string& text) {
    ImVec2 size = ImGui::CalcTextSize(text.c_str());
    ImGui::NewLine();
    float width = ImGui::GetWindowContentRegionWidth() - size.x;
    ImGui::SameLine(width/2);
    ImGui::Text(text.c_str());
}

void AboutPopUp() {
    ImGui::Columns(2, nullptr, false);
    ImGui::Text("Author: Grinch_");
    ImGui::Text("Version: " TESTER_VERSION);
    ImGui::Spacing();
    ImGui::NextColumn();
    ImGui::Text("ImGui: %s", ImGui::GetVersion());
    ImGui::Text("Build: %s", __DATE__);
    ImGui::Columns(1);
    
    ImGui::Dummy(ImVec2(0, 10));
    TextCentered("Credits");
    ImGui::Columns(2, NULL, false);
    ImGui::Text("Freetype");
    ImGui::Text("ImGui");
    ImGui::NextColumn();
    ImGui::Text("SimpleINI");
    ImGui::Columns(1);
    ImGui::Dummy(ImVec2(0, 10));

    ImVec2 btnSz = ImVec2(ImGui::GetWindowWidth()/2 - ImGui::GetStyle().ItemSpacing.x, ImGui::GetFrameHeight());
    if (ImGui::Button("GitHub", btnSz)) {
        ShellExecuteA(nullptr, "open", "https://github.com/user-grinch/IMGEditor/", nullptr, nullptr, SW_SHOWNORMAL);
    }
    ImGui::SameLine();
    if (ImGui::Button("Patreon", btnSz)) {
        ShellExecuteA(nullptr, "open", "https://www.patreon.com/grinch_", nullptr, nullptr, SW_SHOWNORMAL);
    }
    ImGui::Spacing();
    TextCentered("Copyright Grinch_ 2024-2025. All rights reserved.");
}

void WelcomePopup() {
    TextCentered("Welcome to BDIX Tester v" TESTER_VERSION);
    TextCentered("by Grinch_");
    ImGui::Spacing();
    ImGui::Dummy(ImVec2(0, 20));
    ImVec2 btnSz = ImVec2(ImGui::GetWindowWidth()/2 - ImGui::GetStyle().ItemSpacing.x, ImGui::GetFrameHeight());
    if (ImGui::Button("GitHub", btnSz)) {
        ShellExecuteA(nullptr, "open", "https://github.com/user-grinch/BDIXTester/", nullptr, nullptr, SW_SHOWNORMAL);
    }
    ImGui::SameLine();
    if (ImGui::Button("Patreon", btnSz)) {
        ShellExecuteA(nullptr, "open", "https://www.patreon.com/grinch_", nullptr, nullptr, SW_SHOWNORMAL);
    }
    ImGui::Spacing();
    TextCentered("Copyright Grinch_ 2024-2025. All rights reserved.");
}


int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    BDIXTester handler("bdix.db", "https://raw.githubusercontent.com/user-grinch/BDIXTester/main/resource/bdix.db", 10);
    static void *popupFunc = nullptr;
    static Chelz::Specification Spec;
    Spec.Name = L"BDIX Tester";
    Spec.Size = {350, 500};
    popupFunc = WelcomePopup;
    Spec.MenuBarFunc = [&handler](){
        if (ImGui::BeginMenu("About")) {
            if (ImGui::MenuItem("BDIXTester")) {
                popupFunc = AboutPopUp;
            }
            if (ImGui::MenuItem("Welcome")) {
                popupFunc = WelcomePopup;
            }
            ImGui::EndMenu();
        }
    };
    Spec.LayerFunc = [&handler](){
        handler.RenderUI(popupFunc == nullptr);
        if (popupFunc) {
            ImGui::OpenPopup("##Popup");
            auto windowSize = ImGui::GetWindowSize();
            ImGui::SetNextWindowPos(ImVec2(windowSize.x / 2, windowSize.y / 2), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            if (ImGui::BeginPopupModal("##Popup", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize)) {
                ((void(*)())popupFunc)();
                ImGui::NewLine();
                if (ImGui::Button("OK", ImVec2(ImGui::GetWindowContentRegionWidth(), 0))) {
                    popupFunc = nullptr;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
    };
    Chelz::Application App(Spec);
    App.Run();

    return 0;
}