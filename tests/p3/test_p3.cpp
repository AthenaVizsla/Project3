// tests/p3/test_p3.cpp — YOUR P3 TEST SUITE.
//
// The five tests below are worked examples. Keep them, and add your own
// until you have AT LEAST 15 (spec §6 lists what must be covered).
// Name every test <area>_<what>: reg_ for the registry, ws_ for the
// workspace, calc_ for the calculator.
//
// Build & run:   cmake --build build && ./build/test_p3
// One area:      ./build/test_p3 reg_        (names starting with "reg_")
// One test:      ./build/test_p3 =reg_add_and_find

#include "../mini_test.h"
#include "../test_helpers.h"     // TempDir, write_text, args1, ...

#include "tools/calculator.h"
#include "tools/tool_registry.h"
#include "tools/workspace.h"

#include <algorithm>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

// A tool whose name you choose — use it to fill the registry in tests.
class NamedTool : public Tool {
public:
    explicit NamedTool(std::string n) : n_(std::move(n)), schema_(JsonValue::object()) {}
    std::string name() const override { return n_; }
    std::string description() const override { return "test tool"; }
    const JsonValue& schema() const override { return schema_; }
    ToolResult invoke(const JsonValue&, ToolContext&) override { return ToolResult::success(n_); }
private:
    std::string n_;
    JsonValue   schema_;   // an empty JSON object: {}
};

// Tokenize with the provided tokenizer, then run YOUR calc_evaluate.
bool eval(const std::string& expr, double* value, std::string* error) {
    std::vector<CalcToken> tokens;
    if (!calc_tokenize(expr, &tokens, error)) return false;
    return calc_evaluate(tokens, value, error);
}

}  // namespace

// ---------------------------------------------------------------- examples

TEST(reg_add_and_find) {
    ToolRegistry r;
    auto t = std::make_unique<NamedTool>("alpha");
    Tool* raw = t.get();                        // remember the address before moving
    REQUIRE(r.add(std::move(t)));
    CHECK(r.size() == 1);
    CHECK(r.find("alpha") == raw);              // same object, not a copy
    CHECK(r.find("beta") == nullptr);
}

TEST(reg_duplicate_keeps_original) {
    ToolRegistry r;
    auto first = std::make_unique<NamedTool>("alpha");
    Tool* original = first.get();
    REQUIRE(r.add(std::move(first)));
    CHECK(!r.add(std::make_unique<NamedTool>("alpha")));
    CHECK(r.find("alpha") == original);
    CHECK(r.size() == 1);
}

TEST(ws_rejects_parent_escape) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve("notes/todo.txt", &out, &reason));   // sanity: accepts *something*
    CHECK(out == ws.root() / "notes" / "todo.txt");
    CHECK(!ws.resolve("../etc/passwd", &out, &reason));
    CHECK(!reason.empty());
}

TEST(calc_precedence) {
    double v = 0;
    std::string err;
    REQUIRE(eval("2 + 3 * 4", &v, &err));
    CHECK(v == 14);
}

TEST(calc_division_by_zero) {
    double v = 0;
    std::string err;
    CHECK(!eval("1 / 0", &v, &err));
    CHECK(err == "division by zero");
}

// ------------------------------------------------------------- your tests
//
// Still to write (spec §6 has the full list). Suggested names:
//   reg_null_rejected, reg_for_each_visits_each_once
//   reg_10000_tools               REQUIRED: print bucket_count / load_factor /
//                                 max_probe_length, check the bound
//   reg_alternative_hash          REQUIRED: longest chain of a weak hash
//   ws_<case>                     REQUIRED: every row of the table in spec §4.4
//   calc_<case>                   left-to-right, parentheses, negatives

TEST(reg_null_rejected){
    ToolRegistry r;
    CHECK(!r.add(nullptr));
    CHECK(r.size() == 0);
    CHECK(r.find("anything") == nullptr);
}

TEST(reg_empty_table){
    ToolRegistry r;
    CHECK(r.size() == 0);
    CHECK(r.bucket_count() >= 1 && r.bucket_count() <= 64);
    CHECK(r.load_factor() == 0.0);
    CHECK(r.max_probe_length() == 0);
    int visits = 0;
    r.for_each([&](const Tool&){++visits;});
    CHECK(visits == 0);
}

TEST(reg_size_tracks_adds_and_rejections) {
    ToolRegistry r;
    CHECK(r.add(std::make_unique<NamedTool>("a")));
    CHECK(r.size() == 1);
    CHECK(!r.add(std::make_unique<NamedTool>("a")));
    CHECK(r.size() == 1);
    CHECK(!r.add(nullptr));
    CHECK(r.size() == 1);
    CHECK(r.add(std::make_unique<NamedTool>("b")));
    CHECK(r.size() == 2);
}

