#include "core_allvars.h"

#if 0
/*
  MS: Are these only required for hdf5 output? Why
  are they global?
*/
#ifdef HDF5
char          *core_output_file;
size_t         HDF5_dst_size;
size_t        *HDF5_dst_offsets;
size_t        *HDF5_dst_sizes;
const char   **HDF5_field_names;
hid_t         *HDF5_field_types;
int            HDF5_n_props;
#endif
#endif
