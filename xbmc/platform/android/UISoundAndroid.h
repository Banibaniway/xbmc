/*
 *  Copyright (C) 2026 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "cores/AudioEngine/Interfaces/AESound.h"

#include <memory>
#include <string>

/*!
 * \brief GUI sound played through the Android SoundPool, bypassing ActiveAE.
 */
class CUISoundAndroid : public IAESound
{
public:
  /*!
   * \brief Load a sound; returns nullptr if SoundPool can't handle it.
   */
  static std::shared_ptr<IAESound> Create(const std::string& filename);

  ~CUISoundAndroid() override;

  void Play() override;
  void Stop() override;
  bool IsPlaying() override { return false; }
  void SetChannel(AEChannel channel) override {}
  AEChannel GetChannel() override { return AE_CH_NULL; }
  void SetVolume(float volume) override { m_volume = volume; }
  float GetVolume() override { return m_volume; }

private:
  CUISoundAndroid(const std::string& filename, int soundId);

  int m_soundId;
  int m_streamId{0};
  float m_volume{1.0f};
};
