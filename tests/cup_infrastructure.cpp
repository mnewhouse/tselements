/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "catch.hpp"

#include "server/server.hpp"
#include "server/server_message_conveyor.hpp"

#include "client/client_message_conveyor.hpp"
#include "client/local_player_roster.hpp"
#include "client/local_message_dispatcher.hpp"

#include "resources/resource_store.hpp"
#include "resources/car_store.hpp"
#include "resources/settings.hpp"

#include "cup/cup_settings.hpp"
#include "cup/cup.hpp"
#include "cup/cup_synchronizer.hpp"

#include "stage/stage.hpp"
#include "stage/stage_messages.hpp"

#include <thread>

using namespace ts;

/*
namespace test
{
  struct SceneLoader
    : client::SceneLoaderInterface
  {
    client::LocalMessageDispatcher* message_dispatcher = nullptr;
    const stage::Stage* stage_ptr = nullptr;

    virtual void handle_message(const stage::messages::StageLoaded& stage_loaded) override
    {
      (*message_dispatcher)(cup::messages::Ready());

      stage_ptr = stage_loaded.stage_ptr;
    }
  };
}

TEST_CASE("Cup infrastructure.")
{
  resources::ResourceStore resource_store;
  auto& cup_settings = resource_store.settings().cup_settings();
  auto& car_store = resource_store.car_store();
  
  cup_settings.tracks.emplace_back();
  cup_settings.tracks.back().path = "assets/tracks/banaring.trk";
  cup_settings.tracks.back().name = "banaring";

  car_store.load_car_directory("assets/cars");
  auto car_it = car_store.car_definitions().find("slider");
  REQUIRE(car_it != car_store.car_definitions().end());
  cup_settings.selected_cars.push_back(*car_it);

  cup::Cup client_cup;
  cup::CupSynchronizer cup_synchronizer(&client_cup);

  cup::PlayerDefinition local_players[2];
  local_players[0].control_slot = 0;
  local_players[0].name = "Steve";
  local_players[0].id = 0xF00;
  local_players[1].control_slot = 2;
  local_players[1].name = "Johnny";
  local_players[1].id = 0xBAA;

  

  server::Server server_obj(&resource_store);
  client::LocalMessageDispatcher client_message_dispatcher(&server_obj.message_conveyor());

  client::LocalPlayerRoster local_player_controller(local_players, 2);
  test::SceneLoader scene_loader;
  scene_loader.message_dispatcher = &client_message_dispatcher;
  client::MessageContext<client::LocalMessageDispatcher> message_context{};
  /*
  message_context.cup_synchronizer = &cup_synchronizer;
  message_context.local_player_controller = &local_player_controller;
  message_context.scene_loader = &scene_loader;

  client::MessageConveyor<client::LocalMessageDispatcher> message_conveyor(message_context);

  server_obj.initiate_local_connection(&message_conveyor, local_players, 2);
  REQUIRE(client_cup.cup_state() == cup::CupState::Registration);

  client_message_dispatcher(cup::messages::Advance());
  REQUIRE(client_cup.cup_state() == cup::CupState::Intermission);

  client_message_dispatcher(cup::messages::Advance());
  // Wait for the server to load the track

  for (std::size_t n = 0; n != 5000 && client_cup.cup_state() != cup::CupState::Action; n += 20)
  {
    server_obj.update(20);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }
  
  REQUIRE(scene_loader.stage_ptr != nullptr);

  const auto& stage_desc = scene_loader.stage_ptr->stage_description();
  REQUIRE(stage_desc.car_instances.size() == 2);

  REQUIRE(client_cup.cup_state() == cup::CupState::Action);
  client_message_dispatcher(cup::messages::Advance());
  REQUIRE(client_cup.cup_state() == cup::CupState::End);
  client_message_dispatcher(cup::messages::Advance());
  REQUIRE(client_cup.cup_state() == cup::CupState::Registration);
}
*/