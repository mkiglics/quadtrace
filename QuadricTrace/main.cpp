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

glm::vec3 pos[] = {
	{1.5,2.1,1.5},
	{0.1,  2,0.3},
	{  2,1.1,  2},
	{1.5,2.1,1.5},
	{1.5,2.1,1.5},
	{1.5,2.1,1.5},
	{1.5,2.1,1.5},
	{-3.5,2.1,-3.5},
	{1.5,2.1,1.5}
};

double EvalError(std::vector<float>& base, std::vector<float>& curr)
{
	double sum = 0;
	for (unsigned i = 0; i < base.size(); ++i)
	{
		if (base[i] == 0 || abs(base[i] - curr[i]) < 0.001) continue;
		sum += (base[i] - curr[i]) * (base[i] - curr[i]);
	}
	return sqrt(sum);
}

void runSpeedTests(const char* infilename, const char* outfilename, QuadricRender& renderer)
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

void runErrorTests(QuadricRender& renderer)
{
	double res[9];
	for (int i = 0; i < 6; ++i)
	{
		renderer.SetView(pos[i], { 0,0,0 }, { 0,1,0 });
		std::vector<float> base = renderer.RunErrorTest({ models[i], 0, 1000, {}, QuadricRender::SphereTrace::Simple });
		std::vector<float> res1 = renderer.RunErrorTest({ models[i], 0, 3, {1, 70, 0.01}, QuadricRender::SphereTrace::Enhanced});
		std::vector<float> res2 = renderer.RunErrorTest({ models[i], 0, 3, {1, 70, 0.01}, QuadricRender::SphereTrace::Quadric });
		res[i] = EvalError(base, res1) / EvalError(base, res2);
	}
	for (int i = 0; i < 9; ++i) std::cout << res[i] << ' ';
	std::cin.get();
}

int main(int argc, char* args[])
{

	QuadricRender renderer;
	renderer.Init(32);

#ifdef TEST
	//runSpeedTests("test.txt", "Benchmark/out.txt", renderer);
	runErrorTests(renderer);
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
