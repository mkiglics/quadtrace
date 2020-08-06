#include "QuadricRender.h"

QuadricRender::~QuadricRender() 
{
	delete program;
	delete eccComputeProgram;
	delete sdfComputeProgram;
}

void QuadricRender::Init(int gridSize = 16)
{
	grid = glm::ivec3(gridSize);

	if (!LoadSDF("Shaders/sdf"))
	{
		LoadSDF(examples[0].second.c_str()); SaveSDF();
	}
	
	cam.SetView(glm::vec3(15.7, 15, 15), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	cam.SetSpeed(15);
	sam.AddHandlerClass(cam, 5);
	sam.AddStaticHandlerClass<df::ImGuiHandler>(10);

	program = new df::ShaderProgramEditorVF("Shader Editor");
	*program << "Shaders/sdf.glsl"_frag << "Shaders/sdf"_frag << "Shaders/tracer.glsl"_frag << "Shaders/vert.vert"_vert << "Shaders/fragment.frag"_frag << df::LinkProgram;

	int w = df::Backbuffer.getWidth(), h = df::Backbuffer.getHeight();
	auto frameBuff = df::Renderbuffer<df::depth24>(w, h) + df::Texture2D<>(w, h, 1);

	sdfTexture = df::Texture3D<float>(grid.x, grid.y, grid.z);
	eccentricityTexture = df::Texture3D<float>(grid.x - 1, grid.y - 1, grid.z - 1);

	sdfComputeProgram = new df::ComputeProgramEditor("SDF Computer");
	*sdfComputeProgram << "Shaders/sdf.glsl"_comp << "Shaders/sdf"_comp << "Shaders/sdf.compute"_comp << df::LinkProgram;

	eccComputeProgram = new df::ComputeProgramEditor("Eccentricity Computer");
	*eccComputeProgram << "Shaders/tracer.glsl"_comp << "Shaders/ecc.compute"_comp << df::LinkProgram;

	sam.AddResize([&](int w, int h) {frameBuff = frameBuff.MakeResized(w, h); });

	GL_CHECK; //extra opengl error checking in GPU Debug build configuration

	Preprocess();
}

void QuadricRender::Preprocess()
{
	*sdfComputeProgram << "a" << 0;
	glBindImageTexture(0, (GLuint) sdfTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glDispatchCompute(grid.x, grid.y, grid.z);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

	glBindImageTexture(0, (GLuint) eccentricityTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	*eccComputeProgram << "sdf_values" << sdfTexture;
	glDispatchCompute(grid.x - 1, grid.y - 1, grid.z - 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

void QuadricRender::Render()
{
	sam.Run([&](float deltaTime) //delta time in ms
		{

			cam.Update();

			df::Backbuffer << df::Clear() << *program << "eye" << cam.GetEye() << "at" << cam.GetAt() << "up" << cam.GetUp() 
				<< "windowSize" << glm::vec2(cam.GetSize().x, cam.GetSize().y)
				<< "eccentricity" << eccentricityTexture << "N" << grid << "sdf_values" << sdfTexture
				<< "quad" << useQuadricTrace;
			*program << df::NoVao(GL_TRIANGLE_STRIP, 4);

			GL_CHECK;
#ifdef DEBUG
			program->Render(); //only the UI!!
			eccComputeProgram->Render(); sdfComputeProgram->Render();
#endif
			RenderUI();
		}
	);
}

bool QuadricRender::Link()
{
	hasError = false;
	delete sdfComputeProgram;
	sdfComputeProgram = new df::ComputeProgramEditor("SDF Computer");
	*sdfComputeProgram << "Shaders/sdf.glsl"_comp << "Shaders/sdf"_comp << "Shaders/sdf.compute"_comp << df::LinkProgram;
	if (sdfComputeProgram->GetErrors().size() > 0)
	{
		hasError = true;
		errosMsg = sdfComputeProgram->GetErrors();
	}
	delete program;
	program = new df::ShaderProgramEditorVF("Shader Editor");
	*program << "Shaders/sdf.glsl"_frag << "Shaders/sdf"_frag << "Shaders/tracer.glsl"_frag << "Shaders/vert.vert"_vert << "Shaders/fragment.frag"_frag << df::LinkProgram;
	if (hasError || program->GetErrors().size() > 0)
	{
		hasError = true;
		if (program->GetErrors().size() > 0)
			errosMsg = program->GetErrors();
		return false;
	}
	Preprocess();
	hasError = false;
	return true;
}

void QuadricRender::RenderUI()
{
	ImGui::SetNextWindowSize({ 640, 480 }, ImGuiCond_FirstUseEver);
	ImGui::Begin("SDF editor");
	ImGui::Text("SDF Editor");
	ImGui::InputTextMultiline("", &text[0], bufferSize, { 640, 300 });
	if (ImGui::Button("Compile", { 640, 30 }))
	{
		if (SaveSDF())
			Link();
	}
	for (auto e : examples)
	{
		if (ImGui::Button(e.first.c_str(), { 154, 30 }))
		{
			if (LoadSDF(e.second.c_str()))
			{
				SaveSDF();
				Link();
			}
		}
		ImGui::SameLine();
	}
	ImGui::NewLine();
#ifdef DEBUG
	if (ImGui::Button(useQuadricTrace ? "Render using quadric tracing" : "Render using sphere tracing")) useQuadricTrace = !useQuadricTrace;
#endif
	if (hasError)
		ImGui::TextColored({ 255, 0, 0, 255 }, errosMsg.c_str());
	ImGui::End();
}

bool QuadricRender::LoadSDF(const char* name) 
{ 
	std::ifstream in(name);
	if (!in.is_open())
	{
		hasError = true;
		errosMsg = "Cannot open " + std::string(name);
		return false;
	}
	std::string contents((std::istreambuf_iterator<char>(in)),
		std::istreambuf_iterator<char>());
	text = std::vector<char>(contents.begin(), contents.end());
	text.resize(bufferSize);
	hasError = false;
	return true; 
}
bool QuadricRender::SaveSDF()
{ 
	std::ofstream out("Shaders/sdf");
	if (!out.is_open())
	{
		hasError = true;
		errosMsg = "Cannot open Shaders/sdf";
		return false;
	}
	out << text.data();
	out.close();
	hasError = false;
	return true; 
}