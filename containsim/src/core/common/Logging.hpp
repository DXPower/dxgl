#pragma once

#include <string_view>

#include <spdlog/sinks/sink.h>
#include <spdlog/logger.h>

namespace logging {
    using Sink = std::shared_ptr<spdlog::sinks::sink>;
    using Logger = spdlog::logger;

    Logger CreateLogger(std::string_view name);

    void SetCommonSink(Sink sink);
    Sink GetCommonSink();

    Sink CreateConsoleSink();
}