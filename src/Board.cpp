#include "Board.h"
#include <iostream>
#include "test.h"
#include <unordered_map>
#include <limits>
#include <bit>
#include <algorithm>
#include <cmath>
#include <execution>

using namespace std;

const int EVAL_DEPTH = 4;
const bool debug = true;

Board::Board() {
	previous = nullptr;

	uint64_t pawns = 0b11111111ULL << 8;

	uint64_t bishops = 0b00100100;
	uint64_t knights = 0b01000010;
	uint64_t rooks = 0b10000001;
	uint64_t queens = 0b00010000;
	uint64_t kings = 0b00001000;


	board = {
		pawns, bishops, knights, rooks, queens, kings,
		pawns << 40, bishops << 56, knights << 56, rooks << 56, queens << 56, kings << 56
	};

	bcasle = false;
	wcasle = false;

	bkmove = false;
	wkmove = false;

	brrmove = false;
	wrrmove = false;

	blrmove = false;
	wlrmove = false;
}

Board::Board(Board* previous) {
	this->previous = previous;

	board = previous->board;

	bcasle = previous->bcasle;
	wcasle = previous->wcasle;

	bkmove = previous->bkmove;
	wkmove = previous->wkmove;

	brrmove = previous->brrmove;
	wrrmove = previous->wrrmove;

	blrmove = previous->blrmove;
	wlrmove = previous->wlrmove;
}

Board::~Board() {
}

void Board::get_white(uint64_t& mask) {
	for (int i = 0; i < 6; i++) {
		mask |= board[i];
	}
}

void Board::get_black(uint64_t& mask) {
	for (int i = 6; i < 12; i++) {
		mask |= board[i];
	}
}

void Board::get_all(uint64_t& mask) {
	get_white(mask);
	get_black(mask);
}

void Board::knight_attack(const uint8_t& position, uint64_t& mask) {
	mask = 0;
	uint64_t attack = knight_table[position];
	uint64_t w = 0;
	uint64_t b = 0;

	get_white(w);
	get_black(b);
	uint64_t c = 1ULL << position & w ? w : b;

	mask |= (attack & ~c);
}

void Board::king_attack(const uint8_t& position, uint64_t& mask) {
	//castling needs to be added
	mask = 0;
	uint64_t attack = king_table[position];
	uint64_t w = 0;
	uint64_t b = 0;

	get_white(w);
	get_black(b);
	uint64_t c;
	uint64_t attacks = 0;
	uint64_t a = w | b;

	if (1ULL << position & w) {
		c = w;
		attacked_squares(black, attacks);
		if (!wkmove && !check(white)) {
			if (!wlrmove && !(0b01110000 & (a | attacks))) {
				mask |= 0b10000000;
			}
			if (!wrrmove && !(0b00000110 & (a | attacks))) {
				mask |= 0b00000001;
			}
		}
	}
	else {
		c = b;
		attacked_squares(white, attacks);
		if (!bkmove && !check(black)) {
			if (!blrmove && !(8070450532247928832 & (a | attacks))) {
				mask |= 9223372036854775808;
			}
			if (!brrmove && !(432345564227567616 & (a | attacks))) {
				mask |= 72057594037927936;
			}
		}
	}

	mask |= (attack & ~c);
}

void Board::pawn_attack(const uint8_t& position, uint64_t& mask) {
	//Needs en passant
	uint64_t w = 0;
	uint64_t b = 0;

	get_white(w);
	get_black(b);

	uint64_t a = w | b;
	uint64_t pos = 1ULL << position;
	uint64_t m = pos;

	uint8_t y = position >> 3 << 3;
	uint64_t na = ~a;

	mask = 0;

	if (m & w) {
		//en passant white
		if (m & 1095216660480 && previous) {
			uint8_t epawns = board[black + pawns] >> y & (m << 1 | m >> 1);
			epawns &= (uint8_t) (((previous->board[black + pawns] & ~board[black + pawns]) >> y) >> 16);
			mask |= (((uint64_t) epawns) << y) << 8;
		}

		mask |= m = m << 8 & na;
		mask |= m = (m & 16711680) << 8 & na;

		mask |= wpc_table[position] & b;
	}
	else {
		//en passant black
		if (m & 4278190080 && previous) {
			uint8_t epawns = board[pawns] >> y & (m << 1 | m >> 1);
			epawns &= (uint8_t)(((previous->board[pawns] & ~ board[pawns]) << 16) >> y);
			mask |= (((uint64_t)epawns) << y) >> 8;
		}

		mask |= m = m >> 8 & na;
		mask |= m = (m & 280375465082880) >> 8 & na;

		mask |= bpc_table[position] & w;
	}
}

