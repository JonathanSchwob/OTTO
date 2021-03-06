#pragma once

#include "core/engine/engine.hpp"

#include "core/audio/faust.hpp"
#include "core/voices/voice_manager.hpp"

#include <Gamma/Filter.h>
#include <Gamma/Oscillator.h>

#include "util/reflection.hpp"

namespace otto::engines {

  using namespace core;
  using namespace core::engine;
  using namespace props;

  struct GossSynth : SynthEngine, EngineWithEnvelope {
    static constexpr std::string_view name = "Goss";
    struct Props : Properties<> {
      Property<float> drawbar1 = {this, "drawbar1", 1, has_limits::init(0, 1),
                                  steppable::init(0.01)};
      Property<float> drawbar2 = {this, "drawbar2", 0.5, has_limits::init(0, 1),
                                  steppable::init(0.01)};
      Property<float> drawbar3 = {this, "drawbar3", 0.5, has_limits::init(0, 1),
                                  steppable::init(0.01)};
      Property<float> leslie = {this, "leslie", 0.3, has_limits::init(0, 1), steppable::init(0.01)};

      float rotation_value;

    } props;

    GossSynth();

    audio::ProcessData<1> process(audio::ProcessData<1>) override;

    ui::Screen& envelope_screen() override
    {
      return voice_mgr_.envelope_screen();
    }

    ui::Screen& voices_screen() override
    {
      return voice_mgr_.settings_screen();
    }

  private:
    struct Pre : voices::PreBase<Pre, Props> {
      float leslie_speed_hi = 0.f;
      float leslie_speed_lo = 0.f;
      float leslie_amount_hi = 0.f;
      float leslie_amount_lo = 0.f;

      gam::LFO<> leslie_filter_hi;
      gam::LFO<> leslie_filter_lo;
      gam::LFO<> pitch_modulation_lo;
      gam::LFO<> pitch_modulation_hi;

      gam::AccumPhase<> rotation;

      Pre(Props&) noexcept;

      void operator()() noexcept;
    };

    struct Voice : voices::VoiceBase<Voice, Pre> {
      std::array<gam::Osc<>, 4> pipes;
      gam::Osc<> percussion;
      gam::Decay<> perc_env;

      Voice(Pre&) noexcept;

      float operator()() noexcept;

      void on_note_on() noexcept;
    };

    struct Post : voices::PostBase<Post, Voice> {
      gam::Biquad<> lpf;
      gam::Biquad<> hpf;

      Post(Pre&) noexcept;

      float operator()(float) noexcept;
    };

    voices::VoiceManager<Post, 6> voice_mgr_;
  };

} // namespace otto::engines
