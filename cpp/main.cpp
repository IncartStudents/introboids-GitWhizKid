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

class CustomImGui : public UseImGui {
public:
	virtual void Update() override {

		// render your GUI
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("CustomImGui Hello, world!");              // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f

		bool clear_color_changed = ImGui::ColorEdit3("clear color", (float*)clear_color); // Edit 3 floats representing a color

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		ImGui::Button("Rhythm1");
		ImGui::Text("display size: %d, %d", H, W);

        static int e = 0;
		ImGui::RadioButton("Привет!", &e, 0); // ImGui::SameLine();
		//ImGui::RadioButton(u8"Привет!", &e, 1); // ImGui::SameLine();
		ImGui::RadioButton("radio c", &e, 2);

		if (ImGui::TreeNode("Дерево"))
		{
			// Using the generic BeginListBox() API, you have full control over how to display the combo contents.
			// (your selection data could be an index, a pointer to the object, an id for the object, a flag intrusively
			// stored in the object itself, etc.)
			const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK", "LLLLLLL", "MMMM", "OOOOOOO" };
			static int item_current_idx = 0; // Here we store our selection data as an index.
			if (ImGui::BeginListBox("listbox 1"))
			{
				for (int n = 0; n < IM_ARRAYSIZE(items); n++)
				{
					const bool is_selected = (item_current_idx == n);
					if (ImGui::Selectable(items[n], is_selected))
						item_current_idx = n;

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndListBox();
			}

			// Custom size: use all width, 5 items tall
			ImGui::Text("Full-width:");
			if (ImGui::BeginListBox("##listbox 2", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
			{
				for (int n = 0; n < IM_ARRAYSIZE(items); n++)
				{
					const bool is_selected = (item_current_idx == n);
					if (ImGui::Selectable(items[n], is_selected))
						item_current_idx = n;

					// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndListBox();
			}

			ImGui::TreePop();
		}

		ImGui::End();


		if (clear_color_changed) {
			change_clear_color(clear_color[0], clear_color[1], clear_color[2]);
		}

	}

private:
	float clear_color[3] = { .0f, .0f, .0f };
	void change_clear_color(float r, float g, float b) {
		glClearColor(r, g, b, 1.00f);
	}
};

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

    Boid(float x, float y) : max_speed(2.0f) { 
        position = { x, y };
        velocity = { static_cast<float>(rand() % 100) / 100 - 0.5f, static_cast<float>(rand() % 100) / 100 - 0.5f };
    }

    void update(const std::vector<Boid>& boids) {
        Vec2 alignment = { 0, 0 };
        Vec2 cohesion = { 0, 0 };
        Vec2 separation = { 0, 0 };

        int total = 0;
        for (const Boid& other : boids) {
            float distance = (position - other.position).length();
            if (&other != this && distance < 50) {
                // Согласование
                alignment = alignment + other.velocity;

                // Сближение
                cohesion = cohesion + other.position;

                // Избегание
                separation = separation + (position - other.position) * (1.0f / distance);
                total++;
            }
        }

        if (total > 0) {
            alignment = alignment * (1.0f / total);
            cohesion = (cohesion * (1.0f / total) - position) * 0.01f;
            separation = separation * 0.1f;

            velocity = velocity + alignment + cohesion + separation;
        }

        // Ограничение скорости
        if (velocity.length() > max_speed) {
            velocity = velocity.normalized() * max_speed;
        }

        // Обновляем позицию
        position = position + velocity;
    }
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
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

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

	CustomImGui myimgui;
	myimgui.Init(window, glsl_version);
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT);
		myimgui.NewFrame();
		myimgui.Update();
		myimgui.Render();
		glfwSwapBuffers(window);

	}
	myimgui.Shutdown();

	const int NUM_BOIDS = 100;
	std::vector<Boid> boids;

	for (int i = 0; i < NUM_BOIDS; ++i) {
		boids.emplace_back(rand() % W, rand() % H);
	}

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		glClear(GL_COLOR_BUFFER_BIT);

		// Обновление боидов
		for (Boid& boid : boids) {
			boid.update(boids);
		}

		// Отрисовка боидов
		for (const Boid& boid : boids) {
			glBegin(GL_POINTS);
			glVertex2f(boid.position.x, boid.position.y);
			glEnd();
		}

		myimgui.NewFrame();
		myimgui.Update();
		myimgui.Render();
		glfwSwapBuffers(window);
	}
	return 0;
}
