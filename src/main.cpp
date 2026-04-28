#include "cpu.h"
#include "memory.h"
#include "constants.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"
#include <SDL3/SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif
#include <iostream>
#include <string>
#include <array>
#include <unordered_map>
#include <vector>

#define GAME_WIDTH 224
#define GAME_HEIGHT 256

#define CLOCK_SPEED 1996800
#define CYCLES_PER_FRAME (CLOCK_SPEED * 60) // ~33,333 cycles

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

// TODO if we are compiling in debug mode then we should open the debugger
// otherwise just the screen showing the game if argv.contains("--debug" OR
// "-d") then Debugger.debug(emulator) else

struct Breakpoint
{
public:
  Breakpoint() = default;
  explicit Breakpoint(bool state) : m_is_enabled{state} {}

  void toggle() { m_is_enabled = !m_is_enabled; }
  bool isEnabled() const { return m_is_enabled; }

  void render();

private:
  bool m_is_enabled{false};
};

class Instruction
{
public:
  Instruction() : m_mnemonic{"Unknown"} {}
  Instruction(const std::string& mnemonic) : m_mnemonic{ mnemonic } {}

  const std::string& mnemonic() const { return m_mnemonic; }
private:
  std::string m_mnemonic;
};

class Debugger
{
public:
  Debugger()
  {
    for (auto opcode : OPCODE_TABLE)
      m_instructions.push_back(Instruction{opcode});
  };

  std::unordered_map<u16, Breakpoint> m_breakpoints;
  std::vector<Instruction> m_instructions;

  enum class State
  {
    Reset,
    Pause,
    Running,
    Step,
  };

  State& state() { return m_debug_state; }
  const State& state() const { return m_debug_state; }

private:
  const std::vector<std::string> OPCODE_TABLE{
      "nop",     "lxi b,#",  "stax b",  "inx b",   "inr b",   "dcr b",
      "mvi b,#", "rlc",      "ill",     "dad b",   "ldax b",  "dcx b",
      "inr c",   "dcr c",    "mvi c,#", "rrc",     "ill",     "lxi d,#",
      "stax d",  "inx d",    "inr d",   "dcr d",   "mvi d,#", "ral",
      "ill",     "dad d",    "ldax d",  "dcx d",   "inr e",   "dcr e",
      "mvi e,#", "rar",      "ill",     "lxi h,#", "shld",    "inx h",
      "inr h",   "dcr h",    "mvi h,#", "daa",     "ill",     "dad h",
      "lhld",    "dcx h",    "inr l",   "dcr l",   "mvi l,#", "cma",
      "ill",     "lxi sp,#", "sta $",   "inx sp",  "inr M",   "dcr M",
      "mvi M,#", "stc",      "ill",     "dad sp",  "lda $",   "dcx sp",
      "inr a",   "dcr a",    "mvi a,#", "cmc",     "mov b,b", "mov b,c",
      "mov b,d", "mov b,e",  "mov b,h", "mov b,l", "mov b,M", "mov b,a",
      "mov c,b", "mov c,c",  "mov c,d", "mov c,e", "mov c,h", "mov c,l",
      "mov c,M", "mov c,a",  "mov d,b", "mov d,c", "mov d,d", "mov d,e",
      "mov d,h", "mov d,l",  "mov d,M", "mov d,a", "mov e,b", "mov e,c",
      "mov e,d", "mov e,e",  "mov e,h", "mov e,l", "mov e,M", "mov e,a",
      "mov h,b", "mov h,c",  "mov h,d", "mov h,e", "mov h,h", "mov h,l",
      "mov h,M", "mov h,a",  "mov l,b", "mov l,c", "mov l,d", "mov l,e",
      "mov l,h", "mov l,l",  "mov l,M", "mov l,a", "mov M,b", "mov M,c",
      "mov M,d", "mov M,e",  "mov M,h", "mov M,l", "hlt",     "mov M,a",
      "mov a,b", "mov a,c",  "mov a,d", "mov a,e", "mov a,h", "mov a,l",
      "mov a,M", "mov a,a",  "add b",   "add c",   "add d",   "add e",
      "add h",   "add l",    "add M",   "add a",   "adc b",   "adc c",
      "adc d",   "adc e",    "adc h",   "adc l",   "adc M",   "adc a",
      "sub b",   "sub c",    "sub d",   "sub e",   "sub h",   "sub l",
      "sub M",   "sub a",    "sbb b",   "sbb c",   "sbb d",   "sbb e",
      "sbb h",   "sbb l",    "sbb M",   "sbb a",   "ana b",   "ana c",
      "ana d",   "ana e",    "ana h",   "ana l",   "ana M",   "ana a",
      "xra b",   "xra c",    "xra d",   "xra e",   "xra h",   "xra l",
      "xra M",   "xra a",    "ora b",   "ora c",   "ora d",   "ora e",
      "ora h",   "ora l",    "ora M",   "ora a",   "cmp b",   "cmp c",
      "cmp d",   "cmp e",    "cmp h",   "cmp l",   "cmp M",   "cmp a",
      "rnz",     "pop b",    "jnz $",   "jmp $",   "cnz $",   "push b",
      "adi #",   "rst 0",    "rz",      "ret",     "jz $",    "ill",
      "cz $",    "call $",   "aci #",   "rst 1",   "rnc",     "pop d",
      "jnc $",   "out p",    "cnc $",   "push d",  "sui #",   "rst 2",
      "rc",      "ill",      "jc $",    "in p",    "cc $",    "ill",
      "sbi #",   "rst 3",    "rpo",     "pop h",   "jpo $",   "xthl",
      "cpo $",   "push h",   "ani #",   "rst 4",   "rpe",     "pchl",
      "jpe $",   "xchg",     "cpe $",   "ill",     "xri #",   "rst 5",
      "rp",      "pop psw",  "jp $",    "di",      "cp $",    "push psw",
      "ori #",   "rst 6",    "rm",      "sphl",    "jm $",    "ei",
      "cm $",    "ill",      "cpi #",   "rst 7"};


