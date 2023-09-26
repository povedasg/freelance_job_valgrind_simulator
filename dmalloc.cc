#define M61_DISABLE 1
#include "dmalloc.hh"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cinttypes>
#include <cassert>
#include <unordered_map>
#include <map>
#include <string>
#include <vector>
#include <algorithm>

// You may write code here.
// (Helper functions, types, structs, macros, globals, etc.)

#define LOW_CANARY 0xBEEFDEAD
#define HIGH_CANARY 0xDEADBEEF
#define PERCENTAGE_HEAVY_HITTER 12.0
#define FILE_NAME_SIZE 25

enum Error_types
{
    E_OK,
    E_NOT_IN_HEAP,
    E_DOUBLE_FREE,
    E_NOT_ALLOCATED,
    E_INCORRECT_CANARY,
    E_FREE_INSIDE_REGION
};

struct Malloc_info
{
    char file_name[FILE_NAME_SIZE];
    size_t size;
    bool used;
    size_t underflow;
    long line_malloc;
    void* payload;
    size_t* overflow;
};

struct Heavy_hitter
{
    char file_name[FILE_NAME_SIZE];
    long line;
    double percentage;
    size_t bytes;
};

//General Variables:
static std::unordered_map<std::string, Heavy_hitter> g_map_heavy_hitter;
static dmalloc_statistics g_manager_statistics;
static void *g_last_free = 0;
static std::unordered_map<void*,Malloc_info*> active_allocations;


//Internal Functions
void print_error(Error_types error_type, void * ptr, long line, Malloc_info* info, size_t region)
{
   switch (error_type)
    {
        case E_NOT_IN_HEAP:
            fprintf(stderr,"MEMORY BUG: %s:%ld: invalid free of pointer %p, not in heap\n", info->file_name,line,ptr);
            break;
        case E_DOUBLE_FREE:
            fprintf(stderr,"MEMORY BUG: %s:%ld: invalid free of pointer %p, double free\n", info->file_name,line,ptr);
            break;
        case E_NOT_ALLOCATED:
            if(info->file_name != nullptr)
                fprintf(stderr,"MEMORY BUG: %s:%ld: invalid free of pointer %p, not allocated\n", info->file_name,line, ptr);
            else
                fprintf(stderr,"MEMORY BUG???: invalid free of pointer %p, not allocated\n", ptr);
            break;
        case E_INCORRECT_CANARY:
            fprintf(stderr,"MEMORY BUG: %s:%ld: detected wild write during free of pointer %p\n", info->file_name,line, ptr);
            break;
        case E_FREE_INSIDE_REGION:
            fprintf(stderr,"  %s:%ld:  %p is %ld bytes inside a %ld byte region allocated here\n", info->file_name,info->line_malloc, ptr, region, info->size);
            break;
        default:
            break;
    }
}



void* fill_new_structure(void * malloc_payload, size_t size, const char * file, long line)
{
    // here we can use the unordered_map instead
    Malloc_info* new_data = nullptr;
    void* res = nullptr;
    auto iterator = active_allocations.find(malloc_payload);
    if(iterator != active_allocations.end()){
        new_data = iterator->second;
    }
    else{
        void* payload = (void *)((uintptr_t) malloc_payload + sizeof(Malloc_info));
        active_allocations[payload] = (Malloc_info *) malloc_payload;
        new_data = active_allocations[payload];
    }

    if(new_data != nullptr)
    {
        strcpy(new_data->file_name,file);
        new_data->size = size;
        new_data->used = true;
        new_data->underflow = LOW_CANARY;
        new_data->line_malloc = line;
        new_data->payload = (void *)((uintptr_t) malloc_payload + sizeof(Malloc_info));
        size_t* footer = (size_t *)((uintptr_t) new_data->payload + size);
        *footer = HIGH_CANARY;
        new_data->overflow = footer;

        //Update statistics:
        g_manager_statistics.nactive++;
        g_manager_statistics.active_size += size;
        g_manager_statistics.ntotal++;
        g_manager_statistics.total_size += size;
        if( (uintptr_t) new_data->payload > g_manager_statistics.heap_max)
        {
            g_manager_statistics.heap_max = (uintptr_t) new_data->payload + size;
        }
        if((g_manager_statistics.heap_min == 0) | ((uintptr_t) new_data->payload < g_manager_statistics.heap_min))
        {
            g_manager_statistics.heap_min = (uintptr_t) new_data->payload;
        }

        res = new_data->payload;

        //Fill heavy hitter data:
        Heavy_hitter *heavy_hitter = &g_map_heavy_hitter[std::string(new_data->file_name)+":"+std::to_string(new_data->line_malloc)];
        heavy_hitter->bytes += new_data->size;
        double percentage = (double)heavy_hitter->bytes/g_manager_statistics.total_size * 100;
        heavy_hitter->percentage = percentage;
        heavy_hitter->line = new_data->line_malloc;
        strcpy(heavy_hitter->file_name, new_data->file_name);
    }
    return res;
}

