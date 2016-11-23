/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include "imgui.h"

#include <boost/noncopyable.hpp>

namespace ts
{
  namespace imgui
  {
    class StyleGuard
      : boost::noncopyable
    {
    public:
      ~StyleGuard() { ImGui::PopStyleVar(count_); }

      void push(ImGuiStyleVar idx, const ImVec2& value)
      {
        ImGui::PushStyleVar(idx, value);
        ++count_;
      }

      void push(ImGuiStyleVar idx, float value)
      {
        ImGui::PushStyleVar(idx, value);
        ++count_;
      }

      void pop()
      {
        ImGui::PopStyleVar();
        --count_;
      }

    private:
      int count_ = 0;
    };


    class ColorGuard
      : boost::noncopyable
    {
    public:
      ~ColorGuard() { ImGui::PopStyleColor(count_); }

      void push(ImGuiCol idx, const ImVec4& value)
      {
        ImGui::PushStyleColor(idx, value);
        ++count_;
      }

      void pop()
      {
        ImGui::PopStyleColor();
        --count_;
      }

    private:
      int count_ = 0;
    };
  }
}