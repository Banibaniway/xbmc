/*
 *  Copyright (C) 2026 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "UISoundAndroid.h"

#include "filesystem/Directory.h"
#include "filesystem/File.h"
#include "filesystem/SpecialProtocol.h"
#include "platform/android/activity/JNIMainActivity.h"
#include "utils/URIUtils.h"
#include "utils/log.h"

#include <functional>

namespace
{
// SoundPool can't hold samples larger than about 1 MB once decoded
constexpr int64_t MAX_SOUNDPOOL_SIZE = 1000000;
} // namespace

std::shared_ptr<IAESound> CUISoundAndroid::Create(const std::string& filename)
{
  jni::CJNIMainActivity* activity = jni::CJNIMainActivity::GetAppInstance();
  if (!activity)
    return nullptr;

  int64_t size = 0;
  {
    XFILE::CFile file;
    if (!file.Open(filename))
      return nullptr;
    size = file.GetLength();
  }

  if (size > MAX_SOUNDPOOL_SIZE)
  {
    CLog::Log(LOGWARNING, "CUISoundAndroid: {} is too large for SoundPool, using AudioEngine",
              filename);
    return nullptr;
  }

  // SoundPool needs a real file path
  const std::string dir = "special://temp/uisounds/";
  XFILE::CDirectory::Create(dir);
  const std::string local = dir + std::to_string(std::hash<std::string>{}(filename)) +
                            URIUtils::GetExtension(filename);
  if (!XFILE::CFile::Copy(filename, local))
    return nullptr;

  const int soundId = activity->loadUiSound(CSpecialProtocol::TranslatePath(local));
  if (soundId <= 0)
    return nullptr;

  return std::shared_ptr<IAESound>(new CUISoundAndroid(filename, soundId));
}

CUISoundAndroid::CUISoundAndroid(const std::string& filename, int soundId)
  : IAESound(filename), m_soundId(soundId)
{
}

CUISoundAndroid::~CUISoundAndroid()
{
  jni::CJNIMainActivity* activity = jni::CJNIMainActivity::GetAppInstance();
  if (activity)
    activity->unloadUiSound(m_soundId);
}

void CUISoundAndroid::Play()
{
  jni::CJNIMainActivity* activity = jni::CJNIMainActivity::GetAppInstance();
  if (activity)
    m_streamId = activity->playUiSound(m_soundId, m_volume);
}

void CUISoundAndroid::Stop()
{
  jni::CJNIMainActivity* activity = jni::CJNIMainActivity::GetAppInstance();
  if (activity && m_streamId > 0)
    activity->stopUiSound(m_streamId);
  m_streamId = 0;
}