bool is_data_correct(Malloc_info * malloc_info)
{
    bool res = false;
    if(malloc_info->underflow == LOW_CANARY
        && *malloc_info->overflow  == HIGH_CANARY)
    {
        res = true; 
    }
    return res;
}

Error_types free_if_found(void* ptr, long line, Malloc_info* info, size_t region)
{
    Error_types error_free = E_OK;
    //Update statistics:
    g_manager_statistics.nactive--;
    g_manager_statistics.active_size -= info->size;
    //Lets check if the free is done correctly and the data is not Corrupt:
    if(!is_data_correct(info))
    {
        error_free = E_INCORRECT_CANARY;
        print_error(error_free,ptr,line,info,region);
        exit(1);
    }
    g_last_free = ptr;

    return error_free;
}

Error_types free_not_found(void* ptr, long line, Malloc_info* info, size_t region)
{
    Error_types error_free = E_OK;
    /* check if double free */
    if(ptr == g_last_free)
    {
        error_free = E_DOUBLE_FREE;
        print_error(error_free,ptr,line,info,region);
        exit(1);
    }
    for(auto it: active_allocations){
        info = it.second;
        if(info && ptr < info->payload){
            error_free = E_NOT_IN_HEAP;
            print_error(error_free,ptr,line,info, region);
            exit(1);
        }
        if(info && ptr >= info->payload && ptr < (void*)((uintptr_t)info->payload + info->size)){
            error_free = E_NOT_ALLOCATED;
            print_error(error_free,ptr,line,info, region);
            region = (uintptr_t) ptr - (uintptr_t)info->payload;
            error_free = E_FREE_INSIDE_REGION;
            print_error(error_free,info->payload,line,info, region);
            exit(1);
        }
    }
    return error_free;
}

void free_memory_manager(void* ptr, const char* file, long line)
{
    //First find the data structure:
    //int index = 0;
    bool found = 0;
    Error_types error_free = E_OK;
    size_t region = 0;

    if(ptr != nullptr)
    {
        auto data = active_allocations.find(ptr);
        Malloc_info* info = nullptr;
        if(data != active_allocations.end())
        {
            info = data->second;
        }
        else
        {
            error_free = free_not_found(ptr, line, info, region);
        }

        if (info != nullptr && info->used)
        {
            found = true;
            info->used = false;
        }
        else if (info != nullptr && info->used == false)
        {
            error_free = E_DOUBLE_FREE;
            print_error(error_free,ptr,line,info,region);
            exit(1);
        }
        
        if(found)
        {
           error_free = free_if_found(ptr, line, info, region);
        }
        else if((uintptr_t) ptr >= g_manager_statistics.heap_min && (uintptr_t) ptr <= g_manager_statistics.heap_max)
        {
            error_free = E_NOT_ALLOCATED;
            print_error(error_free,ptr,line,info,region);
            exit(1);
        }
        else
        {
            error_free = E_NOT_IN_HEAP;
            Malloc_info aux;
            strcpy(aux.file_name, file);
            print_error(error_free,ptr,line,&aux,region);
            exit(1);
        }
    }

}

bool comp(std::pair<std::string,Heavy_hitter> a, std::pair<std::string,Heavy_hitter> b) {
    return a.second.percentage > b.second.percentage;
}

/// dmalloc_malloc(sz, file, line)
///    Return a pointer to `sz` bytes of newly-allocated dynamic memory.
///    The memory is not initialized. If `sz == 0`, then dmalloc_malloc must
///    return a unique, newly-allocated pointer value. The allocation
///    request was at location `file`:`line`.

