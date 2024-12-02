/*

This file is part of VROOM.

Copyright (c) 2015-2024, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include <cassert>

#include "structures/cl_args.h"

namespace vroom::io {

void update_host(Servers& servers, std::string_view value) {
  // Determine profile and host from a "car:0.0.0.0"-like value.
  std::string profile = DEFAULT_PROFILE;
  std::string host;
  std::string path = "";

  auto index = value.find(':');
  if (index == std::string::npos) {
    host = value;
  } else {
    profile = value.substr(0, index);
    host = value.substr(index + 1);
  }

  if (!host.empty()) {
    // remove any trailing slash
    if (host.back() == '/') {
      host.pop_back();
    }

    // pull out a path if any and append a trailing slash for query building
    index = host.find('/');
    if (index != std::string::npos) {
      path = host.substr(index + 1) + "/";
      host = host.erase(index);
    }
  }

  auto existing_profile = servers.find(profile);
  if (existing_profile != servers.end()) {
    existing_profile->second.host = host;
    existing_profile->second.path = path;
  } else {
    auto [node, emplace_ok] = servers.try_emplace(profile);
    assert(emplace_ok);
    node->second.host = host;
    node->second.path = path;
  }
}

void update_port(Servers& servers, std::string_view value) {
  // Determine profile and port from a "car:0.0.0.0"-like value.
  std::string profile = DEFAULT_PROFILE;
  std::string port;

  if (auto index = value.find(':'); index == std::string::npos) {
    port = value;
  } else {
    profile = value.substr(0, index);
    port = value.substr(index + 1);
  }

  auto existing_profile = servers.find(profile);
  if (existing_profile != servers.end()) {
    existing_profile->second.port = port;
  } else {
    auto [node, emplace_ok] = servers.try_emplace(profile);
    assert(emplace_ok);
    node->second.port = port;
  }
}

unsigned CLArgs::get_depth(unsigned exploration_level) {
  return exploration_level;
}

unsigned CLArgs::get_nb_searches(unsigned exploration_level) {
  assert(exploration_level <= MAX_EXPLORATION_LEVEL);

  unsigned nb_searches = 4 * (exploration_level + 1);
  if (exploration_level >= 4) {
    nb_searches += 4;
  }
  if (exploration_level == MAX_EXPLORATION_LEVEL) {
    nb_searches += 4;
  }

  return nb_searches;
}

void CLArgs::set_exploration_level(unsigned exploration_level) {
  depth = get_depth(exploration_level);

  nb_searches = get_nb_searches(exploration_level);
}

} // namespace vroom::io
