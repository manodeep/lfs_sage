#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    struct locations_with_forests {
        int64_t forestid;
        int64_t treeid;
        int64_t offset;
        int64_t fileid;
    };

    struct filenames_and_fd {
        int *fd;
        int32_t numfiles;
        uint32_t nallocated;
    };

    struct additional_info{
        int64_t id;
        int64_t pid;
        int64_t upid;
        double desc_scale;
        int64_t descid;
        double scale;
    };

    /* externally exposed functions */
    extern int64_t read_forests(const char *filename, int64_t **forestids, int64_t **tree_rootids);
    extern int64_t read_locations(const char *filename, const int64_t ntrees, struct locations_with_forests *l, struct filenames_and_fd *filenames_and_fd);
    extern void assign_forest_ids(const int64_t ntrees, struct locations_with_forests *locations, int64_t *forests, int64_t *tree_roots);
    extern void sort_locations_on_fid_file_offset(const int64_t ntrees, struct locations_with_forests *locations);
    extern int fix_flybys(const int64_t totnhalos, struct halo_data *forest, struct additional_info *info, int verbose);
    extern int fix_upid(const int64_t totnhalos, struct halo_data *forest, struct additional_info *info, int *interrupted, const int verbose);
    extern void assign_mergertree_indices(const int64_t totnhalos, struct halo_data *forest, struct additional_info *info, const int max_snapnum);
    
#ifdef __cplusplus
}
#endif
