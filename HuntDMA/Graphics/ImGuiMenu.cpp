#include "Pch.h"
#include "ImGuiMenu.h"
#include "Input.h"
#include "Init.h"
#include "Kmbox.h"
#include "Globals.h"
#include "ESPRenderer.h"

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

        static char buffer[32];
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
    ImGui::TextDisabled("(?)");
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
    char buf[64];
    sprintf_s(buf, "%s [%s]", label, ImGuiUtils::GetKeyName(*key));
    
    if (ImGui::Button(buf)) {
        ImGui::OpenPopup("Select Key");
    }

    // Modular window for choosing key
    if (ImGui::BeginPopupModal("Select Key", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Press any key or click to cancel");
        
        // Check key input
        for (int i = ImGuiKey_NamedKey_BEGIN; i < ImGuiKey_NamedKey_END; i++) {
            if (ImGui::IsKeyPressed((ImGuiKey)i)) {
                *key = ImGuiUtils::ImGuiKeyToVirtualKey((ImGuiKey)i);
                changed = true;
                ImGui::CloseCurrentPopup();
            }
        }

        // Check mouse input
        if (ImGui::IsMouseClicked(0) || ImGui::IsMouseClicked(1)) {
            ImGui::CloseCurrentPopup();
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

    ImGui::Begin("Hunt DMA", &MenuOpen, ImGuiWindowFlags_NoCollapse);

    if (ImGui::BeginTabBar("##Tabs")) {
        if (ImGui::BeginTabItem("Player ESP")) {
            RenderPlayerESPTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Bosses ESP")) {
            RenderBossesESPTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Supply ESP")) {
            RenderSupplyESPTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("BB ESP")) {
            RenderBloodBondsESPTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Trap ESP")) {
            RenderTrapESPTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("POI ESP")) {
            RenderPOIESPTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Overlay")) {
            RenderOverlayTab();
            ImGui::EndTabItem();
        }
        if (enableAimBot && ImGui::BeginTabItem("Aimbot")) {
            RenderAimbotTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Settings")) {
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

    ImGui::Checkbox("Enable", &Configs.Player.Enable);
    ImGui::SameLine();
    ColorPickerWithText("Text Color", &Configs.Player.TextColor);

    ImGui::SliderInt("Max Distance", &Configs.Player.MaxDistance, 0, 1500, "%d m");

    if (Configs.Player.Enable)
    {
        ImGui::Checkbox("Name", &Configs.Player.Name);
        ImGui::SameLine();
        ImGui::Checkbox("Distance", &Configs.Player.Distance);
        ImGui::SameLine();
        ImGui::Checkbox("HP", &Configs.Player.HP);
    }

    ImGui::Checkbox("Show Dead", &Configs.Player.ShowDead);
    ImGui::SliderInt("Dead Max Distance", &Configs.Player.DeadMaxDistance, 0, 1500, "%d m");

    if (Configs.Player.Enable || Configs.Player.DrawFrames)
    {
        ImGui::Checkbox("Draw Friendly", &Configs.Player.DrawFriends);
        ImGui::SameLine();
        ColorPickerWithText("Friend Color", &Configs.Player.FriendColor);
    }

    if (Configs.Player.Enable)
        RenderFontSizeSlider("Text Size", Configs.Player.FontSize);

    ImGui::Separator();

    const char* chamModes[] = {
        "Outline Red", "Outline Blue", "Outline Yellow",
        "Outline Orange", "Outline Cyan", "Outline Magenta",
        "Outline White", "Filled Red", "Filled Blue",
        "Filled Yellow", "Filled Orange", "Filled Cyan",
        "Filled Magenta", "Filled White"
    };
    ImGui::Combo("Cham Mode", &Configs.Player.ChamMode, chamModes, IM_ARRAYSIZE(chamModes));

    if (ImGui::Button("Write Chams (enable/update)")) {
        Configs.Player.Chams = true;
    }

    ImGui::Separator();

    ImGui::Checkbox("Draw Frames", &Configs.Player.DrawFrames);
    ImGui::SameLine();
    ColorPickerWithText("Frames Color", &Configs.Player.FramesColor);

    if (Configs.Player.DrawFrames)
        ImGui::Checkbox("Draw Head in frames", &Configs.Player.DrawHeadInFrames);

    if (Configs.Player.Enable || Configs.Player.DrawFrames)
        ImGui::Checkbox("Draw Health bars", &Configs.Player.DrawHealthBars);

    ImGui::EndChild();
}

void ImGuiMenu::RenderBossesESPTab() {
    ImGui::BeginChild("BossesESPTab", ImVec2(0, 0), false);

    ImGui::Checkbox("Enable", &Configs.Bosses.Enable);
    ImGui::SameLine();
    ColorPickerWithText("Text Color", &Configs.Bosses.TextColor);

    ImGui::Checkbox("Name", &Configs.Bosses.Name);
    ImGui::Checkbox("Distance", &Configs.Bosses.Distance);

    ImGui::SliderInt("Max Distance", &Configs.Bosses.MaxDistance, 0, 1500, "%d m");
    RenderFontSizeSlider("Text Size", Configs.Bosses.FontSize);

    ImGui::EndChild();
}

void ImGuiMenu::RenderSupplyESPTab() {
    ImGui::BeginChild("SupplyESPTab", ImVec2(0, 0), false);

    ImGui::Checkbox("Enable", &Configs.Supply.Enable);
    ImGui::SameLine();
    ColorPickerWithText("Text Color", &Configs.Supply.TextColor);

    ImGui::Checkbox("Name", &Configs.Supply.Name);
    ImGui::SameLine();
    ImGui::Checkbox("Distance", &Configs.Supply.Distance);

    ImGui::SliderInt("Max Distance", &Configs.Supply.MaxDistance, 0, 1500, "%d m");
    RenderFontSizeSlider("Text Size", Configs.Supply.FontSize);

    ImGui::Separator();

    // Supply Types Column 1
    ImGui::BeginGroup();
    ImGui::Text("Ammo Types:");
    ImGui::Checkbox("Show Special Ammo", &Configs.Supply.ShowSpecialAmmo);
    ImGui::Checkbox("Show Compact Ammo", &Configs.Supply.ShowCompactAmmo);
    ImGui::Checkbox("Show Medium Ammo", &Configs.Supply.ShowMediumAmmo);
    ImGui::Checkbox("Show Long Ammo", &Configs.Supply.ShowLongAmmo);
    ImGui::Checkbox("Show Shotgun Ammo", &Configs.Supply.ShowShortgunAmmo);
    ImGui::EndGroup();

    // Supply Types Column 2
    ImGui::SameLine(250 * Configs.General.UIScale);
    ImGui::BeginGroup();
    ImGui::Text("Other Types:");
    ImGui::Checkbox("Show AmmoSwap Box", &Configs.Supply.ShowAmmoSwapBox);
    ImGui::Checkbox("Show Medkit", &Configs.Supply.ShowMedkit);
    ImGui::Checkbox("Show Yellow Supply Box", &Configs.Supply.ShowSupplyBox);
    ImGui::EndGroup();

    ImGui::EndChild();
}

void ImGuiMenu::RenderBloodBondsESPTab() {
    ImGui::BeginChild("BloodBondsESPTab", ImVec2(0, 0), false);

    ImGui::Checkbox("Enable", &Configs.BloodBonds.Enable);
    ImGui::SameLine();
    HelpMarker("Golden cash register ESP");
    ImGui::SameLine();
    ColorPickerWithText("Text Color", &Configs.BloodBonds.TextColor);

    ImGui::Checkbox("Name", &Configs.BloodBonds.Name);
    ImGui::SameLine();
    ImGui::Checkbox("Distance", &Configs.BloodBonds.Distance);

    ImGui::SliderInt("Max Distance", &Configs.BloodBonds.MaxDistance, 0, 1500, "%d m");
    RenderFontSizeSlider("Text Size", Configs.BloodBonds.FontSize);

    ImGui::EndChild();
}

void ImGuiMenu::RenderTrapESPTab() {
    ImGui::BeginChild("TrapESPTab", ImVec2(0, 0), false);

    ImGui::Checkbox("Enable", &Configs.Trap.Enable);
    ImGui::SameLine();
    ColorPickerWithText("Trap Color", &Configs.Trap.TrapColor);
    ImGui::SameLine();
    ColorPickerWithText("Barrel Color", &Configs.Trap.BarrelColor);

    ImGui::Checkbox("Name", &Configs.Trap.Name);
    ImGui::Checkbox("Distance", &Configs.Trap.Distance);
    ImGui::SliderInt("Max Distance", &Configs.Trap.MaxDistance, 0, 1500, "%d m");
    RenderFontSizeSlider("Text Size", Configs.Trap.FontSize);

    // Traps Column
    ImGui::BeginGroup();
    ImGui::Text("Traps:");
    ImGui::Checkbox("Show Beartraps", &Configs.Trap.ShowBeartrap);
    ImGui::Checkbox("Show Tripmines", &Configs.Trap.ShowTripmines);
    ImGui::Checkbox("Show Darksight Dynamite", &Configs.Trap.ShowDarksightDynamite);
    ImGui::EndGroup();

    // Barrels Column
    ImGui::SameLine(250 * Configs.General.UIScale);
    ImGui::BeginGroup();
    ImGui::Text("Barrels:");
    ImGui::Checkbox("Show Gunpowder Barrels", &Configs.Trap.ShowGunpowderBurrels);
    ImGui::Checkbox("Show Oil Barrels", &Configs.Trap.ShowOilBurrels);
    ImGui::Checkbox("Show Bio Barrels", &Configs.Trap.ShowBioBurrels);
    ImGui::EndGroup();

    ImGui::EndChild();
}

void ImGuiMenu::RenderPOIESPTab() {
    ImGui::BeginChild("POIESPTab", ImVec2(0, 0), false);

    ImGui::Checkbox("Enable", &Configs.POI.Enable);
    ImGui::SameLine();
    ColorPickerWithText("Text Color", &Configs.POI.TextColor);

    ImGui::Checkbox("Name", &Configs.POI.Name);
    ImGui::Checkbox("Distance", &Configs.POI.Distance);
    ImGui::SliderInt("Max Distance", &Configs.POI.MaxDistance, 0, 1500, "%d m");
    RenderFontSizeSlider("Text Size", Configs.POI.FontSize);

    // POI Types Column 1
    ImGui::BeginGroup();
    ImGui::Text("Main POIs:");
    ImGui::Checkbox("Show Extraction Points", &Configs.POI.ShowExtraction);
    ImGui::Checkbox("Show Cash Registers", &Configs.POI.ShowCashRegisters);
    ImGui::Checkbox("Show Pouches", &Configs.POI.ShowPouches);
    ImGui::Checkbox("Show Clues", &Configs.POI.ShowClues);
    ImGui::EndGroup();

    // POI Types Column 2
    ImGui::SameLine(250 * Configs.General.UIScale);
    ImGui::BeginGroup();
    ImGui::Text("Additional POIs:");
    ImGui::Checkbox("Show Posters", &Configs.POI.ShowPosters);
    ImGui::Checkbox("Show Blueprints", &Configs.POI.ShowBlueprints);
    ImGui::Checkbox("Show Gun Oil", &Configs.POI.ShowGunOil);
    ImGui::Checkbox("Show Traits", &Configs.POI.ShowTraits);
    ImGui::Checkbox("Show Seasonal destructibles", &Configs.POI.ShowSeasonalDestructibles);
    ImGui::SameLine();
    HelpMarker("Pumpkins, Balloons, etc");
    ImGui::EndGroup();

    ImGui::EndChild();
}

void ImGuiMenu::RenderOverlayTab() {
    ImGui::BeginChild("OverlayTab", ImVec2(0, 0), false);

    ImGui::BeginGroup();
    ImGui::Text("Resolution Settings:");
    ImGui::Checkbox("Override W2S Resolution", &Configs.General.OverrideResolution);

    if (Configs.General.OverrideResolution)
    {
        ImGui::InputInt("Screen Width", &Configs.General.Width);
        ImGui::InputInt("Screen Height", &Configs.General.Height);
    }
    
    ImGui::EndGroup();

    ImGui::Separator();

    ImGui::BeginGroup();
    ImGui::Text("Player List:");
    ImGui::Checkbox("Show Player List", &Configs.Player.ShowPlayerList);
    ImGui::SameLine();
    ColorPickerWithText("Player List Color", &Configs.Player.PlayerListColor);
    RenderFontSizeSlider("Player List Size", Configs.Player.PlayerListFontSize);
    ImGui::EndGroup();

    ImGui::Separator();

    ImGui::BeginGroup();
    ImGui::Text("FPS Counter:");
    ImGui::Checkbox("Show FPS", &Configs.Overlay.ShowFPS);
    ImGui::SameLine();
    ColorPickerWithText("FPS Color", &Configs.Overlay.FpsColor);
    RenderFontSizeSlider("FPS Font Size", Configs.Overlay.FpsFontSize);
    ImGui::EndGroup();

    ImGui::Separator();

    ImGui::BeginGroup();
    ImGui::Text("Object Counter:");
    ImGui::Checkbox("Show Object Count", &Configs.Overlay.ShowObjectCount);
    ImGui::SameLine();
    ColorPickerWithText("Count Color", &Configs.Overlay.ObjectCountColor);
    RenderFontSizeSlider("Count Font Size", Configs.Overlay.ObjectCountFontSize);
    ImGui::EndGroup();

    ImGui::Separator();

    ImGui::BeginGroup();
    ImGui::Text("Crosshair Settings:");
    static const char* crosshairTypes[] = {
        "None", "Filled Circle", "Outline Circle",
        "Filled Rect", "Outline Rect"
    };
    ImGui::Combo("Crosshair Type", &Configs.Overlay.CrosshairType, crosshairTypes, IM_ARRAYSIZE(crosshairTypes));

    if (Configs.Overlay.CrosshairType != 0) {  // If not "None"
        ImGui::SliderInt("Crosshair Size", &Configs.Overlay.CrosshairSize, 1, 20);
        ColorPickerWithText("Crosshair Color", &Configs.Overlay.CrosshairColor);
    }
    ImGui::EndGroup();

    ImGui::EndChild();
}

void ImGuiMenu::RenderAimbotTab() {
    ImGui::BeginChild("AimbotTab", ImVec2(0, 0), false);

    // Main settings
    ImGui::BeginGroup();
    ImGui::Checkbox("Enable", &Configs.Aimbot.Enable);
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
    ImGui::SliderInt("Max Distance", &Configs.Aimbot.MaxDistance, 0, 1500, "%d m");
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

    if (ImGui::Button("Save Config")) {
        SaveConfig(ConfigPath);
        ImGui::OpenPopup("ConfigSaved");
    }
    if (ImGui::BeginPopup("ConfigSaved",
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text("Configuration saved successfully!");
        ImGui::EndPopup();
    }

    ImGui::SameLine();

    if (ImGui::Button("Load Config")) {
        LoadConfig(ConfigPath);
        ImGui::OpenPopup("ConfigLoaded");
    }
    if (ImGui::BeginPopup("ConfigLoaded",
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text("Configuration loaded successfully!");
        ImGui::EndPopup();
    }

    ImGui::Separator();

    if (ImGui::Checkbox("Overlay Mode", &Configs.General.OverlayMode))
    {
        LOG_INFO("Changing OverlayMode mode to %s. Restart needed.", Configs.General.OverlayMode ? L"True" : L"False");
        SaveConfig(ConfigPath);
        exit(0);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("NEED APP RESTART!\n"
            "If changed, will save config and close this app.\n\n"
            "For single pc setup.\n"
            "Draws application on top of everything.\n"
            "Transparency is available (and possible) only in Overlay mode.\n"
            "Works even in Hunt's fullscreen mode.");
        ImGui::EndTooltip();
    }

    ImGui::SameLine();

    if (ImGui::Checkbox("Prevent recording", &Configs.General.PreventRecording)) {
        BOOL status = SetWindowDisplayAffinity((HWND)ImGui::GetMainViewport()->PlatformHandle, Configs.General.PreventRecording ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE);
        if (!status) {
            LOG_WARNING("Failed to SetWindowDisplayAffinity");
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        // https://stackoverflow.com/questions/74572938/setwindowdisplayaffinity-causes-nvidia-instant-replay-to-turn-off
        ImGui::Text("Hide app from recording.\nNvidia ShadowPlay's stupid policy stops recording\nif sees a window with prevent recording flag (can't record kills).");
        ImGui::EndTooltip();
    }

    ImGui::Separator();

    ImGui::InputFloat("UI Scale", &Configs.General.UIScale);
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("Default is set to you DPI scale.\nNeed restart to apply.");
        ImGui::EndTooltip();
    }

    ImGui::Separator();

    ImGui::Checkbox("Crosshair lower position", &Configs.General.CrosshairLowerPosition);
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("Check if you're using crosshair alt position so overlay can render crosshair in the correct position. Also used for Aimbot.");
        ImGui::EndTooltip();
    }

    ImGui::Separator();

    ImGui::Checkbox("Show dev settings", &showDevSettings);

    if (showDevSettings)
    {
        ImGui::Checkbox("Write entities dump", &createEntitiesDump);
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("Dumps all entity class names to classes-dump.txt\nCheckbox resets after app restart.");
            ImGui::EndTooltip();
        }
    }    

    ImGui::Separator();

    HotKey("Change key to open/close menu", &Configs.General.OpenMenuKey);

    ImGui::Checkbox("Close menu on Esc", &Configs.General.CloseMenuOnEsc);

    ImGui::Separator();

    if (ImGui::Button("Exit App", ImVec2(100 * Configs.General.UIScale, 100 * Configs.General.UIScale))) {
        exit(0);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("Closes the application immediately");
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
        ImGui::Text("%d px", configValue);
    }
    ImGui::EndGroup();

    return changed;
}