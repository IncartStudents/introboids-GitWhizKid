// imGUIexample.cpp : Defines the entry point for the application.
//

#include "UseImGui.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <vector>
#include <cmath>
#include <cstdlib>

using namespace std;

int W = 0;
int H = 0;


struct Vec2 {
    float x, y;

    Vec2 operator+(const Vec2& other) const {
        return { x + other.x, y + other.y };
    }

    Vec2 operator-(const Vec2& other) const {
        return { x - other.x, y - other.y };
    }

    Vec2 operator*(float scalar) const {
        return { x * scalar, y * scalar };
    }

    float length() const {
        return std::sqrt(x * x + y * y);
    }

    Vec2 normalized() const {
        float len = length();
        return { x / len, y / len };
    }
};

class Boid {
public:
    Vec2 position;
    Vec2 velocity;
    float max_speed; // Максимальная скорость
    float max_acc; // Максимальное ускорение

    Boid(float x, float y) : max_speed(2.0f), max_acc(1.0f) { 
        position = { x, y };
        velocity = { static_cast<float>(rand() % 100) / 100 - 0.5f, static_cast<float>(rand() % 100) / 100 - 0.5f };
    }

    void update(const std::vector<Boid>& boids);
};

Vec2 updateAlignment(const Boid& boid, const std::vector<Boid>& boids) {
    Vec2 alignment = { 0, 0 };
    int total = 0;

    for (const Boid& other : boids) {
        if (&other != &boid) {
            float distance = (boid.position - other.position).length();
            if (distance < 50) {
                alignment = alignment + other.velocity;
                total++;
            }
        }
    }

    if (total > 0) {
        alignment = (alignment * (1.0f / total)).normalized() * boid.max_speed;
    }

    return alignment;
};

Vec2 updateCohesion(const Boid& boid, const std::vector<Boid>& boids) {
    Vec2 cohesion = { 0, 0 };
    int total = 0;

    for (const Boid& other : boids) {
        if (&other != &boid) {
            float distance = (boid.position - other.position).length();
            if (distance < 50) {
                cohesion = cohesion + other.position;
                total++;
            }
        }
    }

    if (total > 0) {
        cohesion = (cohesion * (1.0f / total) - boid.position) * 0.01f;
    }

    return cohesion;
};

Vec2 updateSeparation(const Boid& boid, const std::vector<Boid>& boids) {
    Vec2 separation = { 0, 0 };
    int total = 0;

    for (const Boid& other : boids) {
        if (&other != &boid) {
            float distance = (boid.position - other.position).length();
            if (distance < 50) {
                separation = separation + (boid.position - other.position) * (1.0f / distance);
                total++;
            }
        }
    }

    if (total > 0) {
        separation = separation * 0.1f;
    }

    return separation;
};

void Boid::update(const std::vector<Boid>& boids) {
    Vec2 alignment = updateAlignment(*this, boids);
    Vec2 cohesion = updateCohesion(*this, boids);
    Vec2 separation = updateSeparation(*this, boids);

    // Обновляем скорость
    Vec2 acc = alignment + cohesion + separation;

    // Ограничение ускорения
    if (acc.length() > max_acc) {
        acc = acc.normalized() * max_acc;
    }

    velocity = velocity + acc;

    // Ограничение скорости
    if (velocity .length() > max_speed) {
        velocity = velocity.normalized() * max_speed;
    }

    // Обновляем позицию
    position = position + velocity;

    // Обработка выхода за границы экрана
    if (position.x < 0) position.x += W;
    if (position.x > W) position.x -= W;
    if (position.y < 0) position.y += H;
    if (position.y > H) position.y -= H;		
};

int main()
{
	// Setup window
	if (!glfwInit())
		return 1;

	// GL 3.0 + GLSL 130
	const char *glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	// Create window with graphics context
	GLFWwindow *window = glfwCreateWindow(1280, 720, "Dear ImGui - Example", NULL, NULL);
	if (window == NULL)
		return 1;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))  // tie window context to glad's opengl funcs
		throw("Unable to context to OpenGL");

	int screen_width, screen_height;
	glfwGetFramebufferSize(window, &screen_width, &screen_height);
	glViewport(0, 0, screen_width, screen_height);	

	auto* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	W = mode->width;
	H = mode->height;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, W, H, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	const int NUM_BOIDS = 100;
	std::vector<Boid> boids;

	for (int i = 0; i < NUM_BOIDS; ++i) {
		boids.emplace_back(rand() % W, rand() % H);
	}

	// Создаем буферы для хранения позиций и скоростей
    std::vector<Vec2> newPositions(NUM_BOIDS);
    std::vector<Vec2> newVelocities(NUM_BOIDS);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		glClear(GL_COLOR_BUFFER_BIT);

    // Применяем правила    
    for (int i = 0; i < NUM_BOIDS; ++i) {
        boids[i].update(boids); // Обновляем состояние боидов
        newPositions[i] = boids[i].position; // Сохраняем новые позиции
        newVelocities[i] = boids[i].velocity; // Сохраняем новые скорости
    }

    // Меняем текущие позиции и скорости на новые
    for (int i = 0; i < NUM_BOIDS; ++i) {
        boids[i].position = newPositions[i];
        boids[i].velocity = newVelocities[i];
    }

		// Отрисовка боидов
		for (const Boid& boid : boids) {
			glBegin(GL_TRIANGLES);
			glVertex2f(boid.position.x, boid.position.y - 5); // Нижняя точка
			glVertex2f(boid.position.x - 5, boid.position.y + 5); // Левый верхний угол
			glVertex2f(boid.position.x + 5, boid.position.y + 5); // Правый верхний угол
			glEnd();
		}
		glfwSwapBuffers(window);
	}
	return 0;
}