void Board::bishop_attack(const uint8_t& position, uint64_t& mask) {
	mask = 0;
	uint64_t _pos = 1ULL << position;
	uint64_t pos = _pos;
	bool hit = false;
	uint64_t w = 0;
	uint64_t b = 0;

	get_white(w);
	get_black(b);
	uint64_t a = w | b;
	uint64_t o = pos & w ? b : w;

	uint8_t y = position >> 3;
	uint8_t x = position % 8;

	int y_ = y;
	int x_ = x;

	for (int i = 0; i < 7; i++) {
		y_++;
		x_--;
		if (y_ > 7 || x_ < 0) {
			break;
		}
		pos <<= 7;
		if (o & pos) {
			hit = true;
		}
		else if (a & pos) {
			break;
		}
		mask |= pos;
		if (hit) {
			break;
		}
	}

	pos = _pos;
	hit = false;
	y_ = y;
	x_ = x;

	for (int i = 0; i < 7; i++) {
		y_++;
		x_++;
		if (y_ > 7 || x_ > 7) {
			break;
		}
		pos <<= 9;
		if (o & pos) {
			hit = true;
		}
		else if (a & pos) {
			break;
		}
		mask |= pos;
		if (hit) {
			break;
		}
	}

	pos = _pos;
	hit = false;
	y_ = y;
	x_ = x;

	for (int i = 0; i < 7; i++) {
		y_--;
		x_--;
		if (y_ < 0 || x_ < 0) {
			break;
		}
		pos >>= 9;
		if (o & pos) {
			hit = true;
		}
		else if (a & pos) {
			break;
		}
		mask |= pos;
		if (hit) {
			break;
		}
	}

	pos = _pos;
	hit = false;
	y_ = y;
	x_ = x;

	for (int i = 0; i < 7; i++) {
		y_--;
		x_++;
		if (y_ < 0 || x_ > 7) {
			break;
		}
		pos >>= 7;
		if (o & pos) {
			hit = true;
		}
		else if (a & pos) {
			break;
		}
		mask |= pos;
		if (hit) {
			break;
		}
	}

	//There must be an easier way to do this
}

void Board::rank_attack(const uint8_t& position, uint64_t& mask) {
	uint8_t line = 0b11111111;
	uint64_t pos = 1ULL << position;

	uint64_t w = 0;
	uint64_t b = 0;

	uint8_t y = position >> 3 << 3;
	uint8_t x = position % 8;
	uint8_t pos_line = pos >> y;

	get_white(w);
	get_black(b);
	uint64_t a = w | b;
	uint8_t o = (pos & w ? b : w) >> y;

	uint8_t rank = line ^ a >> y;
	uint8_t right = rank & (uint8_t)(line >> 8 - x);
	uint8_t left = rank & (uint8_t)(line << x + 1);
	uint8_t rfirst = std::countl_one((uint8_t)(right << 8 - x));
	uint8_t lfirst = std::countr_one((uint8_t)(left >> x + 1));

	left &= line >> (7 - x - lfirst);
	left |= (pos_line << lfirst + 1) & o;
	right &= line << (x - rfirst);
	right |= (pos_line >> rfirst + 1) & o;
	mask = (left | right);
	mask <<= y;
}

