#version 410

#define SGN(a) (a < 0 ? -1 : 1)

const float MAX_ITERATIONS = 100;
const float MAX_DISTANCE = 10000;
const float EPSILON = 0.01;

uniform sampler3D eccentricity;
uniform sampler3D sdf_values;
uniform ivec3 N;

layout(location = 0) in vec2 fs_in_tex;
out vec4 fs_out_col;

uniform float r = 5;
uniform float _k = 1;
uniform vec3 lightPos = vec3(8, 8, 8);


float tanfov = tan(radians(45)/2);
uniform vec3 eye, at, up;
uniform vec2 windowSize;

float SDF(vec3 p)
{
	vec3 q = abs(p) - vec3(2,3,4);
	return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
	return length(p+vec3(0, 0, 0)) - r;
}

vec3 getNormal(vec3 p, vec3 dim) 
{
    return normalize(vec3(
        SDF(vec3(p.x + EPSILON, p.y, p.z)) - SDF(vec3(p.x - EPSILON, p.y, p.z)),
        SDF(vec3(p.x, p.y + EPSILON, p.z)) - SDF(vec3(p.x, p.y - EPSILON, p.z)),
        SDF(vec3(p.x, p.y, p.z  + EPSILON)) - SDF(vec3(p.x, p.y, p.z - EPSILON))
    ));
}

vec3 grad(vec3 coords, vec3 dim)
{
	float[8] vert;
	float[6] c;
	for (int x = 0; x < 2; ++x)
	{
		for (int y = 0; y < 2; ++y)
		{
			for (int z = 0; z < 2; ++z)
			{
				vert[x * 4 + y * 2 + z] = texelFetch(sdf_values, ivec3(clamp(coords, -dim/2.0+vec3(0.5), dim/2.0-vec3(0.5)) + vec3(x, y, z) + dim/2.0), 0).r;
				//vert[x * 4 + y * 2 + z] = SDF(coords + vec3(x, y, z));
			}
		}
	}
	c[0] = (vert[0] + vert[1] + vert[2] + vert[3]) / 4;
	c[1] = (vert[4] + vert[5] + vert[6] + vert[7]) / 4;
	c[2] = (vert[0] + vert[1] + vert[4] + vert[5]) / 4;
	c[3] = (vert[6] + vert[7] + vert[2] + vert[3]) / 4;
	c[4] = (vert[0] + vert[4] + vert[2] + vert[6]) / 4;
	c[5] = (vert[5] + vert[1] + vert[7] + vert[3]) / 4;


	vec3 norm = vec3(c[0] - c[1], c[2] - c[3], c[4] - c[5]);
	return length(norm) > EPSILON ? normalize(norm) : vec3(0, 1, 0);
}


float getT(vec3 p, vec3 v, float Tmax, float k)
{
	float Inf = Tmax;
	float a = abs(k);
	float b = 2*(abs(k)-0.5);
	float c = -k;
	float e1 = a*v.x*v.x+a*v.z*v.z+b*v.y*v.y;
	float e2 = 2*a*(p.x*v.x+p.z*v.z)+2*b*p.y*v.y+c*v.y;
	float e3 = a*(p.x*p.x+p.z*p.z) + b*p.y*p.y+c*p.y;
	float d = e2*e2-4*e1*e3;
	float tmin = -Tmax-1;
	float tmax = Tmax+1;
	if (d >= 0) {
		float t1 = (-e2-sqrt(d))/2.0/e1;
		float t2 = (-e2+sqrt(d))/2.0/e1;
		tmin = (abs(k) <= 0.5) ? (SGN((p+v*min(t1,t2)).y) != SGN(k) ? -Inf-1 : min(t1,t2)) : min(t1,t2);
		tmax = (abs(k) <= 0.5) ? (SGN((p+v*max(t1,t2)).y) != SGN(k) ?  Inf+1 : max(t1,t2)) : max(t1,t2);
	}
	float t = 0;
	if (k < -0.5) {
		if (d < 0 || tmin > 0 || tmax < 0) t = 0;
		else t = tmax;
	} else if (k < 0) {
		if (tmin > 0 && tmax > Inf) t = tmin;
		else if (tmax < 0 && tmin < -Inf) t = Inf;
		else if (tmin < 0 && tmin > -Inf && tmax < Inf && tmax > 0) t = tmax;
		else t = 0;
	} else if (k < 0.5) {
		if (tmin < -Inf && tmax > Inf) t = Inf;
		else if (tmin < -Inf && tmax > 0) t = tmax;
		else if (tmin < 0 && tmax > Inf) t = Inf;
		else if (tmin > 0 && tmax > Inf) t = 0;
		else if (tmin < -Inf && tmax < 0) t = 0;
		else if (tmax < 0) t = Inf;
		else if (tmin > 0) t = tmin;
		else t = 0;
	} else {
		if (d < 0 || tmax < 0) t = Inf;
		else if (tmin > 0) t = tmin;
		else t = 0;
	}
	return t;
}


TraceResult sphere_trace(in Ray ray, in SphereTraceDesc params)
{
    TraceResult ret = TraceResult(ray.Tmin, 0);
    float d;
    
    int i = 0; do
    {
        d = SDF(ray.P+ret.T*ray.V);
		//d = texture(eccentricity, (ray.P+ret.T*ray.V)/N+vec3(0.5)).r;
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

TraceResult quad_trace(in Ray ray, in SphereTraceDesc params)
{
    TraceResult ret = TraceResult(ray.Tmin, 0);
    float d;
	vec3 dir;
	float k;
    
    int i = 0; do
    {
		vec3 p = ray.P+ret.T*ray.V;
		dir = grad(p, N-vec3(1));
		ivec3 c = ivec3(round(p));
		k = texelFetch(eccentricity, clamp(ivec3(p+(N-vec3(2))/2.0), ivec3(0), N-ivec3(2)), 0).r;
		mat3 rot = getRotation(dir);
		float t = getT(rot*(p-c), rot*ray.V, ray.Tmax, k);
		d = max(t, SDF(p));
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

void main()
{
	Ray r = Camera(fs_in_tex, eye, at, windowSize);

	TraceResult res = quad_trace(r, SphereTraceDesc(0.01, 100));
	if (bool(res.flags & 1))		{fs_out_col = vec4(0,0,0,1); return;}
	if (bool(res.flags & 4))		{fs_out_col = vec4(1,0,0,1); return;}
	vec3 p = eye + r.V*res.T;
	vec3 ambient = vec3(0.1, 0.1, 0.1);
	vec3 n = getNormal(p,N);
	vec3 toLight = -normalize(p - lightPos);
	vec3 diffuse = vec3(0.6, 0.6, 0.6) * clamp(dot(n, toLight), 0, 1);

	fs_out_col = vec4(ambient+diffuse, 1);
	//fs_out_col = vec4(texture(eccentricity, p/(N-vec3(1))+vec3(0.5)).r, 0, 0, 1);//*vec4(ambient+diffuse,1);
}

