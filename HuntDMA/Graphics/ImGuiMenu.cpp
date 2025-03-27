#include "Pch.h"
#include "ImGuiMenu.h"
#include "Input.h"
#include "Init.h"
#include "Kmbox.h"
#include "Globals.h"
#include "ESPRenderer.h"
#include "Localization/Localization.h"

// Global instances
ImGuiMenu g_ImGuiMenu;
bool MenuOpen = false;

namespace ImGuiUtils {
    bool IsKeyPressed(int vKey) {
        return (GetAsyncKeyState(vKey) & 0x8000) != 0;
    }

    float GetTime() {
        return static_cast<float>(clock() * 0.001f);
    }

    // VK to ImGuiKey
    ImGuiKey VirtualKeyToImGuiKey(int vk) {
        switch (vk) {
        case VK_TAB: return ImGuiKey_Tab;
        case VK_LEFT: return ImGuiKey_LeftArrow;
        case VK_RIGHT: return ImGuiKey_RightArrow;
        case VK_UP: return ImGuiKey_UpArrow;
        case VK_DOWN: return ImGuiKey_DownArrow;
        case VK_PRIOR: return ImGuiKey_PageUp;
        case VK_NEXT: return ImGuiKey_PageDown;
        case VK_HOME: return ImGuiKey_Home;
        case VK_END: return ImGuiKey_End;
        case VK_INSERT: return ImGuiKey_Insert;
        case VK_DELETE: return ImGuiKey_Delete;
        case VK_BACK: return ImGuiKey_Backspace;
        case VK_SPACE: return ImGuiKey_Space;
        case VK_RETURN: return ImGuiKey_Enter;
        case VK_ESCAPE: return ImGuiKey_Escape;
        case VK_SHIFT: return ImGuiKey_LeftShift;
        case VK_CONTROL: return ImGuiKey_LeftCtrl;
        case VK_MENU: return ImGuiKey_LeftAlt;
        case VK_LBUTTON: return ImGuiKey_MouseLeft;
        case VK_RBUTTON: return ImGuiKey_MouseRight;
        case VK_MBUTTON: return ImGuiKey_MouseMiddle;
        case VK_XBUTTON1: return ImGuiKey_MouseX1;
        case VK_XBUTTON2: return ImGuiKey_MouseX2;
        default:
            if (vk >= '0' && vk <= '9') return (ImGuiKey)(ImGuiKey_0 + (vk - '0'));
            if (vk >= 'A' && vk <= 'Z') return (ImGuiKey)(ImGuiKey_A + (vk - 'A'));
            return ImGuiKey_None;
        }
    }

    // ImGuiKey to VK
    int ImGuiKeyToVirtualKey(ImGuiKey key) {
        switch (key) {
        case ImGuiKey_Tab: return VK_TAB;
        case ImGuiKey_LeftArrow: return VK_LEFT;
        case ImGuiKey_RightArrow: return VK_RIGHT;
        case ImGuiKey_UpArrow: return VK_UP;
        case ImGuiKey_DownArrow: return VK_DOWN;
        case ImGuiKey_PageUp: return VK_PRIOR;
        case ImGuiKey_PageDown: return VK_NEXT;
        case ImGuiKey_Home: return VK_HOME;
        case ImGuiKey_End: return VK_END;
        case ImGuiKey_Insert: return VK_INSERT;
        case ImGuiKey_Delete: return VK_DELETE;
        case ImGuiKey_Backspace: return VK_BACK;
        case ImGuiKey_Space: return VK_SPACE;
        case ImGuiKey_Enter: return VK_RETURN;
        case ImGuiKey_Escape: return VK_ESCAPE;
        case ImGuiKey_LeftShift: return VK_SHIFT;
        case ImGuiKey_LeftCtrl: return VK_CONTROL;
        case ImGuiKey_LeftAlt: return VK_MENU;
        case ImGuiKey_MouseLeft: return VK_LBUTTON;
        case ImGuiKey_MouseRight: return VK_RBUTTON;
        case ImGuiKey_MouseMiddle: return VK_MBUTTON;
        case ImGuiKey_MouseX1: return VK_XBUTTON1;
        case ImGuiKey_MouseX2: return VK_XBUTTON2;
        default:
            if (key >= ImGuiKey_0 && key <= ImGuiKey_9) return '0' + (key - ImGuiKey_0);
            if (key >= ImGuiKey_A && key <= ImGuiKey_Z) return 'A' + (key - ImGuiKey_A);
            return 0;
        }
    }

    // Get string representation of key
    const char* GetKeyName(int vk) {
        ImGuiKey key = VirtualKeyToImGuiKey(vk);
        if (key != ImGuiKey_None)
            return ImGui::GetKeyName(key);

        static char buffer[64];
        sprintf_s(buffer, "Key %d", vk);
        return buffer;
    }
}

ImGuiMenu::~ImGuiMenu() {
    Shutdown();
}

bool ImGuiMenu::CreateDeviceD3D(HWND hWnd) {
    // Initialize swap chain description
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 3;
    sd.BufferDesc.Width = 0;  // Use window width
    sd.BufferDesc.Height = 0; // Use window height
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 0; // Remove refresh rate lock for unlimited FPS
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    // Enable tearing support when vsync is off
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    if (!Configs.General.OverlayMode)
        sd.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;   // No MSAA
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = Configs.General.OverlayMode ? DXGI_SWAP_EFFECT_DISCARD : DXGI_SWAP_EFFECT_FLIP_DISCARD; // DXGI_SWAP_EFFECT_FLIP_DISCARD prevents transparency

    // Setup feature levels (DX11 and DX10 fallback)
    const D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0
    };
    D3D_FEATURE_LEVEL featureLevel;

    // Create device with debug layer in debug builds
    UINT createDeviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    // Create device and swap chain
    HRESULT res = D3D11CreateDeviceAndSwapChain(
        nullptr,                   // Default adapter
        D3D_DRIVER_TYPE_HARDWARE,  // Hardware acceleration
        nullptr,                   // No software rasterizer
        createDeviceFlags,
        featureLevels,
        2,                         // Feature level count
        D3D11_SDK_VERSION,
        &sd,
        &swapChain,
        &d3dDevice,
        &featureLevel,
        &d3dDeviceContext);

    if (FAILED(res)) {
        return false;
    }

    // Set maximum frame latency for reduced input lag
    IDXGIDevice1* dxgiDevice;
    d3dDevice->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgiDevice);
    dxgiDevice->SetMaximumFrameLatency(2); // Ensure minimum latency
    dxgiDevice->Release();

    // Create render target view
    CreateRenderTarget();
    return true;
}

