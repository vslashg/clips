#ifndef CLIPS_CLIPS_H_
#define CLIPS_CLIPS_H_

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>

#include "absl/container/inlined_vector.h"

namespace clips {

constexpr double CalculateOnePointOneToNth(int n) {
  return (n <= 0) ? 1.0 : 1.1 * CalculateOnePointOneToNth(n - 1);
}

constexpr double one_point_one_to_nth[] = {
    CalculateOnePointOneToNth(0),  CalculateOnePointOneToNth(1),
    CalculateOnePointOneToNth(2),  CalculateOnePointOneToNth(3),
    CalculateOnePointOneToNth(4),  CalculateOnePointOneToNth(5),
    CalculateOnePointOneToNth(6),  CalculateOnePointOneToNth(7),
    CalculateOnePointOneToNth(8),  CalculateOnePointOneToNth(9),
    CalculateOnePointOneToNth(10), CalculateOnePointOneToNth(11),
    CalculateOnePointOneToNth(12), CalculateOnePointOneToNth(13),
    CalculateOnePointOneToNth(14), CalculateOnePointOneToNth(15),
    CalculateOnePointOneToNth(16), CalculateOnePointOneToNth(17),
    CalculateOnePointOneToNth(18), CalculateOnePointOneToNth(19),
    CalculateOnePointOneToNth(20), CalculateOnePointOneToNth(21),
    CalculateOnePointOneToNth(22), CalculateOnePointOneToNth(23),
    CalculateOnePointOneToNth(24), CalculateOnePointOneToNth(25),
    CalculateOnePointOneToNth(26), CalculateOnePointOneToNth(27),
    CalculateOnePointOneToNth(28), CalculateOnePointOneToNth(29),
    CalculateOnePointOneToNth(30), CalculateOnePointOneToNth(31),
    CalculateOnePointOneToNth(32), CalculateOnePointOneToNth(33),
    CalculateOnePointOneToNth(34), CalculateOnePointOneToNth(35),
    CalculateOnePointOneToNth(36), CalculateOnePointOneToNth(37),
    CalculateOnePointOneToNth(38), CalculateOnePointOneToNth(39),
};

constexpr double OnePointOneToNth(int n) { return one_point_one_to_nth[n]; }

class State {
public:
  enum {
    kNothing = 0,
    // sell boost projects
    kImprovedAutoclippers = 0x00001,   //  750 op
    kEvenBetterAutoclippers = 0x00002, // 2500 op
    kOptimizedAutoclippers = 0x00004,  // 5000 op
    kHadwigerClipDiagrams = 0x00008,   // 6000 op
    // wire supply boost projects
    kImprovedWireExtrusion = 0x00010,    // 1750 op
    kOptimizedWireExtrusion = 0x00020,   // 3500 op
    kMicrolatticeShapecasting = 0x00040, // 7500 op
    // marketing boosts
    kNewSlogan = 0x00080,      // 2500 ops + 25 banked
    kCatchyJingle = 0x00100,   // 4500 ops + 45 banked
    kHypnoHarmonics = 0x00200, // 7500 ops + 1 trust
    // creativity
    kCreativity = 0x00400,
    // trust
    kLimerick = 0x00800,
    kLexicalProcessing = 0x01000,
    kCombinatoryHarmonics = 0x02000,
    kHadwigerProblem = 0x04000,
    kTothSausageConjecture = 0x08000,
    kDonkeySpace = 0x10000,
    // meta
    kSloganCreat = 0x20000,
    kJingleCreat = 0x40000,
    kSpreeMemory = 0x80000,
    kSpreeProcessor = 0x100000,
    kWin = 0x200000,
  };

  State() = default;
  State(const State &) = default;
  State(State &&) = default;
  State &operator=(const State &) = default;
  State &operator=(State &&) = default;

  using BranchList = absl::InlinedVector<std::unique_ptr<State>, 4>;

  // Return a copy of this state, after the given amount of time passes.
  std::unique_ptr<State> PassTime(double seconds) const;

  enum LimitType {
    kClipsLimit,
    kTimeLimit,
  };

  // Return a sequence of possible branch states from here.
  BranchList Branches(LimitType limit_type = kTimeLimit,
                      double limit_value = HUGE_VAL) const;

  bool AtGoal(LimitType limit_type, double limit_value) const {
    switch (limit_type) {
    case kClipsLimit:
      return clips_ >= limit_value;
    case kTimeLimit:
      return time_ >= limit_value;
    }
    assert(!"huh?");
    return false;
  }

  bool IsStrictlyWorseThan(const State &other) const;

  friend std::ostream &operator<<(std::ostream &o, const State &s);

  double Time() const { return time_; }
  double Clips() const { return clips_; }
  bool Win() const { return projects_ & kWin; }

