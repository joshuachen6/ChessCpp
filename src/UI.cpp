#include "UI.h"
#include "util.h"
#include <iostream>
#include <cmath>

UI::UI(int piece_size_init) {
    float initial_piece_size = static_cast<float>(piece_size_init);
    board_size = initial_piece_size * 8.0f;
    sidebar_width = board_size * sidebar_width_ratio;
    target_aspect_ratio = (board_size + sidebar_width) / board_size;

    window.create(sf::VideoMode(static_cast<unsigned int>(board_size + sidebar_width), 
                                static_cast<unsigned int>(board_size)), "ChessCpp", sf::Style::Default);
    window.setFramerateLimit(60);
    
    ImGui::SFML::Init(window);

    load_assets();

    square.setFillColor(sf::Color(50, 150, 0, 255));
    
    selection_circle.setFillColor(sf::Color::Transparent);
    selection_circle.setOutlineColor(sf::Color::Red);
    selection_circle.setOutlineThickness(3.0f);

    move_dot.setFillColor(sf::Color(255, 0, 0, 150)); // Semi-transparent red

    handle_resize(window.getSize().x, window.getSize().y);
}

UI::~UI() {
    ImGui::SFML::Shutdown();
    for (int i = 0; i < 12; i++) {
        delete textures[i];
    }
}

void UI::handle_resize(unsigned int width, unsigned int height) {
    float A = (float)width * (float)height;
    float R = target_aspect_ratio;
    
    unsigned int new_height = static_cast<unsigned int>(std::sqrt(A / R));
    unsigned int new_width = static_cast<unsigned int>((float)new_height * R);

    // Only set size if it actually changed to avoid infinite resize loops
    if (new_width != width || new_height != height) {
        window.setSize(sf::Vector2u(new_width, new_height));
        return; // setSize will trigger another Resize event
    }

    // Update internal metrics
    sf::Vector2u size = window.getSize();
    board_size = (float)size.y;
    piece_size = board_size / 8.0f;
    sidebar_width = (float)size.x - board_size;

    sf::View view(sf::FloatRect(0, 0, (float)size.x, (float)size.y));
    window.setView(view);

    square.setSize(sf::Vector2f(piece_size, piece_size));
    selection_circle.setRadius(piece_size / 2.0f - 4.0f);
    selection_circle.setOrigin(-4.0f, -4.0f); // Adjust for thickness
    
    move_dot.setRadius(piece_size / 6.0f);
    move_dot.setOrigin(-piece_size/3.0f, -piece_size/3.0f); // Center the dot
}

void UI::load_assets() {
    const std::string names[] = {
        "wp.png", "wb.png", "wn.png", "wr.png", "wq.png", "wk.png",
        "bp.png", "bb.png", "bn.png", "br.png", "bq.png", "bk.png"
    };
    for (int i = 0; i < 12; i++) {
        textures[i] = load_texture(names[i]);
    }
}

void UI::update(Board* board, color_t perspective, bool has_selection, uint64_t selected, uint64_t possible_moves, double evaluation, std::string status) {
    ImGui::SFML::Update(window, delta_clock.restart());

    // Dynamic Font Scaling
    float scale = board_size / 480.0f; // Base size 480
    ImGui::GetIO().FontGlobalScale = std::max(1.0f, scale);

    // ImGui Sidebar
    ImGui::SetNextWindowPos(ImVec2(board_size, 0));
    ImGui::SetNextWindowSize(ImVec2(sidebar_width, board_size));
    ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    if (!game_started) {
        ImGui::Text("Choose Side:");
        if (ImGui::RadioButton("White", selected_side == white)) selected_side = white;
        if (ImGui::RadioButton("Black", selected_side == black)) selected_side = black;
        
        ImGui::Spacing();
        if (ImGui::Button("Start Game", ImVec2(-1, 40 * scale))) {
            request_start = true;
            game_started = true;
        }
    } else {
        ImGui::Text("Evaluation: %.2f", evaluation);
        ImGui::Separator();
        ImGui::TextWrapped("Status: %s", status.empty() ? "Play" : status.c_str());
        ImGui::Separator();
        
        ImGui::Spacing();
        if (ImGui::Button("Undo Move", ImVec2(-1, 30 * scale))) {
            request_undo = true;
        }
        
        ImGui::Spacing();
        if (ImGui::Button("Reset Game", ImVec2(-1, 30 * scale))) {
            request_reset = true;
            game_started = false;
        }
    }

    ImGui::End();

    // Render Board
    window.clear(sf::Color::White);

    // Draw squares (Green squares on white background)
    for (int i = 0; i < 64; i++) {
        int row = i / 8;
        int column = i % 8;
        if ((row + column) % 2 == 1) { // Standard alternating pattern
            square.setPosition(column * piece_size, perspective == black ? row * piece_size : board_size - (row + 1) * piece_size);
            window.draw(square);
        }
    }

    // Draw selection ring UNDER pieces
    if (has_selection) {
        int row = selected / 8;
        int column = selected % 8;
        selection_circle.setPosition((7 - column) * piece_size, perspective == black ? row * piece_size : board_size - (row + 1) * piece_size);
        window.draw(selection_circle);
    }

    // Draw possible move dots
    if (has_selection) {
        for (int i = 0; i < 64; i++) {
            if (possible_moves & (1ULL << i)) {
                int row = i / 8;
                int column = i % 8;
                move_dot.setPosition((7 - column) * piece_size, perspective == black ? row * piece_size : board_size - (row + 1) * piece_size);
                window.draw(move_dot);
            }
        }
    }

    // Draw pieces
    for (int i = 0; i < 12; i++) {
        sf::Sprite sprite(*textures[i]);
        float tex_size = (float)textures[i]->getSize().x;
        float s = piece_size / tex_size;
        sprite.setScale(s, s);

        uint64_t sub = board->board[i];
        for (int j = 0; j < 64; j++) {
            if (sub & (1ULL << j)) {
                int row = j / 8;
                int column = j % 8;
                sprite.setPosition((7 - column) * piece_size, perspective == black ? row * piece_size : board_size - (row + 1) * piece_size);
                window.draw(sprite);
            }
        }
    }
}

bool UI::poll_event(sf::Event& event) {
    bool polled = window.pollEvent(event);
    if (polled) {
        ImGui::SFML::ProcessEvent(window, event);
        if (event.type == sf::Event::Resized) {
            handle_resize(event.size.width, event.size.height);
        }
    }
    return polled;
}

sf::Vector2i UI::get_mouse_position() {
    return sf::Mouse::getPosition(window);
}

void UI::close() {
    window.close();
}

bool UI::is_open() {
    return window.isOpen();
}

void UI::clear() {
    window.clear();
}

void UI::display() {
    ImGui::SFML::Render(window);
    window.display();
}