void Board::file_attack(const uint8_t& position, uint64_t& mask) {
	mask = 0;
	uint64_t _pos = 1ULL << position;
	uint64_t pos = _pos;
	bool hit = false;
	uint64_t w = 0;
	uint64_t b = 0;

	get_white(w);
	get_black(b);
	uint64_t a = w | b;
	uint64_t o = pos & w ? b : w;

	uint8_t y = position >> 3;

	int y_ = y;

	//up
	for (int i = 0; i < 7; i++) {
		y_++;
		if (y_ > 7) {
			break;
		}
		pos <<= 8;
		if (o & pos) {
			hit = true;
		}
		else if (a & pos) {
			break;
		}
		mask |= pos;
		if (hit) {
			break;
		}
	}

	pos = _pos;
	hit = false;
	y_ = y;

	//down
	for (int i = 0; i < 7; i++) {
		y_--;
		if (y_ < 0) {
			break;
		}
		pos >>= 8;
		if (o & pos) {
			hit = true;
		}
		else if (a & pos) {
			break;
		}
		mask |= pos;
		if (hit) {
			break;
		}
	}
}

void Board::rook_attack(const uint8_t& position, uint64_t& mask) {
	uint64_t r = 0;
	uint64_t f = 0;
	rank_attack(position, r);
	file_attack(position, f);
	mask = r | f;
}

void Board::get_moves(const uint8_t& position, uint64_t& mask) {
	uint64_t pos = 1ULL << position;
	for (int i = 0; i < 6; i++) {
		uint64_t wboard = board[i];
		uint64_t bboard = board[i + black];
		uint64_t r = 0;
		uint64_t b = 0;
		if (wboard & pos || bboard & pos) {
			switch (i) {
			case pawns:
				pawn_attack(position, mask);
				break;
			case bishops:
				bishop_attack(position, mask);
				return;
			case knights:
				knight_attack(position, mask);
				break;
			case rooks:
				rook_attack(position, mask);
				return;
			case queens:
				rook_attack(position, r);
				bishop_attack(position, b);
				mask = r | b;
				return;
			case kings:
				king_attack(position, mask);
				break;
			default:
				cout << "invalid piece" << endl;
				break;
			}
		}
	}
}

void Board::get_attackers(const uint8_t& position, uint64_t& mask) {
	mask = 0;
	uint64_t b = 0;
	uint64_t w = 0;
	get_black(b);
	get_white(w);
	uint64_t pos = 1ULL << position;
	uint64_t o = pos & w ? b: w;

	int leading;
	while (o) {
		leading = std::countr_zero(o);
		uint64_t moves = 0;
		get_attacks(leading, moves);
		if (moves & pos) {
			mask |= 1ULL << leading;
		}
		o -= 1ULL << leading;
	}
}

void Board::get_attacks(const uint8_t& position, uint64_t& mask) {
	uint64_t pos = 1ULL << position;
	if ((pos & board[kings]) || (pos & board[black + kings])) {
		mask |= king_table[position];
	}
	else if (pos & board[pawns]) {
		mask |= wpc_table[position];
	}
	else if (pos & board[black + pawns]) {
		mask |= bpc_table[position];
	}
	else {
		get_moves(position, mask);
	}
}

void Board::attacked_squares(const color_t& color, uint64_t& mask) {
	uint64_t a = 0;
	color == white ? get_white(a) : get_black(a);
	int leading;
	while (a) {
		leading = std::countr_zero(a);
		uint64_t moves = 0;
		get_attacks(leading, moves);
		mask |= moves;
		a -= 1ULL << leading;
	}
}

bool Board::check(const color_t& color) {
	uint64_t king_pos = color == white ? std::countr_zero(board[kings]) : std::countr_zero(board[black + kings]);
	uint64_t attackers = 0;
	get_attackers(king_pos, attackers);
	return attackers;
}

bool Board::checkmate(const color_t& color) {
	vector<Board*> moves;
	bool r = false;
	if (check(color)) {
		moves = get_moves(color);
		for (Board* move : moves) {
			bool check = move->check(color);
			if (!check) {
				r = false;
				goto e;
			}
		}
		r = true;
	}
e:;
	for (Board* move : moves) {
		delete move;
	}
	return r;
}

bool Board::stalemate(const color_t& color) {
	vector<Board*> moves;
	bool r = false;
	if (!check(color)) {
		moves = get_moves(color);
		for (Board* move : moves) {
			bool check = move->check(color);
			if (!check) {
				r = false;
				goto e;
			}
		}
		r = true;
	}
e:;
	for (Board* move : moves) {
		delete move;
	}
	return r;
}

