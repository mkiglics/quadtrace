#include "QuadricRender.h"
#include "CodeGen/Objects/models.h"
#include <sstream>
#include "configurables.h"

// #define TEST

MyExpr* models[] = {
	model1_expr(),
	model2_expr(),
	model3_expr(),
	model4_expr(),
	model5_expr(),
	model7_expr(),
	model8_expr(),
	model9_expr(),
	model10_expr(4)
};

glm::vec3 pos[] = {
	{2,1.1,2},
	{0.1,2,0.3},
	{2,1.1,2},
	{2,1.1,2},
	{0,3.1,0.1},
	{1,4.1,0.4},
	{1,3.1,0},
	{-2,3.1,-2},
	{9,9.1,9}
};

double EvalError(std::vector<float>& base, std::vector<float>& curr)
{
	double sum = 0;
	for (unsigned i = 0; i < base.size(); ++i)
	{
		if (base[i] == 1) continue;
		sum += abs(base[i] - curr[i]);
	}
	return (sum);
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
		if (line.size() == 0) break;
		std::istringstream is(line);
		QuadricRender::TestArg arg;
		int model;
		glm::vec3 eye, at;
		is >> model >> arg.max_frames >> arg.max_steps >> eye.x >> eye.y >> eye.z
			>> at.x >> at.y >> at.z;
		renderer.SetView(eye, at, glm::vec3(0, 1, 0));
		arg.model = models[model];
		arg.method = TraceTypes::sphere;
		out << renderer.RunSpeedTest(arg);
		arg.method = TraceTypes::enhanced;
		out << "," << renderer.RunSpeedTest(arg);
		arg.method = TraceTypes::relaxed;
		out << "," << renderer.RunSpeedTest(arg);
		arg.method = TraceTypes::quadric;
		arg.q_arg = { 0.003, 70, 0.01 };
		out << "," << renderer.RunSpeedTest(arg);
		arg.q_arg = { 0.003, 70, 0.1 };
		out << "," << renderer.RunSpeedTest(arg);
		out << "\n";
	}
	out.close();
	in.close();
}

void runErrorTests(const char* outfilename, QuadricRender& renderer)
{
	std::ofstream out(outfilename);
	for (int i = 0; i < 9; ++i)
	{
		if (i != 8) continue;
		renderer.SetView(pos[i], (i == 7 ? glm::vec3(2, 0, 2) : glm::vec3(0)), { 0,1,0 });
		std::vector<float> base = renderer.RunErrorTest({ models[i], 0, 1000, {}, TraceTypes::sphere });
		for (int n = 8; n <= 64; n *= 2)
		{
			std::vector<float> res = renderer.RunErrorTest({ models[i], 0, n, {1, 70, 0.01}, TraceTypes::sphere });
			out << EvalError(base, res) << ',';
			res = renderer.RunErrorTest({ models[i], 0, n, {1, 70, 0.01}, TraceTypes::relaxed });
			out << EvalError(base, res) << ',';
			res = renderer.RunErrorTest({ models[i], 0, n, {1, 70, 0.01}, TraceTypes::enhanced });
			out << EvalError(base, res) << ',';
			res = renderer.RunErrorTest({ models[i], 0, n, {0.003, 70, 0.01}, TraceTypes::quadric });
			out << EvalError(base, res) << ',';
			res = renderer.RunErrorTest({ models[i], 0, n, { 0.003, 70, 0.1 }, TraceTypes::quadric });
			out << EvalError(base, res) << ',';
		}
		out << '\n';
	}

	out.close();
}

int main(int argc, char* args[])
{

	QuadricRender renderer;
	renderer.Init(16);

#ifdef TEST
	runSpeedTests("test.txt", "Benchmark/out.txt", renderer);
	//runErrorTests("Benchmark/error.txt", renderer);
#else
	renderer.Render();
#endif


	return 0;
}
