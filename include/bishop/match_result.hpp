#ifndef BISHOP_MATCH_RESULT_HPP_
#define BISHOP_MATCH_RESULT_HPP_

#include <string>
#include <unordered_map>
#include <vector>

namespace bishop {

struct Group {
    size_t start{std::string::npos};
    size_t end{std::string::npos};
    bool matched{false};

    bool empty() const noexcept
    {
        return !matched;
    }
    size_t length() const noexcept
    {
        return matched ? end - start : 0;
    }
};

class MatchResult {
public:
    MatchResult() = default;

    explicit MatchResult(const std::string& text, size_t start, size_t end)
        : text_(&text), start_(start), end_(end), matched_(true)
    {}

    bool matched() const noexcept
    {
        return matched_;
    }
    size_t start() const noexcept
    {
        return start_;
    }
    size_t end() const noexcept
    {
        return end_;
    }
    size_t length() const noexcept
    {
        return end_ - start_;
    }
    size_t group_count() const noexcept
    {
        return groups_.size();
    }

    std::string str() const
    {
        if (!matched_ || !text_) return {};
        return text_->substr(start_, end_ - start_);
    }

    std::string group(size_t idx) const
    {
        if (idx == 0) return str();
        if (idx > groups_.size() || !groups_[idx - 1].matched) return {};
        return text_->substr(groups_[idx - 1].start, groups_[idx - 1].end - groups_[idx - 1].start);
    }

    std::string group(const std::string& name) const
    {
        auto it = named_.find(name);
        if (it == named_.end()) return {};
        return group(it->second);
    }

    bool has_group(const std::string& name) const
    {
        return named_.count(name) > 0;
    }

    void add_group(Group g)
    {
        groups_.push_back(g);
    }

    void add_named(const std::string& name, size_t idx)
    {
        named_[name] = idx;
    }

    operator bool() const noexcept
    {
        return matched_;
    }

private:
    const std::string* text_{nullptr};
    size_t start_{0};
    size_t end_{0};
    bool matched_{false};
    std::vector<Group> groups_;
    std::unordered_map<std::string, size_t> named_;
};

}  // namespace bishop
#endif
