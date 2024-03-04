// WIP : can't screen-space transform be avoided by just adding the original values as a vert_out variable ? 
// WIP : ... could then try and differentiate between face edges and inner edges in the buffer ? 
// WIP : still get edges not showing up every so often; keep the geom_edgeDistance extra line in to check for collapsed-origin'd non-flat faces

#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;
uniform mat4 screen;
noperspective out vec3 geom_edgeDistance;
void main(){
	
	vec4 temp;
	temp = gl_in[0].gl_Position;
	vec2 p0 = vec2(screen * (temp / temp.w));
	temp = gl_in[1].gl_Position;
	vec2 p1 = vec2(screen * (temp / temp.w));
	temp = gl_in[2].gl_Position;
	vec2 p2 = vec2(screen * (temp / temp.w));
	
	float a = length(p1 - p2);
	float b = length(p2 - p0);
	float c = length(p1 - p0);
	float alpha = acos((b * b + c * c - a * a) / (2.0 * b * c));
	float beta = acos((a * a + c * c - b * b) / (2.0 * a * c));
	float ca = abs(c * sin(beta));
	float cb = abs(c * sin(alpha));
	float cc = abs(b * sin(alpha));
	
	gl_Position = gl_in[0].gl_Position;
	// geom_edgeDistance = vec3(ca, 0, 0); // WIP : testing
	geom_edgeDistance = vec3(100, 100, 100);
	EmitVertex();
	
	gl_Position = gl_in[1].gl_Position;
	geom_edgeDistance = vec3(0, cb, 0);
	EmitVertex();
	
	gl_Position = gl_in[2].gl_Position;
	geom_edgeDistance = vec3(0, 0, cc);
	EmitVertex();
	
	EndPrimitive();
};