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
 * \brief Runtime switch for the one-time GUI surface refresh during video start (diagnostic).
 *
 * On by default: about 3 s after the video layer has been switched on, the GUI EGL surface is
 * recreated once and the whole GUI redrawn. An empty file "surface_refresh_off" (or
 * "surface_refresh_off.txt") in the userdata folder disables it at the next start.
 */
inline bool SurfaceRefreshEnabled()
{
  static const bool enabled = !(XFILE::CFile::Exists("special://profile/surface_refresh_off") ||
                                XFILE::CFile::Exists("special://profile/surface_refresh_off.txt"));
  return enabled;
}
} // namespace KODI::GUIFIX
