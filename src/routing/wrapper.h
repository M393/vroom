#ifndef WRAPPER_H
#define WRAPPER_H

/*

This file is part of VROOM.

Copyright (c) 2015-2024, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include <mutex>
#include <thread>
#include <vector>

#include "structures/generic/matrix.h"
#include "structures/vroom/location.h"
#include "structures/vroom/matrices.h"
#include "structures/vroom/solution/route.h"
#include "structures/vroom/vehicle.h"
#include "utils/exception.h"

namespace vroom::routing {

class Wrapper {

public:
  std::string profile;

  virtual Matrices get_matrices(const std::vector<Location>& locs) const = 0;

  Matrices
  get_sparse_matrices(const std::string& profile,
                      const std::vector<Location>& locs,
                      const std::vector<Vehicle>& vehicles,
                      const std::vector<Job>& jobs,
                      std::unordered_map<Id, std::string>& v_id_to_geom) const {
    std::size_t m_size = locs.size();
    Matrices m(m_size);

    std::exception_ptr ep = nullptr;
    std::mutex ep_m;
    std::mutex id_to_geom_m;
    std::mutex matrix_m;

    auto run_on_vehicle =
      [this, &jobs, &matrix_m, &m, &id_to_geom_m, &v_id_to_geom, &ep_m, &ep](
        const Vehicle& v) {
        try {
          std::vector<Location> route_locs;
          route_locs.reserve(v.steps.size());

          bool has_job_steps = false;
          for (const auto& step : v.steps) {
            switch (step.type) {
              using enum STEP_TYPE;
            case START:
              if (v.has_start()) {
                route_locs.push_back(v.start.value());
              }
              break;
            case END:
              if (v.has_end()) {
                route_locs.push_back(v.end.value());
              }
              break;
            case BREAK:
              break;
            case JOB:
              has_job_steps = true;
              route_locs.push_back(jobs[step.rank].location);
              break;
            }
          }

          if (!has_job_steps) {
            // No steps provided in input for vehicle, or only breaks in
            // steps.
            return;
          }
          assert(route_locs.size() >= 2);

          this->update_sparse_matrix(v.id,
                                     route_locs,
                                     m,
                                     matrix_m,
                                     v_id_to_geom,
                                     id_to_geom_m);
        } catch (...) {
          std::scoped_lock<std::mutex> lock(ep_m);
          ep = std::current_exception();
        }
      };

    std::vector<std::jthread> vehicles_threads;
    vehicles_threads.reserve(vehicles.size());

    for (const auto& v : vehicles) {
      if (v.profile == profile) {
        vehicles_threads.emplace_back(run_on_vehicle, v);
      }
    }

    for (auto& t : vehicles_threads) {
      t.join();
    }

    if (ep != nullptr) {
      std::rethrow_exception(ep);
    }

    return m;
  };

  // Updates matrix with data from a single route request and returns
  // corresponding route geometry.
  virtual void
  update_sparse_matrix(const Id v_id,
                       const std::vector<Location>& route_locs,
                       Matrices& m,
                       std::mutex& matrix_m,
                       std::unordered_map<Id, std::string>& v_id_to_geom,
                       std::mutex& id_to_geom_m) const = 0;

  virtual void add_geometry(Route& route) const = 0;

  virtual ~Wrapper() = default;

protected:
  explicit Wrapper(std::string profile) : profile(std::move(profile)) {
  }

  static inline void
  check_unfound(const std::vector<Location>& locs,
                const std::vector<unsigned>& nb_unfound_from_loc,
                const std::vector<unsigned>& nb_unfound_to_loc) {
    assert(nb_unfound_from_loc.size() == nb_unfound_to_loc.size());
    unsigned max_unfound_routes_for_a_loc = 0;
    unsigned error_loc = 0; // Initial value never actually used.
    std::string error_direction;
    // Finding the "worst" location for unfound routes.
    for (unsigned i = 0; i < nb_unfound_from_loc.size(); ++i) {
      if (nb_unfound_from_loc[i] > max_unfound_routes_for_a_loc) {
        max_unfound_routes_for_a_loc = nb_unfound_from_loc[i];
        error_loc = i;
        error_direction = "from ";
      }
      if (nb_unfound_to_loc[i] > max_unfound_routes_for_a_loc) {
        max_unfound_routes_for_a_loc = nb_unfound_to_loc[i];
        error_loc = i;
        error_direction = "to ";
      }
    }
    if (max_unfound_routes_for_a_loc > 0) {
      std::string error_msg = "Unfound route(s) ";
      error_msg += error_direction;
      error_msg += "location [" + std::to_string(locs[error_loc].lon()) + "," +
                   std::to_string(locs[error_loc].lat()) + "]";

      throw RoutingException(error_msg);
    }
  }
};

} // namespace vroom::routing

#endif
