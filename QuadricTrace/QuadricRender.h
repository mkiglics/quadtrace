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

	bool useConeTrace = false;
	glm::ivec3 showcaseQuadricCoord = glm::ivec3(0);

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

	df::Sample sam = df::Sample("Quadric Tracing", 640, 480, df::Sample::FLAGS::DEFAULT | df::Sample::FLAGS::RENDERDOC); //handles Events and such
	df::Camera cam;

	const int bufferSize = 8192;
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
	const std::vector<df::detail::_CompShader> trace_path = {
		"Shaders/SphereTrace/sphere_trace.glsl"_comp,
		"Shaders/SphereTrace/relaxed_sphere_trace.glsl"_comp,
		"Shaders/SphereTrace/enhanced_sphere_trace.glsl"_comp,
		"Shaders/quadric_trace.glsl"_comp
	};	

	//codegen
	MyExpr *csg_tree;
};

template<typename InternalFormat_>
void SaveImageZ(const df::Texture3D<InternalFormat_>& texture, const std::string& path)
{
	/*int bufSizePerLevel = texture.getWidth() * texture.getHeight() * texture.getDepth() * texture.getLevels();
	constexpr GLenum internalFormat = df::detail::getInternalChannel<InternalFormat_>();
	constexpr GLenum baseType = df::detail::getInternalBaseType<InternalFormat_>();

	std::vector<df::detail::getBaseType<InternalFormat_>::Type> data(bufSizePerLevel);
	glGetTextureImage((GLuint)texture, 0, internalFormat, baseType, sizeof(InternalFormat_) * bufSizePerLevel, &data[0]);

	std::ofstream myfile;
	myfile.open(path);

	//SDL_Surface* surf = SDL_CreateRGBSurface(0, width, height, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
	std::vector< df::detail::getBaseType<InternalFormat_>::Type> imageData;
	myfile << texture.getWidth() << " " << texture.getHeight() << " " << texture.getDepth() << "\n";
	for (int i = 0; i < texture.getWidth(); i++) 
	{
		for (int k = 0; k < texture.getHeight(); k++) 
		{
			for (int j = 0; j < texture.getDepth(); j++) 
			{
				for (int l = 0; l < texture.getLevels(); l++) 
				{
					myfile << data[i * texture.getHeight() * texture.getDepth() * texture.getLevels() +
						k * texture.getDepth() * texture.getLevels() +
						j * texture.getLevels() +
						l] << " ";
				}
				myfile << "\n";
			}
		}
	}
	//SDL_FreeSurface(surf);

	myfile.close();*/
}
