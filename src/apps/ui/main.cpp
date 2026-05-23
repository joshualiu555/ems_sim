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

#ifdef _WIN32

#include <windows.h>

static HANDLE server_process = nullptr;

static STARTUPINFOA make_silent_si() {
    HANDLE devnull = CreateFileA("NUL", GENERIC_WRITE, FALSE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    STARTUPINFOA si = { 
        sizeof(si) 
    };
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = nullptr;
    si.hStdOutput = devnull;
    si.hStdError = devnull;
    return si;
}

static void start_server() {
    STARTUPINFOA si = make_silent_si();
    PROCESS_INFORMATION pi;
    char command[] = "build\\src\\apps\\dispatch_server\\dispatch_server.exe";
    if (CreateProcessA(nullptr, command, nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        server_process = pi.hProcess;
        CloseHandle(pi.hThread);
    }
    if (si.hStdOutput) CloseHandle(si.hStdOutput);
}
static void kill_server() {
    if (server_process) {
        TerminateProcess(server_process, 0);
        CloseHandle(server_process);
        server_process = nullptr;
    }
}
static void spawn_call_client(int simulation_id = -1) {
    STARTUPINFOA si = make_silent_si();
    PROCESS_INFORMATION pi;
    std::string command = "build\\src\\apps\\call_client\\call_client.exe";
    if (simulation_id != -1) command += " --simulation-id " + std::to_string(simulation_id);
    CreateProcessA(nullptr, command.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    if (si.hStdOutput) CloseHandle(si.hStdOutput);
}

#else

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>

static void silence_child() {
    int devnull = open("/dev/null", O_WRONLY);
    if (devnull >= 0) {
        dup2(devnull, STDOUT_FILENO);
        dup2(devnull, STDERR_FILENO);
        close(devnull);
    }
}

static pid_t server_pid = -1;

static void start_server() {
    server_pid = fork();
    if (server_pid == 0) {
        silence_child();
        execlp("./build/src/apps/dispatch_server/dispatch_server", "dispatch_server", nullptr);
        _exit(1);
    }
}
static void kill_server() {
    if (server_pid > 0) {
        kill(server_pid, SIGTERM);
        waitpid(server_pid, nullptr, 0);
        server_pid = -1;
    }
}
static void spawn_call_client(int simulation_id = -1) {
    pid_t pid = fork();
    if (pid == 0) {
        silence_child();
        if (simulation_id == -1) {
            execlp("./build/src/apps/call_client/call_client", "call_client", nullptr);
        } else {
            std::string id_str = std::to_string(simulation_id);
            execlp("./build/src/apps/call_client/call_client", "call_client", "--simulation-id", id_str.c_str(), nullptr);
        }
        _exit(1);
    }
}

#endif

json fetch(const json& request_payload) {
    try {
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
    } catch (const std::exception&) {
        return json{};
    }
}

void add_simulation() {
    std::thread worker([]() { 
        spawn_call_client(); 
    });
    
    worker.detach();
}

void delete_simulation(int simulation_id) {
    json request = {
        {"delete_simulation", simulation_id}
    };

    fetch(request);
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

            if (!cell["subtype"].is_null()) {
                c.subtype = cell["subtype"].get<int>(); 
            } else {
                c.subtype = std::nullopt; 
            }

            cells.push_back(c);
        }
    }
    
    return cells;
}

json get_analytics(int simulation_id) {
    json request = {
        {"get_analytics", simulation_id}
    };

    return fetch(request);
}

void draw_legend_item(const char *label, ImU32 color) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    int size = 16;

    draw_list -> AddRectFilled(
        cursor,
        ImVec2(cursor.x + size, cursor.y + size),
        color
    );

    ImGui::Dummy(ImVec2(size, size));
    ImGui::SameLine();

    ImGui::Text("%s", label);
};

bool paused = false;
void toggle_pause() {
    json request = {{ 
        paused ? "resume" : "pause", true 
    }};
    fetch(request);
    paused = !paused;
}

void set_speed(double multiplier) {
    json request = {{"set_speed", multiplier}};
    fetch(request);
}

std::vector<int> get_saved_simulations() {
    json request = {{"get_saved_simulations", true}};
    json response = fetch(request);
    if (response.contains("saved_simulation_ids"))
        return response["saved_simulation_ids"].get<std::vector<int>>();
    return {};
}

void save_simulation(int simulation_id) {
    json request = {{"save_simulation", simulation_id}};
    fetch(request);
}

int restore_simulation(int saved_id) {
    json request = {{"restore_simulation", saved_id}};
    json response = fetch(request);
    return response.value("simulation_id", -1);
}

int create_custom_simulation(const std::string& grid) {
    json request = {{"create_custom_simulation", grid}};
    json response = fetch(request);
    return response.value("simulation_id", -1);
}

