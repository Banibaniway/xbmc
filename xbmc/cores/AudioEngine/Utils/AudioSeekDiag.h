/*
 *  Diagnostic + switchable candidates for the passthrough dropout after a seek.
 *  Temporary: removed from the final build once the cause is settled.
 *
 *  A lever is enabled by creating an empty file named audioseek_<N> in the Kodi
 *  userdata folder (special://profile/). audioseek_7 enables levers 1, 2 and 3.
 *   1: IEC pause bursts forced for every silence (no zero filled silence)
 *   2: keep feeding IEC pause bursts between a flush and the first real packet
 *   3: no AudioTrack stop()/pause() in Drain() while passthrough
 *   4: GetDelay without getLatency() (timestamp API as before)
 *   5: stuck-sink threshold of 21.3, no throttling
 *   6: no reset of the sync servo state in SFlushStream
 */
#pragma once

#include "filesystem/SpecialProtocol.h"
#include "utils/log.h"

#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>

#include <sys/stat.h>

namespace AudioSeekDiag
{
using Clock = std::chrono::steady_clock;

struct State
{
  std::mutex lock;
  std::array<std::atomic<bool>, 8> flags{};
  std::atomic<Clock::rep> lastRefresh{0};
  std::atomic<Clock::rep> lastFlush{0};
  std::atomic<Clock::rep> lastWrite{0};
  std::atomic<Clock::rep> lastDelayLog{0};
  std::atomic<const char*> kind{"?"};
};

inline State& S()
{
  static State s;
  return s;
}

inline Clock::rep NowTicks()
{
  return Clock::now().time_since_epoch().count();
}

inline double MsBetween(Clock::rep from, Clock::rep to)
{
  return std::chrono::duration<double, std::milli>(Clock::duration(to - from)).count();
}

inline void Refresh()
{
  State& s = S();
  const Clock::rep now = NowTicks();
  const Clock::rep last = s.lastRefresh.load();
  if (last != 0 && MsBetween(last, now) < 1000.0)
    return;

  std::unique_lock lock(s.lock);
  s.lastRefresh = now;
  std::string changed;
  for (int n = 1; n <= 7; ++n)
  {
    const std::string path =
        CSpecialProtocol::TranslatePath("special://profile/audioseek_" + std::to_string(n));
    struct stat st;
    const bool on = ::stat(path.c_str(), &st) == 0;
    if (on != s.flags[n].load())
    {
      s.flags[n] = on;
      changed += " " + std::to_string(n) + "=" + (on ? "on" : "off");
    }
  }
  if (!changed.empty())
    CLog::Log(LOGINFO, "AUDIOSEEK levers changed:{} (7 = 1+2+3)", changed);
}

inline bool Flag(int n)
{
  Refresh();
  State& s = S();
  if (n >= 1 && n <= 3 && s.flags[7].load())
    return true;
  return s.flags[n].load();
}

inline void MarkFlush()
{
  S().lastFlush = NowTicks();
}

//! ms since the last audio flush, or a huge value when none happened
inline double SinceFlushMs()
{
  const Clock::rep f = S().lastFlush.load();
  if (f == 0)
    return 1e12;
  return MsBetween(f, NowTicks());
}

//! Called on every AudioTrack write. Logs gaps and everything shortly after a flush.
inline void NoteWrite(unsigned int frames)
{
  State& s = S();
  const char* kind = s.kind.load();
  const Clock::rep now = NowTicks();
  const Clock::rep prev = s.lastWrite.exchange(now);
  const double gap = prev != 0 ? MsBetween(prev, now) : 0.0;
  const double sinceFlush = SinceFlushMs();
  if (gap > 100.0 || sinceFlush < 3000.0)
    CLog::Log(LOGINFO, "AUDIOSEEK write kind={} frames={} gap_since_prev_write={:.1f}ms since_flush={:.0f}ms",
              kind, frames, gap, sinceFlush < 1e11 ? sinceFlush : -1.0);
}

//! Rate limited (100 ms) delay trace for the 3 seconds after a flush
inline void NoteDelay(double delayMs, double hwMs, double pauseMs)
{
  if (SinceFlushMs() > 3000.0)
    return;
  State& s = S();
  const Clock::rep now = NowTicks();
  const Clock::rep prev = s.lastDelayLog.load();
  if (prev != 0 && MsBetween(prev, now) < 100.0)
    return;
  s.lastDelayLog = now;
  CLog::Log(LOGINFO, "AUDIOSEEK delay={:.1f}ms hw={:.1f}ms pause={:.1f}ms since_flush={:.0f}ms",
            delayMs, hwMs, pauseMs, SinceFlushMs());
}
} // namespace AudioSeekDiag
