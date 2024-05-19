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
		// Materials
		Material& whiteMatte = m_Scene.Materials.emplace_back();
		whiteMatte.Name = "White Matte";
		whiteMatte.Roughness = 0.8f;

		Material& pinkMatte = m_Scene.Materials.emplace_back();
		pinkMatte.Name = "Pink Matte";
		pinkMatte.Roughness = 0.8f;
		pinkMatte.Albedo = { 1.0f, 0.0f, 1.0f };

		Material& blueMatte = m_Scene.Materials.emplace_back();
		blueMatte.Name = "Blue Matte";
		blueMatte.Roughness = 0.8f;
		blueMatte.Albedo = { 0.0f, 1.0f, 1.0f };

		Material& mirror = m_Scene.Materials.emplace_back();
		mirror.Name = "Mirror";
		mirror.Albedo = { 0.0f, 0.0f, 0.0f };
		mirror.Roughness = 0.04f;
		mirror.Metallic = 1.0f;


		// Spheres
		
		// pink sphere
		Sphere sphere;
		sphere.MaterialIndex = 3;
		sphere.Position = { 0.0f, -15.2f, 0.0f };
		sphere.Radius = 15.0f;
		m_Scene.AddSphere(sphere);

		// blue sphere
		m_Scene.AddSphere({ 0.0f, 0.5f, 0.0f }, 0.5f, 2);
	}

	virtual void OnUpdate(float ts) override
	{
		if (m_Camera.OnUpdate(ts))
			m_Renderer.ResetFrameIndex();
	}

	virtual void OnUIRender() override
	{
		bool ShouldResetFrame = false;

		// Settings
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		if (ImGui::Button("Render")) {
			Render();
		}

		ImGui::Checkbox("Accumulate", &m_Renderer.GetSettings().Accumulate);


		ShouldResetFrame |= ImGui::Button("Reset");
		
		ImGui::End();


		// Scene
		ImGui::Begin("Scene");
		for (size_t i = 0; i < m_Scene.Spheres.size(); ++i)
		{
			// too distinguish the items
			ImGui::PushID(i);

			// sphere parameters
			Sphere& sphere = m_Scene.Spheres[i];
			ShouldResetFrame |= ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.1f);
			ShouldResetFrame |= ImGui::DragFloat("Radius", &sphere.Radius, 0.1f, 0.0f, 100.0f);
			ShouldResetFrame |= ImGui::SliderInt("Material", &sphere.MaterialIndex, 0, (int)m_Scene.Materials.size() - 1);

			// remove button
			if (ImGui::Button("Remove")) {
				m_Scene.Spheres.erase(m_Scene.Spheres.begin() + i);
				ShouldResetFrame = true;
			}
		
			// separator between each spheres
			ImGui::Separator();

			ImGui::PopID();
		}
		ImGui::End();


		// Materials
		ImGui::Begin("Materials");
		for (size_t i = 0; i < m_Scene.Materials.size(); ++i)
		{
			// too distinguish the items
			ImGui::PushID(i);

			// materials parameters
			Material& material = m_Scene.Materials[i];
			ImGui::Text("Name: %s", material.Name);
			ImGui::ColorEdit3("Albedo", glm::value_ptr(material.Albedo));
			ImGui::DragFloat("Roughness", &material.Roughness, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Metallic", &material.Metallic, 0.01f, 0.0f, 1.0f);

			// remove button
			if (ImGui::Button("Remove")) {
				m_Scene.Materials.erase(m_Scene.Materials.begin() + i);
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
		ImGui::DragFloat("Radius", &PreviewSphere.Radius, 0.1f, 0.0f, 100.0f);
		ImGui::DragInt("Material", &PreviewSphere.MaterialIndex, 1.0f, 0, (int)m_Scene.Materials.size() - 1);
		if (ImGui::Button("Add")) {
			m_Scene.AddSphere(PreviewSphere.Position, PreviewSphere.Radius, PreviewSphere.MaterialIndex);
			ShouldResetFrame = true;
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
		

		if (ShouldResetFrame)
			m_Renderer.ResetFrameIndex();


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