bool Board::stalemate() {
	std::vector<std::array<uint64_t, 12>> history;
	Board* curr = this;
	while (curr) {
		history.push_back(curr->board);
		curr = curr->previous;
	}
	for (size_t i = 0; i < history.size(); ++i) {
		int count = 1;
		for (size_t j = i + 1; j < history.size(); ++j) {
			if (history[i] == history[j]) {
				count++;
			}
		}
		if (count >= 3) return true;
	}
	return false;
}

void Board::move(const uint64_t& start, const uint64_t& dest) {
	uint64_t w = 0;
	uint64_t b = 0;
	get_white(w);
	get_black(b);

	for (int i = 11; i >= 0; i--) {
		uint64_t& sub = board[i];
		if (sub & start) {
			sub ^= start;
			if (i == pawns) {
				if (dest & 18374686479671623680) {
					board[queens] |= dest;
					continue;
				}
				else if (start & 1095216660480 && !(dest & b) && !(start << 8 & dest)) {
					board[black + pawns] &= ~(dest >> 8);
				}
			}
			else if (i == black + pawns) {
				if (dest & 255) {
					board[black + queens] |= dest;
					continue;
				}
				else if (start & 4278190080 && !(dest & w) && !(start >> 8 & dest)) {
					board[pawns] &= ~(dest << 8);
				}
			} else if (i == rooks) {
				if (start & 0b10000000) {
					wlrmove = true;
				} else {
					wrrmove = true;
				}
			}
			else if (i == black + rooks) {
				if (start & 9223372036854775808) {
					blrmove = true;
				} else {
					brrmove = true;
				}
			}
			else if (i == kings) {
				wkmove = true;
				if (dest & board[rooks]) {
					if (dest & 0b10000000) {
						board[kings] |= 0b00100000;
						board[rooks] &= ~dest;
						board[rooks] |= 0b00010000;
					}
					else {
						board[kings] |= 0b00000010;
						board[rooks] &= ~dest;
						board[rooks] |= 0b00000100;
					}
					wcasle = true;
					return;
				}
			}
			else if (i == black + kings) {
				bkmove = true;
				if (dest & board[black + rooks]) {
					if (dest & 9223372036854775808) {
						board[black + kings] |= 2305843009213693952;
						board[black + rooks] &= ~dest;
						board[black + rooks] |= 1152921504606846976;
					}
					else {
						board[black + kings] |= 144115188075855872;
						board[black + rooks] &= ~dest;
						board[black + rooks] |= 288230376151711744;
					}
					bcasle = true;
					return;
				}
			}

			sub |= dest;
		}
		else {
			sub &= ~dest;
		}
	}
}

vector<Board*> Board::get_moves(const color_t& color) {
	//gets all moves and return them as an array;
	std::vector<Board*> move_vec;
	uint64_t pieces = 0;
	color == white ? get_white(pieces) : get_black(pieces);
	uint64_t opponents = 0;
	color == white ? get_black(opponents) : get_white(opponents);

	int p_leading;
	while (pieces) {
		p_leading = std::countr_zero(pieces);
		uint64_t moves = 0;
		get_moves(p_leading, moves);
		int m_leading;
		while (moves) {
			m_leading = std::countr_zero(moves);
			Board* next = new Board(this);
			bool is_capture = (1ULL << m_leading) & opponents;
			next->move(1ULL << p_leading, 1ULL << m_leading);
			if (!next->check(color)) {
				if (is_capture) {
					move_vec.insert(move_vec.begin(), next); // Basic move ordering
				} else {
					move_vec.push_back(next);
				}
			}
			else {
				delete next;
			}
			moves -= 1ULL << m_leading;
		}
		pieces -= 1ULL << p_leading;
	}
	return move_vec;
}

