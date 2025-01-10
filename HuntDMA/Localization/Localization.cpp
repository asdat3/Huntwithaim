#include "pch.h"
#include "Localization.h"
#include "ConfigUtilities.h"
#include "Localization/en.h"
#include "Localization/ru.h"

void Localization::Initialize() {
    LOG_INFO("Initializing localization system...");

    if (!std::filesystem::exists(localizationPath)) {
        std::filesystem::create_directory(localizationPath);
    }
    CreateDefaultLocalizationFiles();

    LoadLanguageFile(Configs.General.Language);
}

void Localization::CreateDefaultLocalizationFiles() {
    for (const auto& [lang, data] : Languages) {
        std::string filePath = localizationPath + lang + ".json";

        json jsonData;
        jsonData["version"] = data.version;

        for (const auto& [category, translations] : data.translations) {
            jsonData[category] = translations;
        }

        if (std::filesystem::exists(filePath)) {
            try {
                std::ifstream existingFile(filePath);
                json existingData = json::parse(existingFile);

                if (!existingData.contains("version") ||
                    existingData["version"].get<int>() < data.version) {

                    std::ofstream outFile(filePath, std::ios::out | std::ios::binary);
                    if (outFile.is_open()) {
                        unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
                        outFile.write(reinterpret_cast<char*>(bom), sizeof(bom));

                        outFile << jsonData.dump(4, ' ', false, json::error_handler_t::replace);
                        LOG_INFO("Updated outdated localization file: %s", filePath.c_str());
                    }
                }
            }
            catch (const std::exception& e) {
                LOG_ERROR("Error checking existing localization file %s: %s",
                    filePath.c_str(), e.what());
            }
        }
        else {
            std::ofstream outFile(filePath, std::ios::out | std::ios::binary);
            if (outFile.is_open()) {
                unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
                outFile.write(reinterpret_cast<char*>(bom), sizeof(bom));

                outFile << jsonData.dump(4, ' ', false, json::error_handler_t::replace);
                LOG_INFO("Created new localization file: %s", filePath.c_str());
            }
        }
    }
}

void Localization::LoadLanguageFile(const std::string& language) {
    std::string filePath = localizationPath + language + ".json";

    if (!std::filesystem::exists(localizationPath)) {
        std::filesystem::create_directory(localizationPath);
    }

    LocalizationData defaultData = GetDefaultLocalization();
    translations = defaultData.translations;

    if (!std::filesystem::exists(filePath)) {
        LOG_ERROR("Language file not found: %s, using default language", filePath.c_str());
        return;
    }

    if (!std::filesystem::exists(filePath)) {
        LOG_WARNING("Language file not found: %s, creating new one...", filePath.c_str());

        LocalizationData data = Languages[language];

        json jsonData;
        jsonData["version"] = data.version;
        for (const auto& [category, temp_translations] : data.translations) {
            jsonData[category] = temp_translations;
        }

        std::ofstream outFile(filePath, std::ios::out | std::ios::binary);
        if (outFile.is_open()) {
            unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
            outFile.write(reinterpret_cast<char*>(bom), sizeof(bom));

            outFile << jsonData.dump(4, ' ', false, json::error_handler_t::replace);
            LOG_INFO("Created new localization file: %s", filePath.c_str());
        }
    }

    try {
        std::ifstream file(filePath, std::ios::binary);

        // Skip BOM if present
        unsigned char bom[3];
        file.read(reinterpret_cast<char*>(bom), 3);
        if (!(bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF)) {
            file.seekg(0);
        }

        json langData = json::parse(file);

        if (!langData.contains("version") ||
            langData["version"].get<int>() < LocalizationData::LOCALIZATION_VERSION) {

            LOG_WARNING("Localization file %s is outdated, recreating...", filePath.c_str());
            CreateDefaultLocalizationFiles();

            file = std::ifstream(filePath);
            langData = json::parse(file);
        }

        for (const auto& [category, items] : langData.items()) {
            if (category == "version") continue;

            for (const auto& [key, value] : items.items()) {
                translations[category][key] = value.get<std::string>();
            }
        }

        LOG_INFO("Loaded language file: %s", filePath.c_str());
    }
    catch (const std::exception& e) {
        LOG_ERROR("Failed to load language file %s: %s", filePath.c_str(), e.what());
    }
}

LocalizationData Localization::GetDefaultLocalization() {
    return Localizations::GetLocalization_EN();
}

std::string Localization::GetLocalizedString(const std::string& category, const std::string& key) {
    if (translations.count(category) && translations[category].count(key)) {
        return translations[category][key];
    }
    return category + "." + key; // Return category.key as fallback
}