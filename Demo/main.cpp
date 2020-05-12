#include <Dragonfly/editor.h>
#include <Dragonfly/detail/buffer.h> //will be replaced
#include <Dragonfly/detail/vao.h> //will be replaced
#include <Dragonfly/detail/Texture/Texture3D.h>


int N_x = 32;
int N_y = 32;
int N_z = 32;

void recompute(df::ComputeProgramEditor& program, df::ComputeProgramEditor& sdfProgram, df::Texture3D<float>& sdf_values, df::Texture3D<float>& ecc) {
	sdfProgram << "s" << 1;
	glBindImageTexture(0, (GLuint)sdf_values, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	glDispatchCompute(N_x, N_y, N_z);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

	glBindImageTexture(0, (GLuint)ecc, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
	program << "sdf_values" << sdf_values;
	glDispatchCompute(N_x - 1, N_y - 1, N_z - 1);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

int main(int argc, char* args[])
{
	df::Sample sam; //handles Events and such
	df::Camera cam;
	cam.SetView(glm::vec3(5, 5, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	cam.SetSpeed(15);
	sam.AddHandlerClass(cam, 5);
	sam.AddStaticHandlerClass<df::ImGuiHandler>(10);

	df::ShaderProgramEditorVF program = "MyShaderProgram";
	program << "Shaders/sdf.frag"_frag << "Shaders/tracer.frag"_frag << "Shaders/vert.vert"_vert << "Shaders/frag.frag"_frag << df::LinkProgram;

	int w = df::Backbuffer.getWidth(), h = df::Backbuffer.getHeight();
	auto frameBuff = df::Renderbuffer<df::depth24>(w, h) + df::Texture2D<>(w, h, 1);

	df::Texture3D<float> sdf_values(N_x, N_y, N_z);
	df::Texture3D<float> ecc(N_x - 1, N_y - 1, N_z - 1);
	df::ComputeProgramEditor sdfComputeProgram = "SDFComputer";
	sdfComputeProgram << "Shaders/sdf.frag"_comp << "Shaders/sdf.compute"_comp << df::LinkProgram;

	df::ComputeProgramEditor eccComputeProgram = "EccentricityComputer";
	eccComputeProgram << "Shaders/tracer.frag"_comp << "Shaders/ecc.compute"_comp << df::LinkProgram;

	sam.AddResize([&](int w, int h) {frameBuff = frameBuff.MakeResized(w, h); });
	
	GL_CHECK; //extra opengl error checking in GPU Debug build configuration

	recompute(eccComputeProgram, sdfComputeProgram, sdf_values, ecc);

	sam.Run([&](float deltaTime) //delta time in ms
		{
			cam.Update();

			df::Backbuffer << df::Clear() << program << "k" << cosf(SDL_GetTicks()/1000.f) << "eye" << cam.GetEye() << "at" << cam.GetAt() << "up"
					<< cam.GetUp() << "windowSize" << glm::vec2(cam.GetSize().x,cam.GetSize().y) << "eccentricity" << ecc << "N" << glm::ivec3(N_x, N_y, N_z) << "sdf_values" << sdf_values;
			program << df::NoVao(GL_TRIANGLE_STRIP, 4);	

			GL_CHECK;
			program.Render(); //only the UI!!
			eccComputeProgram.Render(); sdfComputeProgram.Render();
			if (ImGui::Button("Recompute")) {
				recompute(eccComputeProgram, sdfComputeProgram, sdf_values, ecc);
			}
		}
	);
	return 0;
}
