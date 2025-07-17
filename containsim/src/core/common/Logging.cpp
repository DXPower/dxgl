#include <common/Logging.hpp>

#include <spdlog/sinks/stdout_color_sinks.h>

using namespace logging;

namespace {
    Sink common_sink{};
    Level default_level{Level::warn};
}

Logger logging::CreateLogger(std::string name) {
    Logger logger(std::move(name), common_sink);
    logger.set_level(default_level);
    return logger;
}

SharedLogger logging::CreateSharedLogger(std::string name) {
    auto logger = std::make_shared<Logger>(std::move(name), common_sink);
    logger->set_level(default_level);
    return logger;
}

void logging::SetCommonSink(Sink sink) {
    common_sink = std::move(sink);
}

Sink logging::GetCommonSink() {
    return common_sink;
}

Sink logging::CreateConsoleSink() {
    auto sink =  std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    sink->set_level(default_level);
    return sink;
}

void logging::SetDefaultLevel(Level level) {
    default_level = level;
}


