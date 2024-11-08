#include "Pch.h"
#include "ESPRenderer.h"
#include "XorStr.h"

// Static member initialization
ImDrawList* ESPRenderer::s_drawList = nullptr;
std::map<int, ImFont*> ESPRenderer::s_fonts;
ImFont* ESPRenderer::s_defaultFont = nullptr;
float ESPRenderer::s_screenWidth = 0.0f;
float ESPRenderer::s_screenHeight = 0.0f;
ImVec2 ESPRenderer::s_screenCenter = ImVec2(0.0f, 0.0f);

const char* fontPath = "C:\\Windows\\Fonts\\verdana.ttf"; // Verdana
const std::vector<int> ESPRenderer::FONT_SIZES = {
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 22, 24, 26, 28, 30, 40, 50, 60, 70, 80, 90, 100
};

bool ESPRenderer::Initialize()
{
    ImGuiIO& io = ImGui::GetIO();

    // Init default font
    s_defaultFont = io.Fonts->AddFontDefault();
    if (!s_defaultFont) {
        LOG_ERROR("[ESPRenderer] Failed to load default font");
        return false;
    }

    // Load all font sizes
    if (!std::filesystem::exists(fontPath)) {
        LOG_ERROR("[ESPRenderer] Font file not found at: %s", fontPath);
        return false;
    }

    for (int size : FONT_SIZES) {
        ImFontConfig config;
        config.SizePixels = static_cast<float>(size);

        ImFont* font = io.Fonts->AddFontFromFileTTF(fontPath,
            static_cast<float>(size),
            &config);

        if (font) {
            s_fonts[size] = font;
            //LOG_INFO("[ESPRenderer] Loaded font size: %d", size);
        }
        else {
            LOG_ERROR("[ESPRenderer] Failed to load font size: %d", size);
        }
    }

    // Atlas build
    bool buildSuccess = io.Fonts->Build();
    if (!buildSuccess) {
        LOG_ERROR("[ESPRenderer] Failed to build font atlas");
        return false;
    }

    LOG_INFO("[ESPRenderer] Successfully initialized with %zu fonts", s_fonts.size());
    return true;
}

void ESPRenderer::BeginFrame()
{
    s_drawList = ImGui::GetBackgroundDrawList();

    // Update screen dimensions if needed
    if (Configs.Overlay.OverrideResolution) {
        s_screenWidth = static_cast<float>(Configs.Overlay.Width);
        s_screenHeight = static_cast<float>(Configs.Overlay.Height);
    }
    else {
        s_screenWidth = static_cast<float>(GetSystemMetrics(SM_CXSCREEN));
        s_screenHeight = static_cast<float>(GetSystemMetrics(SM_CYSCREEN));
    }

    s_screenCenter = ImVec2(s_screenWidth * 0.5f, s_screenHeight * 0.5f);
}

void ESPRenderer::DrawText(const ImVec2& pos, const std::wstring& text, const ImVec4& color, int fontSize, FontAlignment alignment, float outline)
{
    if (!s_drawList) return;

    const ImU32 outlineColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0.05f, 0.05f, 0.05f, color.w));
    std::string utf8Text = ConvertWideToUTF8(text);
    ImFont* font = GetFont(fontSize);

    ImVec2 newPos;
    if (alignment == TopLeft)
        newPos = pos;
    else
    {
        ImVec2 textSize = font->CalcTextSizeA(static_cast<float>(fontSize), FLT_MAX, 0.0f, utf8Text.c_str());
        switch (alignment)
        {
            case TopLeft:
                newPos = ImVec2(pos.x, pos.y);
                break;
            case TopCenter:
                newPos = ImVec2(pos.x - textSize.x * 0.5f, pos.y);
                break;
            case TopRight:
                newPos = ImVec2(pos.x - textSize.x, pos.y);
                break;
            case MiddleLeft:
                newPos = ImVec2(pos.x, pos.y - textSize.y * 0.5f);
                break;
            case Center:
                newPos = ImVec2(pos.x - textSize.x * 0.5f, pos.y - textSize.y * 0.5f);
                break;
            case MiddleRight:
                newPos = ImVec2(pos.x - textSize.x, pos.y - textSize.y * 0.5f);
                break;
            case BottomLeft:
                newPos = ImVec2(pos.x, pos.y - textSize.y);
                break;
            case BottomCenter:
                newPos = ImVec2(pos.x - textSize.x * 0.5f, pos.y - textSize.y);
                break;
            case BottomRight:
                newPos = ImVec2(pos.x - textSize.x, pos.y - textSize.y);
                break;
            default:
                break;
        }
    }

    if (outline > 0.01f)
    {
        for (int dx = -1; dx <= 1; dx += 1) {
            for (int dy = -1; dy <= 1; dy += 1) {
                if (dx == 0 && dy == 0) continue;
                s_drawList->AddText(font,
                    static_cast<float>(fontSize),
                    ImVec2(newPos.x + (dx * outline), newPos.y + (dy * outline)),
                    outlineColor,
                    utf8Text.c_str());
            }
        }
    }

    s_drawList->AddText(font,
        static_cast<float>(fontSize),
        newPos,
        ImGui::ColorConvertFloat4ToU32(color),
        utf8Text.c_str());
}

void ESPRenderer::DrawLine(const ImVec2& from, const ImVec2& to, const ImVec4& color, float thickness)
{
    if (!s_drawList) return;
    s_drawList->AddLine(from, to, ImGui::ColorConvertFloat4ToU32(color), thickness);
}

void ESPRenderer::DrawCircle(const ImVec2& center, float radius, const ImVec4& color, float thickness, bool filled)
{
    if (!s_drawList) return;
    if (filled) {
        s_drawList->AddCircleFilled(center, radius, ImGui::ColorConvertFloat4ToU32(color));
    }
    else {
        s_drawList->AddCircle(center, radius, ImGui::ColorConvertFloat4ToU32(color), 0, thickness);
    }
}

void ESPRenderer::DrawRect(const ImVec2& min, const ImVec2& max, const ImVec4& color, float thickness, bool filled)
{
    if (!s_drawList) return;
    if (filled) {
        s_drawList->AddRectFilled(min, max, ImGui::ColorConvertFloat4ToU32(color));
    }
    else {
        s_drawList->AddRect(min, max, ImGui::ColorConvertFloat4ToU32(color), 0.0f, ImDrawFlags_None, thickness);
    }
}

std::string ESPRenderer::ConvertWideToUTF8(const std::wstring& wstr)
{
    if (wstr.empty()) return std::string();

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);

    return strTo;
}

ImFont* ESPRenderer::GetFont(int size)
{
    if (s_fonts.contains(size))
        return s_fonts[size];

    // Fallback font if load failed
    return s_defaultFont;
}