  State m_debug_state{State::Pause};
};

class Display
{
public:
  Display() = default;
  ~Display() { glDeleteTextures(1, &m_texture); }

  GLuint m_texture;

  void create_texture()
  {
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GAME_WIDTH, GAME_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  void update()
  {
    // delete later and try to come up with your own implementation
    for (int i = 0; i < 256 * 224 / 8; i++)
    {
      const int y = i * 8 / 256;
      const int base_x = (i * 8) % 256;
      const uint8_t cur_byte = mem_read_byte(VRAM_ADDRESS + i);

      for (uint8_t bit = 0; bit < 8; bit++)
      {
        int px = base_x + bit;
        int py = y;
        const bool is_pixel_lit = (cur_byte >> bit) & 1;
        uint8_t r = 0, g = 0, b = 0;

        if (is_pixel_lit)
        {
          r = 255;
          g = 255;
          b = 255;
        }

        const int temp_x = px;
        px = py;
        py = -temp_x + GAME_HEIGHT - 1;

        pixels[py][px][0] = r;
        pixels[py][px][1] = g;
        pixels[py][px][2] = b;
      }
    }
  }

  void draw()
  {
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GAME_WIDTH, GAME_HEIGHT, GL_RGB,
                    GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

private:
  //uint8_t pixels[GAME_HEIGHT][GAME_WIDTH][3] = {0};
  uint8_t pixels[GAME_HEIGHT][GAME_WIDTH][3] = {0};
};

namespace UI {
  void toolbar() {
    
  }
}

class Emulator
{
public:
  Emulator() = default;

  ~Emulator()
  {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DestroyContext(m_gl_context);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
  }

  int init()
  {
    state = i8080_init();
    state.inte_handle = 0xC7;

    mem_load_file("roms/8080EXM.COM", 0x0100);
    std::copy(std::begin(mem), std::end(mem), std::begin(m_rom));

    // if compiled with -TROM then it should populate with these
    state.Register.pc = 0x0100;

    mem_write_byte(0x0000, 0xD3);
    mem_write_byte(0x0001, 0x00);

    mem_write_byte(0x0005, 0xD3);
    mem_write_byte(0x0006, 0x01);
    mem_write_byte(0x0007, 0xC9);
    //

    m_emulation_speed = 1.0;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
	std::cout << "Error: SDL_Init(): " << SDL_GetError() << '\n';
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
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
			SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    #else
    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
			SDL_GL_CONTEXT_PROFILE_CORE);
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

    m_window = SDL_CreateWindow("Debugger", (int)(1280 * main_scale),
				(int)(720 * main_scale), window_flags);
    if (m_window == nullptr)
    {
	std::cout << "Error: SDL_CreateWindow(): " << SDL_GetError() << '\n';
	return 1;
    }

    m_gl_context = SDL_GL_CreateContext(m_window);
    if (m_gl_context == nullptr)
    {
	std::cout << "Error: SDL_GL_CreateContext(): " << SDL_GetError() << '\n';
	return 1;
    }

    SDL_GL_MakeCurrent(m_window, m_gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync
    SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED,
			    SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(m_window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    m_io = &ImGui::GetIO();
    (void)m_io;
    ImGuiIO& io = *m_io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    ImGui_ImplSDL3_InitForOpenGL(m_window, m_gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);
    display.create_texture();

    return 0;
  }

  void handle_input()
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      ImGui_ImplSDL3_ProcessEvent(&event);
      if (event.type == SDL_EVENT_QUIT)
        m_is_running = true;
      if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
          event.window.windowID == SDL_GetWindowID(m_window))
        m_is_running = true;
    }
  }

