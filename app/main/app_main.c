#include "app_main.h"

#include "app_interface.h"
#include "board.h"
#include "project_config.h"

static const app_interface_t *g_app;
static int g_app_init_status = -1;

void app_main_init(void)
{
    board_init();

    g_app = APP_INTERFACE_GET_FN();
    if ((g_app == 0) || (g_app->init == 0))
    {
        g_app_init_status = -1;
        return;
    }

    g_app_init_status = g_app->init();
}

void app_main_loop(void)
{
    if ((g_app_init_status == 0) &&
        (g_app != 0) &&
        (g_app->loop != 0))
    {
        g_app->loop();
    }
}
