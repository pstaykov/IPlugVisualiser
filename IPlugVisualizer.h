#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "ISender.h"

const int kNumPresets = 1;

enum EParams
{
  kOctaveGain = 0,
  kPulseStrength,
  kPulseSpeed,
  kNumParams
};

using namespace iplug;
using namespace igraphics;

class IParticleFieldControl; // forward declaration

class IPlugVisualizer final : public Plugin
{
public:
  IPlugVisualizer(const InstanceInfo& info);

#if IPLUG_DSP
  void OnReset() override;
  void OnIdle() override;
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  bool OnMessage(int, int, int, const void*) override { return false; }
  ISpectrumSender<2> mSender; // reserved for future FFT use
#endif

#if IPLUG_EDITOR
  void OnParamChangeUI(int paramIdx, EParamSource source) override;
#endif

private:
#if IPLUG_EDITOR
  IParticleFieldControl* mParticleControl = nullptr;
#endif
};
