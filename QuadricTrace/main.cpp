#include "QuadricRender.h"

int main(int argc, char* args[])
{

	QuadricRender renderer;
	renderer.Init(32);
	
	renderer.Render();

	/*renderer.RunSpeedTest("Benchmark/speed_base_data.txt", false, 1000, 100, {}, 5);
	renderer.RunSpeedTest("Benchmark/speed_quadric_0.txt", true, 1000, 100, {}, 5);
	renderer.RunSpeedTest("Benchmark/speed_quadric_1.txt", true, 1000, 100, {0.5f, 50, 0.01}, 5);	
	return 0;*/
	renderer.RunErrorTest("Benchmark/error_base_data.csv", false, 1, 1024, {}, 4);
	for (int i = 2; i < 1024; i *= 2)
	{
		renderer.RunErrorTest(("Benchmark/error_sphere_" + std::to_string(i) + "_steps.csv").c_str(), false, 1, i, {}, 4);
		renderer.RunErrorTest(("Benchmark/error_quadric_" + std::to_string(i) + "_steps.csv").c_str(), true, 1, i, {}, 4);
	}
	//renderer.Render();

	return 0;
}
