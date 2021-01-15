#include "framework.h"

/// Forras: Bsz1 jegyzet
mat4 transpose(const mat4& mat){
	mat4 res;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++){
			res.m[i][j] = mat.m[j][i];
		}
	}

	return res;
}

/// Forras: raytrace.cpp
struct Material {
	vec3 ka, kd, ks;
	float  shininess;
	Material(vec3 _kd, vec3 _ks, float _shininess) : ka(_kd * M_PI), kd(_kd), ks(_ks) { shininess = _shininess; }
};

/// Forras: raytrace.cpp
struct Hit {
	float t;
	vec3 position, normal;
	Material * material;
	Hit() { t = -1; }
}; 

/// Forras: raytrace.cpp
struct Ray {
	vec3 start, dir;
	Ray(vec3 _start, vec3 _dir) { start = _start; dir = normalize(_dir); }
};

/// Forras: raytrace.cpp
struct Intersectable {
	Material * material;
	mat4 matrix;

	virtual Hit intersect(const Ray& ray){};
};

// A transzformaciohoz a forras: https://marctenbosch.com/photon/mbosch_intersection.pdf
// https://www.cs.uaf.edu/2012/spring/cs481/section/0/lecture/01_26_ray_intersections.html
struct Hyperboloid : public Intersectable {
	vec3 center;
	mat4 matrix;

	Hyperboloid(Material* _material,mat4 &_matrix) {
		material = _material;
		matrix = _matrix;
	}

	Hit intersect(const Ray& ray) override {
		
		mat4 T_1 = TranslateMatrix(-1.0f * vec3(0.10f,0.2f,0.1f)) * RotationMatrix(0.4f, vec3(1.0f,1.0f,1.0f));	
		mat4 Q = T_1 * matrix * transpose(T_1);

		float a =  dot((vec4(ray.dir.x,ray.dir.y,ray.dir.z, 0) * Q), vec4(ray.dir.x, ray.dir.y, ray.dir.z, 0));

		float b = 2 * dot((vec4(ray.dir.x,ray.dir.y,ray.dir.z, 0) * Q), vec4(ray.start.x, ray.start.y, ray.start.z, 1));
		
		float c = dot((vec4(ray.start.x,ray.start.y,ray.start.z, 1) * Q), vec4(ray.start.x, ray.start.y, ray.start.z, 1));
		
		Hit hit;

		float discr = b*b - 4.0 * a * c;
		
		if (discr < 0) return hit;
		float sqrt_discr = sqrtf(discr);
		
		float t1 = (-b + sqrt_discr) / (2.0f * a);	// t1 >= t2 for sure
		if (t1 <= 0) return hit;
		
		float t2 = (-b - sqrt_discr) / ( 2.0f * a);

		hit.t = (t2 > 0.0f) ? float(t2) : float(t1);
		hit.position = ray.start + ray.dir * hit.t;

		
		hit.normal = normalize(vec3( 2 * hit.position.x, 2 * hit.position.y, -2 * hit.position.z ));

		hit.material = material;
		
		return hit;
	}

};

// Forras: raytrace.cpp
struct Light {
	vec3 direction;
	vec3 Le;
	Light(vec3 _direction, vec3 _Le) {
		direction = normalize(_direction);
		Le = _Le;
	}

	vec3 directionOf(vec3 point) {
		return direction;
	}

};

// Forras: pathtrace.cpp
struct PointLight : Light {
	vec3 location;
	vec3 power;

	PointLight(vec3 _location, vec3 _power) : Light(vec3(0,0,0), vec3(0,0,0)){
		location = _location;
		power = _power;
	}

	vec3 directionOf(vec3 point) {
		return normalize(location - point);
	}
};

// A transzforaciohoz a forras: https://marctenbosch.com/photon/mbosch_intersection.pdf
// https://www.cl.cam.ac.uk/teaching/1999/AGraphHCI/SMAG/node2.html
struct Cylinder : public Intersectable {
	mat4 matrix;

	Cylinder(Material* _material, mat4 &_matrix) {
		material = _material;
		matrix = _matrix;
	}

