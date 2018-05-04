/*

This file is part of VROOM.

Copyright (c) 2015-2018, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include <iostream>

#include "relocate.h"

relocate::relocate(const input& input,
                   raw_solution& sol,
                   std::vector<amount_t>& amounts,
                   index_t source_vehicle,
                   index_t source_rank,
                   index_t target_vehicle,
                   index_t target_rank)
  : ls_operator(input,
                sol,
                amounts,
                source_vehicle,
                source_rank,
                target_vehicle,
                target_rank) {
  this->compute_gain();
}

void relocate::compute_gain() {
  assert(source_vehicle != target_vehicle);
  assert(_sol[source_vehicle].size() >= 1);
  assert(source_rank < _sol[source_vehicle].size());
  assert(target_rank <= _sol[target_vehicle].size());

  auto m = _input.get_matrix();
  const auto& v_source = _input._vehicles[source_vehicle];
  const auto& v_target = _input._vehicles[target_vehicle];

  // For source vehicle, we consider replacing "previous --> current
  // --> next" with "previous --> next".
  index_t current = _input._jobs[_sol[source_vehicle][source_rank]].index();
  index_t previous;
  if (source_rank == 0) {
    assert(v_source.has_start());
    previous = v_source.start.get().index();
  } else {
    previous = _input._jobs[_sol[source_vehicle][source_rank - 1]].index();
  }
  index_t next;
  if (source_rank == _sol[source_vehicle].size() - 1) {
    assert(v_source.has_end());
    next = v_source.end.get().index();
  } else {
    next = _input._jobs[_sol[source_vehicle][source_rank + 1]].index();
  }

  // Gain for source vehicle.
  gain_t new_edge_cost = m[previous][next];
  if (_sol[source_vehicle].size() == 1) {
    // Trying to empty a route, so cost of start --> end without job
    // is not taken into account.
    new_edge_cost = 0;
  }
  // Implicit cast to gain_t thanks to new_edge_cost.
  gain_t g1 = m[previous][current] + m[current][next] - new_edge_cost;
  BOOST_LOG_TRIVIAL(info) << m[previous][current] << " + " << m[current][next]
                          << " - " << new_edge_cost << " = " << g1;

  // For target vehicle, we consider replacing "previous --> next"
  // with "previous --> current --> next".
  if (target_rank == 0) {
    assert(v_target.has_start());
    previous = v_target.start.get().index();
  } else {
    previous = _input._jobs[_sol[target_vehicle][target_rank - 1]].index();
  }
  if (target_rank == _sol[target_vehicle].size()) {
    assert(v_target.has_end());
    next = v_target.end.get().index();
  } else {
    next = _input._jobs[_sol[target_vehicle][target_rank]].index();
  }

  // Gain for target vehicle.
  gain_t old_edge_cost = m[previous][next];
  if (_sol[target_vehicle].size() == 0) {
    // Adding to empty target route, so cost of start --> end without
    // job is not taken into account.
    old_edge_cost = 0;
  }
  // Implicit cast to gain_t thanks to old_edge_cost.
  gain_t g2 = old_edge_cost - m[previous][current] - m[current][next];
  BOOST_LOG_TRIVIAL(info) << old_edge_cost << " - " << m[previous][current]
                          << " - " << m[current][next] << " = " << g2;

  stored_gain = g1 + g2;
  gain_computed = true;
}

bool relocate::is_valid() const {
  auto relocate_job_rank = _sol[source_vehicle][source_rank];

  bool valid = _input.vehicle_ok_with_job(target_vehicle, relocate_job_rank);

  valid &= (_amounts[target_vehicle] + _input._jobs[relocate_job_rank].amount <=
            _input._vehicles[target_vehicle].capacity);

  return valid;
}

void relocate::log() const {
  const auto& v_source = _input._vehicles[source_vehicle];
  const auto& v_target = _input._vehicles[target_vehicle];

  std::cout << "Relocate gain: " << stored_gain << " - vehicle " << v_source.id
            << ", step " << source_rank << " (job "
            << _input._jobs[_sol[source_vehicle][source_rank]].id
            << ") moved to rank " << target_rank << " in route for vehicle "
            << v_target.id << std::endl;
}
