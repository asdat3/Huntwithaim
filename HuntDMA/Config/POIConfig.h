#pragma once
#include "pch.h"
class POIConfig
{
    std::string ConfigName;

public:
    POIConfig(const std::string& name)
    {
        ConfigName = name;
    }
    bool Enable = true;
    bool Name = true;
    bool Distance = true;
    int MaxDistance = 900;
    bool ShowExtraction = true;
    bool ShowCashRegisters = false;
    bool ShowPouches = false;
    bool ShowPosters = false;
    bool ShowBlueprints = false;
    bool ShowGunOil = false;
    bool ShowClues = false;
    bool ShowTraits = true;
    D2D1::ColorF TextColour = Colour(0, 255, 221);
    int FontSize = 13;
    void ToJsonColour(json* j, const std::string& name, D2D1::ColorF* colour)
    {
        (*j)[ConfigName][name][LIT("r")] = colour->r;
        (*j)[ConfigName][name][LIT("g")] = colour->g;
        (*j)[ConfigName][name][LIT("b")] = colour->b;
        (*j)[ConfigName][name][LIT("a")] = colour->a;

    }
    void FromJsonColour(json j, const std::string& name, D2D1::ColorF* colour)
    {
        if (j[ConfigName].contains(name))
        {
            colour->r = j[ConfigName][name][LIT("r")];
            colour->g = j[ConfigName][name][LIT("g")];
            colour->b = j[ConfigName][name][LIT("b")];
            colour->a = j[ConfigName][name][LIT("a")];
        }
    }

    json ToJson()
    {
        json j;
        j[ConfigName][LIT("Enable")] = Enable;
        j[ConfigName][LIT("Name")] = Name;
        j[ConfigName][LIT("Distance")] = Distance;
        j[ConfigName][LIT("MaxDistance")] = MaxDistance;
        j[ConfigName][LIT("FontSize")] = FontSize;
        j[ConfigName][LIT("ShowExtraction")] = ShowExtraction;
        j[ConfigName][LIT("ShowCashRegisters")] = ShowCashRegisters;
        j[ConfigName][LIT("ShowPouches")] = ShowPouches;
        j[ConfigName][LIT("ShowPosters")] = ShowPosters;
        j[ConfigName][LIT("ShowBlueprints")] = ShowBlueprints;
        j[ConfigName][LIT("ShowGunOil")] = ShowGunOil;
        j[ConfigName][LIT("ShowClues")] = ShowClues;
        j[ConfigName][LIT("ShowTraits")] = ShowTraits;
        ToJsonColour(&j, LIT("TextColour"), &TextColour);

        return j;
    }
    void FromJson(const json& j)
    {
        if (!j.contains(ConfigName))
            return;
        if (j[ConfigName].contains(LIT("Enable")))
            Enable = j[ConfigName][LIT("Enable")];
        if (j[ConfigName].contains(LIT("Name")))
            Name = j[ConfigName][LIT("Name")];
        if (j[ConfigName].contains(LIT("Distance")))
            Distance = j[ConfigName][LIT("Distance")];
        if (j[ConfigName].contains(LIT("FontSize")))
            FontSize = j[ConfigName][LIT("FontSize")];
        if (j[ConfigName].contains(LIT("MaxDistance")))
            MaxDistance = j[ConfigName][LIT("MaxDistance")];
        if (j[ConfigName].contains(LIT("ShowExtraction")))
            ShowExtraction = j[ConfigName][LIT("ShowExtraction")];
        if (j[ConfigName].contains(LIT("ShowCashRegisters")))
            ShowCashRegisters = j[ConfigName][LIT("ShowCashRegisters")];
        if (j[ConfigName].contains(LIT("ShowPouches")))
            ShowPouches = j[ConfigName][LIT("ShowPouches")];
        if (j[ConfigName].contains(LIT("ShowPosters")))
            ShowPosters = j[ConfigName][LIT("ShowPosters")];
        if (j[ConfigName].contains(LIT("ShowBlueprints")))
            ShowBlueprints = j[ConfigName][LIT("ShowBlueprints")];
        if (j[ConfigName].contains(LIT("ShowGunOil")))
            ShowGunOil = j[ConfigName][LIT("ShowGunOil")];
        if (j[ConfigName].contains(LIT("ShowClues")))
            ShowClues = j[ConfigName][LIT("ShowClues")];
        if (j[ConfigName].contains(LIT("ShowTraits")))
            ShowTraits = j[ConfigName][LIT("ShowTraits")];
        FromJsonColour(j, LIT("TextColour"), &TextColour);
    }
};

