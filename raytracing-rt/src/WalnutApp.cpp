#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Camera.h"
#include "Renderer.h"

#include <glm/gtc/type_ptr.hpp>

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:

	ExampleLayer()
		: m_Camera(45.0f, 0.1f, 100.0f) 
	{
		//m_Scene.Spheres.push_back(Sphere{ {0.5f, 0.0f, 0.0f}, 0.5f, {1.0f, 1.0f, 0.0f} });

		Sphere sphere;
		sphere.Albedo = { 0.0f, 1.0f, 1.0f };
		sphere.Position = { -1.0f, 0.0f, 0.0f };
		sphere.Radius = 0.8;

		m_Scene.AddSphere(sphere);

		m_Scene.AddSphere({ 0.5f, 0.0f, 0.0f }, 0.5f, { 1.0f, 1.0f, 0.0f });
	}

	virtual void OnUpdate(float ts) override
	{
		m_Camera.OnUpdate(ts);
	}

	virtual void OnUIRender() override
	{
		// Settings
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		if (ImGui::Button("Render")) {
			Render();
		}
		ImGui::End();


		// Scene
		ImGui::Begin("Scene");
		for (size_t i = 0; i < m_Scene.Spheres.size(); ++i)
		{
			// too distinguish the items
			ImGui::PushID(i);

			// sphere parameters
			Sphere& sphere = m_Scene.Spheres[i];
			ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.1f);
			ImGui::DragFloat("Radius", &sphere.Radius, 0.1f);
			ImGui::ColorEdit3("Albedo", glm::value_ptr(sphere.Albedo));
			
			// remove button
			if (ImGui::Button("Remove")) {
				m_Scene.Spheres.erase(m_Scene.Spheres.begin() + i);
			}
			
			// separator between each spheres
			ImGui::Separator();

			ImGui::PopID();
		}
		ImGui::End();


		// Add Sphere
		ImGui::Begin("Add Sphere");

		// sphere parameters
		ImGui::DragFloat3("Position", glm::value_ptr(PreviewSphere.Position), 0.1f);
		ImGui::DragFloat("Radius", &PreviewSphere.Radius, 0.1f);
		ImGui::ColorEdit3("Albedo", glm::value_ptr(PreviewSphere.Albedo));
		if (ImGui::Button("Add")) {
			m_Scene.AddSphere(PreviewSphere.Position, PreviewSphere.Radius, PreviewSphere.Albedo);
		}
		
		ImGui::End();


		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		m_ViewportWidth = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;
		
		auto image = m_Renderer.GetFinalImage();
		
		if (image) {
			ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth(), (float)image->GetHeight() },
				ImVec2(0, 1), ImVec2(1, 0)); // flip image
		}

		ImGui::End();
		ImGui::PopStyleVar();
		
		Render();
	}


	void Render() {

		Timer timer;

		m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Renderer.Render(m_Scene, m_Camera);

		m_LastRenderTime = timer.ElapsedMillis();
	}

private:
	Scene m_Scene;
	Sphere PreviewSphere;
	Camera m_Camera;
	Renderer m_Renderer;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
	float m_LastRenderTime = 0.0f;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Real Time RayTracing";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}