#include "pch.h"
#include "SystemInfo.h"
#include <wbemidl.h>
#include <comdef.h>
#include <Psapi.h>
#include <VersionHelpers.h>
#include <Lmcons.h>
#pragma comment(lib, "wbemuuid.lib")

std::string SystemInfo::GetOSVersion() {
    std::string result;

    // Get version using RtlGetVersion to bypass Windows version lying
    typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
    HMODULE ntdll = GetModuleHandleW(L"ntdll");
    if (!ntdll) {
        LOG_ERROR("Failed to get ntdll handle");
        return "Unknown Windows Version";
    }

    auto RtlGetVersion = (RtlGetVersionPtr)GetProcAddress(ntdll, "RtlGetVersion");
    if (!RtlGetVersion) {
        LOG_ERROR("Failed to get RtlGetVersion address");
        return "Unknown Windows Version";
    }

    RTL_OSVERSIONINFOW osvi = { 0 };
    osvi.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);
    RtlGetVersion(&osvi);

    // Windows 11 is actually Windows 10 version 22000 or higher
    if (osvi.dwMajorVersion == 10 && osvi.dwMinorVersion == 0) {
        // Read DisplayVersion from registry for more accurate info
        std::string displayVersion = RegGetString(HKEY_LOCAL_MACHINE,
            "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
            "DisplayVersion");

        std::string buildNumberStr = RegGetString(HKEY_LOCAL_MACHINE,
            "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
            "CurrentBuildNumber");

        int buildNumber = 0;
        try {
            buildNumber = std::stoi(buildNumberStr);
        }
        catch (const std::exception& e) {
            LOG_ERROR("Failed to convert build number: %s", e.what());
            buildNumber = 0;
        }

        if (buildNumber >= 22000) {
            result = "Windows 11";
            if (!displayVersion.empty()) {
                result += " " + displayVersion;
            }
        } else {
            result = "Windows 10";
            if (!displayVersion.empty()) {
                result += " " + displayVersion;
            }
        }
    }
    // Older Windows versions
    else if (osvi.dwMajorVersion == 6) {
        switch (osvi.dwMinorVersion) {
        case 3: result = "Windows 8.1"; break;
        case 2: result = "Windows 8"; break;
        case 1: result = "Windows 7"; break;
        case 0: result = "Windows Vista"; break;
        default: result = "Unknown Windows Version"; break;
        }
    }
    else {
        result = "Unknown Windows Version";
    }

    // Add build number
    result += " (Build " + std::to_string(osvi.dwBuildNumber) + ")";

    // Add architecture
    SYSTEM_INFO si;
    GetNativeSystemInfo(&si);
    result += (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
        si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM64) ? " 64-bit" : " 32-bit";

    return result;
}

std::string SystemInfo::GetCPUName() {
    int cpuInfo[4] = { -1 };
    char cpuBrandString[0x40];
    __cpuid(cpuInfo, 0x80000002);
    memcpy(cpuBrandString, cpuInfo, sizeof(cpuInfo));
    __cpuid(cpuInfo, 0x80000003);
    memcpy(cpuBrandString + 16, cpuInfo, sizeof(cpuInfo));
    __cpuid(cpuInfo, 0x80000004);
    memcpy(cpuBrandString + 32, cpuInfo, sizeof(cpuInfo));
    return cpuBrandString;
}

uint32_t SystemInfo::GetCPUCoreCount() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwNumberOfProcessors;
}

std::vector<SystemInfo::GPUInfo> SystemInfo::GetGPUInfo() {
    std::vector<GPUInfo> gpus;

    HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hr)) return gpus;

    IWbemLocator* pLoc = nullptr;
    hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);

    if (SUCCEEDED(hr)) {
        IWbemServices* pSvc = nullptr;
        hr = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);

        if (SUCCEEDED(hr)) {
            hr = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
                RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

            if (SUCCEEDED(hr)) {
                IEnumWbemClassObject* pEnumerator = nullptr;
                hr = pSvc->ExecQuery(bstr_t("WQL"),
                    bstr_t("SELECT * FROM Win32_VideoController"),
                    WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                    NULL,
                    &pEnumerator);

                if (SUCCEEDED(hr)) {
                    IWbemClassObject* pclsObj = nullptr;
                    ULONG uReturn = 0;

                    while (pEnumerator) {
                        hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
                        if (uReturn == 0) break;

                        GPUInfo gpu;
                        VARIANT vtProp;

                        // Get GPU Name
                        hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
                        if (SUCCEEDED(hr)) {
                            gpu.name = _bstr_t(vtProp.bstrVal);
                            VariantClear(&vtProp);
                        }

                        // Get Driver Version
                        hr = pclsObj->Get(L"DriverVersion", 0, &vtProp, 0, 0);
                        if (SUCCEEDED(hr)) {
                            gpu.driver_version = _bstr_t(vtProp.bstrVal);
                            VariantClear(&vtProp);
                        }

                        gpus.push_back(gpu);
                        pclsObj->Release();
                    }
                    pEnumerator->Release();
                }
            }
            pSvc->Release();
        }
        pLoc->Release();
    }

    CoUninitialize();
    return gpus;
}

