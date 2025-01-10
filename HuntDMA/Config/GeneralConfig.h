#pragma once
#include "pch.h"
class GeneralConfig
{
    std::string ConfigName;

public:
    GeneralConfig(const std::string& name)
    {
        ConfigName = name;
    }
    bool OverrideResolution = false;
    int Width = (int)static_cast<float>(GetSystemMetrics(SM_CXSCREEN));
    int Height = (int)static_cast<float>(GetSystemMetrics(SM_CYSCREEN));
    bool OverlayMode = false;
    bool PreventRecording = false;
    bool CrosshairLowerPosition = false;
    float UIScale = (float)GetDpiForSystem() / (float)USER_DEFAULT_SCREEN_DPI;
    int OpenMenuKey = 36;
    bool CloseMenuOnEsc = true;
    std::string Language = "en";

    void ToJsonColor(json* j, const std::string& name, ImVec4* color)
    {
        (*j)[ConfigName][name][LIT("r")] = color->x;
        (*j)[ConfigName][name][LIT("g")] = color->y;
        (*j)[ConfigName][name][LIT("b")] = color->z;
        (*j)[ConfigName][name][LIT("a")] = color->w;
    }

    void FromJsonColor(json j, const std::string& name, ImVec4* color)
    {
        if (j[ConfigName].contains(name))
        {
            color->x = j[ConfigName][name][LIT("r")];
            color->y = j[ConfigName][name][LIT("g")];
            color->z = j[ConfigName][name][LIT("b")];
            color->w = j[ConfigName][name][LIT("a")];
        }
    }

    json ToJson()
    {
        json j;
        j[ConfigName][LIT("OverrideResolution")] = OverrideResolution;
        j[ConfigName][LIT("Width")] = Width;
        j[ConfigName][LIT("Height")] = Height;
        j[ConfigName][LIT("OverlayMode")] = OverlayMode;
        j[ConfigName][LIT("PreventRecording")] = PreventRecording;
        j[ConfigName][LIT("CrosshairLowerPosition")] = CrosshairLowerPosition;
        j[ConfigName][LIT("UIScale")] = UIScale;
        j[ConfigName][LIT("OpenMenuKey")] = OpenMenuKey;
        j[ConfigName][LIT("CloseMenuOnEsc")] = CloseMenuOnEsc;
        j[ConfigName][LIT("Language")] = Language;

        return j;
    }

    void FromJson(const json& j)
    {
        if (!j.contains(ConfigName))
            return;
        if (j[ConfigName].contains(LIT("OverrideResolution")))
            OverrideResolution = j[ConfigName][LIT("OverrideResolution")];
        if (j[ConfigName].contains(LIT("Width")))
            Width = j[ConfigName][LIT("Width")];
        if (j[ConfigName].contains(LIT("Height")))
            Height = j[ConfigName][LIT("Height")];
        if (j[ConfigName].contains(LIT("OverlayMode")))
            OverlayMode = j[ConfigName][LIT("OverlayMode")];
        if (j[ConfigName].contains(LIT("PreventRecording")))
            PreventRecording = j[ConfigName][LIT("PreventRecording")];
        if (j[ConfigName].contains(LIT("CrosshairLowerPosition")))
            CrosshairLowerPosition = j[ConfigName][LIT("CrosshairLowerPosition")];
        if (j[ConfigName].contains(LIT("UIScale")))
            UIScale = j[ConfigName][LIT("UIScale")];
        if (j[ConfigName].contains(LIT("OpenMenuKey")))
            OpenMenuKey = j[ConfigName][LIT("OpenMenuKey")];
        if (j[ConfigName].contains(LIT("CloseMenuOnEsc")))
            CloseMenuOnEsc = j[ConfigName][LIT("CloseMenuOnEsc")];
        if (j[ConfigName].contains(LIT("Language")))
            Language = j[ConfigName][LIT("Language")];
    }
};

