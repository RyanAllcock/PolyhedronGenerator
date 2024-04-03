// WIP : can likely still improve; can select which vertices should produce outlines and how, using instances ? 
// WIP : ... triangle fan -> highest-area tri arrays (how to do with this other method? possibly using instances?) ? 
// WIP : keep individual methods in to test with
// WIP : how else to improve frame sharpness than replacing gradual shading as in method 2 ? answer on video ? 

#version 330 core
in vec3 geom_edgeDistance;
out vec4 frag_colour;
void main(){
	float dmin = min(geom_edgeDistance.x, min(geom_edgeDistance.y, geom_edgeDistance.z));
	vec4 faceColour = vec4(0.7, 0.7, 0, 1);
	vec4 wireColour = vec4(1, 1, 1, 1);
	float wireWidth = 1.5;
	
	// method 1: solid wireframe video method
	// float mixValue = 0.0;
	// if(dmin < wireWidth - 1) mixValue = 1.0;
	// else if(dmin > wireWidth + 1) mixValue = 0.0;
	// else{ float x = dmin - (wireWidth - 1); mixValue = exp2(-2.0 * x * x); }
	// frag_colour = mix(faceColour, wireColour, mixValue);
	
	// method 2: solid wireframe video method with sharp shading
	frag_colour = faceColour + (wireColour - faceColour) * step(0, wireWidth - dmin);
	
	// method 3
	// float I = exp2(-2.0 * dmin * dmin);
	// frag_colour = wireColour * I + faceColour * (1.0 - I);
};