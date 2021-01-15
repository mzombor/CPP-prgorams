#include "framework.h"
 
 
/// Forras: kiadott keret (smoothtriangle.cpp)
const char * vertexSource = R"(
	#version 330				
    precision highp float;
 
	uniform mat4 MVP;			
 
	layout(location = 0) in vec2 vertexPosition;	
	layout(location = 1) in vec3 vertexColor;	    
	
	out vec3 color;									
 
	void main() {
		color = vertexColor;														
		gl_Position = vec4(vertexPosition.x, vertexPosition.y, 0, 1) * MVP;
	}
)";
 
/// Forras: kiadott keret (smoothtriangle.cpp)
const char * const fragmentSource = R"(
	#version 330
	precision highp float;
 
	uniform vec3 u_color;	
	out vec4 out_finalColor;
 
	void main() {
		out_finalColor = vec4(u_color, 1.0);
	}
)";
 
/// Forras: https://stackoverflow.com/questions/2049582/how-to-determine-if-a-point-is-in-a-2d-triangle
inline bool contains(vec2 &a, vec2 &b, vec2 &c, vec2 &p) {
       
	   	bool d1 = (c.x - p.x) * (a.y - p.y) - (a.x - p.x) * (c.y - p.y) >= 0;
        bool d2 = (a.x - p.x) * (b.y - p.y) - (b.x - p.x) * (a.y - p.y) >= 0;
        bool d3 = (b.x - p.x) * (c.y - p.y) - (c.x - p.x) * (b.y - p.y) >= 0;
 
		return d1 && d2 && d3;
}
 
/// Forras: https://math.stackexchange.com/questions/274712/calculate-on-which-side-of-a-straight-line-is-a-given-point-located 
float get_side(vec2 &v_1, vec2 &v0, vec2 &v1) {
	return (v0.x - v_1.x) * (v1.y - v_1.y) - (v0.y - v_1.y) * (v1.x - v_1.x);
}
 
 
/// Forras: kiadott keret (smoothtriangle.cpp)
struct Camera2D {
	vec2 wCenter; 
	vec2 wSize;   
 
	Camera2D() : wCenter(0, 0), wSize(2, 2) { }
 
	mat4 P() { return ScaleMatrix(vec2(2 / wSize.x, 2 / wSize.y)); }
 
	void Zoom(float s) { wSize = wSize * s; }
};
 
vec3 spline_color = vec3 (0.0f, 1.0f, 1.0f);
vec3 outline_color = vec3 (1.0f, 1.0f, 1.0f);
GPUProgram program;
Camera2D camera;
 
struct Spline {
 
	GLuint vao, vbo;
	float area;
 
	vec2 center;
	bool animate = false;
	bool rightflow = true;
 
	std::vector <vec2> cps_cache;
	
	std::vector <vec2> cps;
	std::vector <float> ts;
 
	std::vector <vec2> tangents;
	std::vector <vec2> vertices;
	std::vector <vec2> triangles;
 
	std::vector<vec2> arcs;
	float r;
 
	float sx = 1, sy = 1;
	vec2 wTranslate = vec2(0,0);	
 
	void addCp (vec2 cp) {
 
		cps_cache.push_back (cp);
 
		if (cps_cache.size () >= 3) { 
 
			if (cps_cache[0].x < cps_cache[1].x && cps_cache[0].y > cps_cache[1].y) rightflow = false;
 
			cps.clear ();
			cps.push_back (cps_cache [cps_cache.size () - 1]);
			
			for (auto &cp : cps_cache) cps.push_back (cp);
 
			cps.push_back (cps_cache [0]);
			cps.push_back (cps_cache [1]);
			cps.push_back (cps_cache [2]);
			
			if(cps_cache.size() == 3) {
				cps.push_back(cps_cache[0]);
			}
			else cps.push_back(cps_cache[3]);
 
		}
 
		get_ts_list ();
		get_tangents ();
		get_points ();
	}
 
