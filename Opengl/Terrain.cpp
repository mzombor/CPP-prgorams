#include "framework.h"

float tessellationLevel = 80;

/// Forras: 3dendzsinke
struct Camera
{
    vec3 wEye, wLookat, wVup;
    float fov, asp, fp, bp;
public:
    Camera() {
        asp = (float) windowWidth / windowHeight;
        fov = 80.0f * (float) M_PI / 180.0f;
        fp = 1; bp = 10;
    }

    mat4 V() {
        vec3 w = normalize(wEye - wLookat);
        vec3 u = normalize(cross(wVup, w));
        vec3 v = cross(w, u);
        return TranslateMatrix(wEye * (-1)) * mat4(
                u.x, v.x, w.x, 0,
                u.y, v.y, w.y, 0,
                u.z, v.z, w.z, 0,
                0, 0, 0, 1
        );
    }

    mat4 P() {
        return mat4(
                1 / (tan(fov / 2) * asp), 0, 0, 0,
                0, 1 / tan(fov / 2), 0, 0,
                0, 0, -(fp + bp) / (bp - fp), -1,
                0, 0,-2 * fp * bp / (bp - fp), 0
        );
    }

};

/// Forras: 3dendzsinke
struct Material {
    vec3 kd, ks;
    float shininess;
};

struct Light {
    vec3 Le;
    vec4 wLightPos;
};

/// Forras: 3dendzsinke
struct RenderState {
    mat4	           MVP, M, Minv, V, P;
    Material *         material;
    std::vector<Light> lights;
    vec3	           wEye;
};

/// Forras: 3dendzsinke 
class Shader : public GPUProgram {
public:
    virtual void Bind(RenderState state) = 0;

    void setUniformMaterial(const Material& material, const std::string& name) {
        setUniform(material.kd, name + ".kd");
        setUniform(material.ks, name + ".ks");
        setUniform(material.shininess, name + ".shininess");
    }

    void setUniformLight(const Light& light, const std::string& name) {
        setUniform(light.Le, name + ".Le");
        setUniform(light.wLightPos, name + ".wLightPos");
    }
};

/// Forras: 3dendzsinke
class Terrain_Shader : public Shader {
    const char * vertexSource = R"(
		#version 330
		precision highp float;
		
        struct Light {
			vec3 Le;
			vec4 wLightPos;
		};
		
        uniform mat4  MVP, M, Minv;
		uniform Light[2] lights;

        uniform int   nLights;
		uniform vec3  wEye;
       
		layout(location = 0) in vec3  vtxPos;
		layout(location = 1) in vec3  vtxNorm;
		
        out vec3 wNormal;
		out vec3 wView;
		out vec3 wLight[2];
        out float z;

        void main() {

            //z = ((vtxPos.z) * 2.2) * 0.4;
            z = ((vtxPos.z) + 4) * 0.2;

			gl_Position = vec4(vtxPos, 1) * MVP;
			vec4 wPos = vec4(vtxPos, 1) * M;
			
            for(int i = 0; i < nLights; i++) {
				wLight[i] = lights[i].wLightPos.xyz * wPos.w - wPos.xyz * lights[i].wLightPos.w;
			}
		    
            wView  = wEye * wPos.w - wPos.xyz;
		    wNormal = (Minv * vec4(vtxNorm, 0)).xyz;
		}
	)";

    const char * fragmentSource = R"(
		#version 330
		precision highp float;
		
        struct Light {
			vec3 Le;
			vec4 wLightPos;
		};
		
        struct Material {
			vec3 kd, ks;
			float shininess;
		};
		
        uniform Material material;
		uniform Light[2] lights;
        uniform int   nLights;
		
        in  vec3 wNormal;
		in  vec3 wView;
		in  vec3 wLight[2];
        in  float z;
        
        const vec3 g = vec3(0.1, 0.8, 0.3);
        const vec3 b = vec3(0.3, 0.2, 0.1);
        out vec4 fragmentColor;
		
        void main() {
           
			vec3 N = normalize(wNormal);
			vec3 V = normalize(wView);
		
        	vec3 kd = material.kd * (g * (1 - z) + b * z);
			
            vec3 radiance = vec3(0, 0, 0);
		
        	for(int i = 0; i < nLights; i++) {
				vec3 L = normalize(wLight[i]);
				vec3 H = normalize(L + V);
			
            	float cost = max(dot(N,L), 0), cosd = max(dot(N,H), 0);
				radiance += (kd * cost + material.ks * pow(cosd, material.shininess)) * lights[i].Le;
			}
		
        	fragmentColor = vec4(radiance, 1);
		}
	)";
