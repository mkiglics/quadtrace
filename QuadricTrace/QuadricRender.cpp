#include "QuadricRender.h"

QuadricRender::~QuadricRender()
{
	delete program;
	delete eccComputeProgram;
	delete sdfGradientComputeProgram;
}

void QuadricRender::Init(int gridSize = 16)
{
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
		*frameBuff = frameBuff->MakeResized(w, h);
		frameTexture = frameTexture.MakeResized(w, h);
	});

	GL_CHECK; //extra opengl error checking in GPU Debug build configuration

	Compile();
	Preprocess();
}


void SaveImageZ(const df::Texture3D<glm::vec4>& texture, const std::string& path)
{
	int buffSize = texture.getWidth() * texture.getHeight() * texture.getDepth() * texture.getLevels();


	std::vector<float> data(buffSize);
	glGetTextureImage((GLuint)texture, 0, GL_RGBA32F, GL_FLOAT, sizeof(glm::vec4) * buffSize, &data[0]);

	std::ofstream myfile;
	myfile.open(path);

	//SDL_Surface* surf = SDL_CreateRGBSurface(0, width, height, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
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

	myfile.close();
}


void QuadricRender::Preprocess()
{
	// glBindImageTexture(0, (GLuint)eccentricityTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	*sdfGradientComputeProgram << "outField" << eccentricityTexture;
	glDispatchCompute(grid.x - 1, grid.y - 1, grid.z - 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

	// nem tudom ez jo e most de valszeg nem
	SaveImageZ(eccentricityTexture, "sdfgrad.txt");

	/*glBindImageTexture(0, (GLuint)eccentricityTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	*eccComputeProgram << "sdf_values" << sdfTexture << "N" << quadricArgs.ray_count << "M" << quadricArgs.ray_count << "correction" << quadricArgs.correction << "useConeTrace" << (int)useConeTrace;
	glDispatchCompute(grid.x - 1, grid.y - 1, grid.z - 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);*/
}

void QuadricRender::Render()
{
	return;

	sam.Run([&](float deltaTime) //delta time in ms
		{
			cam.Update();

#ifdef DEBUG
			bool showQuadric = trace_method == TraceTypes::quadric;
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

			RenderUI();
		}
	);
}

std::vector<float> QuadricRender::RunErrorTest(TestArg arg)
{
	quadricArgs = arg.q_arg;
	csg_tree = arg.model;
	trace_method = arg.method;
	build_kernel("Shaders/sdf.tmp", csg_tree);
	LoadSDF("Shaders/sdf.tmp"); 
	Compile();
	Preprocess();
	
	std::vector<float> data(w * h);
	int fr = 0;

	sam.Run([&](float deltaTime)
		{
			cam.Update();

			*frameCompProgram << "eye" << cam.GetEye() << "at" << cam.GetAt() << "up" << cam.GetUp()
				<< "windowSize" << glm::vec2(cam.GetSize().x, cam.GetSize().y)
				<< "eccentricity" << eccentricityTexture << "N" << grid << "sdf_values" << sdfTexture
				<< "render_quadric" << (int)(arg.method == TraceTypes::quadric) << "delta" << quadricArgs.delta << "error_test" << 1 << "max_iter" << arg.max_steps
				<< "trace_method" << arg.method.id << "showQuadric" << 0;
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
	LoadSDF("Shaders/sdf.tmp"); 
	Compile();
	Preprocess();

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


/* Writes the given values in the map to a glsl file as define directives, the key of the map being
the identifier and the value being the token-string.
*/
void writeDefines(std::map<std::string, std::string> defines, std::string path = "Shaders/defines.glsl")
{
	std::ofstream out(path);
	if (!out.is_open())
	{
		std::cout << "Could not open defines file!" << std::endl;
	}
	for (auto const& define: defines) 
	{
		out << "#define " << define.first << " " << define.second << "\n";
	}

	out.close();
}

/* Called before the compile step
*/
void QuadricRender::CompilePreprocess()
{
	std::map<std::string, std::string> defines {
		// cone trace
		{"RAY_DIRECTIONS", std::string("gRay") + std::to_string(coneTraceDesc.rayCount) + "Directions"},
		{"RAY_HALF_TANGENTS", std::string("gRay") + std::to_string(coneTraceDesc.rayCount) + "HalfTangents"},

		// sampling method
		{"UNBOUND_QUADRIC", useConeTrace ? "unboundQuadricConeTrace" : "unboundQuadricBruteForce"},

		// which tracing to use
		//{"TRACE", ""}
	};
	writeDefines(defines);
}

bool QuadricRender::Compile()
{
	// TODO include order:
	// constants.glsl
	// defines.glsl
	// Math/quadric.glsl
	// Math/common.glsl
	// {
	//		SDF/SDFprimitives.glsl
	//		SDF.glsl
	//		SDF/SDFcommon.glsl
	//		...tracing...
	// }
	// Math/interface.glsl
	// .. tracing...
	// main
	CompilePreprocess();
	std::cout << " " << std::endl;

	//build_footmap("Shaders/sdf", csg_tree); // generates the function
	delete sdfGradientComputeProgram;
	sdfGradientComputeProgram = new df::ComputeProgramEditor("SDF/Gradient Computer");
	*sdfGradientComputeProgram
		<< "Shaders/Preprocess/constants.glsl"_comp << "Shaders/defines.glsl"_comp << "Shaders/Math/common.glsl"_comp << "Shaders/Math/quadric.glsl"_comp
		<< "Shaders/SDF/SDFprimitives.glsl"_comp << "Shaders/SDF/SDFcommon.glsl"_comp << "Shaders/sdf.tmp"_comp << "Shaders/Math/interface.glsl"_comp
		<< "Shaders/Tracing/enhanced_sphere_trace.glsl"_comp << "Shaders/Tracing/cone_trace.glsl"_comp
		<< "Shaders/Preprocess/step1.glsl"_comp << df::LinkProgram;
	if (sdfGradientComputeProgram->GetErrors().size() > 0)
	{
		std::cout << sdfGradientComputeProgram->GetErrors() << std::endl;
		return false;
	}

	/*delete program;
	program = new df::ShaderProgramEditorVF("Shader Editor");
	*program << "Shaders/common.glsl"_frag << "Shaders/sdf_common.glsl"_frag << "Shaders/sdf.tmp"_frag << "Shaders/quadric.glsl"_frag << "Shaders/vert.vert"_vert << "Shaders/fragment.frag"_frag << df::LinkProgram;
	if (program->GetErrors().size() > 0)
	{
		std::cout << program->GetErrors() << std::endl;
		return false;
	}

	delete frameCompProgram;
	frameCompProgram = new df::ComputeProgramEditor("Frame Computer");
	*frameCompProgram << "Shaders/common.glsl"_comp << "Shaders/sdf_common.glsl"_comp << "Shaders/sdf.tmp"_comp << "Shaders/quadric.glsl"_comp << "Shaders/frame.comp"_comp << "Shaders/Debug/quadric_showcase.comp"_comp << trace_path[(int)trace_method] << df::LinkProgram;
	if (frameCompProgram->GetErrors().size() > 0)
	{
		std::cout << frameCompProgram->GetErrors() << std::endl;
		return false;
	}

	delete eccComputeProgram;
	eccComputeProgram = new df::ComputeProgramEditor("Eccentricity Computer");
	*eccComputeProgram << "Shaders/common.glsl"_comp << "Shaders/sdf_common.glsl"_comp << "Shaders/sdf.tmp"_comp << "Shaders/quadric.glsl"_comp << "Shaders/eccentricity.glsl"_comp << df::LinkProgram;
	if (eccComputeProgram->GetErrors().size() > 0) {
		std::cout << eccComputeProgram->GetErrors() << std::endl;
		return false;
	}*/

	return true;
}

void QuadricRender::RenderUI()
{
	ImGui::SetNextWindowSize({ 640, 480 }, ImGuiCond_FirstUseEver);
	float windowWidth = ImGui::GetContentRegionAvailWidth();
	ImGui::Begin("SDF editor");
	ImGui::Text("SDF Editor");
	if (ImGui::Button("Compile", { windowWidth, 30 }))
	{
		if (SaveSDF()) {
			Compile();
			Preprocess();
		}
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
	static int renderType = TraceTypes::quadric.id;
	ImGui::RadioButton("Simple", &renderType, TraceTypes::sphere.id); ImGui::SameLine();
	ImGui::RadioButton("Relaxed", &renderType, TraceTypes::relaxed.id); ImGui::SameLine();
	ImGui::RadioButton("Enhanced", &renderType, TraceTypes::enhanced.id); ImGui::SameLine();
	ImGui::RadioButton("Quadric", &renderType, TraceTypes::quadric.id);

	
	static int coneTrace = ConeTraceTypes::cube.id;
	ImGui::RadioButton("Tetrahedron", &renderType, ConeTraceTypes::tetrahedron.id); ImGui::SameLine();
	ImGui::RadioButton("Cube", &renderType, ConeTraceTypes::cube.id); ImGui::SameLine();
	ImGui::RadioButton("Octahedron", &renderType, ConeTraceTypes::octahedron.id); ImGui::SameLine();
	ImGui::RadioButton("Icosahedron", &renderType, ConeTraceTypes::icosahedron.id);

	if (trace_method.id != renderType || coneTraceDesc.id != coneTrace) 
	{
		trace_method = TraceTypes::traceTypes[renderType];
		coneTraceDesc = ConeTraceTypes::coneTraceTypes[coneTrace];
		Compile();
		Preprocess();
	}

	if (trace_method == TraceTypes::quadric) 
	{
		if (ImGui::Checkbox("Use cone trace", &useConeTrace)) 
		{
			Preprocess();
		}

		ImGui::DragInt("Showcase quadric X", &showcaseQuadricCoord.x, 0.1, -grid.x, grid.x);
		ImGui::DragInt("Showcase quadric Y", &showcaseQuadricCoord.y, 0.1, -grid.y, grid.y);
		ImGui::DragInt("Showcase quadric Z", &showcaseQuadricCoord.z, 0.1, -grid.z, grid.z);
	}

	if (ImGui::Button("Save eccentricity")) 
	{
		SaveImageZ(eccentricityTexture, "Eccentricity.txt");
	}
#endif
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
		std::cout << "Cannot open " + std::string(name) << std::endl;
		return false;
	}
	std::string contents((std::istreambuf_iterator<char>(in)),
		std::istreambuf_iterator<char>());

	return true;
}

bool QuadricRender::SaveSDF()
{
	std::ofstream out("Shaders/sdf.tmp");
	if (!out.is_open())
	{
		std::cout << "Cannot open Shaders/sdf.tmp" << std::endl;
		return false;
	}
	out.close();

	return true;
}