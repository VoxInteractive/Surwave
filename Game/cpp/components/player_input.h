#pragma once

#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include <flecs.h>
#include "../../../src/flecs/registry.h"
#include "../../../src/flecs/singleton_setter_registry.h"

struct PlayerInput
{
    // Movement axes: X is horizontal (-1..1), Y is vertical (-1..1)
    float move_x{ 0.0f };
    float move_y{ 0.0f };
    // Look axes: yaw (x), pitch (y)
    float look_x{ 0.0f };
    float look_y{ 0.0f };

    // Zoom and triggers
    float zoom_delta{ 0.0f };
    bool zoom_toggle{ false };
    bool zoom_reset{ false };

    // Analog triggers (0..1)
    float analog_trigger_left{ 0.0f };
    float analog_trigger_right{ 0.0f };

    // Menu / Back (just-pressed only)
    bool menu_just_pressed{ false };
    bool back_just_pressed{ false };

    // Eight generic action buttons: held and just-pressed variants
    bool action_1_pressed{ false };
    bool action_1_just_pressed{ false };
    bool action_2_pressed{ false };
    bool action_2_just_pressed{ false };
    bool action_3_pressed{ false };
    bool action_3_just_pressed{ false };
    bool action_4_pressed{ false };
    bool action_4_just_pressed{ false };
    bool action_5_pressed{ false };
    bool action_5_just_pressed{ false };
    bool action_6_pressed{ false };
    bool action_6_just_pressed{ false };
    bool action_7_pressed{ false };
    bool action_7_just_pressed{ false };
    bool action_8_pressed{ false };
    bool action_8_just_pressed{ false };
};

inline FlecsRegistry register_player_input([](flecs::world& world)
{ world.component<PlayerInput>()
.member<float>("move_x")
.member<float>("move_y")
.member<float>("look_x")
.member<float>("look_y")
.member<float>("zoom_delta")
.member<bool>("zoom_toggle")
.member<bool>("zoom_reset")
.member<float>("analog_trigger_left")
.member<float>("analog_trigger_right")
.member<bool>("menu_just_pressed")
.member<bool>("back_just_pressed")
.member<bool>("action_1_pressed")
.member<bool>("action_1_just_pressed")
.member<bool>("action_2_pressed")
.member<bool>("action_2_just_pressed")
.member<bool>("action_3_pressed")
.member<bool>("action_3_just_pressed")
.member<bool>("action_4_pressed")
.member<bool>("action_4_just_pressed")
.member<bool>("action_5_pressed")
.member<bool>("action_5_just_pressed")
.member<bool>("action_6_pressed")
.member<bool>("action_6_just_pressed")
.member<bool>("action_7_pressed")
.member<bool>("action_7_just_pressed")
.member<bool>("action_8_pressed")
.member<bool>("action_8_just_pressed")
.add(flecs::Singleton); });

static int s_register_player_input_setter = ([]()
{
    register_singleton_setter("PlayerInput", [](flecs::world& world, const godot::Dictionary& data)
    {
        PlayerInput input{};

        if (data.has("move"))
        {
            godot::Variant v = data["move"];
            if (v.get_type() == godot::Variant::VECTOR2)
            {
                godot::Vector2 a = v;
                input.move_x = static_cast<float>(a.x);
                input.move_y = static_cast<float>(a.y);
            }
        }

        if (data.has("look"))
        {
            godot::Variant v = data["look"];
            if (v.get_type() == godot::Variant::VECTOR2)
            {
                godot::Vector2 l = v;
                input.look_x = static_cast<float>(l.x);
                input.look_y = static_cast<float>(l.y);
            }
        }

        if (data.has("zoom_delta"))
        {
            godot::Variant v = data["zoom_delta"];
            if (v.get_type() == godot::Variant::FLOAT || v.get_type() == godot::Variant::INT)
            {
                input.zoom_delta = static_cast<float>(v);
            }
        }

        if (data.has("zoom_toggle"))
        {
            godot::Variant v = data["zoom_toggle"];
            if (v.get_type() == godot::Variant::BOOL)
            {
                input.zoom_toggle = static_cast<bool>(v);
            }
        }

        if (data.has("zoom_reset"))
        {
            godot::Variant v = data["zoom_reset"];
            if (v.get_type() == godot::Variant::BOOL)
            {
                input.zoom_reset = static_cast<bool>(v);
            }
        }

        if (data.has("analog_trigger_left"))
        {
            godot::Variant v = data["analog_trigger_left"];
            if (v.get_type() == godot::Variant::FLOAT || v.get_type() == godot::Variant::INT)
            {
                input.analog_trigger_left = static_cast<float>(v);
            }
        }

        if (data.has("analog_trigger_right"))
        {
            godot::Variant v = data["analog_trigger_right"];
            if (v.get_type() == godot::Variant::FLOAT || v.get_type() == godot::Variant::INT)
            {
                input.analog_trigger_right = static_cast<float>(v);
            }
        }

        if (data.has("menu_just_pressed"))
        {
            godot::Variant v = data["menu_just_pressed"];
            if (v.get_type() == godot::Variant::BOOL)
            {
                input.menu_just_pressed = static_cast<bool>(v);
            }
        }

        if (data.has("back_just_pressed"))
        {
            godot::Variant v = data["back_just_pressed"];
            if (v.get_type() == godot::Variant::BOOL)
            {
                input.back_just_pressed = static_cast<bool>(v);
            }
        }

        for (int i = 1; i <= 8; ++i)
        {
            std::string si = std::to_string(i);
            godot::String base = godot::String("action_") + godot::String(si.c_str());
            godot::String pressed_key = base + godot::String("_pressed");
            godot::String just_key = base + godot::String("_just_pressed");

            if (data.has(pressed_key))
            {
                godot::Variant v = data[pressed_key];
                if (v.get_type() == godot::Variant::BOOL)
                {
                    bool val = static_cast<bool>(v);
                    switch (i)
                    {
                    case 1: input.action_1_pressed = val; break;
                    case 2: input.action_2_pressed = val; break;
                    case 3: input.action_3_pressed = val; break;
                    case 4: input.action_4_pressed = val; break;
                    case 5: input.action_5_pressed = val; break;
                    case 6: input.action_6_pressed = val; break;
                    case 7: input.action_7_pressed = val; break;
                    case 8: input.action_8_pressed = val; break;
                    }
                }
            }

            if (data.has(just_key))
            {
                godot::Variant v = data[just_key];
                if (v.get_type() == godot::Variant::BOOL)
                {
                    bool val = static_cast<bool>(v);
                    switch (i)
                    {
                    case 1: input.action_1_just_pressed = val; break;
                    case 2: input.action_2_just_pressed = val; break;
                    case 3: input.action_3_just_pressed = val; break;
                    case 4: input.action_4_just_pressed = val; break;
                    case 5: input.action_5_just_pressed = val; break;
                    case 6: input.action_6_just_pressed = val; break;
                    case 7: input.action_7_just_pressed = val; break;
                    case 8: input.action_8_just_pressed = val; break;
                    }
                }
            }
        }

        world.set<PlayerInput>(input);
    });

    return 0; })();