public:
    Terrain_Shader() {
        create(vertexSource, fragmentSource, "fragmentColor");
    }

    void Bind(RenderState state) {
        Use();

        setUniform(state.MVP, "MVP");
        setUniform(state.M, "M");
        setUniform(state.Minv, "Minv");
        setUniform(state.wEye, "wEye");
        setUniformMaterial(*state.material, "material");
        setUniform((int)state.lights.size(), "nLights");

        for (unsigned int i = 0; i < state.lights.size(); i++) {
            setUniformLight(state.lights[i], std::string("lights[") + std::to_string(i) + std::string("]"));
        }
    }
};

/// Forras: 3dendzsinke
class Railway_Shader : public Shader {
    const char * vertexSource = R"(
		#version 330
		precision highp float;

		struct Light {
			vec3 Le;
			vec4 wLightPos;
		};

		uniform mat4  MVP, M, Minv;
		uniform Light[2] lights;

        uniform int   nLights;
		uniform vec3  wEye;

		layout(location = 0) in vec3  vtxPos;
		layout(location = 1) in vec3  vtxNorm;

		out vec3 wNormal;
		out vec3 wView;
		out vec3 wLight[2];

		void main() {
			gl_Position = vec4(vtxPos, 1) * MVP;
		
        	vec4 wPos = vec4(vtxPos, 1) * M;
			for(int i = 0; i < nLights; i++) {
				wLight[i] = lights[i].wLightPos.xyz * wPos.w - wPos.xyz * lights[i].wLightPos.w;
			}
		
            wView  = wEye * wPos.w - wPos.xyz;
		    wNormal = (Minv * vec4(vtxNorm, 0)).xyz;
		}
	)";

    const char * fragmentSource = R"(
		#version 330
		precision highp float;

		struct Light {
			vec3 Le;
			vec4 wLightPos;
		};

		struct Material {
			vec3 kd, ks;
			float shininess;
		};

		uniform Material material;
		uniform Light[2] lights;
        uniform int   nLights;

		in  vec3 wNormal;
		in  vec3 wView;
		in  vec3 wLight[2];

        out vec4 fragmentColor;

        vec3 Fresnel(vec3 F0, float cosTheta) {
            return F0 + (vec3(1, 1, 1) - F0) * pow(cosTheta, 5);
        }

		void main() {
			
            vec3 N = normalize(wNormal);
			vec3 V = normalize(wView);
			
			vec3 kd = material.kd;
			vec3 radiance = vec3(0, 0, 0);
			
            for(int i = 0; i < nLights; i++) {
			
            	vec3 L = normalize(wLight[i]);
				vec3 H = normalize(L + V);
			
            	float cost = max(dot(N,L), 0), cosd = max(dot(N,H), 0);
				radiance += kd * lights[i].Le + (material.ks * pow(cosd, material.shininess)) * lights[i].Le;
			}

			fragmentColor = vec4(radiance, 1);
		}
	)";
public:
    Railway_Shader() {
        create(vertexSource, fragmentSource, "fragmentColor");
    }

    void Bind(RenderState state) {
        Use();
        setUniform(state.MVP, "MVP");
        setUniform(state.M, "M");
        setUniform(state.Minv, "Minv");
        setUniform(state.wEye, "wEye");
        setUniformMaterial(*state.material, "material");

        setUniform((int)state.lights.size(), "nLights");
        for (unsigned int i = 0; i < state.lights.size(); i++) {
            setUniformLight(state.lights[i], std::string("lights[") + std::to_string(i) + std::string("]"));
        }
    }
};

/// Forras: 3dendzsinke
struct VertexData {
    vec3 position, normal;
};

/// Forras: 3dendzsinke
class Geometry {
protected:
    unsigned int vao, vbo;
public:
    Geometry() {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
    }
    virtual void draw() = 0;
    ~Geometry() {
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }
};

