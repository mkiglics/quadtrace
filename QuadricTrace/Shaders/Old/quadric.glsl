#version 450

// returns the quadric's parameter
float getK(vec2 pos)
{
	// when x=0 getK would return inf or nan otherwise
	float x = max(abs(pos.x), 0.0001);
	return pos.y > 0 ?
		   solveQuadratic(x * x,  2 * pos.y * pos.y - pos.y, -pos.y * pos.y).y :
		   solveQuadratic(x * x, -2 * pos.y * pos.y - pos.y, -pos.y * pos.y).x;
}

// evaluating A(k), B(k) and C(k) functions
vec3 getABC(float k)
{
	return vec3(k*k, 2*abs(k)-1, -k);
}

// line and quadric intersections
vec2 intersectionPoints(vec3 ABC, vec3 p, vec3 v)
{
	float a = dot(ABC.xyx*v,v);
	float b = 2*dot(ABC.xyx*v,p) + ABC.z*v.y;
	float c = dot(ABC.xyx*p,p) + ABC.z*p.y;
	return solveQuadratic(a,b,c);
}

// ray and quadric intersection
float intersectQuadric(in vec3 p, in vec3 v, in float k)
{
	if (k < -0.99) return 0;
	vec3 ABC = getABC(k);
	vec2 t12 = intersectionPoints(ABC, p, v);
	float t1 = t12.x, t2 = t12.y;
	
	//remove one of the branches of a hyperboloid
	if( (p.y+t1*v.y)*k < 0 ) t1 = -inf;
	if( (p.y+t2*v.y)*k < 0 ) t2 = inf;

	float t = 0;
	if (k < 0) {
		if (t2 == inf) t = t1;
		else if (t1<0 && t1>-inf) t = t2;
		else if (t2 > 0) t = 0;
		else t = inf;
	} else  {
		if (t1==-inf) t = t2;
		else if (t2>0 && t2<inf) t = t1;
		else if (t1<0) t = inf;
	}
	return t;
}

// ray and quadric intersection
vec2 intersectQuadricBoth(in vec3 p, in vec3 v, in float k)
{
	if (k < -0.99) return vec2(0);
	return intersectionPoints(getABC(k), p, v);
}

vec3 quadricGetNormal(vec3 point, float k) {
	vec3 ABC = getABC(k);

	// from the implicit function's gradient
	return normalize(vec3(
		2 * ABC.x * point.x,
		2 * ABC.y * point.y + ABC.z,
		2 * ABC.x * point.z
	));
}
