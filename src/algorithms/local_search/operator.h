#ifndef OPERATOR_H
#define OPERATOR_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "structures/typedefs.h"
#include "structures/vroom/input/input.h"
#include "structures/vroom/solution_state.h"

namespace vroom::ls {

class Operator {
protected:
  const OperatorName _name;
  const Input& _input;
  const utils::SolutionState& _sol_state;

  // Source of move for this operator.
  RawRoute& source;
  std::vector<Index>& s_route;
  const Index s_vehicle;
  const Index s_rank;
  // Target of move for this operator.
  RawRoute& target;
  std::vector<Index>& t_route;
  const Index t_vehicle;
  const Index t_rank;

  bool gain_computed{false};
  Eval s_gain;
  Eval t_gain;
  Eval stored_gain;

  virtual void compute_gain() = 0;

  bool is_valid_for_source_range_bounds() const;

  bool is_valid_for_target_range_bounds() const;

  // Used for internal operators only.
  bool is_valid_for_range_bounds() const;

public:
  Operator(OperatorName name,
           const Input& input,
           const utils::SolutionState& sol_state,
           RawRoute& s_raw_route,
           Index s_vehicle,
           Index s_rank,
           RawRoute& t_raw_route,
           Index t_vehicle,
           Index t_rank)
    : _name(name),
      _input(input),
      _sol_state(sol_state),
      source(s_raw_route),
      s_route(s_raw_route.route),
      s_vehicle(s_vehicle),
      s_rank(s_rank),
      target(t_raw_route),
      t_route(t_raw_route.route),
      t_vehicle(t_vehicle),
      t_rank(t_rank) {
  }

  OperatorName get_name() const;

  virtual Eval gain();

  virtual bool is_valid() = 0;

  virtual void apply() = 0;

  virtual std::vector<Index> addition_candidates() const = 0;

  virtual std::vector<Index> update_candidates() const = 0;

  // Used to check if a move should be made invalid due to a change in
  // unassigned jobs.
  virtual std::vector<Index> required_unassigned() const;

  // Used to check if a move should be made invalid due to an indirect
  // change in another route.
  virtual bool invalidated_by(Index rank) const;

  virtual ~Operator() = default;
};

} // namespace vroom::ls

#endif
