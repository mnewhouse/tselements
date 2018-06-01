/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "imgui.h"

#include <array>

namespace ts
{
  namespace imgui
  {
    struct StyleColor
    {
      ImGuiCol id;
      ImColor color;
    };

    std::array<ImVec4, ImGuiCol_COUNT> load_default_style_colors();
    void push_default_style();

    extern const std::array<ImVec4, ImGuiCol_COUNT> default_style_colors;
  }
}
