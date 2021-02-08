#pragma once

#include <Dragonfly/editor.h>
#include <Dragonfly/detail/buffer.h> //will be replaced
#include <Dragonfly/detail/vao.h> //will be replaced
#include <Dragonfly/detail/Texture/Texture3D.h>
#include <fstream>
#include "CodeGen/ui.h"
#include "CodeGen/Objects/models.h"
#include "configurables.h"

#define DEBUG

class QuadricRender 
{
public:	
	struct QuadricArg {
		float delta = 0.008;
		int ray_count = 30;
		float correction = 0.01;
	};
	struct TestArg {
		MyExpr* model;
		long max_frames;
		int max_steps;
		QuadricArg q_arg;
		TraceTypes::TraceType method;
	};
	struct PreprocessTestArg {
		MyExpr* model;
		long run_count;
		bool use_cone_trace;
		ConeTraceTypes::ConeTraceType cone_trace_type;
		ConeTraceTypes::ConeTraceAlgorithm cone_trace_alg;
	};

	QuadricRender() {}
	~QuadricRender();
	void Init(int);
	void Render();

	void SetView(glm::vec3, glm::vec3, glm::vec3);
	std::vector<float> RunErrorTest(TestArg arg);
	double RunSpeedTest(TestArg arg);
	std::vector<std::chrono::duration<double>> RunPreprocessSpeedTest(PreprocessTestArg arg);

private:

	void Preprocess();
	void RenderUI();

	void CompilePreprocess();
	bool Compile();
	
	bool LoadSDF(const char*);
	bool SaveSDF();

	// cone tracing
	bool useConeTrace = false;
	ConeTraceTypes::ConeTraceType coneTraceDesc = ConeTraceTypes::cube;
	ConeTraceTypes::ConeTraceAlgorithm coneTraceAlg = ConeTraceTypes::gradient;

	// debugging quadric
	glm::ivec3 illustratedQuadricCoord = glm::ivec3(0);

	glm::ivec3 grid;
	// a variable that is used all over the place to store text
	TraceTypes::TraceType trace_method = TraceTypes::quadric;
	QuadricArg quadricArgs;
	int maxIterations = 64;

	df::Texture3D<glm::vec4> eccentricityTexture;
	df::Texture2D<glm::vec4> distanceTexture;
	df::Texture2D<glm::vec4> frameTexture;

	// contains the k values after a preprocess step if we are in debug mode
	std::vector<glm::vec2> kAndDistValues;

	df::Renderbuffer<df::depth24>* frameBuff;

	df::ShaderProgramEditorVF* program;
	df::ComputeProgramEditor* debugCompProgram;
	df::ComputeProgramEditor* sdfGradientComputeProgram;
	df::ComputeProgramEditor* pass1Program;
	df::ComputeProgramEditor* pass2Program;

	df::Sample sam = df::Sample("Quadric Tracing", 640, 480, df::Sample::FLAGS::DEFAULT | df::Sample::FLAGS::RENDERDOC); //handles Events and such
	df::Camera cam;

	int w, h;
	const std::vector<std::pair<std::string, MyExpr*>> examples = {
		{"4-way pipe", model1_expr() },
		{"Inset box", model2_expr() },
		{"Wheel with holes", model3_expr() },
		{"??", model4_expr() },
		{"Box with holes", model5_expr() },
		{"Boxes with holes", model7_expr() },
		{"Inset boxes", model8_expr() },
		{"Wheels with holes", model9_expr() },
		{"Mickey mouse", model10_expr(4) }
	};

	//codegen
	MyExpr *csg_tree;
};

/*template<typename T>
struct ASD {
	using value_type = T;
	int size = 1;
};
template<int I, typename T, glm::qualifier Q>
struct ASD<glm::vec<I, T, Q>>{
	using value_type = T;
	int size = I;
};*/
