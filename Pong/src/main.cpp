#include "raylib.h"
#include "raymath.h"
#include <string>
#include <algorithm>
#include <vector>
#include <memory>
#include <print>
#include <functional>

using uint = uint32_t;

constexpr auto TEXT_SIZE = 40;
constexpr auto MARGIN = 27.0f;
constexpr auto WINDOW_WIDTH = 1080;
constexpr auto WINDOW_HEIGHT = 720;
constexpr auto PADDLE_SPEED = 400.0f;
constexpr auto PADDLE_WIDTH = 14.0f;
constexpr auto BALL_MAX_SPEED = 1500.0f;
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

	void launch(){
		velocity = LAUNCH_VELOCITY;
	}

	void speed_up(Axis axis, float delta){
		if(axis == Axis::X){
			if(velocity.x > 0){
				velocity.x += delta;
			} else {
				velocity.x -= delta;
			}
		} else {
			if(velocity.y > 0){
				velocity.y += delta;
			} else {
				velocity.y -= delta;
			}
		}

		float capped_speed = std::clamp(Vector2Length(velocity), 0.0f, BALL_MAX_SPEED);
		float angle = std::atan2(velocity.y, velocity.x);

		velocity.x = std::cos(angle)*capped_speed;
		velocity.y = std::sin(angle)*capped_speed;
	}

	bool do_collision_with(Rectangle r){
		if(!CheckCollisionRecs(get_rectangle(), r)){
			return false;
		}

		Rectangle overlap = GetCollisionRec(get_rectangle(), r);
		bool sidehit = overlap.width < overlap.height;
		if(sidehit){
			float dx = (velocity.x > 0.0f) ? -overlap.width : overlap.width;
			bounce(Axis::X);
			nudge(Axis::X, dx);
			speed_up(Axis::X, 40.0f);
		} else {
			float dy = (velocity.y > 0.0f) ? -overlap.height : overlap.height;
			nudge(Axis::Y, dy);
			bounce(Axis::Y);
			speed_up(Axis::Y, 40.0f);
		}

		return true;
	}
};

class Paddle;

using Controller = std::function<int(const Paddle&, const std::vector<Ball>)>;

class Paddle{
	Rectangle rectangle;
	Color color{RAYWHITE};
	Side side{Side::LEFT};
	Controller controller;

public:
	Paddle(Side position): side{position}{
		if(side == Side::LEFT || side == Side::RIGHT){
			rectangle = {0.0f, 0.0f, PADDLE_WIDTH, PADDLE_HEIGHT};
		} else {
			rectangle = {0.0f, 0.0f, PADDLE_HEIGHT, PADDLE_WIDTH};
		}
		reset();
	}

	void update(std::vector<Ball> balls, float dt){
		if(side == Side::LEFT || side == Side::RIGHT){
			switch(controller(*this, balls)){
				case -1:
					rectangle.y += PADDLE_SPEED * dt;
					break;
				case 1:
					rectangle.y -= PADDLE_SPEED * dt;
					break;
			}
			const auto MIN_Y = 0.0f;
			const auto MAX_Y = WINDOW_HEIGHT - rectangle.height;
			rectangle.y = std::clamp(rectangle.y, MIN_Y, MAX_Y);
		} else {
			switch(controller(*this, balls)){
				case -1:
					rectangle.x += PADDLE_SPEED * dt;
					break;
				case 1:
					rectangle.x -= PADDLE_SPEED * dt;
					break;
			}
			const auto MIN_X = 0.0f;
			const auto MAX_X = WINDOW_WIDTH - rectangle.width;
			rectangle.x = std::clamp(rectangle.x, MIN_X, MAX_X);
		}
	}

	void render() const{
		DrawRectangleRec(rectangle, color);
	}

	void set_controller(Controller new_controller){
		controller = new_controller;
	}

	void reset(){
		switch(side){
			case Side::LEFT:
				reset_position(MARGIN, (WINDOW_HEIGHT - PADDLE_HEIGHT) / 2.0f);
				break;
			case Side::RIGHT:
				reset_position(WINDOW_WIDTH - PADDLE_WIDTH - MARGIN, (WINDOW_HEIGHT - PADDLE_HEIGHT) / 2.0f);
				break;
			case Side::TOP:
				reset_position((WINDOW_WIDTH - PADDLE_HEIGHT) / 2.0f, MARGIN);
				break;
			case Side::BOTTOM:
				reset_position((WINDOW_WIDTH - PADDLE_HEIGHT) / 2.0f, WINDOW_HEIGHT - MARGIN - PADDLE_WIDTH);
				break;
		}
	}

	Rectangle get_rectangle() const{
		return rectangle;
	}

