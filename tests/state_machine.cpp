/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "catch.hpp"

#include "components/generic_state_machine.hpp"
#include "components/generic_state.hpp"

using namespace ts;

struct MyStateTraits
{
  struct game_context {};
  struct update_context {};
  struct render_context {};
  struct event_type {};  
};

using GenericState = components::GenericState<MyStateTraits>;

template <int N>
struct MyState
  : components::GenericState<MyStateTraits>
{
  MyState(game_context context)
    : GenericState(context)
  {}

  virtual void update(const update_context& context) override
  {
    x = N;
  }

  int x = 0;
};

TEST_CASE("The state machine is an essential part of the program flow.")
{
  using StateMachine = components::StateMachine<GenericState>;
  StateMachine state_machine;

  MyStateTraits::game_context context;
  state_machine.create_state<MyState<1>>(components::state_machine::deferred, context);

  REQUIRE(!state_machine);

  {
    auto transition_guard = state_machine.transition_guard();
    state_machine.activate_state<MyState<1>>();
    REQUIRE(!state_machine);
  }

  REQUIRE(state_machine);

  {
    auto transition_guard = state_machine.transition_guard();
    state_machine.destroy_state<MyState<1>>();
    REQUIRE(state_machine);
  }

  REQUIRE(!state_machine);
  auto one = state_machine.create_state<MyState<1>>(context);
  auto two = state_machine.create_state<MyState<2>>(context);
  auto three = state_machine.create_state<MyState<3>>(context);
  auto bad_one = state_machine.create_state<MyState<1>>(context);

  REQUIRE(bad_one == nullptr);
  REQUIRE(state_machine.active_state() == three);

  state_machine->update({});
  REQUIRE(three->x == 3);

  state_machine.pop_state();
  REQUIRE(state_machine.active_state() == two);

  state_machine.clear();
  REQUIRE(state_machine.active_state() == nullptr);
  REQUIRE(!state_machine);
}