#include "glfw_app.hpp"
#include "renderer.hpp"
#include "dialogue_sample.hpp"

#include <memory>
#include <chrono>

std::shared_ptr<Application> g_app;
std::shared_ptr<Renderer> g_renderer;
std::unique_ptr<DialogueSample> g_sample;

int main()
{
	// TODO: input parameters for application window
	g_app = std::make_shared<Application>(1920, 1080, "DiaBolic");
	g_renderer = std::make_shared<Renderer>(g_app);
	g_sample = std::make_unique<DialogueSample>(g_renderer);

	std::chrono::high_resolution_clock::duration deltaTime(0);
	std::chrono::high_resolution_clock::time_point previousFrameTime = std::chrono::high_resolution_clock::now();

	while (!g_app->ShouldClose())
	{
		auto currentFrameTime = std::chrono::high_resolution_clock::now();
		deltaTime = currentFrameTime - previousFrameTime;
		previousFrameTime = currentFrameTime;

		g_app->Update();
		g_sample->Update();
		g_renderer->Update(static_cast<float>(deltaTime.count() * 1e-9), static_cast<GLFWwindow*>(g_app->GetWindow()));
		g_renderer->Render();
	}

	return 0;
}