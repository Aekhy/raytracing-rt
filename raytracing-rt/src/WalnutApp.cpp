#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Scene.hpp"
#include "Camera.h"
#include "Renderer.h"

#include <glm/gtc/type_ptr.hpp>

using namespace Walnut;

namespace fs = std::filesystem;

std::vector<std::string> GetCubemapFilenames(const std::string& folderPath) {
	std::vector<std::string> filenames;
	if (fs::exists(folderPath) && fs::is_directory(folderPath)) {
		for (const auto& entry : fs::directory_iterator(folderPath)) {
			if (entry.is_regular_file()) {
				filenames.push_back(entry.path().filename().string());
			}
		}
	}
	return filenames;
}


class ExampleLayer : public Walnut::Layer
{
public:

	ExampleLayer()
		: m_Camera(45.0f, 0.1f, 100.0f) 
	{
		m_Scene.Cubemap.exist = false;
		m_Scene.pass = false;

		m_CubeMapFolderExist = fs::exists("./cubemaps") && fs::is_directory("./cubemaps");
		if (m_CubeMapFolderExist)
			m_CubeMapNames = GetCubemapFilenames("./cubemaps");
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
		mirror.Type = METALLIC;
		mirror.Name = "Mirror";
		mirror.Roughness = 0.04f;
		mirror.Metallic = 1.0f;

		Material& emissive = m_Scene.Materials.emplace_back();
		emissive.Name = "Emissive";
		emissive.Albedo = { 0.8f, 0.5f, 0.2f };
		emissive.Roughness = 0.1f;
		emissive.EmissionColor = emissive.Albedo;
		emissive.EmissionPower = 2.0f;

		Material& glass = m_Scene.Materials.emplace_back();
		glass.Type = DIELECTRIC;
		glass.Name = "Glass";
		glass.IndiceOut = 1.0f;
		glass.IndiceIn = 1.5f;
		/*
		// Spheres
		
		// pink sphere
		Sphere sphere;
		sphere.MaterialIndex = 3;
		sphere.Position = { 0.0f, -15.2f, 0.0f };
		sphere.Radius = 15.0f;
		m_Scene.AddSphere(sphere);

		// blue sphere
		m_Scene.AddSphere({ 0.0f, 0.5f, 0.0f }, 0.5f, 2);

		// emssive
		m_Scene.AddSphere({ 1.0f, 0.5f, 0.0f }, 0.5f, 4);
		*/

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

		ImGui::Checkbox("Real Time", &m_RealTime);
		ImGui::Checkbox("Accumulate", &m_Renderer.GetSettings().Accumulate);
		ImGui::Checkbox("Antialiasing", &m_Renderer.GetSettings().Antialiasing);
		ImGui::SliderInt("Monter Carlo nb sample", &m_Renderer.GetSettings().MonteCarloNbSample, 1, 2048);
		ImGui::Text("Nb frame: %i", m_Renderer.GetFrameIndex());

		ShouldResetFrame |= ImGui::Button("Reset");
		
		ImGui::End();


		// Scene
		ImGui::Begin("Scene");

		// Create a collapsible header for the spheres category
		if (ImGui::CollapsingHeader("Spheres")) {
			for (size_t i = 0; i < m_Scene.Spheres.size(); ++i) {
				// Push a unique identifier for each sphere
				ImGui::PushID(static_cast<int>(i));

				// Begin a new tree node for each sphere
				if (ImGui::TreeNode(("Sphere " + std::to_string(i)).c_str())) {
					Sphere& sphere = m_Scene.Spheres[i];
					ShouldResetFrame |= ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.1f);
					ShouldResetFrame |= ImGui::DragFloat("Radius", &sphere.Radius, 0.1f, 0.0f, 100.0f);
					ShouldResetFrame |= ImGui::SliderInt("Material", &sphere.MaterialIndex, 0, (int)m_Scene.Materials.size() - 1);

					// Remove button
					if (ImGui::Button("Remove")) {
						m_Scene.Spheres.erase(m_Scene.Spheres.begin() + i);
						ImGui::PopID(); // Pop the unique identifier
						break; // Exit loop since we modified the vector
					}

					// End the tree node for the current sphere
					ImGui::TreePop();
				}

				// Pop the unique identifier
				ImGui::PopID();
			}
		}

		ImGui::End();



		// load and save scene
		ImGui::Begin("Save and load");

		// Save Scene
		ImGui::InputText("Save File Name", m_SaveFileName, sizeof(m_SaveFileName));
		if (ImGui::Button("Save Scene")) {
			try {
				m_Scene.saveScene(m_SaveFileName);
			}
			catch (const std::exception& e) {
				std::cerr << "Error saving scene: " << e.what() << std::endl;
			}
		}

		ImGui::Separator();

		// Load Scene
		ImGui::InputText("Load File Name", m_LoadFileName, sizeof(m_LoadFileName));
		if (ImGui::Button("Load Scene")) {
			try {
				m_Scene.loadScene(m_LoadFileName);
				ShouldResetFrame = true;
			}
			catch (const std::exception& e) {
				std::cerr << "Error loading scene: " << e.what() << std::endl;
			}
		}

		ImGui::End();


		// Cube map
		static int selectedFileIndex = -1; // No file selected initially

		ImGui::Begin("Cube Maps");


		// Add a button to refresh the list of cubemap filenames
		if (ImGui::Button("Refresh")) {
			m_CubeMapFolderExist = fs::exists("./cubemaps") && fs::is_directory("./cubemaps");
			if (m_CubeMapFolderExist) {
				m_CubeMapNames = GetCubemapFilenames("./cubemap");
				selectedFileIndex = -1; // Reset the selected file index
			}
			else {
				m_CubeMapNames.clear();
			}
		}