void ImGuiMenu::CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer;
    swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (pBackBuffer) {
        d3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &mainRenderTargetView);
        pBackBuffer->Release();
    }
}

void ImGuiMenu::CleanupRenderTarget() {
    if (mainRenderTargetView) { mainRenderTargetView->Release(); mainRenderTargetView = nullptr; }
}

void ImGuiMenu::CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (swapChain) { swapChain->Release(); swapChain = nullptr; }
    if (d3dDeviceContext) { d3dDeviceContext->Release(); d3dDeviceContext = nullptr; }
    if (d3dDevice) { d3dDevice->Release(); d3dDevice = nullptr; }
}

bool ImGuiMenu::Init(HWND hWnd) {
    if (!CreateDeviceD3D(hWnd))
        return false;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    SetupTheme();

    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(d3dDevice, d3dDeviceContext);

    initialized = true;
    return true;
}

void ImGuiMenu::Shutdown() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    initialized = false;
}

void ImGuiMenu::BeginFrame() {
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImGuiMenu::EndFrame() {
    if (!d3dDevice)
        return;

    ImGui::Render();

    const float clear_color[4] = { 0.0f, 0.0f, 0.0f, Configs.General.OverlayMode ? 0.0f : 1.0f };
    d3dDeviceContext->OMSetRenderTargets(1, &mainRenderTargetView, nullptr);
    d3dDeviceContext->ClearRenderTargetView(mainRenderTargetView, clear_color);

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    UINT syncInterval = 0; // Disables V-Sync for immediate presentation
    UINT presentFlags = DXGI_PRESENT_DO_NOT_WAIT;
    if (!Configs.General.OverlayMode)
        presentFlags |= DXGI_PRESENT_ALLOW_TEARING;

    HRESULT hr = swapChain->Present(syncInterval, presentFlags);

    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
        CleanupDeviceD3D();
        CreateDeviceD3D((HWND)ImGui::GetMainViewport()->PlatformHandle);
    }
    else if (hr == DXGI_ERROR_WAS_STILL_DRAWING) {
        // ("Warning: GPU is still drawing, frame skipped.");
        // Optionally introduce a small delay or yield to reduce CPU load
        Sleep(1); // Sleep for 1 ms to yield CPU, adjust as needed
    }
}

void ImGuiMenu::HandleInput() {
    float currentTime = ImGuiUtils::GetTime();

    if (ImGuiUtils::IsKeyPressed(Configs.General.OpenMenuKey) && (currentTime - lastKeyPressTime) > KEY_PRESS_DELAY) {
        MenuOpen = !MenuOpen;
        lastKeyPressTime = currentTime;
    }

    if (Configs.General.CloseMenuOnEsc && ImGuiUtils::IsKeyPressed(VK_ESCAPE)) {
        MenuOpen = false;
    }

    if (ImGuiUtils::IsKeyPressed(Configs.General.OpenMenuKey) && ImGuiUtils::IsKeyPressed(VK_END)) {
        exit(0);
    }
}

void ImGuiMenu::ColorPickerWithText(const char* label, ImVec4* color) {

    ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoInputs;
    flags |= Configs.General.OverlayMode ? ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreview : ImGuiColorEditFlags_NoAlpha;

    // Add label and color picker on the same line
    ImGui::SameLine();
    if (Configs.General.OverlayMode) // Transparency is supported only in OverlayMode
        ImGui::ColorEdit4(label, (float*)color, flags);
    else {
        if (ImGui::ColorEdit3(label, (float*)color, flags)) {
            // Force alpha to 1.0
            color->w = 1.0f;
        }
    }
}

