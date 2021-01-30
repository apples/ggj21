#pragma once

#include <trellis/trellis.hpp>

namespace channel {

using state_updates = trellis::channel_type_reliable_sequenced<struct state_updates_t>;

using actions = trellis::channel_type_reliable_unordered<struct actions_t>;

} // namespace channel

template <template <typename...> typename T>
using apply_channels_t = T<channel::state_updates, channel::actions>;
