/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "car_editor.hpp"

#include "world/car.hpp"
#include "world/world_messages.hpp"

#include "imgui/imgui.h"

#include "utility/math_utilities.hpp"

namespace ts
{
  namespace editor
  {
    bool CarEditor::update(world::messages::CarPropertiesUpdate& msg)
    {
      if (!active_ || !car_) return false;
      
      auto column_width = 150.0f;
      ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(1.0f, 1.0f, 1.0f, 0.3f));
      ImGui::PushStyleColor(ImGuiCol_FrameBg, ImColor(0.0f, 0.0f, 0.0f, 0.2f));

      auto& io = ImGui::GetIO();
      ImGui::Begin("car_stats", &active_, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize |
                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

      auto& handling_state = car_->handling_state();

      ImGui::Text("Speed");
      ImGui::SameLine(column_width);
      ImGui::Text("%.1f", magnitude(car_->velocity()));

      ImGui::Text("Gear");
      ImGui::SameLine(column_width);
      if (handling_state.current_gear == 0) ImGui::Text("N");
      else if (handling_state.current_gear < 1) ImGui::Text("R");
      else ImGui::Text("%d", handling_state.current_gear);

      ImGui::Text("Revs");
      ImGui::SameLine(column_width);
      ImGui::Text("%.0f%%", handling_state.engine_rev_speed * 100.0f);

      ImGui::Text("Longitudinal G");
      ImGui::SameLine(column_width);
      ImGui::Text("%.2f", -handling_state.net_force.y / (car_->mass() * 9.8f * 3.6f));

      ImGui::Text("Lateral G");
      ImGui::SameLine(column_width);
      ImGui::Text("%.2f", handling_state.net_force.x / (car_->mass() * 9.8f * 3.6f));

      ImGui::Dummy(ImVec2(column_width + 60.0f, 0.0f));

      {
        auto size = ImGui::GetWindowSize();
        ImGui::SetWindowPos(ImVec2(io.DisplaySize.x - size.x - 60.0f, 60.0f), ImGuiSetCond_Always);                                   
      }

      ImGui::End();


      ImGui::SetNextWindowPos(ImVec2(60.0f, 60.0f), ImGuiSetCond_Always);
      ImGui::SetNextWindowSizeConstraints(ImVec2(0.0f, 0.0f),
                                          ImVec2(io.DisplaySize.x - 120.0f, io.DisplaySize.y - 120.0f));      

      ImGui::Begin("car_editor", &active_, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize |
                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

      
      auto f = [](auto d) { return static_cast<float>(d); };

      msg.car = car_;
      auto mass = f(car_->mass());
      auto cog = vector2_cast<float>(car_->center_of_mass());
      const auto& handling = car_->handling();
      auto acceleration = f(handling.max_acceleration_force);
      auto braking = f(handling.max_braking_force);
      auto front_driven = handling.front_driven;
      auto rear_driven = handling.rear_driven;
      auto max_revs = f(handling.max_engine_revs);

      auto reverse_ratio = f(handling.reverse_gear_ratio);
      auto drag = f(handling.drag_coefficient);
      auto downforce = f(handling.downforce_coefficient);
      auto rolling_drag = f(handling.rolling_drag_coefficient);
      auto traction_limit = f(handling.traction_limit);
      auto load_transfer = f(handling.load_transfer);
      auto brake_balance = f(handling.brake_balance);
      auto downforce_balance = f(handling.downforce_balance);
      auto steering_balance = f(handling.steering_balance);
      auto cornering = f(handling.cornering);
      auto max_steering_angle = f(handling.max_steering_angle);
      auto non_slide_angle = f(handling.non_slide_angle);
      auto full_slide_angle = f(handling.full_slide_angle);
      auto sliding_grip = f(handling.sliding_grip);
      auto angular_damping = f(handling.angular_damping);
      auto wheelbase_length = f(handling.wheelbase_length);
      auto front_axle_width = f(handling.front_axle_width);
      auto rear_axle_width = f(handling.rear_axle_width);
      
      float cog_array[] = { cog.x, cog.y };
      auto moment = f(car_->moment_of_inertia());

      int r = 0;
      r += ImGui::InputFloat("Mass", &mass, 1.0f, 10.0f, 0); 
      r += ImGui::InputFloat("Center of Mass", &cog.y, 0.01f, 0.1f, 2);
      r += ImGui::InputFloat("Moment of Inertia", &moment, 1.0f, 10.0f, 0);
      ImGui::NewLine();
      r += ImGui::InputFloat("Acceleration Force", &acceleration, 100.0, 1000.0, 0);
      r += ImGui::InputFloat("Braking Force", &braking, 100.0, 1000.0, 0);
      r += ImGui::InputFloat("Max Engine Revs", &max_revs, 1.0f, 10.0f, 1);
      r += ImGui::Checkbox("Front Driven", &front_driven);
      r += ImGui::Checkbox("Rear Driven", &rear_driven);      
      ImGui::NewLine();

      auto num_gears = static_cast<int>(handling.gear_ratios.size());
      r += ImGui::InputInt("Gear Number", &num_gears, 1, 1);

      auto gear_ratios = handling.gear_ratios;      
      num_gears = clamp(num_gears, 1, 8);
      gear_ratios.resize(num_gears);

      r += ImGui::InputFloat("Reverse", &reverse_ratio, 0.01f, 0.1f, 2);
      for (int i = 0; i < num_gears; ++i)
      {
        char buffer[24];
        std::sprintf(buffer, "Gear %d", i + 1);

        auto ratio = static_cast<float>(gear_ratios[i]);
        r += ImGui::InputFloat(buffer, &ratio, 0.01f, 0.1f, 2);

        if (i > 0) ratio = std::min(static_cast<float>(gear_ratios[i - 1]), ratio);
        if (i + 1 < num_gears) ratio = std::max(static_cast<float>(gear_ratios[i + 1]), ratio);

        gear_ratios[i] = ratio;
      }      
      
      

      ImGui::NewLine();
      r += ImGui::InputFloat("Drag Coefficient", &drag, 0.001f, 0.01f, 3);         
      r += ImGui::InputFloat("Downforce Coeff.", &downforce, 0.001f, 0.01f, 3);
      r += ImGui::InputFloat("Rolling Drag Coeff.", &rolling_drag, 0.001f, 0.01f, 2);
      ImGui::NewLine();
      r += ImGui::InputFloat("Traction Limit", &traction_limit, 100.0f, 1000.0f, 0);
      r += ImGui::InputFloat("Load Transfer", &load_transfer, 0.01f, 0.1f, 2);
      ImGui::NewLine();
      r += ImGui::InputFloat("Brake Balance", &brake_balance, 0.01f, 0.1f, 2);
      r += ImGui::InputFloat("Downforce Balance", &downforce_balance, 0.01f, 0.1f, 2);
      r += ImGui::InputFloat("Steering Balance", &steering_balance, 0.01f, 0.1f, 2);
      ImGui::NewLine();
      r += ImGui::InputFloat("Cornering", &cornering, 0.01f, 0.1f, 2);
      r += ImGui::InputFloat("Max. Steering Angle", &max_steering_angle, 0.1f, 1.0f, 1);
      r += ImGui::InputFloat("Non-slide Angle", &non_slide_angle, 0.1f, 1.0f, 1);
      r += ImGui::InputFloat("Full Slide Angle", &full_slide_angle, 0.1f, 1.0f, 1);
      r += ImGui::InputFloat("Sliding Grip", &sliding_grip, 0.01f, 0.1f, 2);
      r += ImGui::InputFloat("Angular Damping", &angular_damping, 0.01f, 0.1f, 2);
      ImGui::NewLine();
      r += ImGui::InputFloat("Wheelbase Length", &wheelbase_length, 0.1f, 1.0f, 1);
      r += ImGui::InputFloat("Front Axle Width", &front_axle_width, 0.1f, 1.0f, 1);
      r += ImGui::InputFloat("Rear Axle Width", &rear_axle_width, 0.1f, 1.0f, 1);
      
      
      ImGui::End();
      ImGui::PopStyleColor(2);

      auto& h = msg.handling;
      msg.center_of_mass = vector2_cast<double>(cog);
      msg.mass = std::max(mass, 1.0f);
      msg.moment = std::max(moment, 1.0f);
      h.max_acceleration_force = std::max(acceleration, 0.0f);
      h.max_braking_force = std::max(braking, 0.0f);
      h.front_driven = front_driven;
      h.rear_driven = rear_driven;
      h.gear_ratios.assign(gear_ratios.begin(), gear_ratios.end());
      h.max_engine_revs = std::max(max_revs, 0.0f);
      h.reverse_gear_ratio = std::max(reverse_ratio, 0.0f);
      h.drag_coefficient = std::max(drag, 0.0f);
      h.downforce_coefficient = std::max(downforce, 0.0f);
      h.rolling_drag_coefficient = std::max(rolling_drag, 0.0f);
      h.traction_limit = std::max(traction_limit, 0.0f);
      h.load_transfer = std::max(load_transfer, 0.0f);
      h.brake_balance = clamp(brake_balance, -0.5f, 0.5f);
      h.downforce_balance = clamp(downforce_balance, -0.5f, 0.5f);
      h.steering_balance = clamp(steering_balance, -0.5f, 0.5f);
      h.cornering = std::max(cornering, 0.0f);
      h.max_steering_angle = std::max(max_steering_angle, 0.0f);
      h.non_slide_angle = std::max(non_slide_angle, 0.0f);
      h.full_slide_angle = std::max(full_slide_angle, 0.0f);
      h.sliding_grip = std::max(sliding_grip, 0.0f);
      h.angular_damping = std::max(angular_damping, 0.0f);
      h.wheelbase_length = std::max(wheelbase_length, 1.0f);
      h.front_axle_width = std::max(front_axle_width, 1.0f);
      h.rear_axle_width = std::max(rear_axle_width, 1.0f);

      return r > 0;
    }

    void CarEditor::activate(const world::Car* car)
    {
      active_ = (car != nullptr);
      car_ = car;
    }

    void CarEditor::deactivate()
    {
      car_ = nullptr;
      active_ = false;
    }

    bool CarEditor::active() const
    {
      return active_ && car_ != nullptr;
    }

  }
}