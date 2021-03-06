#pragma once

#include "common.hpp"

#include <cereal/archives/binary.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/string.hpp>
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
        bool alive = true;
        bool carrying = false;
        int kunaiAmount = 1;
        float respawnTimer = 0.0f;

        template <typename Archive>
        void serialize(Archive& archive) {
            archive(present);
            archive(team);
            archive(position.x, position.y);
            archive(velocity.x, velocity.y);
            archive(alive);
            archive(carrying);
            archive(kunaiAmount);
            archive(respawnTimer);
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

    struct objective_info {
        glm::vec2 position = {30, 15};
        glm::vec2 goal_black = {0, 0};
        glm::vec2 goal_white = {0, 0};

        template <typename Archive>
        void serialize(Archive& archive) {
            archive(position.x, position.y);
            archive(goal_black.x, goal_black.y);
            archive(goal_white.x, goal_white.y);
        }
    };

    int time = 0;
    std::array<player_info, 4> players = {};
    std::array<kunai_info, 8> projectiles = {};
    int me = 0;
    objective_info objective = {};
    int score_black = 0;
    int score_white = 0;

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(time);
        archive(players);
        archive(projectiles);
        archive(me);
        archive(objective);
        archive(score_black);
        archive(score_white);
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
    glm::vec2 direction = {0, 0};
    team_name team = team_name::BLACK;

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(direction.x, direction.y);
        archive(team);
    }
};

struct lobby_flip_color {
    template <typename Archive>
    void serialize(Archive& archive) {}
};

struct lobby_start_game {
    template <typename Archive>
    void serialize(Archive& archive) {}
};

struct game_started {
    glm::vec2 world_size = {0, 0};

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(world_size.x, world_size.y);
    }
};

struct play_sfx {
    std::string name = "";
    std::optional<glm::vec2> position = std::nullopt;

    template <class Archive>
    void save(Archive& ar) const {
        ar(name);
        if (position) {
            ar(true, position->x, position->y);
        } else {
            ar(false);
        }
    }

    template <class Archive>
    void load(Archive& ar) {
        ar(name);
        bool b;
        ar(b);
        if (b) {
            position = glm::vec2{};
            ar(position->x, position->y);
        } else {
            position = std::nullopt;
        }
    }
};

using any = std::variant<
    game_state_update,
    player_move,
    lobby_state_update,
    player_fire,
    lobby_flip_color,
    lobby_start_game,
    game_started,
    play_sfx>;

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
