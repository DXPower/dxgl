#include <common/DrawQueues.hpp>
#include <kangaru_ext/kgr.hpp>

struct DrawQueuesService : kgr::single_service<DrawQueues> { };
auto service_map(const DrawQueues&) -> DrawQueuesService;