double Board::evaluate(color_t color) {
	int min;
	int max;
	int value = 0;
	static const std::array<uint64_t, 12> default_board = Board().board;
	uint64_t center_pawns = 103481868288;
	uint64_t vision = 0;
	bool castled;
	bool cc;
	int target;
	attacked_squares(color, vision);

	uint64_t pieces = 0;
	uint64_t p;
	uint64_t op_atk = 0;
	uint64_t a = 0;
	get_all(a);

	if (color == white) {
		get_white(pieces);
		attacked_squares(black, op_atk);
		p = board[pawns];
		center_pawns &= p;
		min = 0;
		max = 6;
		castled = wcasle;
		cc = !wkmove && (!wrrmove || !wlrmove);
		target = 7;
	}
	else {
		get_black(pieces);
		attacked_squares(white, op_atk);
		p = board[black + pawns];
		center_pawns &= p;
		min = 6;
		max = 12;
		castled = bcasle;
		cc = !bkmove && (!brrmove || !blrmove);
		target = 0;
	}

	double development = 0;
	for (int i = min; i < max; i++) {
		uint64_t sub = board[i];
		uint8_t val = value_table[i];
		uint16_t d = 0;
		uint64_t developed = (default_board[i] ^ sub) & ~default_board[i];
		d += std::popcount(developed & 35604928818740736) * 0.1;
		d += std::popcount(developed & 66229406269440) * 0.75;
		development -= std::popcount(op_atk & sub) * std::pow(val, 0.25);
		development += d / abs(4 * (val - 3.5));
		value += val * std::popcount(sub);
	}

	int leading;
	while (p) {
		leading = std::countr_zero(p);
		int x = leading % 8;
		uint64_t column = 72340172838076673 << x;
		int distance = abs(target - (leading >> 3));

		if (!(column & (op_atk | (a & ~(1ULL << leading))))) {
			development += 8.0 / distance;
		}
		else {
			development += 3 / distance;
		}

		p -= 1ULL << leading;
	}

	return value + development + std::popcount(center_pawns) * 0.5 + std::popcount(vision) * 0.05 + (cc ? 0 : (castled ? 1 : -1));
}

pair<Board*, double> Board::get_best(const color_t& color, const bool& show) {
	color_t ocolor = color == white ? black : white;
	vector<Board*> moves = get_moves(color);
	
	if (moves.empty()) {
		return {nullptr, check(color) ? -10000.0 : 0.0};
	}

	std::vector<double> results(moves.size());
	std::vector<size_t> indices(moves.size());
	std::iota(indices.begin(), indices.end(), 0);

	std::for_each(std::execution::par, indices.begin(), indices.end(), [&](size_t i) {
		double alpha = -std::numeric_limits<double>::infinity();
		double beta = std::numeric_limits<double>::infinity();
		results[i] = reval(moves[i], color, ocolor, 1, alpha, beta);
	});

	pair<Board*, double> best = {nullptr, -std::numeric_limits<double>::infinity()};
	for (size_t i = 0; i < moves.size(); ++i) {
		if (debug && show) {
			cout << "result: " << results[i] << endl;
			render_board(moves[i], results[i]);
		}
		if (results[i] > best.second) {
			best.first = moves[i];
			best.second = results[i];
		}
	}

	if (!best.first && !moves.empty()) best.first = moves[0];
		
	for (Board* move : moves) {
		if (move != best.first) {
			delete move;
		}
	}
	return best;
}

double reval(Board* board, const color_t& og_color, const color_t& curr_color, const int& depth, double alpha, double beta) {
	color_t opog_color = og_color == white ? black : white;
	color_t op_color = curr_color == white ? black : white;
	bool is_maximizing = (og_color == curr_color);

	bool check = board->check(curr_color);
	vector<Board*> moves = board->get_moves(curr_color);

	if (moves.empty()) {
		if (check) {
			return is_maximizing ? -10000.0 - (EVAL_DEPTH - depth) : 10000.0 + (EVAL_DEPTH - depth);
		} else {
			return 0.0;
		}
	}

	if (board->stalemate()) return 0.0;

	if (depth >= EVAL_DEPTH) {
		return board->evaluate(og_color) - board->evaluate(opog_color);
	}

	double eval = is_maximizing ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity();

	for (Board* move : moves) {
		double result = reval(move, og_color, op_color, depth + 1, alpha, beta);

		if (is_maximizing) {
			eval = max(eval, result);
			alpha = max(alpha, eval);
		}
		else {
			eval = min(eval, result);
			beta = min(beta, eval);
		}

		if (beta <= alpha) break;
	}

	for (Board* move : moves) {
		delete move;
	}
	return eval;
}

