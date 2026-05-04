#include "UI.h"
#include "util.h"
#include <iostream>

UI::UI(int piece_size) : piece_size(piece_size) {
    board_size = piece_size * 8;
    window.create(sf::VideoMode(board_size + sidebar_width, board_size), "ChessCpp");
    window.setFramerateLimit(60);
    
    ImGui::SFML::Init(window);

    load_assets();

    square.setSize(sf::Vector2f(piece_size, piece_size));
    square.setFillColor(sf::Color(50, 150, 0, 255));
    
    selection_circle.setRadius(piece_size / 2);
    selection_circle.setFillColor(sf::Color::Transparent);
    selection_circle.setOutlineColor(sf::Color::Red);
    selection_circle.setOutlineThickness(2);
}

UI::~UI() {
    ImGui::SFML::Shutdown();
    for (int i = 0; i < 12; i++) {
        delete textures[i];
    }
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

void UI::update(Board* board, color_t perspective, bool has_selection, uint64_t selected, double evaluation, std::string status) {
    ImGui::SFML::Update(window, delta_clock.restart());

    // ImGui Sidebar
    ImGui::SetNextWindowPos(ImVec2(board_size, 0));
    ImGui::SetNextWindowSize(ImVec2(sidebar_width, board_size));
    ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    if (!game_started) {
        ImGui::Text("Choose Side:");
        if (ImGui::RadioButton("White", selected_side == white)) selected_side = white;
        if (ImGui::RadioButton("Black", selected_side == black)) selected_side = black;
        
        if (ImGui::Button("Start Game", ImVec2(-1, 40))) {
            request_start = true;
            game_started = true;
        }
    } else {
        ImGui::Text("Evaluation: %.2f", evaluation);
        ImGui::Separator();
        ImGui::TextWrapped("Status: %s", status.empty() ? "Play" : status.c_str());
        ImGui::Separator();
        
        if (ImGui::Button("Undo Move", ImVec2(-1, 30))) {
            request_undo = true;
        }
        
        if (ImGui::Button("Reset Game", ImVec2(-1, 30))) {
            request_reset = true;
            game_started = false;
        }
    }

    ImGui::End();

    // Render Board
    window.clear(sf::Color(30, 30, 30));

    // Draw squares
    for (int i = 0; i < 64; i++) {
        int row = i / 8;
        int column = i % 8;
        if (row % 2 == column % 2) {
            square.setPosition(column * piece_size, perspective == black ? row * piece_size : board_size - (row + 1) * piece_size);
            window.draw(square);
        }
    }

    // Draw pieces
    for (int i = 0; i < 12; i++) {
        sf::Sprite sprite(*textures[i]);
        // Scale sprite to fit piece_size
        float scale = (float)piece_size / textures[i]->getSize().x;
        sprite.setScale(scale, scale);

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

    if (has_selection) {
        int row = selected / 8;
        int column = selected % 8;
        selection_circle.setPosition((7 - column) * piece_size, perspective == black ? row * piece_size : board_size - (row + 1) * piece_size);
        window.draw(selection_circle);
    }
}

bool UI::poll_event(sf::Event& event) {
    bool polled = window.pollEvent(event);
    if (polled) {
        ImGui::SFML::ProcessEvent(window, event);
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
