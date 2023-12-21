#pragma once

#include <memory>

namespace services {
    class GameState {
        class Pimpl;
        struct PimplDeleter {
            void operator()(Pimpl* ptr) const;
        };

        std::unique_ptr<Pimpl, PimplDeleter> m_pimpl{};
    public:
        GameState();
        ~GameState();


        void EnterBuildPerspective();
        void ExitPerspective();
    };
}