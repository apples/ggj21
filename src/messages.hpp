#pragma once

#include "common.hpp"

#include <cereal/archives/binary.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/variant.hpp>

#include <glm/glm.hpp>

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

    int time = 0;
    std::array<player_info, 4> players = {};
    int me = 0;

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(time);
        archive(players);
        archive(me);
    }
};

struct player_move {
    int time = 0;
    glm::vec2 input = {0, 0};

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(time);
        archive(input.x, input.y);
    }
};

using any = std::variant<game_state_update, player_move>;

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
