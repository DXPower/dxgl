#pragma once

#include <spdlog/sinks/sink.h>
#include <spdlog/logger.h>

#include <memory>
#include <string>

namespace logging {
    using Sink = std::shared_ptr<spdlog::sinks::sink>;
    using Logger = spdlog::logger;
    using SharedLogger = std::shared_ptr<spdlog::logger>;

    Logger CreateLogger(std::string name);
    SharedLogger CreateSharedLogger(std::string name);

    void SetCommonSink(Sink sink);
    Sink GetCommonSink();

    Sink CreateConsoleSink();
}