#ifndef LIBOSRM_WRAPPER_H
#define LIBOSRM_WRAPPER_H

/*

This file is part of VROOM.

Copyright (c) 2015-2024, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "osrm/coordinate.hpp"
#include "osrm/engine_config.hpp"
#include "osrm/osrm.hpp"

#include "routing/wrapper.h"

namespace vroom {
namespace routing {

class LibosrmWrapper : public Wrapper {

private:
  osrm::EngineConfig _config;
  const osrm::OSRM _osrm;

  osrm::json::Object get_route_with_coordinates(
    std::vector<osrm::util::Coordinate>&& coords) const;

  static osrm::EngineConfig get_config(const std::string& profile);

public:
  explicit LibosrmWrapper(const std::string& profile);

  Matrices get_matrices(const std::vector<Location>& locs) const override;

  void update_sparse_matrix(const Id v_id,
                            const std::vector<Location>& route_locs,
                            Matrices& m,
                            std::mutex& matrix_m,
                            std::unordered_map<Id, std::string>& v_id_to_geom,
                            std::mutex& id_to_geom_m) const override;

  void add_geometry(Route& route) const override;
};

} // namespace routing
} // namespace vroom

#endif
