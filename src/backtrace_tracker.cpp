#include "backtrace_tracker.h"

// #include "zmalloc.h"

#include <string>
#include <map>

#include <execinfo.h>

#define BT_BUF_SIZE 1024*64

using namespace std;

extern "C" {
    // hacky HACK
//     extern void *zmalloc_dram(size_t size);
//     extern void zfree(void*);
// #include "zmalloc.h"
}

static string get_backtrace(void) {
    int nptrs;
    void *buffer[BT_BUF_SIZE];
    char **strings;

    nptrs = backtrace(buffer, BT_BUF_SIZE);

    /* The call backtrace_symbols_fd(buffer, nptrs, STDOUT_FILENO)
        would produce similar output to the following: */

    strings = backtrace_symbols(buffer, nptrs);
    if (strings == NULL) {
        perror("backtrace_symbols");
        exit(EXIT_FAILURE);
    }

    string ret = "";
    for (int j = 0; j < nptrs; j++)
        ret += string(strings[j]);

    free(strings);

    return ret;
}

class Tracker {
    string highestBacktrace;
    size_t highestSize = 0u;
    std::map<string, size_t> backtrace2size;
public:
    void AddAlloc(size_t size) {
        string name = get_backtrace();
        if (backtrace2size.find(name) != backtrace2size.end()) {
            backtrace2size[name] += size;
        } else {
            backtrace2size[name] = size;
        }
        if (backtrace2size[name] > highestSize) {
            highestSize = backtrace2size[name];
            highestBacktrace = name;
        }
    }
};

extern "C" struct tracker_handle {
    Tracker tracker;
};

tracker_handle *tracker_create(void) {
    // WARNING this is called from zmalloc(), so call to zmalloc_dram has
    // to be used to avoid infinite recursion
//     tracker_handle *handle = (tracker_handle*)zmalloc_dram(sizeof(tracker_handle));
    static tracker_handle handle; // = (tracker_handle*)zmalloc_dram(sizeof(tracker_handle));
    new (&handle.tracker) Tracker();

    return &handle;
}

void tracker_destroy(tracker_handle *h) {
    h->tracker.~Tracker();
//     zfree(h);
}

void tracker_add(tracker_handle *h, size_t size) {
    h->tracker.AddAlloc(size);
}
