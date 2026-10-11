#include "raylib.h"
#include "raymath.h"
#include <string>
#include <algorithm>
#include <vector>
#include <print>

using uint = uint32_t;

constexpr auto TEXT_SIZE = 40;
constexpr auto MARGIN = 27.0f;
constexpr auto WINDOW_WIDTH = 1080;
constexpr auto WINDOW_HEIGHT = 720;
constexpr auto PADDLE_SPEED = 400.0f;
constexpr auto PADDLE_WIDTH = 14.0f;
constexpr auto PADDLE_HEIGHT = 80.0f;
constexpr auto LAUNCH_VELOCITY = Vector2(180.0f, -350.0f);
constexpr auto BACKGROUND_COLOR = BLACK;
constexpr auto TEXT_COLOR = DARKGRAY;

enum class Side{
	LEFT, RIGHT, TOP, BOTTOM
};

enum class Axis{
	X, Y
};

struct Ball{
	Vector2 position{0.0f, 0.0f};
	Vector2 velocity{0.0f, 0.0f};
	float radius{8.0f};
	Color color{RAYWHITE};

	Ball(){
		reset();
	}

	void nudge(Axis axis, float distance){
		if(axis == Axis::X){
			position.x += distance;
		} else{
			position.y += distance;
		}
	}

	void bounce(Axis axis){
		if(axis == Axis::X){
			velocity.x *= -1.0f;
		} else{
			velocity.y *= -1.0f;
		}
	}

	void update(float dt){
		position.x += velocity.x * dt;
		position.y += velocity.y * dt;
	}

	void render() const{
		DrawCircleV(position, radius, color);
	}

	void reset(){
		position = Vector2{WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f};
		velocity = Vector2{0.0f, 0.0f};
	}

	Rectangle get_rectangle() const{
		return Rectangle{position.x - radius, position.y - radius, radius * 2, radius * 2};
	}

	bool is_still() const{
		return Vector2Length(velocity) < 0.01f;
	}

	void launch(){
		velocity = LAUNCH_VELOCITY;
	}

	void speed_up(Axis axis, float delta){
		if(axis == Axis::X){
			if(velocity.x > 1000.0f || velocity.x < -1000.0f){
				return;
			}

			if(velocity.x > 0){
				velocity.x += delta;
			} else {
				velocity.x -= delta;
			}
		} else {
			if(velocity.y > 600.0f || velocity.y < -600.0f){
				return;
			}

			if(velocity.y > 0){
				velocity.y += delta;
			} else {
				velocity.y -= delta;
			}
		}
	}

	void do_collision_with(Rectangle r){
		if(!CheckCollisionRecs(get_rectangle(), r)){
			return;
		}

		Rectangle overlap = GetCollisionRec(get_rectangle(), r);
		bool sidehit = overlap.width < overlap.height;
		if(sidehit){
			float dx = (velocity.x > 0.0f) ? -overlap.width : overlap.width;
			bounce(Axis::X);
			nudge(Axis::X, dx);
			speed_up(Axis::X, 40.0f);
		} else{
			float dy = (velocity.y > 0.0f) ? -overlap.height : overlap.height;
			nudge(Axis::Y, dy);
			bounce(Axis::Y);
			speed_up(Axis::Y, 40.0f);
		}
	}
};

class Paddle{
	Rectangle rectangle{0.0f, 0.0f, PADDLE_WIDTH, PADDLE_HEIGHT};
	Color color{RAYWHITE};
	Side side{Side::LEFT};
	uint lives = 5;
	uint score = 0;

public:
	Paddle(Side position){
		side = position;
		if(side == Side::LEFT){
			reset_position(MARGIN, (WINDOW_HEIGHT - PADDLE_HEIGHT) / 2.0f);
		}
		if(side == Side::RIGHT){
			reset_position(WINDOW_WIDTH - PADDLE_WIDTH - MARGIN, (WINDOW_HEIGHT - PADDLE_HEIGHT) / 2.0f);
		}
	}

	void update(float dt){
		if(side == Side::LEFT || side == Side::RIGHT){
			if(IsKeyDown(KEY_UP)){
				rectangle.y -= PADDLE_SPEED * dt;
			}
			if(IsKeyDown(KEY_DOWN)){
				rectangle.y += PADDLE_SPEED * dt;
			}
			const auto MIN_Y = 0.0f;
			const auto MAX_Y = WINDOW_HEIGHT - rectangle.height;
			rectangle.y = std::clamp(rectangle.y, MIN_Y, MAX_Y);
		}
	}

	void render() const{
		DrawRectangleRec(rectangle, color);
	}

	uint get_score() const{
		return score;
	}

	uint get_lives() const{
		return lives;
	}

	void lose() {
		if(lives > 0){
			lives--;
		}
	}

	void score_up() {
		score++;
	}

	void reset(){
		lives = 5;
		score = 0;
	}

