/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
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

      void pop(int count)
      {
        auto c = std::min(count, count_);
        ImGui::PopStyleVar(c);
        count_ -= c;        
      }

      void pop_all()
      {
        ImGui::PopStyleVar(count_);
        count_ = 0;
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

      void pop(int count)
      {
        auto c = std::min(count, count_);
        ImGui::PopStyleColor(c);
        count_ -= c;
      }

      void pop_all()
      {
        ImGui::PopStyleColor(count_);
        count_ = 0;
      }

    private:
      int count_ = 0;
    };
  }
}