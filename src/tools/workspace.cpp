// src/tools/workspace.cpp — YOU IMPLEMENT (spec §4.4).
//
// Workspace::resolve: a purely lexical check, steps 1–6 of the spec.
// Do not touch the filesystem here (no exists(), canonical(), ...).
// The constructor, root() and check_symlinks() are provided in
// workspace_provided.cpp.

#include "tools/workspace.h"

namespace fs = std::filesystem;

bool Workspace::resolve(std::string_view user_path, fs::path* out,
                        std::string* reason) const {
    try{
        // test if empty
        if (user_path.empty()){
            *reason = "empty path";
            return false;
        }
        // NUL
        if (user_path.find('\0') != std::string_view::npos){
            *reason = "path contains a NUL byte";
            return false;
        }
        // only workspace relative paths
        fs::path p{std::string(user_path)};
        if (p.is_absolute() || p.has_root_path()){
            *reason = "absolute paths are not allowed: " + std::string(user_path);
            return false;
        }

        //fold "." duplicate slashes and "x/.." pairs
        fs::path n = p.lexically_normal();
        if (n.empty() || *n.begin() == ".."){
            *reason = "path escapes the workspace: " + std::string(user_path);
            return false;
        }
        // "." means the root itself 
        if (n == "."){
            *out = root();
        } else {
            *out = root() / n;
        }
        return true;
    }
    catch (...){
        *reason = "invalid path";
        return false;
    }
}
