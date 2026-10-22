/*
 *  Copyright (C) 2026 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "filesystem/File.h"

namespace KODI::GUIFIX
{
/*!
 * \brief Runtime switch for the Android GUI redraw fix (diagnostic builds).
 *
 * The fix is on by default. Creating an empty file "gui_fix_off" (or "gui_fix_off.txt")
 * in the userdata folder turns it off at the next start, so that the original behaviour
 * can be observed with the very same build.
 */
inline bool Enabled()
{
  static const bool enabled = !(XFILE::CFile::Exists("special://profile/gui_fix_off") ||
                                XFILE::CFile::Exists("special://profile/gui_fix_off.txt"));
  return enabled;
}

/*!
 * \brief Runtime switch for the Android HDMI-loss deadlock fix (diagnostic builds).
 *
 * On by default. An empty file "hdmi_fix_off" (or "hdmi_fix_off.txt") in the userdata
 * folder restores the original behaviour at the next start.
 */
inline bool HdmiFixEnabled()
{
  static const bool enabled = !(XFILE::CFile::Exists("special://profile/hdmi_fix_off") ||
                                XFILE::CFile::Exists("special://profile/hdmi_fix_off.txt"));
  return enabled;
}
} // namespace KODI::GUIFIX
