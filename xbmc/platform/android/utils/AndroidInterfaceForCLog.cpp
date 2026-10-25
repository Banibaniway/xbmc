/*
 *  Copyright (C) 2020 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "platform/android/utils/AndroidInterfaceForCLog.h"

#include "CompileInfo.h"

#include <dlfcn.h>
#include <mutex>
#include <string>

#include <android/log.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/dist_sink.h>

namespace
{
// Replacement for spdlog's android_sink: logcat refuses (error return) the messages of the
// priorities it has disabled (debug on most retail devices, e.g. Fire TV). The spdlog 1.10 sink
// throws in that case, and since the exception propagates through the distribution sink, the
// sinks registered after it (the kodi.log file) never receive the message: no debug line in
// kodi.log. Here a logcat error is simply ignored.
class CAndroidLogcatSink : public spdlog::sinks::base_sink<std::mutex>
{
public:
  explicit CAndroidLogcatSink(std::string tag) : m_tag(std::move(tag)) {}

protected:
  void sink_it_(const spdlog::details::log_msg& msg) override
  {
    spdlog::memory_buf_t formatted;
    base_sink<std::mutex>::formatter_->format(msg, formatted);
    formatted.push_back('\0');
    __android_log_write(ToPriority(msg.level), m_tag.c_str(), formatted.data());
  }

  void flush_() override {}

private:
  static android_LogPriority ToPriority(spdlog::level::level_enum level)
  {
    switch (level)
    {
      case spdlog::level::trace:
        return ANDROID_LOG_VERBOSE;
      case spdlog::level::debug:
        return ANDROID_LOG_DEBUG;
      case spdlog::level::info:
        return ANDROID_LOG_INFO;
      case spdlog::level::warn:
        return ANDROID_LOG_WARN;
      case spdlog::level::err:
        return ANDROID_LOG_ERROR;
      case spdlog::level::critical:
        return ANDROID_LOG_FATAL;
      default:
        return ANDROID_LOG_DEFAULT;
    }
  }

  std::string m_tag;
};
} // namespace

// On some Android platforms debug logging is deactivated.
// We try to activate debug logging for our app via function "__android_log_set_minimum_priority" from "/system/lib/liblog.so".
// The function is defined in API level 30 (Android 11) as:
//   int32_t __android_log_set_minimum_priority(int32_t priority);
void ActivateAndroidDebugLogging()
{
  void* libHandle = dlopen("liblog.so", RTLD_LAZY);
  if (libHandle)
  {
    void* funcPtr = dlsym(libHandle, "__android_log_set_minimum_priority");
    if (funcPtr)
    {
      typedef int32_t (*android_log_set_minimum_priority_func)(int32_t);
      reinterpret_cast<android_log_set_minimum_priority_func>(funcPtr)(ANDROID_LOG_DEBUG);
    }
    dlclose(libHandle);
  }
}

std::unique_ptr<IPlatformLog> IPlatformLog::CreatePlatformLog()
{
  ActivateAndroidDebugLogging();
  return std::make_unique<CAndroidInterfaceForCLog>();
}

void CAndroidInterfaceForCLog::AddSinks(
    std::shared_ptr<spdlog::sinks::dist_sink<std::mutex>> distributionSink) const
{
  distributionSink->add_sink(
      std::make_shared<CAndroidLogcatSink>(CCompileInfo::GetAppName()));
}
