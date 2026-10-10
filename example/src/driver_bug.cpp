#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cstring>

#define MODE 1

static const char* VS = R"(#version 450 core
layout(location = 0) in mat4 model;     // occupies locations 0..3
layout(location = 4) in vec4 color;
layout(location = 0) uniform mat4 projection;
out vec4 frag_color;
const vec2 corners[6] = vec2[6](
    vec2(0,0), vec2(1,0), vec2(1,1),
    vec2(0,0), vec2(1,1), vec2(0,1));
void main() {
    gl_Position = projection * model * vec4(corners[gl_VertexID], 0.0, 1.0);
    frag_color = color;
})";

static const char* FS = R"(#version 450 core
in vec4 frag_color;
out vec4 out_color;
void main() { out_color = frag_color; })";

static GLuint make_shader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) { char log[1024]; glGetShaderInfoLog(s, 1024, nullptr, log); printf("%s\n", log); }
    return s;
}

static GLuint make_vao(GLuint buf, GLsizei stride) {
    GLuint vao;
    glCreateVertexArrays(1, &vao);
    glVertexArrayVertexBuffer(vao, 0, buf, 0, stride);
    glVertexArrayBindingDivisor(vao, 0, 1);
    for (int i = 0; i < 4; i++) {
        glEnableVertexArrayAttrib(vao, i);
        glVertexArrayAttribFormat(vao, i, 4, GL_FLOAT, GL_FALSE, 16 * i);
        glVertexArrayAttribBinding(vao, i, 0);
    }
    glEnableVertexArrayAttrib(vao, 4);
    glVertexArrayAttribFormat(vao, 4, 4, GL_FLOAT, GL_FALSE, 64);
    glVertexArrayAttribBinding(vao, 4, 0);
    return vao;
}

// column-major 4x4 for: translate(x, y, 0) * scale(w, h, 1)
static void rect_matrix(float* m, float x, float y, float w, float h) {
    memset(m, 0, 16 * sizeof(float));
    m[0] = w; m[5] = h; m[10] = 1; m[12] = x; m[13] = y; m[15] = 1;
}

static void probe(const char* tag, int fb_h, int x, int y) {
    unsigned char px[4] = {};
    glReadPixels(x, fb_h - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, px);
    printf("%s: %d,%d,%d\n", tag, px[0], px[1], px[2]);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);   // 4.5 so llvmpipe works too
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* win = glfwCreateWindow(800, 600, "repro", nullptr, nullptr);
    if (!win) { printf("window creation failed\n"); return 1; }
    glfwMakeContextCurrent(win);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    printf("Renderer: %s\nVersion: %s\n", glGetString(GL_RENDERER), glGetString(GL_VERSION));
    printf("Mode : %d\n", MODE);

    int fb_w, fb_h;
    glfwGetFramebufferSize(win, &fb_w, &fb_h);

    // program
    GLuint prog = glCreateProgram();
    GLuint vs = make_shader(GL_VERTEX_SHADER, VS);
    GLuint fs = make_shader(GL_FRAGMENT_SHADER, FS);
    glAttachShader(prog, vs); glAttachShader(prog, fs);
    glLinkProgram(prog);

    // ortho(0, w, h, 0, -1, 1), column-major
    float proj[16] = {};
    proj[0] = 2.0f / fb_w; proj[5] = -2.0f / fb_h; proj[10] = -1.0f;
    proj[12] = -1.0f; proj[13] = 1.0f; proj[15] = 1.0f;

    const GLsizei STRIDE = 20 * sizeof(float);
    GLuint buf, buf2 = 0;
    glCreateBuffers(1, &buf);
    glNamedBufferStorage(buf, (MODE == 0 || MODE == 3) ? STRIDE : 2 * STRIDE,
                        nullptr, GL_DYNAMIC_STORAGE_BIT);
    GLuint vao = make_vao(buf, STRIDE);

    GLuint vao2 = 0;
    #if MODE == 3
    glCreateBuffers(1, &buf2);
    glNamedBufferStorage(buf2, STRIDE, nullptr, GL_DYNAMIC_STORAGE_BIT);
    vao2 = make_vao(buf2, STRIDE);
    #endif

    float red[20], green[20];
    rect_matrix(red, 50, 50, 50, 50);    memcpy(red + 16,   (float[]){1,0,0,1}, 16);
    rect_matrix(green, 150, 50, 50, 50); memcpy(green + 16, (float[]){0,1,0,1}, 16);

    for (int frame = 0; frame < 6; frame++) {
        printf("--- frame %d ---\n", frame);
        glViewport(0, 0, fb_w, fb_h);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);        // clear once per frame, before both draws

        glUseProgram(prog);
        glUniformMatrix4fv(0, 1, GL_FALSE, proj);
        glBindVertexArray(vao);

        // vary the red rect's brightness each frame so stale data is detectable
        float c = (frame % 2) ? 0.5f : 1.0f;
        float rc[4] = {c, 0, 0, 1};
        memcpy(red + 16, rc, 16);
        printf("expect red channel: %d\n", (int)(c * 255 + 0.5f));

#if MODE == 0
        // red
        glNamedBufferSubData(buf, 0, STRIDE, red);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);
        probe("after red,   red spot  ", fb_h, 75, 75);
        probe("after red,   green spot", fb_h, 175, 75);

        // green: overwrite the same region
        glNamedBufferSubData(buf, 0, STRIDE, green);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);
        probe("after green, red spot  ", fb_h, 75, 75);
        probe("after green, green spot", fb_h, 175, 75);
#elif MODE == 1
        float both[40];
        memcpy(both, red, STRIDE); memcpy(both + 20, green, STRIDE);
        glNamedBufferSubData(buf, 0, 2 * STRIDE, both);

        glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, 1, 0);
        probe("after red,   red spot  ", fb_h, 75, 75);
        probe("after red,   green spot", fb_h, 175, 75);

        glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, 1, 1);
        probe("after green, red spot  ", fb_h, 75, 75);
        probe("after green, green spot", fb_h, 175, 75);
#elif MODE == 2
        // one buffer, disjoint regions, upload interleaved with draws
        glNamedBufferSubData(buf, 0, STRIDE, red);
        glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, 1, 0);
        probe("after red,   red spot  ", fb_h, 75, 75);
        probe("after red,   green spot", fb_h, 175, 75);

        glNamedBufferSubData(buf, STRIDE, STRIDE, green);   // different region
        glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, 1, 1);
        probe("after green, red spot  ", fb_h, 75, 75);
        probe("after green, green spot", fb_h, 175, 75);
#elif MODE == 3
        // two buffers, two VAOs, upload interleaved with draws
        glNamedBufferSubData(buf, 0, STRIDE, red);
        glBindVertexArray(vao);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);
        probe("after red,   red spot  ", fb_h, 75, 75);
        probe("after red,   green spot", fb_h, 175, 75);

        glNamedBufferSubData(buf2, 0, STRIDE, green);        // different buffer
        glBindVertexArray(vao2);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);
        probe("after green, red spot  ", fb_h, 75, 75);
        probe("after green, green spot", fb_h, 175, 75);
#endif
        GLenum err = glGetError();
        if (err) printf("GL error 0x%x\n", err);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }
    glfwTerminate();
}