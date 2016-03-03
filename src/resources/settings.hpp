/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/

#ifndef SETTINGS_HPP_58198519825
#define SETTINGS_HPP_58198519825

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
  }

  namespace resources
  {
    class Settings
    {
    public:
      Settings();
      ~Settings();

      cup::CupSettings& cup_settings();
      const cup::CupSettings& cup_settings() const;

      client::KeySettings& key_settings();
      const client::KeySettings& key_settings() const;

    private:
      struct Impl;
      std::unique_ptr<Impl> impl_;
    };
  }
}

#endif