	Rectangle get_rectangle() const{
		return rectangle;
	}

	void reset_position(float x, float y){
		rectangle.y = y;
		rectangle.x = x;
	}
};

enum class GameState {
	START_SCREEN,
	PLAYING,
	ROUND_OVER,
	GAME_OVER
};

class Game{
	Paddle paddle_0{Side::LEFT};
	Paddle paddle_1{Side::RIGHT};
	Ball ball;
	GameState state = GameState::START_SCREEN;

	void update(float dt){
		switch (state) {
			case GameState::START_SCREEN:
				if(IsKeyPressed(KEY_SPACE)){
					state = GameState::PLAYING;
					paddle_0.reset();
					paddle_1.reset();
					ball.reset();
					ball.launch();
				}
				break;
			case GameState::PLAYING:
				paddle_0.update(dt);
				paddle_1.update(dt);

				ball.update(dt);
				ball.do_collision_with(paddle_0.get_rectangle());
				ball.do_collision_with(paddle_1.get_rectangle());

				Side side;
				if(check_bounds_for(ball, side)) {
					bool scored = false;
					switch (side) {
						case Side::LEFT:
							paddle_0.lose();
							paddle_1.score_up();
							scored = true;
							break;
						case Side::RIGHT:
							paddle_0.score_up();
							paddle_1.lose();
							scored = true;
							break;
					}

					if(scored) {
						if(is_game_over()){
							state = GameState::GAME_OVER;
						} else {
							state = GameState::ROUND_OVER;
						}
					}
				}
				break;
			case GameState::ROUND_OVER:
				break;
			case GameState::GAME_OVER:
				break;
		}

	}

	void render() const{
		BeginDrawing();
		ClearBackground(BACKGROUND_COLOR);
		paddle_0.render();
		paddle_1.render();
		ball.render();
		if(ball.is_still()){
			if(paddle_0.get_lives() == 0) {
				DrawText("Player 1 WIN! Press SPACE to restart...", (int)MARGIN+TEXT_SIZE, WINDOW_HEIGHT - TEXT_SIZE, TEXT_SIZE, TEXT_COLOR);
			} else if (paddle_1.get_lives() == 0){
				DrawText("Player 0 WIN! Press SPACE to restart...", (int)MARGIN+TEXT_SIZE, WINDOW_HEIGHT - TEXT_SIZE, TEXT_SIZE, TEXT_COLOR);
			} else {
				DrawText("Press SPACE to serve!", (int)MARGIN+TEXT_SIZE, WINDOW_HEIGHT - TEXT_SIZE, TEXT_SIZE, TEXT_COLOR);
			}
		} else {
			DrawText(("Player 0: (Lives: " + std::to_string(paddle_0.get_lives()) + "; Score: " + std::to_string(paddle_0.get_score()) + ")").c_str(), (int)MARGIN+TEXT_SIZE, TEXT_SIZE, TEXT_SIZE/2, TEXT_COLOR);
			DrawText(("Player 1: (Lives: " + std::to_string(paddle_1.get_lives()) + "; Score: " + std::to_string(paddle_1.get_score()) + ")").c_str(), (int)MARGIN+TEXT_SIZE, TEXT_SIZE*2, TEXT_SIZE/2, TEXT_COLOR);
		}
		EndDrawing();
	}

	bool is_game_over(){
		return paddle_0.get_lives() == 0 || paddle_1.get_lives() == 0;
	}

	bool check_bounds_for(Ball& b, Side& side){
		auto& pos = b.position; //create a local reference to the Ball::pos member, for convenience
		const auto MAX_X = WINDOW_WIDTH - b.radius;
		const auto MAX_Y = WINDOW_HEIGHT - b.radius;

		if(pos.x < b.radius){
			pos.x = b.radius;
			b.bounce(Axis::X);
			side = Side::LEFT;
			return true;
		}

		if(pos.x > MAX_X){
			pos.x = MAX_X;
			b.bounce(Axis::X);
			side = Side::RIGHT;
			return true;
		}

		if(pos.y < b.radius){
			pos.y = b.radius;
			b.bounce(Axis::Y);
			side = Side::TOP;
			return true;
		}

		if(pos.y > MAX_Y){
			pos.y = MAX_Y;
			b.bounce(Axis::Y);
			side = Side::BOTTOM;
			return true;
		}

		return false;
	}

public:
	Game(std::string title, int width, int height, int fps = 60){
		InitWindow(width, height, title.c_str());
		SetTargetFPS(fps);
	}

	~Game(){
		CloseWindow();
	}

	void run(){
		while(!WindowShouldClose()){
			auto dt = GetFrameTime();
			update(dt);
			render();
		}
	}
};

int main(){
	Game game("Pong!", WINDOW_WIDTH, WINDOW_HEIGHT);
	std::println("Game is running");
	game.run();
	return 0;
}
