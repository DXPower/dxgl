#pragma once

// NOLINTNEXTLINE
#include <memory>

#define CREATE_PIMPL(member_name) \
class Pimpl;\
struct PimplDeleter {\
    void operator()(Pimpl* ptr) const;\
};\
std::unique_ptr<Pimpl, PimplDeleter> member_name{};

#define DEFINE_PIMPL_DELETER(class_name) \
void class_name::PimplDeleter::operator()(Pimpl* ptr) const {\
    delete ptr;\
}

#define DECLARE_PIMPL_SMFS(class_name) \
class_name(class_name&& move) noexcept;\
class_name& operator=(class_name&& move) noexcept;\
~class_name();

#define DEFINE_PIMPL_SMFS(class_name) \
class_name::class_name(class_name&& move) noexcept = default;\
class_name& class_name::operator=(class_name&& move) noexcept = default;\
class_name::~class_name() = default;