	Hit intersect(const Ray& ray) override {
	
		mat4 T_1 = TranslateMatrix(-1.0f * vec3(8.0f,0.0f,2.0f)) * RotationMatrix(-3.2f, vec3(1.0f,1.0f,1.0f));
		mat4 Q = T_1 * matrix * transpose(T_1);

		float a =  dot((vec4(ray.dir.x,ray.dir.y,ray.dir.z, 0) * Q), vec4(ray.dir.x, ray.dir.y, ray.dir.z, 0));

		float b = 2 * dot((vec4(ray.dir.x,ray.dir.y,ray.dir.z, 0) * Q), vec4(ray.start.x, ray.start.y, ray.start.z, 1));
		
		float c = dot((vec4(ray.start.x,ray.start.y,ray.start.z, 1) * Q), vec4(ray.start.x, ray.start.y, ray.start.z, 1));
		
		Hit hit;

		float discr = b*b - 4.0 * a * c;
		
		if (discr < 0) return hit;
		float sqrt_discr = sqrtf(discr);
		
		float t1 = (-b + sqrt_discr) / (2.0f * a);	// t1 >= t2 for sure
		if (t1 <= 0) return hit;
		
		float t2 = (-b - sqrt_discr) / ( 2.0f * a);

		hit.t = (t2 > 0.0f) ? float(t2) : float(t1);
		hit.position = ray.start + ray.dir * hit.t;

		hit.normal = normalize(vec3(  2 * hit.position.x, 2 * hit.position.y, 1 * hit.position.z ));

		hit.material = material;
		return hit;
	}
};

struct Sphere : public Intersectable {
	vec3 center;
	float radius;

	Sphere(const vec3& _center, float _radius, Material* _material) {
		center = _center;
		radius = _radius;
		material = _material;
	}

	Hit intersect(const Ray& ray) {
		Hit hit;
		vec3 dist = ray.start - center;
		float a = dot(ray.dir, ray.dir);
		float b = dot(dist, ray.dir) * 2.0f;
		float c = dot(dist, dist) - radius * radius;
		float discr = b * b - 4.0f * a * c;
		if (discr < 0) return hit;
		float sqrt_discr = sqrtf(discr);
		float t1 = (-b + sqrt_discr) / 2.0f / a;	// t1 >= t2 for sure
		float t2 = (-b - sqrt_discr) / 2.0f / a;
		if (t1 <= 0) return hit;
		hit.t = (t2 > 0) ? t2 : t1;
		hit.position = ray.start + ray.dir * hit.t;
		hit.normal = (hit.position - center) * (1.0f / radius);
		hit.material = material;
		return hit;
	}
};

/// Bezier osztaly forras: kiadott programokból koppintottam
/// A transzformációkat 
class Ushape {

	float wx = 0,wy = 0;
	std::vector<vec4> wCtrlPoints;

	float B(int i, float t) {
		int n = wCtrlPoints.size() - 1; // n deg polynomial = n+1 pts!
		float choose = 1;
		for (int j = 1; j <= i; j++) choose *= (float)(n - j + 1) / j;
		return choose * pow(t, i) * pow(1 - t, n - i);
	}

public:

	float tStart() { return 0; }
	float tEnd() { return 1; }

	vec4 r(float t) {
		vec4 wPoint = vec4(0, 0, 0, 0);
		for (unsigned int n = 0; n < wCtrlPoints.size(); n++) wPoint += wCtrlPoints[n] * B(n, t);
		return wPoint;
	}

	virtual void AddControlPoint(vec2 cPoint) {
		vec4 wVertex = vec4(cPoint.x, cPoint.y, 0, 1);
		wCtrlPoints.push_back(wVertex);
	}