	/// Forras: https://en.wikipedia.org/wiki/Cubic_Hermite_spline kb az elso keplet az oldalon
	inline vec2 HermitInterpolation (vec2 &p0, vec2 &m0, vec2 &p1, vec2 &m1, float &t) {
		return p0 * (2.0f * t * t * t - 3.0f * t * t + 1.0f)
			+ m0 * (t * t * t - 2.0f * t * t + t)
			+ p1 * (-2.0f * t * t * t + 3.0f * t * t)
			+ m1 * (t * t * t - t * t);
	}
 
	/// Forras: maga a knot value számolás az alábbi képelettel megy 
	/// https://en.wikipedia.org/wiki/Centripetal_Catmull%E2%80%93Rom_spline
	void get_ts_list () {
 
		ts.clear ();
 
		if (cps_cache.size () >= 3) {
 
			float prev = 0.0f;
 
			for (int i = 0; i < cps.size () - 1; i++) {
 
				vec2 diff = cps[i] - cps[i+1];
				
				float kv = pow(length (diff), 0.5f) + prev;
				ts.push_back (kv);
 
				prev = kv;
			}
		}
	}
 
	/// Forras: valamelyik regi matek konyvem ...
	void get_tangents () {
 
		tangents.clear ();
		if (cps_cache.size () >= 3) {
 
			for (int i = 1; i < cps.size () - 2; i++) {
 
				vec2 cp0 = cps [i - 1];
				vec2 cp1 = cps [i + 1];
				float kv0 = ts [i - 1];
				float kv1 = ts [i + 1];
 
				vec2 dir = cp1 - cp0;
				float val = kv1 - kv0;
 
				vec2 tangent;
				if (abs (kv1 - kv0) <= 0.0001f) {
					tangent = dir;
				} else {
					tangent = dir * (1.0f / val);
				}
 
				tangents.push_back (tangent);
 
			}
 
		}
 
	}
 
	/// Forras: https://en.wikipedia.org/wiki/Cubic_Hermite_spline
	void get_points () {
 
		vertices.clear ();
 
		if (cps_cache.size () >= 3) {
			
 
			float step = (float) (cps_cache.size () + 1) / (float) 100;
			float totalTime = 0.0f;
 
			for (int i = 0; i < 100; i++) {
 
				float time = totalTime - (int) totalTime;
				int cpi = (int) totalTime;
 
				vec2 cp0 = cps [1 + cpi];
				vec2 cp1 = cps [2 + cpi];
				vec2 m0 = tangents [cpi];
				vec2 m1 = tangents [cpi + 1];
 
				vertices.push_back (HermitInterpolation (cp0, m0, cp1, m1, time));
 
				totalTime += step;
 
			}
 
			get_area (vertices);
			centroid ();
		}
	}
 
	/// Forras: https://en.wikipedia.org/wiki/Shoelace_formula
	void get_area(std::vector<vec2> vertices) {
		
		area = 0;
		vec2 last = vertices [vertices.size()-1];
		vec2 beforelast = vertices [vertices.size()-2];
		
		if (cps_cache.size() >= 3) {
			
			area += vertices[0].x * (vertices[1].y - last.y);
 
			for (int i = 1; i < vertices.size()-1; i++) {
				float ydif = vertices[i+1].y - vertices[i-1].y;
				area += vertices[i].x * ydif;
			}
 
			area += last.x * (vertices[0].y - beforelast.y);
			area *= 0.5f;
 
			if(area < 0) {
				r = -1 * sqrt(fabs(area) * (1.0f / 3.1415));
			}
			else {
				area = fabs(area) * (1.0f / 3.1415);
			}
			
		}
	}
 
	/// Forras: kiadott keret (smoothtriangle.cpp)
	mat4 M() {
		mat4 Mscale(sx, 0, 0, 0,
				    0, sy, 0, 0,
				    0, 0,  0, 0,
					0, 0,  0, 1);
 
 
		mat4 Mtranslate(1,            0,            0, 0,
			            0,            1,            0, 0,
			            0,            0,            0, 0,
			            wTranslate.x, wTranslate.y, 0, 1);
 
		return Mscale * Mtranslate;	
	}
 
