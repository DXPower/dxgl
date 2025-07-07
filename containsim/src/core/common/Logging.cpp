#include <common/Logging.hpp>

#include <spdlog/sinks/stdout_color_sinks.h>

using namespace logging;

namespace {
    Sink common_sink{};
}

Logger logging::CreateLogger(std::string name) {
    return Logger(std::move(name), common_sink);
}

SharedLogger logging::CreateSharedLogger(std::string name) {
    return std::make_shared<Logger>(std::move(name), common_sink);
}

void logging::SetCommonSink(Sink sink) {
    common_sink = std::move(sink);
}

Sink logging::GetCommonSink() {
    return common_sink;
}

Sink logging::CreateConsoleSink() {
    return std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
}



