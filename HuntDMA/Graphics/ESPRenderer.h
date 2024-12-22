#pragma once
#include "imgui.h"
#include "ConfigUtilities.h"

enum FontAlignment
{
    TopLeft = 0,
    TopCenter = 1,
    TopRight = 2,
    MiddleLeft = 3,
    Center = 4,
    MiddleRight = 5,
    BottomLeft = 6,
    BottomCenter = 7,
    BottomRight = 8,
};

class ESPRenderer {
private:
    static ImDrawList* s_drawList;
    static const std::vector<int> FONT_SIZES;
    static std::map<int, ImFont*> s_fonts;
    static ImFont* s_defaultFont;
    static float s_screenWidth;
    static float s_screenHeight;
    static ImVec2 s_screenCenter;

public:
    static bool Initialize();
    static void BeginFrame();

    static void DrawText(const ImVec2& pos, const std::string& text, const ImVec4& color, int fontSize = 13, FontAlignment alignment = TopLeft, float outline = 1.0f);
    static void DrawLine(const ImVec2& from, const ImVec2& to, const ImVec4& color, float thickness = 1.0f);
    static void DrawCircle(const ImVec2& center, float radius, const ImVec4& color, float thickness = 1.0f, bool filled = false);
    static void DrawRect(const ImVec2& min, const ImVec2& max, const ImVec4& color, float thickness = 1.0f, bool filled = false);

    static const std::vector<int>& GetFontSizes() {return FONT_SIZES;}

    static const ImVec2& GetScreenCenter() { return s_screenCenter; }
    static float GetScreenWidth() { return s_screenWidth; }
    static float GetScreenHeight() { return s_screenHeight; }

private:
    static ImFont* GetFont(int size);
};