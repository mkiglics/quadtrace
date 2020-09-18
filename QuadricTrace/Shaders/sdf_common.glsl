#version 450

float torus( vec3 p, float R, float r )
{
  vec2 q = vec2(length(p.xz)-R,p.y);
  return length(q)-r;
}

float pyramid( vec3 p, float h)
{
  float m2 = h*h + 0.25;
    
  p.xz = abs(p.xz);
  p.xz = (p.z>p.x) ? p.zx : p.xz;
  p.xz -= 0.5;

  vec3 q = vec3( p.z, h*p.y - 0.5*p.x, h*p.x + 0.5*p.y);
   
  float s = max(-q.x,0.0);
  float t = clamp( (q.y-0.5*p.z)/(m2+0.25), 0.0, 1.0 );
    
  float a = m2*(q.x+s)*(q.x+s) + q.y*q.y;
  float b = m2*(q.x+0.5*t)*(q.x+0.5*t) + (q.y-m2*t)*(q.y-m2*t);
    
  float d2 = min(q.y,-q.x*m2-q.y*0.5) > 0.0 ? 0.0 : min(a,b);
    
  return sqrt( (d2+q.z*q.z)/m2 ) * sign(max(q.z,-p.y));
}

#define SQRT2 1.4142135623
#define PI 3.14159265359
#define PI2 (2*3.14159265359)

#pragma warning(disable : 4008)

#define INF 3.402823466e+38    //whats the maximum?
#define CONVEXOPT_FPOS(f, vdf) vec2(f, (vdf < 0.0 ? -f/vdf : INF) )
#define CONVEXOPT(f, vdf) vec2(f, f < 0.0 ? f : (vdf < 0.0 ? -f/vdf : INF) )

// Planes
float plane(vec3 p, vec3 n)
{
    return dot(p, normalize(n));
}

// Axis aligned planes
float planeYZ(vec3 p){return p.x;}
float planeXZ(vec3 p){return p.y;}
float planeXY(vec3 p){return p.y;}

// Sphere
float sphere(vec3 p, float r){ return length(p) - r; }

// Infinite cylinders
float cylinderZ(vec3 p, float r){ return length(p.xy) - r; }
float cylinderX(vec3 p, float r){return cylinderZ(p.zyx,r);}
float cylinderY(vec3 p, float r){return cylinderZ(p.xzy,r);}

// Finite cylinders
float cylinderZ(vec3 p, vec2 h)
{
    vec2 d = abs(vec2(length(p.xy), abs(p.z))) - h;
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
}
float cylinderX(vec3 p, vec2 h){return cylinderZ(p.zyx,h);}
float cylinderY(vec3 p, vec2 h){return cylinderZ(p.xzy,h);}

// Box
float box(vec3 p, vec3 size)
{
    vec3 d = abs(p) - size;
    return min(max(d.x, max(d.y, d.z)), 0.0) + length(max(d, 0.0));
}

// ************************
//      SET OPERATIONS
// ________________________


float Offset(float f, float r){return f - r*0.1;}
float Union(float d1, float d2){ return min(d1,d2);}
float Intersect(float d1, float d2){ return max(d1,d2);}
float Substract(float d1, float d2){return max(d1,-d2);}

// ************************
//      Footmap stuff
// ________________________

vec4 fm_sphere(vec3 p, float r)
{
    float d = length(p);
    return vec4(p*(1.-r/d), d - r);
}

vec4 fm_planeZ(vec3 p) {return vec4(0, 0, -p.z, p.z);}
vec4 fm_planeY(vec3 p) {return fm_planeZ(p.xzy).xzyw;}
vec4 fm_planeX(vec3 p) {return fm_planeZ(p.zyx).zyxw;}

vec4 fm_box(vec3 p, vec3 b )
{
	vec3 d = abs(p) - b;
    return vec4(max(d,0.)*sign(p), length(max(d,0.))+min(max(d.x,max(d.y,d.z)),0.));
}


vec4 fm_cylinderZ(vec3 p, float r)
{
    float d = length(p.xy);
    return vec4(p.xy*(1.-r/d),0.,d-r);
}
vec4 fm_cylinderY(vec3 p, float r) {return fm_cylinderZ(p.xzy, r).xzyw;}
vec4 fm_cylinderX(vec3 p, float r) {return fm_cylinderZ(p.zyx, r).zyxw;}

vec4 fm_cylinderZ(vec3 p, vec2 rh)
{
    float len = length(p.xy);
	vec2 d = vec2(len,abs(p.z)) - rh;
    vec2 dm = max(d,0.);
	return vec4(dm.x*p.xy/len, p.z<0.?-dm.y:dm.y, min(max(d.x,d.y),0.0) + length(dm));
}
vec4 fm_cylinderY(vec3 p, vec2 rh) {return fm_cylinderZ(p.xzy, rh).xzyw;}
vec4 fm_cylinderX(vec3 p, vec2 rh) {return fm_cylinderZ(p.zyx, rh).zyxw;}
