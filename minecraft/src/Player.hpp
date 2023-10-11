#pragma once

#include "GlobalState.hpp"

class Player {
    glm::vec2 position{};
    glm::vec2 size{};
    glm::vec2 velocity{};

public:
    Player();

    void SetPosition(glm::vec2 pos) { position = pos; }
    const glm::vec2& GetPosition() const { return position; }
    
    void AddVelocity(glm::vec2 vel) { velocity += vel; }
    void SetVelocity(glm::vec2 vel) { velocity = vel; }
    const glm::vec2& GetVelocity() const { return velocity; }

    void SetSize(glm::vec2 size) { this->size = size; }
    const glm::vec2& GetSize() const { return size; }

    void MoveBy(glm::vec2 shift) { position += shift; }

    void Update(float delta_time);
    void Render() const;
    static void InitDraw(const GlobalState& global_state);
};