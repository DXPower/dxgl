#pragma once

#include <string>
#include <boost/bimap.hpp>
#include <stdexcept>

class IdRegistry {
    boost::bimap<int, std::string> m_ids{};

public:
    int AddId(int id, std::string name) {
        auto [it, ins] = m_ids.insert(decltype(m_ids)::value_type(id, std::move(name)));

        if (!ins) {
            throw std::runtime_error("Attempt to register an id that already exists");
        }

        return id;
    }

    int AddId(std::string name) {
        return AddId(static_cast<int>(m_ids.size()), std::move(name));
    }

    int GetOrAddId(const std::string& name) {
        auto it = m_ids.right.find(name);

        if (it != m_ids.right.end()) {
            return it->second;
        } else {
            return AddId(name);
        }
    }

    auto& GetIds() const { return m_ids; }
};