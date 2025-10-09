#include "raylib.h"
#include "raymath.h"
#include <string>
#include <algorithm>
#include <vector>

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
		} else{
			float dy = (velocity.y > 0.0f) ? -overlap.height : overlap.height;
			nudge(Axis::Y, dy);
			bounce(Axis::Y);
		}
	}
};

class Paddle{
	Rectangle rectangle{0.0f, 0.0f, PADDLE_WIDTH, PADDLE_HEIGHT};
	Color color{RAYWHITE};
	Side side{Side::LEFT};

public:
	Paddle(Side position){
		side = position;
		if(side == Side::LEFT){
			reset_position(MARGIN, (WINDOW_HEIGHT - PADDLE_HEIGHT) / 2.0f);
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

	Rectangle get_rectangle() const{
		return rectangle;
	}

	void reset_position(float x, float y){
		rectangle.y = y;
		rectangle.x = x;
	}
};

class Game{
	Paddle paddle{Side::LEFT};
	Ball ball;

	void update(float dt){
		if(ball.is_still() && IsKeyPressed(KEY_SPACE)){
			ball.launch();
		}
		paddle.update(dt);
		ball.update(dt);
		check_bounds_for(ball);
		ball.do_collision_with(paddle.get_rectangle());
	}

	void render() const{
		BeginDrawing();
		ClearBackground(BACKGROUND_COLOR);
		paddle.render();
		ball.render();
		if(ball.is_still()){
			DrawText("Press SPACE to serve!", (int)MARGIN+TEXT_SIZE, WINDOW_HEIGHT - TEXT_SIZE, TEXT_SIZE, TEXT_COLOR);
		}
		EndDrawing();
	}

	void check_bounds_for(Ball& b){
		auto& pos = b.position; //create a local reference to the Ball::pos member, for convenience		
		const auto MAX_X = WINDOW_WIDTH - b.radius;
		const auto MAX_Y = WINDOW_HEIGHT - b.radius;
		if(pos.x < b.radius){
			pos.x = b.radius;
			b.bounce(Axis::X);
		} else if(pos.x > MAX_X){
			pos.x = MAX_X;
			b.bounce(Axis::X);
		}
		if(pos.y < b.radius){
			pos.y = b.radius;
			b.bounce(Axis::Y);
		} else if(pos.y > MAX_Y){
			pos.y = MAX_Y;
			b.bounce(Axis::Y);
		}
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
	game.run();
	return 0;
}
