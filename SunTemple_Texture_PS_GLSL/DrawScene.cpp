//
//  DrawScene.cpp
//
//  Written for CSE4170
//  Department of Computer Science and Engineering
//  Copyright © 2022 Sogang University. All rights reserved.
//

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/gtc/matrix_inverse.hpp>
#include "LoadScene.h"

// Begin of shader setup
#include "Shaders/LoadShaders.h"
#include "ShadingInfo.h"

extern SCENE scene;

// for simple shaders
GLuint h_ShaderProgram_simple, h_ShaderProgram_TXPS,h_ShaderProgram_Go; // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

// for Phong Shading (Textured) shaders
#define NUMBER_OF_LIGHT_SUPPORTED 17
GLint loc_global_ambient_color;
loc_light_Parameters loc_light[NUMBER_OF_LIGHT_SUPPORTED];
loc_Material_Parameters loc_material;
GLint loc_ModelViewProjectionMatrix_TXPS, loc_ModelViewMatrix_TXPS, loc_ModelViewMatrixInvTrans_TXPS;
GLint loc_texture;
GLint loc_flag_texture_mapping;
GLint loc_flag_fog;
GLint loc_u_fragment_alpha, loc_u_alpha_mode;
// for Gouraud Shading shaders
GLint loc_global_ambient_color_Go;
loc_light_Parameters loc_light_Go[NUMBER_OF_LIGHT_SUPPORTED];
loc_Material_Parameters loc_material_Go;
GLint loc_ModelViewProjectionMatrix_Go, loc_ModelViewMatrix_Go, loc_ModelViewMatrixInvTrans_Go;
// include glm/*.hpp only if necessary
// #include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.
// ViewProjectionMatrix = ProjectionMatrix * ViewMatrix
glm::mat4 ViewProjectionMatrix, ViewMatrix, ProjectionMatrix;
// ModelViewProjectionMatrix = ProjectionMatrix * ViewMatrix * ModelMatrix
glm::mat4 ModelViewProjectionMatrix; // This one is sent to vertex shader when ready.
glm::mat4 ModelViewMatrix;
glm::mat3 ModelViewMatrixInvTrans;
glm::mat4 TempMatrix;

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f

glm::mat4 ModelMatrix_tiger;
glm::mat4 ModelMatrix_tiger_head;
glm::mat4 Matrix_tiger_camera;
glm::vec4 user_light1, user_light2, user_light3,user_light4;
glm::vec3 user_dir1, user_dir2, user_dir3,user_dir4;
glm::vec4 tempv4,tempv4_2,tempv4_3,tempv4_4;
glm::vec3 tempv3,tempv3_2,tempv3_3,tempv3_4;
bool tiger_head = false;
bool tiger_back = false;
bool tiger_nod = false;
bool nod_dir_up = true;
int nod_angle = -90;
int user_light_on = 1;
int eye_light_on = 1;
int mod_light_on = 1;
bool alpha_mode = 1;
int cube_timestamp = 0;

#define N_TEXTURES_USED 3
#define TEXTURE_ID_TANK 1
#define TEXTURE_ID_TIGER 2
GLuint texture_names[N_TEXTURES_USED];
/*********************************  START: camera *********************************/
typedef enum {
	CAMERA_0,
	CAMERA_1,
	CAMERA_2,
	CAMERA_3,
	CAMERA_M,
	NUM_CAMERAS
} CAMERA_INDEX;

typedef struct _Camera {
	float pos[3];
	float uaxis[3], vaxis[3], naxis[3];
	float fovy, aspect_ratio, near_c, far_c;
	int move, rotation_axis;
} Camera;

Camera camera_info[NUM_CAMERAS];
Camera current_camera;
bool move_mode = false;

using glm::mat4;
void set_user_light() {
	glUseProgram(h_ShaderProgram_TXPS);
	for (int i = 13; i < 14; i++) {

		glUniform1i(loc_light[i].light_on, user_light_on);
		user_light1 = glm::vec4(0.0f, -200.0f, 600.0f, 1.0f);
		tempv4 = ViewMatrix * user_light1;
		glUniform4fv(loc_light[i].position, 1, &tempv4[0]);
		glUniform4f(loc_light[i].ambient_color, 0.2f, 0.2f, 0.2f, 1.0f);
		glUniform4f(loc_light[i].diffuse_color, 1.0f, 1.0f, 1.0f, 1.0f);
		glUniform4f(loc_light[i].specular_color, 1.0f, 1.0f, 1.0f, 1.0f);
		user_dir1 = glm::vec3(0.0f, 0.0f, -1.0f);
		tempv3 = glm::mat3(ViewMatrix) * user_dir1;
		glUniform3fv(loc_light[i].spot_direction, 1, &tempv3[0]);
		glUniform1f(loc_light[i].spot_exponent, 20.0f); // [0.0, 128.0]
		glUniform1f(loc_light[i].spot_cutoff_angle, 20.0f); // [0.0, 90.0] or 180.0 (180.0 for no spot light effect)
		glUniform4f(loc_light[i].light_attenuation_factors, 1.0f, 0.0f, 0.00001f, 1.0f); // .w != 0.0f for no ligth attenuation
	}
	for (int i = 14; i < 15; i++) {
		glUniform1i(loc_light[i].light_on, user_light_on);
		user_light2 = glm::vec4(650.0f, -2000.0f, 400.0f, 1.0f);
		tempv4_2 = ViewMatrix * user_light2;
		glUniform4fv(loc_light[i].position, 1, &tempv4_2[0]);
		glUniform4f(loc_light[i].ambient_color, 0.2f, 0.2f, 0.2f, 1.0f);
		glUniform4f(loc_light[i].diffuse_color, 1.0f, 1.0f, 1.0f, 1.0f);
		glUniform4f(loc_light[i].specular_color, 1.0f, 1.0f, 1.0f, 1.0f);
		user_dir2 = glm::vec3(0.0f, 0.0f, -1.0f);
		tempv3_2 = glm::mat3(ViewMatrix) * user_dir2;
		glUniform3fv(loc_light[i].spot_direction, 1, &tempv3_2[0]);
		glUniform1f(loc_light[i].spot_exponent, 20.0f); // [0.0, 128.0]
		glUniform1f(loc_light[i].spot_cutoff_angle, 180.0f); // [0.0, 90.0] or 180.0 (180.0 for no spot light effect)
		glUniform4f(loc_light[i].light_attenuation_factors, 1.0f, 0.0f, 0.00001f, 1.0f); // .w != 0.0f for no ligth attenuation
	}
	for (int i = 15; i < 16; i++) {
		glUniform1i(loc_light[i].light_on, eye_light_on);
		glUniform4f(loc_light[i].position, 0.0f,0.0f,0.0f,1.0f);
		glUniform4f(loc_light[i].ambient_color, 0.2f, 0.2f, 0.2f, 1.0f);
		glUniform4f(loc_light[i].diffuse_color, 1.0f, 1.0f, 1.0f, 1.0f);
		glUniform4f(loc_light[i].specular_color, 1.0f, 1.0f, 1.0f, 1.0f);
		glUniform3f(loc_light[i].spot_direction, 0.0f, 0.0f, -1.0f);
		glUniform1f(loc_light[i].spot_exponent, 20.0f); // [0.0, 128.0]
		glUniform1f(loc_light[i].spot_cutoff_angle, 17.0f); // [0.0, 90.0] or 180.0 (180.0 for no spot light effect)
		glUniform4f(loc_light[i].light_attenuation_factors, 1.0f, 0.0f, 0.00001f, 1.0f); // .w != 0.0f for no ligth attenuation
	}
	for (int i = 16; i < 17; i++) {
		glUniform1i(loc_light[i].light_on, mod_light_on);
		user_light3= glm::vec4(0.0f, 0.0f, 100.0f, 1.0f);
		TempMatrix = ViewMatrix * ModelMatrix_tiger;
		tempv4_3 = TempMatrix * user_light3;
		glUniform4fv(loc_light[i].position, 1,&tempv4_3[0]);
		glUniform4f(loc_light[i].ambient_color, 0.2f, 0.2f, 0.2f, 1.0f);
		glUniform4f(loc_light[i].diffuse_color, 1.0f, 1.0f, 1.0f, 1.0f);
		glUniform4f(loc_light[i].specular_color, 1.0f, 1.0f, 1.0f, 1.0f);
		user_dir3 = glm::vec3(0.0f, -1.0f, 0.0f);
		tempv3_3 = glm::mat3(TempMatrix) * user_dir3;
		glUniform3fv(loc_light[i].spot_direction, 1,&tempv3_3[0]);
		glUniform1f(loc_light[i].spot_exponent, 20.0f); // [0.0, 128.0]
		glUniform1f(loc_light[i].spot_cutoff_angle, 17.0f); // [0.0, 90.0] or 180.0 (180.0 for no spot light effect)
		glUniform4f(loc_light[i].light_attenuation_factors, 1.0f, 0.0f, 0.00001f, 1.0f); // .w != 0.0f for no ligth attenuation
	}
	glUseProgram(h_ShaderProgram_Go);
	for (int i = 14; i < 15; i++) {
		glUniform1i(loc_light_Go[i].light_on, user_light_on);
		user_light4 = glm::vec4(650.0f, -2000.0f, 400.0f, 1.0f);
		tempv4_4 = ViewMatrix * user_light4;
		glUniform4fv(loc_light_Go[i].position, 1, &tempv4_4[0]);
		glUniform4f(loc_light_Go[i].ambient_color, 1.0f, 1.0f, 1.0f, 1.0f);
		glUniform4f(loc_light_Go[i].diffuse_color, 1.0f, 1.0f, 1.0f, 1.0f);
		glUniform4f(loc_light_Go[i].specular_color, 1.0f, 1.0f, 1.0f, 1.0f);
		user_dir4 = glm::vec3(0.0f, 0.0f, -1.0f);
		tempv3_4 = glm::mat3(ViewMatrix) * user_dir4;
		glUniform3fv(loc_light_Go[i].spot_direction, 1, &tempv3_4[0]);
		glUniform1f(loc_light_Go[i].spot_exponent, 20.0f); // [0.0, 128.0]
		glUniform1f(loc_light_Go[i].spot_cutoff_angle, 180.0f); // [0.0, 90.0] or 180.0 (180.0 for no spot light effect)
		glUniform4f(loc_light_Go[i].light_attenuation_factors, 1.0f, 0.0f, 0.00001f, 1.0f); // .w != 0.0f for no ligth attenuation
	}
	for (int i = 15; i < 17; i++) {
		glUniform1i(loc_light_Go[i].light_on, 0);
	}
	glUseProgram(0);

}

