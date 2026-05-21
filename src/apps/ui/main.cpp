// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

static void glfw_error_callback(int error, const char* description)


{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

#include <thread>
#include <vector>

#include <asio.hpp>
#include <nlohmann/json.hpp>

#include "models/map.hpp"

using asio::ip::tcp;
using json = nlohmann::json;

json fetch(const json& request_payload) {
    asio::io_context io_context;
    tcp::socket socket(io_context);
    tcp::resolver resolver(io_context);
    asio::connect(socket, resolver.resolve("127.0.0.1", "8080"));

    std::string request_str = request_payload.dump() + '\n';
    asio::write(socket, asio::buffer(request_str));

    asio::streambuf buffer;
    asio::read_until(socket, buffer, '\n');
    
    std::istream is(&buffer);
    std::string response_str;
    std::getline(is, response_str);

    return json::parse(response_str);
}

void add_simulation() {
    std::thread worker([]() {
        // adding & returns call client immediately
        std::system("./build/src/apps/call_client/call_client &");
    });
    
    worker.detach();
}

std::vector<int> get_all_simulations() {
    std::vector<int> simulation_ids;

    json request = {
        {"get_all_simulations", true}
    };    
    json response = fetch(request);
    if (response.contains("simulation_ids")) {
        simulation_ids = response["simulation_ids"].get<std::vector<int>>();
    }
    
    return simulation_ids;
}

std::vector<Cell> get_simulation_state(int simulation_id) {
    std::vector<Cell> cells;
    
    json request = {
        {"get_simulation", simulation_id}
    };
    json response = fetch(request);
    if (response.contains("simulation")) {
        for (auto &cell : response["simulation"]) {
            Cell c;
            c.x = cell["x"];
            c.y = cell["y"];
            c.cell_type = static_cast<CellType>(cell["cell_type"].get<int>());
            cells.push_back(c);
        }
    }
    
    return cells;
}

// Main code
int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
    const char* glsl_version = "#version 300 es";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
    GLFWwindow* window = glfwCreateWindow((int)(1280 * main_scale), (int)(800 * main_scale), "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If fonts are not explicitly loaded, Dear ImGui will select an embedded font: either AddFontDefaultVector() or AddFontDefaultBitmap().
    //   This selection is based on (style.FontSizeBase * style.FontScaleMain * style.FontScaleDpi) reaching a small threshold.
    // - You can load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - If a file cannot be loaded, AddFont functions will return a nullptr. Please handle those errors in your code (e.g. use an assertion, display an error and quit).
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use FreeType for higher quality font rendering.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //style.FontSizeBase = 20.0f;
    //io.Fonts->AddFontDefaultVector();
    //io.Fonts->AddFontDefaultBitmap();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    //IM_ASSERT(font != nullptr);

    // Our state
    bool show_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        static std::vector<int> all_ids;
        static int simulation_id = -1;
        std::string dropdown_text = (simulation_id == -1) ? "Select Simulation" : "Simulation " + std::to_string(simulation_id);

        // used for both simulation state and all simulations
        auto now = std::chrono::steady_clock::now();  

        if (show_window)
        {
            ImGui::Begin("Control Panel");  

            static auto last_all_simulations_poll_time = std::chrono::steady_clock::now(); 

            if (ImGui::Button("Add Simulation")) {
                add_simulation();  
            }     

            // 
            if (std::chrono::duration_cast<std::chrono::seconds>(now - last_all_simulations_poll_time).count() >= 1) {
                last_all_simulations_poll_time = now;
                std::thread worker([]() {
                    all_ids = get_all_simulations();
                });
                worker.detach();
            }
            
            if (ImGui::BeginCombo(" ", dropdown_text.c_str())) {
                for (int id : all_ids) {
                    bool selected = (simulation_id == id);    
                    if (ImGui::Selectable(std::to_string(id).c_str(), selected)) {
                        simulation_id = id; 
                    }
                    if (selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
    
                ImGui::EndCombo();
            }
                                    
            ImGui::End();

            ImGui::Begin("Simulation");
    
            static std::vector<Cell> cache_cells;
            static int last_seen_simulation_id = -1;

            static auto last_simulation_state_poll_time = std::chrono::steady_clock::now();

            if (simulation_id != -1) {
                if (simulation_id != last_seen_simulation_id || 
                    std::chrono::duration_cast<std::chrono::seconds>(now - last_simulation_state_poll_time).count() >= 1) 
                {
                    last_seen_simulation_id = simulation_id;
                    last_simulation_state_poll_time = now;

                    std::thread worker([]() {
                        auto new_cells = get_simulation_state(simulation_id);
                        cache_cells = std::move(new_cells); 
                    });
                    worker.detach();
                }
            }

            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 offset = ImGui::GetCursorScreenPos();
            int size = 20;
            int gap = 2;

            for (int x = 0; x < 30; x++) {
                for (int y = 0; y < 30; y++) {
                    ImU32 cell_color = IM_COL32(255, 255, 255, 255);

                    if (simulation_id != -1) {
                        for (Cell &cell : cache_cells) {
                            if (cell.x == x && cell.y == y) {
                                if (cell.cell_type == CellType::Call) {
                                    cell_color = IM_COL32(255, 0, 0, 255); // red
                                } else if (cell.cell_type == CellType::Road) {
                                    cell_color = IM_COL32(128, 128, 128, 255); // gray
                                } else if (cell.cell_type == CellType::Ambulance) {
                                    cell_color = IM_COL32(0, 0, 255, 255); // blue
                                } else if (cell.cell_type == CellType::Hospital) {
                                    cell_color = IM_COL32(0, 255, 0, 255); // green
                                } else if (cell.cell_type == CellType::Station) {
                                    cell_color = IM_COL32(255, 255, 0, 255); // yellow
                                } else if (cell.cell_type == CellType::ExpiredCall) {
                                    cell_color = IM_COL32(0, 0, 0, 255); // black
                                }
                                break;
                            }
                        }
                    }

                    float x1 = x * (size + gap);
                    float y1 = y * (size + gap);
                    float x2 = x1 + size;
                    float y2 = y1 + size;
                    ImVec2 top_left = {offset.x + x1, offset.y + y1};
                    ImVec2 bottom_right = {offset.x + x2, offset.y + y2};
                    
                    draw_list->AddRectFilled(top_left, bottom_right, cell_color);
                }
            }

            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}