TEST(reg_for_each_visits_each_once) {
    ToolRegistry r;
    const std::vector<std::string> names = {"read", "write", "list", "calc", "zeta"};
    for (const auto& n : names) REQUIRE(r.add(std::make_unique<NamedTool>(n)));
    std::vector<std::string> seen;
    r.for_each([&](const Tool& t) { seen.push_back(t.name()); });
    CHECK(seen.size() == names.size());
    for (const auto& n : names) {
        int count = 0;
        for (const auto& s : seen) if (s == n) ++count;
        CHECK(count == 1);
    }
}


TEST(reg_tools_destroyed_exactly_once) {
    static int alive = 0;
    struct Counted : NamedTool {
        explicit Counted(std::string n) : NamedTool(std::move(n)) { ++alive; }
        ~Counted() override { --alive; }
    };
    alive = 0;
    {
        ToolRegistry r;
        for (int i = 0; i < 100; ++i)                       // forces several resizes
            r.add(std::make_unique<Counted>("t" + std::to_string(i)));
        CHECK(alive == 100);
        CHECK(!r.add(std::make_unique<Counted>("t5")));     // rejected duplicate is destroyed now
        CHECK(alive == 100);
    }
    CHECK(alive == 0);
}

TEST(reg_pointer_stable_across_resize) {
    ToolRegistry r;
    auto t = std::make_unique<NamedTool>("first");
    Tool* raw = t.get();
    REQUIRE(r.add(std::move(t)));
    std::size_t buckets_before = r.bucket_count();
    for (int i = 0; i < 500; ++i) r.add(std::make_unique<NamedTool>("x" + std::to_string(i)));
    CHECK(r.bucket_count() > buckets_before);               // a resize really happened
    CHECK(r.find("first") == raw);
}

TEST(reg_hash_is_order_sensitive_and_full_width) {
    CHECK(ToolRegistry::hash("ab") != ToolRegistry::hash("ba"));
    CHECK(ToolRegistry::hash("tool_1234") != ToolRegistry::hash("tool_1243"));
    CHECK(ToolRegistry::hash("abc") != ToolRegistry::hash("abd"));   // last char matters
    CHECK(ToolRegistry::hash("abc") == ToolRegistry::hash("abc"));   // deterministic
    std::vector<std::size_t> hs;
    for (int i = 0; i < 10000; ++i) hs.push_back(ToolRegistry::hash("tool_" + std::to_string(i)));
    std::sort(hs.begin(), hs.end());
    hs.erase(std::unique(hs.begin(), hs.end()), hs.end());
    CHECK(hs.size() >= 9990);
}

TEST(reg_10000_tools) {
    ToolRegistry r;
    Tool* first = nullptr;
    for (int i = 0; i < 10000; ++i) {
        auto t = std::make_unique<NamedTool>("tool_" + std::to_string(i));
        Tool* raw = t.get();
        REQUIRE(r.add(std::move(t)));
        if (i == 0) first = raw;
        REQUIRE(r.load_factor() <= 1.0);
    }
    CHECK(r.size() == 10000);
    CHECK(r.find("tool_0") == first);                        // pointer survived every resize
    for (int i = 0; i < 10000; ++i)
        REQUIRE(r.find("tool_" + std::to_string(i)) != nullptr);
    CHECK(r.find("tool_10000") == nullptr);
    CHECK(r.max_probe_length() <= 64);
    std::printf("  [10000 tools] bucket_count=%zu load_factor=%.4f max_probe_length=%zu\n",
                r.bucket_count(), r.load_factor(), r.max_probe_length());
}

TEST(reg_alternative_hash) {
    // Same 10,000 names, same final bucket count, but the weak hash s[0] + s.size().
    ToolRegistry r;
    for (int i = 0; i < 10000; ++i) r.add(std::make_unique<NamedTool>("tool_" + std::to_string(i)));
    const std::size_t buckets = r.bucket_count();

    std::vector<int> weak(buckets, 0), good(buckets, 0);
    for (int i = 0; i < 10000; ++i) {
        std::string n = "tool_" + std::to_string(i);
        std::size_t w = static_cast<unsigned char>(n[0]) + n.size();
        ++weak[w & (buckets - 1)];
        ++good[ToolRegistry::hash(n) & (buckets - 1)];
    }
    int weak_max = *std::max_element(weak.begin(), weak.end());
    int good_max = *std::max_element(good.begin(), good.end());
    std::printf("  [hash comparison, %zu buckets] FNV-1a longest chain=%d (registry reports %zu), "
                "weak s[0]+size longest chain=%d\n",
                buckets, good_max, r.max_probe_length(), weak_max);
    CHECK(static_cast<std::size_t>(good_max) == r.max_probe_length());
    CHECK(weak_max > 64);
    CHECK(weak_max > 10 * good_max);
}
















int main(int argc, char** argv) { return mt::run_all(argc, argv); }
