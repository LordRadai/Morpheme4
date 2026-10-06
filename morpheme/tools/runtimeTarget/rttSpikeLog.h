// Frame spike logger for the runtime target.
//
// Times each phase of the runtime target frame and, when a frame takes noticeably longer than the running average,
// prints a per-phase breakdown to stdout and appends it to rttSpikes.log in the working directory.
// Define RTT_DISABLE_SPIKE_LOGGING to compile it out.

//----------------------------------------------------------------------------------------------------------------------
#ifdef _MSC_VER
  #pragma once
#endif
#ifndef RTT_SPIKE_LOG_H
#define RTT_SPIKE_LOG_H
//----------------------------------------------------------------------------------------------------------------------
#include "NMPlatform/NMPlatform.h"
#include "NMPlatform/NMTimer.h"
#include <stdio.h>
//----------------------------------------------------------------------------------------------------------------------

#ifndef RTT_DISABLE_SPIKE_LOGGING
  #define RTT_SPIKE_LOGGING
#endif

namespace RTTSpikeLog
{

enum Phase
{
  kCommsUpdate,
  kCommsBeginFrame,
  kApplyAnimSetChanges,
  kNetStartUpdate,         // Network::startUpdate: attrib lifespans, updateNodeInstanceConnections, task queuing.
  kConnectivity,
  kNetDispatchController,  // Dispatch until MR_TASKID_NETWORKUPDATECHARACTERCONTROLLER.
  kPreController,
  kControllers,
  kNetDispatchPhysics,     // Dispatch until MR_TASKID_NETWORKUPDATEPHYSICS.
  kPrePhysics,
  kNetDispatchRoot,        // Dispatch until MR_TASKID_NETWORKUPDATEROOT.
  kPostPhysics,
  kNetDispatchFinal,       // Dispatch the rest of the queue.
  kNetEndUpdate,
  kSceneObjects,
  kControllerReps,
  kProfileDebugOutput,
  kCommsEndFrame,
  kNumPhases
};

static const char* const s_phaseNames[kNumPhases] =
{
  "commsUpdate",
  "commsBeginFrame",
  "applyAnimSetChanges",
  "netStartUpdate",
  "connectivity",
  "netDispatchCC",
  "preController",
  "controllers",
  "netDispatchPhysics",
  "prePhysics",
  "netDispatchRoot",
  "postPhysics",
  "netDispatchFinal",
  "netEndUpdate",
  "sceneObjects",
  "controllerReps",
  "profileDebugOutput",
  "commsEndFrame",
};

// A frame is a spike when it takes longer than both of these.
static const float SPIKE_MIN_MS = 2.0f;
static const float SPIKE_AVERAGE_FACTOR = 2.0f;

struct State
{
  float    m_times[kNumPhases];
  float    m_averageMs;
  uint32_t m_numFrames;
};

NM_INLINE State& getState()
{
  static State state = {};
  return state;
}

NM_INLINE void beginFrame()
{
  State& state = getState();
  for (uint32_t i = 0; i < kNumPhases; ++i)
    state.m_times[i] = 0.0f;
}

NM_INLINE void addTime(Phase phase, float ms)
{
  getState().m_times[phase] += ms;
}

NM_INLINE void endFrame(int32_t frameIndex)
{
  State& state = getState();

  float total = 0.0f;
  for (uint32_t i = 0; i < kNumPhases; ++i)
    total += state.m_times[i];

  // Let the average settle before reporting anything.
  static const uint32_t WARMUP_FRAMES = 30;
  const bool isSpike = state.m_numFrames >= WARMUP_FRAMES &&
                       total > SPIKE_MIN_MS &&
                       total > state.m_averageMs * SPIKE_AVERAGE_FACTOR;

  if (isSpike)
  {
    char line[1024];
    int len = NMP_SPRINTF(line, sizeof(line), "[RTT spike] frame %d: %.3f ms (avg %.3f ms) |", frameIndex, total, state.m_averageMs);
    for (uint32_t i = 0; i < kNumPhases && len > 0 && len < (int)sizeof(line); ++i)
    {
      if (state.m_times[i] >= 0.05f)
        len += NMP_SPRINTF(line + len, sizeof(line) - len, " %s=%.3f", s_phaseNames[i], state.m_times[i]);
    }

    printf("%s\n", line);

    FILE* file = NULL;
  #ifdef _MSC_VER
    fopen_s(&file, "rttSpikes.log", "a");
  #else
    file = fopen("rttSpikes.log", "a");
  #endif
    if (file)
    {
      fprintf(file, "%s\n", line);
      fclose(file);
    }
  }
  else
  {
    // Spikes are kept out of the average so one long frame doesn't hide the next.
    state.m_averageMs = (state.m_numFrames == 0) ? total : state.m_averageMs * 0.95f + total * 0.05f;
    ++state.m_numFrames;
  }
}

/// Adds the time spent in its scope to a phase.
class ScopedPhase
{
public:
  NM_INLINE ScopedPhase(Phase phase) : m_phase(phase), m_timer(true) {}
  NM_INLINE ~ScopedPhase() { addTime(m_phase, m_timer.stop()); }

private:
  Phase      m_phase;
  NMP::Timer m_timer;
};

} // namespace RTTSpikeLog

#ifdef RTT_SPIKE_LOGGING
  #define RTT_SPIKE_BEGIN_FRAME() RTTSpikeLog::beginFrame()
  #define RTT_SPIKE_END_FRAME(frameIndex) RTTSpikeLog::endFrame(frameIndex)
  #define RTT_SPIKE_PHASE(phase) RTTSpikeLog::ScopedPhase NM_ASSERT_CONCAT(rttSpikePhase_, __LINE__)(RTTSpikeLog::phase)
#else
  #define RTT_SPIKE_BEGIN_FRAME()
  #define RTT_SPIKE_END_FRAME(frameIndex)
  #define RTT_SPIKE_PHASE(phase)
#endif

//----------------------------------------------------------------------------------------------------------------------
#endif // RTT_SPIKE_LOG_H
//----------------------------------------------------------------------------------------------------------------------
