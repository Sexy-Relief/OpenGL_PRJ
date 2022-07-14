#version 400

struct LIGHT {
	vec4 position; // assume point or direction in EC in this example shader
	vec4 ambient_color, diffuse_color, specular_color;
	vec4 light_attenuation_factors; // compute this effect only if .w != 0.0f
	vec3 spot_direction;
	float spot_exponent;
	float spot_cutoff_angle;
	bool light_on;
};

struct MATERIAL {
	vec4 ambient_color;
	vec4 diffuse_color;
	vec4 specular_color;
	vec4 emissive_color;
	float specular_exponent;
};

uniform vec4 u_global_ambient_color_Go;
#define NUMBER_OF_LIGHTS_SUPPORTED 17
uniform LIGHT u_light_Go[NUMBER_OF_LIGHTS_SUPPORTED];
uniform MATERIAL u_material_Go;
const float zero_f = 0.0f;
const float one_f = 1.0f;

uniform mat4 u_ModelViewProjectionMatrix_Go;
uniform mat4 u_ModelViewMatrix_Go;
uniform mat3 u_ModelViewMatrixInvTrans_Go;  

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
out vec4 v_shaded_color;

vec4 lighting_equation(vec3 P_EC, vec3 N_EC) {
	vec4 color_sum;
	float local_scale_factor, tmp_float; 
	vec3 L_EC;
	color_sum = u_material_Go.emissive_color + u_global_ambient_color_Go * u_material_Go.ambient_color ;
	for (int i = 0; i < NUMBER_OF_LIGHTS_SUPPORTED; i++) {
		if (!u_light_Go[i].light_on) continue;
		local_scale_factor = one_f;
		if (u_light_Go[i].position.w != zero_f) { // point light source
			L_EC = u_light_Go[i].position.xyz - P_EC.xyz;

			if (u_light_Go[i].light_attenuation_factors.w  != zero_f) {
				vec4 tmp_vec4;

				tmp_vec4.x = one_f;
				tmp_vec4.z = dot(L_EC, L_EC);
				tmp_vec4.y = sqrt(tmp_vec4.z);
				tmp_vec4.w = zero_f;
				local_scale_factor = one_f/dot(tmp_vec4, u_light_Go[i].light_attenuation_factors);
			}

			L_EC = normalize(L_EC);

			if (u_light_Go[i].spot_cutoff_angle < 180.0f) { // [0.0f, 90.0f] or 180.0f
				float spot_cutoff_angle = clamp(u_light_Go[i].spot_cutoff_angle, zero_f, 90.0f);
				vec3 spot_dir = normalize(u_light_Go[i].spot_direction);

				tmp_float = dot(-L_EC, spot_dir);
				if (tmp_float >= cos(radians(spot_cutoff_angle))) {
					tmp_float = pow(tmp_float, u_light_Go[i].spot_exponent);
				}
				else 
					tmp_float = zero_f;
				local_scale_factor *= tmp_float;
			}
		}
		else {  // directional light source
			L_EC = normalize(u_light_Go[i].position.xyz);
		}	
		if (local_scale_factor > zero_f) {				
		 	vec4 local_color_sum = u_light_Go[i].ambient_color * u_material_Go.ambient_color;
			tmp_float = dot(N_EC, L_EC);
			if (tmp_float>zero_f) { 
				local_color_sum += u_light_Go[i].diffuse_color*u_material_Go.diffuse_color*tmp_float;
				vec3 H_EC = normalize(L_EC - normalize(P_EC));
				tmp_float = dot(N_EC, H_EC); 
				if (tmp_float > zero_f) {
					local_color_sum += u_light_Go[i].specular_color
				                       *u_material_Go.specular_color*pow(tmp_float, u_material_Go.specular_exponent);
				}
			}
			color_sum += local_scale_factor*local_color_sum;
		}
	}
 	return color_sum;
}

void main(void) {	
	vec3 v_position_EC = vec3(u_ModelViewMatrix_Go*vec4(a_position, 1.0f));
	vec3 v_normal_EC = normalize(u_ModelViewMatrixInvTrans_Go*a_normal);  
	v_shaded_color= lighting_equation(v_position_EC,v_normal_EC);
	gl_Position = u_ModelViewProjectionMatrix_Go*vec4(a_position, 1.0f);
}