#include "IPlugVisualizer.h"
#include "IControls.h"
#include "IPlug_include_in_plug_src.h"

#include <random>
#include <vector>

constexpr int kMsgTagAudioLevel = 1000;
constexpr int kCtrlTagParticles = 2001;

// -----------------------------
// Particle Field Control
// -----------------------------
struct Particle
{
  float x, y;
  float vx, vy;
  float brightness;
  float size;
};

class IParticleFieldControl : public IControl
{
public:
  IParticleFieldControl(const IRECT& bounds, int numParticles = 600)
    : IControl(bounds)
  {
    mParticles.resize(numParticles);
    RandomizeParticles();
  }

  void Draw(IGraphics& g) override
  {
    // Faint translucent overlay for motion trails
    g.FillRect(IColor(10, 0, 0, 0), mRECT);

    for (auto& p : mParticles)
    {
      IColor color = IColor((int)(p.brightness * 255.f), 0, 200, 255);
      g.FillCircle(color, p.x, p.y, p.size);
    }
  }

  void OnResize() override { RandomizeParticles(); }

  void Animate()
  {
    UpdateParticles();
    SetDirty(false);
  }

  void OnMsgFromDelegate(int msgTag, int dataSize, const void* pData) override
  {
    if (msgTag == kMsgTagAudioLevel)
    {
      double level = *reinterpret_cast<const double*>(pData);
      mAudioLevel = (float)std::clamp(level * 10.0, 0.0, 1.0);
    }
  }

  void SetPulseParams(double strength, double speed)
  {
    mPulseStrength = (float)strength;
    mPulseSpeed = (float)speed;
  }

private:
  std::vector<Particle> mParticles;
  std::default_random_engine mRng{std::random_device{}()};
  std::uniform_real_distribution<float> mDistX;
  std::uniform_real_distribution<float> mDistY;
  float mAudioLevel = 0.0f;
  float mPulseStrength = 1.0f;
  float mPulseSpeed = 1.0f;

  void RandomizeParticles()
  {
    mDistX = std::uniform_real_distribution<float>(mRECT.L, mRECT.R);
    mDistY = std::uniform_real_distribution<float>(mRECT.T, mRECT.B);

    for (auto& p : mParticles)
    {
      p.x = mDistX(mRng);
      p.y = mDistY(mRng);
      p.vx = ((float)rand() / RAND_MAX - 0.5f) * 0.5f;
      p.vy = ((float)rand() / RAND_MAX - 0.5f) * 0.5f;
      p.brightness = 0.2f + 0.8f * ((float)rand() / RAND_MAX);
      p.size = 1.0f + 2.5f * ((float)rand() / RAND_MAX);
    }
  }

void UpdateParticles()
  {
    const float cx = mRECT.MW();
    const float cy = mRECT.MH();

    for (auto& p : mParticles)
    {
      // Vector from center
      float dx = p.x - cx;
      float dy = p.y - cy;
      float dist = std::sqrt(dx * dx + dy * dy) + 0.0001f;

      // Normalize
      dx /= dist;
      dy /= dist;

      // ----- 1. Outward force from audio -----
      float pulse = (mAudioLevel - 0.25f) * 12.0f * mPulseStrength;
      p.vx += dx * pulse * 0.15f * mPulseSpeed;
      p.vy += dy * pulse * 0.15f * mPulseSpeed;

      // ----- 2. Inward attraction toward center -----
      float pull = (dist * 0.003f) + 0.015f; // stronger pull the further out
      p.vx -= dx * pull;
      p.vy -= dy * pull;

      // ----- 3. Gentle random drift -----
      p.vx += ((float)rand() / RAND_MAX - 0.5f) * 0.02f;
      p.vy += ((float)rand() / RAND_MAX - 0.5f) * 0.02f;

      // ----- 4. Damping -----
      p.vx *= 0.9f;
      p.vy *= 0.9f;

      // ----- 5. Move -----
      p.x += p.vx;
      p.y += p.vy;

      // ----- 6. Keep within safe bounds -----
      const float margin = 10.f;
      if (p.x < mRECT.L + margin)
        p.vx = std::abs(p.vx) * 0.5f;
      if (p.x > mRECT.R - margin)
        p.vx = -std::abs(p.vx) * 0.5f;
      if (p.y < mRECT.T + margin)
        p.vy = std::abs(p.vy) * 0.5f;
      if (p.y > mRECT.B - margin)
        p.vy = -std::abs(p.vy) * 0.5f;

      // ----- 7. Brightness pulse -----
      p.brightness = 0.4f + mAudioLevel * 0.9f + ((float)rand() / RAND_MAX) * 0.1f;
      p.brightness = std::clamp(p.brightness, 0.3f, 1.f);
    }
  }
};

