/*
 *  Copyright (C) 2026 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "filesystem/File.h"

#include <atomic>
#include <string>

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

/*!
 * \brief Runtime switch for the order of the display mode change (diagnostic builds).
 *
 * On by default: the display mode is changed first, then (once the display has confirmed and
 * a video is playing) the display is declared lost, as in Kodi 20. An empty file
 * "modeorder_off" (or "modeorder_off.txt") in the userdata folder restores the Kodi 21/22
 * order (display declared lost before the mode change) at the next start.
 */
inline bool ModeOrderNexusEnabled()
{
  static const bool enabled = !(XFILE::CFile::Exists("special://profile/modeorder_off") ||
                                XFILE::CFile::Exists("special://profile/modeorder_off.txt"));
  return enabled;
}

/*!
 * \brief Automatic GUI surface refresh during video start (diagnostic builds).
 *
 * Off by default. An empty file "auto_refresh_1", "auto_refresh_2" or "auto_refresh_3" (with or
 * without .txt) in the userdata folder selects the refresh method that is run once, 3 s after
 * the video layer has been switched on. Returns 0 when disabled.
 *   1 = recreate the EGL surface, 2 = hide/show the Java GUI view, 3 = nudge the layer alpha.
 */
inline int AutoRefreshMethod()
{
  static const int method = []()
  {
    for (int m = 1; m <= 3; ++m)
    {
      const std::string base = "special://profile/auto_refresh_" + std::to_string(m);
      if (XFILE::CFile::Exists(base) || XFILE::CFile::Exists(base + ".txt"))
        return m;
    }
    return 0;
  }();
  return method;
}

/*!
 * \brief Number of upcoming frames for which the whole GUI must be redrawn (diagnostic).
 */
inline std::atomic<int>& ForcedRedrawFrames()
{
  static std::atomic<int> frames{0};
  return frames;
}
} // namespace KODI::GUIFIX
