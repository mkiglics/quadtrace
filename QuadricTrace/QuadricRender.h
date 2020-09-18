#pragma once

#include <Dragonfly/editor.h>
#include <Dragonfly/detail/buffer.h> //will be replaced
#include <Dragonfly/detail/vao.h> //will be replaced
#include <Dragonfly/detail/Texture/Texture3D.h>
#include <fstream>
#include "CodeGen/demo.h"
#include "CodeGen/ui.h"

#define DEBUG

class QuadricRender {
public:
	enum SphereTrace {Simple, Enhanced, Relaxed};
	struct QuadricParam {
		float delta = 0.8;
		int ray_count = 70;
		float correction = 0.01;
	};

	QuadricRender() :text({ ' ' }) {}
	~QuadricRender();
	void Init(int);
	void Render();

	void SetView(glm::vec3, glm::vec3, glm::vec3);
	void RunErrorTest(const char* filename, bool quadric, long max_frames, int max_steps, QuadricParam arg = {}, int scene = 0, SphereTrace sphere_trace_method = Simple);
	void RunSpeedTest(const char* filename, bool quadric, long max_frames, int max_steps, QuadricParam arg = {}, int scene = 0, SphereTrace sphere_trace_method = Simple);

private:
	void Preprocess();
	void RenderUI();
	bool Link();
	bool LoadSDF(const char*);
	bool SaveSDF();
	bool SaveTexture(const char*);

	bool hasError = false;
<<<<<<< HEAD
	std::string errorMsg = "";

=======
	std::string errosMsg = "";
>>>>>>> 0d1eae062da656f5f2a24c900f78bf4039059cc3
	glm::ivec3 grid;
	std::vector<char> text;
	int useQuadricTrace = 1;
	QuadricParam quadricArgs;

	df::Texture3D<float> sdfTexture;
	df::Texture3D<float> eccentricityTexture;
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

	//codegen
	MyExpr *csg_tree;
};