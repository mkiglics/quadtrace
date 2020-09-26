#include "QuadricRender.h"
#include "CodeGen/Objects/models.h"
#include <sstream>

#define TEST

MyExpr* models[] = {
	model1_expr(),
	model2_expr(),
	model3_expr(),
	model4_expr(),
	model5_expr(),
	model7_expr(),
	model8_expr(),
	model9_expr(),
	model10_expr()
};

void runTests(const char* infilename, const char* outfilename, QuadricRender& renderer)
{
	std::ifstream in(infilename);
	if (!in.is_open()) return;

	std::ofstream out(outfilename);
	std::string line;
	std::getline(in, line);
	while (!in.eof())
	{
		std::getline(in, line);
		if (line.size() == 0) continue;
		std::istringstream is(line);
		QuadricRender::TestArg arg;
		int model, method;
		glm::vec3 eye, at;
		is >> model >> method >> arg.max_frames >> arg.max_steps >> eye.x >> eye.y >> eye.z
			>> at.x >> at.y >> at.z;
		renderer.SetView(eye, at, glm::vec3(0, 1, 0));
		arg.model = models[model];
		if (method == 0) {
			arg.method = QuadricRender::SphereTrace::Quadric;
			arg.q_arg = {0.8, 70, 0.01};
			out << renderer.RunSpeedTest(arg);
			arg.q_arg = { 1, 70, 0.01 };
			out << "," << renderer.RunSpeedTest(arg);
			arg.q_arg = { 0.8, 70, 0.0 };
			out << "," << renderer.RunSpeedTest(arg);
			arg.q_arg = { 0.5, 70, 0.01 };
			out << "," << renderer.RunSpeedTest(arg);
			arg.q_arg = { 0.8, 70, 0.1 };
			out << "," << renderer.RunSpeedTest(arg);
			arg.q_arg = { 1, 30, 0.01 };
			out << "," << renderer.RunSpeedTest(arg);
		}
		else
		{
			arg.method = QuadricRender::SphereTrace::Simple;
			out << renderer.RunSpeedTest(arg);
			arg.method = QuadricRender::SphereTrace::Enhanced;
			out << "," << renderer.RunSpeedTest(arg);
			arg.method = QuadricRender::SphereTrace::Relaxed;
			out << "," << renderer.RunSpeedTest(arg);
		}
		out << "\n";
	}
	out.close();
	in.close();
}

int main(int argc, char* args[])
{

	QuadricRender renderer;
	renderer.Init(32);

#ifdef TEST
	runTests("test.txt", "out.txt", renderer);
#else
	renderer.Render();
#endif


	//renderer.Render();

	/*renderer.RunSpeedTest("Benchmark/speed_base_data.txt", false, 1000, 100, {}, 5);
	renderer.RunSpeedTest("Benchmark/speed_quadric_0.txt", true, 1000, 100, {}, 5);
	renderer.RunSpeedTest("Benchmark/speed_quadric_1.txt", true, 1000, 100, {0.5f, 50, 0.01}, 5);	
	return 0;*/
	/*
	renderer.RunErrorTest("Benchmark/error_base_data.csv", false, 1, 1024, {}, 4);
	for (int i = 2; i < 1024; i *= 2)
	{
		renderer.RunErrorTest(("Benchmark/error_sphere_" + std::to_string(i) + "_steps.csv").c_str(), false, 1, i, {}, 4);
		renderer.RunErrorTest(("Benchmark/error_quadric_" + std::to_string(i) + "_steps.csv").c_str(), true, 1, i, {}, 4);
	}
	*/
	//renderer.Render();

	return 0;
}
