#pragma once

#include <services/commands/Command.hpp>

#include <chain/chain.hpp>

namespace services::commands {
    template<typename CommandType>
    using CommandConsumer = chain::Consumer<CommandPtr<CommandType>>;
    
    template<typename CommandType>
    using CommandProducer = chain::Producer<CommandPtr<CommandType>>;
}