/// Forras: 3dendzsinke
class ParamSurface : public Geometry {
protected:
    unsigned int nVtxPerStrip, nStrips;
public:
    ParamSurface() { nVtxPerStrip = nStrips = 0; }

    virtual VertexData genVertexData(float u, float v) = 0;

    void create(int N = tessellationLevel, int M = tessellationLevel) {
        
        nVtxPerStrip = (M + 1) * 2;
        nStrips = N;
        
        std::vector<VertexData> vtxData;
        
        for (int i = 0; i < N; i++) {
            for (int j = 0; j <= M; j++) {
                vtxData.push_back(genVertexData((float) j / M, (float) i / N));
                vtxData.push_back(genVertexData((float) j / M, (float) (i + 1) / N));
            }
        }
        glBufferData(GL_ARRAY_BUFFER, nVtxPerStrip * nStrips * sizeof(VertexData), &vtxData[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, position));
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)offsetof(VertexData, normal));
    }

    void draw() {
        glBindVertexArray(vao);
        for (unsigned int i = 0; i < nStrips; i++)
            glDrawArrays(GL_TRIANGLE_STRIP, i *  nVtxPerStrip, nVtxPerStrip);
    }
};

/// Forras: 3dendzsinke alapjan, az egyenlet kb 3 oras totyorges es probalgatas eredmenye
class Terrain : public ParamSurface {
public:

    vec3 Point(float u, float v) {
		float x = u;
        float y = v;
        float z =  cosf(-1.0f * u + 0.3f * v + 0.23f) + sinf(0.3f * u + v);

        return vec3(x,y,z);
	}

     vec3 Normal(float u, float v) {
        vec3 normal;

        normal.x = (sinf(-1 * u - 0.3f * v + 0.23f) + 0.3f * cosf(0.3f * u + v));
        normal.y = (-0.3f * sinf(-1 * u + 0.3f * v + 0.23f) + cosf(0.3f * u + v));
        normal.z = 1;
        
        return normalize(normal);
    }

    Terrain() { create(); }

    VertexData genVertexData(float u, float v) {
    
        VertexData vd;
        u *= 15;
        v *= 15;

        vd.position = Point(u,v);
        vd.normal = Normal(u, v);
    
        return vd;
    }
};



/// Forras: 3dendzsinke
struct Object {
    Shader *   shader;
    Material * material;
    Geometry * geometry;
    vec3 translation;
public:
    Object(Shader * _shader, Material * _material, Geometry * _geometry) :
    translation(vec3(0, 0, 0)) {
        shader = _shader;
        material = _material;
        geometry = _geometry;
    }
    virtual void setModelingTransform(mat4& M, mat4& Minv) {
        M = TranslateMatrix(translation);
        Minv = TranslateMatrix(-translation);
    }

    void draw(RenderState state) {
        mat4 M, Minv;
        setModelingTransform(M, Minv);
        state.M = M;
        state.Minv = Minv;
        state.MVP = state.M * state.V * state.P;
        state.material = material;
        shader->Bind(state);
        geometry->draw();
    }
};

// Forras: a kod nagyreszet az elso hazibol masoltam, kisse atalakitottam kozben
struct Spline {
 
	GLuint vao, vbo;
 
    Camera camera;
    Shader* shader;

	vec2 center;
    vec3 translation;

    Material * material;

	std::vector <vec2> cps_cache;
	
	std::vector <vec2> cps;
	std::vector <float> ts;
 
	std::vector <vec2> tangents;
	std::vector <VertexData> vertices;
 
	float sx = 1, sy = 1;
	vec2 wTranslate = vec2(0,0);	

    Spline() {
        vao = 0; vbo = 0;
    }

    Spline (Shader* _shader, Material* _material) {
        shader = _shader;
        material = _material;
    }

