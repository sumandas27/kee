#include "kee/scene.hpp"

namespace kee {

hit_object::hit_object(float beat) :
    beat(beat),
    duration(0.0f)
{ }

hit_object::hit_object(float beat, float duration) :
    beat(beat),
    duration(duration),
    hold_is_held(false),
    hold_press_complete(false)
{ 
    if (duration == 0.0f)
        throw std::invalid_argument("A hold hit object must have non-zero hold duration!");

    if (std::floor(beat + 1.0f) < beat + duration)
        hold_next_combo = std::floor(beat + 1.0f);
}

key::key(float prop_pos_x, float prop_pos_y) :
    proportional_pos(prop_pos_x, prop_pos_y),
    is_pressed(false),
    combo_lost_time(0.0f)
{ }

const std::deque<kee::hit_object>& key::get_hit_objects() const
{
    return hit_objects;
}

void key::push(const kee::hit_object& object)
{
    if (!hit_objects.empty() && object.beat <= hit_objects.back().beat + hit_objects.back().duration)
        throw std::invalid_argument("A new hit object must be strictly after all other ones in its key!");

    hit_objects.push_back(object);
}

void key::pop()
{
    hit_objects.pop_front();
}

kee::hit_object& key::front()
{
    return hit_objects.front();
}

scene::scene(const raylib::Vector2& window_dim) :
    window_dim(window_dim),
    rect_key_grid_dim(10, 4),
    percent_key_space_empty(0.05f),
    load_time(2.0f),
    max_combo_time(0.25f),
    max_combo_lost_time(1.0f),
    approach_beats(2.0f),
    input_tolerance(0.25f),
    music("assets/daft-punk-something-about-us/daft-punk-something-about-us.mp3"),
    hitsound("assets/sfx/hitsound.wav"),
    combo_lost_sfx("assets/sfx/combo_lost.wav"),
    music_start_offset(0.5f),
    music_bpm(100.0f),
    end_beat(0.0f),
    combo_time(0.0f),
    game_time(0.0f),
    combo(0)
{
    music.SetLooping(false);
    music.SetVolume(0.1f);
    hitsound.SetVolume(0.01f);
    combo_lost_sfx.SetVolume(0.05f);

    font.baseSize = 72;
    font.glyphCount = 95;
    font.glyphPadding = 0;
    font.recs = nullptr;
    
    raylib::FileData font_file("assets/fonts/Montserrat-Light.ttf");
    font.glyphs = LoadFontData(font_file.GetData(), font_file.GetBytesRead(), font.baseSize, nullptr, font.glyphCount, FontType::FONT_SDF);

    raylib::Image font_atlas = GenImageFontAtlas(font.glyphs, &font.recs, font.glyphCount, font.baseSize, font.glyphPadding, 1);
    font.SetTexture(font_atlas.LoadTexture());
    font.GetTexture().SetFilter(TextureFilter::TEXTURE_FILTER_BILINEAR);

    const float size_diff = window_dim.x * 0.2f;
    const raylib::Vector2 rect_keys_raw_size(window_dim.x - size_diff, window_dim.y - size_diff);

    if (rect_key_grid_dim.x * rect_keys_raw_size.y >= rect_key_grid_dim.y * rect_keys_raw_size.x)
    {
        rect_keys.width = rect_keys_raw_size.x;
        rect_keys.height = rect_keys.width * rect_key_grid_dim.y / rect_key_grid_dim.x;
    }
    else
    {
        rect_keys.height = rect_keys_raw_size.y;
        rect_keys.width = rect_keys.height * rect_key_grid_dim.x / rect_key_grid_dim.y;
    }

    rect_keys.x = (window_dim.x - rect_keys.width) / 2;
    rect_keys.y = (window_dim.y - rect_keys.height) / 2;

    keys_font_size = static_cast<int>((rect_keys.width / rect_key_grid_dim.x) * (1.0f - percent_key_space_empty) * 0.5f);

    keys.emplace(KeyboardKey::KEY_Q, kee::key(0.05f, 0.125f));
    keys.emplace(KeyboardKey::KEY_W, kee::key(0.15f, 0.125f));
    keys.emplace(KeyboardKey::KEY_E, kee::key(0.25f, 0.125f));
    keys.emplace(KeyboardKey::KEY_R, kee::key(0.35f, 0.125f));
    keys.emplace(KeyboardKey::KEY_T, kee::key(0.45f, 0.125f));
    keys.emplace(KeyboardKey::KEY_Y, kee::key(0.55f, 0.125f));
    keys.emplace(KeyboardKey::KEY_U, kee::key(0.65f, 0.125f));
    keys.emplace(KeyboardKey::KEY_I, kee::key(0.75f, 0.125f));
    keys.emplace(KeyboardKey::KEY_O, kee::key(0.85f, 0.125f));
    keys.emplace(KeyboardKey::KEY_P, kee::key(0.95f, 0.125f));
    keys.emplace(KeyboardKey::KEY_A, kee::key(0.1f, 0.375f));
    keys.emplace(KeyboardKey::KEY_S, kee::key(0.2f, 0.375f));
    keys.emplace(KeyboardKey::KEY_D, kee::key(0.3f, 0.375f));
    keys.emplace(KeyboardKey::KEY_F, kee::key(0.4f, 0.375f));
    keys.emplace(KeyboardKey::KEY_G, kee::key(0.5f, 0.375f));
    keys.emplace(KeyboardKey::KEY_H, kee::key(0.6f, 0.375f));
    keys.emplace(KeyboardKey::KEY_J, kee::key(0.7f, 0.375f));
    keys.emplace(KeyboardKey::KEY_K, kee::key(0.8f, 0.375f));
    keys.emplace(KeyboardKey::KEY_L, kee::key(0.9f, 0.375f));
    keys.emplace(KeyboardKey::KEY_Z, kee::key(0.15f, 0.625f));
    keys.emplace(KeyboardKey::KEY_X, kee::key(0.25f, 0.625f));
    keys.emplace(KeyboardKey::KEY_C, kee::key(0.35f, 0.625f));
    keys.emplace(KeyboardKey::KEY_V, kee::key(0.45f, 0.625f));
    keys.emplace(KeyboardKey::KEY_B, kee::key(0.55f, 0.625f));
    keys.emplace(KeyboardKey::KEY_N, kee::key(0.65f, 0.625f));
    keys.emplace(KeyboardKey::KEY_M, kee::key(0.75f, 0.625f));
    keys.emplace(KeyboardKey::KEY_COMMA, kee::key(0.85f, 0.625f));
    keys.emplace(KeyboardKey::KEY_SPACE, kee::key(0.5f, 0.875f));

    /* TODO: replace with some file parser eventually */

    keys.at(KeyboardKey::KEY_Q).push(kee::hit_object(0.0f, 16.0f));
    //keys.at(KeyboardKey::KEY_W).push(kee::hit_object(4.0f));
    //keys.at(KeyboardKey::KEY_P).push(kee::hit_object(8.0f));
    //keys.at(KeyboardKey::KEY_O).push(kee::hit_object(12.0f));

    for (const auto& [val, key] : keys)
    {
        if (key.get_hit_objects().empty())
            continue;

        const kee::hit_object& last_hit_object = key.get_hit_objects().back();
        if (end_beat < last_hit_object.beat + last_hit_object.duration)
            end_beat = last_hit_object.beat + last_hit_object.duration;
    }
}

/* TODO: replace `std::println`s with combo updates */

void scene::update(float dt)
{
    for (int key = raylib::Keyboard::GetKeyPressed(); key != 0; key = raylib::Keyboard::GetKeyPressed())
    {
        if (!keys.contains(key) || keys.at(key).get_hit_objects().empty())
            continue;

        kee::hit_object& front = keys.at(key).front();
        const bool is_hold_active = (front.duration != 0.0f && front.hold_is_held);
        if (get_beat() < front.beat - input_tolerance || is_hold_active)
            continue;

        bool gain_tap_combo = false;
        if (std::abs(front.beat - get_beat()) <= input_tolerance)
        {
            gain_tap_combo = true;
            combo++;
            combo_time = max_combo_time;

            hitsound.Play();
            if (front.duration == 0.0f)
                keys.at(key).pop();
        }
    
        if (front.duration != 0.0f)
        {
            front.hold_is_held = true;
            front.hold_press_complete = true;

            if (!gain_tap_combo)
                front.hold_next_combo = (std::floor(get_beat() + 1.0f) < front.beat + front.duration)
                    ? std::make_optional(std::floor(get_beat() + 1.0f))
                    : std::nullopt;
        }
    }

    for (auto& [val, key] : keys)
    {
        key.is_pressed = raylib::Keyboard::IsKeyDown(val);
        if (key.combo_lost_time > 0.0f)
            key.combo_lost_time -= dt;

        if (key.get_hit_objects().empty())
            continue;
    
        if (key.front().duration == 0.0f || (!key.front().hold_press_complete && !key.front().hold_is_held))
        {
            if (get_beat() - key.front().beat > input_tolerance)
            {
                lose_combo(key);
                if (key.front().duration == 0.0f)
                    key.pop();
                else
                    key.front().hold_press_complete = true;
            }
        }
        else if (key.front().hold_is_held && !raylib::Keyboard::IsKeyDown(val))
        {
            if (get_beat() < key.front().beat + key.front().duration - input_tolerance)
            {
                lose_combo(key);
                key.front().hold_is_held = false;   
            }
            else
            {
                combo++;
                combo_time = max_combo_time;

                hitsound.Play();
                key.pop();
            }
        }
        else if (key.front().hold_is_held && key.front().hold_next_combo.has_value() && get_beat() >= key.front().hold_next_combo.value())
        {
            combo++;
            combo_time = max_combo_time;

            key.front().hold_next_combo = (key.front().hold_next_combo.value() + 1.0f < key.front().beat + key.front().duration)
                ? std::make_optional(key.front().hold_next_combo.value() + 1.0f)
                : std::nullopt;
        }
        else if (get_beat() - (key.front().beat + key.front().duration) > input_tolerance)
        {
            if (key.front().hold_is_held)
                lose_combo(key);
            
            key.pop();
        }
    }

    if (combo_time > 0.0f)
        combo_time -= dt;

    game_time += dt;
    if (!music.IsPlaying())
    {
        if (game_time > load_time)
            music.Play();
    }
    else
        music.Update();
}

void scene::render() const
{
    if (game_time < load_time)
    {
        const float load_rect_h = window_dim.y * (1.0f - game_time / load_time);
        const raylib::Rectangle load_rect(0, window_dim.y - load_rect_h, window_dim.x, load_rect_h);
        load_rect.Draw(raylib::Color(255, 255, 255, 10));
    }

    if (end_beat > 0.0f)
    {
        const float progress = std::min(get_beat() / end_beat, 1.0f);
        const raylib::Rectangle progress_rect(0, 0, window_dim.x * progress, 10);
        progress_rect.Draw(raylib::Color(255, 255, 255, 255));
    }

    for (const auto& [val, key] : keys)
    {
        static const raylib::Color key_color_regular = raylib::Color::White();
        static const raylib::Color key_color_down = raylib::Color::Green();
        const raylib::Color key_color = key.is_pressed ? key_color_down : key_color_regular;

        const float key_grid_size = rect_keys.width / rect_key_grid_dim.x;
        const float space_empty = percent_key_space_empty * key_grid_size;

        const float w_multiplier = val == KeyboardKey::KEY_SPACE ? 7.0f : 1.0f;
        const float key_w = key_grid_size * w_multiplier - space_empty;
        const float key_h = key_grid_size - space_empty;
        const float key_x = rect_keys.x + rect_keys.width * key.proportional_pos.x - key_w / 2;
        const float key_y = rect_keys.y + rect_keys.height * key.proportional_pos.y - key_h / 2;
        const raylib::Rectangle key_rect(key_x, key_y, key_w, key_h);

        static constexpr float percent_key_border = 0.02f;
        const float key_thickness = key_h * percent_key_border;

        for (const kee::hit_object& object : key.get_hit_objects())
        {
            if (object.beat + object.duration < get_beat())
                continue;

            if (object.beat > get_beat() + approach_beats)
                break;

            const float start_progress = std::min(1.0f - ((object.beat - this->get_beat()) / approach_beats), 1.0f);
            const float hit_obj_h = key_h * start_progress; /* TODO: test for space */
            const float hit_obj_x = rect_keys.x + rect_keys.width * key.proportional_pos.x - hit_obj_h / 2;
            const float hit_obj_y = rect_keys.y + rect_keys.height * key.proportional_pos.y - hit_obj_h / 2;
            const raylib::Rectangle hit_obj_rect(hit_obj_x, hit_obj_y, hit_obj_h, hit_obj_h);

            const float end_progress = std::min(1.0f - ((object.beat + object.duration - this->get_beat()) / approach_beats), 1.0f);
            const float hit_obj_thickness = std::max((start_progress - end_progress) * hit_obj_h, key_thickness);
            hit_obj_rect.DrawLines(raylib::Color::White(), hit_obj_thickness);
        }
          
        const std::string key_text_str = val != KeyboardKey::KEY_SPACE ? std::string(1, static_cast<char>(val)) : "___";
        const raylib::Vector2 key_text_size = font.MeasureText(key_text_str.c_str(), static_cast<float>(keys_font_size), 0.0f);
        const raylib::Vector2 key_text_pos(
            rect_keys.x + rect_keys.width * key.proportional_pos.x - key_text_size.x / 2,
            rect_keys.y + rect_keys.height * key.proportional_pos.y - key_text_size.y / 2
        );

        key_rect.DrawLines(key_color, key_thickness);
        font.DrawText(key_text_str.c_str(), key_text_pos, static_cast<float>(keys_font_size), 0.0f, key_color);

        if (key.combo_lost_time > 0.0f)
        {
            const unsigned char combo_lost_alpha = static_cast<unsigned char>(255 * 0.5 * key.combo_lost_time / max_combo_lost_time);
            key_rect.Draw(raylib::Color(255, 0, 0, combo_lost_alpha));
        }
    }

    static constexpr float font_cap_height_multiplier_approx = 0.9f;
    static constexpr float combo_str_offset = 40.0f;
    const std::string combo_str = std::to_string(combo) + "x";

    if (combo_time > 0.0f)
    {
        const float bg_combo_font_size = static_cast<float>((1.0f + 0.5f * combo_time / max_combo_time) * keys_font_size * 1.5);
        const raylib::Vector2 bg_combo_text_size = font.MeasureText(combo_str.c_str(), bg_combo_font_size, 0.0f);
        const raylib::Vector2 bg_combo_text_pos(combo_str_offset, window_dim.y - combo_str_offset - (bg_combo_text_size.y * font_cap_height_multiplier_approx));
        const raylib::Color bg_combo_color = raylib::Color(255, 255, 255, static_cast<unsigned char>(255 * 0.5 * combo_time / max_combo_time));
        font.DrawText(combo_str.c_str(), bg_combo_text_pos, bg_combo_font_size, 0.0f, bg_combo_color);
    }

    const float combo_font_size_multipler = (combo_time > 0.0f) ? 1.0f + 0.1f * combo_time / max_combo_time : 1.0f;
    const float combo_font_size = static_cast<float>(keys_font_size * 1.5) * combo_font_size_multipler;
    const raylib::Vector2 combo_text_size = font.MeasureText(combo_str.c_str(), combo_font_size, 0.0f);
    const raylib::Vector2 combo_text_pos(combo_str_offset, window_dim.y - combo_str_offset - (combo_text_size.y * font_cap_height_multiplier_approx));
    font.DrawText(combo_str.c_str(), combo_text_pos, combo_font_size, 0.0f, raylib::Color::White());
}

float scene::get_beat() const
{
    return (game_time - load_time - music_start_offset) * music_bpm / 60.0f;
}

void scene::lose_combo(kee::key& key)
{
    combo = 0;
    combo_lost_sfx.Play();
    combo_time = 0.0f;

    key.combo_lost_time = max_combo_lost_time;
}

} // namespace kee