#include "clips.h"

#include <cassert>
#include <cmath>

#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"

namespace clips {

// Don't set above 7 without expanding lookup tables in the header.
constexpr int max_procs = 6;

bool State::IsStrictlyWorseThan(const State& other) const {
  if ((projects_ & kWin) && !(other.projects_ & kWin)) {
    return false;
  }
  if ((other.projects_ & kWin) && (time_ > other.time_ + eps)) {
    return true;
  }
  if (time_ + eps < other.time_ || ops_ > other.ops_ + eps ||
      creat_ > other.creat_ + eps || clips_ > other.clips_ + eps ||
      dollars_ > other.dollars_ + eps || processors_ != other.processors_ ||
      memory_ != other.memory_ || auto_clippers_ != other.auto_clippers_ ||
      mlvl_ != other.mlvl_ || (projects_ & other.projects_) != projects_) {
    return false;
  }
  return true;
}

std::unique_ptr<State> State::PassTime(double seconds) const {
  auto copy = absl::make_unique<State>(*this);
  copy->time_ += seconds;
  copy->clips_ += ClipsPerSecond() * seconds;
  copy->dollars_ += DollarsPerSecond() * seconds;
  copy->ops_ += OpsPerSecond() * seconds;
  copy->creat_ = std::min(copy->creat_ + CreatPerSecond() * seconds, 250.);
  return copy;
}

bool State::MeetsPrereqs(uint32_t project) const {
  if (project & projects_) {
    // already purchased
    return false;
  }
  if (project == kEvenBetterAutoclippers) {
    return (projects_ & kImprovedAutoclippers);
  } else if (project == kOptimizedAutoclippers) {
    return (projects_ & kEvenBetterAutoclippers);
  } else if (project == kHadwigerClipDiagrams) {
    return (projects_ & kHadwigerProblem);
  } else if (project == kOptimizedWireExtrusion) {
    return (projects_ & kImprovedWireExtrusion);
  } else if (project == kMicrolatticeShapecasting) {
    return (projects_ & kOptimizedWireExtrusion);
  } else if (project == kNewSlogan) {
    return (projects_ & (kLexicalProcessing | kSloganCreat)) ==
           (kLexicalProcessing | kSloganCreat);
  } else if (project == kCatchyJingle) {
    return (projects_ & (kLexicalProcessing | kSloganCreat)) ==
           (kLexicalProcessing | kSloganCreat);
  } else if (project == kHypnoHarmonics) {
    return (projects_ & kCatchyJingle);
  }
  return true;
}

double State::NextOpsLimit() const {
  const double ops_limit = 1000. * memory_;
  if (ops_ == ops_limit || clips_ < 2000.) {
    return HUGE_VAL;  // We aren't earning ops, nothing to save for
  } else if (ops_ < 750. && MeetsPrereqs(kImprovedAutoclippers)) {
    return 750.;
  } else if (ops_ < 1000. && (memory_ == 1 || MeetsPrereqs(kCreativity))) {
    return 1000.;
  } else if (ops_ < 1750. && MeetsPrereqs(kImprovedWireExtrusion)) {
    return 1750.;
  } else if (ops_ < 2000. && memory_ == 2) {
    return 2000.;
  } else if (ops_ < 2500. && (MeetsPrereqs(kEvenBetterAutoclippers) ||
                              MeetsPrereqs(kNewSlogan))) {
    return 2500.;
  } else if (ops_ < 3000. && memory_ == 3) {
    return 3000.;
  } else if (ops_ < 3500. && MeetsPrereqs(kOptimizedWireExtrusion)) {
    return 3500.;
  } else if (ops_ < 4000. && memory_ == 4) {
    return 4000.;
  } else if (ops_ < 5000. &&
             (memory_ == 5 || MeetsPrereqs(kOptimizedAutoclippers))) {
    return 5000.;
  } else if (ops_ < 6000. &&
             (memory_ == 6 || MeetsPrereqs(kHadwigerClipDiagrams))) {
    return 6000.;
  } else if (ops_ < 7000. && memory_ == 7) {
    return 7000.;
  } else if (ops_ < 7500. && (MeetsPrereqs(kMicrolatticeShapecasting) ||
                              MeetsPrereqs(kHypnoHarmonics))) {
    return 7500.;
  } else {
    return ops_limit;
  }
}

std::pair<double, bool> State::NextCreatLimit() const {
  if (ops_ < memory_ * 1000. || !(projects_ & kCreativity) || creat_ > 250.) {
    // We aren't earning creat or have bought all creat projects; nothing to
    // save for
    return {HUGE_VAL, false};
  } else if (creat_ < 10. && MeetsPrereqs(kLimerick)) {
    const uint32_t rest = kSloganCreat | kJingleCreat | kLexicalProcessing |
                          kCombinatoryHarmonics | kHadwigerProblem |
                          kTothSausageConjecture | kDonkeySpace;
    const bool must_buy = ((projects_ & rest) == rest);
    return {10., must_buy};
  } else if (creat_ < 25. && MeetsPrereqs(kSloganCreat)) {
    const uint32_t rest = kJingleCreat | kLexicalProcessing |
                          kCombinatoryHarmonics | kHadwigerProblem |
                          kTothSausageConjecture | kDonkeySpace;
    const bool must_buy = ((projects_ & rest) == rest);
    return {25., must_buy};
  } else if (creat_ < 45. && MeetsPrereqs(kJingleCreat)) {
    const uint32_t rest = kLexicalProcessing | kCombinatoryHarmonics |
                          kHadwigerProblem | kTothSausageConjecture |
                          kDonkeySpace;
    const bool must_buy = ((projects_ & rest) == rest);
    return {45., must_buy};
  } else if (creat_ < 50. && MeetsPrereqs(kLexicalProcessing)) {
    const uint32_t rest = kCombinatoryHarmonics | kHadwigerProblem |
                          kTothSausageConjecture | kDonkeySpace;
    const bool must_buy = ((projects_ & rest) == rest);
    return {50., must_buy};
  } else if (creat_ < 100. && MeetsPrereqs(kCombinatoryHarmonics)) {
    const uint32_t rest =
        kHadwigerProblem | kTothSausageConjecture | kDonkeySpace;
    const bool must_buy = ((projects_ & rest) == rest);
    return {100., must_buy};
  } else if (creat_ < 150. && MeetsPrereqs(kHadwigerProblem)) {
    const uint32_t rest = kTothSausageConjecture | kDonkeySpace;
    const bool must_buy = ((projects_ & rest) == rest);
    return {150., must_buy};
  } else if (creat_ < 200. && MeetsPrereqs(kTothSausageConjecture)) {
    const uint32_t rest = kDonkeySpace;
    const bool must_buy = ((projects_ & rest) == rest);
    return {200., must_buy};
  } else if (creat_ < 250. && MeetsPrereqs(kDonkeySpace)) {
    return {250., true};
  }
  return {HUGE_VAL, false};
}

namespace {

struct OpsProject {
  double cost;
  uint32_t project;
  uint32_t next_project;
};

constexpr int kNumOpsProjects = 11;
constexpr OpsProject kOpsProjects[kNumOpsProjects] = {
    {7500., State::kHypnoHarmonics, State::kMicrolatticeShapecasting},
    {7500., State::kMicrolatticeShapecasting, State::kHadwigerClipDiagrams},
    {6000., State::kHadwigerClipDiagrams, State::kOptimizedAutoclippers},
    {5000., State::kOptimizedAutoclippers, State::kCatchyJingle},
    {4500., State::kCatchyJingle, State::kOptimizedWireExtrusion},
    {3500., State::kOptimizedWireExtrusion, State::kNewSlogan},
    {2500., State::kNewSlogan, State::kEvenBetterAutoclippers},
    {2500., State::kEvenBetterAutoclippers, State::kImprovedWireExtrusion},
    {1750., State::kImprovedWireExtrusion, State::kCreativity},
    {1000., State::kCreativity, State::kImprovedAutoclippers},
    {750., State::kImprovedAutoclippers, State::kNothing},
};

}  // namespace

// Potentially purchase things when we reach a threshold.
void State::AddOpsPurchases(BranchList* br, double ops_thresh,
                            double ops_thresh_time) const {
  for (const auto& item : kOpsProjects) {
    if (ops_thresh == item.cost && MeetsPrereqs(item.project)) {
      br->push_back(PassTime(ops_thresh_time));
      br->back()->ops_ = 0.;
      br->back()->AwardProject(item.project);
    }
  }
}

void State::AddCreatPurchase(BranchList* br, double creat_thresh,
                             double creat_thresh_time) const {
  struct Purchase {
    double cost;
    uint32_t project;
    bool earns_trust;
  };
  static constexpr Purchase creat_project_list[] = {
      {10., kLimerick, true},
      {25., kSloganCreat, false},
      {45., kJingleCreat, false},
      {50., kLexicalProcessing, true},
      {100., kCombinatoryHarmonics, true},
      {150., kHadwigerProblem, true},
      {200., kTothSausageConjecture, true},
      {250., kDonkeySpace, true},
  };
  for (const auto& item : creat_project_list) {
    if (creat_thresh == item.cost && MeetsPrereqs(item.project)) {
      br->push_back(PassTime(creat_thresh_time));
      br->back()->creat_ = 0.;
      br->back()->AwardProject(item.project);
      if (item.earns_trust) {
        br->back()->trust_ += 1;
        br->back()->spree_ = kSpreeProcessor;
      } else {
        br->back()->spree_ = kSpreeMemory;
      }
    }
  }
}

// Return a sequence of possible branch states from here.
State::BranchList State::DoBranches(double limit) const {
  State::BranchList ret;

  // Abandon this branch if we are losing money, are capped on creat, are
  // earning creat unnecessarily, or have won.
  double dollars_per_second = DollarsPerSecond();
  if (dollars_per_second <= 0. || creat_ >= 250. || Win()) {
    return ret;
  }
  constexpr uint32_t all_creat_sinks =
      kLimerick | kLexicalProcessing | kCombinatoryHarmonics |
      kHadwigerProblem | kTothSausageConjecture | kDonkeySpace | kSloganCreat |
      kJingleCreat;
  if ((projects_ & all_creat_sinks) == all_creat_sinks && creat_ > 0.) {
    return ret;
  }

  double next_autoclipper_thresh;
  double dollars_spent = DollarsSpent();
  if (auto_clippers_ > 0) {
    next_autoclipper_thresh =
        dollars_spent + 5. + std::pow(1.1, auto_clippers_);
  } else {
    next_autoclipper_thresh = dollars_spent + 5.;
  }
  double next_mlvl_thresh = dollars_spent + 50 * std::pow(2., mlvl_);
  double lower_cost = std::min(next_autoclipper_thresh, next_mlvl_thresh);
  double higher_cost = std::max(next_autoclipper_thresh, next_mlvl_thresh);
  bool optional_dollar_purchase = (dollars_ < lower_cost);
  double dollars_thresh = (dollars_ < lower_cost) ? lower_cost : higher_cost;
  double dollars_thresh_time = (dollars_thresh - dollars_) / dollars_per_second;

  // Find next clips threshold
  static const double clips_limits[11] = {2000.,  3000.,   5000.,   8000.,
                                          13000., 21000.,  34000.,  55000.,
                                          89000., 144000., HUGE_VAL};
  double clips_thresh =
      *std::upper_bound(clips_limits, clips_limits + 11, clips_);
  bool halt = false;
  if (clips_thresh > limit) {
    clips_thresh = limit;
    halt = true;
  }
  double clips_thresh_time = (clips_thresh - clips_) / ClipsPerSecond();

  // Find next ops threshold
  double ops_thresh = NextOpsLimit();
  double ops_thresh_time = HUGE_VAL;
  if (ops_thresh < HUGE_VAL) {
    ops_thresh_time = (ops_thresh - ops_) / OpsPerSecond();
  }

  // Find next creat threshold
  double creat_thresh;
  double creat_thresh_time = HUGE_VAL;
  bool creat_must_buy;
  std::tie(creat_thresh, creat_must_buy) = NextCreatLimit();
  if (creat_thresh < HUGE_VAL) {
    creat_thresh_time = (creat_thresh - creat_) / CreatPerSecond();
  }

  if (creat_thresh_time < HUGE_VAL && creat_thresh_time == ops_thresh_time) {
    assert(!"fuck you I'll pee my pants");
  }

  // Figure out what our next decision point will be
  // Dollars decision point?
  if (dollars_thresh_time < clips_thresh_time &&
      dollars_thresh_time < ops_thresh_time &&
      dollars_thresh_time < creat_thresh_time) {
    if (dollars_thresh == next_autoclipper_thresh) {
      // buy an autoclipper
      ret.push_back(PassTime(dollars_thresh_time));
      ret.back()->dollars_ = dollars_thresh;
      ret.back()->auto_clippers_ += 1;
    } else {
      // buy a market level
      assert(dollars_thresh == next_mlvl_thresh);
      ret.push_back(PassTime(dollars_thresh_time));
      ret.back()->dollars_ = dollars_thresh;
      ret.back()->mlvl_ += 1;
      ret.back()->LogMlvl();
    }
    if (optional_dollar_purchase) {
      // Branch: Save up for the more expensive thing instead
      ret.push_back(PassTime(dollars_thresh_time));
      ret.back()->dollars_ = dollars_thresh;
    }
    return ret;
  }
  // Clips decision point?
  if (clips_thresh_time < dollars_thresh_time &&
      clips_thresh_time < ops_thresh_time &&
      clips_thresh_time < creat_thresh_time) {
    if (halt) {
      // forced stopping point
      ret.push_back(PassTime(clips_thresh_time));
      ret.back()->clips_ = clips_thresh;
      return ret;
    }
    if (clips_thresh == 2000.) {
      // Operations are now online.  (Doesn't earn trust.)
      ret.push_back(PassTime(clips_thresh_time));
      ret.back()->clips_ = clips_thresh;
      return ret;
    }
    int hypno_harmonics = (projects_ & kHypnoHarmonics) ? 1 : 0;
    if (trust_ < memory_ + processors_ + hypno_harmonics) {
      // We earned trust, but were in the red so can't spend now.
      // (This can happen when we spend trust on hypno harmonics.)
      ret.push_back(PassTime(clips_thresh_time));
      ret.back()->clips_ = clips_thresh;
      ret.back()->trust_ += 1;
      return ret;
    }
    // Branch options when we are awarded a trust:
    // Branch 1: buy a processor.  Don't buy more than 7.
    if (processors_ < max_procs) {
      ret.push_back(PassTime(clips_thresh_time));
      ret.back()->clips_ = clips_thresh;
      ret.back()->trust_ += 1;
      ret.back()->processors_ += 1;
      ret.back()->LogProcessor();
      // If this is the 5th processor and we have 10000 ops, we win!
      if (processors_ == 5 && ops_ == 10000.) {
        ret.back()->projects_ |= kWin;
        return ret;
      }
    }
    // Branch 2: Don't spend the new trust.  This can happen when:
    //   a) we aren't capped and will buy memory when we hit the cap
    //   b) we are capped, but want to keep earning trust for now
    // If we already have enough trust to purchase both 10 memory and
    // hypno harmonics, don't do this: there's nothing left to save for.
    if (trust_ < processors_ + 11) {
      ret.push_back(PassTime(clips_thresh_time));
      ret.back()->clips_ = clips_thresh;
      ret.back()->trust_ += 1;
    }
    // Branch 3: Immediately buy new memory.  This only makes sense if
    // we're currently capped on ops and don't have 10 memory already.
    // TODO(only if all other trust was allocated)
    if (memory_ < 10 && ops_ == memory_ * 1000.) {
      ret.push_back(PassTime(clips_thresh_time));
      ret.back()->clips_ = clips_thresh;
      ret.back()->trust_ += 1;
      ret.back()->memory_ += 1;
      ret.back()->LogMemory();
    }
    return ret;
  }
  // Ops decision point?
  if (ops_thresh_time < dollars_thresh_time &&
      ops_thresh_time < clips_thresh_time &&
      ops_thresh_time < creat_thresh_time) {
    // Immediate win?
    if (ops_thresh == 10000. && processors_ >= 5) {
      ret.push_back(PassTime(ops_thresh_time));
      ret.back()->ops_ = 10000.;
      ret.back()->projects_ |= kWin;
      return ret;
    }
    // Branch 1: If we can purchase anything with ops, add those branches
    AddOpsPurchases(&ret, ops_thresh, ops_thresh_time);
    // Branch 2: If we're not capped, or if we can start to earn creativity,
    // buy nothing to earn more ops/creat.
    if (ops_thresh != memory_ * 1000. || (projects_ & kCreativity)) {
      ret.push_back(PassTime(ops_thresh_time));
      ret.back()->ops_ = ops_thresh;
    }
    // Branch 3: Buy more memory.  This only works if we're at the cap, and
    // we have the trust to spend.
    int hypno_harmonics = (projects_ & kHypnoHarmonics) ? 1 : 0;
    if (ops_thresh == memory_ * 1000. &&
        trust_ > processors_ + memory_ + hypno_harmonics) {
      ret.push_back(PassTime(ops_thresh_time));
      ret.back()->ops_ = ops_thresh;
      ret.back()->memory_ += 1;
      ret.back()->LogMemory();
    }
    return ret;
  }
  // Creat decision point?
  if (creat_thresh_time < dollars_thresh_time &&
      creat_thresh_time < clips_thresh_time &&
      creat_thresh_time < ops_thresh_time) {
    // Branch 1: Buy if you can
    AddCreatPurchase(&ret, creat_thresh, creat_thresh_time);
    // Branch 2: Save for the next thing.
    if (!creat_must_buy) {
      ret.push_back(PassTime(creat_thresh_time));
      ret.back()->creat_ = creat_thresh;
    }
    return ret;
  }
  std::cerr << "NO CHOICE WAS BEST? " << creat_thresh_time << " "
            << dollars_thresh_time << " " << clips_thresh_time << " "
            << ops_thresh_time << "\n";
  std::cerr << *this << "\n";
  assert(!"No choice was best?");
  __builtin_trap();
}

State::BranchList State::Branches(double limit) const {
  State::BranchList br = DoBranches(limit);
  for (size_t i = 0; i < br.size(); ++i) {
    if (br[i]->spree_ != kNothing) {
      br[i]->AddSpreePurchases(&br);
      br[i]->spree_ = kNothing;
    }
  }
  return br;
}

void State::AddSpreePurchases(BranchList* out) const {
  int hypno_harmonics = (projects_ & kHypnoHarmonics) ? 1 : 0;
  bool at_thresh = false;
  switch (spree_) {
    case kSpreeProcessor:
      // Buy a processor?
      if (trust_ > memory_ + processors_ + hypno_harmonics &&
          processors_ < max_procs) {
        out->push_back(absl::make_unique<State>(*this));
        out->back()->processors_ += 1;
        out->back()->LogProcessor();
        out->back()->spree_ = kSpreeMemory;
      }
      ABSL_FALLTHROUGH_INTENDED;
    case kSpreeMemory:
      // Buy memory (to stop collecting creat?)
      if (trust_ > memory_ + processors_ + hypno_harmonics) {
        out->push_back(absl::make_unique<State>(*this));
        out->back()->memory_ += 1;
        out->back()->LogMemory();
        out->back()->spree_ = kHypnoHarmonics;
      }
      at_thresh = true;
      ABSL_FALLTHROUGH_INTENDED;
    default:
      for (int i = 0; i < kNumOpsProjects; ++i) {
        const auto& item = kOpsProjects[i];
        // don't buy items we considered in a previous pass
        if (item.project == spree_) {
          at_thresh = true;
        }
        if (!at_thresh) {
          continue;
        }
        if (ops_ >= item.cost && MeetsPrereqs(item.project)) {
          out->push_back(absl::make_unique<State>(*this));
          out->back()->AwardProject(item.project);
          out->back()->ops_ -= item.cost;
          out->back()->spree_ = item.next_project;
        }
      }
  }
}

void State::AwardProject(uint32_t proj) {
  uint32_t project_keys[19] = {
      kImprovedAutoclippers,
      kCreativity,
      kImprovedWireExtrusion,
      kEvenBetterAutoclippers,
      kNewSlogan,
      kOptimizedWireExtrusion,
      kCatchyJingle,
      kOptimizedAutoclippers,
      kHadwigerClipDiagrams,
      kMicrolatticeShapecasting,
      kHypnoHarmonics,
      kLimerick,
      kSloganCreat,
      kJingleCreat,
      kLexicalProcessing,
      kCombinatoryHarmonics,
      kHadwigerProblem,
      kTothSausageConjecture,
      kDonkeySpace,
  };
  projects_ |= proj;
  for (int i = 0; i < 19; ++i) {
    if (proj == project_keys[i]) {
      LogPurchase(i);
      return;
    }
  }
  assert(!"WAAA");
}

void State::Log(uint8_t v) {
  if (history_idx_ < kHistorySize) {
    history_[history_idx_++] = v;
  }
}

void State::LogMlvl() { Log(std::min<int>(127, auto_clippers_)); }
void State::LogProcessor() { Log(128); }
void State::LogMemory() { Log(129); }
// purchases use log IDs 130 through 148 inclusive
void State::LogPurchase(uint8_t id) { Log(130 + id); }

std::ostream& operator<<(std::ostream& o, const State& s) {
  int minutes = floor(s.time_ / 60);
  double seconds = s.time_ - 60. * minutes;
  int hypno_harmonics = (s.projects_ & s.kHypnoHarmonics) ? 1 : 0;
  o << absl::StrFormat(
      "%02d:%08.5f tr=%02d (m/p=%02d/%02d) auto=%03d/%02d $=%08.2f "
      "ops=%05d "
      "cre=%03d cp=%06d ",
      minutes, seconds, s.trust_ - hypno_harmonics, s.memory_, s.processors_,
      s.auto_clippers_, s.mlvl_, s.dollars_, int(s.ops_), int(s.creat_),
      int(s.clips_));
  for (int mask = 0x00001; mask <= 0x200000; mask <<= 1) {
    if (s.projects_ & mask) {
      o << "\u2611";
    } else if (s.MeetsPrereqs(mask)) {
      o << "\u2610";
    } else {
      o << "\u2612";
    }
    if (mask == 0x8 || mask == 0x40 || mask == 0x200 || mask == 0x400 ||
        mask == 0x10000 || mask == 0x40000) {
      o << " ";
    }
  }
  return o << "\n";
}

std::string State::Detail() const {
  return absl::StrFormat("%10e %10e %10e %10e %10e\n", time_, ops_, creat_,
                         clips_, dollars_);
}

std::string State::History() const {
  std::vector<int> h;
  for (int i = 0; i < history_idx_; ++i) {
    h.push_back(history_[i]);
    return absl::StrJoin(h, " ");
  }
}

}  // namespace clips