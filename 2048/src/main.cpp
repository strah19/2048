#include "Application.h"
#include "RandomNumberGenerator.h"
#include "Font.h"

constexpr uint32_t ROWS = 4;
constexpr uint32_t COLS = 4;

constexpr int SQ_SIZE = 128;
constexpr int SQ_PADDING = 16;
constexpr int MAX_SQ_INITS = 2;

const Ember::Color DEFAULT_SQ_COLOR = Ember::Color(204, 192, 179, 255);

const Ember::Color GAMEBOARD_COLOR = Ember::Color(187, 173, 160, 255);

const Ember::IVec2 GAMEBOARD_OFFSET = { 64, 128 };

class Game2048 : public Ember::Application {
public:
	Game2048() { }
	void OnCreate() { 
		color_mapper[2]    = Ember::Color(238, 228, 218, 255);
		color_mapper[4]    = Ember::Color(237, 224, 200, 255);
		color_mapper[8]    = Ember::Color(242, 177, 121, 255);
		color_mapper[16]   = Ember::Color(245, 149, 99, 255);
		color_mapper[32]   = Ember::Color(246, 124, 96, 255);
		color_mapper[64]   = Ember::Color(246, 94, 59, 255);
		color_mapper[128]  = Ember::Color(237, 207, 115, 255);
		color_mapper[256]  = Ember::Color(237, 204, 98, 255);
		color_mapper[512]  = Ember::Color(237, 200, 80, 255);
		color_mapper[1024] = Ember::Color(237, 197, 63, 255);
		color_mapper[2048] = Ember::Color(237, 194, 45, 255);

		init();

		text.Initialize(renderer, "res/ClearSans-Medium.ttf", 48);
	}

	void init() {
		game_over = false;
		score = 0;
		int initialized = 0;
		for (int y = 0; y < COLS; y++) {
			for (int x = 0; x < ROWS; x++) {
				squares[x][y] = (initialized == MAX_SQ_INITS) ? 0 : (Ember::RandomGenerator::GenRandom(0, 2) == 2) ? 0 : 2;

				if (squares[x][y] == 2)
					initialized++;
			}
		}
	}

	virtual ~Game2048() { }

	void OnUserUpdate() {
		window->Update();
		
		game_over = true;
		for (int y = 0; y < COLS; y++) {
			for (int x = 0; x < ROWS; x++) {
				if (can_shift_down(x, y) || can_shift_left(x, y) || can_shift_right(x, y) || can_shift_up(x, y)) {
					game_over = false;
					break;
				}
			}
		}

		renderer->Clear(background_color);

		renderer->Rectangle({ GAMEBOARD_OFFSET.x - SQ_PADDING, GAMEBOARD_OFFSET.y - SQ_PADDING, ROWS * SQ_SIZE + SQ_PADDING, COLS * SQ_SIZE + SQ_PADDING }, GAMEBOARD_COLOR);

		for (int y = 0; y < COLS; y++) {
			for (int x = 0; x < ROWS; x++) {
				Ember::Color square_color = DEFAULT_SQ_COLOR;
				if (squares[x][y]) 
					square_color = color_mapper[squares[x][y]];
				
				renderer->Rectangle({ x * SQ_SIZE + GAMEBOARD_OFFSET.x, y * SQ_SIZE + GAMEBOARD_OFFSET.y, SQ_SIZE - SQ_PADDING, SQ_SIZE - SQ_PADDING }, square_color);

				if (squares[x][y]) {
					uint32_t v = squares[x][y];
					Ember::Font::Draw(squares[x][y], { 30, 30, 30, 255 }, 
						{ x * SQ_SIZE + GAMEBOARD_OFFSET.x + (SQ_SIZE / 2) - (text.GetSizeFromText(std::to_string(v)).x / 2) - (SQ_PADDING / 2),
						  y * SQ_SIZE + GAMEBOARD_OFFSET.y + (SQ_SIZE / 2) - (text.GetSizeFromText(std::to_string(v)).y / 2) - (SQ_PADDING / 2) }, &text);
				}
			}
		}

		std::stringstream ss;
		ss << "Score: ";
		ss << score;

		Ember::Font::Draw(ss.str(), { 0, 0, 0, 255 }, { 0, 0 }, &text);

		if (game_over) {
			Ember::Font::Draw("Game Over!", { 0, 0, 0, 255 }, 
				{ (properties->width / 2) - (text.GetSizeFromText("Game Over!").x / 2), (properties->height / 2) - (text.GetSizeFromText("Game Over!").y / 2) }, &text);
		}

		renderer->Show();
	}

	bool can_shift_down(int x, int y) {
		return ((squares[x][y + 1] == 0 && y + 1 < COLS) || (squares[x][y + 1] == squares[x][y] && y + 1 < COLS));
	}

	bool can_shift_up(int x, int y) {
		return(squares[x][y - 1] == 0 && y - 1 > -1 || squares[x][y - 1] == squares[x][y] && y - 1 > -1);
	}

	bool can_shift_left(int x, int y) {
		return(squares[x - 1][y] == 0 && x - 1 > -1 || squares[x - 1][y] == squares[x][y] && x - 1 > -1);
	}

