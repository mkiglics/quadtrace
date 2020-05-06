#version 450

struct Ray
{
	vec3 P;
	float Tmin;
	vec3 V;
	float Tmax;
};

struct TraceResult
{
	float T;		// Distance taken on ray
	int flags;		// bit 0:   distance condition:     true if travelled to far t > t_max
					// bit 1:   surface condition:      true if distance to surface is small < error threshold
};                  // bit 2:   iteration condition:    true if took too many iterations

struct SphereTraceDesc
{
	float epsilon;  //Stopping distance to surface
	int maxiters;   //Maximum iteration count
};

Ray Camera(vec2 pix, vec3 eye, vec3 at, vec2 dim)
{    
	vec3 w = normalize(at-eye);
    vec3 u = normalize(cross(w,vec3(0,1,0)));
	vec3 v = cross(u,w);
    
    vec2 px = (pix*2.-1.)*1.*normalize(dim);
    
    return Ray(eye,							//P
               0.1,							//minT
               normalize(w+px.x*u+px.y*v),	//V
               100.);						//maxT
}

mat3 getRotation(vec3 normal)
{
	vec3 y = vec3(0, 1, 0);
	vec3 v = cross(normal, y);
	float c = dot(y, v);
	mat3 m = mat3(0, -v.z, v.y, v.z, 0, -v.x, -v.y, v.x, 0);
	return mat3(1) + m + m * m / (1 + c);
}


float getK(mat3 rot, vec3 surfaceDir)
{
	vec3 p = rot * surfaceDir;
	float t = atan(p.y / sqrt(p.x * p.x + p.z * p.z));
	float st = sin(t), ct = cos(t);
	float a = ct * ct + 2 * st * st;
	float b = -st * st;
	float c = p.y / st / st;
	float k1 = b * c / (1 + a * c);
	float k2 = b * c / (1 - a * c);
	return sign(k1) == sign(p.y) ? k1 : k2;
}

float getGeogebraK(vec2 pos)
{	
	if (pos.y > 0)
		return pos.y*pos.y/( pos.x*pos.x+2*pos.y*pos.y-pos.y);
	return pos.y*pos.y/(-pos.x*pos.x-2*pos.y*pos.y-pos.y);
}