  // A state-based key.  Two states in different bins can't be strictly worse
  // than each other.
  using BinType = std::tuple<int, int, int, int>;
  BinType Bin() const {
    return std::make_tuple(processors_, memory_, auto_clippers_, mlvl_);
  }
  static constexpr double eps = 1e-9;

private:
  // clips multiplier based on active projects
  double ClipBoost() const {
    static constexpr double s[16] = {1.0,  1.25, 1.5,  1.75, 1.75, 2.0,
                                     2.25, 2.5,  6.0,  6.25, 6.5,  6.75,
                                     6.75, 7.0,  7.25, 7.5};
    return s[projects_ & 0xf];
  }

  // Wire supply per purchase, based on active projects
  double WireSupply() const {
    static constexpr double s[8] = {1000.0, 1500.0, 1750.0, 2625.0,
                                    2000.0, 3000.0, 3500.0, 5250.0};
    return s[(projects_ >> 4) & 0x7];
  }

  // Boost to sales prices, based on active projects and marketing levels.
  double MarketBoost() const {
    static constexpr double s[8] = {1.0, 1.5, 2.0, 3.0, 5.0, 7.5, 10.0, 15.0};
    double boost = s[(projects_ >> 7) & 0x7];
    return boost * OnePointOneToNth(mlvl_ - 1);
  }

public:
  // Clips generated per second
  double ClipsPerSecond() const {
    const double repeat_rate = 25.0000007;
    return repeat_rate + ClipBoost() * auto_clippers_;
  }

private:
  // Dollars earned from sales per second
  double EarningsPerSecond() const {
    double cps = ClipsPerSecond();
    return std::min(0.2322342578195798 * std::pow(cps, 0.5348837209302326),
                    4.344680531523482 * std::pow(cps, 0.13043478260869557)) *
           MarketBoost();
  }

public:
  // Net dollars earned per scond
  double DollarsPerSecond() const {
    double base_cost = 20.0;
    double cps = ClipsPerSecond();
    double wire_expense_per_second = base_cost * cps / WireSupply();
    return EarningsPerSecond() - wire_expense_per_second;
  }

  double DollarsSpent() const {
    double dollars_spent_on_clips = 0.;
    if (auto_clippers_ > 0) {
      dollars_spent_on_clips = auto_clippers_ * 5. - 1. +
                               (1 - std::pow(1.1, auto_clippers_)) / (-.1);
    }
    double dollars_spent_on_marketing = 100. * std::pow(2., mlvl_ - 1) - 100.;
    return dollars_spent_on_clips + dollars_spent_on_marketing;
  }

  // Ops per second
  double OpsPerSecond() const {
    if (clips_ < 2000. || ops_ >= memory_ * 1000.) {
      return 0.;
    }
    return processors_ * 10.;
  }

  // Creat per second
  double CreatPerSecond() const {
    if (ops_ < memory_ * 1000. || !(projects_ & kCreativity)) {
      return 0.;
    }
    static const double seconds_per_creat[] = {4., 4., 2.44, 1.12,
                                               .7, .5, .38,  .31};
    // small fudge factor, to avoid ties
    return 1. / seconds_per_creat[processors_] + 3e-8;
  }

  std::string History() const;
  std::string Detail() const;

private:
  // Returns true if you meet the criteria to purchase the given project.
  // Does not check if costs can be paid.  Returns false if the project is
  // already purchased.
  bool MeetsPrereqs(uint32_t project) const;

  // Returns the next ops level where a purchase is possible.
  double NextOpsLimit() const;

  // Add all purchases possible for this threshold
  void AddOpsPurchases(BranchList *br, double ops_thresh,
                       double ops_thresh_time) const;
  void AddCreatPurchase(BranchList *br, double creat_thresh,
                        double creat_thresh_time) const;

  // Returns the next limit for buying a creativity project.  The attached
  // boolean is true if this is a "must buy" price, the highest remaining
  // priced item.
  std::pair<double, bool> NextCreatLimit() const;

  void AwardProject(uint32_t project);

  BranchList DoBranches(LimitType limit_type = kTimeLimit,
                        double limit_value = HUGE_VAL) const;

  // Make all spree purchases possible from this state, and append them to
  // the branch list.
  //
  // It is assumed that *this is a member of the BranchList.  This is only
  // safe because the list is a container of unique_ptrs, so *this won't
  // be moved if the container grows.
  void AddSpreePurchases(BranchList *out) const;

  // Logging functions
  void Log(uint8_t v);
  void LogMlvl();
  void LogProcessor();
  void LogMemory();
  void LogPurchase(uint8_t id);

  double time_ = 0.;
  double ops_ = 0.;
  double creat_ = 0.;
  double clips_ = 0.;
  double dollars_ = 0.;

  int trust_ = 2;
  int processors_ = 1;
  int memory_ = 1;
  int auto_clippers_ = 0;
  int mlvl_ = 1;

  uint32_t projects_ = 0;
  uint32_t spree_ = kNothing;
  static constexpr uint8_t kHistorySize = 47;
  uint8_t history_idx_ = 0;
  uint8_t history_[kHistorySize] = {0};
};

} // namespace clips

#endif // CLIPS_CLIPS_H_