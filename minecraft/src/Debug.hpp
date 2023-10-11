#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

struct GlobalState;

struct DebugSquare {
    glm::vec2 position{};
    glm::vec2 size{};
};

struct DebugLine {
    glm::vec2 from{};
    glm::vec2 to{};
};

struct DebugArrow {
    glm::vec2 from{};
    glm::vec2 to{};
};

struct DebugPolygon {
    std::span<glm::vec2> points{};
};

class DebugDraws {
    class Pimpl;
    std::unique_ptr<Pimpl> pimpl;

    // std::vector<glm::vec2> vertices{};
    // std::vector<uint16_t> indices{};
public:
    DebugDraws();
    ~DebugDraws();

    void Init(const GlobalState& global_state);

    void Draw(const DebugSquare& square, const glm::vec4& color);
    void Draw(const DebugLine& line, const glm::vec4& color);
    void Draw(const DebugArrow& arrow, const glm::vec4& color);
    void Draw(const DebugPolygon& polygon, const glm::vec4& color);

    void Render() const;
};