void ImGuiMenu::HelpMarker(const char* desc) {
    ImGui::TextDisabled(LOC("menu", "general.?").c_str());
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

bool ImGuiMenu::HotKey(const char* label, int* key) {
    bool changed = false;
    ImGui::PushID(label);

    // Show current key
    char buf[128];
    sprintf_s(buf, "%s [%s]", label, ImGuiUtils::GetKeyName(*key));
    
    if (ImGui::Button(buf)) {
        ImGui::OpenPopup(LOC("menu", "hotkey.SelectKey").c_str());
    }

    // Modular window for choosing key
    if (ImGui::BeginPopupModal(LOC("menu", "hotkey.SelectKey").c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text(LOC("menu", "hotkey.Instructions").c_str());
        
        // Check mouse input
        if (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)) {
            ImGui::CloseCurrentPopup();
            return false;
        }

        // Check key input
        for (int i = ImGuiKey_NamedKey_BEGIN; i < ImGuiKey_NamedKey_END; i++) {
            if (ImGui::IsKeyPressed((ImGuiKey)i)) {
                *key = ImGuiUtils::ImGuiKeyToVirtualKey((ImGuiKey)i);
                changed = true;
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::EndPopup();
    }

    ImGui::PopID();
    return changed;
}

void ImGuiMenu::SetupTheme() {
    ImGuiStyle& style = ImGui::GetStyle();

    // Colors
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.08f, 0.50f, 0.72f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
    colors[ImGuiCol_Header] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
    colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.41f, 0.42f, 0.44f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.29f, 0.30f, 0.31f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.08f, 0.09f, 0.83f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.33f, 0.34f, 0.36f, 0.83f);
    colors[ImGuiCol_TabActive] = ImVec4(0.23f, 0.23f, 0.24f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

    // Style
    style.WindowPadding = ImVec2(8, 8);
    style.FramePadding = ImVec2(4, 3);
    style.CellPadding = ImVec2(4, 2);
    style.ItemSpacing = ImVec2(8, 4);
    style.ItemInnerSpacing = ImVec2(4, 4);
    style.TouchExtraPadding = ImVec2(0, 0);
    style.IndentSpacing = 21;
    style.ScrollbarSize = 14;
    style.GrabMinSize = 10;

    style.WindowBorderSize = 1;
    style.ChildBorderSize = 1;
    style.PopupBorderSize = 1;
    style.FrameBorderSize = 1;
    style.TabBorderSize = 0;

    style.WindowRounding = 4;
    style.ChildRounding = 4;
    style.FrameRounding = 3;
    style.PopupRounding = 4;
    style.ScrollbarRounding = 9;
    style.GrabRounding = 3;
    style.LogSliderDeadzone = 4;
    style.TabRounding = 4;

    style.ScaleAllSizes(Configs.General.UIScale);
}

void ImGuiMenu::RenderMenu() {
    if (!MenuOpen)
        return;

    // Set default window position and size on first use
    ImGui::SetNextWindowPos(ImVec2(190, 102), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(580, 362), ImGuiCond_FirstUseEver);

    ImGui::Begin(LOC("menu", "window.name").c_str(), &MenuOpen, ImGuiWindowFlags_NoCollapse);

    if (ImGui::BeginTabBar("##Tabs")) {
        if (ImGui::BeginTabItem(LOC("menu", "tabs.PlayerESP").c_str())) {
            RenderPlayerESPTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(LOC("menu", "tabs.BossesESP").c_str())) {
            RenderBossesESPTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(LOC("menu", "tabs.SupplyESP").c_str())) {
            RenderSupplyESPTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(LOC("menu", "tabs.BBESP").c_str())) {
            RenderBloodBondsESPTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(LOC("menu", "tabs.TrapESP").c_str())) {
            RenderTrapESPTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(LOC("menu", "tabs.POIESP").c_str())) {
            RenderPOIESPTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(LOC("menu", "tabs.TraitESP").c_str())) {
            RenderTraitESPTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(LOC("menu", "tabs.Overlay").c_str())) {
            RenderOverlayTab();
            ImGui::EndTabItem();
        }
        if (enableAimBot && ImGui::BeginTabItem(LOC("menu", "tabs.Aimbot").c_str())) {
            RenderAimbotTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(LOC("menu", "tabs.Settings").c_str())) {
            RenderSettingsTab();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    // Check if mouse clicked outside window
    if (Configs.General.OverlayMode)
    {
        if (ImGui::IsMouseClicked(0) &&
            !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) &&
            !ImGui::IsAnyItemHovered() &&
            !ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopup))
            MenuOpen = false;
    }

    ImGui::End();
}

void ImGuiMenu::RenderPlayerESPTab() {
    ImGui::BeginChild("PlayerESPTab", ImVec2(0, 0), false);

    ImGui::Checkbox(LOC("menu", "general.Enable").c_str(), &Configs.Player.Enable);
    ImGui::SameLine();
    ColorPickerWithText(LOC("menu", "general.TextColor").c_str(), &Configs.Player.TextColor);

    ImGui::SliderInt(LOC("menu", "general.MaxDistance").c_str(), &Configs.Player.MaxDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    if (Configs.Player.Enable)
    {
        ImGui::Checkbox(LOC("menu", "general.Name").c_str(), &Configs.Player.Name);
        ImGui::SameLine();
        ImGui::Checkbox(LOC("menu", "general.Distance").c_str(), &Configs.Player.Distance);
        ImGui::SameLine();
        ImGui::Checkbox(LOC("menu", "players.HP").c_str(), &Configs.Player.HP);
    }

    ImGui::Checkbox(LOC("menu", "players.ShowDead").c_str(), &Configs.Player.ShowDead);
    ImGui::SliderInt(LOC("menu", "players.DeadMaxDistance").c_str(), &Configs.Player.DeadMaxDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    if (Configs.Player.Enable)
    {
        ImGui::Checkbox(LOC("menu", "players.DrawFriendlyHP").c_str(), &Configs.Player.DrawFriendsHP);
        //ImGui::SameLine();
        //ColorPickerWithText(LOC("menu", "players.FriendColor").c_str(), &Configs.Player.FriendColor);
    }

    if (Configs.Player.Enable)
        RenderFontSizeSlider(LOC("menu", "general.TextSize").c_str(), Configs.Player.FontSize);

    ImGui::Separator();

    std::vector<std::string> chamModes = {
        LOC("menu", "chams.OutlineRed"), LOC("menu", "chams.OutlineBlue"), LOC("menu", "chams.OutlineYellow"),
        LOC("menu", "chams.OutlineOrange"), LOC("menu", "chams.OutlineCyan"), LOC("menu", "chams.OutlineMagenta"),
        LOC("menu", "chams.OutlineWhite"), LOC("menu", "chams.FilledRed"), LOC("menu", "chams.FilledBlue"),
        LOC("menu", "chams.FilledYellow"), LOC("menu", "chams.FilledOrange"), LOC("menu", "chams.FilledCyan"),
        LOC("menu", "chams.FilledMagenta"), LOC("menu", "chams.FilledWhite")
    };
    if (ImGui::BeginCombo(LOC("menu", "players.ChamMode").c_str(), chamModes[Configs.Player.ChamMode].c_str())) {
        for (int n = 0; n < chamModes.size(); n++) {
            bool is_selected = (Configs.Player.ChamMode == n);
            if (ImGui::Selectable(chamModes[n].c_str(), is_selected))
                Configs.Player.ChamMode = n;
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    if (ImGui::Button(LOC("menu", "players.WriteChams").c_str())) {
        Configs.Player.Chams = true;
    }

    ImGui::Separator();

    ImGui::Checkbox(LOC("menu", "players.DrawFrames").c_str(), &Configs.Player.DrawFrames);
    ImGui::SameLine();
    ColorPickerWithText(LOC("menu", "players.FramesColor").c_str(), &Configs.Player.FramesColor);

    if (Configs.Player.DrawFrames)
        ImGui::Checkbox(LOC("menu", "players.DrawHead").c_str(), &Configs.Player.DrawHeadInFrames);

    if (Configs.Player.Enable || Configs.Player.DrawFrames)
        ImGui::Checkbox(LOC("menu", "players.DrawHealth").c_str(), &Configs.Player.DrawHealthBars);

    ImGui::EndChild();
}

void ImGuiMenu::RenderBossesESPTab() {
    ImGui::BeginChild("BossesESPTab", ImVec2(0, 0), false);

    ImGui::Checkbox(LOC("menu", "general.Enable").c_str(), &Configs.Bosses.Enable);
    ImGui::SameLine();
    ColorPickerWithText(LOC("menu", "general.TextColor").c_str(), &Configs.Bosses.TextColor);

    ImGui::Checkbox(LOC("menu", "general.Name").c_str(), &Configs.Bosses.Name);
    ImGui::SameLine();
    ImGui::Checkbox(LOC("menu", "general.Distance").c_str(), &Configs.Bosses.Distance);

    ImGui::SliderInt(LOC("menu", "general.MaxDistance").c_str(), &Configs.Bosses.MaxDistance, 0, 1500, LOC("menu", "general.Meters").c_str());
    RenderFontSizeSlider(LOC("menu", "general.TextSize").c_str(), Configs.Bosses.FontSize);

    ImGui::EndChild();
}

void ImGuiMenu::RenderSupplyESPTab() {
    ImGui::BeginChild("SupplyESPTab", ImVec2(0, 0), false);

    ImGui::Checkbox(LOC("menu", "general.Enable").c_str(), &Configs.Supply.Enable);
    ImGui::SameLine();
    ColorPickerWithText(LOC("menu", "general.TextColor").c_str(), &Configs.Supply.TextColor);

    ImGui::Checkbox(LOC("menu", "general.Name").c_str(), &Configs.Supply.Name);
    ImGui::SameLine();
    ImGui::Checkbox(LOC("menu", "general.Distance").c_str(), &Configs.Supply.Distance);

    ImGui::SliderInt(LOC("menu", "general.MaxDistance").c_str(), &Configs.Supply.MaxDistance, 0, 1500, LOC("menu", "general.Meters").c_str());
    RenderFontSizeSlider(LOC("menu", "general.TextSize").c_str(), Configs.Supply.FontSize);

    ImGui::Separator();

    // Supply Types Column 1
    ImGui::BeginGroup();
    ImGui::Text(LOC("menu", "supply.AmmoTypes").c_str());
    ImGui::Checkbox(LOC("menu", "supply.ShowSpecialAmmo").c_str(), &Configs.Supply.ShowSpecialAmmo);
    ImGui::Checkbox(LOC("menu", "supply.ShowCompactAmmo").c_str(), &Configs.Supply.ShowCompactAmmo);
    ImGui::Checkbox(LOC("menu", "supply.ShowMediumAmmo").c_str(), &Configs.Supply.ShowMediumAmmo);
    ImGui::Checkbox(LOC("menu", "supply.ShowLongAmmo").c_str(), &Configs.Supply.ShowLongAmmo);
    ImGui::Checkbox(LOC("menu", "supply.ShowShotgunAmmo").c_str(), &Configs.Supply.ShowShortgunAmmo);
    ImGui::EndGroup();

    // Supply Types Column 2
    ImGui::SameLine(250 * Configs.General.UIScale);
    ImGui::BeginGroup();
    ImGui::Text(LOC("menu", "supply.OtherTypes").c_str());
    ImGui::Checkbox(LOC("menu", "supply.ShowAmmoSwapBox").c_str(), &Configs.Supply.ShowAmmoSwapBox);
    ImGui::Checkbox(LOC("menu", "supply.ShowMedkit").c_str(), &Configs.Supply.ShowMedkit);
    ImGui::Checkbox(LOC("menu", "supply.ShowYellowSupplyBox").c_str(), &Configs.Supply.ShowSupplyBox);
    ImGui::EndGroup();

    ImGui::EndChild();
}

void ImGuiMenu::RenderBloodBondsESPTab() {
    ImGui::BeginChild("BloodBondsESPTab", ImVec2(0, 0), false);

    ImGui::Checkbox(LOC("menu", "general.Enable").c_str(), &Configs.BloodBonds.Enable);
    ImGui::SameLine();
    HelpMarker(LOC("menu", "bb.HelpMarker").c_str());
    ImGui::SameLine();
    ColorPickerWithText(LOC("menu", "general.TextColor").c_str(), &Configs.BloodBonds.TextColor);

    ImGui::Checkbox(LOC("menu", "general.Name").c_str(), &Configs.BloodBonds.Name);
    ImGui::SameLine();
    ImGui::Checkbox(LOC("menu", "general.Distance").c_str(), &Configs.BloodBonds.Distance);

    ImGui::SliderInt(LOC("menu", "general.MaxDistance").c_str(), &Configs.BloodBonds.MaxDistance, 0, 1500, LOC("menu", "general.Meters").c_str());
    RenderFontSizeSlider(LOC("menu", "general.TextSize").c_str(), Configs.BloodBonds.FontSize);

    ImGui::EndChild();
}

void ImGuiMenu::RenderTrapESPTab() {
    ImGui::BeginChild("TrapESPTab", ImVec2(0, 0), false);

    ImGui::Checkbox(LOC("menu", "general.Enable").c_str(), &Configs.Trap.Enable);
    ImGui::SameLine();
    ColorPickerWithText(LOC("menu", "traps.TrapColor").c_str(), &Configs.Trap.TrapColor);
    ImGui::SameLine();
    ColorPickerWithText(LOC("menu", "traps.BarrelColor").c_str(), &Configs.Trap.BarrelColor);

    ImGui::Checkbox(LOC("menu", "general.Name").c_str(), &Configs.Trap.Name);
    ImGui::SameLine();
    ImGui::Checkbox(LOC("menu", "general.Distance").c_str(), &Configs.Trap.Distance);

    ImGui::SliderInt(LOC("menu", "general.MaxDistance").c_str(), &Configs.Trap.MaxDistance, 0, 1500, LOC("menu", "general.Meters").c_str());
    RenderFontSizeSlider(LOC("menu", "general.TextSize").c_str(), Configs.Trap.FontSize);

    // Traps Column
    ImGui::BeginGroup();
    ImGui::Text(LOC("menu", "traps.Traps").c_str());
    ImGui::Checkbox(LOC("menu", "traps.ShowBeartraps").c_str(), &Configs.Trap.ShowBeartrap);
    ImGui::Checkbox(LOC("menu", "traps.ShowTripmines").c_str(), &Configs.Trap.ShowTripmines);
    ImGui::Checkbox(LOC("menu", "traps.ShowDarksightDynamite").c_str(), &Configs.Trap.ShowDarksightDynamite);
    ImGui::EndGroup();

    // Barrels Column
    ImGui::SameLine(250 * Configs.General.UIScale);
    ImGui::BeginGroup();
    ImGui::Text(LOC("menu", "traps.Barrels").c_str());
    ImGui::Checkbox(LOC("menu", "traps.ShowGunpowderBarrels").c_str(), &Configs.Trap.ShowGunpowderBurrels);
    ImGui::Checkbox(LOC("menu", "traps.ShowOilBarrels").c_str(), &Configs.Trap.ShowOilBurrels);
    ImGui::Checkbox(LOC("menu", "traps.ShowBioBarrels").c_str(), &Configs.Trap.ShowBioBurrels);
    ImGui::EndGroup();

    ImGui::EndChild();
}

void ImGuiMenu::RenderPOIESPTab() {
    ImGui::BeginChild("POIESPTab", ImVec2(0, 0), false);

    ImGui::Checkbox(LOC("menu", "general.Enable").c_str(), &Configs.POI.Enable);
    ImGui::SameLine();
    ColorPickerWithText(LOC("menu", "general.TextColor").c_str(), &Configs.POI.TextColor);

    ImGui::Checkbox(LOC("menu", "general.Name").c_str(), &Configs.POI.Name);
    ImGui::SameLine();
    ImGui::Checkbox(LOC("menu", "general.Distance").c_str(), &Configs.POI.Distance);

    ImGui::SliderInt(LOC("menu", "general.MaxDistance").c_str(), &Configs.POI.MaxDistance, 0, 1500, LOC("menu", "general.Meters").c_str());
    RenderFontSizeSlider(LOC("menu", "general.TextSize").c_str(), Configs.POI.FontSize);

    // POI Types Column 1
    ImGui::BeginGroup();
    ImGui::Text(LOC("menu", "poi.MainPOIs").c_str());
    ImGui::Checkbox(LOC("menu", "poi.ShowResupplyPoints").c_str(), &Configs.POI.ShowResupplyStation);
    ImGui::Checkbox(LOC("menu", "poi.ShowExtractionPoints").c_str(), &Configs.POI.ShowExtraction);
    ImGui::Checkbox(LOC("menu", "poi.ShowCashRegisters").c_str(), &Configs.POI.ShowCashRegisters);
    ImGui::Checkbox(LOC("menu", "poi.ShowPouches").c_str(), &Configs.POI.ShowPouches);
    ImGui::Checkbox(LOC("menu", "poi.ShowClues").c_str(), &Configs.POI.ShowClues);
    ImGui::EndGroup();

    // POI Types Column 2
    ImGui::SameLine(250 * Configs.General.UIScale);
    ImGui::BeginGroup();
    ImGui::Text(LOC("menu", "poi.AdditionalPOIs").c_str());
    ImGui::Checkbox(LOC("menu", "poi.ShowPosters").c_str(), &Configs.POI.ShowPosters);
    ImGui::Checkbox(LOC("menu", "poi.ShowBlueprints").c_str(), &Configs.POI.ShowBlueprints);
    ImGui::Checkbox(LOC("menu", "poi.ShowGunOil").c_str(), &Configs.POI.ShowGunOil);
    ImGui::Checkbox(LOC("menu", "poi.ShowSeasonalDestructibles").c_str(), &Configs.POI.ShowSeasonalDestructibles);
    ImGui::SameLine();
    HelpMarker(LOC("menu", "poi.ShowSeasonalDestructiblesInfo").c_str());
    ImGui::EndGroup();

    ImGui::EndChild();
}

void ImGuiMenu::RenderTraitESPTab() {
    ImGui::BeginChild("TraitESPTab", ImVec2(0, 0), false);

    ImGui::BeginGroup();

    ImGui::Checkbox(LOC("menu", "general.Enable").c_str(), &Configs.Traits.Enable);
    ImGui::SameLine();
    ColorPickerWithText(LOC("menu", "general.TextColor").c_str(), &Configs.Traits.TraitColor);
    
    ImGui::Checkbox(LOC("menu", "general.Name").c_str(), &Configs.Traits.Name);
    ImGui::SameLine();
    ImGui::Checkbox(LOC("menu", "general.Distance").c_str(), &Configs.Traits.Distance);

    RenderFontSizeSlider(LOC("menu", "general.TextSize").c_str(), Configs.Traits.FontSize);

    ImGui::EndGroup();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::BeginGroup();

    ImGui::Checkbox(LOC("trait", "blademancer").c_str(), &Configs.Traits.EnableBlademancer);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "blademancer").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##blademancer").c_str(), &Configs.Traits.BlademancerDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "corpseseer").c_str(), &Configs.Traits.EnableCorpseseer);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "corpseseer").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##corpseseer").c_str(), &Configs.Traits.CorpseseerDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "gunrunner").c_str(), &Configs.Traits.EnableGunrunner);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "gunrunner").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##gunrunner").c_str(), &Configs.Traits.GunrunnerDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::EndGroup();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::BeginGroup();

    ImGui::Checkbox(LOC("trait", "berserker").c_str(), &Configs.Traits.EnableBerserker);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "berserker").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##berserker").c_str(), &Configs.Traits.BerserkerDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "deathcheat").c_str(), &Configs.Traits.EnableDeathcheat);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "deathcheat").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##deathcheat").c_str(), &Configs.Traits.DeathcheatDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "necromancer").c_str(), &Configs.Traits.EnableNecromancer);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "necromancer").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##necromancer").c_str(), &Configs.Traits.NecromancerDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "rampage").c_str(), &Configs.Traits.EnableRampage);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "rampage").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##rampage").c_str(), &Configs.Traits.RampageDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "relentless").c_str(), &Configs.Traits.EnableRelentless);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "relentless").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##relentless").c_str(), &Configs.Traits.RelentlessDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "remedy").c_str(), &Configs.Traits.EnableRemedy);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "remedy").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##remedy").c_str(), &Configs.Traits.RemedyDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "shadow").c_str(), &Configs.Traits.EnableShadow);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "shadow").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##shadow").c_str(), &Configs.Traits.ShadowDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "shadowleap").c_str(), &Configs.Traits.EnableShadowleap);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "shadowleap").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##shadowleap").c_str(), &Configs.Traits.ShadowleapDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::EndGroup();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::BeginGroup();

    ImGui::Checkbox(LOC("trait", "beastface").c_str(), &Configs.Traits.EnableBeastface);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "beastface").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##beastface").c_str(), &Configs.Traits.BeastfaceDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "bloodless").c_str(), &Configs.Traits.EnableBloodless);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "bloodless").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##bloodless").c_str(), &Configs.Traits.BloodlessDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "bulletgrubber").c_str(), &Configs.Traits.EnableBulletgrubber);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "bulletgrubber").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##bulletgrubber").c_str(), &Configs.Traits.BulletgrubberDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "conduit").c_str(), &Configs.Traits.EnableConduit);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "conduit").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##conduit").c_str(), &Configs.Traits.ConduitDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "determination").c_str(), &Configs.Traits.EnableDetermination);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "determination").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##determination").c_str(), &Configs.Traits.DeterminationDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "doctor").c_str(), &Configs.Traits.EnableDoctor);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "doctor").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##doctor").c_str(), &Configs.Traits.DoctorDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "fanning").c_str(), &Configs.Traits.EnableFanning);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "fanning").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##fanning").c_str(), &Configs.Traits.FanningDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "fastfingers").c_str(), &Configs.Traits.EnableFastfingers);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "fastfingers").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##fastfingers").c_str(), &Configs.Traits.FastfingersDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "gatorlegs").c_str(), &Configs.Traits.EnableGatorlegs);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "gatorlegs").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##gatorlegs").c_str(), &Configs.Traits.GatorlegsDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "ghoul").c_str(), &Configs.Traits.EnableGhoul);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "ghoul").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##ghoul").c_str(), &Configs.Traits.GhoulDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "greyhound").c_str(), &Configs.Traits.EnableGreyhound);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "greyhound").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##greyhound").c_str(), &Configs.Traits.GreyhoundDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "levering").c_str(), &Configs.Traits.EnableLevering);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "levering").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##levering").c_str(), &Configs.Traits.LeveringDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "lightfoot").c_str(), &Configs.Traits.EnableLightfoot);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "lightfoot").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##lightfoot").c_str(), &Configs.Traits.LightfootDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "magpie").c_str(), &Configs.Traits.EnableMagpie);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "magpie").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##magpie").c_str(), &Configs.Traits.MagpieDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "packmule").c_str(), &Configs.Traits.EnablePackmule);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "packmule").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##packmule").c_str(), &Configs.Traits.PackmuleDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "physician").c_str(), &Configs.Traits.EnablePhysician);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "physician").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##physician").c_str(), &Configs.Traits.PhysicianDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "pitcher").c_str(), &Configs.Traits.EnablePitcher);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "pitcher").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##pitcher").c_str(), &Configs.Traits.PitcherDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "quartermaster").c_str(), &Configs.Traits.EnableQuartermaster);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "quartermaster").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##quartermaster").c_str(), &Configs.Traits.QuartermasterDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "resilience").c_str(), &Configs.Traits.EnableResilience);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "resilience").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##resilience").c_str(), &Configs.Traits.ResilienceDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "salveskin").c_str(), &Configs.Traits.EnableSalveskin);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "salveskin").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##salveskin").c_str(), &Configs.Traits.SalveskinDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "serpent").c_str(), &Configs.Traits.EnableSerpent);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "serpent").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##serpent").c_str(), &Configs.Traits.SerpentDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "vigor").c_str(), &Configs.Traits.EnableVigor);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "vigor").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##vigor").c_str(), &Configs.Traits.VigorDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "whispersmith").c_str(), &Configs.Traits.EnableWhispersmith);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "whispersmith").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##whispersmith").c_str(), &Configs.Traits.WhispersmithDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::Checkbox(LOC("trait", "witness").c_str(), &Configs.Traits.EnableWitness);
    if (ImGui::IsItemHovered()) { ImGui::BeginTooltip(); ImGui::Text(LOC("trait", "witness").c_str()); ImGui::EndTooltip(); }
    ImGui::SameLine(120 * Configs.General.UIScale);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##witness").c_str(), &Configs.Traits.WitnessDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::EndGroup();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::BeginGroup();

    ImGui::Checkbox(LOC("menu", "traits.Other").c_str(), &Configs.Traits.EnableOther);
    ImGui::SliderInt((LOC("menu", "general.MaxDistance") + "##other").c_str(), &Configs.Traits.OtherDistance, 0, 1500, LOC("menu", "general.Meters").c_str());

    ImGui::EndGroup();

    ImGui::EndChild();
}

void ImGuiMenu::RenderOverlayTab() {
    ImGui::BeginChild("OverlayTab", ImVec2(0, 0), false);

    ImGui::BeginGroup();
    ImGui::Text(LOC("menu", "overlay.ResolutionSettings").c_str());
    ImGui::Checkbox(LOC("menu", "overlay.OverrideResolution").c_str(), &Configs.General.OverrideResolution);

    if (Configs.General.OverrideResolution)
    {
        ImGui::InputInt(LOC("menu", "overlay.ScreenWidth").c_str(), &Configs.General.Width);
        ImGui::InputInt(LOC("menu", "overlay.ScreenHeight").c_str(), &Configs.General.Height);
    }
    ImGui::EndGroup();

    ImGui::Separator();

    ImGui::BeginGroup();
    ImGui::Text(LOC("menu", "overlay.Radar").c_str());
    ImGui::Checkbox(LOC("menu", "overlay.DrawMapRadar").c_str(), &Configs.Overlay.DrawRadar);
    ImGui::SameLine();
    ImGui::Checkbox(LOC("menu", "overlay.DrawSelf").c_str(), &Configs.Overlay.RadarDrawSelf);
    ImGui::SameLine();
    ColorPickerWithText(LOC("menu", "overlay.PlayerColor").c_str(), &Configs.Overlay.PlayerRadarColor);
    ImGui::SameLine();
    ColorPickerWithText(LOC("menu", "overlay.EnemyColor").c_str(), &Configs.Overlay.EnemyRadarColor);
    ImGui::EndGroup();

    ImGui::Separator();

    ImGui::BeginGroup();
    ImGui::Text(LOC("menu", "overlay.PlayerList").c_str());
    ImGui::Checkbox(LOC("menu", "overlay.ShowPlayerList").c_str(), &Configs.Player.ShowPlayerList);
    ImGui::SameLine();
    ColorPickerWithText(LOC("menu", "overlay.PlayerListColor").c_str(), &Configs.Player.PlayerListColor);
    RenderFontSizeSlider(LOC("menu", "overlay.PlayerListSize").c_str(), Configs.Player.PlayerListFontSize);
    ImGui::EndGroup();

    ImGui::Separator();

    ImGui::BeginGroup();
    ImGui::Text(LOC("menu", "overlay.FPSCounter").c_str());
    ImGui::Checkbox(LOC("menu", "overlay.ShowFPS").c_str(), &Configs.Overlay.ShowFPS);
    ImGui::SameLine();
    ColorPickerWithText(LOC("menu", "overlay.FPSColor").c_str(), &Configs.Overlay.FpsColor);
    RenderFontSizeSlider(LOC("menu", "overlay.FPSFontSize").c_str(), Configs.Overlay.FpsFontSize);
    ImGui::EndGroup();

    ImGui::Separator();

    ImGui::BeginGroup();
    ImGui::Text(LOC("menu", "overlay.ObjectCounter").c_str());
    ImGui::Checkbox(LOC("menu", "overlay.ShowObjectCount").c_str(), &Configs.Overlay.ShowObjectCount);
    ImGui::SameLine();
    ColorPickerWithText(LOC("menu", "overlay.CountColor").c_str(), &Configs.Overlay.ObjectCountColor);
    RenderFontSizeSlider(LOC("menu", "overlay.CountFontSize").c_str(), Configs.Overlay.ObjectCountFontSize);
    ImGui::EndGroup();

    ImGui::Separator();

    ImGui::BeginGroup();
    ImGui::Text(LOC("menu", "overlay.CrosshairSettings").c_str());
    std::vector<std::string> crosshairTypes = {
        LOC("menu", "crosshair.None"), LOC("menu", "crosshair.FilledCircle"), LOC("menu", "crosshair.OutlineCircle"),
        LOC("menu", "crosshair.FilledRect"), LOC("menu", "crosshair.OutlineRect")
    };
    if (ImGui::BeginCombo(LOC("menu", "overlay.CrosshairType").c_str(), crosshairTypes[Configs.Overlay.CrosshairType].c_str())) {
        for (int n = 0; n < crosshairTypes.size(); n++) {
            bool is_selected = (Configs.Overlay.CrosshairType == n);
            if (ImGui::Selectable(crosshairTypes[n].c_str(), is_selected))
                Configs.Overlay.CrosshairType = n;
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    if (Configs.Overlay.CrosshairType != 0) {  // If not "None"
        ImGui::SliderInt(LOC("menu", "overlay.CrosshairSize").c_str(), &Configs.Overlay.CrosshairSize, 1, 20);
        ColorPickerWithText(LOC("menu", "overlay.CrosshairColor").c_str(), &Configs.Overlay.CrosshairColor);
    }
    ImGui::EndGroup();

    ImGui::EndChild();
}

void ImGuiMenu::RenderAimbotTab() {
    ImGui::BeginChild("AimbotTab", ImVec2(0, 0), false);

    // Main settings
    ImGui::BeginGroup();
    ImGui::Checkbox(LOC("menu", "general.Enable").c_str(), &Configs.Aimbot.Enable);
    ImGui::SameLine(); HelpMarker("Enables aimbot functionality");

    ImGui::Checkbox("Target Players", &Configs.Aimbot.TargetPlayers);
    ImGui::SameLine(); HelpMarker("Only target player entities");
    ImGui::EndGroup();

    ImGui::Separator();

    // Visual settings
    ImGui::BeginGroup();
    ImGui::Checkbox("Draw FOV", &Configs.Aimbot.DrawFOV);
    if (Configs.Aimbot.DrawFOV) {
        ImGui::SameLine();
        ColorPickerWithText("FOV Color", &Configs.Aimbot.FOVColor);
    }

    ImGui::SliderInt("FOV", &Configs.Aimbot.FOV, 1, 1000, "%d°");
    ImGui::SameLine(); HelpMarker("Field of View in degrees where aimbot will activate");
    ImGui::EndGroup();

    ImGui::Separator();

    // Distance settings
    ImGui::BeginGroup();
    ImGui::SliderInt(LOC("menu", "general.MaxDistance").c_str(), &Configs.Aimbot.MaxDistance, 0, 1500, LOC("menu", "general.Meters").c_str());
    ImGui::SameLine(); HelpMarker("Maximum distance to target");

    static const char* priorityTypes[] = {
        "Distance", "Crosshair", "Both"
    };
    ImGui::Combo("Priority", &Configs.Aimbot.Priority, priorityTypes, IM_ARRAYSIZE(priorityTypes));
    ImGui::SameLine(); HelpMarker(
        "Distance: Prioritize closest targets\n"
        "Crosshair: Prioritize targets near crosshair\n"
        "Both: Balance between distance and crosshair"
    );
    ImGui::EndGroup();

    ImGui::Separator();

    // Key binding
    ImGui::BeginGroup();
    HotKey("Aim Key", &Configs.Aimbot.Aimkey);
    ImGui::SameLine(); HelpMarker("Key to activate aimbot");
    ImGui::EndGroup();

    ImGui::Separator();

    // Kmbox connection
    ImGui::BeginGroup();
    if (ImGui::Button("Connect To Kmbox")) {
        kmbox::KmboxInitialize("");
    }
    ImGui::SameLine();
    if (kmbox::connected) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Connected");
    }
    else {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Not Connected");
    }
    ImGui::EndGroup();

    ImGui::EndChild();
}

static bool showDevSettings = false;
void ImGuiMenu::RenderSettingsTab() {
    ImGui::BeginChild("SettingsTab", ImVec2(0, 0), false);

    if (ImGui::Button(LOC("menu", "settings.SaveConfig").c_str())) {
        SaveConfig(ConfigPath);
        ImGui::OpenPopup("ConfigSaved");
    }
    if (ImGui::BeginPopup("ConfigSaved",
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text(LOC("menu", "settings.SaveConfigPopup").c_str());
        ImGui::EndPopup();
    }

    ImGui::SameLine();

    if (ImGui::Button(LOC("menu", "settings.LoadConfig").c_str())) {
        LoadConfig(ConfigPath);
        ImGui::OpenPopup("ConfigLoaded");
    }
    if (ImGui::BeginPopup("ConfigLoaded",
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text(LOC("menu", "settings.LoadConfigPopup").c_str());
        ImGui::EndPopup();
    }

    ImGui::Separator();

    {
        ImGui::Text(LOC("menu", "settings.Language").c_str());
        ImGui::SameLine();

        // Get current language name for display
        std::string currentLang = Configs.General.Language;

        if (ImGui::BeginCombo("##Language", currentLang.c_str())) {
            std::vector<std::string> availableLanguages;
            availableLanguages.reserve(Localization::Languages.size());

            for (const auto& kv : Localization::Languages) {
                availableLanguages.push_back(kv.first.c_str());
            }

            for (const auto& lang : availableLanguages) {
                bool isSelected = (currentLang == lang);
                if (ImGui::Selectable(lang.c_str(), isSelected)) {
                    Configs.General.Language = lang;

                    // Reload localization with new language
                    Localization::Initialize();

                    // Save config to persist language choice
                    SaveConfig(ConfigPath);
                }

                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text(LOC("menu", "settings.LanguageInfo").c_str());
            ImGui::EndTooltip();
        }
    }

    ImGui::Separator();

    if (ImGui::Checkbox(LOC("menu", "settings.OverlayMode").c_str(), &Configs.General.OverlayMode))
    {
        LOG_INFO("Changing OverlayMode mode to %s. Restart needed.", Configs.General.OverlayMode ? L"True" : L"False");
        SaveConfig(ConfigPath);
        exit(0);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text(LOC("menu", "settings.OverlayModeInfo").c_str());
        ImGui::EndTooltip();
    }

    ImGui::SameLine();

    if (ImGui::Checkbox(LOC("menu", "settings.PreventRecording").c_str(), &Configs.General.PreventRecording)) {
        BOOL status = SetWindowDisplayAffinity((HWND)ImGui::GetMainViewport()->PlatformHandle, Configs.General.PreventRecording ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE);
        if (!status) {
            LOG_WARNING("Failed to SetWindowDisplayAffinity");
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        // https://stackoverflow.com/questions/74572938/setwindowdisplayaffinity-causes-nvidia-instant-replay-to-turn-off
        ImGui::Text(LOC("menu", "settings.PreventRecordingInfo").c_str());
        ImGui::EndTooltip();
    }

    ImGui::Separator();

    ImGui::InputFloat(LOC("menu", "settings.UIScale").c_str(), &Configs.General.UIScale);
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text(LOC("menu", "settings.UIScaleInfo").c_str());
        ImGui::EndTooltip();
    }

    ImGui::Separator();

    ImGui::Checkbox(LOC("menu", "settings.CrosshairLowerPosition").c_str(), &Configs.General.CrosshairLowerPosition);
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text(LOC("menu", "settings.CrosshairLowerPositionInfo").c_str());
        ImGui::EndTooltip();
    }

    ImGui::Separator();

    ImGui::Checkbox(LOC("menu", "settings.ShowDevSettings").c_str(), &showDevSettings);

    if (showDevSettings)
    {
        ImGui::Checkbox(LOC("menu", "settings.WriteEntitiesDump").c_str(), &createEntitiesDump);
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text(LOC("menu", "settings.WriteEntitiesDumpInfo").c_str());
            ImGui::EndTooltip();
        }
    }    

    ImGui::Separator();

    HotKey(LOC("menu", "settings.OpenCloseMenuKey").c_str(), &Configs.General.OpenMenuKey);

    ImGui::Checkbox(LOC("menu", "settings.CloseMenuOnEsc").c_str(), &Configs.General.CloseMenuOnEsc);

    ImGui::Separator();

    if (ImGui::Button(LOC("menu", "settings.ExitApp").c_str(), ImVec2(100 * Configs.General.UIScale, 100 * Configs.General.UIScale))) {
        exit(0);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text(LOC("menu", "settings.ExitAppInfo").c_str());
        ImGui::EndTooltip();
    }

    ImGui::EndChild();
}

bool ImGuiMenu::RenderFontSizeSlider(const char* label, int& configValue)
{
    bool changed = false;
    static const auto& sizes = ESPRenderer::GetFontSizes();

    int currentIndex = 0;
    for (size_t i = 0; i < sizes.size(); i++) {
        if (sizes[i] == configValue) {
            currentIndex = static_cast<int>(i);
            break;
        }
    }

    ImGui::BeginGroup();
    {
        ImGui::Text("%s", label);
        ImGui::SameLine();

        ImGui::PushID(label);
        if (ImGui::SliderInt("##v", &currentIndex, 0, static_cast<int>(sizes.size() - 1), ""))
        {
            configValue = sizes[currentIndex];
            changed = true;
        }
        ImGui::PopID();

        ImGui::SameLine();
        ImGui::Text(LOC("menu", "general.Pixels").c_str(), configValue);
    }
    ImGui::EndGroup();

    return changed;
}