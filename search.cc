#include <memory>
#include <thread>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/memory/memory.h"
#include "absl/time/clock.h"
#include "clips.h"

using StateVec = std::vector<std::unique_ptr<clips::State>>;

constexpr double time_upper_bound = 1026.;

void ForEachVec(std::vector<StateVec *> vecs,
                std::function<void(StateVec &)> fn) {
  std::vector<std::thread> threads;
  for (StateVec *v : vecs) {
    threads.emplace_back([=]() { fn(*v); });
  }
  for (std::thread &t : threads) {
    t.join();
  }
}

void CullEntriesInBin(StateVec &vec) {
  std::sort(vec.begin(), vec.end(),
            [](const std::unique_ptr<clips::State> &s1,
               const std::unique_ptr<clips::State> &s2) {
              return s1->Time() < s2->Time();
            });
  // Sorted by time means that, generally, only later entries can be strictly
  // worse than earlier ones.  The exception is close ties on time.
  int i = 0;
  while (i < vec.size()) {
    // First look backwards for states with the same time as us but strictly
    // worse values.
    int j = i - 1;
    while (j >= 0) {
      if (vec[j]->Time() + clips::State::eps < vec[i]->Time()) {
        // Likely early exit.
        break;
      }
      if (vec[j]->IsStrictlyWorseThan(*vec[i])) {
        vec.erase(vec.begin() + j);
        i -= 1; // index of our current item shifts left
      }
      --j;
    }
    // Now look forward for states that took longer to get to strictly worse
    for (j = i + 1; j < vec.size();) {
      if (vec[j]->IsStrictlyWorseThan(*vec[i])) {
        vec.erase(vec.begin() + j);
        continue;
      }
      ++j;
    }
    ++i;
  }
}

void CullEntries(StateVec &vec) {
  absl::flat_hash_map<clips::State::BinType, StateVec> bin_map;
  for (auto &sp : vec) {
    auto &bin = bin_map[sp->Bin()];
    bin.push_back(std::move(sp));
  }
  vec.clear();
  for (auto &node : bin_map) {
    CullEntriesInBin(node.second);
    for (auto &entry : node.second) {
      vec.push_back(std::move(entry));
    }
  }
}

void CullEntriesSharded(StateVec &vec) {
  absl::flat_hash_map<clips::State::BinType, StateVec> bin_map;
  for (auto &sp : vec) {
    auto &bin = bin_map[sp->Bin()];
    bin.push_back(std::move(sp));
  }
  vec.clear();
  std::vector<StateVec *> parts;
  for (auto &node : bin_map) {
    parts.push_back(&node.second);
  }
  ForEachVec(parts, CullEntriesInBin);
  for (auto &node : bin_map) {
    for (auto &entry : node.second) {
      vec.push_back(std::move(entry));
    }
  }
}

void Advance(StateVec &prev, clips::State::LimitType goal_type,
             double goal_value, double opt_time) {
  StateVec next;

  while (!prev.empty()) {
    std::unique_ptr<clips::State> cur = std::move(prev.back());
    prev.pop_back();
    for (auto &item : cur->Branches(goal_type, goal_value)) {
      if (item->AtGoal(goal_type, goal_value) || item->Win()) {
        next.push_back(std::move(item));
      } else if (item->Time() < opt_time) {
        prev.push_back(std::move(item));
      }
    }
  }
  prev = std::move(next);
}

void AdvanceSharded(StateVec &s, clips::State::LimitType goal_type,
                    double goal_value, double opt_time) {
  if (s.size() < 240) {
    Advance(s, goal_type, goal_value, opt_time);
  } else {
    StateVec vec_shards[24];
    int i = 0;
    for (auto &entry : s) {
      vec_shards[(i++) % 24].push_back(std::move(entry));
    }
    std::vector<StateVec *> addrs;
    for (int i = 0; i < 24; ++i) {
      addrs.push_back(&vec_shards[i]);
    }
    ForEachVec(addrs, [goal_type, goal_value, opt_time](StateVec &v) {
      Advance(v, goal_type, goal_value, opt_time);
    });
    s.clear();
    for (StateVec &result_vec : vec_shards) {
      for (auto &item : result_vec) {
        s.push_back(std::move(item));
      }
    }
  }
}

int main() {
  StateVec pool;
  pool.push_back(absl::make_unique<clips::State>());
  int stride = 25;
  for (int i = stride; i < 1100; i += stride) {
    AdvanceSharded(pool, clips::State::kTimeLimit, i, time_upper_bound);
    std::cout << absl::FormatTime("%H:%M:%E2S", absl::Now(),
                                  absl::UTCTimeZone())
              << " " << i << " " << pool.size() << "";
    if (i % 100 == 0) {
      CullEntriesSharded(pool);
    }
    std::cout << " " << pool.size() << "\n";
  }
  AdvanceSharded(pool, clips::State::kTimeLimit, 15000., time_upper_bound);
  std::cout << absl::FormatTime("%H:%M:%E2S", absl::Now(), absl::UTCTimeZone())
            << " " << 15000 << " " << pool.size() << "";
  CullEntriesSharded(pool);
  std::cout << " " << pool.size() << "\n";
}