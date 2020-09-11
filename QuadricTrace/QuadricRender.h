#pragma once

#include <Dragonfly/editor.h>
#include <Dragonfly/detail/buffer.h> //will be replaced
#include <Dragonfly/detail/vao.h> //will be replaced
#include <Dragonfly/detail/Texture/Texture3D.h>
#include <fstream>

#define DEBUG

class QuadricRender {
public:
	QuadricRender() :text({ ' ' }) {}
	~QuadricRender();
	void Init(int);
	void Render();
private:
	void Preprocess();
	void RenderUI();
	bool Link();
	bool LoadSDF(const char*);
	bool SaveSDF();

	bool hasError = false;
	std::string errosMsg = "";

	glm::ivec3 grid;
	std::vector<char> text;
	int useQuadricTrace = 1;

	df::Texture3D<float> sdfTexture;
	df::Texture3D<float> eccentricityTexture;

	df::ShaderProgramEditorVF* program;
	df::ComputeProgramEditor* sdfComputeProgram;
	df::ComputeProgramEditor* eccComputeProgram;

	df::Sample sam = df::Sample("Quadric Tracing", 640, 480, 0); //handles Events and such
	df::Camera cam;

	const int bufferSize = 8192;
	const std::vector<std::pair<std::string, std::string>> examples = { 
		{"Sphere", "Shaders/Examples/default.glsl"},
		{"Ring", "Shaders/Examples/ring.glsl"},
		{"Sphere + Box", "Shaders/Examples/spherebox.glsl"},
		{"Tower", "Shaders/Examples/tower.glsl"},
		{"Blocks", "Shaders/Examples/blocks.glsl"}
	};
};