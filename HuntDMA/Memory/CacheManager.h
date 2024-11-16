#pragma once
#include "pch.h"

class CacheManager {
private:
    std::atomic<bool> should_run{ true };
    std::unique_ptr<std::thread> cache_thread;

    void CacheThreadFunction();

public:
    CacheManager() = default;
    ~CacheManager();

    void Start();
    void Stop();
};

// Global instance
extern CacheManager g_CacheManager;