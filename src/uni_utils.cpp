#include <uni_utils.h>

timespec get_current_time()
{
    timespec ts{};
    timespec_get(&ts, TIME_UTC);
    return ts;
}
