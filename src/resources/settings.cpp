/*
* TS Elements
* Copyright 2015-2016 M. Newhouse
* Released under the MIT license.
*/


#include "settings.hpp"

#include "cup/cup_settings.hpp"

#include "client/key_settings.hpp"
#include "client/player_settings.hpp"

namespace ts
{
  namespace resources
  {
    struct Settings::Impl 
    {
      cup::CupSettings cup_settings;
      client::KeySettings key_settings = client::default_key_settings();
      client::PlayerSettings player_settings;
    };

    Settings::Settings()
      : impl_(std::make_unique<Impl>())
    {
    }

    Settings::~Settings()
    {
    }

    cup::CupSettings& Settings::cup_settings()
    {
      return impl_->cup_settings;
    }

    const cup::CupSettings& Settings::cup_settings() const
    {
      return impl_->cup_settings;
    }

    client::KeySettings& Settings::key_settings()
    {
      return impl_->key_settings;
    }

    const client::KeySettings& Settings::key_settings() const
    {
      return impl_->key_settings;
    }

    client::PlayerSettings& Settings::player_settings()
    {
      return impl_->player_settings;
    }

    const client::PlayerSettings& Settings::player_settings() const
    {
      return impl_->player_settings;
    }
  }
}