void set_ViewMatrix_from_camera_frame(void) {
	ViewMatrix = glm::mat4(current_camera.uaxis[0], current_camera.vaxis[0], current_camera.naxis[0], 0.0f,
		current_camera.uaxis[1], current_camera.vaxis[1], current_camera.naxis[1], 0.0f,
		current_camera.uaxis[2], current_camera.vaxis[2], current_camera.naxis[2], 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	ViewMatrix = glm::translate(ViewMatrix, glm::vec3(-current_camera.pos[0], -current_camera.pos[1], -current_camera.pos[2]));
	printf("%f %f %f\n", current_camera.pos[0], current_camera.pos[1], current_camera.pos[2]);
	set_user_light();
}

void set_current_camera(int camera_num) {
	Camera* pCamera = &camera_info[camera_num];
	memcpy(&current_camera, pCamera, sizeof(Camera));
	ProjectionMatrix = glm::perspective(current_camera.fovy, current_camera.aspect_ratio,
		current_camera.near_c, current_camera.far_c);
	set_ViewMatrix_from_camera_frame();
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
}

void initialize_camera(void) {
	//CAMERA_0 : initial camera
	Camera* pCamera = &camera_info[CAMERA_0];
	pCamera->pos[0] = 733.748657f; pCamera->pos[1] = -3679.084473f; pCamera->pos[2] = 1176.228516f;
	pCamera->uaxis[0] = 0.881656f; pCamera->uaxis[1] = 0.454952f; pCamera->uaxis[2] = -0.125442f;
	pCamera->vaxis[0] = -0.215361f; pCamera->vaxis[1] = 0.624382f; pCamera->vaxis[2] = 0.750858f;
	pCamera->naxis[0] = 0.419926f; pCamera->naxis[1] = -0.634979f; pCamera->naxis[2] = 0.648464f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = (scene.camera.aspect), pCamera->near_c = 0.1f; pCamera->far_c = 30000.0f;

	//CAMERA_1
	pCamera = &camera_info[CAMERA_1];
	pCamera->pos[0] = -41.199409f; pCamera->pos[1] = 328.475220f; pCamera->pos[2] = 2275.126709f;
	pCamera->uaxis[0] = -0.977975f; pCamera->uaxis[1] = 0.193060f; pCamera->uaxis[2] = -0.079481f;
	pCamera->vaxis[0] = -0.205206f; pCamera->vaxis[1] = -0.958985f; pCamera->vaxis[2] = 0.195603f;
	pCamera->naxis[0] = -0.038458f; pCamera->naxis[1] = 0.207604f; pCamera->naxis[2] = 0.977472f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 30000.0f;

	//CAMERA_2
	pCamera = &camera_info[CAMERA_2];
	pCamera->pos[0] = 26.795864f; pCamera->pos[1] = -7180.814453f; pCamera->pos[2] = 951.206665;
	pCamera->uaxis[0] = 0.999672f; pCamera->uaxis[1] = 0.025581f; pCamera->uaxis[2] = -0.003558;
	pCamera->vaxis[0] = -0.011069f; pCamera->vaxis[1] = 0.548807f; pCamera->vaxis[2] = 0.835880f;
	pCamera->naxis[0] = 0.023333f; pCamera->naxis[1] = -0.835563f; pCamera->naxis[2] = 0.548909f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 30000.0f;

	//CAMERA_3
	pCamera = &camera_info[CAMERA_3];
	pCamera->pos[0] = 0.0f; pCamera->pos[1] = 0.0f; pCamera->pos[2] = 0.0f;
	pCamera->uaxis[0] = -1.0f; pCamera->uaxis[1] = 0.0f; pCamera->uaxis[2] = 0.0f;
	pCamera->vaxis[0] = 0.0f; pCamera->vaxis[1] = 0.0f; pCamera->vaxis[2] = 1.0f;
	pCamera->naxis[0] = 0.0f; pCamera->naxis[1] = 1.0f; pCamera->naxis[2] = 0.0f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 30000.0f;

	//CAMERA_M : moving view
	pCamera = &camera_info[CAMERA_M];
	pCamera->pos[0] = 0.0f; pCamera->pos[1] = -200.0f; pCamera->pos[2] = 400.0f;
	pCamera->uaxis[0] = -1.0f; pCamera->uaxis[1] = 0.0f; pCamera->uaxis[2] = 0.0f;
	pCamera->vaxis[0] = 0.0f; pCamera->vaxis[1] = 0.0f; pCamera->vaxis[2] = 1.0f;
	pCamera->naxis[0] = 0.0f; pCamera->naxis[1] = 1.0f; pCamera->naxis[2] = 0.0f;
	pCamera->move = 0;
	pCamera->fovy = TO_RADIAN * scene.camera.fovy, pCamera->aspect_ratio = scene.camera.aspect, pCamera->near_c = 0.1f; pCamera->far_c = 30000.0f;


	set_current_camera(CAMERA_0);
}
/*********************************  END: camera *********************************/

/******************************  START: shader setup ****************************/
// Begin of Callback function definitions
void prepare_shader_program(void) {
	char string[256];

	ShaderInfo shader_info[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};

	ShaderInfo shader_info_TXPS[3] = {
	{ GL_VERTEX_SHADER, "Shaders/Phong_Tx.vert" },
	{ GL_FRAGMENT_SHADER, "Shaders/Phong_Tx.frag" },
	{ GL_NONE, NULL }
	};

	ShaderInfo shader_info_Go[3] = {
	{ GL_VERTEX_SHADER, "Shaders/Gouraud.vert" },
	{ GL_FRAGMENT_SHADER, "Shaders/Gouraud.frag" },
	{ GL_NONE, NULL }
	};

	h_ShaderProgram_simple = LoadShaders(shader_info);
	glUseProgram(h_ShaderProgram_simple);

	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram_simple, "u_ModelViewProjectionMatrix");
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram_simple, "u_primitive_color");

	h_ShaderProgram_TXPS = LoadShaders(shader_info_TXPS);
	glUseProgram(h_ShaderProgram_TXPS);
	loc_ModelViewProjectionMatrix_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_TXPS = glGetUniformLocation(h_ShaderProgram_TXPS, "u_ModelViewMatrixInvTrans");

	loc_global_ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_global_ambient_color");

	for (int i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		sprintf(string, "u_light[%d].light_on", i);
		loc_light[i].light_on = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].position", i);
		loc_light[i].position = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].ambient_color", i);
		loc_light[i].ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].diffuse_color", i);
		loc_light[i].diffuse_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].specular_color", i);
		loc_light[i].specular_color = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].spot_direction", i);
		loc_light[i].spot_direction = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].spot_exponent", i);
		loc_light[i].spot_exponent = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].spot_cutoff_angle", i);
		loc_light[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_TXPS, string);
		sprintf(string, "u_light[%d].light_attenuation_factors", i);
		loc_light[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram_TXPS, string);
	}

	loc_material.ambient_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.ambient_color");
	loc_material.diffuse_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.diffuse_color");
	loc_material.specular_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.specular_color");
	loc_material.emissive_color = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.emissive_color");
	loc_material.specular_exponent = glGetUniformLocation(h_ShaderProgram_TXPS, "u_material.specular_exponent");
	loc_texture = glGetUniformLocation(h_ShaderProgram_TXPS, "u_base_texture");
	loc_flag_texture_mapping = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_texture_mapping");
	loc_flag_fog = glGetUniformLocation(h_ShaderProgram_TXPS, "u_flag_fog");
	loc_u_fragment_alpha = glGetUniformLocation(h_ShaderProgram_TXPS, "u_fragment_alpha");
	loc_u_alpha_mode = glGetUniformLocation(h_ShaderProgram_TXPS, "u_alpha_mode");


	h_ShaderProgram_Go = LoadShaders(shader_info_Go);
	glUseProgram(h_ShaderProgram_Go);
	loc_ModelViewProjectionMatrix_Go = glGetUniformLocation(h_ShaderProgram_Go, "u_ModelViewProjectionMatrix_Go");
	loc_ModelViewMatrix_Go = glGetUniformLocation(h_ShaderProgram_Go, "u_ModelViewMatrix_Go");
	loc_ModelViewMatrixInvTrans_Go = glGetUniformLocation(h_ShaderProgram_Go, "u_ModelViewMatrixInvTrans_Go");

	loc_global_ambient_color_Go = glGetUniformLocation(h_ShaderProgram_Go, "u_global_ambient_color_Go");

	for (int i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		sprintf(string, "u_light_Go[%d].light_on", i);
		loc_light_Go[i].light_on = glGetUniformLocation(h_ShaderProgram_Go, string);
		sprintf(string, "u_light_Go[%d].position", i);
		loc_light_Go[i].position = glGetUniformLocation(h_ShaderProgram_Go, string);
		sprintf(string, "u_light_Go[%d].ambient_color", i);
		loc_light_Go[i].ambient_color = glGetUniformLocation(h_ShaderProgram_Go, string);
		sprintf(string, "u_light_Go[%d].diffuse_color", i);
		loc_light_Go[i].diffuse_color = glGetUniformLocation(h_ShaderProgram_Go, string);
		sprintf(string, "u_light_Go[%d].specular_color", i);
		loc_light_Go[i].specular_color = glGetUniformLocation(h_ShaderProgram_Go, string);
		sprintf(string, "u_light_Go[%d].spot_direction", i);
		loc_light_Go[i].spot_direction = glGetUniformLocation(h_ShaderProgram_Go, string);
		sprintf(string, "u_light_Go[%d].spot_exponent", i);
		loc_light_Go[i].spot_exponent = glGetUniformLocation(h_ShaderProgram_Go, string);
		sprintf(string, "u_light_Go[%d].spot_cutoff_angle", i);
		loc_light_Go[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_Go, string);
		sprintf(string, "u_light_Go[%d].light_attenuation_factors", i);
		loc_light_Go[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram_Go, string);
	}

	loc_material_Go.ambient_color = glGetUniformLocation(h_ShaderProgram_Go, "u_material_Go.ambient_color");
	loc_material_Go.diffuse_color = glGetUniformLocation(h_ShaderProgram_Go, "u_material_Go.diffuse_color");
	loc_material_Go.specular_color = glGetUniformLocation(h_ShaderProgram_Go, "u_material_Go.specular_color");
	loc_material_Go.emissive_color = glGetUniformLocation(h_ShaderProgram_Go, "u_material_Go.emissive_color");
	loc_material_Go.specular_exponent = glGetUniformLocation(h_ShaderProgram_Go, "u_material_Go.specular_exponent");
	glUseProgram(0);
}
/*******************************  END: shder setup ******************************/

