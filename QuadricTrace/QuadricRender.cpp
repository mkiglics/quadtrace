#include "QuadricRender.h"

QuadricRender::~QuadricRender()
{
	delete program;
	delete sdfGradientComputeProgram;
}

void QuadricRender::Init(int gridSize = 16)
{
	grid = glm::ivec3(gridSize);
	kAndDistValues.resize(grid.x * grid.y * grid.z);

	csg_tree = model4_expr();
	build_kernel("Shaders/sdf.tmp", csg_tree); // generates the function

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

	eccentricityTexture = df::Texture3D<glm::vec4>(grid.x, grid.y, grid.z);
	distanceTexture = df::Texture2D<glm::vec4>(w, h);
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

void QuadricRender::Preprocess()
{
	glBindImageTexture(0, (GLuint)eccentricityTexture, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	*sdfGradientComputeProgram << "a" << 0;
	glDispatchCompute(grid.x, grid.y, grid.z);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

#ifdef DEBUG
	GLuint textureId = (GLuint)eccentricityTexture;

	std::vector<glm::vec4> data(grid.x * grid.y * grid.z);
	glGetTextureImage(textureId, 0, GL_RGBA, GL_FLOAT, sizeof(glm::vec4) * grid.x * grid.y * grid.z, &data[0]);

	for (int i = 0; i < kAndDistValues.size(); i++)
	{
		kAndDistValues[i].x = glm::length(glm::vec3(data[i].y, data[i].z, data[i].w)) - 2.0f;
		kAndDistValues[i].y = data[i].x;
	}
#endif
}

void QuadricRender::Render()
{
	sam.Run([&](float deltaTime) //delta time in ms
		{
			cam.Update();

			*frameCompProgram << "uCameraEye" << cam.GetEye() << "uCameraCenter" << cam.GetAt();
			glBindImageTexture(0, (GLuint)eccentricityTexture, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
			glBindImageTexture(1, (GLuint)frameTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
			glBindImageTexture(2, (GLuint)distanceTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
			glDispatchCompute(w, h, 1);
			glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

#ifdef DEBUG
			*debugCompProgram << "uCameraEye" << cam.GetEye() << "uCameraCenter" << cam.GetAt()
							  << "uIllustratedQuadricCood" << illustratedQuadricCoord;
			glBindImageTexture(0, (GLuint)eccentricityTexture, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
			glBindImageTexture(1, (GLuint)distanceTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
			glBindImageTexture(2, (GLuint)frameTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
			glDispatchCompute(w, h, 1);
			glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
#endif

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
				<< "eccentricity" << eccentricityTexture << "N" << grid
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
			<< "eccentricity" << eccentricityTexture << "N" << grid
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
		{"PASS1_TRACING(ray, desc, inField)", trace_method.macro_value }
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

	delete sdfGradientComputeProgram;
	sdfGradientComputeProgram = new df::ComputeProgramEditor("SDF/Gradient Computer");
	*sdfGradientComputeProgram
		<< "Shaders/defines.glsl"_comp << "Shaders/Preprocess/constants.glsl"_comp << "Shaders/Math/common.glsl"_comp << "Shaders/Math/quadric.glsl"_comp
		<< "Shaders/SDF/SDFprimitives.glsl"_comp << "Shaders/SDF/SDFcommon.glsl"_comp << "Shaders/sdf.tmp"_comp << "Shaders/Math/interface.glsl"_comp
		<< "Shaders/Tracing/enhanced_sphere_trace.glsl"_comp << "Shaders/Tracing/cone_trace.glsl"_comp
		<< "Shaders/Tracing/sphere_trace.glsl"_comp
		<< "Shaders/Preprocess/step1.glsl"_comp << df::LinkProgram;
	if (sdfGradientComputeProgram->GetErrors().size() > 0)
	{
		std::cout << sdfGradientComputeProgram->GetErrors() << std::endl;
		return false;
	}

	delete frameCompProgram;
	frameCompProgram = new df::ComputeProgramEditor("Frame computer");
	*frameCompProgram
		<< "Shaders/defines.glsl"_comp << "Shaders/Preprocess/constants.glsl"_comp << "Shaders/Math/common.glsl"_comp << "Shaders/Math/quadric.glsl"_comp
		<< "Shaders/SDF/SDFprimitives.glsl"_comp << "Shaders/SDF/SDFcommon.glsl"_comp << "Shaders/sdf.tmp"_comp << "Shaders/Math/interface.glsl"_comp
		<< "Shaders/Math/distanceInterface.glsl"_comp << "Shaders/Math/graphics.comp"_comp
		<< "Shaders/Tracing/sphere_trace.glsl"_comp << "Shaders/Tracing/relaxed_sphere_trace.glsl"_comp
		<< "Shaders/Tracing/enhanced_sphere_trace.glsl"_comp << "Shaders/Tracing/quadric_trace.glsl"_comp
		<< "Shaders/Render/pass1.comp"_comp << df::LinkProgram;
	if (frameCompProgram->GetErrors().size() > 0)
	{
		std::cout << frameCompProgram->GetErrors() << std::endl;
		return false;
	}

#ifdef DEBUG
	delete debugCompProgram;
	debugCompProgram = new df::ComputeProgramEditor("Frame computer");
	*debugCompProgram
		<< "Shaders/defines.glsl"_comp << "Shaders/Preprocess/constants.glsl"_comp << "Shaders/Math/common.glsl"_comp << "Shaders/Math/quadric.glsl"_comp
		<< "Shaders/SDF/SDFprimitives.glsl"_comp << "Shaders/SDF/SDFcommon.glsl"_comp << "Shaders/sdf.tmp"_comp << "Shaders/Math/interface.glsl"_comp
		<< "Shaders/Math/distanceInterface.glsl"_comp << "Shaders/Math/graphics.comp"_comp
		<< "Shaders/Math/box.glsl"_comp
		<< "Shaders/Render/passDebug.comp"_comp << df::LinkProgram;
	if (debugCompProgram->GetErrors().size() > 0)
	{
		std::cout << debugCompProgram->GetErrors() << std::endl;
		return false;
	}
#endif

	delete program;
	program = new df::ShaderProgramEditorVF("Shader Editor");
	*program << "Shaders/Render/vert.vert"_vert << "Shaders/Render/fragment.frag"_frag << df::LinkProgram;
	if (program->GetErrors().size() > 0)
	{
		std::cout << program->GetErrors() << std::endl;
		return false;
	}

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
	ImGui::RadioButton("Tetrahedron", &coneTrace, ConeTraceTypes::tetrahedron.id); ImGui::SameLine();
	ImGui::RadioButton("Cube", &coneTrace, ConeTraceTypes::cube.id); ImGui::SameLine();
	ImGui::RadioButton("Octahedron", &coneTrace, ConeTraceTypes::octahedron.id); ImGui::SameLine();
	ImGui::RadioButton("Icosahedron", &coneTrace, ConeTraceTypes::icosahedron.id);

	if (trace_method.id != renderType || coneTraceDesc.id != coneTrace) 
	{
		trace_method = TraceTypes::traceTypes[renderType];
		coneTraceDesc = ConeTraceTypes::coneTraceTypes[coneTrace];
		Compile();
		Preprocess();
	}

	if (true || trace_method == TraceTypes::quadric) 
	{
		if (ImGui::Checkbox("Use cone trace", &useConeTrace)) 
		{
			Compile();
			Preprocess();
		}

		ImGui::DragInt("Illustrated quadric X", &illustratedQuadricCoord.x, 0.1, 0, grid.x - 1);
		ImGui::DragInt("Illustrated quadric Y", &illustratedQuadricCoord.y, 0.1, 0, grid.y - 1);
		ImGui::DragInt("Illustrated quadric Z", &illustratedQuadricCoord.z, 0.1, 0, grid.z - 1);

		int kIndex = (illustratedQuadricCoord.x) * grid.y * grid.z +
					 (illustratedQuadricCoord.y) * grid.z +
					 (illustratedQuadricCoord.z);
		ImGui::Text("K = %f", kAndDistValues[kIndex].x);
		ImGui::Text("Dist = %f", kAndDistValues[kIndex].y);
	}

	if (ImGui::Button("Save eccentricity")) 
	{
		// SaveImageZ(eccentricityTexture, "Eccentricity.txt");
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