	Ushape(std::vector<Light *> &lights, Material material, mat4 model) {

		AddControlPoint(vec2(0.0f,0.0f));
		AddControlPoint(vec2(0.0f,1.0f));
		AddControlPoint(vec2(0.1f, 1.0f));
		AddControlPoint(vec2(0.1f,0.0f));

		mat4 transformation = TranslateMatrix(vec3(8.0f,0.0f,2.0f)) * RotationMatrix(3.2f, vec3(1.0f,1.0f,1.0f));

	
		for (int i = 0; i < 100; i++) {
			
			float tNormalized = (float)i / (100 - 1);
			float t = tStart() + (tEnd() - tStart()) * tNormalized;
			
			vec4 wVertex = r(t);
			wVertex.x = cos(wVertex.x);
			wVertex.y = sin(wVertex.x);
			wVertex.z = wVertex.y;

			wVertex = wVertex * transformation;

			//Sphere* newobject = new Sphere(vec3(wVertex.x, wVertex.y, wVertex.z), 0.02f, &material);
			PointLight *newlight = new PointLight(vec3(2,2,2), vec3(wVertex.x, wVertex.y, wVertex.z));
			lights.push_back(newlight);
		}
	}
};

// Forras: raytrace.cpp
class Camera {
	vec3 eye, lookat, right, up;
public:
	void set(vec3 _eye, vec3 _lookat, vec3 vup, float fov) {
		eye = _eye;
		lookat = _lookat;
		vec3 w = eye - lookat;
		float focus = length(w);
		right = normalize(cross(vup, w)) * focus * tanf(fov / 2);
		up = normalize(cross(w, right)) * focus * tanf(fov / 2);
	}
	Ray getRay(int X, int Y) {
		vec3 dir = lookat + right * (2.0f * (X + 0.5f) / windowWidth - 1) + up * (2.0f * (Y + 0.5f) / windowHeight - 1) - eye;
		return Ray(eye, dir);
	}
};

const float epsilon = 0.01f;

// Forras: raytrace.cpp
class Scene {
	std::vector<Intersectable *> objects;
	std::vector<Light *> lights;
	Camera camera;
	vec3 La;
public:
	void build() {

		mat4 cylindermodel = mat4(1.0f, 0.0f, 0.0f, 0.0f,     
		                      		0.0f, 1.0f, 0.0f, 0.0f,    
		                      		0.0f, 0.0f, 0.0f, 0.0f,
		                      		0.0f, 0.0f, 0.0f, -2.0f);

		mat4 hiperboloidmodel = mat4(1.0f, 0.0f, 0.0f, 0.0f,     
		                      		0.0f, 1.0f, 0.0f, 0.0f,    
		                      		0.0f, 0.0f, -1.0f, 0.0f,
		                      		0.0f, 0.0f, 0.0f, -1.0f);

		// 15 0 2                            0  1  0 
		vec3 eye = vec3(-5, 0, 2), vup = vec3(0, 1, 0), lookat = vec3(0, 0, 0);
		
		float fov = 45 * M_PI / 180;
		camera.set(eye, lookat, vup, fov);

		La = vec3(0.4f, 0.4f, 0.4f);
		vec3 lightDirection(1, 1, 1), Le(2, 2, 2);
		lights.push_back(new Light(lightDirection, Le));

		vec3 kd(0.3f, 0.2f, 0.1f), ks(2, 2, 2);
		Material * material = new Material(kd, ks, 50);
		
		//objects.push_back(new Hyperboloid(material, hiperboloidmodel));
		objects.push_back(new Cylinder(material, cylindermodel));
		new Ushape(lights, *material, cylindermodel);
	}

	void render(std::vector<vec4>& image) {
		for (int Y = 0; Y < windowHeight; Y++) {
			#pragma omp parallel for
			for (int X = 0; X < windowWidth; X++) {
				vec3 color = trace(camera.getRay(X, Y));
				image[Y * windowWidth + X] = vec4(color.x, color.y, color.z, 1);
			}
		}
	}

	Hit firstIntersect(Ray ray) {
		Hit bestHit;
		for (Intersectable * object : objects) {
			Hit hit = object->intersect(ray); 
			if (hit.t > 0 && (bestHit.t < 0 || hit.t < bestHit.t))  bestHit = hit;
		}
		if (dot(ray.dir, bestHit.normal) > 0) bestHit.normal = bestHit.normal * (-1);
		return bestHit;
	}

