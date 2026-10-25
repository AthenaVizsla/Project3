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

void ToolRegistry::rehash(std::size_t new_bucket_count){
    std::vector<Bucket> fresh(new_bucket_count);
    for (Bucket& b : buckets_){
        for (std::unique_ptr<Tool>& t : b){
            std::size_t  idx = index_for(t->name(), new_bucket_count);
            fresh[idx].push_back(std::move(t));
        }
    }
    buckets_ = std::move(fresh);
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

Tool* ToolRegistry::find(std::string_view name) const {
    const Bucket& b = buckets_[index_for(name, buckets_.size())];
    for (const std::unique_ptr<Tool>& t : b){
        if (t->name() == name) return t.get();
    }
    return nullptr;   
}

std::size_t ToolRegistry::size() const noexcept {
    return size_;   
}

void ToolRegistry::for_each(const std::function<void(const Tool&)>& fn) const {
    for (const Bucket& b : buckets_){
        for (const std::unique_ptr<Tool>& t : b){
            fn(*t);
        }
    }
}

std::size_t ToolRegistry::bucket_count() const noexcept {
    return buckets_.size();   
}

double ToolRegistry::load_factor() const noexcept {
    return static_cast<double>(size_) / static_cast<double>(buckets_.size());
}

std::size_t ToolRegistry::max_probe_length() const {
    std::size_t longest = 0;
    for (const Bucket& b : buckets_){
        if (b.size() > longest) longest = b.size();
    }
    return longest; 
}