	void addCp (vec2 cp) {
 
		cps_cache.push_back (cp);
 
		if (cps_cache.size () >= 3) { 

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
 
	inline vec2 HermitInterpolation (vec2 &p0, vec2 &m0, vec2 &p1, vec2 &m1, float &t) {
		return p0 * (2.0f * t * t * t - 3.0f * t * t + 1.0f)
			+ m0 * (t * t * t - 2.0f * t * t + t)
			+ p1 * (-2.0f * t * t * t + 3.0f * t * t)
			+ m1 * (t * t * t - t * t);
	}
 
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
 
                vec2 uv = HermitInterpolation (cp0, m0, cp1, m1, time);

                // Something is terribly wrong here ...

                Terrain tr = Terrain();
                float z = tr.Point(uv.x, uv.y).z + 2.0f;

                VertexData vtx = VertexData();
                vtx.position = vec3(uv.x + 5, uv.y + 5, z);
                vtx.normal = tr.Normal(uv.x, uv.y);

                vertices.push_back(vtx);

				totalTime += step;
 
			}
		}
	}
 
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

    VertexData genVertexData(float u, float v) {
        return VertexData();
    }

    /// Forras: 3dendzsinke.cpp
    virtual void setModelingTransform(mat4& M, mat4& Minv) {
        M = TranslateMatrix(translation);
        Minv = TranslateMatrix(-translation);
    }

    /// Forras: reszben 3dendzsinke.cpp reszben sajat mokolas 
    void draw(RenderState state) {

        mat4 M, Minv;
     
        setModelingTransform(M, Minv);
        state.M = M;
        state.Minv = Minv;
        state.MVP = state.M * state.V * state.P;
        state.material = material;
     
        shader->Bind(state);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
		

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexData), &vertices[0], GL_STATIC_DRAW);

        glDrawArrays (GL_TRIANGLE_STRIP, 0, vertices.size());
    }
 
};

/// Forras: 3dendzsinke
class Scene {

    std::vector<Object *> objects;
    Camera * camera;
    
    std::vector<Light> lights;
    std::vector<Shader *> shaders;
    std::vector<Material *> materials;
    Terrain * terrain;

public:

    Spline spline;

    void build() {
        shaders.push_back(new Terrain_Shader());
        shaders.push_back(new Railway_Shader());

        materials.push_back(new Material);
        materials[0]->kd = vec3(1, 1, 1);
        materials[0]->ks = vec3(0.2, 0.2, 0.2);
        materials[0]->shininess = 3;

        materials.push_back(new Material);
        materials[1]->kd = vec3(0.2f, 0.2f, 0.1f);
        materials[1]->ks = vec3(4.0f, 2.0f, 3.0f);
        materials[1]->shininess = 20;

        terrain = new Terrain();
        spline = Spline(shaders[1], materials[1]);

        Object * tobj = new Object(shaders[0], materials[0], terrain);
        tobj->translation = vec3(-5, -5, 0);
        objects.push_back(tobj);

        camera = new Camera();

        camera->wEye = vec3(0, 0, 6);
        camera->wLookat = vec3(0, 0, 0);
        camera->wVup = vec3(0, 1, 0);

        lights.resize(2);
        lights[0].wLightPos = vec4(0, 0,  10, 0);
        lights[0].Le = vec3(0.3, 0.4, 0.5);
        lights[1].wLightPos = vec4(0, 0, 8, 0);
        lights[1].Le = vec3(0.5, 0.5, 0.5);
    }

    RenderState getState() {
        RenderState state;
        state.wEye = camera->wEye;
        state.V = camera->V();
        state.P = camera->P();
        state.lights = lights;

        return state;
    }

    void render() {
        RenderState state;
        state.wEye = camera->wEye;
        state.V = camera->V();
        state.P = camera->P();
        state.lights = lights;
        for (Object * obj : objects) obj->draw(state);
    }


};

Scene scene;

void onInitialization() {
    glViewport(0, 0, windowWidth, windowHeight);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    scene.build();
    scene.spline.Init();
}

void onDisplay() {
    glClearColor(	0, 0, 0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    scene.render();
    scene.spline.draw(scene.getState());
    glutSwapBuffers();
}

void onKeyboard(unsigned char key, int pX, int pY) {

}

void onKeyboardUp(unsigned char key, int pX, int pY) {
}

void onMouseMotion(int pX, int pY) {
}

void onMouse(int button, int state, int pX, int pY) {
    float cX = 2.0f * pX / windowWidth - 1;
    float cY = 1.0f - 2.0f * pY / windowHeight;

    switch (button) {
        case GLUT_LEFT_BUTTON:
            if (state == GLUT_DOWN)
                scene.spline.addCp({cX * 5, cY * 5});
            break;
    }

}

void onIdle() {
    glutPostRedisplay();
}
