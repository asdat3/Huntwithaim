#pragma once
#include "pch.h"

class Logger {
public:
    enum class LogLevel {
        Debug,
        Info,
        Warning,
        Error,
        Critical
    };

    static Logger& GetInstance() {
        static Logger instance;
        return instance;
    }

    void Init(const std::string& logFileName = "logs_huntdma.log");
    void Log(LogLevel level, const char* format, ...);
    void LogDebug(const char* format, ...);
    void LogInfo(const char* format, ...);
    void LogWarning(const char* format, ...);
    void LogError(const char* format, ...);
    void LogCritical(const char* format, ...);

    // Crash handling
    void SetupCrashHandler();
    static void WriteMiniDump(const std::string& path, EXCEPTION_POINTERS* exceptionPointers);
    static LONG WINAPI UnhandledExceptionFilter(EXCEPTION_POINTERS* exceptionPointers);

private:
    Logger() = default;
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::recursive_mutex logMutex;
    std::ofstream logFile;
    std::string currentLogFileName;
    bool isInitialized = false;

    const char* LogLevelToString(LogLevel level);
    std::string GetTimeStamp();
    void WriteToFile(const std::string& message);
};