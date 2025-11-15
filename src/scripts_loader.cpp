#include <algorithm>
#include <filesystem>
#include <vector>

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/file_access.hpp>

#include "src/scripts_loader.h"

using godot::UtilityFunctions;

void FlecsScriptsLoader::load(flecs::world& world) const
{
    namespace fs = std::filesystem;

    std::error_code ec;
    std::string resolved_path = root_path;

    // If a resource path was provided (res://) prefer to globalize it so the filesystem iterator can operate on a real path.
    // ProjectSettings may not be available in all contexts, but in the engine it will be.
    const std::string res_prefix = "res://";
    if (root_path.rfind(res_prefix, 0) == 0)
    {
        godot::ProjectSettings* ps = godot::ProjectSettings::get_singleton();
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
        const fs::directory_entry& entry = *it;
        if (!entry.is_regular_file())
            continue;
        if (entry.path().extension() == ".flecs")
            files.emplace_back(entry.path());
    }

    // Sort files by directory depth, then alphabetically. This ensures that scripts in parent directories
    // (which often define common components) are loaded before scripts in subdirectories.
    std::sort(files.begin(), files.end(), [](const fs::path& a, const fs::path& b) {
        size_t a_depth = std::distance(a.begin(), a.end());
        size_t b_depth = std::distance(b.begin(), b.end());
        if (a_depth != b_depth) {
            return a_depth < b_depth;
        }
        return a < b;
    });

    std::vector<std::string> loaded_scripts;
    for (const fs::path& p : files)
    {
        std::string path_str = p.string();
        godot::String godot_path(path_str.c_str());
        godot::String file_contents = godot::FileAccess::get_file_as_string(godot_path);
        std::string script_str = file_contents.utf8().get_data();
        // Normalize CRLF to LF to prevent parsing issues with flecs scripts.
        // https://discord.com/channels/633826290415435777/1437044505185615882/1437229465183715500
        script_str.erase(std::remove(script_str.begin(), script_str.end(), '\r'), script_str.end());

        if (script_str.empty())
        {
            UtilityFunctions::push_error(godot::String("Failed to read flecs script file: ") + godot::String(path_str.c_str()));
            continue;
        }

        // Run the script from the in-memory string. Pass the absolute path for better error reporting.
        int result = world.script_run(path_str.c_str(), script_str.c_str());
        if (result != 0)
        {
            UtilityFunctions::push_error(godot::String("Error running flecs script: ") + godot::String(path_str.c_str()));
        }
        else
        {
            fs::path relative_path = fs::relative(p, root);
            loaded_scripts.push_back(relative_path.string());
        }
    }

    if (!loaded_scripts.empty())
    {
        godot::String output = godot::String::num_int64(loaded_scripts.size()) + " Flecs scripts loaded: ";
        for (size_t i = 0; i < loaded_scripts.size(); ++i) {
            output += loaded_scripts[i].c_str();
            if (i < loaded_scripts.size() - 1) {
                output += ", ";
            }
            else {
                output += ".";
            }
        }
        UtilityFunctions::print(output);
    }
}
