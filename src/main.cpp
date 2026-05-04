#include <SFML/Graphics.hpp>
#include <cstdio>
#include <iostream>
#include <unordered_map>
#include <string>
#include <thread>
#include <vector>
#include "Board.h"
#include "util.h"
#include "test.h"
#include "UI.h"

void delete_chain(Board* b) {
	while (b) {
		Board* p = b->previous;
		delete b;
		b = p;
	}
}

int engine() {
	char color;
	std::cin >> color;
	color_t engine_color = color == 'w' ? white : black;

	Board* board = new Board();

	while (true) {
		board = new Board(board);
		for (int i = 0; i < 12; i++) {
			uint64_t value;
			if (!(cin >> value)) return 0;
			board->board[i] = value;
		}

		board = board->get_best(engine_color).first;
		uint64_t prev = 0;
		engine_color == white ? board->previous->get_white(prev): board->previous->get_black(prev);
		uint64_t curr = 0;
		engine_color == white ? board->get_white(curr) : board->get_black(curr);

		uint64_t diff = prev ^ curr;
		int start = std::countr_zero(prev & diff);
		int end = std::countr_zero(curr & diff);

		std::cout << to_string(start) << std::endl;
		std::cout << to_string(end) << std::endl;
	}
}

int play() {
	UI ui(60);
	color_t color = white;
	bool can_move = false;
	std::thread* move_thread = nullptr;

	Board* board = new Board();
	double current_eval = 0;
	std::string current_status = "Welcome! Choose side and start.";

	auto start_bot = [&]() {
		if (move_thread) {
			if (move_thread->joinable()) move_thread->join();
			delete move_thread;
		}
		move_thread = new std::thread(
			[&]() {
				auto best = board->get_best(color == white ? black : white);
				board = best.first;
				current_eval = best.second;
				can_move = true;
				
				if (board->checkmate(white)) current_status = "White Checkmate";
				else if (board->checkmate(black)) current_status = "Black Checkmate";
				else if (board->stalemate()) current_status = "Stalemate";
				else current_status = "";
			}
		);
	};

	bool has_selection = false;
	uint64_t selected = 0;
	uint64_t current_possible_moves = 0;

	while (ui.is_open()) {
		sf::Event event;
		while (ui.poll_event(event)) {
			if (event.type == sf::Event::Closed) {
				ui.close();
			}

			if (event.type == sf::Event::MouseButtonPressed && ui.game_started && can_move) {
				sf::Vector2i position = ui.get_mouse_position();
				float piece_size = ui.get_piece_size();
				float board_size = piece_size * 8;
				
				if (position.x < board_size) { // Only handle clicks on board
					int x = (int)((board_size - (float)position.x) / piece_size);
					int y = (int)((board_size - (float)position.y) / piece_size);
					if (color == black) y = 7 - y;
					int pos = x + y * 8;
					uint64_t click_pos = 1ULL << pos;
					uint64_t mask = 0ULL;
					color == white ? board->get_white(mask) : board->get_black(mask);

					if (has_selection) {
						if (current_possible_moves & click_pos) {
							Board* next = new Board(board);
							next->move(1ULL << selected, click_pos);
							if (!next->check(color)) {
								board = next;
								can_move = false;
								has_selection = false;
								current_possible_moves = 0;
								
								if (board->checkmate(white)) current_status = "White Checkmate";
								else if (board->checkmate(black)) current_status = "Black Checkmate";
								else if (board->stalemate()) current_status = "Stalemate";
								else {
									current_status = "Bot thinking...";
									start_bot();
								}
							} else {
								delete next;
							}
						}
					}
					
					if (mask & click_pos) {
						selected = pos;
						has_selection = true;
						current_possible_moves = 0;
						board->get_moves(selected, current_possible_moves);
					} else {
						has_selection = false;
						current_possible_moves = 0;
					}
				}
			}
		}

		// Handle UI requests
		if (ui.request_start) {
			ui.request_start = false;
			color = ui.selected_side;
			can_move = (color == white);
			current_status = (color == white) ? "Your turn (White)" : "Bot thinking...";
			if (color == black) start_bot();
		}

		if (ui.request_undo && board->previous && board->previous->previous) {
			ui.request_undo = false;
			Board* prev = board->previous;
			Board* prevprev = prev->previous;
			board->previous = nullptr;
			delete board;
			prev->previous = nullptr;
			delete prev;
			board = prevprev;
			current_status = "Undone.";
			current_eval = board->evaluate(white) - board->evaluate(black);
			can_move = true;
			has_selection = false;
			current_possible_moves = 0;
		} else if (ui.request_undo) {
			ui.request_undo = false;
		}

		if (ui.request_reset) {
			ui.request_reset = false;
			if (move_thread) {
				if (move_thread->joinable()) move_thread->join();
				delete move_thread;
				move_thread = nullptr;
			}
			delete_chain(board);
			board = new Board();
			can_move = false;
			current_eval = 0;
			current_status = "Game reset.";
			has_selection = false;
			current_possible_moves = 0;
		}

		ui.update(board, color, has_selection, selected, current_possible_moves, current_eval, current_status);
		ui.display();
		
		if (move_thread && can_move) {
			if (move_thread->joinable()) move_thread->join();
			delete move_thread;
			move_thread = nullptr;
		}
	}

	if (move_thread) {
		if (move_thread->joinable()) move_thread->join();
		delete move_thread;
	}
	delete_chain(board);
	return 0;
}

int main(int argc, char** argv) {
	if (argc > 1 && std::string(argv[1]) == "--engine") {
		return engine();
	}
	return play();
}
