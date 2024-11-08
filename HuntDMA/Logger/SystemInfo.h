#pragma once
#include "pch.h"

// Forward declarations to avoid including heavy Windows headers
typedef BOOL(WINAPI* LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

class SystemInfo {
public:
    struct GPUInfo {
        std::string name;
        std::string driver_version;
        size_t dedicated_memory;
        std::string vendor_id;
    };

    struct SystemData {
        std::string os_version;
        std::string os_build;
        std::string cpu_name;
        uint32_t cpu_cores;
        uint32_t cpu_threads;
        uint64_t total_memory;
        uint64_t available_memory;
        std::vector<GPUInfo> gpu_info;
        std::string process_path;
        bool is_admin;
        std::string windows_username;
    };

    static SystemData CollectSystemInfo();
    static void LogSystemInfo();

private:
    static std::string GetOSVersion();
    static std::string GetCPUName();
    static uint32_t GetCPUCoreCount();
    static std::vector<GPUInfo> GetGPUInfo();
    static bool IsRunningAsAdmin();
    static std::string GetWindowsUsername();
    static std::string GetProcessPath();

    // Helper methods
    static std::string RegGetString(HKEY hKey, const std::string& subKey, const std::string& value);
    static DWORD RegGetDWORD(HKEY hKey, const std::string& subKey, const std::string& value);
    static std::string WMIQuery(const std::string& wmiClass, const std::string& property);
};