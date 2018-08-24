/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#include "track_saving.hpp"
#include "track.hpp"

#include <fstream>

namespace ts
{
  namespace resources
  {
    bool save_track(const Track& track)
    {
      return save_track(track, track.path());
    }

    bool save_track(const Track& track, const std::string& dest_path)
    {
      std::ofstream out(dest_path);
      if (!out.is_open()) return false;

      out << "Size td " << track.height_level_count() << " " << track.size().x << " " << track.size().y << "\n";
      out << "Maker " << track.author() << "\n";

      const auto& control_points = track.control_points();
      const auto& start_points = track.custom_start_points();

      for (const auto& asset : track.assets())
      {
        out << "Include " << asset << "\n";
      }

      out << "\n";

      if (!control_points.empty())
      {
        out << "ControlPoints " << control_points.size() << "\n";
        for (auto& cp : control_points)
        {
          if (cp.type == ControlPoint::HorizontalLine)
          {
            auto w = cp.end.x - cp.start.x;
            out << "Point " << cp.start.x << " " << cp.start.y << " " << w << " 1 " << cp.flags << "\n";
          }

          else if (cp.type == ControlPoint::VerticalLine)
          {
            auto h = cp.end.y - cp.start.y;
            out << "Point " << cp.start.x << " " << cp.start.y << " " << h << " 0 " << cp.flags << "\n";
          }

          else if (cp.type == ControlPoint::Arbitrary)
          {
            out << "APoint " << cp.start.x << " " << cp.start.y << " " << 
              cp.end.x << " " << cp.end.y << " " << cp.flags << "\n";
          }
        }

        out << "End\n\n";
      }

      if (!start_points.empty())
      {
        out << "StartPoints" << start_points.size() << "\n";
        for (const auto& sp : start_points)
        {
          out << "Point " << sp.position.x << " " << sp.position.y << " " << sp.rotation << " " << sp.level << "\n";
        }

        out << "End\n\n";
      }

      const auto& path_library = track.path_library();
      for (const auto& path : path_library.paths())
      {
        out << "Path " << path.id << "\n";
        for (const auto& sub_path : path.sub_paths)
        {
          out << "SubPath\n";
          if (sub_path.closed) out << "Closed\n";
          for (const auto& node : sub_path.nodes)
          {
            out << "Node " <<
              node.first_control.x << " " << node.first_control.y << " " <<
              node.position.x << " " << node.position.y << " " <<
              node.second_control.x << " " << node.second_control.y << " " <<
              node.width << "\n";
          }          
        }

        out << "End\n\n";
      }

      for (const auto& layer : track.layers())
      {        
        if (const auto* base_terrain = layer.base_terrain())
        {
          out << "BaseTerrain\n";
          out << "Terrain " << base_terrain->terrain_id << "\n";
          out << "Texture " << base_terrain->texture_id << "\n";
        }

        else if (const auto* tiles = layer.tiles())
        {
          out << "TileLayer " << layer.level() << " " << layer.name() << "\n";
          for (const auto& t : *tiles)
          {
            if (t.level == 0)
            {
              out << "A ";
            }

            else
            {
              out << "LevelTile " << t.level << " ";
            }

            out << t.id << " " << t.position.x << " " << t.position.y << " " << t.rotation << "\n";
          }
        }

        else if (const auto* path_style = layer.path_style())
        {
          const auto& style = path_style->style;
          out << "PathLayer " << layer.name() << "\n";
          out << "Path " << path_style->path->id << "\n";

          if (style.preset_id != 0)
          {
            out << "StylePreset " << style.preset_id << "\n";

          }

          else
          {
            
            out << "BaseTexture " << style.base_texture << "\n";
            out << "BorderTexture " << style.border_texture << "\n";
            out << "BorderWidth " << style.border_width << "\n";
            out << "BorderOnly " << +style.border_only << "\n";
            out << "Terrain " << style.terrain_id << "\n";
            out << "TextureMode " << style.texture_mode << "\n";
            out << "FadeLength " << style.fade_length << "\n";
            out << "Width " << style.width << "\n";            
          }

          out << "Segmented " << +style.is_segmented << "\n";
          if (style.is_segmented)
          {
            for (auto& seg : style.segments)
            {
              out << "Segment " << seg.sub_path_id << " "  << seg.start_time_point << " " << 
                seg.end_time_point << " " << static_cast<int>(seg.side) << "\n";
            }
          }
        }

        if (!layer.visible()) out << "Hidden\n";
        out << "Level " << layer.level() << "\n";
        out << "End\n\n";
      }

      return true;
    }
  }
}