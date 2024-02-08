#include "SSAO.h"
#include <random>

inline float lerp(float a, float b, float f) {
    return a + f * (b - a);
}

SSAO::SSAO() {
    ssao_program.Init({
        { GL_VERTEX_SHADER,		"Shaders/SSAO.vert" },
        { GL_FRAGMENT_SHADER,	"Shaders/SSAO.frag" }
    });

    std::uniform_real_distribution<float> random_m1_p1(-1.0, 1.0);
    std::uniform_real_distribution<float> random_p0_p1(0.0, 1.0);
    std::default_random_engine generator;
    std::vector<glm::vec3> ssaoKernel;
    for (unsigned int i = 0; i < 64; ++i)
    {
        glm::vec3 sample(
            random_m1_p1(generator),
            random_m1_p1(generator),
            random_p0_p1(generator)
        );
        sample = glm::normalize(sample);
        sample *= random_p0_p1(generator);

        float scale = (float)i / 64.0;
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }

    ssao_program.Use();
    glUniform3fv(glGetUniformLocation(ssao_program, "samples"), 64, (const GLfloat*)ssaoKernel.data());
    ssao_program.Unuse();

    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++)
    {
        glm::vec3 noise;
        while (true) {
            noise = glm::vec3(
                random_m1_p1(generator),
                random_m1_p1(generator),
                0.0f);
            if (glm::length(noise) != 0)break;
        }
        ssaoNoise.push_back(noise);
    }

    glGenTextures(1, &noise_texture);
    glBindTexture(GL_TEXTURE_2D, noise_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

SSAO::~SSAO() {
    if (frameBuffersCreated) {
        glDeleteTextures(1, &ssaoTexture);
        glDeleteFramebuffers(1, &ssaoFrameBuffer);
    }
    glDeleteTextures(1, &noise_texture);
}

void SSAO::renderSSAO(const GLuint normalBuffer, const GLuint depthBuffer, const gCamera& camera) {
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFrameBuffer);
    ssao_program.Use();
    ssao_program.SetTexture("normalTexture", 0, normalBuffer);
    ssao_program.SetTexture("depthTexture", 1, depthBuffer);
    ssao_program.SetTexture("noiseTexture", 2, noise_texture);
    ssao_program.SetUniform("P", camera.GetProj());
    ssao_program.SetUniform("PI", glm::inverse(camera.GetProj()));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    ssao_program.Unuse();
}

void SSAO::createFrameBuffer(int width, int height) {
    if (frameBuffersCreated) {
        glDeleteTextures(1, &ssaoTexture);
        glDeleteFramebuffers(1, &ssaoFrameBuffer);
    }

    glGenFramebuffers(1, &ssaoFrameBuffer);
    glGenTextures(1, &ssaoTexture);

    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFrameBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoTexture, 0);
    if (glGetError() != GL_NO_ERROR) {
        std::cout << "Error creating color attachment 0" << std::endl;
        exit(1);
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Incomplete framebuffer (";
        switch (status) {
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:			std::cout << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";		 break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:	std::cout << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"; break;
        case GL_FRAMEBUFFER_UNSUPPORTED:					std::cout << "GL_FRAMEBUFFER_UNSUPPORTED";					 break;
        }
        std::cout << ")" << std::endl;
        exit(1);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    frameBuffersCreated = true;

    ssao_program.Use();
    ssao_program.SetUniform("noiseScale", glm::vec2(width / 4, height / 4));
    ssao_program.Unuse();
}

GLuint SSAO::getSSAO() {
    return ssaoTexture;
}