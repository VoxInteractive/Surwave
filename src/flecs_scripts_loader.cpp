#include "flecs_scripts_loader.h"

#include <algorithm>
#include <filesystem>
#include <vector>

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/file_access.hpp>

using godot::UtilityFunctions;

void FlecsScriptsLoader::load(flecs::world &world) const
{
    namespace fs = std::filesystem;

    std::error_code ec;
    std::string resolved_path = root_path;

    // If a resource path was provided (res://) prefer to globalize it so the
    // filesystem iterator can operate on a real path. ProjectSettings may not
    // be available in all contexts, but in the engine it will be.
    const std::string res_prefix = "res://";
    if (root_path.rfind(res_prefix, 0) == 0)
    {
        godot::ProjectSettings *ps = godot::ProjectSettings::get_singleton();
        if (ps)
        {
            godot::String g = ps->globalize_path(godot::String(root_path.c_str()));
            resolved_path = g.utf8().get_data();
        }
        else
        {
            // Can't resolve res:// to filesystem here; report and return.
            UtilityFunctions::push_error(godot::String("Flecs scripts: ProjectSettings not available to resolve resource path: ") + godot::String(root_path.c_str()));
            return;
        }
    }

    fs::path root(resolved_path);
    if (!fs::exists(root, ec))
    {
        UtilityFunctions::push_warning(godot::String("Flecs scripts path does not exist: ") + godot::String(resolved_path.c_str()));
        return;
    }

    // Collect .flecs files (non-recursive entries are filtered by extension).
    std::vector<fs::path> files;
    for (auto it = fs::recursive_directory_iterator(root, fs::directory_options::skip_permission_denied, ec);
         it != fs::recursive_directory_iterator(); it.increment(ec))
    {
        if (ec)
        {
            UtilityFunctions::push_error(godot::String("Error iterating flecs scripts directory: ") + godot::String(ec.message().c_str()));
            break;
        }
        const fs::directory_entry &entry = *it;
        if (!entry.is_regular_file())
            continue;
        if (entry.path().extension() == ".flecs")
            files.emplace_back(entry.path());
    }

    int success_count = 0;
    for (const fs::path &p : files)
    {
        std::string path_str = p.string();
        godot::String godot_path = godot::String(path_str.c_str());
        godot::String file_contents = godot::FileAccess::get_file_as_string(godot_path);
        std::string script_str = file_contents.utf8().get_data();

        if (script_str.empty())
        {
            UtilityFunctions::push_error(godot::String("Failed to read flecs script file: ") + godot::String(path_str.c_str()));
            continue;
        }

        // Run the script from the in-memory string. Passing nullptr as name.
        int result = world.script_run(nullptr, script_str.c_str());
        if (result != 0)
        {
            UtilityFunctions::push_error(godot::String("Error running flecs script: ") + godot::String(path_str.c_str()));
        }
        else
        {
            ++success_count;
        }
    }

    UtilityFunctions::print(godot::String("Flecs scripts loaded: ") + godot::String::num_int64(success_count));
}
