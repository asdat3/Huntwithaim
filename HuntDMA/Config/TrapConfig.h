#pragma once
#include "pch.h"
class TrapConfig
{
    std::string ConfigName;

public:
    TrapConfig(const std::string& name)
    {
        ConfigName = name;
    }
    bool Enable = true;
    bool Name = true;
    bool Distance = true;
    int MaxDistance = 75;
    bool ShowBeartrap = true;
    bool ShowTripmines = true;
    bool ShowDarksightDynamite = true;
    bool ShowGunpowderBurrels = false;
    bool ShowOilBurrels = false;
    bool ShowBioBurrels = false;
    D2D1::ColorF TrapColour = Colour(255, 0, 0);
    D2D1::ColorF BarrelColour = Colour(245, 60, 0);
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
        j[ConfigName][LIT("ShowBeartrap")] = ShowBeartrap;
        j[ConfigName][LIT("ShowTripmines")] = ShowTripmines;
        j[ConfigName][LIT("ShowDarksightDynamite")] = ShowDarksightDynamite;
        j[ConfigName][LIT("ShowGunpowderBurrels")] = ShowGunpowderBurrels;
        j[ConfigName][LIT("ShowOilBurrels")] = ShowOilBurrels;
        j[ConfigName][LIT("ShowBioBurrels")] = ShowBioBurrels;
        ToJsonColour(&j, LIT("TrapColour"), &TrapColour);
        ToJsonColour(&j, LIT("BarrelColour"), &BarrelColour);

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
        if (j[ConfigName].contains(LIT("ShowBeartrap")))
            ShowBeartrap = j[ConfigName][LIT("ShowBeartrap")];
        if (j[ConfigName].contains(LIT("ShowTripmines")))
            ShowTripmines = j[ConfigName][LIT("ShowTripmines")];
        if (j[ConfigName].contains(LIT("ShowDarksightDynamite")))
            ShowDarksightDynamite = j[ConfigName][LIT("ShowDarksightDynamite")];
        if (j[ConfigName].contains(LIT("ShowGunpowderBurrels")))
            ShowGunpowderBurrels = j[ConfigName][LIT("ShowGunpowderBurrels")];
        if (j[ConfigName].contains(LIT("ShowOilBurrels")))
            ShowOilBurrels = j[ConfigName][LIT("ShowOilBurrels")];
        if (j[ConfigName].contains(LIT("ShowBioBurrels")))
            ShowBioBurrels = j[ConfigName][LIT("ShowBioBurrels")];
        FromJsonColour(j, LIT("TrapColour"), &TrapColour);
        FromJsonColour(j, LIT("BarrelColour"), &BarrelColour);
    }
};