// Helper method for getting registry DWORD values
DWORD SystemInfo::RegGetDWORD(HKEY hKey, const std::string& subKey, const std::string& value) {
    DWORD data = 0;
    DWORD dataSize = sizeof(DWORD);
    HKEY hSubKey;

    if (RegOpenKeyExA(hKey, subKey.c_str(), 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
        RegQueryValueExA(hSubKey, value.c_str(), nullptr, nullptr, reinterpret_cast<LPBYTE>(&data), &dataSize);
        RegCloseKey(hSubKey);
    }

    return data;
}

// Helper method for getting registry string values
std::string SystemInfo::RegGetString(HKEY hKey, const std::string& subKey, const std::string& value) {
    char data[1024];
    DWORD dataSize = sizeof(data);
    HKEY hSubKey;

    if (RegOpenKeyExA(hKey, subKey.c_str(), 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
        if (RegQueryValueExA(hSubKey, value.c_str(), nullptr, nullptr,
            reinterpret_cast<LPBYTE>(data), &dataSize) == ERROR_SUCCESS) {
            RegCloseKey(hSubKey);
            return std::string(data);
        }
        RegCloseKey(hSubKey);
    }

    return "";
}

bool SystemInfo::IsRunningAsAdmin() {
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }

    return isAdmin != FALSE;
}

std::string SystemInfo::GetWindowsUsername() {
    char username[UNLEN + 1];
    DWORD username_len = UNLEN + 1;
    if (GetUserNameA(username, &username_len)) {
        return std::string(username);
    }
    return "Unknown";
}

std::string SystemInfo::GetProcessPath() {
    char path[MAX_PATH];
    if (GetModuleFileNameA(NULL, path, MAX_PATH) != 0) {
        return std::string(path);
    }
    return "Unknown";
}

SystemInfo::SystemData SystemInfo::CollectSystemInfo() {
    SystemData data;

    data.os_version = GetOSVersion();
    data.os_build = RegGetString(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "BuildLabEx");
    data.cpu_name = GetCPUName();
    data.cpu_cores = GetCPUCoreCount();
    data.cpu_threads = std::thread::hardware_concurrency();

    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        data.total_memory = memInfo.ullTotalPhys;
        data.available_memory = memInfo.ullAvailPhys;
    }

    data.gpu_info = GetGPUInfo();
    data.process_path = GetProcessPath();
    data.is_admin = IsRunningAsAdmin();
    data.windows_username = GetWindowsUsername();

    return data;
}

void SystemInfo::LogSystemInfo() {
    LOG_DEBUG("=== System Information ===");

    SystemData sysInfo = CollectSystemInfo();

    LOG_DEBUG("OS: %s", sysInfo.os_version.c_str());
    LOG_DEBUG("Build: %s", sysInfo.os_build.c_str());
    LOG_DEBUG("CPU: %s", sysInfo.cpu_name.c_str());
    LOG_DEBUG("CPU Cores: %d", sysInfo.cpu_cores);
    LOG_DEBUG("CPU Threads: %d", sysInfo.cpu_threads);
    LOG_DEBUG("Total Memory: %.2f GB", sysInfo.total_memory / (1024.0 * 1024.0 * 1024.0));
    LOG_DEBUG("Available Memory: %.2f GB", sysInfo.available_memory / (1024.0 * 1024.0 * 1024.0));

    LOG_DEBUG("GPUs:");
    for (const auto& gpu : sysInfo.gpu_info) {
        LOG_DEBUG("  Name: %s", gpu.name.c_str());
        LOG_DEBUG("  Vendor ID: %s", gpu.vendor_id.c_str());
        LOG_DEBUG("  Dedicated memory: %zu MB", gpu.dedicated_memory);
        LOG_DEBUG("  Driver Version: %s", gpu.driver_version.c_str());
    }

    LOG_DEBUG("Process Path: %s", sysInfo.process_path.c_str());
    LOG_DEBUG("Admin Rights: %s", sysInfo.is_admin ? "Yes" : "No");

    LOG_DEBUG("========================");
}