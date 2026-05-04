#pragma once
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include "Board.h"
#include <string>
#include <vector>

class UI {
public:
    UI(int piece_size = 60); // Larger pieces to accommodate UI
    ~UI();

    void update(Board* board, color_t perspective, bool has_selection, uint64_t selected, double evaluation, std::string status);
    void render_imgui();
    bool poll_event(sf::Event& event);
    sf::Vector2i get_mouse_position();
    void close();
    bool is_open();
    void clear();
    void display();

    // UI State flags for main loop to consume
    bool request_undo = false;
    bool request_reset = false;
    bool request_start = false;
    color_t selected_side = white;
    bool game_started = false;

private:
    sf::RenderWindow window;
    sf::Clock delta_clock;
    int piece_size;
    int board_size;
    int sidebar_width = 200;
    sf::Texture* textures[12];
    sf::RectangleShape square;
    sf::CircleShape selection_circle;

    void load_assets();
};
