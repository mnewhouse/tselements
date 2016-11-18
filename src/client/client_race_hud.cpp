/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"
#include "client_race_hud.hpp"

#include "stage/race_tracker.hpp"

#include "scene/viewport_arrangement.hpp"

#include "imgui/imgui.h"

namespace ts
{
  namespace client
  {
    void RaceHUD::update(const scene::ViewportArrangement& viewport_arrangement, 
                         const stage::RaceTracker& race_tracker)
    {
      auto lap_count = race_tracker.lap_count();
      auto race_time = race_tracker.race_time();

      auto& buffer = format_buffer_;
      buffer.resize(64);

      // For each view port...
      // Get the camera target and update their HUD.
      for (const auto& viewport : viewport_arrangement.viewports())
      {
        if (auto camera_target = viewport.camera().followed_entity())
        {
          if (auto car_info = race_tracker.car_info(camera_target))
          {
            auto screen_rect = viewport.screen_rect();

            ImVec2 area_pos(screen_rect.left + 50.f, screen_rect.top + 50.f);
            ImVec2 area_size(200.0f, 120.0f);

            ImGui::SetNextWindowPos(area_pos, ImGuiSetCond_Always);
            ImGui::SetNextWindowSize(area_size, ImGuiSetCond_Always);

            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(50, 50, 80, 60));
            ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 240, 150));
            ImGui::Begin("hud", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs
                         | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);

            std::string last = "---", best = last;
            if (car_info->laps_done != 0)
            {
              last = stage::format_lap_time(car_info->last_lap_time);
              best = stage::format_lap_time(car_info->best_lap_time);
            }

            ImGui::Text("Last:");
            ImGui::SameLine(area_size.x * 0.5f - ImGui::CalcTextSize(last.c_str()).x - 10);
            ImGui::TextUnformatted(last.c_str());
            ImGui::SameLine(area_size.x * 0.5f);
            ImGui::Text("Best:");
            ImGui::SameLine(area_size.x - ImGui::CalcTextSize(best.c_str()).x - 10);
            ImGui::TextUnformatted(best.c_str());

            auto lap_time_elapsed = race_time - car_info->last_lap_start;
            auto minutes = lap_time_elapsed / 60000;
            auto seconds = (lap_time_elapsed % 60000) / 1000;
            auto tenths = (lap_time_elapsed % 1000) / 100;

            ImGui::TextUnformatted("Lap");

            if (car_info->laps_done >= lap_count) buffer = "Finished!";
            else sprintf(&buffer[0], "%d/%d", car_info->laps_done + 1, lap_count);

            ImGui::SameLine(area_size.x * 0.5f - ImGui::CalcTextSize(buffer.c_str()).x - 10);
            ImGui::TextUnformatted(buffer.c_str());

            if (minutes >= 1) sprintf(&buffer[0], "%d:%02d.%01d", minutes, seconds, tenths);
            else sprintf(&buffer[0], "%d.%01d", seconds, tenths);
      
            ImGui::SameLine(area_size.x - ImGui::CalcTextSize(buffer.c_str()).x - 10);           
            ImGui::TextUnformatted(buffer.c_str());

            auto sector_id = car_info->current_sector;
            if (sector_id != 0 && (lap_time_elapsed - car_info->last_sector_time) < 5000)
            {
              auto last = car_info->last_sector_time;
              auto formatted_time = stage::format_lap_time(last);
              ImGui::Text("S%d:", sector_id);
              ImGui::SameLine(area_size.x * 0.5f - ImGui::CalcTextSize(formatted_time.c_str()).x - 10);
              ImGui::TextUnformatted(formatted_time.c_str());

              if (sector_id <= car_info->best_lap_sector_times.size())
              {
                auto best = car_info->best_lap_sector_times[sector_id - 1];

                ImGui::SameLine(area_size.x * 0.5f);
                if (last < best)
                {
                  ImGui::TextColored(ImColor(0.0f, 0.8f, 0.0f, 1.0f), "-%s", stage::format_lap_time(best - last).c_str());
                }

                else
                {
                  ImGui::TextColored(ImColor(0.8f, 0.0f, 0.0f, 1.0f), "+%s", stage::format_lap_time(last - best).c_str());
                } 
              }
            }
                        
            ImGui::End();
            ImGui::PopStyleColor(2);
          }
        }
      }
    }
  }
}