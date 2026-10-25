// src/tools/tool_registry.cpp — YOU IMPLEMENT (spec §4.3).
//
// A hand-written hash table. No std::unordered_map / std::map / std::set.
// Every function below is a stub so the starter compiles. Replace each body.

#include "tools/tool_registry.h"

#include <cstdint>
#include <utility>

ToolRegistry::ToolRegistry() : buckets_(kInitialBuckets), size_(0){}
ToolRegistry::~ToolRegistry() = default;   // unique_ptrs in your buckets free the tools

std::size_t ToolRegistry::hash(std::string_view key) noexcept {
    std::uint64_t h = 1469598103934665603ULL;
    for (unsigned char c : key){
        h ^= c;
        h *= 1099511628211ULL;
    }
    return static_cast<std::size_t>(h);   
}

std::size_t ToolRegistry::index_for(std::string_view key, std::size_t buckets) const noexcept{
    return hash(key) & (buckets - 1);
}

bool ToolRegistry::add(std::unique_ptr<Tool> tool) {
    if (tool == nullptr) return false;

    const std::string name = tool->name();
    if (find(name) != nullptr) return false;

    if (static_cast<double>(size_ + 1) > kMaxLoad * static_cast<double>(buckets_.size())){
        rehash(buckets_.size() * 2);
    }

    buckets_[index_for(name, buckets_.size())].push_back(std::move(tool));
    ++size_;
    return true;   
}

Tool* ToolRegistry::find(std::string_view /*name*/) const {
    return nullptr;   // TODO
}

std::size_t ToolRegistry::size() const noexcept {
    return size_;   // TODO
}

void ToolRegistry::for_each(const std::function<void(const Tool&)>& /*fn*/) const {
    // TODO
}

std::size_t ToolRegistry::bucket_count() const noexcept {
    return 0;   // TODO
}

double ToolRegistry::load_factor() const noexcept {
    return 0.0;   // TODO
}

std::size_t ToolRegistry::max_probe_length() const {
    return 0;   // TODO
}
