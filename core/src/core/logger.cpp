#include "logger.hpp"
#include "spdlog/logger.h"
#include "spdlog/spdlog.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <stdarg.h>

internal_variable spdlog::logger* core_logger = nullptr;
internal_variable spdlog::logger* client_logger = nullptr;

b8 log_init() {
    // Create core logger
    auto core_log = spdlog::stdout_color_mt("prometheus_core");
    core_logger = core_log.get();

    // Create client logger
    auto client_log = spdlog::stdout_color_mt("client_app");
    client_logger = client_log.get();

    // Set default logger to core
    spdlog::set_default_logger(core_log);
    spdlog::set_level(spdlog::level::trace);
    spdlog::set_pattern("%^[%Y-%m-%d %H:%M:%S.%e] [%-7l] [%n] %v%$");

    return true;
}

void log_shutdown() {
    spdlog::shutdown();
    core_logger = nullptr;
    client_logger = nullptr;
}

void log_output(
    Log_Scope scope,
    Log_Level level,
    const char* message, ...) {

    // Select the appropriate logger based on scope
    spdlog::logger* logger = nullptr;

    switch (scope) {
    case Log_Scope::CORE:
        logger = core_logger;
        break;
    case Log_Scope::CLIENT:
        logger = client_logger;
        break;
    default:
        logger = core_logger;
        break;
    }

    if (!logger) {
        printf("[LOG] %s\n", message);
        return;
    }

    // Handle variable arguments for spdlog
    va_list args;
    va_start(args, message);
    char formatted_message[4096];

    vsnprintf(formatted_message, sizeof(formatted_message), message, args);

    va_end(args);

    // Use spdlog's built-in formatting functions
    switch (level) {
    case Log_Level::FATAL:
        logger->critical(formatted_message);
        break;
    case Log_Level::ERROR:
        logger->error(formatted_message);
        break;
    case Log_Level::WARN:
        logger->warn(formatted_message);
        break;
    case Log_Level::INFO:
        logger->info(formatted_message);
        break;
    case Log_Level::DEBUG:
        logger->debug(formatted_message);
        break;
    case Log_Level::TRACE:
        logger->trace(formatted_message);
        break;
    default:
        logger->info(formatted_message);
        break;
    }
}
