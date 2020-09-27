#pragma once

#include <Dragonfly/editor.h>
#include <Dragonfly/detail/buffer.h> //will be replaced
#include <Dragonfly/detail/vao.h> //will be replaced
#include <Dragonfly/detail/Texture/Texture3D.h>
#include <fstream>
#include "CodeGen/ui.h"
#include "CodeGen/Objects/models.h"

#define DEBUG

class QuadricRender {
public:
	enum SphereTrace {Simple, Relaxed, Enhanced, Quadric};
	struct QuadricArg {
		float delta = 0.008;
		int ray_count = 70;
		float correction = 0.01;
	};
	struct TestArg {
		MyExpr* model;
		long max_frames;
		int max_steps;
		QuadricArg q_arg;
		SphereTrace method = Quadric;
	};

	QuadricRender() :text({ ' ' }) {}
	~QuadricRender();
	void Init(int);
	void Render();

	void SetView(glm::vec3, glm::vec3, glm::vec3);
	std::vector<float> RunErrorTest(TestArg arg);
	double RunSpeedTest(TestArg arg);

private:
	void Preprocess();
	void RenderUI();
	bool Link();
	bool LoadSDF(const char*);
	bool SaveSDF();

	bool hasError = false;
	std::string errorMsg = "";
	glm::ivec3 grid;
	std::vector<char> text;
	SphereTrace trace_method = Quadric;
	QuadricArg quadricArgs;

	df::Texture3D<float> sdfTexture;
	df::Texture3D<glm::vec4> eccentricityTexture;
	df::Texture2D<glm::vec4> frameTexture;

	df::Renderbuffer<df::depth24>* frameBuff;

	df::ShaderProgramEditorVF* program;
	df::ComputeProgramEditor* frameCompProgram;
	df::ComputeProgramEditor* sdfComputeProgram;
	df::ComputeProgramEditor* eccComputeProgram;

	df::Sample sam = df::Sample("Quadric Tracing", 640, 480); //handles Events and such
	df::Camera cam;

	const int bufferSize = 8192;
	int w, h;
	const std::vector<std::pair<std::string, std::string>> examples = { 
		{"Sphere", "Shaders/Examples/default.glsl"},
		{"Ring", "Shaders/Examples/ring.glsl"},
		{"Sphere + Box", "Shaders/Examples/spherebox.glsl"},
		{"Tower", "Shaders/Examples/tower.glsl"},
		{"Blocks", "Shaders/Examples/blocks.glsl"},
		{"Menger Spone", "Shaders/Examples/menger.glsl"},
		{"Spheres", "Shaders/Examples/spheres.glsl"}
	};
	const std::vector<df::detail::_CompShader> trace_path = {
		"Shaders/sphere_trace.glsl"_comp,
		"Shaders/relaxed_sphere_trace.glsl"_comp,
		"Shaders/enhanced_sphere_trace.glsl"_comp,
		"Shaders/quadric_trace.glsl"_comp
	};	

	//codegen
	MyExpr *csg_tree;
};