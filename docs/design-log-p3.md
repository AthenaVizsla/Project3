# Project 3 Design Log
**Author:** Athena Larson· **P2 code used:** my own
**Tools used:** Claude- used to explain concepts and to format design log in markdown

## 1. Hash table
I chose **separate chaining**: each bucket is a `std::vector<std::unique_ptr<Tool>>`. P3 never removes tools, so open addressing's main weakness (tombstones) wouldn't have hurt, but chaining degrades gracefully, has no "full table" hazard, and lets the load factor sit at 0.75 to 1.0 without a clustering cliff. The hash is **FNV-1a 64-bit**, returned at full width and reduced separately with `hash & (buckets - 1)` (bucket count is always a power of two, starting at 16). Before each insert I check whether `size + 1 > 0.75 * buckets`; if so I double the buckets and re-insert every entry by recomputing its index. Checking *before* inserting keeps `load_factor()` at or below the threshold whenever `add()` returns.

Measurements for `tool_0` … `tool_9999`:

| | FNV-1a (mine) | weak `s[0] + s.size()` |
|---|---|---|
| bucket_count | 16384 | 16384 (same table size) |
| load_factor | 0.6104 | n/a |
| longest chain | **6** | **9000** |

The weak hash sends almost every name to the same few buckets (all names of the same length starting with `t` collide), so lookups degrade to a linear scan. FNV-1a mixes every character, so chains stay near the average load.

`add()` is amortized O(1): a resize from *n* to *2n* buckets costs O(n), but only happens after about *n/2* cheap inserts since the previous resize, so each insert pays O(1) extra on average.

## 2. Pointer stability
The buckets hold `unique_ptr<Tool>`, so the `Tool` objects live on the heap and never move. A rehash moves only the `unique_ptr`s into the new bucket array; the address each pointer holds is unchanged. So a `Tool*` from `find()` stays valid for the registry's lifetime. The test `reg_10000_tools` checks `find("tool_0")` after 10,000 inserts and many resizes.

## 3. Calculator trace
`2 * (3 + 4) - 5`: push 2 on `values`. `*`: `ops` is empty, push. `(`: push. Push 3. `+`: top of `ops` is `(`, which is not an operator, so push `+`. Push 4. `)`: apply `+` (pop 4 as right, 3 as left, push 7), then pop the `(`. Now `values` is `2 7`, `ops` is `*`. `-`: top `*` has precedence 2 >= 1, so apply it (2*7 = 14); `ops` is empty, push `-`. Push 5. End of input: apply `-` (14 - 5 = 9). One value is left: **9**. The `>=` rule is what makes `10-4-3` come out left to right, and a loop (not recursion) lets 5,000 nested parentheses work.

## 4. Errors as values
A tool failing is normal in an agent loop: the file is missing, a path is forbidden, a division hits zero. The model is expected to read the message and try something else, so the failure is part of the program's ordinary data flow and belongs in a return value (`ToolResult::failure`, or `false` plus a message), where the caller must handle it visibly. Exceptions are for programmer errors, such as calling `as_string()` on a JSON number: nothing at runtime should "recover" from that, and it should be loud. Using exceptions for expected failures would also make every tool call a potential non-local exit through the harness.

## 5. What I would design differently
`find()` recomputes `name()` (a string copy) for every entry it compares, and `add()` calls it again on rehash. I would cache the key and hash inside each bucket entry so lookups compare a stored hash first and rehashing never calls virtual functions.