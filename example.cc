#include <iostream>
#include <memory>
#include <vector>

#include "absl/strings/str_format.h"
#include "clips.h"

int ToLimit(clips::State::LimitType limit_type, double limit_value) {
  std::vector<std::unique_ptr<clips::State>> pool;
  std::vector<std::unique_ptr<clips::State>> half_result;
  pool.push_back(absl::make_unique<clips::State>());
  while (!pool.empty()) {
    std::unique_ptr<clips::State> item = std::move(pool.back());
    pool.pop_back();
    for (auto &next : item->Branches(limit_type, limit_value)) {
      // std::cout << pool.size() << " <- " << *item;
      if (next->AtGoal(limit_type, limit_value)) {
        // std::cout << "--> " << *next;
        half_result.push_back(std::move(next));
      } else {
        pool.push_back(std::move(next));
      }
    }
  }
  // std::cout << " [" << half_result.size() << "] ";
  std::sort(half_result.begin(), half_result.end(),
            [](const std::unique_ptr<clips::State> &lhs,
               const std::unique_ptr<clips::State> &rhs) {
              return lhs->Time() < rhs->Time();
            });
  std::vector<std::unique_ptr<clips::State>> result;
  std::vector<std::unique_ptr<clips::State>> booted;
  int i1 = 0;
  std::cout << half_result.size() << "\n";
  for (auto &item : half_result) {
  try_again:
    int i2 = 0;
    for (auto it = result.begin(); it < result.end(); ++it) {
      if (item->IsStrictlyWorseThan(**it)) {
        // std::cout << absl::StrFormat("[%4d] ", i2++) << " " << *item << "\n";
        goto nope;
      }
      if ((*it)->IsStrictlyWorseThan(*item)) {
        booted.push_back(std::move(*it));
        result.erase(it);
        goto try_again;
      }
      i2++;
    }
    result.push_back(std::move(item));
    // std::cout << absl::StrFormat("%6d ", i1++) << " " << *result.back() <<
    // "\n";
  nope:
    0;
  }

  /*
  int i = 0;
  for (const auto& node : result) {
    std::cout << absl::StrFormat("%3d: ", i++) << *node << "\n";
  }
  std::sort(booted.begin(), booted.end(),
            [](const std::unique_ptr<clips::State>& lhs,
               const std::unique_ptr<clips::State>& rhs) {
              return lhs->Time() < rhs->Time();
            });
  for (const auto& node : booted) {
    std::cout << "OUT: " << *node << "\n";
  }
  */
  return result.size();
}

int main() {
  /*
    for (int i : {8500}) {
      std::cout << i << ":" << ToLimit(i) << "\n";
    }
    return 0;*/
  auto state = absl::make_unique<clips::State>();
  std::cout << "x: " << state->IsStrictlyWorseThan(*state) << "\n";
  while (true) {
    auto next = state->Branches(clips::State::kTimeLimit, 1e99);
    for (int i = 0; i < next.size(); ++i) {
      std::cout << i << ") "
                << *next[i] << next[i]->Detail();
    }
    int ni;
    std::cin >> ni;
    if (ni < 0) {
      break;
    }
    state = std::move(next[ni]);
  }
}
