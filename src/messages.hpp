#pragma once

#include "common.hpp"

#include <cereal/archives/binary.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/variant.hpp>

#include <glm/glm.hpp>

#include <array>
#include <optional>

namespace message {

struct game_state_update {
    struct player_info {
        bool present = false;
        team_name team = team_name::BLACK;
        glm::vec2 position = {0, 0};
        glm::vec2 velocity = {0, 0};

        template <typename Archive>
        void serialize(Archive& archive) {
            archive(present);
            archive(team);
            archive(position.x, position.y);
            archive(velocity.x, velocity.y);
        }
    };

    struct kunai_info {
        kunai_state state = kunai_state::ON_FLOOR;
        team_name color = team_name::BLACK;
        std::optional<team_name> team = std::nullopt;
        glm::vec2 position = {0, 0};
        glm::vec2 velocity = {0, 0};

        template <typename Archive>
        void serialize(Archive& archive) {
            archive(state);
            archive(color);
            archive(team);
            archive(position.x, position.y);
            archive(velocity.x, velocity.y);
        }
    };

    int time = 0;
    std::array<player_info, 4> players = {};
    std::array<kunai_info, 8> projectiles = {};
    int me = 0;

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(time);
        archive(players);
        archive(projectiles);
        archive(me);
    }
};

struct player_move {
    int time = 0;
    glm::vec2 input = {0, 0};
    glm::vec2 direction = {0, 0};

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(time);
        archive(input.x, input.y);
    }
};

struct lobby_state_update {
    struct player_info {
        team_name team = team_name::BLACK;
        bool occupied = false;

        template <typename Archive>
        void serialize(Archive& archive) {
            archive(team);
            archive(occupied);
        }
    };

    std::array<player_info, 4> players = {};
    std::optional<int> me = {};

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(players);
        archive(me);
    }
};

struct player_fire {
    //int time = 0;//necessary?
    glm::vec2 direction = {0, 0};
    team_name team = team_name::BLACK;

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(direction.x, direction.y);
        archive(team);
        //archive(time);
    }
};

struct lobby_flip_color {
    template <typename Archive>
    void serialize(Archive& archive) {}
};

struct lobby_start {
    template <typename Archive>
    void serialize(Archive& archive) {}
};

using any = std::variant<
    game_state_update,
    player_move,
    lobby_state_update,
    player_fire,
    lobby_flip_color,
    lobby_start>;

} // namespace message

template <typename Channel, typename ConnPtr>
void send_message(const ConnPtr& conn, const message::any& payload) {
    conn->template send<Channel>([&](std::ostream& ostream) {
        auto archive = cereal::BinaryOutputArchive(ostream);
        archive(payload);
    });
}

inline auto receive_message(std::istream& istream) -> message::any {
    auto payload = message::any{};
    {
        auto archive = cereal::BinaryInputArchive(istream);
        archive(payload);
    }
    return payload;
}
