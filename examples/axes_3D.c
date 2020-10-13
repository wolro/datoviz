#include <visky/visky.h>


#define N                100
#define MAX_VERTEX_COUNT 10000000
#define MAX_INDEX_COUNT  10000000
#define COLS             (N + 1)
#define ROWS             (2 * N + 1)
#define POINTS           (COLS * ROWS)

static const char* items[] = {"mesh", "spiral"};
static const char* citems[] = {"axes 3D", "FPS", "fly"};
static const VkyControllerType ctypes[] = {
    VKY_CONTROLLER_AXES_3D, VKY_CONTROLLER_FPS, VKY_CONTROLLER_FLY};
static int previous_item = 1;
static int current_item = 1;
static int cprevious_item = 0;
static int ccurrent_item = 0;
static VkyVisual* surface_visual = NULL;
static VkyVisual* spiral_visual = NULL;


static void generate_surface(VkyMesh* mesh)
{
    float* heights = calloc(POINTS, sizeof(float));
    VkyColor* color = calloc(POINTS, sizeof(VkyColor));
    float w = 1;
    float x, y, z;
    for (uint32_t i = 0; i < ROWS; i++)
    {
        x = (float)i / (ROWS - 1);
        x = -w + 2 * w * x;
        for (uint32_t j = 0; j < COLS; j++)
        {
            y = (float)j / (COLS - 1);
            y = -w + 2 * w * y;
            z = .25 * sin(10 * x) * cos(10 * y);
            heights[COLS * i + j] = z;
            color[COLS * i + j] = vky_color(VKY_CMAP_JET, 2 * (z + .25), 0, 1, 1);
        }
    }
    vec3 p00 = {-1, 0, -1}, p10 = {+1, 0, -1}, p01 = {-1, 0, +1};
    vky_mesh_grid_surface(mesh, ROWS, COLS, p00, p01, p10, heights, (cvec4*)color);
}


static void spiral(VkyPanel* panel)
{
    // Create the path visual.
    VkyPathParams params = {10, 2., VKY_CAP_ROUND, VKY_JOIN_ROUND, VKY_DEPTH_ENABLE};
    spiral_visual = vky_visual(panel->scene, VKY_VISUAL_PATH, &params, NULL);
    vky_add_visual_to_panel(spiral_visual, panel, VKY_VIEWPORT_INNER, VKY_VISUAL_PRIORITY_NONE);
    const uint32_t N_path = 1000;
    vec3 points[1000];
    VkyColor color[1000];
    double t = 0;
    for (uint32_t i = 0; i < N_path; i++)
    {
        t = (float)i / N_path;

        points[i][0] = t * cos(8 * M_2PI * t);
        points[i][1] = 2 * (.5 - t);
        points[i][2] = t * sin(8 * M_2PI * t);

        color[i] = vky_color(VKY_CMAP_JET, i, 0, N_path, 1);
    }
    VkyPathData path = {N_path, points, color, VKY_PATH_OPEN};
    spiral_visual->data.item_count = 1;
    spiral_visual->data.items = (VkyPathData[]){path};
    vky_visual_data_raw(spiral_visual);
}


static void surface(VkyPanel* panel)
{
    VkyMeshParams params = vky_default_mesh_params(
        VKY_MESH_COLOR_RGBA, VKY_MESH_SHADING_BLINN_PHONG, (ivec2){0, 0}, 0);
    surface_visual = vky_visual(panel->scene, VKY_VISUAL_MESH, &params, NULL);
    vky_allocate_vertex_buffer(surface_visual, MAX_VERTEX_COUNT * sizeof(VkyMeshVertex));
    vky_allocate_index_buffer(surface_visual, MAX_INDEX_COUNT * sizeof(VkyIndex));
    vky_add_visual_to_panel(surface_visual, panel, VKY_VIEWPORT_INNER, VKY_VISUAL_PRIORITY_NONE);
    VkyMesh mesh = vky_create_mesh(1e6, 1e6);
    generate_surface(&mesh);
    vky_mesh_upload(&mesh, surface_visual);
    vky_mesh_destroy(&mesh);
}


static void callback(VkyCanvas* canvas)
{
    if (current_item != previous_item)
    {
        vky_toggle_visual_visibility(current_item == 0 ? surface_visual : spiral_visual, true);
        vky_toggle_visual_visibility(current_item == 1 ? surface_visual : spiral_visual, false);
        previous_item = current_item;
    }
    if (ccurrent_item != cprevious_item)
    {
        vky_set_controller(&canvas->scene->grid->panels[0], ctypes[ccurrent_item], NULL);
        cprevious_item = ccurrent_item;
    }
}


int main()
{
    log_set_level_env();

    VkyApp* app = vky_create_app(VKY_DEFAULT_BACKEND);
    VkyCanvas* canvas = vky_create_canvas(app, VKY_DEFAULT_WIDTH, VKY_DEFAULT_HEIGHT);
    VkyScene* scene = vky_create_scene(canvas, VKY_CLEAR_COLOR_WHITE, 1, 1);
    vky_add_vertex_buffer(canvas->gpu, MAX_VERTEX_COUNT * sizeof(VkyMeshVertex));
    vky_add_index_buffer(canvas->gpu, MAX_INDEX_COUNT * sizeof(VkyIndex));

    // Set the panel controllers.
    VkyPanel* panel = vky_get_panel(scene, 0, 0);
    vky_set_controller(panel, VKY_CONTROLLER_AXES_3D, NULL);

    // axes(panel);
    surface(panel);
    vky_toggle_visual_visibility(surface_visual, false);
    spiral(panel);

    // GUI.
    VkyGui* gui = vky_create_gui(canvas, (VkyGuiParams){0, 0});

    VkyGuiListParams params = {2, items};
    vky_gui_control(gui, VKY_GUI_LISTBOX, "object", &params, &current_item);

    VkyGuiListParams cparams = {3, citems};
    vky_gui_control(gui, VKY_GUI_LISTBOX, "controller", &cparams, &ccurrent_item);
    vky_add_frame_callback(canvas, callback);

    vky_run_app(app);
    vky_destroy_app(app);

    return 0;
}