	bool shadowIntersect(Ray ray) {	
		for (Intersectable * object : objects) if (object->intersect(ray).t > 0) return true;
		return false;
	}

	vec3 trace(Ray ray, int depth = 0) {
		Hit hit = firstIntersect(ray);
		if (hit.t < 0) return La;
		vec3 outRadiance = hit.material->ka * La;
		for (Light * light : lights) {
			Ray shadowRay(hit.position + hit.normal * epsilon, light->directionOf(hit.position));
			float cosTheta = dot(hit.normal, light->directionOf(hit.position));
			if (cosTheta > 0 && !shadowIntersect(shadowRay)) {	// shadow computation
				outRadiance = outRadiance + light->Le * hit.material->kd * cosTheta;
				vec3 halfway = normalize(-ray.dir + light->directionOf(hit.position));
				float cosDelta = dot(hit.normal, halfway);
				if (cosDelta > 0) outRadiance = outRadiance + light->Le * hit.material->ks * powf(cosDelta, hit.material->shininess);
			}
		}
		return outRadiance;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////
/// Az alabbi kod a raytrace.cpp fajlbol lett idemasolva, modositatlanul
/// ehhez a reszhez en mar nem nagyon nyultam, csak efelett dolgoztam a kódon
/// (hacsak nem vettem eszre valamit es veletlen beleirtam) 
/// Azon kivul mas valtoztatast nem vegeztem rajta
////////////////////////////////////////////////////////////////////////////////////////////////

GPUProgram gpuProgram;
Scene scene;

const char *vertexSource = R"(
	#version 330
    precision highp float;

	layout(location = 0) in vec2 cVertexPosition;	// Attrib Array 0
	out vec2 texcoord;

	void main() {
		texcoord = (cVertexPosition + vec2(1, 1))/2;							// -1,1 to 0,1
		gl_Position = vec4(cVertexPosition.x, cVertexPosition.y, 0, 1); 		// transform to clipping space
	}
)";

const char *fragmentSource = R"(
	#version 330
    precision highp float;

	uniform sampler2D textureUnit;
	in  vec2 texcoord;			// interpolated texture coordinates
	out vec4 fragmentColor;		// output that goes to the raster memory as told by glBindFragDataLocation

	void main() {
		fragmentColor = texture(textureUnit, texcoord); 
	}
)";

class FullScreenTexturedQuad {
	unsigned int vao;
	Texture texture;
public:
	FullScreenTexturedQuad(int windowWidth, int windowHeight, std::vector<vec4>& image)
		: texture(windowWidth, windowHeight, image) 
	{
		glGenVertexArrays(1, &vao);	
		glBindVertexArray(vao);	

		unsigned int vbo;		
		glGenBuffers(1, &vbo);	

		glBindBuffer(GL_ARRAY_BUFFER, vbo); 
		float vertexCoords[] = { -1, -1,  1, -1,  1, 1,  -1, 1 };
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexCoords), vertexCoords, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	}

	void Draw() {
		glBindVertexArray(vao);	
		gpuProgram.setUniform(texture, "textureUnit");
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}
};

FullScreenTexturedQuad * fullScreenTexturedQuad;

void onInitialization() {
	glViewport(0, 0, windowWidth, windowHeight);
	scene.build();

	std::vector<vec4> image(windowWidth * windowHeight);
	long timeStart = glutGet(GLUT_ELAPSED_TIME);
	scene.render(image);
	long timeEnd = glutGet(GLUT_ELAPSED_TIME);
	printf("Rendering time: %d milliseconds\n", (timeEnd - timeStart));

	fullScreenTexturedQuad = new FullScreenTexturedQuad(windowWidth, windowHeight, image);

	gpuProgram.create(vertexSource, fragmentSource, "fragmentColor");
}

void onDisplay() {
	fullScreenTexturedQuad->Draw();
	glutSwapBuffers();
}

void onKeyboard(unsigned char key, int pX, int pY) {
}

void onKeyboardUp(unsigned char key, int pX, int pY) {

}

void onMouse(int button, int state, int pX, int pY) {
}

void onMouseMotion(int pX, int pY) {
}

void onIdle() {
}
