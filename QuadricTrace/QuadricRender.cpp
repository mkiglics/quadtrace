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
	
	csg_tree = demo_expr();
	build_kernel("Shaders/sdf", csg_tree); // generates the function
	//build_footmap("Shaders/Footmap/footmap.glsl", csg_tree);

	if (!LoadSDF("Shaders/sdf"))
	{
		LoadSDF(examples[0].second.c_str()); SaveSDF();
	}
	
	cam.SetView(glm::vec3(15.1, 15, 15), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	cam.SetSpeed(15);
	sam.AddHandlerClass(cam, 5);
	sam.AddHandlerClass<df::ImGuiHandler>(10);

	program = new df::ShaderProgramEditorVF("Shader Editor");
	*program << "Shaders/vert.vert"_vert << "Shaders/fragment.frag"_frag << df::LinkProgram;

	w = df::Backbuffer.getWidth(), h = df::Backbuffer.getHeight();

<<<<<<< HEAD
	frameBuff = new df::Renderbuffer<df::depth24>(w, h);
	//frameBuff = new (df::Renderbuffer<df::depth24>(w, h) + df::Texture2D<>(w, h, 1));

=======
>>>>>>> 0d1eae062da656f5f2a24c900f78bf4039059cc3
	sdfTexture = df::Texture3D<float>(grid.x, grid.y, grid.z);
	eccentricityTexture = df::Texture3D<float>(grid.x - 1, grid.y - 1, grid.z - 1);
	frameTexture = df::Texture2D<glm::vec4>(w, h);

	sdfComputeProgram = new df::ComputeProgramEditor("SDF Computer");
	*sdfComputeProgram << "Shaders/sdf_common.glsl"_comp << "Shaders/sdf"_comp << "Shaders/sdf_precompute.glsl"_comp << df::LinkProgram;

	eccComputeProgram = new df::ComputeProgramEditor("Eccentricity Computer");
	*eccComputeProgram << "Shaders/tracing.glsl"_comp << "Shaders/eccentricity.glsl"_comp << df::LinkProgram;

<<<<<<< HEAD
	frameCompProgram = new df::ComputeProgramEditor("Frame Computer");
	*frameCompProgram << "Shaders/sdf_common.glsl"_comp << "Shaders/sdf"_comp << "Shaders/tracing.glsl"_comp << "Shaders/quadric.glsl"_comp << "Shaders/frame.comp"_comp << df::LinkProgram;

	sam.AddResize([&](int w, int h) {*frameBuff = frameBuff->MakeResized(w, h); });

=======
>>>>>>> 0d1eae062da656f5f2a24c900f78bf4039059cc3
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
	*eccComputeProgram << "sdf_values" << sdfTexture << "N" << quadricArgs.ray_count << "M" << quadricArgs.ray_count << "correction" << quadricArgs.correction;
	glDispatchCompute(grid.x - 1, grid.y - 1, grid.z - 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

void QuadricRender::Render()
{
	sam.Run([&](float deltaTime) //delta time in ms
		{

			cam.Update();

			*frameCompProgram << "eye" << cam.GetEye() << "at" << cam.GetAt() << "up" << cam.GetUp()
				<< "windowSize" << glm::vec2(cam.GetSize().x, cam.GetSize().y)
				<< "eccentricity" << eccentricityTexture << "N" << grid << "sdf_values" << sdfTexture
				<< "render_quadric" << useQuadricTrace << "delta" << quadricArgs.delta << "error_test" << 0;
			glBindImageTexture(0, (GLuint)frameTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
			glDispatchCompute(w, h, 1);
			glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

			df::Backbuffer << df::Clear() << *program << "frame" << frameTexture;

			*program << df::NoVao(GL_TRIANGLE_STRIP, 4);

			GL_CHECK;
#ifdef DEBUG
			program->Render(); //only the UI!!
			eccComputeProgram->Render(); sdfComputeProgram->Render();
			frameCompProgram->Render();
#endif
			RenderUI();
		}
	);
}

void QuadricRender::RunErrorTest(const char* filename, bool quadric, long max_frames, int max_steps, QuadricParam arg, int scene, SphereTrace sphere_trace_method)
{
	quadricArgs = arg;
	long frame = 0;
	if (scene < examples.size() && LoadSDF(examples[scene].second.c_str()))
	{
		SaveSDF();
		Link();
	}
	else {
		std::cout << "Could not open scene " << scene << std::endl;
		return;
	}
	sam.Run([&](float deltaTime)
		{
			if (frame == max_frames) sam.Quit();
			++frame;
			cam.Update();

			*frameCompProgram << "eye" << cam.GetEye() << "at" << cam.GetAt() << "up" << cam.GetUp()
				<< "windowSize" << glm::vec2(cam.GetSize().x, cam.GetSize().y)
				<< "eccentricity" << eccentricityTexture << "N" << grid << "sdf_values" << sdfTexture
				<< "render_quadric" << (int) quadric << "delta" << quadricArgs.delta << "error_test" << 1 << "max_iter" << max_steps;
			glBindImageTexture(0, (GLuint)frameTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
			glDispatchCompute(w, h, 1);
			glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

			df::Backbuffer << df::Clear() << *program << "frame" << frameTexture;

			*program << df::NoVao(GL_TRIANGLE_STRIP, 4);
			/*program->Render(); //only the UI!!
			eccComputeProgram->Render(); sdfComputeProgram->Render();
			frameCompProgram->Render();*/
		}
	);
	SaveTexture(filename);
}

void QuadricRender::RunSpeedTest(const char* filename, bool quadric, long max_frames, int max_steps, QuadricParam arg, int scene, SphereTrace sphere_trace_method)
{
	quadricArgs = arg;
	long frame = -50;
	if (scene < examples.size() && LoadSDF(examples[scene].second.c_str()))
	{
		SaveSDF();
		Link();
	}
	else {
		std::cout << "Could not open scene " << scene << std::endl;
		return;
	}
	auto start = std::chrono::steady_clock::now();
	sam.Run([&](float deltaTime)
		{
			if (frame == 0) {
				start = std::chrono::steady_clock::now();
			}
			if (frame == max_frames)
			{
				auto end = std::chrono::steady_clock::now();
				std::chrono::duration<double> elapsed = end - start;
				std::ofstream out(filename);
				if (out.is_open()) {
					out << elapsed.count() << std::endl;
					out.close();
				}
				sam.Quit();
			}
			++frame;
			cam.Update();

			*frameCompProgram << "eye" << cam.GetEye() << "at" << cam.GetAt() << "up" << cam.GetUp()
				<< "windowSize" << glm::vec2(cam.GetSize().x, cam.GetSize().y)
				<< "eccentricity" << eccentricityTexture << "N" << grid << "sdf_values" << sdfTexture
				<< "render_quadric" << (int)quadric << "delta" << quadricArgs.delta << "error_test" << 0 << "max_iter" << max_steps;
			glBindImageTexture(0, (GLuint)frameTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
			glDispatchCompute(w, h, 1);
			glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

			df::Backbuffer << df::Clear() << *program << "frame" << frameTexture;

			*program << df::NoVao(GL_TRIANGLE_STRIP, 4);
		}
	);
}

void QuadricRender::SetView(glm::vec3 eye, glm::vec3 at, glm::vec3 up)
{
	cam.SetView(eye, at, up);
}

bool QuadricRender::Link()
{
	hasError = false;
	build_footmap("Shaders/sdf", csg_tree); // generates the function
	delete sdfComputeProgram;
	sdfComputeProgram = new df::ComputeProgramEditor("SDF Computer");
	*sdfComputeProgram << "Shaders/sdf_common.glsl"_comp << "Shaders/sdf"_comp << "Shaders/sdf_precompute.glsl"_comp << df::LinkProgram;
	if (sdfComputeProgram->GetErrors().size() > 0)
	{
		hasError = true;
		errorMsg = sdfComputeProgram->GetErrors();
	}
	delete program;
	program = new df::ShaderProgramEditorVF("Shader Editor");
	*program << "Shaders/sdf_common.glsl"_frag << "Shaders/sdf"_frag << "Shaders/tracing.glsl"_frag << "Shaders/quadric.glsl"_frag << "Shaders/vert.vert"_vert << "Shaders/fragment.frag"_frag << df::LinkProgram;
	if (hasError || program->GetErrors().size() > 0)
	{
		hasError = true;
		if (program->GetErrors().size() > 0)
			errorMsg = program->GetErrors();
		return false;
	}
	delete frameCompProgram;
	frameCompProgram = new df::ComputeProgramEditor("Frame Computer");
	*frameCompProgram << "Shaders/sdf_common.glsl"_comp << "Shaders/sdf"_comp << "Shaders/tracing.glsl"_comp << "Shaders/quadric.glsl"_comp << "Shaders/frame.comp"_comp << df::LinkProgram;
	if (hasError || program->GetErrors().size() > 0)
	{
		hasError = true;
		if (program->GetErrors().size() > 0)
			errorMsg = program->GetErrors();
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
		ImGui::TextColored({ 255, 0, 0, 255 }, errorMsg.c_str());
	ImGui::End();

	ImGui::Begin("CSG Editor");
	RenderCSG_UI(*csg_tree);
	ImGui::End();
}

bool QuadricRender::LoadSDF(const char* name) 
{ 
	std::ifstream in(name);
	if (!in.is_open())
	{
		hasError = true;
		errorMsg = "Cannot open " + std::string(name);
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
		errorMsg = "Cannot open Shaders/sdf";
		return false;
	}
	out << text.data();
	out.close();
	hasError = false;
	return true; 
}

bool QuadricRender::SaveTexture(const char* filename)
{

	std::ofstream out(filename);
	if (!out.is_open())
	{
		std::cout << "Cannot open " << filename << std::endl;
		return false;
	}
	std::vector<float> data(w * h);
	glReadPixels(0, 0, w, h, GL_RED, GL_FLOAT, &data[0]);
	
	for (int i = 0; i < h; ++i)
	{
		for (int j = 0; j < w; ++j)
		{
			out << data[i*w+j] << ',';
		}
		out << "\n";
	}
	out.close();
	return true;
}