	/// Forras: https://stackoverflow.com/questions/2792443/finding-the-centroid-of-a-polygon
	/// valamint https://math.stackexchange.com/questions/90463/how-can-i-calculate-the-centroid-of-polygon
	void centroid(){
		
		center = vec2(0,0);
		for(size_t i = 0; i < vertices.size()-1; i++){
			center = center + vertices[i];
		}
 
		center = center * (1.0f / (float)vertices.size());
	}
 
	/// Forras: Hazi kiados eloadason kerult fel a tablara
	vec2 get_lagrange_curve(vec2 r1, vec2 r2, vec2 r3, vec2 vertex, float t){
 
		vec2 left = normalize(r1-r2) * 0.5f;
		vec2 right = normalize(r3-r2) * 0.5f;
		
		vec2 res = (center + (normalize(left+right) * (r * 0.80f)) );
		
		return res;
	}
 
	void transform() {
 
		std::vector <vec2> new_points;
		vec2 first = vertices [0];
		vec2 last = vertices [vertices.size ()-1];
 
		if (cps_cache.size () >= 3) {
 
			if(arcs.size () != vertices.size ()) {			
				arcs.push_back (get_lagrange_curve (last, first, vertices[1], first, first.x));
				for (int i = 1; i < 100; i++) {
					arcs.push_back (get_lagrange_curve (vertices[i-1], vertices[i], vertices[i+1], vertices[i], vertices[i].x));
				}
				arcs.push_back (get_lagrange_curve (vertices[vertices.size()-2], last, first, last, last.x));
			}
 
			for ( auto &p: vertices) {
				new_points.push_back(p);
			}
 
			vertices.clear();
			for (int i = 0; i < 100; i++) {
				vertices.push_back (new_points[i] + ((arcs[i] - new_points[i])*0.005f) );
			}
 
			new_points.clear ();
		}
 
		//sort_vertices
		get_area (vertices);
	}
 
	
	void Triangulate () {
 
		if (cps_cache.size () < 3) return;
 
		if(animate) {
			if(!rightflow) {
				std::vector<vec2> temp = cps_cache;
				cps_cache.clear();
				cps_cache.push_back(temp[0]);
				for(int i = temp.size()-1; i > 0; i--) cps_cache.push_back(temp[i]);
 
				cps.clear ();
				cps.push_back (cps_cache [cps_cache.size () - 1]);
 
				for (auto &cp : cps_cache) cps.push_back (cp);
 
				cps.push_back (cps_cache [0]);
				cps.push_back (cps_cache [1]);
				cps.push_back (cps_cache [2]);
 
				if(cps_cache.size() == 3) {
					cps.push_back(cps_cache[0]);
				}
				else cps.push_back(cps_cache[3]);
				rightflow = true;
 
				get_points();			
			}
		}
 
		std::vector<vec2> points;
		for (auto &v : vertices) points.push_back (v);
 
		triangles.clear ();
		for(int k = 0;k < 100; k++) {
 
			int earIndex = -1;
			for (int i = 0; i < points.size (); i++) {
 
				vec2 v_1 = points[0];
				if (i == 0) v_1 = points [points.size() -1]; 
				else v_1 = points [i - 1];
 
				vec2 v0 = points [i];
 
				vec2 v1 = points [0];
				if (i == points.size()-1) v1 = points [0]; 
				else v1 = points [i + 1];
				
				if(!rightflow) {
					if (get_side(v_1, v0, v1) > 0) {
						for (int j = 0; j < points.size (); j++) {
 
							auto &p = points [j];
	
							if (contains (v1, v0, v_1, p)) break;
							else {
								earIndex = i;
								break;
							}
						}
					}
				}
				else {
					if(get_side(v_1, v0, v1) < 0) {
						for (int j = 0; j < points.size (); j++) {
 
							auto &p = points [j];
	
							if (contains (v1, v0, v_1, p)) break;						
							else {
								earIndex = i;
								break;
							}
						}
					}
				}
			}
 
 
			if (earIndex >= 0) {
				
				triangles.push_back (points [earIndex == 0 ? points.size () - 1 : earIndex - 1]);
				triangles.push_back (points [earIndex]);
				triangles.push_back (points [earIndex == points.size () - 1 ? 0 : earIndex + 1]);
				
				points.erase (points.begin () + earIndex);
			}
 
		}
	}
 
