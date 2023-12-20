#include <services/UiCallback.hpp>

#include <format>

using namespace services;

UiCallback::ArityMismatchError::ArityMismatchError(std::size_t expected, std::size_t actual) 
    : std::runtime_error(std::format("Wrong number of arguments gotten from UI callback. Got {} arguments; expected {}", expected, actual)),
      expected(expected),
      actual(actual)
{ }

UiCallback::ArgTypeMismatchError::ArgTypeMismatchError(std::size_t arg_index)
    : std::runtime_error(std::format("Wrong type supplied from UI callback for argument {}", arg_index)),
      arg_index(arg_index)
{ }