	Side get_side() const{
		return side;
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

class Scoreboard {
	std::vector<uint> lives;
	std::vector<uint> scores;

public:
	void reset(uint player_count){
		lives.assign(player_count, 5);
		scores.assign(player_count, 0);
	}

	void score_up(uint loser){
		if(lives[loser] == 0){
			return;
		}

		lives[loser]--;
		for(uint i = 0; i < scores.size(); i++){
			if(i != loser && lives[i] > 0){
				scores[i]++;
			}
		}
	}

	bool is_dead(uint index) const{
		return lives[index] == 0;
	}

	uint alive_count() const{
		return (uint)std::count_if(lives.begin(), lives.end(), [](uint l){ return l > 0; });
	}

	bool is_game_over() const{
		return alive_count() <= 1;
	}

	void render() const{
		if(!is_game_over()){
			for(uint i = 0; i < lives.size(); i++){
				std::string text = "Player " + std::to_string(i) + ": (lives: " + std::to_string(lives[i]) + "; Score: " + std::to_string(scores[i]) + ")";
				DrawText(text.c_str(), (int)MARGIN+TEXT_SIZE, TEXT_SIZE*(i+1), TEXT_SIZE/2, TEXT_COLOR);
			}
			return;
		}

		for(uint i = 0; i < lives.size(); i++){
			if(lives[i] > 0){
				std::string text = "Player " + std::to_string(i) + " WIN! Press SPACE to restart...";
				DrawText(text.c_str(), (int)MARGIN+TEXT_SIZE, WINDOW_HEIGHT - TEXT_SIZE, TEXT_SIZE, TEXT_COLOR);
				return;
			}
		}

		DrawText("Everyone's out! Press SPACE to restart...", (int)MARGIN+TEXT_SIZE, WINDOW_HEIGHT - TEXT_SIZE, TEXT_SIZE, TEXT_COLOR);
	}
};

class Game;

class Screen {
protected:
	Game& game;

public:
	Screen(Game& game): game{game}{}
	virtual ~Screen() = default;

	virtual void update(float dt) = 0;
	virtual void render() const = 0;
};

class StartScreen final : public Screen {
public:
	using Screen::Screen;

	void update(float dt) override;
	void render() const override;
};

class PlayingScreen final : public Screen {
public:
	using Screen::Screen;

	void update(float dt) override;
	void render() const override;
};

class RoundOverScreen final : public Screen {
public:
	using Screen::Screen;

	void update(float dt) override;
	void render() const override;
};

class GameOverScreen final : public Screen {
public:
	using Screen::Screen;

	void update(float dt) override;
	void render() const override;
};

class Game{
	std::vector<Paddle> paddles;
	std::vector<Ball> balls;
	GameState state = GameState::START_SCREEN;
	std::unique_ptr<Screen> screen;
	Scoreboard scoreboard;
	uint paddle_bounces;

	void update(float dt){
		screen->update(dt);
	}

	void render() const{
		BeginDrawing();
		ClearBackground(BACKGROUND_COLOR);
		screen->render();
		EndDrawing();
	}

public:
	Game(std::string title, int width, int height, int fps = 60){
		InitWindow(width, height, title.c_str());
		SetTargetFPS(fps);
		set_state(GameState::START_SCREEN);
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

	void set_state(GameState new_state){
		state = new_state;
		switch (state) {
			case GameState::START_SCREEN:
				screen = std::make_unique<StartScreen>(*this);
				break;
			case GameState::PLAYING:
				screen = std::make_unique<PlayingScreen>(*this);
				break;
			case GameState::ROUND_OVER:
				screen = std::make_unique<RoundOverScreen>(*this);
				break;
			case GameState::GAME_OVER:
				screen = std::make_unique<GameOverScreen>(*this);
				break;
		}
	}

	Scoreboard& get_scoreboard(){
		return scoreboard;
	}

	std::vector<Paddle>& get_paddles() {
		return paddles;
	}

	std::vector<Ball>& get_balls() {
		return balls;
	}

	void paddle_bounced(){
		paddle_bounces++;
		if(paddle_bounces == 5){
			balls.emplace_back();
			Ball& ball = balls.back();
			ball.launch();
			paddle_bounces = 0;
		}
	}

	void reset_balls(){
		paddle_bounces = 0;
		balls.clear();
		balls.emplace_back();
	}

	void serve_all_balls(){
		for(Ball& ball: balls){
			ball.launch();
		}
	}

	void render_balls() const{
		for(const Ball& ball : balls){
			ball.render();
		}
	}

	void render_paddles() const{
		for(uint i = 0; i < paddles.size(); i++){
			if(!scoreboard.is_dead(i)){
				paddles[i].render();
			}
		}
	}
};

void StartScreen::update(float dt){
	if(IsKeyPressed(KEY_SPACE)){
		game.reset_balls();
		game.serve_all_balls();

		std::vector<Paddle>& paddles = game.get_paddles();
		paddles.clear();
		paddles.emplace_back(Side::LEFT);
		paddles.emplace_back(Side::RIGHT);
		paddles.emplace_back(Side::TOP);
		paddles.emplace_back(Side::BOTTOM);

		game.get_scoreboard().reset(paddles.size());

		Controller human_controller = [](const Paddle& paddle, const std::vector<Ball> balls){
			if(paddle.get_side() == Side::LEFT || paddle.get_side() == Side::RIGHT){
				if(IsKeyDown(KEY_UP) && !IsKeyDown(KEY_DOWN)){
					return 1;
				}

				if(!IsKeyPressed(KEY_UP) && IsKeyDown(KEY_DOWN)){
					return -1;
				}
			} else {
				if(IsKeyDown(KEY_LEFT) && !IsKeyDown(KEY_RIGHT)){
					return 1;
				}

				if(!IsKeyPressed(KEY_LEFT) && IsKeyDown(KEY_RIGHT)){
					return -1;
				}
			}

			return 0;
		};

		Controller ai_controller = [](const Paddle& paddle, const std::vector<Ball> balls){
			Rectangle rectangle = paddle.get_rectangle();
			bool vertical = paddle.get_side() == Side::LEFT || paddle.get_side() == Side::RIGHT;

			const Ball* nearest_ball = &balls.front();
			float distance = vertical
				? std::abs(rectangle.x - nearest_ball->position.x)
				: std::abs(rectangle.y - nearest_ball->position.y);
			for (const Ball& ball : balls){
				float new_distance = vertical
					? std::abs(rectangle.x - ball.position.x)
					: std::abs(rectangle.y - ball.position.y);
				if(new_distance < distance){
					distance = new_distance;
					nearest_ball = &ball;
				}
			}

			if(vertical){
				float y = rectangle.y + PADDLE_HEIGHT/2;
				if(y > nearest_ball->position.y){
					return 1;
				}

				if(y < nearest_ball->position.y){
					return -1;
				}
			} else {
				float x = rectangle.x + PADDLE_HEIGHT/2;
				if(x > nearest_ball->position.x){
					return 1;
				}

				if(x < nearest_ball->position.x){
					return -1;
				}
			}

			return 0;
		};

		paddles.front().set_controller(human_controller);
		for(uint i = 1; i < paddles.size(); i++){
			paddles[i].set_controller(ai_controller);
		}

		game.set_state(GameState::PLAYING);
	}
}

void StartScreen::render() const{
	for(Paddle& paddle : game.get_paddles()){
		paddle.render();
	}
	game.render_balls();
	DrawText("Press SPACE to serve!", (int)MARGIN+TEXT_SIZE, WINDOW_HEIGHT - TEXT_SIZE, TEXT_SIZE, TEXT_COLOR);
}

void PlayingScreen::update(float dt){
	std::vector<Paddle>& paddles = game.get_paddles();
	std::vector<Ball>& balls = game.get_balls();

	for(Paddle& paddle : paddles){
		paddle.update(balls, dt);
	}

	bool scored = false;
	uint bounces = 0;
	std::erase_if(
		balls,
		[&](Ball& ball){
			ball.update(dt);
			for(Paddle& paddle : paddles){
				if(ball.do_collision_with(paddle.get_rectangle())){
					bounces++;
				}
			}

			Side side;
			if(game.check_bounds_for(ball, side)) {
				game.get_scoreboard().score_up(static_cast<uint>(side));
				scored = true;
				return true;
			}

			return false;
		}
	);

	for(uint i = 0; i < bounces; i++){
		game.paddle_bounced();
	}

	if(scored) {
		if(game.get_scoreboard().is_game_over()){
			game.set_state(GameState::GAME_OVER);
		} else {
			game.set_state(GameState::ROUND_OVER);
		}
	}
}

void PlayingScreen::render() const{
	game.render_paddles();
	game.render_balls();
	game.get_scoreboard().render();
}

void RoundOverScreen::update(float dt){
	std::vector<Ball>& balls = game.get_balls();
	if(balls.size() == 0){
		balls.emplace_back();
		balls.back().launch();
	}
	game.set_state(GameState::PLAYING);
}

void RoundOverScreen::render() const{
	game.render_paddles();
	game.render_balls();
	game.get_scoreboard().render();
}

void GameOverScreen::update(float dt){
	if(IsKeyPressed(KEY_SPACE)){
		for(Paddle& paddle : game.get_paddles()){
			paddle.reset();
		}
		game.reset_balls();
		game.serve_all_balls();
		game.get_scoreboard().reset(game.get_paddles().size());
		game.set_state(GameState::PLAYING);
	}
}

void GameOverScreen::render() const{
	game.get_scoreboard().render();
}

int main(){
	Game game("Pong!", WINDOW_WIDTH, WINDOW_HEIGHT);
	std::println("Game is running");
	game.run();
	return 0;
}
