#include "cpu.h"
#include "memory.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif
#include <string>

#define GAME_WIDTH 224
#define GAME_HEIGHT 256

#define CLOCK_SPEED 1996800
#define CYCLES_PER_FRAME = CLOCK_SPEED * 60; // ~33,333 cycles

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

int main(int argc, char *argv[]) {

  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
    printf("Error: SDL_Init(): %s\n", SDL_GetError());
    return 1;
  }

#if defined(IMGUI_IMPL_OPENGL_ES2)
  // GL ES 2.0 + GLSL 100 (WebGL 1.0)
  const char *glsl_version = "#version 100";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
  // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
  const char *glsl_version = "#version 300 es";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
  // GL 3.2 Core + GLSL 150
  const char *glsl_version = "#version 150";
  SDL_GL_SetAttribute(
      SDL_GL_CONTEXT_FLAGS,
      SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
  // GL 3.0 + GLSL 130
  const char *glsl_version = "#version 130";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
  SDL_WindowFlags window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
                                 SDL_WINDOW_HIDDEN |
                                 SDL_WINDOW_HIGH_PIXEL_DENSITY;
  SDL_Window *window = SDL_CreateWindow("Debugger", (int)(1280 * main_scale),
                                        (int)(720 * main_scale), window_flags);
  if (window == nullptr) {
    printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
    return 1;
  }
  SDL_GLContext gl_context = SDL_GL_CreateContext(window);
  if (gl_context == nullptr) {
    printf("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
    return 1;
  }

  SDL_GL_MakeCurrent(window, gl_context);
  SDL_GL_SetSwapInterval(1); // Enable vsync
  SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
  SDL_ShowWindow(window);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

  ImGui::StyleColorsDark();

  ImGuiStyle &style = ImGui::GetStyle();
  style.ScaleAllSizes(
      main_scale); // Bake a fixed style scale. (until we have a solution for
                   // dynamic style scaling, changing this requires resetting
                   // Style + calling this again)
  style.FontScaleDpi =
      main_scale; // Set initial font scale. (in docking branch: using
                  // io.ConfigDpiScaleFonts=true automatically overrides this
                  // for every window depending on the current monitor)

  // Setup Platform/Renderer backends
  ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
  ImGui_ImplOpenGL3_Init(glsl_version);

  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  struct i8080 state = i8080_init();

  state.inte_handle = 0xC7;

  mem_load_file("roms/invaders", 0);

  uint8_t video[GAME_HEIGHT][GAME_WIDTH][3] = {0};

  GLuint my_texture;
  glGenTextures(1, &my_texture);
  glBindTexture(GL_TEXTURE_2D, my_texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GAME_WIDTH, GAME_HEIGHT, 0, GL_RGB,
               GL_UNSIGNED_BYTE, video);
  glBindTexture(GL_TEXTURE_2D, 0);

  bool done = false;
  bool debug_reset = false;
  bool debug_run = false;
  bool debug_step = false;
  std::string goto_offset;
  uint16_t address{0};

  float emulation_speed = 1.0;

  u32 last_time = 0;

  bool mid_frame_done = false;
  while (!done) {
    float dt = ImGui::GetIO().DeltaTime;

    float cycle_accumulator = 0.0;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL3_ProcessEvent(&event);
      if (event.type == SDL_EVENT_QUIT)
        done = true;
      if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
          event.window.windowID == SDL_GetWindowID(window))
        done = true;
    }
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
      SDL_Delay(10);
      continue;
    }

    glBindTexture(GL_TEXTURE_2D, my_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GAME_WIDTH, GAME_HEIGHT, GL_RGB,
                    GL_UNSIGNED_BYTE, video);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (debug_run) {
      cycle_accumulator = dt * emulation_speed * 1000;

      int count = 0;
      while (count < cycle_accumulator * CLOCK_SPEED / 1000) {
        size_t cyc = state.cycle;
        i8080_execute(&state);
        size_t elapsed = state.cycle - cyc;
        count += elapsed;

        if (state.cycle >= (CLOCK_SPEED / 60) / 2) {
          state.cycle -= (CLOCK_SPEED / 60) / 2;

          i8080_interrupt(&state, state.inte_handle);

          if (state.inte_handle == 0xD7) {

            // delete later and try to come up with your own implementation
            for (int i = 0; i < 256 * 224 / 8; i++) {
              const int y = i * 8 / 256;
              const int base_x = (i * 8) % 256;
              const uint8_t cur_byte = mem[0x2400 + i];

              for (uint8_t bit = 0; bit < 8; bit++) {
                int px = base_x + bit;
                int py = y;
                const bool is_pixel_lit = (cur_byte >> bit) & 1;
                uint8_t r = 0, g = 0, b = 0;

                if (is_pixel_lit) {
                  r = 255;
                  g = 255;
                  b = 255;
                }

                const int temp_x = px;
                px = py;
                py = -temp_x + GAME_HEIGHT - 1;

                video[py][px][0] = r;
                video[py][px][1] = g;
                video[py][px][2] = b;
              }
            }
          }
          state.inte_handle = (state.inte_handle == 0xCF) ? 0xD7 : 0xCF;
        }
      }
    } else if (debug_step) {
      i8080_execute(&state);
      debug_step = false;
    }

    if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Tools")) {
        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Help")) {
        ImGui::EndMenu();
      }
      ImGui::EndMainMenuBar();
    }

    if (ImGui::Begin("Cpu", 0, ImGuiWindowFlags_NoCollapse)) {
      uint8_t f = 0;
      f |= state.Flag.s << 7;
      f |= state.Flag.z << 6;
      f |= state.Flag.ac << 4;
      f |= state.Flag.p << 2;
      f |= 1 << 1;
      f |= state.Flag.cy << 0;

      ImGui::Text("PC: $%04X", state.Register.pc);
      ImGui::Text("SP: $%04X", state.Register.sp);

      ImGui::Separator();

      ImGui::Text("Flags");

      bool flag_s = state.Flag.s;
      ImGui::Checkbox("S", &flag_s);
      ImGui::SameLine();

      bool flag_z = state.Flag.z;
      ImGui::Checkbox("Z", &flag_z);
      ImGui::SameLine();

      bool flag_a = state.Flag.ac == 1;
      ImGui::Checkbox("A", &flag_a);
      ImGui::SameLine();

      bool flag_p = state.Flag.p;
      ImGui::Checkbox("P", &flag_p);
      ImGui::SameLine();

      bool flag_c = state.Flag.cy;
      ImGui::Checkbox("C", &flag_c);

      ImGui::Separator();

      ImGui::Text("Registers");
      ImGui::Text("A: $%02X", state.Register.a);
      ImGui::Text("B: $%02X", state.Register.b);
      ImGui::Text("C: $%02X", state.Register.c);
      ImGui::Text("D: $%02X", state.Register.d);
      ImGui::Text("E: $%02X", state.Register.e);
      ImGui::Text("H: $%02X", state.Register.h);
      ImGui::Text("L: $%02X", state.Register.l);
      ImGui::Text("F: $%02X", f);

      ImGui::End();
    }

    if (ImGui::Begin("Dissassembly", 0,
                     ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoCollapse)) {

      ImGui::BeginChild("##simulation",
                        ImVec2(0.0, ImGui::GetFrameHeightWithSpacing()));
      if (ImGui::Button("Reset")) {
        state = i8080_init();
        debug_run = false;
      }

      ImGui::SameLine();

      if (ImGui::Button("Step")) {
        debug_step = true;
      }

      ImGui::SameLine();

      if (ImGui::Button("Pause")) {
        debug_run = false;
      }

      ImGui::SameLine();
      if (ImGui::Button("Run")) {
        debug_run = !debug_run;
      }

      ImGui::EndChild();

      ImGui::Separator();

      int column_count = 1;
      if (ImGui::BeginTable("instructions", 1,
                            ImGuiTableFlags_NoHostExtendX |
                                ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg,
                            ImVec2(0.0f, 0.0f))) {

        ImGui::TableSetupColumn("instruction_table",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoHide |
                                    ImGuiTableColumnFlags_NoReorder,
                                ImGui::GetWindowWidth());

        ImGuiListClipper clipper;
        clipper.Begin(ARRAY_SIZE(mem), ImGui::GetTextLineHeight());
        while (clipper.Step()) {
          for (int row = clipper.DisplayStart; row < clipper.DisplayEnd;
               row++) {
            uint16_t row_numbers = row * column_count;

            ImGui::SetScrollY((state.Register.pc * ImGui::GetTextLineHeight()) -
                              (ImGui::GetTextLineHeight() * 30));

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(128, 128, 128, 255));
            ImGui::Text("%04X", row);
            ImGui::SameLine();
            ImGui::PopStyleColor();
            ImGui::Text("%s", instruction_table[mem[row]]);

            if (state.Register.pc == row) {
              ImU32 cell_bg_color =
                  ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.65f));
              ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
            }
          }
        }
        clipper.End();
        ImGui::EndTable();
      }

      ImGui::End();
    }

    if (ImGui::Begin("Emulator", 0, ImGuiWindowFlags_NoCollapse)) {
      ImGui::AlignTextToFramePadding();
      ImGui::Text("Speed: ");
      ImGui::SameLine();
      ImGui::SliderFloat("##", &emulation_speed, 0.1, 1.0);
      ImGui::Text("Clock speed:  %d", CLOCK_SPEED);
      ImGui::Text("total cycles: %ld", state.cycle);
      ImGui::End();
    }

    if (ImGui::Begin("Screen", 0, ImGuiWindowFlags_NoCollapse)) {
      ImVec2 canvas_size = ImGui::GetContentRegionAvail();
      ImGui::Image((ImTextureID)(intptr_t)my_texture, canvas_size);
      ImGui::End();
    }

    // if (ImGui::Begin("Sound", 0, ImGuiWindowFlags_NoCollapse)) {
    //   ImGui::End();
    // }

    if (ImGui::Begin("Memory", 0, ImGuiWindowFlags_NoCollapse)) {

      bool tracking_is_enabled = true;
      int column_count = 16;

      if (ImGui::BeginTable(
              "Column_headers", 18,
              ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_ScrollY,
              ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 16))) {

        ImGui::TableSetupColumn("",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoHide |
                                    ImGuiTableColumnFlags_NoReorder,
                                40.0f);
        for (int header_num = 0; header_num < column_count; header_num++) {
          char buffer[3];
          snprintf(buffer, 3, "%02X", header_num);
          ImGui::TableSetupColumn(buffer, ImGuiTableColumnFlags_WidthFixed);
        }
        ImGui::TableSetupScrollFreeze(0, 1);

        ImGui::TableSetupColumn("ascii_header",
                                ImGuiTableColumnFlags_WidthStretch |
                                    ImGuiTableColumnFlags_NoHide |
                                    ImGuiTableColumnFlags_NoReorder |
                                    ImGuiTableColumnFlags_NoHeaderLabel |
                                    ImGuiTableColumnFlags_NoHeaderWidth);

        ImGui::TableHeadersRow();

        ImGuiListClipper clipper;
        clipper.Begin((ARRAY_SIZE(mem) + column_count - 1) / column_count,
                      ImGui::GetTextLineHeight());

        while (clipper.Step()) {
          for (int row = clipper.DisplayStart; row < clipper.DisplayEnd;
               row++) {
            uint16_t row_numbers = row * column_count;

            // enables autoscroll
            if (goto_offset.size() != 0) {
              address = std::stoi(goto_offset, 0, 16);
            }
            if (io.WantTextInput) {
              ImGui::SetScrollY(address * ImGui::GetTextLineHeight());
            }

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%04X:", row_numbers);

            for (int col = 0; col < column_count; col++) {
              if (ImGui::TableSetColumnIndex(col + 1)) {
                if (mem[row * column_count + col] == 0) {
                  ImGui::PushStyleColor(ImGuiCol_Text,
                                        IM_COL32(128, 128, 128, 255));
                  ImGui::Text("%02X", mem[row * column_count + col]);
                  ImGui::PopStyleColor();
                } else {
                  ImGui::Text("%02X", mem[row * column_count + col]);
                }
              }

              ImGui::TableSetColumnIndex(17);
              ImGui::TextUnformatted("|");
              ImGui::SameLine();
              for (int col = 0; col < column_count; col++) {
                ImGui::Text("%c", mem[row * column_count + col] >= 32 &&
                                          mem[row * column_count + col] <= 126
                                      ? mem[row * column_count + col]
                                      : '.');
                ImGui::SameLine();
              }
              ImGui::TextUnformatted("|");

              row_numbers++;
            }
          }
        }
        clipper.End();
        ImGui::EndTable();
      }

      ImGui::Separator();

      ImGui::BeginChild("memory_selector", ImVec2(0, 0));
      ImGui::AlignTextToFramePadding();
      ImGui::Button("Options");
      ImGui::SameLine();
      ImGui::Text("Range %04X..%04lX ", 0000, ARRAY_SIZE(mem));
      ImGui::SameLine();
      ImGui::PushItemWidth(40);
      // ImGui::InputText("##goto", &goto_offset,
      //                  ImGuiInputTextFlags_CharsHexadecimal |
      //                      ImGuiInputTextFlags_CharsUppercase);
      ImGui::PopItemWidth();
      ImGui::EndChild();

      ImGui::End();
    }

    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                 clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window);
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
  glDeleteTextures(1, &my_texture);

  SDL_GL_DestroyContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