// Main code
int main(int, char**)
{
    start_server();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

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
        static int last_seen_simulation_id = -1;
        static std::vector<int> saved_ids;
        static int selected_save_id = -1;
        static auto last_saved_sims_poll_time = std::chrono::steady_clock::now();

        // used for both simulation state and all simulations
        auto now = std::chrono::steady_clock::now();  

        if (show_window)
        {
            ImGui::Begin("Control Panel");  

            ImGui::Separator();

            if (ImGui::Button(paused ? "Resume" : "Pause")) {
                toggle_pause();
            }

            ImGui::SameLine();
            if (ImGui::Button("0.5x")) { 
                set_speed(0.5); 
            }
            ImGui::SameLine();
            if (ImGui::Button("1x")) { 
                set_speed(1.0); 
            }
            ImGui::SameLine();
            if (ImGui::Button("2x")) { 
                set_speed(2.0); 
            }

            static auto last_all_simulations_poll_time = std::chrono::steady_clock::now(); 

            std::string dropdown_text = (simulation_id == -1) ? "Select Simulation" : "Simulation " + std::to_string(simulation_id);

            if (ImGui::Button("Add Simulation")) {
                add_simulation();  
            }     

            if (ImGui::Button("Delete Simulation")) {
                delete_simulation(simulation_id);
                simulation_id = -1;
                last_seen_simulation_id = -1;
            }   

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

                if (simulation_id != -1) {
                if (ImGui::Button("Save")) {
                    int sid = simulation_id;
                    std::thread([sid]() { save_simulation(sid); }).detach();
                }
            }

            ImGui::Separator();

            if (std::chrono::duration_cast<std::chrono::seconds>(now - last_saved_sims_poll_time).count() >= 1) {
                last_saved_sims_poll_time = now;
                std::thread worker([]() {
                    saved_ids = get_saved_simulations();
                });
                worker.detach();
            }

            std::string restore_label = (selected_save_id == -1)
                ? "Select Save" : "Save " + std::to_string(selected_save_id);
            if (ImGui::BeginCombo("##restore", restore_label.c_str())) {
                for (int sid : saved_ids) {
                    bool sel = (selected_save_id == sid);
                    if (ImGui::Selectable(("Save " + std::to_string(sid)).c_str(), sel))
                        selected_save_id = sid;
                    if (sel) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            ImGui::SameLine();
            if (selected_save_id != -1 && ImGui::Button("Restore")) {
                int rsid = selected_save_id;
                std::thread([rsid]() {
                    int new_id = restore_simulation(rsid);
                    simulation_id = new_id;
                }).detach();
            }
                                    
            ImGui::End();

            ImGui::Begin("Custom Simulation");

            static char grid_buffer[930] = {};
            static bool grid_initialized = false;
            static std::string custom_status;

            if (!grid_initialized) {
                int position = 0;
                for (int row = 0; row < 30; row++) {
                    for (int col = 0; col < 30; col++) {
                        grid_buffer[position++] = '.';
                    }
                    if (row < 29) {
                        grid_buffer[position++] = '\n';
                    }
                }
                grid_buffer[position] = '\0';
                grid_initialized = true;
            }

            ImGui::Text("A=ALS  B=BLS  H=Hospital  .=empty");
            ImGui::InputTextMultiline("##customgrid", grid_buffer, sizeof(grid_buffer), ImVec2(0, 300));

            if (ImGui::Button("Generate")) {
                std::string raw(grid_buffer);
                std::string stripped;
                stripped.reserve(900);
                for (char c : raw) {
                    if (c != '\n' && c != '\r') {
                        stripped += (char)toupper(c);
                    }
                }

                bool valid = true;
                custom_status.clear();

                if (stripped.size() != 900) {
                    custom_status = "Error: grid must have exactly 900 non-newline characters (got " + std::to_string(stripped.size()) + ")";
                    valid = false;
                }
                if (valid) {
                    bool has_ambulance = false, has_hospital = false;
                    for (char c : stripped) {
                        if (c != '.' && c != 'A' && c != 'B' && c != 'H') {
                            custom_status = std::string("Error: invalid character '") + c + "' (use A, B, H, or .)";
                            valid = false;
                            break;
                        }
                        if (c == 'A' || c == 'B') has_ambulance = true;
                        if (c == 'H') has_hospital = true;
                    }
                    if (valid && !has_ambulance) { custom_status = "Error: place at least one ambulance (A or B)"; valid = false; }
                    if (valid && !has_hospital) { custom_status = "Error: place at least one hospital (H)"; valid = false; }
                }

                if (valid) {
                    std::thread([stripped]() {
                        int new_id = create_custom_simulation(stripped);
                        if (new_id != -1) {
                            simulation_id = new_id;
                            spawn_call_client(new_id);
                        }
                    }).detach();
                    custom_status = "Generating...";
                }
            }

            if (!custom_status.empty()) {
                ImGui::TextUnformatted(custom_status.c_str());
            }

            ImGui::End();

            ImGui::Begin("Legend");

            draw_legend_item("Alpha", IM_COL32(255, 204, 204, 255));
            draw_legend_item("Echo", IM_COL32(153, 0, 0, 255));
            draw_legend_item("ALS", IM_COL32(0, 0, 255, 255));
            draw_legend_item("BLS", IM_COL32(173, 216, 230, 255));
            draw_legend_item("Hospital", IM_COL32(0, 255, 0, 255));
            draw_legend_item("Station", IM_COL32(255, 255, 0, 255));
            draw_legend_item("Death", IM_COL32(0, 0, 0, 255));
            draw_legend_item("Discharged", IM_COL32(128, 0, 128, 255));

            ImGui::End();

            ImGui::Begin("Analytics");

            static json analytics;
            static auto last_analytics_poll_time = std::chrono::steady_clock::now();
            if (simulation_id != -1) {
                if (simulation_id != last_seen_simulation_id || 
                    std::chrono::duration_cast<std::chrono::seconds>(now - last_analytics_poll_time).count() >= 1) 
                {
                    last_seen_simulation_id = simulation_id;
                    last_analytics_poll_time = now;

                    std::thread worker([]() {
                        analytics = get_analytics(simulation_id);

                    });
                    worker.detach();
                }
            }

            if (!analytics.is_null()) {
                ImGui::Text("Completed: %d", analytics["completed_calls"].get<int>());
                ImGui::Text("Total: %d", analytics["total_calls"].get<int>());
                ImGui::Text("Success Rate: %.2f%%", analytics["success_rate"].get<double>() * 100);
                ImGui::Text("Avg Call Time: %.2f", analytics["average_call_time"].get<double>());
            } else {
                ImGui::Text("No data yet");
            }

            ImGui::End();

            ImGui::Begin("Simulation");

            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            static std::vector<Cell> cache_cells;

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

            ImVec2 simulation_offset = ImGui::GetCursorScreenPos();
            int size = 20;
            int gap = 2;

            for (int x = 0; x < 30; x++) {
                for (int y = 0; y < 30; y++) {
                    ImU32 cell_color = IM_COL32(255, 255, 255, 255);

                    if (simulation_id != -1) {
                        for (Cell &cell : cache_cells) {
                            if (cell.x == x && cell.y == y) {
                                if (cell.cell_type == CellType::Call) {
                                    if (cell.subtype == 0) cell_color = IM_COL32(255, 204, 204, 255); // alpha
                                    else if (cell.subtype == 1) cell_color = IM_COL32(255, 102, 102, 255); 
                                    else if (cell.subtype == 2) cell_color = IM_COL32(255, 0, 0, 255);   
                                    else if (cell.subtype == 3) cell_color = IM_COL32(204, 0, 0, 255); 
                                    else if (cell.subtype == 4) cell_color = IM_COL32(153, 0, 0, 255); // echo
                                } else if (cell.cell_type == CellType::Road) {
                                    cell_color = IM_COL32(128, 128, 128, 255); // gray
                                } else if (cell.cell_type == CellType::Ambulance) {
                                    if (cell.subtype == 0) cell_color = IM_COL32(0, 0, 255, 255); // als
                                    else if (cell.subtype == 1) cell_color = IM_COL32(173, 216, 230, 255); // bls
                                } else if (cell.cell_type == CellType::Hospital) {
                                    cell_color = IM_COL32(0, 255, 0, 255); // green
                                } else if (cell.cell_type == CellType::ALSStation || cell.cell_type == CellType::BLSStation) {
                                    cell_color = IM_COL32(255, 255, 0, 255); // yellow
                                } else if (cell.cell_type == CellType::ExpiredCall) {
                                    cell_color = IM_COL32(0, 0, 0, 255); // black
                                } else if (cell.cell_type == CellType::PatientDischarged) {
                                    cell_color = IM_COL32(128, 0, 128, 255); // purple
                                }
                                break;
                            }
                        }
                    }

                    float x1 = x * (size + gap);
                    float y1 = y * (size + gap);
                    float x2 = x1 + size;
                    float y2 = y1 + size;
                    ImVec2 top_left = {simulation_offset.x + x1, simulation_offset.y + y1};
                    ImVec2 bottom_right = {simulation_offset.x + x2, simulation_offset.y + y2};
                    
                    draw_list -> AddRectFilled(top_left, bottom_right, cell_color);
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

    kill_server();

    return 0;
}