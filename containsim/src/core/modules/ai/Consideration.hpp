#pragma once

#include <algorithm>
#include <numeric>
#include <execution>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include <flecs.h>

namespace ai {
    
struct ConsiderationContext {
    flecs::entity_view potential_performance{};
    flecs::entity_view performer{};
    flecs::entity_view target{}; // May be null
};

class Consideration {
    std::string m_name{};
public:
    Consideration() = default;
    Consideration(std::string name) : m_name(std::move(name)) { }

    virtual ~Consideration() = default;

    virtual float GetValue(const ConsiderationContext& context) const = 0;
    std::string_view GetName() const { return m_name; }
};

class ConstantConsideration final : public Consideration {
    float m_value{};
public:
    ConstantConsideration(std::string name, float value)
        : Consideration(std::move(name)), m_value(value) { }

    float GetValue(const ConsiderationContext&) const override {
        return m_value;
    }
};

class NullConsideration final : public Consideration {
public:
    NullConsideration(std::string name)
        : Consideration(std::move(name)) { }

    float GetValue(const ConsiderationContext&) const override {
        return 0.f;
    }
};

class VacuousConsideration final : public Consideration {
public:
    VacuousConsideration(std::string name)
        : Consideration(std::move(name)) { }

    float GetValue(const ConsiderationContext&) const override {
        return 1.f;
    }
};

enum class BehaviorWithX : bool {
    Unappealing,
    Appealing
};

template<BehaviorWithX Dir>
class LinearConsideration final : public Consideration {
    std::shared_ptr<Consideration> m_input{};
    float m_max{};
public:
    LinearConsideration(std::string name, std::shared_ptr<Consideration> input, float max)
        : Consideration(std::move(name)), m_input(std::move(input)), m_max(max) { }

    float GetValue(const ConsiderationContext& context) const override {
        if constexpr (Dir == BehaviorWithX::Appealing) {
            return m_input->GetValue(context) / m_max;
        } else {
            return 1 - (m_input->GetValue(context) / m_max);
        }
    }
};

template<BehaviorWithX Dir>
class QuadraticConsideration final : public Consideration {
    std::shared_ptr<Consideration> m_input{};
    float m_max{};
    float m_k{};
public:
    QuadraticConsideration(std::string name, std::shared_ptr<Consideration> input, float max, float k)
        : Consideration(std::move(name)), m_input(std::move(input)), m_max(max), m_k(k) { }

    float GetValue(const ConsiderationContext& context) const override {
        if constexpr (Dir == BehaviorWithX::Appealing) {
            return std::pow(m_input->GetValue(context) / m_max, m_k);
        } else {
            return 1 - std::pow(m_input->GetValue(context) / m_max, m_k);
        }
    }
};

template<BehaviorWithX Dir>
class SigmoidConsideration final : public Consideration {
    std::shared_ptr<Consideration> m_input{};
    float m_zero{};
    float m_one{};
    float m_epsilon{};

public:
    SigmoidConsideration(std::string name, std::shared_ptr<Consideration> input, float zero_point, float one_point, float epsilon = 0.01f)
        : Consideration(std::move(name)), m_input(std::move(input)), m_zero(zero_point), m_one(one_point), m_epsilon(epsilon) { }

    float GetValue(const ConsiderationContext& context) const override {
        const float p = (1.f - m_epsilon) / m_epsilon; // epsilon adjustment
        const float k = 2.f * std::log(p) / (m_zero - m_one); // growth factor
        const float c = (m_zero + m_one) / 2.f; // center

        const float s = 1.f / (1 + std::exp(k * (m_input->GetValue(context) - c)));
        
        return (Dir == BehaviorWithX::Appealing) ? s : 1 - s;
    }
};

enum class AggregationKind : uint8_t {
    Sum,
    Multiply,
    Mean,
    CompensatedMean,
    Min,
    Max
};

template<AggregationKind Kind>
class AggregationConsideration final : public Consideration {
    std::vector<std::shared_ptr<Consideration>> m_inputs{};

public:
    AggregationConsideration(std::string name, std::vector<std::shared_ptr<Consideration>> inputs)
        : Consideration(std::move(name)), m_inputs(std::move(inputs)) { }

    void AddInput(std::shared_ptr<Consideration>&& input) {
        m_inputs.push_back(std::move(input));
    }

    float GetValue(const ConsiderationContext& context) const override {
        using enum AggregationKind;

        if constexpr (Kind == Sum || Kind == Mean || Kind == CompensatedMean) {
            float sum = std::reduce(std::execution::unseq, m_inputs.begin(), m_inputs.end(), 
                [](const auto& lhs, const auto& rhs) {
                    return lhs->GetValue() + rhs->GetValue();
                });

            if constexpr (Kind == Sum) {
                return sum;
            } else {
                sum /= m_inputs.size();

                if constexpr (Kind == Mean) {
                    return sum;
                } else {
                    const float mod_factor = 1.f - (1.f / m_inputs.size());
                    const float make_up = (1.f - sum) / mod_factor;
                    return sum + (make_up * sum);
                }
            }
        } else if constexpr (Kind == Multiply) { 
            return std::reduce(std::execution::unseq, m_inputs.begin(), m_inputs.end(), 
                [](const auto& lhs, const auto& rhs) {
                    return lhs->GetValue() * rhs->GetValue();
                });
        } else {
            auto proj = [&context](const std::shared_ptr<Consideration>& c) {
                return c->GetValue(context);
            };

            if constexpr (Kind == Min) {
                return std::ranges::min(m_inputs, {}, proj);
            } else {
                return std::ranges::max(m_inputs, {}, proj);
            }
        }
    }
};
}