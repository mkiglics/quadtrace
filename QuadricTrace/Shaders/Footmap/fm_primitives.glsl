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