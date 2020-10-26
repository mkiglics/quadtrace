#include "QuadricRender.h"

QuadricRender::~QuadricRender()
{
	delete program;
	delete eccComputeProgram;
	delete sdfComputeProgram;
}

void QuadricRender::Init(int gridSize = 16)
{
	//SaveImageZ(eccentricityTexture, "asd");
	grid = glm::ivec3(gridSize);

	csg_tree = model4_expr();
	build_kernel("Shaders/sdf.tmp", csg_tree); // generates the function
	//build_footmap("Shaders/Footmap/footmap.glsl", csg_tree);

	if (!LoadSDF("Shaders/sdf.tmp"))
	{
		LoadSDF("Examples/default.glsl"); SaveSDF();
	}

	cam.SetView(glm::vec3(15.1, 15, 15), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	cam.SetSpeed(15);
	sam.AddHandlerClass(cam, 5);
	sam.AddHandlerClass<df::ImGuiHandler>(10);

	w = df::Backbuffer.getWidth(), h = df::Backbuffer.getHeight();

	frameBuff = new df::Renderbuffer<df::depth24>(w, h);
	//frameBuff = new (df::Renderbuffer<df::depth24>(w, h) + df::Texture2D<>(w, h, 1));

	sdfTexture = df::Texture3D<float>(grid.x, grid.y, grid.z);
	eccentricityTexture = df::Texture3D<glm::vec4>(grid.x - 1, grid.y - 1, grid.z - 1);
	frameTexture = df::Texture2D<glm::vec4>(w, h);

	sam.AddResize([&](int _w, int _h) {
		w = _w; h = _h;
		*frameBuff = frameBuff->MakeResized(w, h); }
	);

	GL_CHECK; //extra opengl error checking in GPU Debug build configuration

	if (!Link())
	{
		std::cout << "Error with linking " << errorMsg << std::endl;
	}
}

void QuadricRender::Preprocess()
{
	*sdfComputeProgram << "a" << 0;
	glBindImageTexture(0, (GLuint)sdfTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glDispatchCompute(grid.x, grid.y, grid.z);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

	glBindImageTexture(0, (GLuint)eccentricityTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	*eccComputeProgram << "sdf_values" << sdfTexture << "N" << quadricArgs.ray_count << "M" << quadricArgs.ray_count << "correction" << quadricArgs.correction << "useConeTrace" << (int)useConeTrace;
	glDispatchCompute(grid.x - 1, grid.y - 1, grid.z - 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

void QuadricRender::Render()
{
	int frameCount = 0;
	sam.Run([&](float deltaTime) //delta time in ms
		{
			cam.Update();


#ifdef DEBUG
			bool showQuadric = trace_method == Quadric;
#else
			bool showQuadric = false;
#endif
			*frameCompProgram << "eye" << cam.GetEye() << "at" << cam.GetAt() << "up" << cam.GetUp()
				<< "windowSize" << glm::vec2(cam.GetSize().x, cam.GetSize().y)
				<< "eccentricity" << eccentricityTexture << "N" << grid << "sdf_values" << sdfTexture
				<< "delta" << quadricArgs.delta << "error_test" << 0 << "showQuadric" << (int)showQuadric
				<< "showcaseQuadricCoord" << showcaseQuadricCoord;
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

			if (++frameCount == 5) {
				SaveImageZ(eccentricityTexture, "Eccentricity");
			}
		}
	);
}

std::vector<float> QuadricRender::RunErrorTest(TestArg arg)
{
	quadricArgs = arg.q_arg;
	csg_tree = arg.model;
	trace_method = arg.method;
	build_kernel("Shaders/sdf.tmp", csg_tree);
	LoadSDF("Shaders/sdf.tmp"); Link();
	
	std::vector<float> data(w * h);
	int fr = 0;

	sam.Run([&](float deltaTime)
		{
			cam.Update();

			*frameCompProgram << "eye" << cam.GetEye() << "at" << cam.GetAt() << "up" << cam.GetUp()
				<< "windowSize" << glm::vec2(cam.GetSize().x, cam.GetSize().y)
				<< "eccentricity" << eccentricityTexture << "N" << grid << "sdf_values" << sdfTexture
				<< "render_quadric" << (int)(arg.method == SphereTrace::Quadric) << "delta" << quadricArgs.delta << "error_test" << 1 << "max_iter" << arg.max_steps
				<< "trace_method" << (int) (arg.method) << "showQuadric" << 0;
			glBindImageTexture(0, (GLuint)frameTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
			glDispatchCompute(w, h, 1);
			glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

			df::Backbuffer << df::Clear() << *program << "frame" << frameTexture;

			*program << df::NoVao(GL_TRIANGLE_STRIP, 4);

			if (fr == 1) {
				glReadPixels(0, 0, w, h, GL_RED, GL_FLOAT, &data[0]);
				sam.Quit();
			}
			++fr;
		}
	);

	return data;
}

double QuadricRender::RunSpeedTest(TestArg arg)
{
	csg_tree = arg.model;
	quadricArgs = arg.q_arg;
	trace_method = arg.method;
	build_kernel("Shaders/sdf.tmp", csg_tree);
	LoadSDF("Shaders/sdf.tmp"); Link();
	long frame = -10;
	std::chrono::duration<double> elapsed;
	
	auto start = std::chrono::steady_clock::now();
	sam.Run([&](float deltaTime) {
		if (frame == 0) {
			start = std::chrono::steady_clock::now();
		}
		if (frame == arg.max_frames)
		{
			auto end = std::chrono::steady_clock::now();
			elapsed = end - start;
			sam.Quit();
		}
		++frame;
		cam.Update();
		*frameCompProgram << "eye" << cam.GetEye() << "at" << cam.GetAt() << "up" << cam.GetUp()
			<< "windowSize" << glm::vec2(cam.GetSize().x, cam.GetSize().y)
			<< "eccentricity" << eccentricityTexture << "N" << grid << "sdf_values" << sdfTexture
			<< "delta" << quadricArgs.delta << "error_test" << 0 << "max_iter" << arg.max_steps
			<< "showQuadric" << 0;
		glBindImageTexture(0, (GLuint)frameTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		glDispatchCompute(w, h, 1);
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

		df::Backbuffer << df::Clear() << *program << "frame" << frameTexture;

		*program << df::NoVao(GL_TRIANGLE_STRIP, 4);
	});
	return elapsed.count() / (arg.max_frames - 10);
}

void QuadricRender::SetView(glm::vec3 eye, glm::vec3 at, glm::vec3 up)
{
	cam.SetView(eye, at, up);
}

bool QuadricRender::Link()
{
	hasError = false;
	//build_footmap("Shaders/sdf", csg_tree); // generates the function
	delete sdfComputeProgram;
	sdfComputeProgram = new df::ComputeProgramEditor("SDF Computer");
	*sdfComputeProgram << "Shaders/common.glsl"_comp << "Shaders/sdf_common.glsl"_comp 
					   << "Shaders/sdf.tmp"_comp << "Shaders/sdf_precompute.glsl"_comp << df::LinkProgram;
	if (sdfComputeProgram->GetErrors().size() > 0)
	{
		hasError = true;
		errorMsg = sdfComputeProgram->GetErrors();
	}
	delete program;
	program = new df::ShaderProgramEditorVF("Shader Editor");
	*program << "Shaders/common.glsl"_frag << "Shaders/sdf_common.glsl"_frag << "Shaders/sdf.tmp"_frag << "Shaders/tracing.glsl"_frag << "Shaders/quadric.glsl"_frag << "Shaders/vert.vert"_vert << "Shaders/fragment.frag"_frag << df::LinkProgram;
	if (hasError || program->GetErrors().size() > 0)
	{
		hasError = true;
		if (program->GetErrors().size() > 0)
			errorMsg = program->GetErrors();
		return false;
	}
	delete frameCompProgram;
	frameCompProgram = new df::ComputeProgramEditor("Frame Computer");
	*frameCompProgram << "Shaders/common.glsl"_comp << "Shaders/sdf_common.glsl"_comp << "Shaders/sdf.tmp"_comp << "Shaders/tracing.glsl"_comp << "Shaders/quadric.glsl"_comp << "Shaders/frame.comp"_comp << "Shaders/Debug/quadric_showcase.comp"_comp << trace_path[(int)trace_method] << df::LinkProgram;
	if (hasError || frameCompProgram->GetErrors().size() > 0)
	{
		hasError = true;
		if (frameCompProgram->GetErrors().size() > 0)
			errorMsg = frameCompProgram->GetErrors();
		return false;
	}
	delete eccComputeProgram;
	eccComputeProgram = new df::ComputeProgramEditor("Eccentricity Computer");
	*eccComputeProgram << "Shaders/common.glsl"_comp << "Shaders/sdf_common.glsl"_comp << "Shaders/sdf.tmp"_comp << "Shaders/tracing.glsl"_comp << "Shaders/eccentricity.glsl"_comp << df::LinkProgram;
	if (hasError || eccComputeProgram->GetErrors().size() > 0) {
		hasError = true;
		if (eccComputeProgram->GetErrors().size() > 0)
			errorMsg = eccComputeProgram->GetErrors();
		return false;
	}

	Preprocess();
	hasError = false;
	return true;
}

void QuadricRender::RenderUI()
{
	ImGui::SetNextWindowSize({ 640, 480 }, ImGuiCond_FirstUseEver);
	float windowWidth = ImGui::GetContentRegionAvailWidth();
	ImGui::Begin("SDF editor");
	ImGui::Text("SDF Editor");
	ImGui::InputTextMultiline("", &text[0], bufferSize, { windowWidth, 300 });
	if (ImGui::Button("Compile", { windowWidth, 30 }))
	{
		if (SaveSDF())
			Link();
	}
	for (int i = 0; i < examples.size(); i++)
	{
		if (ImGui::Button(examples[i].first.c_str(), { windowWidth / 4, 30 }))
		{
			csg_tree = examples[i].second;
			build_kernel("Shaders/sdf.tmp", csg_tree);

			if (!LoadSDF("Shaders/sdf.tmp"))
			{
				LoadSDF("Examples/default.glsl"); 
				SaveSDF();
			}
		}
		if ((i + 1) % 4 != 0) 
		{
			ImGui::SameLine();
		}
	}
	ImGui::NewLine();
#ifdef DEBUG
	static int renderType = (int)SphereTrace::Quadric;
	ImGui::RadioButton("Simple", &renderType, (int) SphereTrace::Simple); ImGui::SameLine();
	ImGui::RadioButton("Relaxed", &renderType, (int)SphereTrace::Relaxed); ImGui::SameLine();
	ImGui::RadioButton("Enhanced", &renderType, (int)SphereTrace::Enhanced); ImGui::SameLine();
	ImGui::RadioButton("Quadric", &renderType, (int)SphereTrace::Quadric);

	if ((int)trace_method != renderType) 
	{
		trace_method = (SphereTrace)renderType;
		Link();
	}

	if (trace_method == Quadric) 
	{
		if (ImGui::Checkbox("Use cone trace", &useConeTrace)) 
		{
			Preprocess();
		}

		ImGui::DragInt("Showcase quadric X", &showcaseQuadricCoord.x, 0.1, -grid.x, grid.x);
		ImGui::DragInt("Showcase quadric Y", &showcaseQuadricCoord.y, 0.1, -grid.y, grid.y);
		ImGui::DragInt("Showcase quadric Z", &showcaseQuadricCoord.z, 0.1, -grid.z, grid.z);
	}
#endif

	if (hasError) 
	{
		ImGui::TextColored({ 255, 0, 0, 255 }, errorMsg.c_str());
	}
	ImGui::End();

	/*ImGui::Begin("CSG Editor");
	RenderCSG_UI(*csg_tree);
	ImGui::End();*/
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
	std::ofstream out("Shaders/sdf.tmp");
	if (!out.is_open())
	{
		hasError = true;
		errorMsg = "Cannot open Shaders/sdf.tmp";
		return false;
	}
	out << text.data();
	out.close();
	hasError = false;
	return true;
}