/****************************  START: geometry setup ****************************/
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))
#define INDEX_VERTEX_POSITION	0
#define INDEX_NORMAL			1
#define INDEX_TEX_COORD			2

bool b_draw_grid = false;

//axes
GLuint axes_VBO, axes_VAO;
GLfloat axes_vertices[6][3] = {
	{ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }
};
GLfloat axes_color[3][3] = { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };

void prepare_axes(void) {
	// Initialize vertex buffer object.
	glGenBuffers(1, &axes_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, axes_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes_vertices), &axes_vertices[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &axes_VAO);
	glBindVertexArray(axes_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, axes_VBO);
	glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(INDEX_VERTEX_POSITION);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	fprintf(stdout, " * Loaded axes into graphics memory.\n");
}

void draw_axes(void) {
	if (!b_draw_grid)
		return;

	glUseProgram(h_ShaderProgram_simple);
	ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(8000.0f, 8000.0f, 8000.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(2.0f);
	glBindVertexArray(axes_VAO);
	glUniform3fv(loc_primitive_color, 1, axes_color[0]);
	glDrawArrays(GL_LINES, 0, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[1]);
	glDrawArrays(GL_LINES, 2, 2);
	glUniform3fv(loc_primitive_color, 1, axes_color[2]);
	glDrawArrays(GL_LINES, 4, 2);
	glBindVertexArray(0);
	glLineWidth(1.0f);
	glUseProgram(0);
}

//grid
#define GRID_LENGTH			(100)
#define NUM_GRID_VETICES	((2 * GRID_LENGTH + 1) * 4)
GLuint grid_VBO, grid_VAO;
GLfloat grid_vertices[NUM_GRID_VETICES][3];
GLfloat grid_color[3] = { 0.5f, 0.5f, 0.5f };

void prepare_grid(void) {

	//set grid vertices
	int vertex_idx = 0;
	for (int x_idx = -GRID_LENGTH; x_idx <= GRID_LENGTH; x_idx++)
	{
		grid_vertices[vertex_idx][0] = x_idx;
		grid_vertices[vertex_idx][1] = -GRID_LENGTH;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;

		grid_vertices[vertex_idx][0] = x_idx;
		grid_vertices[vertex_idx][1] = GRID_LENGTH;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;
	}

	for (int y_idx = -GRID_LENGTH; y_idx <= GRID_LENGTH; y_idx++)
	{
		grid_vertices[vertex_idx][0] = -GRID_LENGTH;
		grid_vertices[vertex_idx][1] = y_idx;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;

		grid_vertices[vertex_idx][0] = GRID_LENGTH;
		grid_vertices[vertex_idx][1] = y_idx;
		grid_vertices[vertex_idx][2] = 0.0f;
		vertex_idx++;
	}

	// Initialize vertex buffer object.
	glGenBuffers(1, &grid_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, grid_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(grid_vertices), &grid_vertices[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &grid_VAO);
	glBindVertexArray(grid_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, grid_VAO);
	glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(INDEX_VERTEX_POSITION);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	fprintf(stdout, " * Loaded grid into graphics memory.\n");
}

void draw_grid(void) {
	if (!b_draw_grid)
		return;

	glUseProgram(h_ShaderProgram_simple);
	ModelViewMatrix = glm::scale(ViewMatrix, glm::vec3(100.0f, 100.0f, 100.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(1.0f);
	glBindVertexArray(grid_VAO);
	glUniform3fv(loc_primitive_color, 1, grid_color);
	glDrawArrays(GL_LINES, 0, NUM_GRID_VETICES);
	glBindVertexArray(0);
	glLineWidth(1.0f);
	glUseProgram(0);
}
GLuint cube_VBO, cube_VAO;
GLfloat cube_vertices[72][3] = { // vertices enumerated counterclockwise
	{ -1.0f, -1.0f, -1.0f }, { -1.0f, 0.0f, 0.0f }, { -1.0f, -1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f },
	{ -1.0f, 1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, -1.0f },
	{ -1.0f, 1.0f, -1.0f }, { 0.0f, 0.0f, -1.0f },
	{ 1.0f, -1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f }, { -1.0f, -1.0f, -1.0f }, { 0.0f, -1.0f, 0.0f },
	{ 1.0f, -1.0f, -1.0f }, { 0.0f, -1.0f, 0.0f },
	{ 1.0f, 1.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, -1.0f },
	{ -1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, -1.0f },
	{ -1.0f, -1.0f, -1.0f }, { -1.0f, 0.0f, 0.0f }, { -1.0f, 1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f },
	{ -1.0f, 1.0f, -1.0f }, { -1.0f, 0.0f, 0.0f },
	{ 1.0f, -1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f }, { -1.0f, -1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f },
	{ -1.0f, -1.0f, -1.0f }, { 0.0f, -1.0f, 0.0f },
	{ -1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { -1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f },
	{ 1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f },
	{ 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, -1.0f, -1.0f }, { 1.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, -1.0f }, { 1.0f, 0.0f, 0.0f },
	{ 1.0f, -1.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f },
	{ 1.0f, -1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, -1.0f }, { 0.0f, 1.0f, 0.0f },
	{ -1.0f, 1.0f, -1.0f }, { 0.0f, 1.0f, 0.0f },
	{ 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { -1.0f, 1.0f, -1.0f }, { 0.0f, 1.0f, 0.0f },
	{ -1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f },
	{ 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { -1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f },
	{ 1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }
};

Material_Parameters material_cube;
float cube_alpha;

void prepare_cube(void) {
	// Initialize vertex buffer object.
	glGenBuffers(1, &cube_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, cube_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), &cube_vertices[0][0], GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &cube_VAO);
	glBindVertexArray(cube_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, cube_VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	material_cube.ambient_color[0] = 0.1745f;
	material_cube.ambient_color[1] = 0.01175f;
	material_cube.ambient_color[2] = 0.01175f;
	material_cube.ambient_color[3] = 1.0f;

	material_cube.diffuse_color[0] = 0.21424f;
	material_cube.diffuse_color[1] = 0.04136f;
	material_cube.diffuse_color[2] = 0.74136f;
	material_cube.diffuse_color[3] = 1.0f;

	material_cube.specular_color[0] = 0.727811f;
	material_cube.specular_color[1] = 0.626959f;
	material_cube.specular_color[2] = 0.626959f;
	material_cube.specular_color[3] = 1.0f;

	material_cube.specular_exponent = 76.8f;

	material_cube.emissive_color[0] = 0.0f;
	material_cube.emissive_color[1] = 0.0f;
	material_cube.emissive_color[2] = 0.0f;
	material_cube.emissive_color[3] = 1.0f;

	cube_alpha = 0.5f;
}

void set_material_cube(void) {
	glUniform4fv(loc_material.ambient_color, 1, material_cube.ambient_color);
	glUniform4fv(loc_material.diffuse_color, 1, material_cube.diffuse_color);
	glUniform4fv(loc_material.specular_color, 1, material_cube.specular_color);
	glUniform1f(loc_material.specular_exponent, material_cube.specular_exponent);
	glUniform4fv(loc_material.emissive_color, 1, material_cube.emissive_color);
}

void draw_cube(void) {
	glFrontFace(GL_CCW);

	glBindVertexArray(cube_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6 * 6);
	glBindVertexArray(0);
}
//sun_temple
#define LOC_VERTEX 0
#define LOC_NORMAL 1
#define LOC_TEXCOORD 2
GLuint* sun_temple_VBO;
GLuint* sun_temple_VAO;
int* sun_temple_n_triangles;
int* sun_temple_vertex_offset;
GLfloat** sun_temple_vertices;
GLuint* sun_temple_texture_names;


int flag_fog;
bool* flag_texture_mapping;

void initialize_lights(void) { // follow OpenGL conventions for initialization
	int i;

	glUseProgram(h_ShaderProgram_TXPS);

	glUniform4f(loc_global_ambient_color, 1.0f, 1.0f, 1.0f, 1.0f);

	for (i = 0; i < scene.n_lights; i++) {
		glUniform1i(loc_light[i].light_on, 0);
		glUniform4f(loc_light[i].position,
			scene.light_list[i].pos[0],
			scene.light_list[i].pos[1],
			scene.light_list[i].pos[2],
			1.0f);

		glUniform4f(loc_light[i].ambient_color, 0.13f, 0.13f, 0.13f, 1.0f);
		glUniform4f(loc_light[i].diffuse_color, 0.5f, 0.5f, 0.5f, 1.0f);
		glUniform4f(loc_light[i].specular_color, 0.8f, 0.8f, 0.8f, 1.0f);
		glUniform3f(loc_light[i].spot_direction, 0.0f, 0.0f, -1.0f);
		glUniform1f(loc_light[i].spot_exponent, 0.0f); // [0.0, 128.0]
		glUniform1f(loc_light[i].spot_cutoff_angle, 180.0f); // [0.0, 90.0] or 180.0 (180.0 for no spot light effect)
		glUniform4f(loc_light[i].light_attenuation_factors, 20.0f, 0.0f, 0.0f, 1.0f); // .w != 0.0f for no ligth attenuation
	}
	glUseProgram(0);

	glUseProgram(h_ShaderProgram_Go);
	glUniform4f(loc_global_ambient_color_Go, 1.0f, 1.0f, 1.0f, 1.0f);

	for (i = 0; i < 14; i++) {
		glUniform1i(loc_light_Go[i].light_on, 0);
		glUniform4f(loc_light_Go[i].position,
			scene.light_list[i].pos[0],
			scene.light_list[i].pos[1],
			scene.light_list[i].pos[2],
			1.0f);

		glUniform4f(loc_light_Go[i].ambient_color, 0.13f, 0.13f, 0.13f, 1.0f);
		glUniform4f(loc_light_Go[i].diffuse_color, 0.5f, 0.5f, 0.5f, 1.0f);
		glUniform4f(loc_light_Go[i].specular_color, 0.8f, 0.8f, 0.8f, 1.0f);
		glUniform3f(loc_light_Go[i].spot_direction, 0.0f, 0.0f, -1.0f);
		glUniform1f(loc_light_Go[i].spot_exponent, 0.0f); // [0.0, 128.0]
		glUniform1f(loc_light_Go[i].spot_cutoff_angle, 180.0f); // [0.0, 90.0] or 180.0 (180.0 for no spot light effect)
		glUniform4f(loc_light_Go[i].light_attenuation_factors, 20.0f, 0.0f, 0.0f, 1.0f); // .w != 0.0f for no ligth attenuation
	}
	glUseProgram(0);
}

void initialize_flags(void) {
	flag_fog = 0;

	glUseProgram(h_ShaderProgram_TXPS);
	glUniform1i(loc_flag_fog, flag_fog);
	glUseProgram(0);
}

bool readTexImage2D_from_file(char* filename) {
	FREE_IMAGE_FORMAT tx_file_format;
	int tx_bits_per_pixel;
	FIBITMAP* tx_pixmap, * tx_pixmap_32;

	int width, height;
	GLvoid* data;

	tx_file_format = FreeImage_GetFileType(filename, 0);
	// assume everything is fine with reading texture from file: no error checking
	tx_pixmap = FreeImage_Load(tx_file_format, filename);
	if (tx_pixmap == NULL)
		return false;
	tx_bits_per_pixel = FreeImage_GetBPP(tx_pixmap);

	//fprintf(stdout, " * A %d-bit texture was read from %s.\n", tx_bits_per_pixel, filename);
	if (tx_bits_per_pixel == 32)
		tx_pixmap_32 = tx_pixmap;
	else {
		//fprintf(stdout, " * Converting texture from %d bits to 32 bits...\n", tx_bits_per_pixel);
		tx_pixmap_32 = FreeImage_ConvertTo32Bits(tx_pixmap);
	}

	width = FreeImage_GetWidth(tx_pixmap_32);
	height = FreeImage_GetHeight(tx_pixmap_32);
	data = FreeImage_GetBits(tx_pixmap_32);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
	//fprintf(stdout, " * Loaded %dx%d RGBA texture into graphics memory.\n\n", width, height);

	FreeImage_Unload(tx_pixmap_32);
	if (tx_bits_per_pixel != 32)
		FreeImage_Unload(tx_pixmap);

	return true;
}

void prepare_sun_temple(void) {
	int n_bytes_per_vertex, n_bytes_per_triangle;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	// VBO, VAO malloc
	sun_temple_VBO = (GLuint*)malloc(sizeof(GLuint) * scene.n_materials);
	sun_temple_VAO = (GLuint*)malloc(sizeof(GLuint) * scene.n_materials);

	sun_temple_n_triangles = (int*)malloc(sizeof(int) * scene.n_materials);
	sun_temple_vertex_offset = (int*)malloc(sizeof(int) * scene.n_materials);

	flag_texture_mapping = (bool*)malloc(sizeof(bool) * scene.n_textures);

	// vertices
	sun_temple_vertices = (GLfloat**)malloc(sizeof(GLfloat*) * scene.n_materials);

	for (int materialIdx = 0; materialIdx < scene.n_materials; materialIdx++) {
		MATERIAL* pMaterial = &(scene.material_list[materialIdx]);
		GEOMETRY_TRIANGULAR_MESH* tm = &(pMaterial->geometry.tm);

		// vertex
		sun_temple_vertices[materialIdx] = (GLfloat*)malloc(sizeof(GLfloat) * 8 * tm->n_triangle * 3);

		int vertexIdx = 0;
		for (int triIdx = 0; triIdx < tm->n_triangle; triIdx++) {
			TRIANGLE tri = tm->triangle_list[triIdx];
			for (int triVertex = 0; triVertex < 3; triVertex++) {
				sun_temple_vertices[materialIdx][vertexIdx++] = tri.position[triVertex].x;
				sun_temple_vertices[materialIdx][vertexIdx++] = tri.position[triVertex].y;
				sun_temple_vertices[materialIdx][vertexIdx++] = tri.position[triVertex].z;

				sun_temple_vertices[materialIdx][vertexIdx++] = tri.normal_vetcor[triVertex].x;
				sun_temple_vertices[materialIdx][vertexIdx++] = tri.normal_vetcor[triVertex].y;
				sun_temple_vertices[materialIdx][vertexIdx++] = tri.normal_vetcor[triVertex].z;

				sun_temple_vertices[materialIdx][vertexIdx++] = tri.texture_list[triVertex][0].u;
				sun_temple_vertices[materialIdx][vertexIdx++] = tri.texture_list[triVertex][0].v;
			}
		}

		// # of triangles
		sun_temple_n_triangles[materialIdx] = tm->n_triangle;

		if (materialIdx == 0)
			sun_temple_vertex_offset[materialIdx] = 0;
		else
			sun_temple_vertex_offset[materialIdx] = sun_temple_vertex_offset[materialIdx - 1] + 3 * sun_temple_n_triangles[materialIdx - 1];

		glGenBuffers(1, &sun_temple_VBO[materialIdx]);

		glBindBuffer(GL_ARRAY_BUFFER, sun_temple_VBO[materialIdx]);
		glBufferData(GL_ARRAY_BUFFER, sun_temple_n_triangles[materialIdx] * 3 * n_bytes_per_vertex,
			sun_temple_vertices[materialIdx], GL_STATIC_DRAW);

		// As the geometry data exists now in graphics memory, ...
		free(sun_temple_vertices[materialIdx]);

		// Initialize vertex array object.
		glGenVertexArrays(1, &sun_temple_VAO[materialIdx]);
		glBindVertexArray(sun_temple_VAO[materialIdx]);

		glBindBuffer(GL_ARRAY_BUFFER, sun_temple_VBO[materialIdx]);
		glVertexAttribPointer(INDEX_VERTEX_POSITION, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(0));
		glEnableVertexAttribArray(INDEX_VERTEX_POSITION);
		glVertexAttribPointer(INDEX_NORMAL, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(3 * sizeof(float)));
		glEnableVertexAttribArray(INDEX_NORMAL);
		glVertexAttribPointer(INDEX_TEX_COORD, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(6 * sizeof(float)));
		glEnableVertexAttribArray(INDEX_TEX_COORD);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		if ((materialIdx > 0) && (materialIdx % 100 == 0))
			fprintf(stdout, " * Loaded %d sun temple materials into graphics memory.\n", materialIdx / 100 * 100);
	}
	fprintf(stdout, " * Loaded %d sun temple materials into graphics memory.\n", scene.n_materials);

	// textures
	sun_temple_texture_names = (GLuint*)malloc(sizeof(GLuint) * scene.n_textures);
	glGenTextures(scene.n_textures, sun_temple_texture_names);

	for (int texId = 0; texId < scene.n_textures; texId++) {
		glActiveTexture(GL_TEXTURE0 + texId);
		glBindTexture(GL_TEXTURE_2D, sun_temple_texture_names[texId]);

		bool bReturn = readTexImage2D_from_file(scene.texture_file_name[texId]);

		if (bReturn) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			flag_texture_mapping[texId] = true;
		}
		else {
			flag_texture_mapping[texId] = false;
		}

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	fprintf(stdout, " * Loaded sun temple textures into graphics memory.\n\n");
	
	free(sun_temple_vertices);
}

void draw_sun_temple(void) {
	glUseProgram(h_ShaderProgram_TXPS);
	ModelViewMatrix = ViewMatrix;
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::transpose(glm::inverse(glm::mat3(ModelViewMatrix)));

	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);

	for (int materialIdx = 0; materialIdx < scene.n_materials; materialIdx++) {
		// set material
		glUniform4fv(loc_material.ambient_color, 1, scene.material_list[materialIdx].shading.ph.ka);
		glUniform4fv(loc_material.diffuse_color, 1, scene.material_list[materialIdx].shading.ph.kd);
		glUniform4fv(loc_material.specular_color, 1, scene.material_list[materialIdx].shading.ph.ks);
		glUniform1f(loc_material.specular_exponent, scene.material_list[materialIdx].shading.ph.spec_exp);
		glUniform4f(loc_material.emissive_color, 0.0f, 0.0f, 0.0f, 1.0f);

		int texId = scene.material_list[materialIdx].diffuseTexId;
		glUniform1i(loc_texture, texId);
		glUniform1i(loc_flag_texture_mapping, flag_texture_mapping[texId]);

		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0 + texId);
		glBindTexture(GL_TEXTURE_2D, sun_temple_texture_names[texId]);

		glBindVertexArray(sun_temple_VAO[materialIdx]);
		glDrawArrays(GL_TRIANGLES, 0, 3 * sun_temple_n_triangles[materialIdx]);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBindVertexArray(0);
	}
	glUseProgram(0);
}
//tiger start
#define N_TIGER_FRAMES 12
GLuint tiger_VBO, tiger_VAO;
int cur_frame_tiger = 0;
int tigerlocstamp = 0;
bool tiger_stop = false;
int tiger_n_triangles[N_TIGER_FRAMES];
int tiger_vertex_offset[N_TIGER_FRAMES];
GLfloat* tiger_vertices[N_TIGER_FRAMES];

int read_geometry(GLfloat** object, int bytes_per_primitive, char* filename) {
	int n_triangles;
	FILE* fp;

	// fprintf(stdout, "Reading geometry from the geometry file %s...\n", filename);
	fp = fopen(filename, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Cannot open the object file %s ...", filename);
		return -1;
	}
	fread(&n_triangles, sizeof(int), 1, fp);
	*object = (float*)malloc(n_triangles * bytes_per_primitive);
	if (*object == NULL) {
		fprintf(stderr, "Cannot allocate memory for the geometry file %s ...", filename);
		return -1;
	}

	fread(*object, bytes_per_primitive, n_triangles, fp);
	// fprintf(stdout, "Read %d primitives successfully.\n\n", n_triangles);
	fclose(fp);

	return n_triangles;
}

void prepare_tiger(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, tiger_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_TIGER_FRAMES; i++) {
		sprintf(filename, "Data/Tiger_%d%d_triangles_vnt.geom", i / 10, i % 10);
		tiger_n_triangles[i] = read_geometry(&tiger_vertices[i], n_bytes_per_triangle, filename);
		// Assume all geometry files are effective.
		tiger_n_total_triangles += tiger_n_triangles[i];

		if (i == 0)
			tiger_vertex_offset[i] = 0;
		else
			tiger_vertex_offset[i] = tiger_vertex_offset[i - 1] + 3 * tiger_n_triangles[i - 1];
	}

	// Initialize vertex buffer object.
	glGenBuffers(1, &tiger_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, tiger_VBO);
	glBufferData(GL_ARRAY_BUFFER, tiger_n_total_triangles * n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_TIGER_FRAMES; i++)
		glBufferSubData(GL_ARRAY_BUFFER, tiger_vertex_offset[i] * n_bytes_per_vertex,
			tiger_n_triangles[i] * n_bytes_per_triangle, tiger_vertices[i]);

	// As the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_TIGER_FRAMES; i++)
		free(tiger_vertices[i]);

	// Initialize vertex array object.
	glGenVertexArrays(1, &tiger_VAO);
	glBindVertexArray(tiger_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, tiger_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glActiveTexture(GL_TEXTURE0 + TEXTURE_ID_TIGER);
	glBindTexture(GL_TEXTURE_2D, texture_names[TEXTURE_ID_TIGER]);

	char str[256];
	sprintf(str, "Data/star.png");
	readTexImage2D_from_file(str);

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
}

int flag_cull_face = 0;

void draw_tiger_20181650(void) {
	glUseProgram(h_ShaderProgram_TXPS);
	glUniform1i(loc_texture, TEXTURE_ID_TIGER);
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	glUniform4f(loc_material.ambient_color, 0.2125f, 0.1275f, 0.054f, 1.0f);
	glUniform4f(loc_material.diffuse_color, 0.26f, 0.66f, 0.1f, 1.0f);
	glUniform4f(loc_material.specular_color, 0.8f, 0.8f, 0.8f, 1.0f);
	glUniform1f(loc_material.specular_exponent, 2.6f);
	glUniform4f(loc_material.emissive_color, 0.0f, 0.0f, 0.0f, 1.0f);
	glUniform1f(loc_flag_texture_mapping, 1);

	glBindVertexArray(tiger_VAO);
	glDrawArrays(GL_TRIANGLES, tiger_vertex_offset[cur_frame_tiger], 3 * tiger_n_triangles[cur_frame_tiger]);
	glBindVertexArray(0);

	glLineWidth(1.0f);
	glUniform1f(loc_flag_texture_mapping, 0);
	glUseProgram(0);
}
//tiger end
// cow start
GLuint cow_VBO, cow_VAO;
int cow_n_triangles;
GLfloat* cow_vertices;

Material_Parameters material_cow;
void prepare_cow(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, cow_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/cow_vn.geom");
	cow_n_triangles = read_geometry(&cow_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	cow_n_total_triangles += cow_n_triangles;


	// initialize vertex buffer object
	glGenBuffers(1, &cow_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, cow_VBO);
	glBufferData(GL_ARRAY_BUFFER, cow_n_total_triangles * 3 * n_bytes_per_vertex, cow_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(cow_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &cow_VAO);
	glBindVertexArray(cow_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, cow_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
void draw_cow_20181650(void) {
	glLineWidth(2.0f);
	glUseProgram(h_ShaderProgram_TXPS);
	//glUniform3f(loc_primitive_color, 0.0f, 1.0f, 1.0f);
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	glUniform4f(loc_material.ambient_color, 0.2125f, 0.1275f, 0.054f, 1.0f);
	glUniform4f(loc_material.diffuse_color, 0.26f, 0.66f, 0.1f, 1.0f);
	glUniform4f(loc_material.specular_color, 0.8f, 0.8f, 0.8f, 1.0f);
	glUniform1f(loc_material.specular_exponent, 2.6f);
	glUniform4f(loc_material.emissive_color, 0.0f, 0.0f, 0.0f, 1.0f);
	glUniform1f(loc_flag_texture_mapping, 0);

	glBindVertexArray(cow_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * cow_n_triangles);
	glBindVertexArray(0);

	glLineWidth(1.0f);
	glUseProgram(0);

}
// cow end
// dragon start
GLuint dragon_VBO, dragon_VAO;
int dragon_n_triangles;
GLfloat* dragon_vertices;

Material_Parameters material_dragon;
void prepare_dragon(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, dragon_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/dragon_vnt.geom");
	dragon_n_triangles = read_geometry(&dragon_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	dragon_n_total_triangles += dragon_n_triangles;


	// initialize vertex buffer object
	glGenBuffers(1, &dragon_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, dragon_VBO);
	glBufferData(GL_ARRAY_BUFFER, dragon_n_total_triangles * 3 * n_bytes_per_vertex, dragon_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(dragon_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &dragon_VAO);
	glBindVertexArray(dragon_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, dragon_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
void draw_dragon_20181650(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(2.0f);
	glUseProgram(h_ShaderProgram_simple);
	glUniform3f(loc_primitive_color, 0.0f, 0.0f, 0.0f);
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glBindVertexArray(dragon_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * dragon_n_triangles);
	glBindVertexArray(0);

	glLineWidth(1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glUseProgram(0);
}
// dragon end
// tank start
GLuint tank_VBO, tank_VAO;
int tank_n_triangles;
GLfloat* tank_vertices;

Material_Parameters material_tank;

void prepare_tank(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, tank_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/tank_vnt.geom");
	tank_n_triangles = read_geometry(&tank_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	tank_n_total_triangles += tank_n_triangles;


	// initialize vertex buffer object
	glGenBuffers(1, &tank_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, tank_VBO);
	glBufferData(GL_ARRAY_BUFFER, tank_n_total_triangles * 3 * n_bytes_per_vertex, tank_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(tank_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &tank_VAO);
	glBindVertexArray(tank_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, tank_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(LOC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

	glActiveTexture(GL_TEXTURE0 + TEXTURE_ID_TANK);
	glBindTexture(GL_TEXTURE_2D, texture_names[TEXTURE_ID_TANK]);

	char str[256];
	sprintf(str, "Data/digital.png");
	readTexImage2D_from_file(str);

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}
void draw_tank_20181650(void) {
	
	glUseProgram(h_ShaderProgram_TXPS);
	glUniform1i(loc_texture, TEXTURE_ID_TANK);
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	glUniform4f(loc_material.ambient_color, 0.2125f, 0.1275f, 0.054f, 1.0f);
	glUniform4f(loc_material.diffuse_color, 1.0f, 1.0f, 0.0f, 1.0f);
	glUniform4f(loc_material.specular_color, 0.8f, 0.8f, 0.8f, 1.0f);
	glUniform1f(loc_material.specular_exponent, 2.6f);
	glUniform4f(loc_material.emissive_color, 0.0f, 0.0f, 0.0f, 1.0f);
	glUniform1f(loc_flag_texture_mapping,1);

	glBindVertexArray(tank_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * tank_n_triangles);
	glBindVertexArray(0);

	glLineWidth(1.0f);
	glUniform1f(loc_flag_texture_mapping, 0);
	glUseProgram(0);

}
// tank end
// bus start
GLuint bus_VBO, bus_VAO;
int bus_n_triangles;
GLfloat* bus_vertices;

Material_Parameters material_bus;
void prepare_bus(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, bus_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/bus_vnt.geom");
	bus_n_triangles = read_geometry(&bus_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	bus_n_total_triangles += bus_n_triangles;


	// initialize vertex buffer object
	glGenBuffers(1, &bus_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, bus_VBO);
	glBufferData(GL_ARRAY_BUFFER, bus_n_total_triangles * 3 * n_bytes_per_vertex, bus_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(bus_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &bus_VAO);
	glBindVertexArray(bus_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, bus_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
void draw_bus_20181650(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(2.0f);
	glUseProgram(h_ShaderProgram_simple);
	glUniform3f(loc_primitive_color, 0.0f, 0.0f, 1.0f);
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glBindVertexArray(bus_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * bus_n_triangles);
	glBindVertexArray(0);

	glLineWidth(1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glUseProgram(0);
}
// bus end
// ironman start
// ironman object
GLuint ironman_VBO, ironman_VAO;
int ironman_n_triangles;
GLfloat* ironman_vertices;

Material_Parameters material_ironman;

void prepare_ironman(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, ironman_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	sprintf(filename, "Data/static_objects/ironman_vnt.geom");
	ironman_n_triangles = read_geometry(&ironman_vertices, n_bytes_per_triangle, filename);
	// assume all geometry files are effective
	ironman_n_total_triangles += ironman_n_triangles;


	// initialize vertex buffer object
	glGenBuffers(1, &ironman_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, ironman_VBO);
	glBufferData(GL_ARRAY_BUFFER, ironman_n_total_triangles * 3 * n_bytes_per_vertex, ironman_vertices, GL_STATIC_DRAW);

	// as the geometry data exists now in graphics memory, ...
	free(ironman_vertices);

	// initialize vertex array object
	glGenVertexArrays(1, &ironman_VAO);
	glBindVertexArray(ironman_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, ironman_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
void draw_ironman_20181650(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(2.0f);
	glUseProgram(h_ShaderProgram_simple);
	glUniform3f(loc_primitive_color, 1.0f, 0.0f, 1.0f);
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glBindVertexArray(ironman_VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3 * ironman_n_triangles);
	glBindVertexArray(0);

	glLineWidth(1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glUseProgram(0);
}
// ironman end
// wolf start
int cur_frame_wolf = 0, wolflocstamp = 0;
#define N_WOLF_FRAMES 17
GLuint wolf_VBO, wolf_VAO;
int wolf_n_triangles[N_WOLF_FRAMES];
int wolf_vertex_offset[N_WOLF_FRAMES];
GLfloat* wolf_vertices[N_WOLF_FRAMES];
Material_Parameters material_wolf;

void prepare_wolf(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, wolf_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_WOLF_FRAMES; i++) {
		sprintf(filename, "Data/dynamic_objects/wolf/wolf_%02d_vnt.geom", i);
		wolf_n_triangles[i] = read_geometry(&wolf_vertices[i], n_bytes_per_triangle, filename);
		// assume all geometry files are effective
		wolf_n_total_triangles += wolf_n_triangles[i];

		if (i == 0)
			wolf_vertex_offset[i] = 0;
		else
			wolf_vertex_offset[i] = wolf_vertex_offset[i - 1] + 3 * wolf_n_triangles[i - 1];
	}

	// initialize vertex buffer object
	glGenBuffers(1, &wolf_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, wolf_VBO);
	glBufferData(GL_ARRAY_BUFFER, wolf_n_total_triangles * n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_WOLF_FRAMES; i++)
		glBufferSubData(GL_ARRAY_BUFFER, wolf_vertex_offset[i] * n_bytes_per_vertex,
			wolf_n_triangles[i] * n_bytes_per_triangle, wolf_vertices[i]);

	// as the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_WOLF_FRAMES; i++)
		free(wolf_vertices[i]);

	// initialize vertex array object
	glGenVertexArrays(1, &wolf_VAO);
	glBindVertexArray(wolf_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, wolf_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
void draw_wolf_20181650(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(2.0f);
	glUseProgram(h_ShaderProgram_simple);
	glUniform3f(loc_primitive_color, 1.0f, 0.5f, 0.5f);
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glBindVertexArray(wolf_VAO);
	glDrawArrays(GL_TRIANGLES, wolf_vertex_offset[cur_frame_wolf], 3 * wolf_n_triangles[cur_frame_wolf]);
	glBindVertexArray(0);

	glLineWidth(1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glUseProgram(0);
}
// worl end
// spider start
#define N_SPIDER_FRAMES 16
int cur_frame_spider = 0, spiderlocstamp = 0;
GLuint spider_VBO, spider_VAO;
int spider_n_triangles[N_SPIDER_FRAMES];
int spider_vertex_offset[N_SPIDER_FRAMES];
GLfloat* spider_vertices[N_SPIDER_FRAMES];

Material_Parameters material_spider;

void prepare_spider(void) {
	int i, n_bytes_per_vertex, n_bytes_per_triangle, spider_n_total_triangles = 0;
	char filename[512];

	n_bytes_per_vertex = 8 * sizeof(float); // 3 for vertex, 3 for normal, and 2 for texcoord
	n_bytes_per_triangle = 3 * n_bytes_per_vertex;

	for (i = 0; i < N_SPIDER_FRAMES; i++) {
		sprintf(filename, "Data/dynamic_objects/spider/spider_vnt_%d%d.geom", i / 10, i % 10);
		spider_n_triangles[i] = read_geometry(&spider_vertices[i], n_bytes_per_triangle, filename);
		// assume all geometry files are effective
		spider_n_total_triangles += spider_n_triangles[i];

		if (i == 0)
			spider_vertex_offset[i] = 0;
		else
			spider_vertex_offset[i] = spider_vertex_offset[i - 1] + 3 * spider_n_triangles[i - 1];
	}

	// initialize vertex buffer object
	glGenBuffers(1, &spider_VBO);

	glBindBuffer(GL_ARRAY_BUFFER, spider_VBO);
	glBufferData(GL_ARRAY_BUFFER, spider_n_total_triangles * n_bytes_per_triangle, NULL, GL_STATIC_DRAW);

	for (i = 0; i < N_SPIDER_FRAMES; i++)
		glBufferSubData(GL_ARRAY_BUFFER, spider_vertex_offset[i] * n_bytes_per_vertex,
			spider_n_triangles[i] * n_bytes_per_triangle, spider_vertices[i]);

	// as the geometry data exists now in graphics memory, ...
	for (i = 0; i < N_SPIDER_FRAMES; i++)
		free(spider_vertices[i]);

	// initialize vertex array object
	glGenVertexArrays(1, &spider_VAO);
	glBindVertexArray(spider_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, spider_VBO);
	glVertexAttribPointer(LOC_VERTEX, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(LOC_NORMAL, 3, GL_FLOAT, GL_FALSE, n_bytes_per_vertex, BUFFER_OFFSET(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
void draw_spider_20181650(void) {
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(2.0f);
	glUseProgram(h_ShaderProgram_simple);
	glUniform3f(loc_primitive_color, 0.5f, 0.5f, 1.0f);
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);

	glBindVertexArray(spider_VAO);
	glDrawArrays(GL_TRIANGLES, spider_vertex_offset[cur_frame_spider], 3 * spider_n_triangles[cur_frame_spider]);
	glBindVertexArray(0);

	glLineWidth(1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glUseProgram(0);
}
// spider end
// TO DO
void prepare_objects(void) {
	prepare_tiger();

	prepare_cow();
	prepare_dragon();
	prepare_tank();
	prepare_bus();
	prepare_ironman();
	prepare_wolf();
	prepare_spider();
	prepare_cube();
}

void draw_objects_20181650(void) {

	if (tigerlocstamp <= 180) {
		ModelMatrix_tiger = glm::rotate(glm::mat4(1.0f), 2 * tigerlocstamp * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix_tiger = glm::translate(ModelMatrix_tiger, glm::vec3(0.0f, -500.0f, 230.0f));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, 90 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (180 < tigerlocstamp&&tigerlocstamp<=210) {
		ModelMatrix_tiger = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -500.0f-(tigerlocstamp-180)*5, 230.0f));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, (90-3*(tigerlocstamp-180)) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (210 < tigerlocstamp && tigerlocstamp <= 360) {
		ModelMatrix_tiger = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -650.0f-(tigerlocstamp-210)*8, 230.0f));
	}
	else if (360 < tigerlocstamp && tigerlocstamp <= 390) {
		ModelMatrix_tiger = glm::translate(glm::mat4(1.0f), glm::vec3(-(tigerlocstamp-360)*3.0f, -1850.0f, 230.0f));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, (- 3 * (tigerlocstamp - 360)) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (390 < tigerlocstamp && tigerlocstamp <= 480) {
		ModelMatrix_tiger = glm::translate(glm::mat4(1.0f), glm::vec3(-90.0f-(tigerlocstamp-390)*5.0, -1850.0f, 230.0f));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, -90 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (480 < tigerlocstamp && tigerlocstamp <= 510) {
		ModelMatrix_tiger = glm::translate(glm::mat4(1.0f), glm::vec3(-540.0f, -1850.0f, 230.0f+(tigerlocstamp-480)*15));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, ( - 90 + 6 * (tigerlocstamp - 480)) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (510 < tigerlocstamp && tigerlocstamp <= 540) {
		ModelMatrix_tiger = glm::translate(glm::mat4(1.0f), glm::vec3(-540.0f + (tigerlocstamp - 510) * 18.0, -1850.0f, 680.0f - (tigerlocstamp - 510) * 15));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, 90 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (540 < tigerlocstamp && tigerlocstamp <= 570) {
		ModelMatrix_tiger = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1850.0f+(tigerlocstamp-540)*5, 230.0f));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, (90+(tigerlocstamp-540)*3) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (570 < tigerlocstamp && tigerlocstamp <= 720) {
		ModelMatrix_tiger = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1700.0f + (tigerlocstamp - 570) * 8, 230.0f));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, 180 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (720 < tigerlocstamp && tigerlocstamp < 750) {
		ModelMatrix_tiger = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -500.0f, 230.0f));
		ModelMatrix_tiger = glm::rotate(ModelMatrix_tiger, (180-(tigerlocstamp-720)*3) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	if (tiger_head) {
		if (tiger_nod) {
			if (nod_dir_up) {
				if (nod_angle < -70) {
					ModelMatrix_tiger_head = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -88.0f, 62.0f));
					ModelMatrix_tiger_head = glm::rotate(ModelMatrix_tiger_head, 180 * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
					ModelMatrix_tiger_head = glm::rotate(ModelMatrix_tiger_head, (++nod_angle) * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
				}
				else
					nod_dir_up = false;
			}
			else {
				if (nod_angle > -110) {
					ModelMatrix_tiger_head = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -88.0f, 62.0f));
					ModelMatrix_tiger_head = glm::rotate(ModelMatrix_tiger_head, 180 * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
					ModelMatrix_tiger_head = glm::rotate(ModelMatrix_tiger_head, (--nod_angle) * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
				}
				else
					nod_dir_up = true;
			}
		}
		else if (tiger_back) {
			ModelMatrix_tiger_head = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 150.0f, 62.0f));
			ModelMatrix_tiger_head = glm::rotate(ModelMatrix_tiger_head, 180 * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
			ModelMatrix_tiger_head = glm::rotate(ModelMatrix_tiger_head, -90 * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
		}
		else {
			ModelMatrix_tiger_head = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -88.0f, 62.0f));
			ModelMatrix_tiger_head = glm::rotate(ModelMatrix_tiger_head, 180 * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
			ModelMatrix_tiger_head = glm::rotate(ModelMatrix_tiger_head, -90 * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
		}
		Matrix_tiger_camera = ModelMatrix_tiger * ModelMatrix_tiger_head;
		ProjectionMatrix = glm::perspective(scene.camera.fovy, scene.camera.aspect, 0.1f, 30000.0f);
		ViewMatrix = glm::affineInverse(Matrix_tiger_camera);
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
	}
	ModelViewMatrix = ViewMatrix * ModelMatrix_tiger;
	set_user_light();
	ModelViewMatrixInvTrans = glm::transpose(glm::inverse(glm::mat3(ModelViewMatrix)));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	draw_tiger_20181650();

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, -250.0f, 280.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90 * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(150.0f, 150.0f, 150.0f));
	ModelViewMatrixInvTrans = glm::transpose(glm::inverse(glm::mat3(ModelViewMatrix)));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	draw_cow_20181650();

	//ModelViewMatrix = ViewMatrix;
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, -400.0f, 700.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 45 * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(10.0f, 10.0f, 10.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	draw_dragon_20181650();

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(540.0f, -2300.0f, 230.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(20.0f, 20.0f, 20.0f));
	ModelViewMatrixInvTrans = glm::transpose(glm::inverse(glm::mat3(ModelViewMatrix)));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	draw_tank_20181650();

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, -4000.0f, 30.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 180 * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, -90 * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(10.0f, 10.0f, 10.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	draw_bus_20181650();

	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, -6000.0f, 300.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	//ModelViewMatrix = glm::rotate(ModelViewMatrix, 180 * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 135 * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(50.0f, 50.0f, 50.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	draw_ironman_20181650();

	if (wolflocstamp <= 30) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-(wolflocstamp)*20.0f, -2600.0f, 30.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -90 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 90 * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	}
	else if (30 < wolflocstamp && wolflocstamp <= 45) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-600.0f, -2600.0f, 30.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (- 90-(wolflocstamp-30)*6) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (90-(wolflocstamp-30)) * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	}
	else if (45 < wolflocstamp && wolflocstamp <= 115) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-600.0f, -2600.0f+(wolflocstamp-45)*8.5f, 30.0f+(wolflocstamp-45)*3.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -180 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 75 * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	}
	else if (115 < wolflocstamp && wolflocstamp <= 130) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-600.0f, -2000.0f, 240.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (-180 - (wolflocstamp - 115) * 6) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (75 + (wolflocstamp - 115)) * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	}
	else if (130 < wolflocstamp && wolflocstamp <= 330) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-600.0f + (wolflocstamp - 130) * 5, -2000.0f, 240.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -270 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 90 * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	}
	else if (330 < wolflocstamp && wolflocstamp <= 345) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(400.0f, -2000.0f, 240.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, ( - 270 -(wolflocstamp-330)*6)* TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (90+(wolflocstamp-330)) * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	}
	else if (345 < wolflocstamp && wolflocstamp <= 410) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(400.0f, -2000.0f - (wolflocstamp - 345) * 8.5f, 240.0f - (wolflocstamp - 345) * 3.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 105 * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	}
	else if (410 < wolflocstamp && wolflocstamp <= 425) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(400.0f, -2600.0f, 30.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, - (wolflocstamp - 410) * 6 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (105-(wolflocstamp-410)) * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	}
	else if (425 < wolflocstamp && wolflocstamp <= 450) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(400.0f-(wolflocstamp-425)*16, -2600.0f, 30.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -90*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 90 * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	}
	else if (425 < wolflocstamp && wolflocstamp <= 450) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(400.0f - (wolflocstamp - 425) * 16, -2600.0f, 30.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -90 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 90 * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	}
	else if (450 < wolflocstamp && wolflocstamp <= 500) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, -2600.0f, 30.0f+(wolflocstamp-450)*6.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -90 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 90 * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	}
	else if (500 < wolflocstamp && wolflocstamp <= 550) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, -2600.0f, 330.0f - (wolflocstamp - 500) * 6.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -90 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 90 * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	}
	else if (550 < wolflocstamp && wolflocstamp <= 650) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, -2600.0f, 30.0f + (wolflocstamp - 550) * 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (- 90 + (wolflocstamp-550)*3.6f) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 90 * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	}
	else if (650 < wolflocstamp && wolflocstamp <= 750) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, -2600.0f, 530.0f - (wolflocstamp - 650) * 5.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (-90 - (wolflocstamp - 650) * 7.2f) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, 90 * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	}
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(150.0f, 150.0f, 150.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	draw_wolf_20181650();

	if (spiderlocstamp <= 100) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, -6500.0f + (spiderlocstamp) * 25, 150.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -180 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (100 < spiderlocstamp && spiderlocstamp <= 130) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, -4000.0f, 150.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (-180 + (spiderlocstamp - 100) * 3) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (130 < spiderlocstamp && spiderlocstamp <= 150) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-(spiderlocstamp-130)*25.0f, -4000.0f, 150.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -90 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (150 < spiderlocstamp && spiderlocstamp <= 180) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-500.0f, -4000.0f, 150.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (- 90 - (spiderlocstamp-150)*3 )* TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (180 < spiderlocstamp && spiderlocstamp <= 200) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-500.0f, (- 4000.0f + (spiderlocstamp-180)*30.0f), 150.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -180 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (200 < spiderlocstamp && spiderlocstamp <= 230) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(-500.0f, -3400.0f, 150.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (- 180-(spiderlocstamp-200)*3) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (230 < spiderlocstamp && spiderlocstamp <= 280) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(- 500.0f + (spiderlocstamp-230)*20.0f, -3400.0f, 150.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -270 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (280 < spiderlocstamp && spiderlocstamp <= 310) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(500.0f, -3400.0f, 150.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (- 270 -(spiderlocstamp-280)*3)* TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (310 < spiderlocstamp && spiderlocstamp <= 330) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(500.0f,- 3400.0f - (spiderlocstamp-310)*30.0f, 150.0f));
	}
	else if (330 < spiderlocstamp && spiderlocstamp <= 360) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(500.0f, -4000.0f, 150.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -(spiderlocstamp-330)*3 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (360 < spiderlocstamp && spiderlocstamp <= 380) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(500.0f - (spiderlocstamp - 360) * 25.0f, -4000.0f, 150.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, -90 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (380 < spiderlocstamp && spiderlocstamp <= 410) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, -4000.0f, 150.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (- 90 +(spiderlocstamp-380)*3)* TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else if (410 < spiderlocstamp && spiderlocstamp <= 510) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, -4000.0f - (spiderlocstamp-410)*25.0f, 150.0f));
	}
	else if (510 < spiderlocstamp && spiderlocstamp <= 540) {
		ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, -6500.0f, 150.0f));
		ModelViewMatrix = glm::rotate(ModelViewMatrix, (spiderlocstamp-510)*6 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 180 * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, 90 * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(100.0f, 100.0f, 100.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	draw_spider_20181650();

}
/*****************************  END: geometry setup *****************************/
void draw_cubes() {
	glUseProgram(h_ShaderProgram_TXPS);
	glEnable(GL_BLEND);
	//glDisable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER);
	//glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	set_material_cube();
	ModelViewMatrix = glm::translate(ViewMatrix, glm::vec3(0.0f, -500.0f, 500.0f));
	ModelViewMatrix = glm::scale(ModelViewMatrix, glm::vec3(20.0f, 20.0f, 20.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, cube_timestamp*TO_RADIAN,glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewMatrix = glm::rotate(ModelViewMatrix, cube_timestamp * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewProjectionMatrix = ProjectionMatrix * ModelViewMatrix;
	ModelViewMatrixInvTrans = glm::transpose(glm::inverse(glm::mat3(ModelViewMatrix)));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix_TXPS, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glUniformMatrix4fv(loc_ModelViewMatrix_TXPS, 1, GL_FALSE, &ModelViewMatrix[0][0]);
	glUniformMatrix3fv(loc_ModelViewMatrixInvTrans_TXPS, 1, GL_FALSE, &ModelViewMatrixInvTrans[0][0]);
	glUniform1f(loc_flag_texture_mapping, 0);
	glUniform1f(loc_u_fragment_alpha, cube_alpha);
	glCullFace(GL_FRONT);
	draw_cube();
	glUniform1f(loc_u_fragment_alpha, cube_alpha);
	glCullFace(GL_BACK);
	draw_cube();
	glUniform1f(loc_u_fragment_alpha, 1.0f);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	//glEnable(GL_DEPTH_TEST);
	glUseProgram(0);
}
/********************  START: callback function definitions *********************/
void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	draw_objects_20181650();
	draw_grid();
	draw_axes();
	draw_sun_temple();
	draw_cubes();
	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'f':
		b_draw_grid = b_draw_grid ? false : true;
		glutPostRedisplay();
		break;
	case 'w':
		user_light_on = 1 - user_light_on;
		glUseProgram(h_ShaderProgram_TXPS);
		for (int i = 13; i < 15; i++) {
			glUniform1i(loc_light[i].light_on, user_light_on);
		}
		glUseProgram(0);
		break;
	case 'e':
		eye_light_on = 1 - eye_light_on;
		glUseProgram(h_ShaderProgram_TXPS);
		glUniform1i(loc_light[15].light_on, eye_light_on);
		glUseProgram(0);
		break;
	case 'm':
		mod_light_on = 1 - mod_light_on;
		glUseProgram(h_ShaderProgram_TXPS);
		glUniform1i(loc_light[16].light_on, mod_light_on);
		glUseProgram(0);
		break;
	case 'i':
		if (move_mode) {
			current_camera.pos[0] -= 20*current_camera.naxis[0];
			current_camera.pos[1] -= 20*current_camera.naxis[1];
			current_camera.pos[2] -= 20*current_camera.naxis[2];
			set_ViewMatrix_from_camera_frame();
			ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
			glutPostRedisplay();
		}
		break;
	case 'j':
		if (move_mode) {
			current_camera.pos[0] -= 20 * current_camera.uaxis[0];
			current_camera.pos[1] -= 20 * current_camera.uaxis[1];
			current_camera.pos[2] -= 20 * current_camera.uaxis[2];
			set_ViewMatrix_from_camera_frame();
			ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
			glutPostRedisplay();
		}
		break;
	case 'k':
		if (move_mode) {
			current_camera.pos[0] += 20 * current_camera.naxis[0];
			current_camera.pos[1] += 20 * current_camera.naxis[1];
			current_camera.pos[2] += 20 * current_camera.naxis[2];
			set_ViewMatrix_from_camera_frame();
			ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
			glutPostRedisplay();
		}
		break;
	case 'l':
		if (move_mode) {
			current_camera.pos[0] += 20 * current_camera.uaxis[0];
			current_camera.pos[1] += 20 * current_camera.uaxis[1];
			current_camera.pos[2] += 20 * current_camera.uaxis[2];
			set_ViewMatrix_from_camera_frame();
			ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
			glutPostRedisplay();
		}
		break;
	case 'u':
		if (move_mode) {
			current_camera.pos[0] += 20 * current_camera.vaxis[0];
			current_camera.pos[1] += 20 * current_camera.vaxis[1];
			current_camera.pos[2] += 20 * current_camera.vaxis[2];
			set_ViewMatrix_from_camera_frame();
			ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
			glutPostRedisplay();
		}
		break;
	case 'o':
		if (move_mode) {
			current_camera.pos[0] -= 20 * current_camera.vaxis[0];
			current_camera.pos[1] -= 20 * current_camera.vaxis[1];
			current_camera.pos[2] -= 20 * current_camera.vaxis[2];
			set_ViewMatrix_from_camera_frame();
			ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
			glutPostRedisplay();
		}
		break;
	case '0':
		move_mode = false;
		tiger_head = false;
		tiger_nod = false;
		set_current_camera(CAMERA_0);
		glutPostRedisplay();
		break;
	case '1':
		move_mode = false;
		tiger_head = false;
		tiger_nod = false;
		set_current_camera(CAMERA_1);
		glutPostRedisplay();
		break;
	case '2':
		move_mode = false;
		tiger_head = false;
		tiger_nod = false;
		set_current_camera(CAMERA_2);
		glutPostRedisplay();
		break;
	case '3':
		move_mode = false;
		tiger_head = false;
		tiger_nod = false;
		set_current_camera(CAMERA_3);
		glutPostRedisplay();
		break;
	case 'n':
		tiger_head = false;
		tiger_nod = false;
		if (move_mode)
			move_mode = false;
		else {
			move_mode = true;
			set_current_camera(CAMERA_M);
			glutPostRedisplay();
		}
		break;
	case 't':
		alpha_mode = 1 - alpha_mode;
		glUseProgram(h_ShaderProgram_TXPS);
		glUniform1i(loc_u_alpha_mode, alpha_mode);
		glUseProgram(0);
		glutPostRedisplay();
		break;
	case 'q':
		if(cube_alpha<1.0f)
			cube_alpha += 0.1f;
		glUseProgram(h_ShaderProgram_TXPS);
		glUniform1i(loc_u_fragment_alpha, cube_alpha);
		glUseProgram(0);
		glutPostRedisplay();
		break;
	case 'a':
		if(cube_alpha>0.0f)
			cube_alpha -= 0.1f;
		glUseProgram(h_ShaderProgram_TXPS);
		glUniform1i(loc_u_fragment_alpha, cube_alpha);
		glUseProgram(0);
		glutPostRedisplay();
		break;
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	}
}

void reshape(int width, int height) {
	float aspect_ratio;

	glViewport(0, 0, width, height);

	ProjectionMatrix = glm::perspective(current_camera.fovy, current_camera.aspect_ratio, current_camera.near_c, current_camera.far_c);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &axes_VAO);
	glDeleteBuffers(1, &axes_VBO);

	glDeleteVertexArrays(1, &grid_VAO);
	glDeleteBuffers(1, &grid_VBO);

	glDeleteVertexArrays(scene.n_materials, sun_temple_VAO);
	glDeleteBuffers(scene.n_materials, sun_temple_VBO);

	glDeleteTextures(scene.n_textures, sun_temple_texture_names);

	free(sun_temple_n_triangles);
	free(sun_temple_vertex_offset);

	free(sun_temple_VAO);
	free(sun_temple_VBO);

	free(sun_temple_texture_names);
	free(flag_texture_mapping);
	glDeleteTextures(N_TEXTURES_USED, texture_names);
}
bool leftmousepressed = false, rightmousepressed = false;
int prev_x = 0, prev_y = 0;

void mousepress(int button, int state, int x, int y) {
	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN)) {
		leftmousepressed = true;
		prev_x = x, prev_y = y;
	}
	else if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP)) {
		leftmousepressed = false;
	}
	else if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN)) {
		rightmousepressed = true;
		prev_x = x, prev_y = y;
	}
	else if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_UP))
		rightmousepressed = false;
}

void mousemove(int x, int y) {
	if (move_mode&&leftmousepressed) {
		int del_x = x - prev_x, del_y = y - prev_y;
		float u[3], v[3], n[3];
		if (abs(del_x) >= abs(del_y)) {
			if (-10 < del_x && del_x <10) {
				memcpy(u, current_camera.uaxis, sizeof(u));
				memcpy(v, current_camera.vaxis, sizeof(v));
				memcpy(n, current_camera.naxis, sizeof(n));
				auto rotmat = glm::rotate(glm::mat4(1.0f), -del_x * TO_RADIAN, glm::vec3(v[0], v[1], v[2]));
				current_camera.uaxis[0] = rotmat[0][0] * u[0] + rotmat[1][0] * u[1] + rotmat[2][0] * u[2];
				current_camera.uaxis[1] = rotmat[0][1] * u[0] + rotmat[1][1] * u[1] + rotmat[2][1] * u[2];
				current_camera.uaxis[2] = rotmat[0][2] * u[0] + rotmat[1][2] * u[1] + rotmat[2][2] * u[2];
				current_camera.naxis[0] = rotmat[0][0] * n[0] + rotmat[1][0] * n[1] + rotmat[2][0] * n[2];
				current_camera.naxis[1] = rotmat[0][1] * n[0] + rotmat[1][1] * n[1] + rotmat[2][1] * n[2];
				current_camera.naxis[2] = rotmat[0][2] * n[0] + rotmat[1][2] * n[1] + rotmat[2][2] * n[2];
			}
		}
		else {
			if (-10 < del_y && del_y < 10) {
				memcpy(u, current_camera.uaxis, sizeof(u));
				memcpy(v, current_camera.vaxis, sizeof(v));
				memcpy(n, current_camera.naxis, sizeof(n));
				auto rotmat = glm::rotate(glm::mat4(1.0f), -del_y * TO_RADIAN, glm::vec3(u[0], u[1], u[2]));
				current_camera.vaxis[0] = rotmat[0][0] * v[0] + rotmat[1][0] * v[1] + rotmat[2][0] * v[2];
				current_camera.vaxis[1] = rotmat[0][1] * v[0] + rotmat[1][1] * v[1] + rotmat[2][1] * v[2];
				current_camera.vaxis[2] = rotmat[0][2] * v[0] + rotmat[1][2] * v[1] + rotmat[2][2] * v[2];
				current_camera.naxis[0] = rotmat[0][0] * n[0] + rotmat[1][0] * n[1] + rotmat[2][0] * n[2];
				current_camera.naxis[1] = rotmat[0][1] * n[0] + rotmat[1][1] * n[1] + rotmat[2][1] * n[2];
				current_camera.naxis[2] = rotmat[0][2] * n[0] + rotmat[1][2] * n[1] + rotmat[2][2] * n[2];
			}
		}
	}
	else if (move_mode && rightmousepressed) {
		int del_x = x - prev_x, del_y = y - prev_y;
		float u[3], v[3], n[3];
		if (-10 < del_x && del_x < 10) {
			memcpy(u, current_camera.uaxis, sizeof(u));
			memcpy(v, current_camera.vaxis, sizeof(v));
			memcpy(n, current_camera.naxis, sizeof(n));
			auto rotmat = glm::rotate(glm::mat4(1.0f), -del_x * TO_RADIAN, glm::vec3(n[0], n[1], n[2]));
			current_camera.uaxis[0] = rotmat[0][0] * u[0] + rotmat[1][0] * u[1] + rotmat[2][0] * u[2];
			current_camera.uaxis[1] = rotmat[0][1] * u[0] + rotmat[1][1] * u[1] + rotmat[2][1] * u[2];
			current_camera.uaxis[2] = rotmat[0][2] * u[0] + rotmat[1][2] * u[1] + rotmat[2][2] * u[2];
			current_camera.vaxis[0] = rotmat[0][0] * v[0] + rotmat[1][0] * v[1] + rotmat[2][0] * v[2];
			current_camera.vaxis[1] = rotmat[0][1] * v[0] + rotmat[1][1] * v[1] + rotmat[2][1] * v[2];
			current_camera.vaxis[2] = rotmat[0][2] * v[0] + rotmat[1][2] * v[1] + rotmat[2][2] * v[2];
		}
	}
	set_ViewMatrix_from_camera_frame();
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
	prev_x = x, prev_y = y;
	glutPostRedisplay();
}

void mousewhell(int button, int dir, int x, int y) {
	if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) {
		if (dir == 0)
			return;
		if (dir > 0&&current_camera.fovy>0.05) {
			current_camera.fovy-=0.01;
		}
		else if(dir < 0 && current_camera.fovy < 1.5) {
			current_camera.fovy+=0.01;
		}
		ProjectionMatrix = glm::perspective(current_camera.fovy, current_camera.aspect_ratio,
			current_camera.near_c, current_camera.far_c);
		ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
		glutPostRedisplay();
	}
}
void timer_scene(int timestamp_scene) {
	cur_frame_tiger = timestamp_scene % N_TIGER_FRAMES;
	cur_frame_wolf = timestamp_scene % N_WOLF_FRAMES;
	cur_frame_spider = timestamp_scene % N_SPIDER_FRAMES;
	cube_timestamp = timestamp_scene % 360;
	if(!tiger_stop)
		tigerlocstamp = (tigerlocstamp+1) % 750;
	wolflocstamp = (wolflocstamp + 1) % 750;
	spiderlocstamp = (spiderlocstamp + 1) % 540;
	glutPostRedisplay();
	glutTimerFunc(100, timer_scene, (timestamp_scene + 1) % INT_MAX);
}
/*********************  END: callback function definitions **********************/

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutCloseFunc(cleanup);
	glutMouseFunc(mousepress);
	glutMotionFunc(mousemove);
	glutMouseWheelFunc(mousewhell);
	glutTimerFunc(100, timer_scene, 0);
}

void initialize_OpenGL(void) {
	glEnable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	ViewMatrix = glm::mat4(1.0f);
	ProjectionMatrix = glm::mat4(1.0f);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	initialize_lights();
	initialize_flags();
	glGenTextures(N_TEXTURES_USED, texture_names);
}

void prepare_scene(void) {
	prepare_axes();
	prepare_grid();
	prepare_sun_temple();
	prepare_objects();
}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	initialize_camera();
	prepare_scene();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "********************************************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "********************************************************************************\n\n");
}

void print_message(const char* m) {
	fprintf(stdout, "%s\n\n", m);
}

void greetings(char* program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "********************************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n********************************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 9
void drawScene(int argc, char* argv[]) {
	char program_name[64] = "Sogang CSE4170 Sun Temple Scene";
	char messages[N_MESSAGE_LINES][256] = { 
		"    - Keys used:",
		"		'f' : draw x, y, z axes and grid",
		"		'1' : set the camera for original view",
		"		'2' : set the camera for bronze statue view",
		"		'3' : set the camera for bronze statue view",	
		"		'4' : set the camera for top view",
		"		'5' : set the camera for front view",
		"		'6' : set the camera for side view",
		"		'ESC' : program close",
	};

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(900, 600);
	glutInitWindowPosition(20, 20);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}
