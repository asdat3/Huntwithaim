#include "pch.h"
#include "Logger.h"
#include <DbgHelp.h>
#pragma comment(lib, "dbghelp.lib")

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

void Logger::Init(const std::string& logFileName) {
    std::lock_guard<std::recursive_mutex> lock(logMutex);

    if (isInitialized) {
        return;
    }

    currentLogFileName = logFileName;
    logFile.open(logFileName, std::ios::out | std::ios::trunc);

    if (!logFile.is_open()) {
        MessageBoxA(NULL, "Failed to open log file", "Logger Error", MB_OK | MB_ICONERROR);
        return;
    }

    isInitialized = true;
    SetupCrashHandler();

    LOG_INFO("Logger initialized successfully");
}

void Logger::Log(LogLevel level, const char* format, ...) {
    if (!isInitialized) return;
    std::lock_guard<std::recursive_mutex> lock(logMutex);

    char buffer[4096];
    va_list args;
    va_start(args, format);
    if (level != LogLevel::Debug)
    {
        vprintf(format, args);
        puts("");
    }
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    std::string message = GetTimeStamp() + " [" + LogLevelToString(level) + "] " + buffer + "\n";
    WriteToFile(message);
}

void Logger::LogDebug(const char* format, ...) {
    if (!isInitialized) return;
    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    Log(LogLevel::Debug, buffer);
}

void Logger::LogInfo(const char* format, ...) {
    if (!isInitialized) return;
    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    Log(LogLevel::Info, buffer);
}

void Logger::LogWarning(const char* format, ...) {
    if (!isInitialized) return;
    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    Log(LogLevel::Warning, buffer);
}

void Logger::LogError(const char* format, ...) {
    if (!isInitialized) return;
    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    Log(LogLevel::Error, buffer);
}

void Logger::LogCritical(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    Log(LogLevel::Critical, buffer);
}

const char* Logger::LogLevelToString(LogLevel level) {
    switch (level) {
    case LogLevel::Debug: return "DEBUG";
    case LogLevel::Info: return "INFO";
    case LogLevel::Warning: return "WARNING";
    case LogLevel::Error: return "ERROR";
    case LogLevel::Critical: return "CRITICAL";
    default: return "UNKNOWN";
    }
}

std::string Logger::GetTimeStamp() {
    auto now = std::chrono::system_clock::now();
    auto timer = std::chrono::system_clock::to_time_t(now);
    struct tm timeinfo;
    localtime_s(&timeinfo, &timer);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return std::string(buffer);
}

void Logger::WriteToFile(const std::string& message) {
    if (logFile.is_open()) {
        logFile << message;
        logFile.flush();
    }
}

void Logger::SetupCrashHandler() {
    SetUnhandledExceptionFilter(UnhandledExceptionFilter);
    LOG_INFO("Crash handler initialized");
}

LONG WINAPI Logger::UnhandledExceptionFilter(EXCEPTION_POINTERS* exceptionPointers) {
    Logger& logger = Logger::GetInstance();

    logger.LogCritical("Application crashed!");
    logger.LogCritical("Exception code: 0x%08X", exceptionPointers->ExceptionRecord->ExceptionCode);
    logger.LogCritical("Exception address: 0x%p", exceptionPointers->ExceptionRecord->ExceptionAddress);

    std::string dumpPath = "crash_" + std::to_string(std::time(nullptr)) + ".dmp";
    WriteMiniDump(dumpPath, exceptionPointers);

    logger.LogCritical("Minidump written to: %s", dumpPath.c_str());

    // Show error message to user
    std::string errorMsg = "The application has crashed. Please check the log file and crash dump for more information.\n";
    errorMsg += "Log: " + logger.currentLogFileName + "\n";
    errorMsg += "Dump: " + dumpPath;

    MessageBoxA(NULL, errorMsg.c_str(), "Hunt DMA Crash", MB_OK | MB_ICONERROR);

    return EXCEPTION_EXECUTE_HANDLER;
}

void Logger::WriteMiniDump(const std::string& path, EXCEPTION_POINTERS* exceptionPointers) {
    HANDLE hFile = CreateFileA(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        LOG_ERROR("Failed to create dump file");
        return;
    }

    MINIDUMP_EXCEPTION_INFORMATION mei;
    mei.ThreadId = GetCurrentThreadId();
    mei.ExceptionPointers = exceptionPointers;
    mei.ClientPointers = TRUE;

    MiniDumpWriteDump(
        GetCurrentProcess(),
        GetCurrentProcessId(),
        hFile,
        MiniDumpNormal,
        &mei,
        NULL,
        NULL
    );

    CloseHandle(hFile);
}