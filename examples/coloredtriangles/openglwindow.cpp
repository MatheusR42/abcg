#include "openglwindow.hpp"

#include <imgui.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "abcg.hpp"

void OpenGLWindow::initializeGL() {
  const auto *vertexShader{R"gl(
    #version 410

    layout(location = 0) in vec2 inPosition;
    layout(location = 1) in vec4 inColor;

    out vec4 fragColor;

    void main() {
      gl_Position = vec4(inPosition, 0, 1);
      fragColor = inColor;
    }
  )gl"};

  const auto *fragmentShader{R"gl(
    #version 410

    in vec4 fragColor;

    out vec4 outColor;

    void main() { outColor = fragColor; }
  )gl"};

  // Create shader program
  m_program = createProgramFromString(vertexShader, fragmentShader);

  // Clear window
  glClearColor(0, 0, 0, 1);
  glClear(GL_COLOR_BUFFER_BIT);

  // Start pseudo-random number generator
  auto seed{std::chrono::steady_clock::now().time_since_epoch().count()};
  m_randomEngine.seed(seed);
}

void OpenGLWindow::paintGL() {
  setupModel();

  abcg::glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  abcg::glUseProgram(m_program);
  abcg::glBindVertexArray(m_vao);

  abcg::glDrawArrays(GL_TRIANGLES, 0, 3);

  abcg::glBindVertexArray(0);
  abcg::glUseProgram(0);
}

void OpenGLWindow::paintUI() {
  abcg::OpenGLWindow::paintUI();

  {
    auto widgetSize{ImVec2(250, 90)};
    ImGui::SetNextWindowPos(ImVec2(m_viewportWidth - widgetSize.x - 5,
                                   m_viewportHeight - widgetSize.y - 5));
    ImGui::SetNextWindowSize(widgetSize);
    auto windowFlags{ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar};
    ImGui::Begin(" ", nullptr, windowFlags);

    // Edit vertex colors
    auto colorEditFlags{ImGuiColorEditFlags_NoTooltip |
                        ImGuiColorEditFlags_NoPicker};
    ImGui::PushItemWidth(215);
    ImGui::ColorEdit3("v0", &m_vertexColors[0].x, colorEditFlags);
    ImGui::ColorEdit3("v1", &m_vertexColors[1].x, colorEditFlags);
    ImGui::ColorEdit3("v2", &m_vertexColors[2].x, colorEditFlags);
    ImGui::PopItemWidth();

    ImGui::End();
  }
}

void OpenGLWindow::resizeGL(int width, int height) {
  m_viewportWidth = width;
  m_viewportHeight = height;

  abcg::glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGLWindow::terminateGL() {
  abcg::glDeleteProgram(m_program);
  abcg::glDeleteBuffers(1, &m_vboPositions);
  abcg::glDeleteBuffers(1, &m_vboColors);
  abcg::glDeleteVertexArrays(1, &m_vao);
}

void OpenGLWindow::setupModel() {
  abcg::glDeleteBuffers(1, &m_vboPositions);
  abcg::glDeleteBuffers(1, &m_vboColors);
  abcg::glDeleteVertexArrays(1, &m_vao);

  // Create vertex positions
  std::uniform_real_distribution<float> rd(-1.5f, 1.5f);
  std::array positions{glm::vec2(rd(m_randomEngine), rd(m_randomEngine)),
                       glm::vec2(rd(m_randomEngine), rd(m_randomEngine)),
                       glm::vec2(rd(m_randomEngine), rd(m_randomEngine))};

  // Create vertex colors
  std::vector<glm::vec4> colors(0);
  colors.emplace_back(m_vertexColors[0]);
  colors.emplace_back(m_vertexColors[1]);
  colors.emplace_back(m_vertexColors[2]);

  // Generate VBO of positions
  abcg::glGenBuffers(1, &m_vboPositions);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_vboPositions);
  abcg::glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions.data(),
                     GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Generate VBO of colors
  abcg::glGenBuffers(1, &m_vboColors);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_vboColors);
  abcg::glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec4),
                     colors.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Get location of attributes in the program
  GLint positionAttribute{abcg::glGetAttribLocation(m_program, "inPosition")};
  GLint colorAttribute{abcg::glGetAttribLocation(m_program, "inColor")};

  // Create VAO
  abcg::glGenVertexArrays(1, &m_vao);

  // Bind vertex attributes to current VAO
  abcg::glBindVertexArray(m_vao);

  abcg::glEnableVertexAttribArray(positionAttribute);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_vboPositions);
  abcg::glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0,
                              nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  abcg::glEnableVertexAttribArray(colorAttribute);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_vboColors);
  abcg::glVertexAttribPointer(colorAttribute, 4, GL_FLOAT, GL_FALSE, 0,
                              nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // End of binding to current VAO
  abcg::glBindVertexArray(0);
}