		if (m_CubeMapFolderExist) {
			// Display the filenames in a combo box
			if (ImGui::BeginCombo("Cube map", selectedFileIndex >= 0 ? m_CubeMapNames[selectedFileIndex].c_str() : "Select a file")) {
				for (int i = 0; i < m_CubeMapNames.size(); ++i) {
					bool isSelected = (selectedFileIndex == i);
					if (ImGui::Selectable(m_CubeMapNames[i].c_str(), isSelected)) {
						selectedFileIndex = i;
					}
					if (isSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			// Add a button to load the selected cubemap
			if (ImGui::Button("Load cube map")  && selectedFileIndex >= 0) {
				std::string selectedFilename = m_CubeMapNames[selectedFileIndex];
				const char* filename = selectedFilename.c_str();
				m_Scene.loadCubemap(filename);
			}
		}
		else {
			ImGui::Text("Cubemap folder does not exist.");
		}

		ImGui::End();


		// Materials
		ImGui::Begin("Materials");

		for (size_t i = 0; i < m_Scene.Materials.size(); ++i) {
			Material& material = m_Scene.Materials[i];

			// Push a unique identifier for each collapsible header
			ImGui::PushID(static_cast<int>(i));

			// Create a collapsible header for each material using its index and name
			char headerLabel[64];
			snprintf(headerLabel, 64, "%zu - %s", i, material.Name);
			if (ImGui::CollapsingHeader(headerLabel)) {
				ShouldResetFrame |= ImGui::ColorEdit3("Albedo", glm::value_ptr(material.Albedo));
				ShouldResetFrame |= ImGui::DragFloat("Roughness", &material.Roughness, 0.01f, 0.0f, 1.0f);
				ShouldResetFrame |= ImGui::DragFloat("Metallic", &material.Metallic, 0.01f, 0.0f, 1.0f);
				ShouldResetFrame |= ImGui::ColorEdit3("Emssion color", glm::value_ptr(material.EmissionColor));
				ShouldResetFrame |= ImGui::DragFloat("Emission power", &material.EmissionPower, 0.01f, 0.0f, FLT_MAX);
				ShouldResetFrame |= ImGui::DragFloat("Indice in", &material.IndiceIn, 0.01f, 0.0f, 2.0f);
				ShouldResetFrame |= ImGui::DragFloat("Indice out", &material.IndiceOut, 0.01f, 0.0f, 2.0f);
				// Remove button
				if (ImGui::Button("Remove")) {
					m_Scene.Materials.erase(m_Scene.Materials.begin() + i);
					break; // Exit loop since we modified the vector
				}
			}

			// Pop the unique identifier
			ImGui::PopID();
		}

		ImGui::End();




		// Add Sphere
		ImGui::Begin("Add Sphere");

		// sphere parameters
		ImGui::DragFloat3("Position", glm::value_ptr(PreviewSphere.Position), 0.1f);
		ImGui::DragFloat("Radius", &PreviewSphere.Radius, 0.1f, 0.0f, 100.0f);
		ImGui::SliderInt("Material", &PreviewSphere.MaterialIndex, 0, (int)m_Scene.Materials.size() - 1);
		if (ImGui::Button("Add")) {
			m_Scene.AddSphere(PreviewSphere.Position, PreviewSphere.Radius, PreviewSphere.MaterialIndex);
			ShouldResetFrame = true;
		}
		
		ImGui::End();

		// Add Material
		ImGui::Begin("Add Material");

		// material parameters
		ImGui::InputText("Material name", m_BaseInput, sizeof(m_BaseInput));
		ImGui::DragInt("Material type", reinterpret_cast<int*>(&PreviewMaterial.Type), 1.0f, DIFFUSE, DIELECTRIC);
		ImGui::ColorEdit3("Albedo", glm::value_ptr(PreviewMaterial.Albedo));
		ImGui::DragFloat("Roughness", &PreviewMaterial.Roughness, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat("Metallic", &PreviewMaterial.Metallic, 0.01f, 0.0f, 1.0f);
		ImGui::ColorEdit3("Emssion color", glm::value_ptr(PreviewMaterial.EmissionColor));
		ImGui::DragFloat("Emission power", &PreviewMaterial.EmissionPower, 0.01f, 0.0f, FLT_MAX);
		ImGui::DragFloat("Indice in", &PreviewMaterial.IndiceIn, 0.01f, 0.0f, 2.0f);
		ImGui::DragFloat("Indice out", &PreviewMaterial.IndiceOut, 0.01f, 0.0f, 2.0f);
		if (ImGui::Button("Add")) {
			m_Scene.AddMaterial(m_BaseInput,
				PreviewMaterial.Albedo,
				PreviewMaterial.Roughness,
				PreviewMaterial.Metallic,
				PreviewMaterial.EmissionColor,
				PreviewMaterial.EmissionPower,
				PreviewMaterial.Type,
				PreviewMaterial.IndiceOut,
				PreviewMaterial.IndiceIn);
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


		if (m_RealTime)
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
	Material PreviewMaterial;
	Camera m_Camera;
	Renderer m_Renderer;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
	float m_LastRenderTime = 0.0f;
	bool m_RealTime = true;
	char m_BaseInput[101] = "100 char name";
	char m_SaveFileName[256] = "scene.json"; // Default file name for saving
	char m_LoadFileName[256] = "scene.json"; // Default file name for loading

	bool m_CubeMapFolderExist;
	std::vector<std::string> m_CubeMapNames;
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