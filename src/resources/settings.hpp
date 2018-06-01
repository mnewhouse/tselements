/*
* TS Elements
* Copyright 2015-2018 M. Newhouse
* Released under the MIT license.
*/

#pragma once

#include <memory>

namespace ts
{
  namespace cup
  {
    struct CupSettings;
  }

  namespace client
  {
    struct KeySettings;
    struct PlayerSettings;
  }

  namespace resources
  {
    // The Settings class encompasses all kinds of different settings
    // classes, and provides const and non-const accessors for each one.
    class Settings
    {
    public:
      Settings();
      ~Settings();

      cup::CupSettings& cup_settings();
      const cup::CupSettings& cup_settings() const;

      client::KeySettings& key_settings();
      const client::KeySettings& key_settings() const;

      client::PlayerSettings& player_settings();
      const client::PlayerSettings& player_settings() const;

    private:
      struct Impl;
      std::unique_ptr<Impl> impl_;
    };
  }
}
