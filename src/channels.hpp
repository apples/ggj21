#pragma once

#include <trellis/trellis.hpp>

namespace channel {

using derp = trellis::channel_type_reliable_sequenced<struct derp_t>;

} // namespace channel

template <template <typename...> typename T>
using apply_channels_t = T<channel::derp>;
