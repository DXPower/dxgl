#pragma once

#include <common/Action.hpp>
#include <chain/chain.hpp>

using ActionConsumer = chain::Consumer<Action, chain::ParamKind::Move>;
using ActionProducer = chain::Producer<Action, chain::ParamKind::Move>;
