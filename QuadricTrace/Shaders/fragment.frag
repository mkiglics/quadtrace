#version 450

const float MAX_ITERATIONS = 100;
const float MAX_DISTANCE = 10000;
const float EPSILON = 0.01;

uniform sampler3D eccentricity;
uniform sampler3D sdf_values;
uniform ivec3 N;

layout(location = 0) in vec2 fs_in_tex;
out vec4 fs_out_col;

uniform int max_iter = 150;
uniform bool render_quadric = true;
uniform float delta = 0.8;

uniform vec3 lightPos = vec3(50, 50, 50);


float tanfov = tan(radians(45)/2);
uniform vec3 eye, at, up;
uniform vec2 windowSize;


vec3 getNormal(vec3 p, vec3 dim) 
{
    return normalize(vec3(
        SDF(vec3(p.x + EPSILON, p.y, p.z)) - SDF(vec3(p.x - EPSILON, p.y, p.z)),
        SDF(vec3(p.x, p.y + EPSILON, p.z)) - SDF(vec3(p.x, p.y - EPSILON, p.z)),
        SDF(vec3(p.x, p.y, p.z  + EPSILON)) - SDF(vec3(p.x, p.y, p.z - EPSILON))
    ));
}

vec3 gradient(in ivec3 coords)
{
	float[8] vert;
	float[6] c;
	for (int x = 0; x < 2; ++x)
	{
		for (int y = 0; y < 2; ++y)
		{
			for (int z = 0; z < 2; ++z)
			{
				vert[x*4+y*2+z]=texelFetch(sdf_values, coords+ivec3(x,y,z), 0).r;
			}
		}
	}
	c[0] = (vert[0] + vert[1] + vert[2] + vert[3]);
	c[1] = (vert[4] + vert[5] + vert[6] + vert[7]);
	c[2] = (vert[0] + vert[1] + vert[4] + vert[5]);
	c[3] = (vert[6] + vert[7] + vert[2] + vert[3]);
	c[4] = (vert[0] + vert[4] + vert[2] + vert[6]);
	c[5] = (vert[5] + vert[1] + vert[7] + vert[3]);


	vec3 norm = vec3(c[0] - c[1], c[2] - c[3], c[4] - c[5]);
	return length(norm) > EPSILON ? normalize(norm) : vec3(0, 1, 0);
}


TraceResult sphere_trace(in Ray ray, in SphereTraceDesc params)
{
    TraceResult ret = TraceResult(ray.Tmin, 0);
    float d;
    
    int i = 0; do
    {
        d = SDF(ray.P+ret.T*ray.V);
        ret.T+=d;
        ++i;
    } while (
		ret.T < ray.Tmax &&       			// Stay within bound box
		d	  > params.epsilon * ret.T &&	// Stop if cone is close to surface
		i     < params.maxiters	        	// Stop if too many iterations
	);
    
    ret.flags =  int(ret.T >= ray.Tmax)
              | (int(d <= params.epsilon* ret.T)  << 1)
              | (int(i >= params.maxiters) << 2);
    return ret;
}

TraceResult quadric_trace(in Ray ray, in SphereTraceDesc params)
{
    TraceResult ret = TraceResult(ray.Tmin, 0);
    float d;
	vec3 dir, p;
	ivec3 c;
	float k, t;
	mat3 rot;
    
    int i = 0; do
    {
		p = ray.P+ret.T*ray.V;

		//center of the current cell
		c =  globalToTexel(p, N-ivec3(1));
		//direction of the surface
		dir = gradient(c);
		//parameter of the quadric
		k = texelFetch(eccentricity, c, 0).r;
		rot = getRotation(dir);
		//the parameter of the closest intersection point
		t = intersectQuadric(rot*(p-round(p)), rot*ray.V, k);
		d = max(t, texelFetch(sdf_values, globalToTexel(p, N), 0).r-0.8);
		if (d<delta) {						// use SDF if too close to surface
			d = max(t, SDF(ray.P+ret.T*ray.V));
		}
		ret.T += d;
        ++i;
    } while (
		ret.T < ray.Tmax &&       			// Stay within bound box
		d	  > params.epsilon * ret.T &&	// Stop if close to surface
		i     < params.maxiters	        	// Stop if too many iterations
	);
    
    ret.flags =  int(ret.T >= ray.Tmax)
              | (int(d <= params.epsilon* ret.T)  << 1)
              | (int(i >= params.maxiters) << 2);
    return ret;
}

vec3 lighting(in vec3 p)
{

	vec3 ambient = vec3(0.1, 0.1, 0.1);
	vec3 n = getNormal(p,N);
	vec3 toLight = -normalize(p - lightPos);
	vec3 diffuse = vec3(0.6, 0.6, 0.6) * clamp(dot(n, toLight), 0, 1);
	vec3 lightDir = p-lightPos;
	vec3 ref = normalize(reflect(lightDir, n));
	vec3 e = normalize(eye - p);
	vec3 specular = vec3(1) * pow(clamp(dot(ref,e), 0, 1), 32);

	return ambient + diffuse + specular;
}

void main()
{
	Ray r = Camera(fs_in_tex, eye, at, windowSize);

	TraceResult res = render_quadric?quadric_trace(r, SphereTraceDesc(0.001, max_iter)):sphere_trace(r, SphereTraceDesc(0.001, max_iter));
	if (bool(res.flags & 1))		{fs_out_col = vec4(0,0,0,1); return;}
	if (bool(res.flags & 4))		{fs_out_col = vec4(1,0,0,1); return;}
	vec3 p = eye + r.V*res.T;

	vec3 color = lighting(p);
	fs_out_col = vec4(color, 1);
}