	void Init () {
 
		glGenVertexArrays (1, &vao);
		glGenBuffers (1, &vbo);
 
		glBindVertexArray (vao);
 
		glBindBuffer (GL_ARRAY_BUFFER, vbo);
 
		glEnableVertexAttribArray (0);
		glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, 0);
 
		glBindVertexArray (0);
		glBindBuffer (GL_ARRAY_BUFFER, 0);
 
	}
 
	void Draw () {
 
		triangles.clear();
		Triangulate();
 
		glBindBuffer (GL_ARRAY_BUFFER, vbo);
		glBufferData (GL_ARRAY_BUFFER, triangles.size() * sizeof (vec2), &(triangles [0]), GL_DYNAMIC_DRAW);
		glBindBuffer (GL_ARRAY_BUFFER, 0);
 
		///// Forras: kiadott keret (smoothtriangle.cpp)		
		mat4 MVPTransform = M() * camera.P();
		program.setUniform(MVPTransform, "MVP");
 
		program.setUniform(spline_color, "u_color");
//
		glBindVertexArray (vao);
		glDrawArrays (GL_TRIANGLES, 0, triangles.size ());
		glBindVertexArray (0);
	
 
		glBindBuffer (GL_ARRAY_BUFFER, vbo);
		glBufferData (GL_ARRAY_BUFFER, vertices.size() * sizeof (vec2), &(vertices [0]), GL_DYNAMIC_DRAW);
		glBindBuffer (GL_ARRAY_BUFFER, 0);
		
		program.setUniform(outline_color, "u_color");
		
		glBindVertexArray (vao);
		glDrawArrays (GL_LINE_STRIP, 0, vertices.size());
		glBindVertexArray (0);
		
	}
 
};
 
Spline spline;
 
void onInitialization() {
	
	glViewport(0, 0, windowWidth, windowHeight);
 
	program.create (vertexSource, fragmentSource, "out_finalColor");
 
	spline.Init ();
 
}
 
void onDisplay() {
	
	glClearColor(0, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
 
	spline.Draw();
 
	glutSwapBuffers();
}
 
void onKeyboard(unsigned char key, int pX, int pY) {
	switch (key) {
		case 'p': spline.wTranslate = spline.wTranslate + vec2(0.1f, 0.0f); break;
		case 'd': spline.animate = true; break;
		case 'z': camera.Zoom(0.9f); break;
	}
	glutPostRedisplay();   	
}
 
void onKeyboardUp(unsigned char key, int pX, int pY) {
}
 
void onMouseMotion(int pX, int pY) {
	
	float cX = 2.0f * pX / windowWidth - 1;
	float cY = 1.0f - 2.0f * pY / windowHeight;
	
}
 
void onMouse(int button, int state, int pX, int pY) {
	
	float cX = 2.0f * pX / windowWidth - 1.0f;;
	float cY = 1.0f - 2.0f * pY / windowHeight;
 
	switch (button) {
		case GLUT_LEFT_BUTTON:
			if (state == GLUT_UP) {
				spline.addCp(vec2 (cX, cY));
			}
			break;
	}
 
}
 
// Forras: eloadas hw/sw alapok
void onIdle() {	
 
	static float tend = 0;
	const float dt = 0.001;
	float tstart = tend;
	tend = glutGet(GLUT_ELAPSED_TIME)/1000.0f;
 
	if (spline.animate) {
		for(float t = tstart; t < tend; t+= dt) {
			spline.transform();
		}
	}
 
	glutPostRedisplay();

}