// -----------------------------
// IPlugVisualizer Plugin
// -----------------------------
IPlugVisualizer::IPlugVisualizer(const InstanceInfo& info)
  : iplug::Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kOctaveGain)->InitDouble("OctaveGain", 0.0, 0., 12.0, 0.1, "dB");
  GetParam(kPulseStrength)->InitDouble("Pulse Strength", 1.0, 0.0, 5.0, 0.01, "");
  GetParam(kPulseSpeed)->InitDouble("Pulse Speed", 1.0, 0.0, 5.0, 0.01, "");

#if IPLUG_EDITOR
  mMakeGraphicsFunc = [&]() { return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT)); };

  mLayoutFunc = [&](IGraphics* pGraphics) {
    const IRECT b = pGraphics->GetBounds();

    pGraphics->AttachPanelBackground(COLOR_BLACK);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    pGraphics->ShowFPSDisplay(true);

    // Attach particle field
    mParticleControl = static_cast<IParticleFieldControl*>(pGraphics->AttachControl(new IParticleFieldControl(b, 800), kCtrlTagParticles));

    // Title
    pGraphics->AttachControl(new ITextControl(IRECT(0, 0, b.R, 30), "Audio-Reactive Particle Visualizer", IText(18, COLOR_WHITE, "Roboto-Regular", EAlign::Center)));

    // Styled sliders for Pulse Strength / Speed
    const float sliderH = 40.f;
    const float sliderW = 120.f;
    const float margin = 20.f;

    IVStyle sliderStyle = DEFAULT_STYLE.WithColor(kBG, IColor(255, 30, 30, 30))
                            .WithColor(kFG, IColor(255, 0, 200, 255))
                            .WithColor(kPR, IColor(255, 0, 150, 255))
                            .WithColor(kSH, IColor(255, 255, 255, 255))
                            .WithLabelText(IText(14, COLOR_WHITE, "Roboto-Regular"))
                            .WithValueText(IText(14, COLOR_WHITE, "Roboto-Regular"));

    pGraphics->AttachControl(new IVSliderControl(IRECT(30, b.B - sliderH - margin, 30 + sliderW, b.B - margin), kPulseStrength, "Strength", sliderStyle));

    pGraphics->AttachControl(new IVSliderControl(IRECT(180, b.B - sliderH - margin, 180 + sliderW, b.B - margin), kPulseSpeed, "Speed", sliderStyle));

  };
#endif
}

// -----------------------------
// DSP
// -----------------------------
#if IPLUG_DSP
void IPlugVisualizer::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const int nChans = NOutChansConnected();
  double sum = 0.0;
  int count = 0;

  // Pass-through & RMS
  for (int s = 0; s < nFrames; ++s)
  {
    double mono = 0.0;
    for (int c = 0; c < nChans; ++c)
    {
      outputs[c][s] = inputs[c][s];
      mono += inputs[c][s];
    }
    mono /= std::max(1, nChans);
    sum += mono * mono;
    count++;
  }

  const double rms = sqrt(sum / (double)std::max(1, count));

  SendControlMsgFromDelegate(kCtrlTagParticles, kMsgTagAudioLevel, sizeof(double), &rms);
}

void IPlugVisualizer::OnIdle()
{
  #if IPLUG_EDITOR
  if (mParticleControl)
  {
    const double strength = GetParam(kPulseStrength)->Value();
    const double speed = GetParam(kPulseSpeed)->Value();
    mParticleControl->SetPulseParams(strength, speed);
    mParticleControl->Animate();
  }
  #endif
}

void IPlugVisualizer::OnReset() {}
#endif

// -----------------------------
// UI callbacks
// -----------------------------
#if IPLUG_EDITOR
void IPlugVisualizer::OnParamChangeUI(int paramIdx, EParamSource)
{
  if (paramIdx == kOctaveGain)
  {
    const double v = GetParam(kOctaveGain)->Value();
    DBGMSG("OctaveGain changed: %f\n", v);
  }
}
#endif