void* dmalloc_malloc(size_t sz, const char* file, long line) {
    // Your code here.
    void * res_malloc = 0;
    // We need to limit the size for the base_malloc to a maximum of size_t(Header, data, footer)
    if(sz > 0 && sz <= ((size_t) -1)-sizeof(Malloc_info)-sizeof(size_t))
    {
        res_malloc = base_malloc(sizeof(Malloc_info) + sz + sizeof(size_t)); //Add footer fow overflow.

        if (res_malloc != 0)
        {
            res_malloc = fill_new_structure(res_malloc,sz,file, line);
        }
        else
        {
            g_manager_statistics.nfail++;
            g_manager_statistics.fail_size += sz;
        }
    }
    else
    {
        g_manager_statistics.nfail++;
        g_manager_statistics.fail_size += sz;
    }

    return res_malloc;
}


/// dmalloc_free(ptr, file, line)
///    Free the memory space pointed to by `ptr`, which must have been
///    returned by a previous call to dmalloc_malloc. If `ptr == NULL`,
///    does nothing. The free was called at location `file`:`line`.

void dmalloc_free(void* ptr, const char* file, long line) {
    // Your code here.
    free_memory_manager(ptr, file, line);
    //Search whole memory to free (not only the payload data, need whole ST_Info stored):
    auto iterator = active_allocations.find(ptr);
    if(iterator != active_allocations.end())
    {
        base_free(iterator->second);
        active_allocations.erase(iterator->first);
    }
}


/// dmalloc_calloc(nmemb, sz, file, line)
///    Return a pointer to newly-allocated dynamic memory big enough to
///    hold an array of `nmemb` elements of `sz` bytes each. If `sz == 0`,
///    then must return a unique, newly-allocated pointer value. Returned
///    memory should be initialized to zero. The allocation request was at
///    location `file`:`line`.

void* dmalloc_calloc(size_t nmemb, size_t sz, const char* file, long line) {
    // Your code here (to fix test014).
    void* ptr = nullptr;
    if( (nmemb * sz) > 0 && (nmemb * sz) >= nmemb)//Check for overflow.
    {
        ptr = dmalloc_malloc(nmemb * sz, file, line);
        if (ptr) {
            memset(ptr, 0, nmemb * sz);
        }
    }
    else
    {
        g_manager_statistics.nfail++;
        g_manager_statistics.fail_size += (nmemb * sz);
    }
    return ptr;
}


/// dmalloc_get_statistics(stats)
///    Store the current memory statistics in `*stats`.

void dmalloc_get_statistics(dmalloc_statistics* stats) {
    // Stub: set all statistics to enormous numbers
    memset(stats, 255, sizeof(dmalloc_statistics));
    // Your code here.
    *stats = g_manager_statistics;
}


/// dmalloc_print_statistics()
///    Print the current memory statistics.

void dmalloc_print_statistics() {
    dmalloc_statistics stats;
    dmalloc_get_statistics(&stats);

    printf("alloc count: active %10llu   total %10llu   fail %10llu\n",
           stats.nactive, stats.ntotal, stats.nfail);
    printf("alloc size:  active %10llu   total %10llu   fail %10llu\n",
           stats.active_size, stats.total_size, stats.fail_size);
}


/// dmalloc_print_leak_report()
///    Print a report of all currently-active allocated blocks of dynamic
///    memory.

void dmalloc_print_leak_report() {
    // Your code here.

    Malloc_info* temp = nullptr;

    for(auto it : active_allocations){
         temp = it.second;
         if (temp != nullptr && temp->used == true)
        {
            printf("LEAK CHECK: %s:%ld: allocated object %p with size %ld\n",
                temp->file_name,
                temp->line_malloc,
                temp->payload,
                temp->size);
        }
    }
} 

/// dmalloc_print_heavy_hitter_report()
///    Print a report of heavily-used allocation locations.

void dmalloc_print_heavy_hitter_report() {
    // Your heavy-hitters code here

    std::vector<std::pair<std::string,Heavy_hitter>> values(g_map_heavy_hitter.begin(),g_map_heavy_hitter.end());
    std::sort(values.begin(),values.end(),comp);

    for(auto it: values)
    {
        if(it.second.percentage > PERCENTAGE_HEAVY_HITTER)
        {
            printf("HEAVY HITTER: %s:%ld: %ld bytes (~%.1lf%%)\n",
                it.second.file_name,
                it.second.line,
                it.second.bytes,
                it.second.percentage);
        }
    }
}
