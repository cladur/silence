#ifndef SILENCE_UI_MANAGER_H
#define SILENCE_UI_MANAGER_H

#include "render/render_scene.h"
#include "render/transparent_elements/text/text_draw.h"
#include "render/transparent_elements/ui/sprite_draw.h"
#include "render/transparent_elements/ui/ui_elements/ui_anchor.h"
#include "render/transparent_elements/ui/ui_elements/ui_button.h"
#include "render/transparent_elements/ui/ui_elements/ui_element.h"
#include "render/transparent_elements/ui/ui_elements/ui_image.h"
#include "render/transparent_elements/ui/ui_elements/ui_slider.h"
#include "render/transparent_elements/ui/ui_elements/ui_text.h"

// class UIElement;
// class UIImage;
// class UIButton;
// class UIText;
// class UISlider;
// class UIAnchor;

struct UIScene {
    bool is_active = false;
    std::unordered_map<std::string, UIElement*> roots;

    // i suppose this isn't necessary because anchords *should* usually be the roots
    std::unordered_map<std::string, UIAnchor> anchors;
    std::unordered_map<std::string, UIImage> images;
    std::unordered_map<std::string, UIButton> buttons;
    std::unordered_map<std::string, UIText> texts;
    std::unordered_map<std::string, UISlider> sliders;
};

class UIManager {
    std::unordered_map<std::string, UIScene> scenes;
    RenderScene *render_scene;

public:

    TextDraw text_draw;
    SpriteDraw sprite_draw;

    static UIManager &get();
    void startup();
    void set_render_scene(RenderScene *render_scene);
    void create_ui_scene(const std::string &scene_name);
    void delete_ui_scene(const std::string &scene_name);
    void activate_ui_scene(const std::string &scene_name);
    void deactivate_ui_scene(const std::string &scene_name);

    UIAnchor &add_ui_anchor(const std::string &scene_name, const std::string &anchor_name);
    UIImage &add_ui_image(const std::string &scene_name, const std::string &image_name);
    UIButton &add_ui_button(
        const std::string &scene_name, 
        const std::string &button_name, 
        const std::string &hover_event_name, 
        const std::string &click_event_name);
    UIText &add_ui_text(const std::string &scene_name, const std::string &text_name);
    UISlider &add_ui_slider(const std::string &scene_name, const std::string &lider_name);
    void add_as_root(const std::string &scene_name, const std::string &element_name);
    void add_to_root(const std::string &scene_name, const std::string &element_name, const std::string &root_name);

    UIAnchor &get_ui_anchor(const std::string &scene_name, const std::string &anchor_name);
    UIImage &get_ui_image(const std::string &scene_name, const std::string &image_name);
    UIButton &get_ui_button(const std::string &scene_name, const std::string &button_name);
    UIText &get_ui_text(const std::string &scene_name, const std::string &text_name);
    UISlider &get_ui_slider(const std::string &scene_name, const std::string &lider_name);

    void draw();
};

#endif