#pragma once
#include <optional>
#include <string>
#include <unordered_set>

// Only usable when the operator has opted into exclusive flight authority.
// The engine exposes a shared bool, not a multi-provider capability lease.
class FlightOwnership {
public:
    std::optional<bool> reconcile(const std::string& player, bool eligible,
                                  bool allowed, bool survival) {
        if (!survival) { grants_.erase(player); return std::nullopt; }
        if (eligible && !allowed) { grants_.insert(player); return true; }
        if (!eligible && grants_.erase(player)) return false;
        return std::nullopt;
    }
    bool remove(const std::string& player) { return grants_.erase(player) != 0; }
    const auto& grants() const { return grants_; }
    void clear() { grants_.clear(); }
private:
    std::unordered_set<std::string> grants_;
};