  void run()
  {
    while (!m_is_running)
    {
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplSDL3_NewFrame();
      ImGui::NewFrame();
      if (SDL_GetWindowFlags(m_window) & SDL_WINDOW_MINIMIZED)
      {
        SDL_Delay(10);
        continue;
      }

      handle_input();

    float dt = ImGui::GetIO().DeltaTime;

    float cycle_accumulator = 0.0;

    if (state.Register.pc == 0x0005)
    {
      if (state.Register.c == 2)
      {
        printf("%c", state.Register.e);
      }
      else if (state.Register.c == 9)
      {
        u16 i = state.Register.de;
        u8 byte;
        while ((byte = mem_read_byte(i)) != '$')
        {
          printf("%c", byte);
          i++;
        }
      }
    }

    switch (m_debugger.state())
    {
    case Debugger::State::Pause: break;
    case Debugger::State::Reset: state = i8080_init(); break;
    case Debugger::State::Step:
      i8080_execute(&state);
      m_debugger.state() = Debugger::State::Pause; 
      break;
    case Debugger::State::Running: {
      cycle_accumulator = dt * m_emulation_speed * 1000;
      int count = 0;
      while (count < cycle_accumulator * CLOCK_SPEED / 1000)
      {
        size_t cyc = state.cycle;
        i8080_execute(&state);
        size_t elapsed = state.cycle - cyc;
        count += elapsed;

        if (state.cycle >= (CLOCK_SPEED / 60) / 2)
        {
          state.cycle -= (CLOCK_SPEED / 60) / 2;

          i8080_interrupt(&state, state.inte_handle);

          if (state.inte_handle == 0xD7)
          {
	    display.update();
          }
          state.inte_handle = (state.inte_handle == 0xCF) ? 0xD7 : 0xCF;
        }
      }
      break;
      }
    }

    if (ImGui::BeginMainMenuBar())
    {
      if (ImGui::BeginMenu("File"))
      {
        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Tools"))
      {
        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Help"))
      {
        ImGui::EndMenu();
      }
      ImGui::EndMainMenuBar();
    }

    if (ImGui::Begin("Cpu", 0, ImGuiWindowFlags_NoCollapse))
    {
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
                         ImGuiWindowFlags_NoCollapse))
    {

      ImGui::BeginChild("##simulation",
                        ImVec2(0.0, ImGui::GetFrameHeightWithSpacing()));

      if (ImGui::Button("Reset")) m_debugger.state() = Debugger::State::Reset;
      ImGui::SameLine();

      if (ImGui::Button("Step")) m_debugger.state() = Debugger::State::Step;
      ImGui::SameLine();

      if (ImGui::Button("Pause")) m_debugger.state() = Debugger::State::Pause;
      ImGui::SameLine();

      if (ImGui::Button("Run")) m_debugger.state() = Debugger::State::Running;
      ImGui::EndChild();

      ImGui::Separator();

      int column_count = 1;
      if (ImGui::BeginTable("instructions", 2,
                            ImGuiTableFlags_NoHostExtendX |
                                ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg,
                            ImVec2(0.0f, 0.0f)))
      {
        ImGui::TableSetupColumn("breakpoints",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoHide |
                                    ImGuiTableColumnFlags_NoReorder,
                                20.0f);

        ImGui::TableSetupColumn("instruction_table",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoHide |
                                    ImGuiTableColumnFlags_NoReorder,
                                ImGui::GetWindowWidth());

        ImDrawList *draw_list = ImGui::GetWindowDrawList();

        ImGuiListClipper clipper;
        clipper.Begin(ARRAY_SIZE(mem), ImGui::GetTextLineHeight());
        while (clipper.Step())
        {
          for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
          {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);

	    // how I draw the breakpoint
            float radius = 4.0f;
            ImU32 color = IM_COL32(255, 50, 50, 255);
            ImVec2 current_pos = ImGui::GetCursorScreenPos();
            float mid_y = current_pos.y + (ImGui::GetTextLineHeight() / 2.0f);
            float circle_x = current_pos.x + 10.0f;

            ImGui::PushID(row);
            ImGui::InvisibleButton(
                "##bp", ImVec2(-FLT_MIN, ImGui::GetTextLineHeight()));

            if (ImGui::IsItemClicked())
	    {
		auto breakpoint = m_debugger.m_breakpoints.find(row);
		if (breakpoint == m_debugger.m_breakpoints.end())
		  m_debugger.m_breakpoints[row] = Breakpoint{};
		else 
		  m_debugger.m_breakpoints[row].toggle();
	    }

            if (ImGui::IsItemHovered())
              draw_list->AddCircleFilled(ImVec2(circle_x, mid_y), radius,
                                         IM_COL32(255, 50, 50, 50));

	    if (m_debugger.m_breakpoints[row].isEnabled())
              draw_list->AddCircleFilled(ImVec2(circle_x, mid_y), radius, color);

            ImGui::PopID();
            ImGui::TableSetColumnIndex(1);

            // ImGui::SetScrollY(
            //     (state.Register.pc * ImGui::GetTextLineHeight()) -
            //     (ImGui::GetTextLineHeight() * 30));

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(128, 128, 128, 255));
            ImGui::Text("%04X", row);
            ImGui::SameLine();
            ImGui::PopStyleColor();
	    // TODO m_rom[row] will scan every opcode and data
            ImGui::Text("%s", m_debugger.m_instructions[m_rom[row]].mnemonic().c_str());

            if (m_debugger.m_breakpoints[state.Register.pc].isEnabled())
	    {
	      m_debugger.state() = Debugger::State::Pause;
	    }

            if (state.Register.pc == row)
            {
              ImU32 cell_bg_color =
                  ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.65f));
              ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, cell_bg_color);
            }

          }
        }
        clipper.End();
        ImGui::EndTable();
      }

      ImGui::End();
    }

    std::cout << static_cast<int>(m_debugger.state()) << '\n';

    if (ImGui::Begin("Emulator", 0, ImGuiWindowFlags_NoCollapse))
    {
      ImGui::AlignTextToFramePadding();
      ImGui::Text("Speed: ");
      ImGui::SameLine();
      ImGui::SliderFloat("##", &m_emulation_speed, 0.1, 1.0);
      ImGui::Text("Clock speed:  %d", CLOCK_SPEED);
      ImGui::Text("total cycles: %ld", state.cycle);
      ImGui::End();
    }

    if (ImGui::Begin("Screen", 0, ImGuiWindowFlags_NoCollapse))
    {
      ImVec2 canvas_size = ImGui::GetContentRegionAvail();
      ImGui::Image((ImTextureID)(intptr_t)display.m_texture, canvas_size);
      ImGui::End();
    }

    if (ImGui::Begin("Memory", 0, ImGuiWindowFlags_NoCollapse))
    {
      int column_count = 16;

      if (ImGui::BeginTable(
              "Column_headers", 18,
              ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_ScrollY,
              ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 16)))
      {

        ImGui::TableSetupColumn("",
                                ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoHide |
                                    ImGuiTableColumnFlags_NoReorder,
                                40.0f);
        for (int header_num = 0; header_num < column_count; header_num++)
        {
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

        while (clipper.Step())
        {
          for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
          {
            uint16_t row_numbers = row * column_count;

            // enables autoscroll
            if (m_goto_offset.size() != 0)
            {
              m_address = std::stoi(m_goto_offset, 0, 16);
            }
            if (m_io->WantTextInput)
            {
              ImGui::SetScrollY(m_address * ImGui::GetTextLineHeight());
            }

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%04X:", row_numbers);

            for (int col = 0; col < column_count; col++)
            {
              if (ImGui::TableSetColumnIndex(col + 1))
              {
                if (mem[row * column_count + col] == 0)
                {
                  ImGui::PushStyleColor(ImGuiCol_Text,
                                        IM_COL32(128, 128, 128, 255));
                  ImGui::Text("%02X", mem[row * column_count + col]);
                  ImGui::PopStyleColor();
                }
                else
                {
                  ImGui::Text("%02X", mem[row * column_count + col]);
                }
              }

              ImGui::TableSetColumnIndex(17);
              ImGui::TextUnformatted("|");
              ImGui::SameLine();
              for (int col = 0; col < column_count; col++)
              {
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

    ImGuiIO& io = *m_io;
      ImGui::Render();
      glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
      glClearColor(m_clear_color.x * m_clear_color.w,
                   m_clear_color.y * m_clear_color.w,
                   m_clear_color.z * m_clear_color.w, m_clear_color.w);
      glClear(GL_COLOR_BUFFER_BIT);
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
      SDL_GL_SwapWindow(m_window);
    }
  }

private:
  SDL_Window *m_window;
  SDL_GLContext m_gl_context;
  ImGuiIO* m_io = nullptr;
  Display display;
  struct i8080 state;
  Debugger m_debugger;
  float m_emulation_speed;
  ImVec4 m_clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  std::string m_goto_offset;
  uint16_t m_address{0};
  std::array<u8, 0xFFFF> m_rom;

  bool m_is_running{false};
};

int main(int argc, char *argv[])
{
  Emulator emulator;
  emulator.init();
  std::cout << "test" << '\n';
  emulator.run();
  return 0;
}