	bool can_shift_right(int x, int y) {
		return(squares[x + 1][y] == 0 && x + 1 < ROWS || squares[x + 1][y] == squares[x][y] && x + 1 < ROWS);
	}

	void shift_value_down(int x, int y) {
		if (squares[x][y + 1] == 0 && y + 1 < COLS) {
			swap_value({ x, y }, { x, y + 1 });
			shift_value_down(x, y + 1);
		}
		else if (squares[x][y + 1] == squares[x][y] && y + 1 < COLS) {
			squares[x][y + 1] *= 2;
			score += squares[x][y + 1];
			squares[x][y] = 0;
			shift_value_down(x, y + 1);
		}
	}

	void shift_value_up(int x, int y) {
		if (squares[x][y - 1] == 0 && y - 1 > -1) {
			swap_value({ x, y }, { x, y - 1 });
			shift_value_up(x, y - 1);
		}
		else if (squares[x][y - 1] == squares[x][y] && y - 1 > -1) {
			squares[x][y - 1] *= 2;
			score += squares[x][y - 1];
			squares[x][y] = 0;
			shift_value_up(x, y - 1);
		}
	}

	void shift_value_left(int x, int y) {
		if (squares[x - 1][y] == 0 && x - 1 > -1) {
			swap_value({ x, y }, { x - 1, y });
			shift_value_left(x - 1, y);
		}
		else if (squares[x - 1][y] == squares[x][y] && x - 1 > -1) {
			squares[x - 1][y] *= 2;
			score += squares[x - 1][y];
			squares[x][y] = 0;
			shift_value_left(x - 1, y);
		}
	}

	void shift_value_right(int x, int y) {
		if (squares[x + 1][y] == 0 && x + 1 < ROWS) {
			swap_value({ x, y }, { x + 1, y});
			shift_value_right(x + 1, y);
		}
		else if (squares[x + 1][y] == squares[x][y] && x + 1 < ROWS) {
			squares[x + 1][y] *= 2;
			score += squares[x + 1][y];
			squares[x][y] = 0;
			shift_value_right(x + 1, y);
		}
	}

	void swap_value(const Ember::IVec2& index1, const Ember::IVec2& index2) {
		squares[index2.x][index2.y] = squares[index1.x][index1.y];
		squares[index1.x][index1.y] = 0;
	}

	void add_new_two() {
		int size = 0;
		int possible_locs[COLS * ROWS];
		for (int y = 0; y < COLS; y++) {
			for (int x = 0; x < ROWS; x++) {
				if (squares[x][y] == 0) {
					possible_locs[size] = (x * ROWS) + y;
					size++;
				}
			}
		}

		if(size) {
			int r = Ember::RandomGenerator::GenRandom(0, size - 1);
			if(r < size)
				squares[possible_locs[r] / ROWS][possible_locs[r] % COLS] = 2;
		}
	}

	void KeyboardEvent(Ember::KeyboardEvents& keyboard) {
		switch (keyboard.scancode) {
		case Ember::EmberKeyCode::LeftArrow:
			for (int y = 0; y < COLS; y++) {
				for (int x = 0; x < ROWS; x++) {
					if (squares[x][y] != 0 && x != 0) {
						shift_value_left(x, y);
					}
				}
			}
			add_new_two();
			break;
		case Ember::EmberKeyCode::RightArrow:
			for (int y = 0; y < COLS; y++) {
				for (int x = ROWS - 1; x >= 0; x--) {
					if (squares[x][y] != 0 && x != ROWS - 1) {
						shift_value_right(x, y);
					}
				}
			}
			add_new_two();
			break;
		case Ember::EmberKeyCode::UpArrow:
			for (int x = 0; x < ROWS; x++) {
				for (int y = 0; y < COLS; y++) {
					if (squares[x][y] != 0 && y != 0) {
						shift_value_up(x, y);
					}
				}
			}
			add_new_two();
			break;
		case Ember::EmberKeyCode::DownArrow:
			for (int x = 0; x < ROWS; x++) {
				for (int y = COLS - 1; y >= 0; y--) {
					if (squares[x][y] != 0 && y != COLS - 1) {
						shift_value_down(x, y);
					}
				}
			}
			add_new_two();
			break;
		case Ember::EmberKeyCode::R:
			init();
			break;
		default:
			break;
		}
	}

	void UserDefEvent(Ember::Event& event) {
		Ember::EventDispatcher dispatch(&event);
		dispatch.Dispatch<Ember::KeyboardEvents>(EMBER_BIND_FUNC(KeyboardEvent));
	}
private:
	Ember::Color background_color = { 240, 240, 240, 255 };

	std::unordered_map<int, Ember::Color> color_mapper;
	Ember::Font text;
	int squares[COLS][ROWS];

	int score = 0;
	bool game_over = false;
};

int main(int argc, char** argv) {
	Game2048 game2048;
	game2048.Initialize("2048", GAMEBOARD_OFFSET.x + (ROWS * SQ_SIZE) + (SQ_PADDING * 3), GAMEBOARD_OFFSET.y + (ROWS * SQ_SIZE) + (SQ_PADDING * 3));

	game2048.Run();

	return 0;
}