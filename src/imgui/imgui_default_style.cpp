/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#include "stdinc.hpp"
#include "imgui_default_style.hpp"
#include "imgui.h"

namespace ts
{
  namespace imgui
  {
    static const StyleColor default_colors[] =
    {
      { ImGuiCol_Text,{ 250, 250, 250 } },
      { ImGuiCol_TextDisabled,{ 180, 180, 180 } },
      { ImGuiCol_WindowBg,{ 130, 130, 130 } }, // Background of normal windows
      { ImGuiCol_ChildWindowBg,{ 130, 130, 130 } }, // Background of child windows       
      { ImGuiCol_PopupBg,{ 70, 70, 75 } }, // Background of popups, menus, tooltips windows
      { ImGuiCol_Border,{ 50, 50, 50 } },
      { ImGuiCol_BorderShadow,{ 40, 40, 40 } },
      { ImGuiCol_FrameBg,{ 70, 70, 75 } },              // Background of checkbox, radio button, plot, slider, text input
      { ImGuiCol_FrameBgHovered,{ 70, 70, 75 } },
      { ImGuiCol_FrameBgActive,{ 70, 70, 75 } },
      { ImGuiCol_TitleBg,{ 50, 90, 180 } },
      { ImGuiCol_TitleBgCollapsed,{ 50, 90, 180 } },
      { ImGuiCol_TitleBgActive,{ 50, 90, 180 } },
      { ImGuiCol_MenuBarBg,{ 70, 70, 75 } },
      { ImGuiCol_ScrollbarBg,{ 90, 90, 100 } },
      { ImGuiCol_ScrollbarGrab,{ 200, 200, 220 } },
      { ImGuiCol_ScrollbarGrabHovered,{ 220, 220, 250 } },
      { ImGuiCol_ScrollbarGrabActive,{ 230, 230, 255 } },
      //ImGuiCol_ComboBg,
      //ImGuiCol_CheckMark,
      //ImGuiCol_SliderGrab,
      //ImGuiCol_SliderGrabActive,
      { ImGuiCol_Button, { 50, 90, 170 } },
      { ImGuiCol_ButtonHovered,{ 70, 120, 200 } },
      { ImGuiCol_ButtonActive,{ 70, 120, 200 } },
      { ImGuiCol_Header,{ 70, 120, 200 } },
      { ImGuiCol_HeaderHovered,{ 120, 170, 250 } },
      { ImGuiCol_HeaderActive,{ 70, 120, 200 } },
      { ImGuiCol_Column,{ 70, 70, 75 } },
      { ImGuiCol_ColumnHovered,{ 70, 70, 75 } },
      { ImGuiCol_ColumnActive,{ 70, 70, 75 } },
      //ImGuiCol_ResizeGrip,
      //ImGuiCol_ResizeGripHovered,
      //ImGuiCol_ResizeGripActive,
      //ImGuiCol_CloseButton,
      //ImGuiCol_CloseButtonHovered,
      //ImGuiCol_CloseButtonActive,
      //ImGuiCol_PlotLines,
      //ImGuiCol_PlotLinesHovered,
      { ImGuiCol_PlotHistogram,{ 250, 150, 50 } },
      { ImGuiCol_PlotHistogramHovered,{ 110, 160, 220 } },
      //ImGuiCol_TextSelectedBg,
      { ImGuiCol_ModalWindowDarkening,{ 0, 0, 50, 150 } }
    };

    std::array<ImVec4, ImGuiCol_COUNT> load_default_style_colors()
    {
      std::array<ImVec4, ImGuiCol_COUNT> result;
      for (auto c : default_colors)
      {
        result[c.id] = c.color;
      }    

      return result;
    }

    void push_default_style()
    {      
      for (auto style : default_colors)
      {
        ImGui::PushStyleColor(style.id, style.color);
      }

      ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 24.f);
    }

    extern const std::array<ImVec4, ImGuiCol_COUNT> default_style_colors = load_default_style_colors();
  }
}
